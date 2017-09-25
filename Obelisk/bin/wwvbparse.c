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

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_DELAY = 2;
static const int HERTZ_TIMER = 10;

static const const char * TOKEN[] = {
    "ZERO",     /* OBELISK_TOKEN_ZERO */
    "ONE",      /* OBELISK_TOKEN_ONE */
    "MARKER",   /* OBELISK_TOKEN_MARKER */
    "INVALID",  /* OBELISK_TOKEN_INVALID */
    "PENDING",  /* OBELISK_TOKEN_PENDING */
};

static const char * STATE[] = {
    "WAIT",     /* OBELISK_STATE_WAIT */
    "SYNC",     /* OBELISK_STATE_SYNC */
    "START",    /* OBELISK_STATE_START */
    "DATA",     /* OBELISK_STATE_DATA */
    "MARK",     /* OBELISK_STATE_MARK */
    "END",      /* OBELISK_STATE_END */
};

int main(int argc, char ** argv)
{
    int rc = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    diminuto_sticks_t ticks_frequency = -1;
    diminuto_ticks_t ticks_delay = -1;
    diminuto_ticks_t ticks_timer = -1;
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_epoch = -1;
    diminuto_sticks_t ticks_then = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_ticks_t ticks_elapsed = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_before = -1;
    int level_after = -1;
    int milliseconds_before = -1;
    int milliseconds_after = -1;
    int milliseconds_pulse = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state_before = (obelisk_state_t)-1;
    obelisk_state_t state_after = (obelisk_state_t)-1;
    obelisk_buffer_t buffer = (obelisk_buffer_t)-1;
    obelisk_frame_t frame = { 0 };
    int field = -1;
    int bit = -1;
    int length = -1;
    int leap = -1;
    const char * program = (const char *)0;
    int opt = -1;
    extern char * optarg;
    char * endptr = (char *)0;
    int debug = 0;
    int reset = 0;
    int verbose = 0;
    int pin_out_p1 = PIN_OUT_P1;
    int pin_in_t = PIN_IN_T;
    int unexport = 0;

    program = strrchr(argv[0], '/');
    program = (program == (const char *)0) ? argv[0] : program + 1;

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
        case '?':
        default:
            fprintf(stderr, "usage: %s [ -d ] [ -h ] [ -p PIN ] [ -r ] [ -t PIN ] [ -u ] [ -v ]\n", program);
            fprintf(stderr, "       -d               Display debug output.\n");
            fprintf(stderr, "       -h               Display help menu.\n");
            fprintf(stderr, "       -p PIN           Define P1 output PIN (default %d).\n", pin_out_p1);
            fprintf(stderr, "       -r               Reset device initially.\n");
            fprintf(stderr, "       -t PIN           Define T input PIN (default %d).\n", pin_in_t);
            fprintf(stderr, "       -u               Unexport pins initially.\n");
            fprintf(stderr, "       -v               Display verbose output.\n");
            if (opt != 'h') { return 1; };
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
            fprintf(stderr, "%s: 0 UNEXPORTING.\n", program);
        }

        (void)diminuto_pin_unexport(pin_out_p1);

        (void)diminuto_pin_unexport(pin_in_t);

    }

    if (debug) {
        fprintf(stderr, "%s: 0 EXPORTING.\n", program);
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
            fprintf(stderr, "%s: 0 RESETTING.\n", program);
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
        fprintf(stderr, "%s: 0 INITIALIZING.\n", program);
    }

    diminuto_cue_init(&cue, 0);

    milliseconds_before = 0;
    milliseconds_pulse = 0;

    level_before = OBELISK_LEVEL_ZERO;

    token = OBELISK_TOKEN_PENDING;

    state_before = OBELISK_STATE_WAIT;

    /*
    ** Enter work loop.
    */

    rc = diminuto_alarm_install(!0);
    assert(rc >= 0);

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    ticks_timer = ticks_frequency / HERTZ_TIMER;

    ticks_slack = diminuto_timer_periodic(ticks_timer);
    assert(ticks_slack == 0);

    while (!0) {

        rc = pause();
        assert(rc == -1);

        if (diminuto_terminator_check()) {
            break;
        } else if (!diminuto_alarm_check()) {
            continue;
        } else {
            /* Do nothing. */
        }

        if (verbose) {
            ticks_now = diminuto_time_elapsed();
            assert(ticks_now >= 0);
            assert(ticks_now >= ticks_then);
            fprintf(stderr, "%s: 1 TICK %lldms.\n", program, diminuto_frequency_ticks2units(ticks_now - ticks_then, 1000));
        }

        /*
        ** Poll for T input pin change.
        */

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_after = diminuto_cue_debounce(&cue, level_raw);

        if (diminuto_cue_is_rising(&cue)) {
            ticks_epoch = diminuto_time_elapsed();;
            if (debug) {
                fprintf(stderr, "%s: 2 EPOCH.\n", program);
            }
        }

        if (!debug) {
            /* Di nothing. */
        } else if (level_after > level_before) {
            fprintf(stderr, "%s: 3 RISING.\n", program);
        } else if (level_after < level_before) {
            fprintf(stderr, "%s: 3 FALLING.\n", program);
        } else {
            /* Do nothing. */
        }

        level_before = level_after;

        /*
        ** Respond to edge transitions.
        */

        milliseconds_after = obelisk_measure(&cue, milliseconds_before, 1000 / HERTZ_TIMER);

        if (milliseconds_after < milliseconds_before) {
            milliseconds_pulse = milliseconds_before;
            if (debug) {
                fprintf(stderr, "%s: 4 PULSE %dms.\n", program, milliseconds_pulse);
            }
        }

        milliseconds_before = milliseconds_after;

        /*
        ** Classify pulse.
        */

        token = obelisk_tokenize(milliseconds_pulse);
        assert((0 <= token) && (token < countof(TOKEN)));

        if (!debug) {
            /* Do nothing. */
        } else if (token != OBELISK_TOKEN_PENDING) {
            fprintf(stderr, "%s: 5 TOKEN %s.\n", program, TOKEN[token]);
        } else {
            /* Do nothing. */
        }

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_after = obelisk_parse(state_before, token, &field, &length, &bit, &leap, &buffer);
        assert((0 <= state_before) && (state_before < countof(STATE)));
        assert((0 <= state_after) && (state_after < countof(STATE)));

        if (!debug) {
            /* Do nothing. */
        } else if (token == OBELISK_TOKEN_PENDING) {
            /* Do nothing. */
        } else if (state_after == OBELISK_STATE_WAIT) {
            /* Do nothing. */
        } else {
            fprintf(stderr, "%s: 6 STATE %s %s %d %d %d %d 0x%llx.\n", program, STATE[state_before], STATE[state_after], field, length, bit, leap, buffer);
        }

        if (!debug) {
            /* Do nothing. */
        } else if (state_before != OBELISK_STATE_END) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_START) {
            /* Do nothing. */
        } else {
            obelisk_extract(&frame, buffer);
            fprintf(stderr, "%s: 7 TIME %d%d-%d%d%d %d%d:%d%d %c0.%d %cLWI %cLSW %cDST.\n",
                program,
                frame.year10, frame.year1,
                frame.day100, frame.day10, frame.day1,
                frame.hours10, frame.hours1,
                frame.minutes10, frame.minutes1,
                (frame.sign == OBELISK_SIGN_POSITIVE) ? '+' : (frame.sign == OBELISK_SIGN_NEGATIVE) ? '-' : '?',
                frame.dut1,
                frame.lyi ? '+' : '-',
                frame.lsw ? '+' : '-',
                (frame.dst == OBELISK_DST_OFF) ? '-' : (frame.dst == OBELISK_DST_ENDS) ? '<' : (frame.dst == OBELISK_DST_BEGINS) ? '>' : (frame.dst == OBELISK_DST_ON) ? '+' : '?');
        }

        state_before = state_after;

    }

    /*
    ** Release resources.
    */

    if (debug) {
        fprintf(stderr, "%s: 0 RELEASING.\n", program);
    }

    pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, pin_in_t);
    assert(pin_in_t_fp == (FILE *)0);

    pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, pin_out_p1);
    assert(pin_out_p1_fp == (FILE *)0);

    /*
    ** Exit.
    */

    if (debug) {
        fprintf(stderr, "%s: 0 EXITING.\n", program);
    }

    return 0;
}
