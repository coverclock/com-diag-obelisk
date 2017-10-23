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

#include "com/diag/obelisk/databases.h"

static obelisk_status_t parse(obelisk_frame_t * framep, const char * sentence)
{
    obelisk_status_t status = OBELISK_STATUS_INVALID;
    obelisk_state_t state = OBELISK_STATE_START;
    obelisk_state_t state_prime;
    obelisk_token_t token;
    obelisk_buffer_t buffer;
    int field;
    int length;
    const char * ss;

    for (ss = sentence; *ss != '\0'; ++ss) {
        switch (*ss) {
        case 'M': token = OBELISK_TOKEN_MARKER; break;
        case '0': token = OBELISK_TOKEN_ZERO;   break;
        case '1': token = OBELISK_TOKEN_ONE;    break;
        case '?': token = OBELISK_TOKEN_INVALID; break;
        default:  token = OBELISK_TOKEN_ZERO;   break;
        }
        state_prime = state;
        status = obelisk_parse(&state, token, &field, &length, &buffer, framep);
        fprintf(stderr, "parse %s %s %s %s\n", STATE[state], TOKEN[token], STATE[state_prime], STATUS[status]);
    }

    return status;
}

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MM01100000M000000111M000000110M011000010M001100000M100001000M");
        EXPECT(status == OBELISK_STATUS_FRAME);
        EXPECT(frame.minutes10 == 3);
        EXPECT(frame.minutes1 == 0);
        EXPECT(frame.hours10 == 0);
        EXPECT(frame.hours1 == 7);
        EXPECT(frame.day100 == 0);
        EXPECT(frame.day10 == 6);
        EXPECT(frame.day1 == 6);
        EXPECT(frame.dutonesign == 2);
        EXPECT(frame.dutone1 == 3);
        EXPECT(frame.year10 == 0);
        EXPECT(frame.year1 == 8);
        EXPECT(frame.lyi == 1);
        EXPECT(frame.lsw == 0);
        EXPECT(frame.dst == 0);

        STATUS();
    }

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MM01100000M000000111M000000110M011000010M001100000M100001000MM");
        EXPECT(status == OBELISK_STATUS_TIME);
        EXPECT(frame.minutes10 == 3);
        EXPECT(frame.minutes1 == 0);
        EXPECT(frame.hours10 == 0);
        EXPECT(frame.hours1 == 7);
        EXPECT(frame.day100 == 0);
        EXPECT(frame.day10 == 6);
        EXPECT(frame.day1 == 6);
        EXPECT(frame.dutonesign == 2);
        EXPECT(frame.dutone1 == 3);
        EXPECT(frame.year10 == 0);
        EXPECT(frame.year1 == 8);
        EXPECT(frame.lyi == 1);
        EXPECT(frame.lsw == 0);
        EXPECT(frame.dst == 0);

        STATUS();
    }

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MM01100000M000000111M000000110M011000010M001100000M100001000MMM");
        EXPECT(status == OBELISK_STATUS_LEAP);
        EXPECT(frame.minutes10 == 3);
        EXPECT(frame.minutes1 == 0);
        EXPECT(frame.hours10 == 0);
        EXPECT(frame.hours1 == 7);
        EXPECT(frame.day100 == 0);
        EXPECT(frame.day10 == 6);
        EXPECT(frame.day1 == 6);
        EXPECT(frame.dutonesign == 2);
        EXPECT(frame.dutone1 == 3);
        EXPECT(frame.year10 == 0);
        EXPECT(frame.year1 == 8);
        EXPECT(frame.lyi == 1);
        EXPECT(frame.lsw == 0);
        EXPECT(frame.dst == 0);

        STATUS();
    }

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MM01100000M000000111M000000110M011000010M001100000M100001000MMMM");
        EXPECT(status == OBELISK_STATUS_INVALID);

        STATUS();
    }

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MMM01100000M000000111M000000110M011000010M001100000M100001000M");
        EXPECT(status == OBELISK_STATUS_FRAME);
        EXPECT(frame.minutes10 == 3);
        EXPECT(frame.minutes1 == 0);
        EXPECT(frame.hours10 == 0);
        EXPECT(frame.hours1 == 7);
        EXPECT(frame.day100 == 0);
        EXPECT(frame.day10 == 6);
        EXPECT(frame.day1 == 6);
        EXPECT(frame.dutonesign == 2);
        EXPECT(frame.dutone1 == 3);
        EXPECT(frame.year10 == 0);
        EXPECT(frame.year1 == 8);
        EXPECT(frame.lyi == 1);
        EXPECT(frame.lsw == 0);
        EXPECT(frame.dst == 0);

        STATUS();
    }

    {
        obelisk_frame_t frame;
        obelisk_status_t status;

        TEST();

        status = parse(&frame, "0MMMM01100000M000000111M000000110M011000010M001100000M100001000M");
        EXPECT(status == OBELISK_STATUS_WAITING);

        STATUS();
    }

    EXIT();
}

