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

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

    diminuto_log_setmask();

    ticks_frequency = diminuto_frequency();
    assert(ticks_frequency > 0);

    /*
    ** Configure P1 output and T input pins.
    */

    pin_out_p1_fp = diminuto_pin_output(PIN_OUT_P1);
    if (pin_out_p1_fp == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_OUT_P1);
        assert(rc >= 0);
        pin_out_p1_fp = diminuto_pin_output(PIN_OUT_P1);
        assert(pin_out_p1_fp != (FILE *)0);
    }

    pin_in_t_fp = diminuto_pin_input(PIN_IN_T);
    if (pin_in_t_fp == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_IN_T);
        assert(rc >= 0);
        pin_in_t_fp = diminuto_pin_input(PIN_IN_T);
        assert(pin_in_t_fp != (FILE *)0);
    }

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

    level_before = LEVEL_ZERO;

    token = TOKEN_PENDING;

    state_before = STATE_WAIT;

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

        /*
        ** Poll for T input pin change.
        */

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        if (diminuto_cue_is_rising(&cue)) {
            ticks_epoch = diminuto_time_elapsed();;
            LOG("0: EPOCH.\n");
        }

        level_after = diminuto_cue_debounce(&cue, level_raw);

        if (level_after > level_before) {
            LOG("1: RISING.\n");
        } else if (level_after < level_before) {
            LOG("1: FALLING.\n");
        } else {
            /* Do nothing. */
        }

        /*
        ** Respond to edge transitions.
        */

        milliseconds_after = obelisk_measure(&cue, milliseconds_before, 1000 / HERTZ_TIMER);

        if (milliseconds_after < milliseconds_before) {
            milliseconds_pulse = milliseconds_before;
            LOG("2: PULSE %dms.\n", milliseconds_pulse);
        }

        milliseconds_before = milliseconds_after;

        /*
        ** Classify pulse.
        */

        token = obelisk_tokenize(milliseconds_pulse);

        if (token != TOKEN_PENDING) {
            assert((0 <= token) && (token < countof(TOKEN)));
            LOG("3: TOKEN %s.\n", TOKEN[token]);
        }

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_after = obelisk_parse(state_before, token, &field, &length, &bit, &leap, &buffer);

        if ((token != TOKEN_PENDING) && (state_after != STATE_WAIT)) {
            assert((0 <= state_before) && (state_before < countof(STATE)));
            assert((0 <= state_after) && (state_after < countof(STATE)));
            LOG("4: STATE %s->%s %d %d %d 0x%llx.\n", STATE[state_before], STATE[state_after], field, length, bit, buffer);
        }

        if ((state_before == STATE_END) && (state_after == STATE_START)) {
            LOG("5: FRAME 0x%llx.\n", buffer);
        }

        state_before = state_after;

        /*
        ** Iterate while controlling jitter.
        */

        level_before = level_after;

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
