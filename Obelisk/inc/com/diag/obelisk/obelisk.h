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
    OBELISK_LEVEL_ZERO  = 0,    /*   0 dBr */
    OBELISK_LEVEL_ONE   = 1,    /* -17 dBr */
} obelisk_level_t;

extern int obelisk_measure(diminuto_cue_state_t * cuep, int milliseconds_pulse, int milliseconds_cycle);

typedef struct ObeliskRange {
    int minimum;
    int maximum;
} obelisk_range_t;

typedef enum ObeliskToken {
    OBELISK_TOKEN_ZERO      = 0,    /* 200ms */
    OBELISK_TOKEN_ONE       = 1,    /* 500ms */
    OBELISK_TOKEN_MARKER    = 2,    /* 800ms */
    OBELISK_TOKEN_INVALID   = 3,
    OBELISK_TOKEN_PENDING   = 4,
} obelisk_token_t;

extern obelisk_token_t obelisk_tokenize(int milliseconds_pulse);

typedef enum ObeliskState {
    OBELISK_STATE_WAIT  = 0,
    OBELISK_STATE_SYNC  = 1,
    OBELISK_STATE_START = 2,
    OBELISK_STATE_DATA  = 3,
    OBELISK_STATE_MARK  = 4,
    OBELISK_STATE_END   = 5,
} obelisk_state_t;

typedef uint64_t obelisk_buffer_t;

extern obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * bitp, int * leapp, obelisk_buffer_t * bufferp);

typedef struct ObeliskFrame {   /* TIME */          /* SPACE */
                                /* :00 MARKER */
    unsigned minutes10  :  3;   /* :01 .. :03 */    /* 63 .. 61 */
    unsigned            :  1;   /* :04 .. :04 */    /* 60 .. 60 */
    unsigned minutes1   :  4;   /* :05 .. :08 */    /* 59 .. 56 */
                                /* :09 MARKER */
    unsigned            :  2;   /* :10 .. :11 */    /* 55 .. 54 */
    unsigned hours10    :  2;   /* :12 .. :13 */    /* 53 .. 52 */
    unsigned            :  1;   /* :14 .. :14 */    /* 51 .. 51 */
    unsigned hours1     :  4;   /* :15 .. :18 */    /* 50 .. 47 */
                                /* :19 MARKER */
    unsigned            :  2;   /* :20 .. :21 */    /* 46 .. 45 */
    unsigned day100     :  2;   /* :22 .. :23 */    /* 44 .. 43 */
    unsigned            :  1;   /* :24 .. :24 */    /* 42 .. 42 */
    unsigned day10      :  4;   /* :25 .. :28 */    /* 41 .. 38 */
                                /* :29 MARKER */
    unsigned day1       :  4;   /* :30 .. :33 */    /* 37 .. 34 */
    unsigned            :  2;   /* :34 .. :35 */    /* 33 .. 32 */
    unsigned sign       :  3;   /* :36 .. :38 */    /* 31 .. 29 */
                                /* :39 MARKER */
    unsigned dut1       :  4;   /* :40 .. :43 */    /* 28 .. 25 */
    unsigned            :  1;   /* :44 .. :44 */    /* 24 .. 24 */
    unsigned year10     :  4;   /* :45 .. :48 */    /* 23 .. 20 */
                                /* :49 MARKER */
    unsigned year1      :  4;   /* :50 .. :53 */    /* 19 .. 16 */
    unsigned            :  1;   /* :54 */           /* 15 .. 15 */
    unsigned lyi        :  1;   /* :55 */           /* 14 .. 14 */
    unsigned lsw        :  1;   /* :56 */           /* 13 .. 13 */
    unsigned dst        :  2;   /* :57 .. :58 */    /* 12 .. 11 */
                                /* :60 MARKER */
    unsigned filler     : 11;                       /* 10 .. 00 */
} obelisk_frame_t;

extern void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer);

typedef enum ObeliskSign {
    OBELISK_SIGN_NEGATIVE   = 0x2,  /* 0b010 */
    OBELISK_SIGN_POSITIVE   = 0x3,  /* 0b101 */
} obelisk_sign_t;

typedef enum ObeliskDst {
    OBELISK_DST_OFF     = 0x0,  /* 0b00 */
    OBELISK_DST_ENDS    = 0x1,  /* 0b01 */
    OBELISK_DST_BEGINS  = 0x2,  /* 0b10 */
    OBELISK_DST_ON      = 0x3,  /* 0b11 */
} obelisk_dst_t;

#endif /*  _COM_DIAG_OBELISK_OBELISK_H_ */
