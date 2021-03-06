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
 * These are the values that the GPIO pin may assume. Since the raw
 * GPIO values are fed to the debouncer, and all subsequent decisions
 * are based on its edge detector, these values are mostly used for
 * documentation purposes. Notice though that the SYM-RFT-60 radio
 * receiver inverts the raw modulated signal for us.
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
    OBELISK_TOKEN_FIRST = OBELISK_TOKEN_ZERO,
    OBELISK_TOKEN_LAST = OBELISK_TOKEN_INVALID,
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
    OBELISK_STATE_START,    /* Expecting initial END MARKER.. */
    OBELISK_STATE_WAIT,     /* Expecting initial BEGIN or LEAP MARKER. */
    OBELISK_STATE_SYNC,     /* Expecting initial LEAP MARKER, ZERO, or ONE. */
    OBELISK_STATE_DATA,     /* Expecting ZERO or ONE. */
    OBELISK_STATE_MARK,     /* Expecting separator MARKER. */
    OBELISK_STATE_END,      /* Expecting END MARKER. */
    OBELISK_STATE_BEGIN,    /* Expecting BEGIN MARKER. */
    OBELISK_STATE_LEAP,     /* Expecting LEAP MARKER, ZERO, or ONE. */
    OBELISK_STATE_FIRST = OBELISK_STATE_START,
    OBELISK_STATE_LAST = OBELISK_STATE_LEAP,
} obelisk_state_t;

/**
 * THese are the events the state machine returns to the caller to
 * suggest what it should do based on the most recent state transition.
 */
typedef enum ObeliskEvent {
    OBELISK_EVENT_WAITING,  /* Waiting for frame synchronization. */
    OBELISK_EVENT_NOMINAL,  /* Processing input normally. */
    OBELISK_EVENT_INVALID,  /* Restarting due to invalid data. */
    OBELISK_EVENT_TIME,     /* This is the beginning of the minute. */
    OBELISK_EVENT_FRAME,    /* This is the end of the minute. */
    OBELISK_EVENT_LEAP,     /* A leap second was inserted. */
    OBELISK_EVENT_FIRST = OBELISK_EVENT_WAITING,
    OBELISK_EVENT_LAST = OBELISK_EVENT_LEAP,
} obelisk_event_t;

/**
 * This type describes the buffer into which the IRIQ timecode frame is
 * stored, one bit at a time, at one bit per second.
 */
typedef uint64_t obelisk_buffer_t;

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
typedef struct ObeliskFrame {           	/* TIME       */    /* SPACE    */
    obelisk_buffer_t _FILLER    	:  4;                       /* 63 .. 60 */
    obelisk_buffer_t _MARKER    	:  1;   /* :00 .. :00 */    /* 59 .. 59 */
    obelisk_buffer_t minutes10  	:  3;   /* :01 .. :03 */    /* 58 .. 56 */
    obelisk_buffer_t _UNUSED    	:  1;   /* :04 .. :04 */    /* 55 .. 55 */
    obelisk_buffer_t minutes1   	:  4;   /* :05 .. :08 */    /* 54 .. 51 */
    obelisk_buffer_t _MARKER    	:  1;   /* :09 .. :09 */    /* 50 .. 50 */
    obelisk_buffer_t _UNUSED    	:  2;   /* :10 .. :11 */    /* 49 .. 48 */
    obelisk_buffer_t hours10    	:  2;   /* :12 .. :13 */    /* 47 .. 46 */
    obelisk_buffer_t _UNUSED    	:  1;   /* :14 .. :14 */    /* 45 .. 45 */
    obelisk_buffer_t hours1     	:  4;   /* :15 .. :18 */    /* 44 .. 41 */
    obelisk_buffer_t _MARKER    	:  1;   /* :19 .. :19 */    /* 40 .. 40 */
    obelisk_buffer_t _UNUSED    	:  2;   /* :20 .. :21 */    /* 39 .. 38 */
    obelisk_buffer_t day100     	:  2;   /* :22 .. :23 */    /* 37 .. 36 */
    obelisk_buffer_t _UNUSED    	:  1;   /* :24 .. :24 */    /* 35 .. 35 */
    obelisk_buffer_t day10     	 	:  4;   /* :25 .. :28 */    /* 34 .. 31 */
    obelisk_buffer_t _MARKER    	:  1;   /* :29 .. :29 */    /* 30 .. 30 */
    obelisk_buffer_t day1      	 	:  4;   /* :30 .. :33 */    /* 29 .. 26 */
    obelisk_buffer_t _UNUSED    	:  2;   /* :34 .. :35 */    /* 25 .. 24 */
    obelisk_buffer_t dut1sign   	:  3;   /* :36 .. :38 */    /* 23 .. 21 */	/* dUT1 sign */
    obelisk_buffer_t _MARKER    	:  1;   /* :39 .. :39 */    /* 20 .. 20 */
    obelisk_buffer_t dut1magnitude	:  4;   /* :40 .. :43 */    /* 19 .. 16 */	/* difference with UT1 */
    obelisk_buffer_t _UNUSED    	:  1;   /* :44 .. :44 */    /* 15 .. 15 */
    obelisk_buffer_t year10     	:  4;   /* :45 .. :48 */    /* 14 .. 11 */
    obelisk_buffer_t _MARKER    	:  1;   /* :49 .. :49 */    /* 10 .. 10 */
    obelisk_buffer_t year1      	:  4;   /* :50 .. :53 */    /*  9 ..  6 */
    obelisk_buffer_t _UNUSED    	:  1;   /* :54 .. :54 */    /*  5 ..  5 */
    obelisk_buffer_t lyi        	:  1;   /* :55 .. :55 */    /*  4 ..  4 */	/* leap year indicator */
    obelisk_buffer_t lsw        	:  1;   /* :56 .. :56 */    /*  3 ..  3 */	/* leap second warning (at end of month) */
    obelisk_buffer_t dst        	:  2;   /* :57 .. :58 */    /*  2 ..  1 */	/* daylight saving time satus */
    obelisk_buffer_t _MARKER    	:  1;   /* :59 .. :59 */    /*  0 ..  0 */
} obelisk_frame_t;

#undef _FILLER
#undef _UNUSED
#undef _MARKER

/**
 * Parse the IRIQ timecode frame one token at a time. Change states as we
 * consume tokens. Pointers to variables into which the finite state machine
 * saves intermediate state are provided by the caller, who does not need
 * to initialize them.
 * @param statep points to the variable containing the state.
 * @param token is the latest token.
 * @param fieldp points to the number of the field being processed.
 * @param lengthp points to the unconnsumed number of bits in the field.
 * @param framep points to a frame into which a completed frame is stored.
 * @return the event that tells the caller what, if anything, to do.
 */
extern obelisk_event_t obelisk_parse(obelisk_state_t * statep, obelisk_token_t token, int * fieldp, int * lengthp, obelisk_buffer_t * bufferp, obelisk_frame_t * framep);

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
