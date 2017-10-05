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

#define FSM(_FROM_, _FF_, _FL_, _FY_, _TOKEN_, _TO_, _TF_, _TL_, _TY_) \
    do { \
        obelisk_state_t state = (obelisk_state_t)-1; \
        obelisk_token_t token = (obelisk_token_t)-1; \
        obelisk_buffer_t buffer = { 0 }; \
        int field = _FF_; \
        int length = _FL_; \
        int leap = _FY_; \
        state = OBELISK_STATE_ ## _FROM_; \
        token = OBELISK_TOKEN_ ## _TOKEN_; \
        state = obelisk_parse(state, token, &field, &length, &leap, &buffer); \
        ASSERT(state == OBELISK_STATE_ ## _TO_); \
        ASSERT(field == _TF_); \
        ASSERT(length == _TL_); \
        ASSERT(leap == _TY_); \
    } while (0)

/*
 * States
 * 
 * OBELISK_STATE_START 
 * OBELISK_STATE_WAIT
 * OBELISK_STATE_BEGIN
 * OBELISK_STATE_LEAP
 * OBELISK_STATE_DATA
 * OBELISK_STATE_MARK
 * OBELISK_STATE_END
*/

/*
 * Tokens
 * 
 * OBELISK_TOKEN_ZERO
 * OBELISK_TOKEN_ONE
 * OBELISK_TOKEN_MARKER
 * OBELISK_TOKEN_INVALID
*/

/* Bits/Field
 * [0] 8
 * [1] 9
 * [2] 9
 * [3] 9
 * [4] 9
 * [5] 9
 */

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    {
        TEST();

        FSM(START, -1, -1, -1, ZERO, WAIT, -1, -1, -1);
        FSM(START, -1, -1, -1, ONE, WAIT, -1, -1, -1);
        FSM(START, -1, -1, -1, MARKER, START, -1, -1, -1);
        FSM(START, -1, -1, -1, INVALID, START, -1, -1, -1);

        FSM(START, 5, 0, 0, ZERO, WAIT, 5, 0, 0);
        FSM(START, 5, 0, 0, ONE, WAIT, 5, 0, 0);
        FSM(START, 5, 0, 0, MARKER, START, 5, 0, 0);
        FSM(START, 5, 0, 0, INVALID, START, 5, 0, 0);

        FSM(START, 5, 0, 1, ZERO, WAIT, 5, 0, 1);
        FSM(START, 5, 0, 1, ONE, WAIT, 5, 0, 1);
        FSM(START, 5, 0, 1, MARKER, START, 5, 0, 1);
        FSM(START, 5, 0, 1, INVALID, START, 5, 0, 1);

        STATUS();
    }

    {
        TEST();

        FSM(WAIT, -1, -1, -1, ZERO, WAIT, -1, -1, -1);
        FSM(WAIT, -1, -1, -1, ONE, WAIT, -1, -1, -1);
        FSM(WAIT, -1, -1, -1, MARKER, BEGIN, -1, -1, -1);
        FSM(WAIT, -1, -1, -1, INVALID, START, -1, -1, -1);

        FSM(WAIT, 5, 0, 0, ZERO, WAIT, 5, 0, 0);
        FSM(WAIT, 5, 0, 0, ONE, WAIT, 5, 0, 0);
        FSM(WAIT, 5, 0, 0, MARKER, BEGIN, 5, 0, 0);
        FSM(WAIT, 5, 0, 0, INVALID, START, 5, 0, 0);

        FSM(WAIT, 5, 0, 1, ZERO, WAIT, 5, 0, 1);
        FSM(WAIT, 5, 0, 1, ONE, WAIT, 5, 0, 1);
        FSM(WAIT, 5, 0, 1, MARKER, BEGIN, 5, 0, 1);
        FSM(WAIT, 5, 0, 1, INVALID, START, 5, 0, 1);

        STATUS();
    }

    {
        TEST();

        FSM(BEGIN, -1, -1, -1, ZERO, WAIT, -1, -1, -1);
        FSM(BEGIN, -1, -1, -1, ONE, WAIT, -1, -1, -1);
        FSM(BEGIN, -1, -1, -1, MARKER, LEAP, 0, 8, 0);
        FSM(BEGIN, -1, -1, -1, INVALID, START, -1, -1, -1);

        FSM(BEGIN, 5, 0, 0, ZERO, WAIT, 5, 0, 0);
        FSM(BEGIN, 5, 0, 0, ONE, WAIT, 5, 0, 0);
        FSM(BEGIN, 5, 0, 0, MARKER, LEAP, 0, 8, 0);
        FSM(BEGIN, 5, 0, 0, INVALID, START, 5, 0, 0);

        FSM(BEGIN, 5, 0, 1, ZERO, WAIT, 5, 0, 1);
        FSM(BEGIN, 5, 0, 1, ONE, WAIT, 5, 0, 1);
        FSM(BEGIN, 5, 0, 1, MARKER, LEAP, 0, 8, 0);
        FSM(BEGIN, 5, 0, 1, INVALID, START, 5, 0, 1);

        STATUS();
    }

    {
        TEST();

        FSM(LEAP, 0, 8, 0, ZERO, DATA, 0, 7, 0);
        FSM(LEAP, 0, 8, 0, ONE, DATA, 0, 7, 0);
        FSM(LEAP, 0, 8, 0, MARKER, DATA, 0, 8, 1);
        FSM(LEAP, 0, 8, 0, INVALID, START, 0, 8, 0);

        STATUS();
    }

    {
        TEST();

        FSM(DATA, 0, 8, 0, ZERO, DATA, 0, 7, 0);
        FSM(DATA, 0, 8, 0, ONE, DATA, 0, 7, 0);
        FSM(DATA, 0, 8, 0, MARKER, START, 0, 8, 0);
        FSM(DATA, 0, 8, 0, INVALID, START, 0, 8, 0);

        FSM(DATA, 0, 8, 1, ZERO, DATA, 0, 7, 1);
        FSM(DATA, 0, 8, 1, ONE, DATA, 0, 7, 1);
        FSM(DATA, 0, 8, 1, MARKER, START, 0, 8, 1);
        FSM(DATA, 0, 8, 1, INVALID, START, 0, 8, 1);

        FSM(DATA, 4, 1, 0, ZERO, MARK, 4, 0, 0);
        FSM(DATA, 4, 1, 0, ONE, MARK, 4, 0, 0);
        FSM(DATA, 4, 1, 0, MARKER, START, 4, 1, 0);
        FSM(DATA, 4, 1, 0, INVALID, START, 4, 1, 0);

        FSM(DATA, 4, 1, 1, ZERO, MARK, 4, 0, 1);
        FSM(DATA, 4, 1, 1, ONE, MARK, 4, 0, 1);
        FSM(DATA, 4, 1, 1, MARKER, START, 4, 1, 1);
        FSM(DATA, 4, 1, 1, INVALID, START, 4, 1, 1);

        FSM(DATA, 5, 1, 0, ZERO, END, 5, 0, 0);
        FSM(DATA, 5, 1, 0, ONE, END, 5, 0, 0);
        FSM(DATA, 5, 1, 0, MARKER, START, 5, 1, 0);
        FSM(DATA, 5, 1, 0, INVALID, START, 5, 1, 0);

        FSM(DATA, 5, 1, 1, ZERO, END, 5, 0, 1);
        FSM(DATA, 5, 1, 1, ONE, END, 5, 0, 1);
        FSM(DATA, 5, 1, 1, MARKER, START, 5, 1, 1);
        FSM(DATA, 5, 1, 1, INVALID, START, 5, 1, 1);

        STATUS();
    }

    {
        TEST();

        FSM(MARK, 0, 0, 0, ZERO, START, 0, 0, 0);
        FSM(MARK, 0, 0, 0, ONE, START, 0, 0, 0);
        FSM(MARK, 0, 0, 0, MARKER, DATA, 1, 9, 0);
        FSM(MARK, 0, 0, 0, INVALID, START, 0, 0, 0);

        FSM(MARK, 0, 0, 1, ZERO, START, 0, 0, 1);
        FSM(MARK, 0, 0, 1, ONE, START, 0, 0, 1);
        FSM(MARK, 0, 0, 1, MARKER, DATA, 1, 9, 1);
        FSM(MARK, 0, 0, 1, INVALID, START, 0, 0, 1);

        STATUS();
    }

    {
        TEST();

        FSM(END, 5, 0, 0, ZERO, START, 5, 0, 0);
        FSM(END, 5, 0, 0, ONE, START, 5, 0, 0);
        FSM(END, 5, 0, 0, MARKER, BEGIN, 5, 0, 0);
        FSM(END, 5, 0, 0, INVALID, START, 5, 0, 0);

        FSM(END, 5, 0, 1, ZERO, START, 5, 0, 1);
        FSM(END, 5, 0, 1, ONE, START, 5, 0, 1);
        FSM(END, 5, 0, 1, MARKER, BEGIN, 5, 0, 1);
        FSM(END, 5, 0, 1, INVALID, START, 5, 0, 1);

        STATUS();
    }

    EXIT();
}
