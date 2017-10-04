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
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/obelisk/obelisk.h"
#include "obelisk.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        obelisk_frame_t before = { 0 };
        obelisk_frame_t after = { 0 };
        obelisk_buffer_t buffer = 0;
        static const int OFFSET = 60 - 1;

        TEST();

	    before.minutes10 = 0x7;
        before.minutes1 = 0xf;
        before.hours10 = 0x3;
        before.hours1 = 0xf;
        before.day100 = 0x3;
        before.day10 = 0xf;
        before.day1 = 0xf;
        before.dutonesign = 0x7;
        before.dutone1 = 0xf;
        before.year10 = 0xf;
        before.year1 = 0xf;
        before.lyi = 0x1;
        before.lsw = 0x1;
        before.dst = 0x3;

        fprintf(stderr, "BEFORE:\n");
        diminuto_dump(stderr, &before, sizeof(before));

        /*
         * Reference:   Wikipedia, "WWVB", https://en.wikipedia.org/wiki/WWVB,
         *              2017-05-02
         */

        buffer =    ((obelisk_buffer_t)before.minutes10     << (OFFSET -  3)) |
                    ((obelisk_buffer_t)before.minutes1      << (OFFSET -  8)) |
                    ((obelisk_buffer_t)before.hours10       << (OFFSET - 13)) |
                    ((obelisk_buffer_t)before.hours1        << (OFFSET - 18)) |
                    ((obelisk_buffer_t)before.day100        << (OFFSET - 23)) |
                    ((obelisk_buffer_t)before.day10         << (OFFSET - 28)) |
                    ((obelisk_buffer_t)before.day1          << (OFFSET - 33)) |
                    ((obelisk_buffer_t)before.dutonesign    << (OFFSET - 38)) |
                    ((obelisk_buffer_t)before.dutone1       << (OFFSET - 43)) |
                    ((obelisk_buffer_t)before.year10        << (OFFSET - 48)) |
                    ((obelisk_buffer_t)before.year1         << (OFFSET - 53)) |
                    ((obelisk_buffer_t)before.lyi           << (OFFSET - 55)) |
                    ((obelisk_buffer_t)before.lsw           << (OFFSET - 56)) |
                    ((obelisk_buffer_t)before.dst           << (OFFSET - 58));

        fprintf(stderr, "BUFFER:\n");
        diminuto_dump(stderr, &buffer, sizeof(buffer));

        obelisk_extract(&after, buffer);

        fprintf(stderr, "AFTER:\n");
        diminuto_dump(stderr, &after, sizeof(after));

	    EXPECT(before.minutes10 == after.minutes10);
        EXPECT(before.minutes1 == after.minutes1);
        EXPECT(before.hours10 == after.hours10);
        EXPECT(before.hours1 == after.hours1);
        EXPECT(before.day100 == after.day100);
        EXPECT(before.day10 == after.day10);
        EXPECT(before.day1 == after.day1);
        EXPECT(before.dutonesign == after.dutonesign);
        EXPECT(before.dutone1 == after.dutone1);
        EXPECT(before.year10 == after.year10);
        EXPECT(before.year1 == after.year1);
        EXPECT(before.lyi == after.lyi);
        EXPECT(before.lsw == after.lsw);
        EXPECT(before.dst == after.dst);

        STATUS();
    }

    {
        obelisk_frame_t before = { 0 };
        obelisk_frame_t after = { 0 };
        obelisk_buffer_t buffer = 0;
        static const int OFFSET = 60 - 1;

        TEST();

	    before.minutes10 = 0x4;
        before.minutes1 = 0x8;
        before.hours10 = 0x2;
        before.hours1 = 0x9;
        before.day100 = 0x3;
        before.day10 = 0xa;
        before.day1 = 0xb;
        before.dutonesign = 0x5;
        before.dutone1 = 0xc;
        before.year10 = 0xd;
        before.year1 = 0xe;
        before.lyi = 0x1;
        before.lsw = 0x1;
        before.dst = 0x3;

        fprintf(stderr, "BEFORE:\n");
        diminuto_dump(stderr, &before, sizeof(before));

        /*
         * Reference:   Wikipedia, "WWVB", https://en.wikipedia.org/wiki/WWVB,
         *              2017-05-02
         */

        buffer =    ((obelisk_buffer_t)before.minutes10     << (OFFSET -  3)) |
                    ((obelisk_buffer_t)before.minutes1      << (OFFSET -  8)) |
                    ((obelisk_buffer_t)before.hours10       << (OFFSET - 13)) |
                    ((obelisk_buffer_t)before.hours1        << (OFFSET - 18)) |
                    ((obelisk_buffer_t)before.day100        << (OFFSET - 23)) |
                    ((obelisk_buffer_t)before.day10         << (OFFSET - 28)) |
                    ((obelisk_buffer_t)before.day1          << (OFFSET - 33)) |
                    ((obelisk_buffer_t)before.dutonesign    << (OFFSET - 38)) |
                    ((obelisk_buffer_t)before.dutone1       << (OFFSET - 43)) |
                    ((obelisk_buffer_t)before.year10        << (OFFSET - 48)) |
                    ((obelisk_buffer_t)before.year1         << (OFFSET - 53)) |
                    ((obelisk_buffer_t)before.lyi           << (OFFSET - 55)) |
                    ((obelisk_buffer_t)before.lsw           << (OFFSET - 56)) |
                    ((obelisk_buffer_t)before.dst           << (OFFSET - 58));

        fprintf(stderr, "BUFFER:\n");
        diminuto_dump(stderr, &buffer, sizeof(buffer));

        obelisk_extract(&after, buffer);

        fprintf(stderr, "AFTER:\n");
        diminuto_dump(stderr, &after, sizeof(after));

	    EXPECT(before.minutes10 == after.minutes10);
        EXPECT(before.minutes1 == after.minutes1);
        EXPECT(before.hours10 == after.hours10);
        EXPECT(before.hours1 == after.hours1);
        EXPECT(before.day100 == after.day100);
        EXPECT(before.day10 == after.day10);
        EXPECT(before.day1 == after.day1);
        EXPECT(before.dutonesign == after.dutonesign);
        EXPECT(before.dutone1 == after.dutone1);
        EXPECT(before.year10 == after.year10);
        EXPECT(before.year1 == after.year1);
        EXPECT(before.lyi == after.lyi);
        EXPECT(before.lsw == after.lsw);
        EXPECT(before.dst == after.dst);

        STATUS();
    }

    EXIT();
}
