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

#define BADDATA(_JULIAN_, _LYI_) \
    do { \
        EXPECT(obelisk_julian2gregorian(_JULIAN_, _LYI_, &month, &day) < 0); \
        EXPECT(month == 0xdeadbeef); \
        EXPECT(day == 0xc0edbabe); \
    } while (0)

#define GOODDATA(_JULIAN_, _LYI_, _MONTH_, _DAY_) \
    do { \
        EXPECT(obelisk_julian2gregorian(_JULIAN_, _LYI_, &month, &day) >= 0); \
        EXPECT(month == _MONTH_); \
        EXPECT(day == _DAY_); \
    } while (0)

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        int month = 0xdeadbeef;
        int day = 0xc0edbabe;

        TEST();

        BADDATA( -1,  0);
        BADDATA( -1, !0);
        BADDATA(  0,  0);
        BADDATA(  0, !0);
        BADDATA(366,  0);
        BADDATA(367, !0);

        STATUS();
    }

	{
        int month = -1;
        int day = -1;

        TEST();

        GOODDATA(  1, 0,  1,  1);
        GOODDATA( 31, 0,  1,  31);
        GOODDATA( 32, 0,  2,  1);
        GOODDATA( 59, 0,  2,  28);
        GOODDATA( 60, 0,  3,  1);
        GOODDATA( 90, 0,  3,  31);
        GOODDATA( 91, 0,  4,  1);
        GOODDATA(120, 0,  4,  30);
        GOODDATA(121, 0,  5,  1);
        GOODDATA(151, 0,  5,  31);
        GOODDATA(152, 0,  6,  1);
        GOODDATA(181, 0,  6,  30);
        GOODDATA(182, 0,  7,  1);
        GOODDATA(212, 0,  7,  31);
        GOODDATA(213, 0,  8,  1);
        GOODDATA(243, 0,  8,  31);
        GOODDATA(244, 0,  9,  1);
        GOODDATA(273, 0,  9,  30);
        GOODDATA(274, 0, 10,  1);
        GOODDATA(304, 0, 10,  31);
        GOODDATA(305, 0, 11,  1);
        GOODDATA(334, 0, 11,  30);
        GOODDATA(335, 0, 12,  1);
        GOODDATA(365, 0, 12,  31);

        STATUS();
    }

	{
        int month = -1;
        int day = -1;

        TEST();

        GOODDATA(  1, !0,  1,  1);
        GOODDATA( 31, !0,  1,  31);
        GOODDATA( 32, !0,  2,  1);
        GOODDATA( 60, !0,  2,  29);
        GOODDATA( 61, !0,  3,  1);
        GOODDATA( 91, !0,  3,  31);
        GOODDATA( 92, !0,  4,  1);
        GOODDATA(121, !0,  4,  30);
        GOODDATA(122, !0,  5,  1);
        GOODDATA(152, !0,  5,  31);
        GOODDATA(153, !0,  6,  1);
        GOODDATA(182, !0,  6,  30);
        GOODDATA(183, !0,  7,  1);
        GOODDATA(213, !0,  7,  31);
        GOODDATA(214, !0,  8,  1);
        GOODDATA(244, !0,  8,  31);
        GOODDATA(245, !0,  9,  1);
        GOODDATA(274, !0,  9,  30);
        GOODDATA(275, !0, 10,  1);
        GOODDATA(305, !0, 10,  31);
        GOODDATA(306, !0, 11,  1);
        GOODDATA(335, !0, 11,  30);
        GOODDATA(336, !0, 12,  1);
        GOODDATA(366, !0, 12,  31);

        STATUS();
    }

    EXIT();
}
