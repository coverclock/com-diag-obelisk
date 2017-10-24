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

#define FSM(_FROM_, _FF_, _FL_, _TOKEN_, _TO_, _TF_, _TL_, _EVENT_) \
    do { \
        obelisk_state_t state = (obelisk_state_t)-1; \
        obelisk_token_t token = (obelisk_token_t)-1; \
        obelisk_buffer_t buffer = { 0 }; \
        obelisk_frame_t frame = { 0 }; \
        obelisk_event_t event = (obelisk_event_t)-1; \
        int field = _FF_; \
        int length = _FL_; \
        state = OBELISK_STATE_ ## _FROM_; \
        token = OBELISK_TOKEN_ ## _TOKEN_; \
        event = obelisk_parse(&state, token, &field, &length, &buffer, &frame); \
        EXPECT(state == OBELISK_STATE_ ## _TO_); \
        EXPECT(field == _TF_); \
        EXPECT(length == _TL_); \
        EXPECT(event == OBELISK_EVENT_ ## _EVENT_); \
    } while (0)

/*
 * States: START WAIT BEGIN LEAP DATA MARK END
 */

/*
 * Tokens: ZERO ONE MARKER INVALID
 */

/*
 * Statuses: NOMINAL INVALID TIME FRAME LEAP
 */

/*
 * Bits/Field: [0] 8 [1] 9 [2] 9 [3] 9 [4] 9 [5] 9
 */

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        TEST();

        FSM(START, -1, -1, ZERO, START, -1, -1, WAITING);
        FSM(START, -1, -1, ONE, START, -1, -1, WAITING);
        FSM(START, -1, -1, MARKER, WAIT, -1, -1, WAITING);
        FSM(START, -1, -1, INVALID, START, -1, -1, INVALID);

        FSM(START, 5, 0, ZERO, START, 5, 0, WAITING);
        FSM(START, 5, 0, ONE, START, 5, 0, WAITING);
        FSM(START, 5, 0, MARKER, WAIT, 5, 0, WAITING);
        FSM(START, 5, 0, INVALID, START, 5, 0, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(WAIT, -1, -1, ZERO, START, -1, -1, WAITING);
        FSM(WAIT, -1, -1, ONE, START, -1, -1, WAITING);
        FSM(WAIT, -1, -1, MARKER, SYNC, 0, 8, WAITING);
        FSM(WAIT, -1, -1, INVALID, START, -1, -1, INVALID);

        FSM(WAIT, 5, 0, ZERO, START, 5, 0, WAITING);
        FSM(WAIT, 5, 0, ONE, START, 5, 0, WAITING);
        FSM(WAIT, 5, 0, MARKER, SYNC, 0, 8, WAITING);
        FSM(WAIT, 5, 0, INVALID, START, 5, 0, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(SYNC, 0, 8, ZERO, DATA, 0, 7, NOMINAL);
        FSM(SYNC, 0, 8, ONE, DATA, 0, 7, NOMINAL);
        FSM(SYNC, 0, 8, MARKER, DATA, 0, 8, NOMINAL);
        FSM(SYNC, 0, 8, INVALID, START, 0, 8, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(DATA, 0, 8, ZERO, DATA, 0, 7, NOMINAL);
        FSM(DATA, 0, 8, ONE, DATA, 0, 7, NOMINAL);
        FSM(DATA, 0, 8, MARKER, START, 0, 8, INVALID);
        FSM(DATA, 0, 8, INVALID, START, 0, 8, INVALID);

        FSM(DATA, 4, 1, ZERO, MARK, 4, 0, NOMINAL);
        FSM(DATA, 4, 1, ONE, MARK, 4, 0, NOMINAL);
        FSM(DATA, 4, 1, MARKER, START, 4, 1, INVALID);
        FSM(DATA, 4, 1, INVALID, START, 4, 1, INVALID);

        FSM(DATA, 5, 1, ZERO, END, 5, 0, NOMINAL);
        FSM(DATA, 5, 1, ONE, END, 5, 0, NOMINAL);
        FSM(DATA, 5, 1, MARKER, START, 5, 1, INVALID);
        FSM(DATA, 5, 1, INVALID, START, 5, 1, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(MARK, 0, 0, ZERO, START, 0, 0, INVALID);
        FSM(MARK, 0, 0, ONE, START, 0, 0, INVALID);
        FSM(MARK, 0, 0, MARKER, DATA, 1, 9, NOMINAL);
        FSM(MARK, 0, 0, INVALID, START, 0, 0, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(END, 5, 0, ZERO, START, 5, 0, INVALID);
        FSM(END, 5, 0, ONE, START, 5, 0, INVALID);
        FSM(END, 5, 0, MARKER, BEGIN, 5, 0, FRAME);
        FSM(END, 5, 0, INVALID, START, 5, 0, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(BEGIN, 5, 0, ZERO, START, 5, 0, INVALID);
        FSM(BEGIN, 5, 0, ONE, START, 5, 0, INVALID);
        FSM(BEGIN, 5, 0, MARKER, LEAP, 0, 8, TIME);
        FSM(BEGIN, 5, 0, INVALID, START, 5, 0, INVALID);

        STATUS();
    }

    {
        TEST();

        FSM(LEAP, 0, 8, ZERO, DATA, 0, 7, NOMINAL);
        FSM(LEAP, 0, 8, ONE, DATA, 0, 7, NOMINAL);
        FSM(LEAP, 0, 8, MARKER, DATA, 0, 8, LEAP);
        FSM(LEAP, 0, 8, INVALID, START, 0, 8, INVALID);

        STATUS();
    }

    EXIT();
}
