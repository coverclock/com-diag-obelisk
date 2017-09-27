/* vim: set ts=4 expandtab shiftwidth=4: */

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock<BR>
 * mailto:coverclock@diag.com<BR>
 * http://github.com/coverclock/com-diag-obelisk<BR>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_timer.h"
#include "com/diag/diminuto/diminuto_alarm.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/obelisk/obelisk.h"

#define LOG(_FORMAT_, ...) fprintf(stderr, "%s: " _FORMAT_ "\n", program, ## __VA_ARGS__)

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_DELAY = 2;
static const int HERTZ_TIMER = 100;

static const const char * TOKEN[] = {
    "ZERO",     /* OBELISK_TOKEN_ZERO */
    "ONE",      /* OBELISK_TOKEN_ONE */
    "MARKER",   /* OBELISK_TOKEN_MARKER */
    "INVALID",  /* OBELISK_TOKEN_INVALID */
};

static const char * STATE[] = {
    "START",    /* OBELISK_STATE_START */
    "BEGIN",    /* OBELISK_STATE_BEGIN */
    "LEAP",     /* OBELISK_STATE_LEAP */
    "DATA",     /* OBELISK_STATE_DATA */
    "MARK",     /* OBELISK_STATE_MARK */
    "END",      /* OBELISK_STATE_END */
};

static const char * program = (const char *)0;
static int debug = 0;
static int reset = 0;
static int verbose = 0;
static int pin_out_p1 = -1;
static int pin_in_t = -1;
static int unexport = 0;

static void usage(void)
{
    fprintf(stderr, "usage: %s [ -d ] [ -h ] [ -p PIN ] [ -r ] [ -t PIN ] [ -u ] [ -v ]\n", program);
    fprintf(stderr, "       -d               Display debug output.\n");
    fprintf(stderr, "       -h               Display help menu.\n");
    fprintf(stderr, "       -p PIN           Define P1 output PIN (default %d).\n", pin_out_p1);
    fprintf(stderr, "       -r               Reset device initially.\n");
    fprintf(stderr, "       -t PIN           Define T input PIN (default %d).\n", pin_in_t);
    fprintf(stderr, "       -u               Unexport pins initially.\n");
    fprintf(stderr, "       -v               Display verbose output.\n");
}

int main(int argc, char ** argv)
{
    int rc = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    diminuto_sticks_t ticks_frequency = -1;
    diminuto_ticks_t ticks_delay = -1;
    diminuto_ticks_t ticks_timer = -1;
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_then = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_ticks_t ticks_elapsed = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_cooked = -1;
    diminuto_sticks_t milliseconds_elapsed = -1;
    diminuto_sticks_t milliseconds_leading = -1;
    diminuto_sticks_t milliseconds_rising = -1;
    diminuto_sticks_t milliseconds_epoch = -1;
    int milliseconds_cycle = -1;
    int milliseconds_pulse = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state_before = (obelisk_state_t)-1;
    obelisk_state_t state_after = (obelisk_state_t)-1;
    obelisk_buffer_t buffer = (obelisk_buffer_t)-1;
    obelisk_frame_t frame = { 0 };
    int field = -1;
    int length = -1;
    int leap = -1;
    int opt = -1;
    extern char * optarg;
    char optstr[2] = { '\0', '\0' };
    char * endptr = (char *)0;

    program = strrchr(argv[0], '/');
    program = (program == (const char *)0) ? argv[0] : program + 1;

    pin_out_p1 = PIN_OUT_P1;
    pin_in_t = PIN_IN_T;

    while ((opt = getopt(argc, argv, "dhp:rt:uv")) >= 0) {

        switch (opt) {

        case 'd':
            debug = !0;
            break;

        case 'p':
            pin_out_p1 = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_p1 < 0)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'r':
            reset = !0;
            break;

        case 't':
            pin_in_t = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_in_t < 0)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'u':
            unexport = !0;
            break;

        case 'v':
            verbose = !0;
            break;

        case 'h':
            usage();
            break;

        case '?':
        default:
            optstr[0] = opt;
            errno = EINVAL;
            perror(optstr);
            return 1;
            break;

        }

    }

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

    ticks_then = diminuto_time_elapsed();
    assert(ticks_then >= 0);

    ticks_frequency = diminuto_frequency();
    assert(ticks_frequency > 0);

    /*
    ** Configure P1 output and T input pins.
    */

    if (unexport) {

        if (debug) {
            LOG("0 UNEXPORTING.");
        }

        (void)diminuto_pin_unexport(pin_out_p1);

        (void)diminuto_pin_unexport(pin_in_t);

    }

    if (debug) {
        LOG("0 EXPORTING.");
    }

     pin_out_p1_fp = diminuto_pin_output(pin_out_p1);
     assert(pin_out_p1_fp != (FILE *)0);

     pin_in_t_fp = diminuto_pin_input(pin_in_t);
     assert(pin_in_t_fp != (FILE *)0);

    /*
    ** Toggle P1 output pin (active low) if requested.
    */

    if (reset) {

        if (debug) {
            LOG("0 RESETTING.");
        }

        ticks_delay = ticks_frequency / HERTZ_DELAY;

        rc = diminuto_pin_set(pin_out_p1_fp);
        assert(rc == 0);

        ticks_slack = diminuto_delay(ticks_delay, 0);
        assert(ticks_slack == 0);

        rc = diminuto_pin_clear(pin_out_p1_fp);
        assert(rc == 0);

        ticks_slack = diminuto_delay(ticks_delay, 0);
        assert(ticks_slack == 0);

    }

    /*
    ** Initialize.
    */

    if (debug) {
        LOG("0 INITIALIZING.");
    }

    diminuto_cue_init(&cue, 0);

    milliseconds_pulse = 0;

    milliseconds_cycle  = 1000 / HERTZ_TIMER;

    level_raw = OBELISK_LEVEL_ZERO;

    token = OBELISK_TOKEN_INVALID;

    state_before = OBELISK_STATE_START;

    rc = diminuto_alarm_install(!0);
    assert(rc >= 0);

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    ticks_timer = ticks_frequency / HERTZ_TIMER;

    ticks_slack = diminuto_timer_periodic(ticks_timer);
    assert(ticks_slack == 0);

    buffer = 0;

    /*
    ** Begin  work loop.
    */

    while (!0) {

        rc = pause();
        assert(rc == -1);

        if (diminuto_terminator_check()) {
            break;
        }

        if (!diminuto_alarm_check()) {
            continue;
        }

        ticks_now = diminuto_time_elapsed();
        assert(ticks_now >= 0);
        assert(ticks_now >= ticks_then);
        milliseconds_elapsed = diminuto_frequency_ticks2units(ticks_now - ticks_then, 1000);
        if (verbose) { LOG("1 TICK %lldms.", milliseconds_elapsed); }

        /*
        ** Determine T input pin state.
        */

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_cooked = diminuto_cue_debounce(&cue, level_raw);

        if (diminuto_cue_is_rising(&cue)) {
            milliseconds_leading = milliseconds_elapsed;
        }

        /*
        ** Look for edge transitions and measure pulse duration.
        */

        edge = diminuto_cue_edge(&cue);

        switch (edge) {

        case DIMINUTO_CUE_EDGE_LOW:
            /* Do nothing. */
            break;

        case DIMINUTO_CUE_EDGE_RISING:
            if (debug) { LOG("3 RISING %lldms.", milliseconds_elapsed); }
            milliseconds_pulse = milliseconds_cycle;
            milliseconds_rising = milliseconds_leading;
            break;

        case DIMINUTO_CUE_EDGE_HIGH:
            milliseconds_pulse += milliseconds_cycle;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:
            milliseconds_pulse += milliseconds_cycle;
            if (debug) { LOG("3 FALLING %lldms.", milliseconds_elapsed); }
            break;

        default:
            assert(edge != edge);
            milliseconds_pulse = 0;
            break;

        }

        if (edge != DIMINUTO_CUE_EDGE_FALLING) {
            continue;
        }

       if (debug) { LOG("4 PULSE %dms.", milliseconds_pulse); }

        /*
        ** Classify pulse.
        */

        token = obelisk_tokenize(milliseconds_pulse);
        assert((0 <= token) && (token < countof(TOKEN)));

        if (debug) { LOG("5 TOKEN %s.", TOKEN[token]); }

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_after = obelisk_parse(state_before, token, &field, &length, &leap, &buffer);
        assert((0 <= state_before) && (state_before < countof(STATE)));
        assert((0 <= state_after) && (state_after < countof(STATE)));

        if (!debug) {
            /* Do nothing. */
        } else if (state_before != OBELISK_STATE_START) {
            /* Do nothing. */
        } else if (state_after == OBELISK_STATE_START) {
            /* Do nothing. */
        } else {
            LOG("6 RESYNC!");
        }

        if (debug) {
            LOG("6 STATE %s %s %d %d %d 0x%llx.", STATE[state_before], STATE[state_after], field, length, leap, buffer);
        }


        if (state_before != OBELISK_STATE_BEGIN) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_LEAP) {
            /* Do nothing. */
        } else {
            milliseconds_epoch = milliseconds_rising;
            if (debug) { LOG("6 EPOCH %lldms.", milliseconds_epoch); }
        }

        if (!debug) {
            /* Do nothing. */
        } else if (state_before != OBELISK_STATE_END) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_BEGIN) {
            /* Do nothing. */
        } else {
            obelisk_extract(&frame, buffer);
            LOG("7 TIME %d %d / %d %d %d T %d %d : %d %d - %d %d %d %d %d.",
                frame.year10, frame.year1,
                frame.day100, frame.day10, frame.day1,
                frame.hours10, frame.hours1,
                frame.minutes10, frame.minutes1,
                frame.dutonesign,
                frame.dutone1,
                frame.lyi,
                frame.lsw,
                frame.dst
            );
        }

        state_before = state_after;

    }

    /*
    ** Release resources.
    */

    if (debug) {
        LOG("0 RELEASING.");
    }

    pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, pin_in_t);
    assert(pin_in_t_fp == (FILE *)0);

    pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, pin_out_p1);
    assert(pin_out_p1_fp == (FILE *)0);

    /*
    ** Exit.
    */

    if (debug) {
        LOG("0 EXITING.");
    }

    return 0;
}
