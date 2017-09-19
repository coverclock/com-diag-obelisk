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

static const int PIN_OUT_P1 = 23; /* output, active low. */
static const int PIN_IN_T = 24; /* input */

int main(int argc, char ** argv)
{
    int rc = -1;
    FILE * pin_out_p1 = (FILE *)0;
    FILE * pin_in_t = (FILE *)0;
    diminuto_sticks_t ticks_before = -1;
    diminuto_sticks_t ticks_after = -1;

    /* Configure P1 output and T input pins. */

    pin_out_p1 = diminuto_pin_output(PIN_OUT_P1);
    if (pin_out_p1 == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_OUT_P1);
        assert(rc >= 0);
        pin_out_p1 = diminuto_pin_output(PIN_OUT_P1);
        assert(pin_out_p1 != (FILE *)0);
    }

    pin_in_t = diminuto_pin_input(PIN_IN_T);
    if (pin_in_t == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_IN_T);
        assert(rc >= 0);
        pin_in_t = diminuto_pin_input(PIN_IN_T);
        assert(pin_in_t != (FILE *)0);
    }

    rc = diminuto_pin_edge(PIN_IN_T, DIMINUTO_PIN_EDGE_BOTH);
    assert(rc >= 0);

    /* Toggle P1 output pin (active low). */

    rc = diminuto_pin_set(pin_out_p1);
    assert(rc == 0);

    diminuto_delay(diminuto_delay_frequency() / 2, 0);

    rc = diminuto_pin_clear(pin_out_p1);
    assert(rc == 0);

    diminuto_delay(diminuto_delay_frequency() / 2, 0);

   /* Peruse T input pin. */

    ticks_before = diminuto_time_elapsed();
    assert(ticks_before >= 0);

    ticks_after = diminuto_time_elapsed();
    assert(ticks_after >= 0);

    /* Release P1 output and T input pins. */

    pin_in_t = diminuto_pin_unused(pin_in_t, PIN_IN_T);
    assert(pin_in_t == (FILE *)0);

    pin_out_p1 = diminuto_pin_unused(pin_out_p1, PIN_OUT_P1);
    assert(pin_out_p1 == (FILE *)0);

    return 0;
}
