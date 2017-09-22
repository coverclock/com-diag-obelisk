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

static const obelisk_range_t RANGE[] = {
    { 180, 220, }, /* TOKEN_ZERO */
    { 480, 520, }, /* TOKEN_ONE */
    { 780, 820, }, /* TOKEN_MARKER */
    { 0,   0,   }, /* TOKEN_INVALID */
    { 0,   0,   }, /* TOKEN_PENDING */
};

static const int LENGTH[] = {
    8,
    9,
    9,
    9,
    9,
    9,
};

int obelisk_measure(diminuto_cue_state_t * cuep, int cycles_count)
{
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;

    edge = diminuto_cue_edge(cuep);

    switch (edge) {

    case DIMINUTO_CUE_EDGE_LOW:
        cycles_count = 0;
        break;

    case DIMINUTO_CUE_EDGE_RISING:
        cycles_count = 1;
        break;

    case DIMINUTO_CUE_EDGE_HIGH:
        cycles_count += 1;
        break;

    case DIMINUTO_CUE_EDGE_FALLING:
        cycles_count = 0;
        break;

    default:
        assert(edge != edge);
        cycles_count = 0;
        break;

    }

    return cycles_count;
}

obelisk_token_t obelisk_tokenize(int milliseconds_pulse)
{
    obelisk_token_t token = TOKEN_PENDING;

    if (milliseconds_pulse > 0) {

        token = TOKEN_INVALID;
        for (obelisk_token_t tt = TOKEN_ZERO; tt <= TOKEN_MARKER; ++tt) {
            assert((0 <= tt) && (tt < countof(RANGE)));
            if (milliseconds_pulse < RANGE[tt].minimum) {
                /* Do nothing. */
            } else if (milliseconds_pulse > RANGE[tt].maximum) {
                /* Do nothing. */
            } else {
                token = tt;
                break;
            }
        }

        milliseconds_pulse = 0;

        assert((0 <= token) && (token < countof(RANGE)));

    }

    return token;
}

obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * bitp, int * leapp, obelisk_buffer_t * bufferp)
{
    switch (state) {

    case STATE_WAIT:

        switch (token) {

        case TOKEN_ZERO:
        case TOKEN_ONE:
        case TOKEN_INVALID:
        case TOKEN_PENDING:
            /* Do nothing. */
            break;

        case TOKEN_MARKER:
            state = STATE_SYNC;
            break;

        default:
            assert(token != token);
            break;

        }

        break;

    case STATE_SYNC:

        switch (token) {

        case TOKEN_ZERO:
        case TOKEN_ONE:
        case TOKEN_INVALID:
        case TOKEN_PENDING:
            state = STATE_WAIT;
            break;

        case TOKEN_MARKER:
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            *bitp = sizeof(bufferp->word) * 8;
            *leapp = 0;
            bufferp->word = 0;
            state = STATE_START;
            break;

        default:
            assert(token != token);
            state = STATE_WAIT;
            break;

        }

        break;

    case STATE_START:

        switch (token) {

        case TOKEN_ZERO:
            *bitp -= 1;
            *lengthp -= 1;
            state = STATE_DATA;
            break;

        case TOKEN_ONE:
            *bitp -= 1;
            bufferp->word |= 1ULL << *bitp;
            *lengthp -= 1;
            state = STATE_DATA;
            break;

        case TOKEN_MARKER:
            *leapp = !0;
            state = STATE_DATA;
            break;

        case TOKEN_INVALID:
        case TOKEN_PENDING:
            state = STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = STATE_WAIT;
            break;

        }

        break;

    case STATE_DATA:

        switch (token) {

        case TOKEN_ZERO:
            *bitp -= 1;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = STATE_MARK;
            } else {
                state = STATE_END;
            }
            break;

        case TOKEN_ONE:
            *bitp -= 1;
            bufferp->word |= 1ULL << *bitp;
            *lengthp -= 1;
            if (*lengthp > 0) {
                state = STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = STATE_MARK;
            } else {
                state = STATE_END;
            }
            break;

        case TOKEN_MARKER:
        case TOKEN_INVALID:
        case TOKEN_PENDING:
            state = STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = STATE_WAIT;
            break;

        }

        break;

    case STATE_MARK:

        switch (token) {

        case TOKEN_MARKER:
            *fieldp += 1;
            state = STATE_DATA;
            break;

        case TOKEN_ZERO:
        case TOKEN_ONE:
        case TOKEN_INVALID:
        case TOKEN_PENDING:
            state = STATE_WAIT;
            break;

        default:
            assert(token != token);
            state = STATE_WAIT;
            break;

        }

        break;

    case STATE_END:

        switch (token) {

        case TOKEN_ZERO:
        case TOKEN_ONE:
        case TOKEN_INVALID:
        case TOKEN_PENDING:
            state = STATE_WAIT;
            break;

        case TOKEN_MARKER:
            *fieldp = 0;
            assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
            *lengthp = LENGTH[*fieldp];
            *bitp = sizeof(bufferp->word) * 8;
            state = STATE_START;
            break;

        default:
            assert(token != token);
            state = STATE_WAIT;
            break;

        }

        break;

    default:
        assert(state != state);
        state = STATE_WAIT;
        break;

    }

    return state;
}
