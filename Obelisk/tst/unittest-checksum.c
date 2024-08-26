/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2022 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "com/diag/hazer/hazer.h"

int main(void)
{
    int rc = 0;
    uint8_t msn = 0;
    uint8_t lsn = 0;
    uint8_t cs = 0;
    uint8_t ck = 0;
    char lsc = '\0';
    char msc = '\0';
    static const char NIB[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

    for (lsn = 0; lsn < 16; ++lsn) {
        for (msn = 0; msn < 16; ++msn) {
            ck = (msn << 4) | lsn;
            hazer_checksum2characters(ck, &msc, &lsc);
            assert(msc == NIB[msn]);
            assert(lsc == NIB[lsn]);
        }
    }

    hazer_checksum_buffer("", 0, &msc, &lsc);
    /* There is no wrong answer here, we just want to make sure it doesn't core dump. */

    hazer_checksum_buffer("$V*TU\r\n", 8, &msc, &lsc);
    assert(msc == '5');
    assert(lsc == '6');

    hazer_checksum_buffer("$VW*TU\r\n", 9, &msc, &lsc);
    assert(msc == '0');
    assert(lsc == '1');

    hazer_checksum_buffer("$VWX*TU\r\n", 10, &msc, &lsc);
    assert(msc == '5');
    assert(lsc == '9');

    hazer_checksum_buffer("$VWXY*TU\r\n", 11, &msc, &lsc);
    assert(msc == '0');
    assert(lsc == '0');

    hazer_checksum_buffer("$VWXYZ*TU\r\n", 12, &msc, &lsc);
    assert(msc == '5');
    assert(lsc == 'A');
}
