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
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <time.h>

/**
 * These are the values that the GPIO pin may assume.
 */
typedef enum ObeliskLevel {
    OBELISK_LEVEL_ZERO  = 0,    /*   0 dBr */
    OBELISK_LEVEL_ONE   = 1,    /* -17 dBr */
} obelisk_level_t;

/**
 * When we classify a pulse according to its duration, these are the
 * classifications it can assume.
 */
typedef enum ObeliskToken {
    OBELISK_TOKEN_ZERO,     /* 200ms */
    OBELISK_TOKEN_ONE,      /* 500ms */
    OBELISK_TOKEN_MARKER,   /* 800ms */
    OBELISK_TOKEN_INVALID,
} obelisk_token_t;

/**
 * Classify a pulse according to its duration and return a token.
 * @param milliseconds_pulse is the length of the pulse in milliseconds.
 * @return a token classifying the pulse according to its duration.
 */
extern obelisk_token_t obelisk_tokenize(int milliseconds_pulse);

/**
 * When we parse the IRIQ timecode frame using a finite state machine,
 * these are the states the FSM may assume.
 */
typedef enum ObeliskState {
    OBELISK_STATE_START,    /* Expecting final ZERO or ONE. */
    OBELISK_STATE_WAIT,     /* Expecting initial END MARKER. */
    OBELISK_STATE_BEGIN,    /* Expecting BEGIN MARKER. */
    OBELISK_STATE_LEAP,     /* Expecting LEAP MARKER, ZERO, or ONE. */
    OBELISK_STATE_DATA,     /* Expecting ZERO or ONE. */
    OBELISK_STATE_MARK,     /* Expecting intermediate MARKER. */
    OBELISK_STATE_END,      /* Expecting END MARKER. */
} obelisk_state_t;

/**
 * When we parse the IRIQ timecode frame using a finite state machine,
 * these are the actions that the FSM may perform.
 */
typedef enum ObeliskAction {
    OBELISK_ACTION_NONE,    /* Do nothing. */
    OBELISK_ACTION_CLEAR,   /* Empty frame. */
    OBELISK_ACTION_ZERO,    /* Insert 0. */
    OBELISK_ACTION_ONE,     /* Insert 1. */
    OBELISK_ACTION_LEAP,    /* Leap second indicated. */
    OBELISK_ACTION_MARK,    /* Insert MARK. */
    OBELISK_ACTION_FINAL,   /* Complete frame. */
} obelisk_action_t;

/**
 * This type describes the buffer into which the IRIQ timecode frame is
 * stored, one bit at a time, at one bit per second.
 */
typedef uint64_t obelisk_buffer_t;

/**
 * Parse the IRIQ timecode frame one token at a time. Change states as we
 * consume tokens. Pointers to variables into which the finite state machine
 * saves intermediate state are provided by the caller, who does not need
 * to initialize them.
 * @param state is the prior state (initially the START state).
 * @param token is the latest token.
 * @param fieldp points to the number of the field being processed.
 * @param lengthp points to the unconnsumed number of bits in the field.
 * @param leapp points to a variable that stores !0 if a leap second was seen.
 * @param bufferp points to a buffer into which bits are stored.
 * @return the next state.
 */
extern obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * leapp, obelisk_buffer_t * bufferp);

#define _MARKER
#define _UNUSED
#define _FILLER

/**
 * This structure describes an IRIQ timecode frame as used by WWVB.
 * N.B. bit fields in C are not portable! The order of the bit fields
 * are not guaranteed by the standard. Hence, we use this struct just
 * to insure the bit fields are the right length, but we cannot make
 * any assumptions about order. This means you cannot, for example, use
 * a union of an obelisk_frame_t and an obelisk_buffer_t.
 */
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

/**
 * These are the values that the dUT1 sign field in the IRIQ timecode frame
 * may assume.
 */
typedef enum ObeliskSign {
    OBELISK_SIGN_NEGATIVE   = 0x2,  /* 0b010 */
    OBELISK_SIGN_POSITIVE   = 0x5,  /* 0b101 */
} obelisk_sign_t;

/**
 * These are the values that the DST field in the IRIQ timecode frame may
 * assume.
 */
typedef enum ObeliskDst {
    OBELISK_DST_OFF     = 0x0,  /* 0b00 */
    OBELISK_DST_ENDS    = 0x1,  /* 0b01 */
    OBELISK_DST_BEGINS  = 0x2,  /* 0b10 */
    OBELISK_DST_ON      = 0x3,  /* 0b11 */
} obelisk_dst_t;

/**
 * Extract the individual IRIQ timecode fields from the buffer and store
 * them in a frame.
 * @param framep points to the output frame structure.
 * @param buffer is the input buffer.
 */
extern void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer);

/**
 * Validity check the individual fields in the frame structure. This only
 * checks for basic sanity of the binary coded digits.
 * @param framep points to the frame structure.
 * @return >= 0 if the data is valid, <0 otherwise.
 */
extern int obelisk_validate(const obelisk_frame_t * framep);

/**
 * Convert the frame binary coded decimal fields into binary data in POSIX
 * struct tm form.
 * @param timep points to the output tm structure.
 * @param framep points to the input frame structure.
 * @return >= 0 if the conversion was successful, <0 otherwise.
 */
extern int obelisk_decode(struct tm * timep, const obelisk_frame_t * framep);

/**
 * Validity check the individual fields in the POSIX tm structure.
 * @param timep points to the tm structure.
 * @return >= 0 if the data is valid, <0 otherwise.
 */
extern int obelisk_revalidate(const struct tm * timep);

#endif /*  _COM_DIAG_OBELISK_OBELISK_H_ */
