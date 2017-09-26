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

extern obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * leapp, obelisk_buffer_t * bufferp);

#define _MARKER
#define _UNUSED
#define _FILLER

typedef struct ObeliskFrame {           /* TIME       */    /* SPACE    */
    obelisk_buffer_t _FILLER    :  4;                       /* 63 .. 60 */
    obelisk_buffer_t _MARKER    :  1;   /* :00 .. :00 */    /* 59 .. 59 */
    obelisk_buffer_t minutes10  :  3;   /* :01 .. :03 */    /* 58 .. 56 */
    obelisk_buffer_t _UNUSED    :  1;   /* :04 .. :04 */    /* 55 .. 55 */
    obelisk_buffer_t minutes1   :  4;   /* :05 .. :08 */    /* 54 .. 51 */
    obelisk_buffer_t _MARKER    :  1;   /* :09 .. :09 */    /* 50 .. 50 */
    obelisk_buffer_t _UNUSED    :  2;   /* :10 .. :11 */    /* 49 .. 48 */
    obelisk_buffer_t hours10    :  2;   /* :12 .. :13 */    /* 47 .. 46 */
    obelisk_buffer_t _UNUSED    :  1;   /* :14 .. :14 */    /* 45 .. 45 */
    obelisk_buffer_t hours1     :  4;   /* :15 .. :18 */    /* 44 .. 41 */
    obelisk_buffer_t _MARKER    :  1;   /* :19 .. :19 */    /* 40 .. 40 */
    obelisk_buffer_t _UNUSED    :  2;   /* :20 .. :21 */    /* 39 .. 38 */
    obelisk_buffer_t day100     :  2;   /* :22 .. :23 */    /* 37 .. 36 */
    obelisk_buffer_t _UNUSED    :  1;   /* :24 .. :24 */    /* 35 .. 35 */
    obelisk_buffer_t day10      :  4;   /* :25 .. :28 */    /* 34 .. 31 */
    obelisk_buffer_t _MARKER    :  1;   /* :29 .. :29 */    /* 30 .. 30 */
    obelisk_buffer_t day1       :  4;   /* :30 .. :33 */    /* 29 .. 26 */
    obelisk_buffer_t _UNUSED    :  2;   /* :34 .. :35 */    /* 25 .. 24 */
    obelisk_buffer_t dutonesign :  3;   /* :36 .. :38 */    /* 23 .. 21 */
    obelisk_buffer_t _MARKER    :  1;   /* :39 .. :39 */    /* 20 .. 20 */
    obelisk_buffer_t dutone1    :  4;   /* :40 .. :43 */    /* 19 .. 16 */
    obelisk_buffer_t _UNUSED    :  1;   /* :44 .. :44 */    /* 15 .. 15 */
    obelisk_buffer_t year10     :  4;   /* :45 .. :48 */    /* 14 .. 11 */
    obelisk_buffer_t _MARKER    :  1;   /* :49 .. :49 */    /* 10 .. 10 */
    obelisk_buffer_t year1      :  4;   /* :50 .. :53 */    /*  9 ..  6 */
    obelisk_buffer_t _UNUSED    :  1;   /* :54 .. :54 */    /*  5 ..  5 */
    obelisk_buffer_t lyi        :  1;   /* :55 .. :55 */    /*  4 ..  4 */
    obelisk_buffer_t lsw        :  1;   /* :56 .. :56 */    /*  3 ..  3 */
    obelisk_buffer_t dst        :  2;   /* :57 .. :58 */    /*  2 ..  1 */
    obelisk_buffer_t _MARKER    :  1;   /* :59 .. :59 */    /*  0 ..  0 */
} obelisk_frame_t;

#undef _FILLER
#undef _UNUSED
#undef _MARKER

typedef enum ObeliskOffset {
     OBELISK_OFFSET_MINUTES10   = 58,
     OBELISK_OFFSET_MINUTES1    = 54,
     OBELISK_OFFSET_HOURS10     = 47,
     OBELISK_OFFSET_HOURS1      = 44,
     OBELISK_OFFSET_DAY100      = 37,
     OBELISK_OFFSET_DAY10       = 34,
     OBELISK_OFFSET_DAY1        = 29,
     OBELISK_OFFSET_DUTONESIGN  = 23,
     OBELISK_OFFSET_DUTONE1     = 19,
     OBELISK_OFFSET_YEAR10      = 14,
     OBELISK_OFFSET_YEAR1       = 5,
     OBELISK_OFFSET_LYI         = 4,
     OBELISK_OFFSET_LSW         = 3,
     OBELISK_OFFSET_DST         = 2,
} obelisk_offset_t;

typedef enum ObeliskMask {
    OBELISK_MASK_MINUTES10    = 0x7,
    OBELISK_MASK_MINUTES1     = 0xf,
    OBELISK_MASK_HOURS10      = 0x3,
    OBELISK_MASK_HOURS1       = 0xf,
    OBELISK_MASK_DAY100       = 0x3,
    OBELISK_MASK_DAY10        = 0xf,
    OBELISK_MASK_DAY1         = 0xf,
    OBELISK_MASK_DUTONESIGN   = 0x7,
    OBELISK_MASK_DUTONE1      = 0xf,
    OBELISK_MASK_YEAR10       = 0xf,
    OBELISK_MASK_YEAR1        = 0xf,
    OBELISK_MASK_LYI          = 0x1,
    OBELISK_MASK_LSW          = 0x1,
    OBELISK_MASK_DST          = 0x3,
} obelisk_mask_t;

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

#define OBELISK_EXTRACT(_BUFFER_, _FIELD_) ((_BUFFER_ >> OBELISK_OFFSET_ ## _FIELD_) & OBELISK_MASK_ ## _FIELD_)

extern void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer);

#endif /*  _COM_DIAG_OBELISK_OBELISK_H_ */
