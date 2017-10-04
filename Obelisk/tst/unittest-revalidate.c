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

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

	{
        int count = -1;
        struct tm time = { 0 };
        int leap = -1;
        int days = -1;
        int gmonth = -1;
        int gday = -1;
        int rc = -1;

        TEST();

        count = 0;

        time.tm_wday = 5; /* 2016-01-01 */
        for (int year = 2016; year <= 2017; ++year) {
            leap = (((year % 4) == 0) && ((year % 400) != 0));
            time.tm_year = year - 1900;
            time.tm_yday = 0;
            for (int month = 1; month <= 12; ++month) {
                switch (month) {
                case 2:
                    days = leap ? 29 : 28;
                    break;
                case 4:
                case 6:
                case 9:
                case 11:
                    days = 30;
                    break;
                default:
                    days = 31;
                    break;
                }
                time.tm_mon = month - 1;
                for (time.tm_mday = 1; time.tm_mday <= days; ++time.tm_mday) {
                    rc = obelisk_zeller(time.tm_year + 1900, time.tm_mon + 1, time.tm_mday);
                    if (rc != time.tm_wday) {
                        CHECKPOINT("%04d-%02d-%02d %d (%d)\n",
                            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                            time.tm_wday, rc);
                    }
                    ASSERT(rc == time.tm_wday);
                    gmonth = -1;
                    gday = -1;
                    rc = obelisk_julian2gregorian(time.tm_yday + 1, leap, &gmonth, &gday);
                    if (rc < 0) {
                        CHECKPOINT("%04d-%02d-%02d %d (%d) %d %d\n",
                            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                            time.tm_yday, rc, gmonth, gday);
                    }
                    ASSERT(rc >= 0);
                    ASSERT(gmonth == (time.tm_mon + 1));
                    ASSERT(gday == time.tm_mday);
                    for (time.tm_hour = 0; time.tm_hour <= 23; ++time.tm_hour) {
                        for (time.tm_min = 0; time.tm_min <= 59; ++time.tm_min) {
                            for (time.tm_sec = 0; time.tm_sec <= 60; ++time.tm_sec) {
                                for (time.tm_isdst = -1; time.tm_isdst <= 1; ++time.tm_isdst) {
                                    ++count;
                                    rc = obelisk_revalidate(&time);
                                    if (rc < 0) {
                                        CHECKPOINT("%04d-%02d-%02dT%02d:%02d:%02d %d %d %d (%d)\n",
                                            time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                                            time.tm_hour, time.tm_min, time.tm_sec,
                                            time.tm_wday, time.tm_yday, time.tm_isdst,
                                            rc);
                                    }
                                    ASSERT(rc >= 0);
                                }
                            }
                        }
                    }
                    time.tm_wday = (time.tm_wday + 1) % 7;
                    time.tm_yday += 1;
                }
            }
        }

        CHECKPOINT("count %d\n", count);
        fflush(stdout);
        ASSERT(count == (366 + 367) * 24 * 60 * 60 * 3);

        STATUS();
    }

    EXIT();
}


