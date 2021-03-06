/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
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

/*
 * Runs about 4 hours 10 minutes on the Pi 3,
 * about 25 minutes on an Intel i7.
 */

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        int rc = -1;
        obelisk_frame_t frame = { 0 };
        struct tm time = { 0 };
        long long count = -1;
        static const long long EXPECTED = (60LL * 24LL * (365LL + 366LL) * 2LL * 10LL * 100LL * 2LL * 4LL);
        int days = -1;

        TEST();

        count = 0;

        for (int minutes = 0; minutes <= 59; ++minutes) {
            frame.minutes10 = minutes / 10;
            frame.minutes1 = minutes % 10;
            for (int hours = 0; hours <= 23; ++hours) {
                frame.hours10 = hours / 10;
                frame.hours1 = hours % 10;
                for (int lyi = 1; lyi >= 0; --lyi) {
                    frame.lyi = lyi;
                    days = lyi ? 366 : 365;
                    for (int day = 1; day <= days; ++day) {
                        frame.day100 = day / 100;
                        frame.day10 = (day % 100) / 10;
                        frame.day1 = (day % 100) % 10;
                        for (int dut1sign = 2; dut1sign <= 5; dut1sign += 3) {
                            frame.dut1sign = dut1sign;
                            for (int dut1magnitude = 0; dut1magnitude <= 9; ++dut1magnitude) {
                                frame.dut1magnitude = dut1magnitude;
                                for (int year = 0; year <= 99; ++year) {
                                    frame.year10 = year / 10;
                                    frame.year1 = year % 10;
                                    for (int lsw = 0; lsw <= 1; ++lsw) {
                                        frame.lsw = lsw;
                                        for (int dst = 0; dst <= 3; ++dst) {
                                            frame.dst = dst;
                                            rc = obelisk_validate(&frame);
                                            if (rc < 0) {
                                                CHECKPOINT("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                                                    rc,
                                                    frame.minutes10, frame.minutes1,
                                                    frame.hours10, frame.hours1,
                                                    frame.day100, frame.day10, frame.day1,
                                                    frame.dut1sign, frame.dut1magnitude,
                                                    frame.year10, frame.year1,
                                                    frame.lyi, frame.lsw, frame.dst);
                                            }
                                            ASSERT(rc >= 0);
                                            rc = obelisk_decode(&time, &frame);
                                            if (rc < 0) {
                                                CHECKPOINT("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                                                    rc,
                                                    frame.minutes10, frame.minutes1,
                                                    frame.hours10, frame.hours1,
                                                    frame.day100, frame.day10, frame.day1,
                                                    frame.dut1sign, frame.dut1magnitude,
                                                    frame.year10, frame.year1,
                                                    frame.lyi, frame.lsw, frame.dst);
                                            }
                                            ASSERT(rc >= 0);
                                            rc = obelisk_revalidate(&time);
                                            if (rc < 0) {
                                                CHECKPOINT("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                                                    rc,
                                                    frame.minutes10, frame.minutes1,
                                                    frame.hours10, frame.hours1,
                                                    frame.day100, frame.day10, frame.day1,
                                                    frame.dut1sign, frame.dut1magnitude,
                                                    frame.year10, frame.year1,
                                                    frame.lyi, frame.lsw, frame.dst);
                                            }
                                            ASSERT(rc >= 0);
                                            if ((count % 10000000) == 0) {
                                                CHECKPOINT("%lld %02d/%03dT%02d:%02d\n", count, year, day, hours, minutes);
                                            }
                                            ++count;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        CHECKPOINT("count %lld %lld\n", count, EXPECTED);
        ASSERT(count == EXPECTED);
        // computed  16,842,240,000
        // measured  16,842,240,000

        STATUS();
    }

    /*
     * TODO: add some failing test cases.
     */

    EXIT();
}
