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

int obelisk_measure(diminuto_cue_state_t * cuep, int milliseconds_pulse, int milliseconds_cycle)
{
    diminuto_cue_edge_t edge;

    edge = diminuto_cue_edge(cuep);

    switch (edge) {

    case DIMINUTO_CUE_EDGE_LOW:
        milliseconds_pulse = 0;
        break;

    case DIMINUTO_CUE_EDGE_RISING:
        milliseconds_pulse = milliseconds_cycle;
        break;

    case DIMINUTO_CUE_EDGE_HIGH:
        milliseconds_pulse += milliseconds_cycle;
        break;

    case DIMINUTO_CUE_EDGE_FALLING:
        milliseconds_pulse = 0;
        break;

    default:
        assert(edge != edge);
        milliseconds_pulse = 0;
        break;

    }

    return milliseconds_pulse;
}

static const obelisk_range_t MILLISECONDS[] = {
    { 180, 220, },  /* OBELISK_TOKEN_ZERO */
    { 480, 520, },  /* OBELISK_TOKEN_ONE */
    { 780, 820, },  /* OBELISK_TOKEN_MARKER */
};

obelisk_token_t obelisk_tokenize(int milliseconds_pulse)
{
    obelisk_token_t token = OBELISK_TOKEN_PENDING;

    if (milliseconds_pulse > 0) {

        token = OBELISK_TOKEN_INVALID;

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

    }

    return token;
}

static const int LENGTH[] = {
    /* OBELISK_STATE_START */
    /* (OBELISK_STATE_MARK for leap second) */
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

obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * bitp, int * leapp, obelisk_buffer_t * bufferp)
{
    switch (state) {

    case OBELISK_STATE_WAIT:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
        case OBELISK_TOKEN_PENDING:
            /* Do nothing. */
            break;

        case OBELISK_TOKEN_MARKER:
            state = OBELISK_STATE_SYNC;
            break;

        default:
            assert(token != token);
            break;

        }

        break;

    case OBELISK_STATE_SYNC:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
        case OBELISK_TOKEN_PENDING:
            state = OBELISK_STATE_WAIT;
            break;

        case OBELISK_TOKEN_MARKER:
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            *bitp = sizeof(*bufferp) * 8;
            *leapp = 0;
            *bufferp = 0;
            state = OBELISK_STATE_START;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_WAIT;
            break;

        }

        break;

    case OBELISK_STATE_START:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            *bitp -= 1;
            *bufferp = 0;
            *lengthp -= 1;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_ONE:
            *bitp -= 1;
            *bufferp = 1ULL << *bitp;
            *lengthp -= 1;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_MARKER:
            *leapp = !0;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_INVALID:
        case OBELISK_TOKEN_PENDING:
            state = OBELISK_STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_WAIT;
            break;

        }

        break;

    case OBELISK_STATE_DATA:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            *bitp -= 1;
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
            *bitp -= 1;
            *bufferp |= 1ULL << *bitp;
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
        case OBELISK_TOKEN_PENDING:
            state = OBELISK_STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_WAIT;
            break;

        }

        break;

    case OBELISK_STATE_MARK:

        switch (token) {

        case OBELISK_TOKEN_MARKER:
            *fieldp += 1;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
        case OBELISK_TOKEN_PENDING:
            state = OBELISK_STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_WAIT;
            break;

        }

        break;

    case OBELISK_STATE_END:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
        case OBELISK_TOKEN_INVALID:
        case OBELISK_TOKEN_PENDING:
            state = OBELISK_STATE_WAIT;
            break;

        case OBELISK_TOKEN_MARKER:
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            *bitp = sizeof(*bufferp) * 8;
            state = OBELISK_STATE_START;
            break;

        default:
            assert(token != token);
            state = OBELISK_STATE_WAIT;
            break;

        }

        break;

    default:
        assert(state != state);
        state = OBELISK_STATE_WAIT;
        break;

    }

    return state;
}

static const uint64_t MASK_MINUTES  = 0xffULL   << 56;
static const uint64_t MASK_HOURS    = 0x7fULL   << 47;
static const uint64_t MASK_DAY      = 0x7ffULL  << 34;
static const uint64_t MASK_SIGN     = 0x7ULL    << 29;
static const uint64_t MASK_DUT1     = 0xfULL    << 25;
static const uint64_t MASK_YEAR     = 0xffULL   << 16;
static const uint64_t MASK_LYI      = 0x1ULL    << 14;
static const uint64_t MASK_LSW      = 0x1ULL    << 13;
static const uint64_t MASK_DST      = 0x3ULL    << 11;

void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer)
{
    framep->minutes = (buffer & MASK_MINUTES)   >> 56;
    framep->hours   = (buffer & MASK_HOURS)     >> 47;
    framep->day     = (buffer & MASK_DAY)       >> 34;
    framep->sign    = (buffer & MASK_SIGN)      >> 29;
    framep->dut1    = (buffer & MASK_DUT1)      >> 25;
    framep->year    = (buffer & MASK_YEAR)      >> 16;
    framep->lyi     = (buffer & MASK_LYI)       >> 14;
    framep->lsw     = (buffer & MASK_LSW)       >> 13;
    framep->dst     = (buffer & MASK_DST)       >> 11;
}
