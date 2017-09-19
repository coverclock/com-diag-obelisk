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

static const int PIN_OUT_P1 = 23; /* output, active low. */
static const int PIN_IN_T = 24; /* input */

int main(int argc, char ** argv)
{
    FILE * pin_out_p1 = (FILE *)0;
    FILE * pin_in_t = (FILE *)0;

    pin_out_p1 = diminuto_pin_output(PIN_OUT_P1);
    assert(pin_out_p1 != (FILE *)0);

    pin_in_t = diminuto_pin_input(PIN_IN_T);
    assert(pin_in_t != (FILE *)0);

    pin_in_t = diminuto_pin_unused(pin_in_t, PIN_IN_T);
    assert(pin_in_t == (FILE *)0);

    pin_out_p1 = diminuto_pin_unused(pin_out_p1, PIN_OUT_P1);
    assert(pin_out_p1 == (FILE *)0);

    return 0;
}
