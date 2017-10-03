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
#include <stdio.h>
#include <errno.h>

int main(int argc, char ** argv)
{
    obelisk_token_t token = OBELISK_TOKEN_INVALID;

    SETLOGMASK();

    diminuto_core_enable();

    {
        TEST();

        for (int milliseconds = -1; milliseconds <= 1001; ++milliseconds) {

            token = obelisk_tokenize(milliseconds);

            ASSERT(
                ((100 <= milliseconds) && (milliseconds <= 300) && (token == OBELISK_TOKEN_ZERO)) ||
                ((400 <= milliseconds) && (milliseconds <= 600) && (token == OBELISK_TOKEN_ONE)) ||
                ((700 <= milliseconds) && (milliseconds <= 900) && (token == OBELISK_TOKEN_MARKER)) ||
                (token == OBELISK_TOKEN_INVALID)
            );

        }

        STATUS();

    }

    EXIT();
}

