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

#define FSM(_FS_, _FF_, _FL_, _FY_, _TOKEN_, _TS_, _TF_, _TL_, _TY_) \
    do { \
        obelisk_state_t state = _FS_; \
        int field = _FF_; \
        int length = _FL_; \
        int leap = _FY_; \
        obelisk_token_t token = _TOKEN_; \
        obelisk_buffer_t buffer = 0; \
        state = obelisk_parse(state, token, &field, &length, &leap, &buffer); \
        ASSERT(state == _TS_); \
        ASSERT(field == _TF_); \
        ASSERT(length == _TL_); \
        ASSERT(leap == _TY_); \
    } while (0)

int main(int argc, char ** argv)
{
    SETLOGMASK();

    diminuto_core_enable();

    /* 8 9 9 9 9 9 */

#if 0
    {
        TEST();

        FSM(OBELISK_STATE_START, -1, -1, -1, OBELISK_TOKEN_ZERO, OBELISK_STATE_START, -1, -1, -1);
        FSM(OBELISK_STATE_START, -1, -1, -1, OBELISK_TOKEN_ONE, OBELISK_STATE_START, -1, -1, -1);
        FSM(OBELISK_STATE_START, -1, -1, -1, OBELISK_TOKEN_MARKER, OBELISK_STATE_BEGIN, -1, -1, -1);
        FSM(OBELISK_STATE_START, -1, -1, -1, OBELISK_TOKEN_INVALID, OBELISK_STATE_START, -1, -1, -1);

        STATUS();
    }

    {
        TEST();

        FSM(OBELISK_STATE_BEGIN, -1, -1, -1, OBELISK_TOKEN_ZERO, OBELISK_STATE_START, -1, -1, -1);
        FSM(OBELISK_STATE_BEGIN, -1, -1, -1, OBELISK_TOKEN_ONE, OBELISK_STATE_START, -1, -1, -1);
        FSM(OBELISK_STATE_BEGIN, -1, -1, -1, OBELISK_TOKEN_MARKER, OBELISK_STATE_LEAP, 0, 8, 0);
        FSM(OBELISK_STATE_BEGIN, -1, -1, -1, OBELISK_TOKEN_INVALID, OBELISK_STATE_START, -1, -1, -1);

        STATUS();
    }

    {
        TEST();

        FSM(OBELISK_STATE_LEAP, 0, 8, 0, OBELISK_TOKEN_ZERO, OBELISK_STATE_START, 0, 8, 0);
        FSM(OBELISK_STATE_LEAP, 0, 8, 0, OBELISK_TOKEN_ONE, OBELISK_STATE_START, 0, 8, 0);
        FSM(OBELISK_STATE_LEAP, 0, 8, 0, OBELISK_TOKEN_MARKER, OBELISK_STATE_LEAP, 0, 8, 0);
        FSM(OBELISK_STATE_LEAP, 0, 8, 0, OBELISK_TOKEN_INVALID, OBELISK_STATE_START, 0, 8, 0);

        STATUS();
    }
#endif

    EXIT();
}


