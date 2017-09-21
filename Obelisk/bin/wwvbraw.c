/* vi: set ts=4 expandtab shiftwidth=4: */

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock<BR>
 * mailto:coverclock@diag.com<BR>
 * http://github.com/coverclock/com-diag-obelisk<BR>
 */

#include <stdio.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_types.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_dump.h"

static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_RESET = 2;
static const int HERTZ_POLL = 100;
static const int MILLISECONDS_POLL = 1000 / 100;

typedef enum ObeliskLevel {
    LEVEL_ZERO  = 0,    /*   0 dBr */
    LEVEL_ONE   = 1,    /* -17 dBr */
} obelisk_level_t;

typedef enum ObeliskToken {
    TOKEN_ZERO      = 0,    /* 200ms */
    TOKEN_ONE       = 1,    /* 500ms */
    TOKEN_MARKER    = 2,    /* 800ms */
    TOKEN_INVALID   = 3,
    TOKEN_MISSING   = 4,
} obelisk_token_t;

typedef struct ObeliskRange {
    int symbol;
    int minimum;
    int maximum;
} obelisk_range_t;

static const obelisk_range_t RANGE[] = {
    { '0', 180, 220, }, /* TOKEN_ZERO */
    { '1', 480, 520, }, /* TOKEN_ONE */
    { 'M', 780, 820, }, /* TOKEN_MARKER */
    { '?', 0,   0,   }, /* TOKEN_INVALID */
    { '-', 0,   0,   }, /* TOKEN_MISSING */
};

typedef enum ObeliskState {
    STATE_WAITING   = 0,
    STATE_START     = 1,
    STATE_DATA      = 2,
    STATE_END       = 3,
} obelisk_state_t;

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
} obelisk_record_t;

typedef union ObeliskBuffer {
    uint64_t word;
    obelisk_record_t record;
    uint8_t octet[sizeof(obelisk_record_t)];
} obelisk_buffer_t;

static const int LENGTH[] = {
    8, 9, 9, 9, 9, 9,
};

int main(int argc, char ** argv)
{
    int rc = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    diminuto_sticks_t ticks_frequency = -1;
    diminuto_ticks_t ticks_delay = -1;
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_before = -1;
    diminuto_sticks_t ticks_after = -1;
    diminuto_sticks_t ticks_epoch = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_ticks_t ticks_elapsed = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_before = -1;
    int level_after = -1;
    int cycles_count = -1;
    int milliseconds_pulse = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state = (obelisk_state_t)-1;
    int fields = -1;
    int bits = -1;
    int length = -1;

    diminuto_log_setmask();

    /*
    ** Configure P1 output and T input pins.
    */

    pin_out_p1_fp = diminuto_pin_output(PIN_OUT_P1);
    if (pin_out_p1_fp == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_OUT_P1);
        assert(rc >= 0);
        pin_out_p1_fp = diminuto_pin_output(PIN_OUT_P1);
        assert(pin_out_p1_fp != (FILE *)0);
    }

    pin_in_t_fp = diminuto_pin_input(PIN_IN_T);
    if (pin_in_t_fp == (FILE *)0) {
        rc = diminuto_pin_unexport(PIN_IN_T);
        assert(rc >= 0);
        pin_in_t_fp = diminuto_pin_input(PIN_IN_T);
        assert(pin_in_t_fp != (FILE *)0);
    }

    /*
    ** Toggle P1 output pin (active low).
    */

    ticks_frequency = diminuto_delay_frequency();
    assert(ticks_frequency > 0);
    ticks_delay = ticks_frequency / HERTZ_RESET;

    rc = diminuto_pin_set(pin_out_p1_fp);
    assert(rc == 0);

    ticks_slack = diminuto_delay(ticks_delay, 0);
    assert(ticks_slack == 0);

    rc = diminuto_pin_clear(pin_out_p1_fp);
    assert(rc == 0);

    ticks_slack = diminuto_delay(ticks_delay, 0);
    assert(ticks_slack == 0);

    /*
    ** Initialize.
    */

    diminuto_cue_init(&cue, 0);

    ticks_delay = ticks_frequency / HERTZ_POLL;

    cycles_count = 0;
    milliseconds_pulse = 0;
    level_before = LEVEL_ZERO;

    token = TOKEN_MISSING;
    state = STATE_WAITING;

    /*
    ** Enter work loop.
    */

    while (!0) {

        /*
        ** Poll for T input pin changes.
        */

        ticks_before = diminuto_time_elapsed();
        assert(ticks_before >= 0);

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_after = diminuto_cue_debounce(&cue, level_raw);

        if (diminuto_cue_is_rising(&cue)) {
            ticks_epoch = ticks_before;
        }

        if (level_after > level_before) {
            DIMINUTO_LOG_DEBUG("0. RISING.\n");
        } else if (level_after < level_before) {
            DIMINUTO_LOG_DEBUG("0. FALLING.\n");
        } else {
            /* Do nothing. */
        }

        /*
        ** Respond to edge transitions.
        */

        edge = diminuto_cue_edge(&cue);

        switch (edge) {

        case DIMINUTO_CUE_EDGE_LOW:
            /* Do nothing. */
            break;

        case DIMINUTO_CUE_EDGE_RISING:
            cycles_count = 1;
            break;

        case DIMINUTO_CUE_EDGE_HIGH:
            ++cycles_count;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:
            milliseconds_pulse = cycles_count * MILLISECONDS_POLL;;
            cycles_count = 0;
            break;

        default:
            assert(edge != edge);
            break;

        }

        /*
        ** Classify pulse.
        */

        if (milliseconds_pulse > 0) {

            token = TOKEN_INVALID;
            for (obelisk_token_t tt = TOKEN_ZERO; tt <= TOKEN_MARKER; ++tt) {
                if (milliseconds_pulse < RANGE[tt].minimum) {
                    /* Do nothing. */
                } else if (milliseconds_pulse > RANGE[tt].maximum) {
                    /* Do nothing. */
                } else {
                    token = tt;
                    break;
                }
            }

            DIMINUTO_LOG_INFORMATION("1. PULSE %dms '%c'.\n", milliseconds_pulse, RANGE[token].symbol);

            milliseconds_pulse = 0;

        } else {

            token = TOKEN_MISSING;

        }

        /*
        ** Transition state based on token.
        */

        switch (state) {

        case STATE_WAITING:

            switch (token) {

            case TOKEN_ZERO:
            case TOKEN_ONE:
            case TOKEN_INVALID:
            case TOKEN_MISSING:
                break;

            case TOKEN_MARKER:
                state = STATE_END;
                break;

            default:
                assert(token != token);
                break;

            }

            break;

        case STATE_END:

            switch (token) {

            case TOKEN_ZERO:
            case TOKEN_ONE:
            case TOKEN_INVALID:
            case TOKEN_MISSING:
                state = STATE_WAITING;
                break;

            case TOKEN_MARKER:
                /* TODO RESET */
                state = STATE_START;
                break;

            default:
                assert(token != token);
                state = STATE_WAITING;
                break;

            }

            break;

        case STATE_START:

            switch (token) {

            case TOKEN_ZERO:
            case TOKEN_ONE:
                /* TODO SAVE */
                state = STATE_DATA;
                break;

            case TOKEN_MARKER:
                /* TODO LEAP SECOND */
                state = STATE_DATA;
                break;

            case TOKEN_INVALID:
            case TOKEN_MISSING:
                state = STATE_WAITING;
                break;

            default:
                assert(token != token);
                state = STATE_WAITING;
                break;

            }

            break;

        case STATE_DATA:

            switch (token) {

            case TOKEN_ZERO:
            case TOKEN_ONE:
                /* TODO SAVE */
                break;

            case TOKEN_MARKER:
                /* TODO PROCESS  */
                state = STATE_WAITING;
                state = STATE_DATA;
                state = STATE_END;
                break;

            case TOKEN_INVALID:
            case TOKEN_MISSING:
                state = STATE_WAITING;
                break;

            default:
                assert(token != token);
                state = STATE_WAITING;
                break;

            }

            break;

        default:
            assert(state != state);
            break;

        }

        /*
        ** Iterate while controlling jitter.
        */

        level_before = level_after;

        ticks_after = diminuto_time_elapsed();
        assert(ticks_after >= 0);

        assert(ticks_after >= ticks_before);
        ticks_elapsed = ticks_after - ticks_before;

        if (ticks_elapsed < ticks_delay) {
            ticks_slack = diminuto_delay(ticks_delay - ticks_elapsed, 0);
            assert(ticks_slack == 0);
        }

    }

    /*
    ** Release resources.
    */

    pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, PIN_IN_T);
    assert(pin_in_t_fp == (FILE *)0);

    pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, PIN_OUT_P1);
    assert(pin_out_p1_fp == (FILE *)0);

    return 0;
}
