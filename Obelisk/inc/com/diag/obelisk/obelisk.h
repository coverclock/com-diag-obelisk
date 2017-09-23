/* vim: set ts=4 expandtab shiftwidth=4: */
#ifndef _COM_DIAG_OBELISK_OBELISK_H_
#define _COM_DIAG_OBELISK_OBELISK_H_

/**
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock<BR>
 * mailto:coverclock@diag.com<BR>
 * http://github.com/coverclock/com-diag-obelisk<BR>
 */

#include <stdint.h>
#include "com/diag/diminuto/diminuto_cue.h"

typedef enum ObeliskLevel {
    LEVEL_ZERO  = 0,    /*   0 dBr */
    LEVEL_ONE   = 1,    /* -17 dBr */
} obelisk_level_t;

typedef enum ObeliskToken {
    TOKEN_ZERO      = 0,    /* 200ms */
    TOKEN_ONE       = 1,    /* 500ms */
    TOKEN_MARKER    = 2,    /* 800ms */
    TOKEN_INVALID   = 3,
    TOKEN_PENDING   = 4,
} obelisk_token_t;

typedef struct ObeliskRange {
    int minimum;
    int maximum;
} obelisk_range_t;

typedef enum ObeliskState {
    STATE_WAIT      = 0,
    STATE_SYNC      = 1,
    STATE_START     = 2,
    STATE_DATA      = 3,
    STATE_MARK      = 4,
    STATE_END       = 5,
} obelisk_state_t;

typedef uint64_t obelisk_buffer_t;

typedef struct ObeliskRecord {  /* TIME */          /* SPACE */
                                /* :00 MARKER */
    unsigned minutes    :  8;   /* :01 .. :08 */    /* 00 .. 07 */
                                /* :09 MARKER */
    unsigned unused0    :  2;   /* :10 .. :11 */    /* 08 .. 09 */
    unsigned hours      :  7;   /* :12 .. :18 */    /* 10 .. 16 */
                                /* :19 MARKER */
    unsigned unused1    :  2;   /* :20 .. :21 */    /* 17 .. 18 */
    unsigned day        : 11;   /* :22 .. :28 */    /* 19 .. 29 */
                                /* :29 MARKER */
                                /* :30 .. :33 */
    unsigned unused2    :  2;   /* :34 .. :35 */    /* 30 .. 31 */
    unsigned sign       :  3;   /* :36 .. :38 */    /* 32 .. 34 */
                                /* :39 MARKER */
    unsigned dut1       :  4;   /* :40 .. :43 */    /* 35 .. 38 */
    unsigned unused3    :  1;   /* :44 */           /* 39 .. 39 */
    unsigned year       :  8;   /* :45 .. :48 */    /* 40 .. 47 */
                                /* :49 MARKER */
                                /* :50 .. :53 */
    unsigned unused4    :  1;   /* :54 */           /* 48 .. 48 */
    unsigned lyi        :  1;   /* :55 */           /* 49 .. 49 */
    unsigned lsw        :  1;   /* :56 */           /* 50 .. 50 */
    unsigned dst        :  2;   /* :57 .. :58 */    /* 51 .. 52 */
                                /* :60 MARKER */
    unsigned filler     : 11;                       /* 53 .. 63 */
} obelisk_frame_t;

extern int obelisk_measure(diminuto_cue_state_t * cuep, int milliseconds_pulse, int milliseconds_cycle);

extern obelisk_token_t obelisk_tokenize(int milliseconds_pulse);

extern obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * bitp, int * leapp, obelisk_buffer_t * bufferp);

#endif /*  _COM_DIAG_OBELISK_OBELISK_H_ */
