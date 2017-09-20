/* vi: set ts=4 expandtab shiftwidth=4: */

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
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_RESET = 2;
static const int HERTZ_POLL = 100;
static const int MILLISECONDS_POLL = 1000 / 100;

typedef enum Token {
    TOKEN_ZERO = 0,
    TOKEN_ONE = 1, 
    TOKEN_MARKER = 2,
    TOKEN_INVALID = 3,
} token_t;

typedef struct Range {
    int symbol;
    int minimum;
    int maximum;
} range_t;

static const range_t RANGE[] = {
    { '0', 180, 220, }, /* TOKEN_ZERO */
    { '1', 480, 520, }, /* TOKEN_ONE */
    { 'M', 780, 820, }, /* TOKEN_MARKER */
    { '?', 0,   0,   }, /* TOKEN_INVALID */
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
    diminuto_ticks_t ticks_elapsed = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int state_raw = -1;
    int state_before = -1;
    int state_after = -1;
    int cycles_count = -1;
    int milliseconds_pulse = -1;
    token_t token = TOKEN_INVALID;

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
    ** Poll for T input pin changes.
    */

    diminuto_cue_init(&cue, 0);

    ticks_delay = ticks_frequency / HERTZ_POLL;

    cycles_count = 0;
    milliseconds_pulse = 0;
    state_before = 0;

    while (!0) {

        ticks_before = diminuto_time_elapsed();
        assert(ticks_before >= 0);

        state_raw = diminuto_pin_get(pin_in_t_fp);
        assert(state_raw >= 0);

        state_after = diminuto_cue_debounce(&cue, state_raw);

        if (state_after > state_before) {
            fprintf(stderr, "RISING!\n");
        } else if (state_after < state_before) {
            fprintf(stderr, "FALLING!\n");
        } else {
            /* Do nothing. */
        }

        edge = diminuto_cue_edge(&cue);

        switch (edge) {

        case DIMINUTO_CUE_EDGE_LOW:
            /* Do nothing. */
            break;

        case DIMINUTO_CUE_EDGE_RISING:
            cycles_count = 1;
            break;

        case DIMINUTO_CUE_EDGE_HIGH:
            ++cycles_count;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:
            milliseconds_pulse = cycles_count * MILLISECONDS_POLL;;
            cycles_count = 0;
            break;

        default:
            assert(edge == edge);
            break;

        }

        if (milliseconds_pulse > 0) {

            token = TOKEN_INVALID;
            for (token_t tt = TOKEN_ZERO; tt <= TOKEN_MARKER; ++tt) {
                if (milliseconds_pulse < RANGE[tt].minimum) {
                    /* Do nothing. */
                } else if (milliseconds_pulse > RANGE[tt].maximum) {
                    /* Do nothing. */
                } else {
                    token = tt;
                }
            }

            fprintf(stderr, "PULSE %dms %c\n", milliseconds_pulse, RANGE[token].symbol);

            milliseconds_pulse = 0;
        }

        state_before = state_after;

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
