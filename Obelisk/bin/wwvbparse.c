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
#include <string.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_timer.h"
#include "com/diag/diminuto/diminuto_alarm.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/obelisk/obelisk.h"

#define LOG DIMINUTO_LOG_DEBUG

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_DELAY = 2;
static const int HERTZ_TIMER = 10;

static const const char * TOKEN[] = {
    "ZERO",
    "ONE",
    "MARKER",
    "INVALID",
    "PENDING",
};

static const char * STATE[] = {
    "WAIT",
    "SYNC",
    "START",
    "DATA",
    "MARK",
    "END",
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
    int field = -1;
    int bit = -1;
    int length = -1;
    int leap = -1;
    const char * program = (const char *)0;
    int opt = -1;
    extern char * optarg;
    int debug = 0;
    int verbose = 0;

    program = strrchr(argv[0], '/');
    program = (program == (const char *)0) ? argv[0] : program + 1;

    while ((opt = getopt(argc, argv, "dhv")) >= 0) {

        switch (opt) {

        case 'd':
            debug = !0;
            break;

        case 'v':
            verbose = !0;
            break;

        case 'h':
        case '?':
        default:
            fprintf(stderr, "usage: %s [ -d ] [ -h ] [ -v ]\n", program);
            fprintf(stderr, "       -d               Log debug output.\n");
            fprintf(stderr, "       -h               Display help menu.\n");
            fprintf(stderr, "       -v               Log verbose output.\n");
            if (opt != 'h') { return 1; };
            break;

        }

    }

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

    diminuto_log_setmask();

    ticks_frequency = diminuto_frequency();
    assert(ticks_frequency > 0);

    /*
    ** Configure P1 output and T input pins.
    */

     rc = diminuto_pin_unexport(PIN_OUT_P1);
     assert(rc >= 0);

     pin_out_p1_fp = diminuto_pin_output(PIN_OUT_P1);
     assert(pin_out_p1_fp != (FILE *)0);

     rc = diminuto_pin_unexport(PIN_IN_T);
     assert(rc >= 0);

     pin_in_t_fp = diminuto_pin_input(PIN_IN_T);
     assert(pin_in_t_fp != (FILE *)0);

    /*
    ** Toggle P1 output pin (active low).
    */

    ticks_delay = ticks_frequency / HERTZ_DELAY;

    rc = diminuto_pin_set(pin_out_p1_fp);
    assert(rc == 0);

    ticks_slack = diminuto_delay(ticks_delay, 0);
    assert(ticks_slack == 0);

    rc = diminuto_pin_clear(pin_out_p1_fp);
    assert(rc == 0);

    ticks_slack = diminuto_delay(ticks_delay, 0);
    assert(ticks_slack == 0);

    /*
    ** Initialize.
    */

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

    ticks_timer = ticks_frequency / HERTZ_TIMER;

    ticks_slack = diminuto_timer_periodic(ticks_timer);
    assert(ticks_slack == 0);

    while (!0) {

        rc = pause();
        assert(rc == -1);

        if (!diminuto_alarm_check()) {
            continue;
        }

        if (verbose) { LOG("0: TICK.\n"); }

        /*
        ** Poll for T input pin change.
        */

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_after = diminuto_cue_debounce(&cue, level_raw);

        if (diminuto_cue_is_rising(&cue)) {
            ticks_epoch = diminuto_time_elapsed();;
            if (debug) { LOG("1: EPOCH.\n"); }
        }

        if (!debug) {
            /* Di nothing. */
        } else if (level_after > level_before) {
            LOG("2: RISING.\n");
        } else if (level_after < level_before) {
            LOG("2: FALLING.\n");
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
            if (debug) { LOG("3: PULSE %dms.\n", milliseconds_pulse); }
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
            LOG("4: TOKEN %s.\n", TOKEN[token]);
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
        } else if ((token != OBELISK_TOKEN_PENDING) && (state_after != OBELISK_STATE_WAIT)) {
            LOG("5: STATE %s %s %d %d %d 0x%llx.\n", STATE[state_before], STATE[state_after], field, length, bit, buffer);
        } else {
            /* Do nothing. */
        }

        if (!debug) {
            /* Do nothing. */
        } else if ((state_before == OBELISK_STATE_END) && (state_after == OBELISK_STATE_START)) {
            LOG("6: FRAME 0x%llx.\n", buffer);
        } else {
            /* Do nothing. */
        }

        state_before = state_after;

    }

    /*
    ** Release resources.
    */

    pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, PIN_IN_T);
    assert(pin_in_t_fp == (FILE *)0);

    pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, PIN_OUT_P1);
    assert(pin_out_p1_fp == (FILE *)0);

    return 0;
}
