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
        struct tm time = { 0 };
        int days = -1;
        int rc = -1;

        TEST();

        for (int year = 2016; year <= 2017; ++year) {
            time.tm_year = year - 1900;
            time.tm_yday = 0;
            for (int month = 1; month <= 12; ++month) {
                switch (month) {
                case 2:
                    days = (((year % 4) == 0) && ((year % 400) != 0)) ? 29 : 28;
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
                    time.tm_yday += 1;
                    for (time.tm_hour = 0; time.tm_hour <= 23; ++time.tm_hour) {
                        for (time.tm_min = 0; time.tm_min <= 59; ++time.tm_min) {
                            for (time.tm_sec = 0; time.tm_sec <= 60; ++time.tm_sec) {
                                for (time.tm_isdst = -1; time.tm_isdst <= 1; ++time.tm_isdst) {
                                    rc = obelisk_revalidate(&time);
                                    if (rc < 0) {
                                        printf("%04d-%02d-%02dT%02d:%02d:%02d %d %d %d (%d)\n",
                
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
                }
            }
        }

        STATUS();
    }

    EXIT();
}


