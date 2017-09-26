/* vim: set ts=4 expandtab shiftwidth=4: */

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
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/obelisk/obelisk.h"

static const obelisk_range_t MILLISECONDS[] = {
    { 100, 300, },  /* 200ms OBELISK_TOKEN_ZERO */
    { 400, 600, },  /* 500ms OBELISK_TOKEN_ONE */
    { 700, 900, },  /* 800ms OBELISK_TOKEN_MARKER */
};

obelisk_token_t obelisk_tokenize(int milliseconds_pulse)
{
    obelisk_token_t token = OBELISK_TOKEN_INVALID;

     for (obelisk_token_t tt = OBELISK_TOKEN_ZERO; tt <= OBELISK_TOKEN_MARKER; ++tt) {
         assert((0 <= tt) && (tt < countof(MILLISECONDS)));
         if (milliseconds_pulse < MILLISECONDS[tt].minimum) {
             /* Do nothing. */
         } else if (milliseconds_pulse > MILLISECONDS[tt].maximum) {
             /* Do nothing. */
         } else {
             token = tt;
             break;
         }
     }

    return token;
}

static const int LENGTH[] = {
    /* OBELISK_STATE_BEGIN */
    /* (optional OBELISK_STATE_MARK for leap second) */
    8,
    /* OBELISK_STATE_MARK */
    9,
    /* OBELISK_STATE_MARK */
    9,
    /* OBELISK_STATE_MARK */
    9,
    /* OBELISK_STATE_MARK */
    9,
    /* OBELISK_STATE_MARK */
    9,
    /* OBELISK_STATE_END */
};

obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * leapp, obelisk_buffer_t * bufferp)
{
    switch (state) {

    case OBELISK_STATE_START:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
            break;

        case OBELISK_TOKEN_MARKER:
            state = OBELISK_STATE_BEGIN;
            break;

        default:
            assert(token != token);
            break;

        }

        break;

    case OBELISK_STATE_BEGIN:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
            state = OBELISK_STATE_START;
            break;

        case OBELISK_TOKEN_MARKER:
            *bufferp = 0;
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            state = OBELISK_STATE_LEAP;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_LEAP:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            *bufferp <<= 1;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_ONE:
            *bufferp <<= 1;
            *bufferp |= 1;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_MARKER:
            *leapp = !0;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_INVALID:
            state = OBELISK_STATE_START;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_DATA:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            *bufferp <<= 1;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_ONE:
            *bufferp <<= 1;
            *bufferp |= 1;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_MARKER:
        case OBELISK_TOKEN_INVALID:
            state = OBELISK_STATE_START;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_MARK:

        switch (token) {

        case OBELISK_TOKEN_MARKER:
            *bufferp <<= 1;
            *fieldp += 1;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
            state = OBELISK_STATE_START;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_END:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
            state = OBELISK_STATE_START;
            break;

        case OBELISK_TOKEN_MARKER:
            *bufferp <<= 1;
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            state = OBELISK_STATE_BEGIN;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_START;
            break;

        }

        break;

    default:
        assert(state != state);
        state = OBELISK_STATE_START;
        break;

    }

    return state;
}

void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer)
{
    framep->minutes10   = OBELISK_EXTRACT(buffer, MINUTES10);
    framep->minutes1    = OBELISK_EXTRACT(buffer, MINUTES1);
    framep->hours10     = OBELISK_EXTRACT(buffer, HOURS10);
    framep->hours1      = OBELISK_EXTRACT(buffer, HOURS1);
    framep->day100      = OBELISK_EXTRACT(buffer, DAY100);
    framep->day10       = OBELISK_EXTRACT(buffer, DAY10);
    framep->day1        = OBELISK_EXTRACT(buffer, DAY1);
    framep->dutonesign  = OBELISK_EXTRACT(buffer, DUTONESIGN);
    framep->dutone1     = OBELISK_EXTRACT(buffer, DUTONE1);
    framep->year10      = OBELISK_EXTRACT(buffer, YEAR10);
    framep->year1       = OBELISK_EXTRACT(buffer, YEAR1);
    framep->lyi         = OBELISK_EXTRACT(buffer, LYI);
    framep->lsw         = OBELISK_EXTRACT(buffer, LSW);
    framep->dst         = OBELISK_EXTRACT(buffer, DST);
}
