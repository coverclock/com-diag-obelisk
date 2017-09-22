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
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/obelisk/obelisk.h"

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_RESET = 2;
static const int HERTZ_POLL = 100;
static const int MILLISECONDS_POLL = 1000 / 100;

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
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_before = -1;
    diminuto_sticks_t ticks_after = -1;
    diminuto_sticks_t ticks_epoch = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_ticks_t ticks_elapsed = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_before = -1;
    int level_after = -1;
    int cycles_before = -1;
    int cycles_after = -1;
    int milliseconds_pulse = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state_before = (obelisk_state_t)-1;
    obelisk_state_t state_after = (obelisk_state_t)-1;
    obelisk_buffer_t buffer = { 0 };
    int field = -1;
    int bit = -1;
    int length = -1;
    int leap = -1;

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_record_t) == sizeof(uint64_t));

    diminuto_log_setmask();

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

    ticks_frequency = diminuto_delay_frequency();
    assert(ticks_frequency > 0);
    ticks_delay = ticks_frequency / HERTZ_RESET;

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

    ticks_delay = ticks_frequency / HERTZ_POLL;

    cycles_before = 0;
    cycles_after = 0;

    milliseconds_pulse = 0;

    level_before = LEVEL_ZERO;

    token = TOKEN_PENDING;

    state_after = STATE_WAIT;

    /*
    ** Enter work loop.
    */

    while (!0) {

        /*
        ** Poll for T input pin change.
        */

        ticks_before = diminuto_time_elapsed();
        assert(ticks_before >= 0);

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_after = diminuto_cue_debounce(&cue, level_raw);

        if (diminuto_cue_is_rising(&cue)) {
            ticks_epoch = ticks_before;
            DIMINUTO_LOG_DEBUG("EPOCH.\n");
        }

        if (level_after > level_before) {
            DIMINUTO_LOG_DEBUG("RISING.\n");
        } else if (level_after < level_before) {
            DIMINUTO_LOG_DEBUG("FALLING.\n");
        } else {
            /* Do nothing. */
        }

        /*
        ** Respond to edge transitions.
        */

        cycles_after = obelisk_measure(&cue, cycles_before);

        if (cycles_after < cycles_before) {
            milliseconds_pulse = cycles_before * MILLISECONDS_POLL;
            DIMINUTO_LOG_DEBUG("PULSE %dms.\n", milliseconds_pulse);
        }

        cycles_before = cycles_after;

        /*
        ** Classify pulse.
        */

        token = obelisk_tokenize(milliseconds_pulse);

        if (token != TOKEN_PENDING) {
            assert((0 <= token) && (token < countof(TOKEN)));
            DIMINUTO_LOG_DEBUG("TOKEN %s.\n", TOKEN[token]);
        }

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_before = state_after;
        state_after = obelisk_parse(state_before, token, &field, &length, &bit, &leap, &buffer);

        if ((token != TOKEN_PENDING) && (state_after != STATE_WAIT)) {
            assert((0 <= state_before) && (state_before < countof(STATE)));
            assert((0 <= state_after) && (state_after < countof(STATE)));
            DIMINUTO_LOG_DEBUG("STATE %s->%s %d %d %d 0x%llx.\n", STATE[state_before], STATE[state_after], field, length, bit, buffer.word);
        }

        if ((state_before == STATE_END) && (state_after == STATE_START)) {
            DIMINUTO_LOG_DEBUG("FRAME 0x%llx.\n", buffer.word);
        }

        /*
        ** Iterate while controlling jitter.
        */

        level_before = level_after;

        ticks_after = diminuto_time_elapsed();
        assert(ticks_after >= 0);

        assert(ticks_after >= ticks_before);
        ticks_elapsed = ticks_after - ticks_before;

        if (ticks_elapsed < ticks_delay) {
            ticks_slack = diminuto_delay(ticks_delay - ticks_elapsed, 0);
            assert(ticks_slack == 0);
        }

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
