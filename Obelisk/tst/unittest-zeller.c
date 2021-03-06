/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Diminuto.html<BR>
 */

#include "com/diag/diminuto/diminuto_unittest.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_core.h"
#include "com/diag/obelisk/obelisk.h"
#include "obelisk.h"
#include <stdio.h>
#include <errno.h>

#define GOODDATA(_YEAR_, _MONTH_, _DAY_, _WEEKDAY_) \
    EXPECT(obelisk_zeller(_YEAR_, _MONTH_, _DAY_) == _WEEKDAY_)

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        TEST();

        ASSERT(OBELISK_ZELLER_SUNDAY    == 0);
        ASSERT(OBELISK_ZELLER_MONDAY    == 1);
        ASSERT(OBELISK_ZELLER_TUESDAY   == 2);
        ASSERT(OBELISK_ZELLER_WEDNESDAY == 3);
        ASSERT(OBELISK_ZELLER_THURSDAY  == 4);
        ASSERT(OBELISK_ZELLER_FRIDAY    == 5);
        ASSERT(OBELISK_ZELLER_SATURDAY  == 6);

        STATUS();
    }

    {
        TEST();

        GOODDATA(2017,  1,  1, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2017,  1,  2, OBELISK_ZELLER_MONDAY);
        GOODDATA(2017,  1,  3, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2017,  1,  4, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2017,  1,  5, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2017,  1,  6, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2017,  1,  7, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2017,  1, 31, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2017,  2,  1, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2017,  2, 28, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2017,  3,  1, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2017,  3, 31, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2017,  4,  1, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2017,  4, 30, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2017,  5,  1, OBELISK_ZELLER_MONDAY);
        GOODDATA(2017,  5, 31, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2017,  6,  1, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2017,  6, 30, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2017,  7,  1, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2017,  7, 31, OBELISK_ZELLER_MONDAY);
        GOODDATA(2017,  8,  1, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2017,  8, 31, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2017,  9,  1, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2017,  9, 30, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2017, 10,  1, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2017, 10, 31, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2017, 11,  1, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2017, 11, 30, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2017, 12,  1, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2017, 12, 31, OBELISK_ZELLER_SUNDAY);

        STATUS();
    }

    {
        TEST();

        GOODDATA(2016,  1,  1, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2016,  1,  2, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2016,  1,  3, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2016,  1,  4, OBELISK_ZELLER_MONDAY);
        GOODDATA(2016,  1,  5, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2016,  1,  6, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2016,  1,  7, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2016,  1, 31, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2016,  2,  1, OBELISK_ZELLER_MONDAY);
        GOODDATA(2016,  2, 29, OBELISK_ZELLER_MONDAY);
        GOODDATA(2016,  3,  1, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2016,  3, 31, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2016,  4,  1, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2016,  4, 30, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2016,  5,  1, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2016,  5, 31, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2016,  6,  1, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2016,  6, 30, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2016,  7,  1, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2016,  7, 31, OBELISK_ZELLER_SUNDAY);
        GOODDATA(2016,  8,  1, OBELISK_ZELLER_MONDAY);
        GOODDATA(2016,  8, 31, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2016,  9,  1, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2016,  9, 30, OBELISK_ZELLER_FRIDAY);
        GOODDATA(2016, 10,  1, OBELISK_ZELLER_SATURDAY);
        GOODDATA(2016, 10, 31, OBELISK_ZELLER_MONDAY);
        GOODDATA(2016, 11,  1, OBELISK_ZELLER_TUESDAY);
        GOODDATA(2016, 11, 30, OBELISK_ZELLER_WEDNESDAY);
        GOODDATA(2016, 12,  1, OBELISK_ZELLER_THURSDAY);
        GOODDATA(2016, 12, 31, OBELISK_ZELLER_SATURDAY);

        STATUS();
    }

    EXIT();
}

