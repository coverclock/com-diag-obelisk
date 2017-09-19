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
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_time.h"

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */

int main(int argc, char ** argv)
{
    int rc = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    diminuto_sticks_t frequency = -1;
    diminuto_ticks_t delay = -1;
    diminuto_sticks_t slack = -1;
    diminuto_cue_state_t cue = { 0 };
    int edge_raw = -1;
    int edge_before = -1;
    int edge_after = -1;
    diminuto_sticks_t ticks_before = -1;
    diminuto_sticks_t ticks_after = -1;

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

    frequency = diminuto_delay_frequency();
    assert(frequency > 0);
    delay = frequency / 2;

    rc = diminuto_pin_set(pin_out_p1_fp);
    assert(rc == 0);

    slack = diminuto_delay(delay, 0);
    assert(slack == 0);

    rc = diminuto_pin_clear(pin_out_p1_fp);
    assert(rc == 0);

    slack = diminuto_delay(delay, 0);
    assert(slack == 0);

    /*
    ** Poll T input pin changes.
    */

    delay = frequency / 100;

    frequency = diminuto_time_frequency();
    assert(frequency > 0);

    diminuto_cue_init(&cue, 0);

    while (!0) {

        edge_raw = diminuto_pin_get(pin_in_t_fp);
        assert(edge_raw >= 0);

        edge_after = diminuto_cue_debounce(&cue, edge_raw);

        ticks_after = diminuto_time_elapsed();
        assert(ticks_after >= 0);

        if (edge_before < 0) {
            /* Do nothing: initialization. */
        } else if (edge_before == edge_after) {
            /* Do nothing: should never happen. */
        } else {
            printf("T %d %d %lld\n", edge_before, edge_after, (ticks_after - ticks_before) * 1000 / frequency);
        }

        edge_before = edge_after;
        ticks_before = ticks_after;

        slack = diminuto_delay(delay, 0);
        assert(slack == 0);
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
