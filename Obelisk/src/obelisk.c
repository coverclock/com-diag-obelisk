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
#include "com/diag/obelisk/obelisk.h"
#include "obelisk.h"

#define countof(_ARRAY_) (sizeof(_ARRAY_) / sizeof(_ARRAY_[0]))

/**
 * Reference: MAS, "MAS6180C AM Receiver IC", DA6180C.001, Micro Analog Systems,
 * 2014-09-17, p. 7, t. 5
 */
static const struct {
    int16_t minimum;
    int16_t maximum;
} MILLISECONDS[] = {
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

/**
 * Reference: Wikipedia, "WWVB", https://en.wikipedia.org/wiki/WWVB,
 * 2017-05-02
 */
static const int8_t LENGTH[] = {
    8,  /* minutes10, minutes1 */
    9,  /* hours10, hours1 */
    9,  /* day100, day10 */
    9,  /* day1, dutonesign */
    9,  /* dutone1, year10 */
    9,  /* year1, lyi, lsw, dst */
};

obelisk_state_t obelisk_parse(obelisk_state_t state, obelisk_token_t token, int * fieldp, int * lengthp, int * leapp, obelisk_buffer_t * bufferp)
{
    obelisk_action_t action = OBELISK_ACTION_NONE;

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
            action = OBELISK_ACTION_CLEAR;
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
            action = OBELISK_ACTION_ZERO;
            if (*lengthp > 1) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_ONE:
            action = OBELISK_ACTION_ONE;
            if (*lengthp > 1) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_MARKER:
            action = OBELISK_ACTION_LEAP;
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
            action = OBELISK_ACTION_ZERO;
            if (*lengthp > 1) {
                state = OBELISK_STATE_DATA;
            } else if (*fieldp < (countof(LENGTH) - 1)) {
                state = OBELISK_STATE_MARK;
            } else {
                state = OBELISK_STATE_END;
            }
            break;

        case OBELISK_TOKEN_ONE:
            action = OBELISK_ACTION_ONE;
            if (*lengthp > 1) {
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
            action = OBELISK_ACTION_MARK;
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
            action = OBELISK_ACTION_FINAL;
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

    switch (action) {

    case OBELISK_ACTION_NONE:
        break;

    case OBELISK_ACTION_CLEAR:
        *leapp = 0;
        *bufferp = 0;
        *fieldp = 0;
        *lengthp = LENGTH[0];
        break;

    case OBELISK_ACTION_ZERO:
        *bufferp <<= 1;
        *lengthp -= 1;
        break;

    case OBELISK_ACTION_ONE:
        *bufferp <<= 1;
        *bufferp |= 1;
        *lengthp -= 1;
        break;

    case OBELISK_ACTION_LEAP:
        *leapp = !0;
        break;

    case OBELISK_ACTION_MARK:
        *bufferp <<= 1;
        *fieldp += 1;
        assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
        *lengthp = LENGTH[*fieldp];
        break;

    case OBELISK_ACTION_FINAL:
        *bufferp <<= 1;
        break;

    default:
        assert(action != action);
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

static const int8_t DAYS[2][12] = {
/*    JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC */
    {  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },    /* !LYI */
    {  31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },    /* LYI */
};

/*
 * Exposed for unit testing.
 */
int obelisk_julian2gregorian(int julian, int lyi, int * monthp, int * dayp) {
    int rc = -1;

    for (int month = 0; month < countof(DAYS[0]); ++month) {
        if (julian <= DAYS[lyi][month]) {
            *monthp = month + 1;
            *dayp = julian;
            rc = 0;
            break;
        }
        julian -= DAYS[lyi][month];
    }

    return rc;
}

/*
 * Exposed for unit testing.
 */
int obelisk_zeller(int year, int month, int day)
{
    int ye;
    int a;
    int y;
    int m;
    int d;

    /*
     * Reference: C. Overclock, Date::weekday, Date.cpp,
     * https://github.com/coverclock/com-diag-grandote,
     * 2017-09-27
     *
     * Reference: Wikipedia, "Zeller's congruence",
     * https://en.wikipedia.org/wiki/Zeller%27s_congruence,
     * 2017-08-31
     */
    ye = ((year - 1) % 400) + 1;
    a = (14 - month) / 12;
    y = ye - a;
    m = month + (12 * a) - 2;
    d = day + y + (y / 4) - (y / 100) + (y / 400) + ((31 * m) / 12);

    return (((((d % 7) + 6) % 7) + 1) % 7);
}

int obelisk_validate(struct tm * timep, const obelisk_frame_t * framep)
{
    int rc = -1;
    int days = -1;

    timep->tm_sec = 0;
    timep->tm_min = (framep->minutes10 * 10) + framep->minutes1;
    timep->tm_hour = (framep->hours10 * 10) + framep->hours1;
    timep->tm_year = (framep->year10 * 10) + framep->year1;
    timep->tm_year += (timep->tm_year < 17) ? 200 : 100;
    timep->tm_isdst = !!framep->dst;
    timep->tm_gmtoff = 0;
    timep->tm_zone = "UTC";

    days  = (framep->day100 * 100) + (framep->day10 * 10) + framep->day1;
    timep->tm_yday = days - 1;
    
    rc = obelisk_julian2gregorian(days, framep->lyi, &(timep->tm_mon), &(timep->tm_mday));

    timep->tm_wday = obelisk_zeller(timep->tm_year + 1900, timep->tm_mon, timep->tm_mday);

    timep->tm_mon -= 1;

    days = framep->lyi ? 365 : 364;

    /*
     * Reference: ctime(3)
     *
     * Reference: /usr/include/time.h
     */
    if (rc < 0) {
        /* Do nothing. */
    } else if (!((0 <= timep->tm_min) && (timep->tm_min <= 59))) {
        rc = -2;
    } else if (!((0 <= timep->tm_hour) && (timep->tm_hour <= 23))) {
        rc = -3;
    } else if (!((0 <= timep->tm_mon) && (timep->tm_mon <= 11))) {
        rc = -4;
    } else if (!((1 <= timep->tm_mday) && (timep->tm_mday <= DAYS[framep->lyi][timep->tm_mon]))) {
        rc = -5;
    } else if (!((117 <= timep->tm_year) && (timep->tm_year <= 216))) {
        rc = -6;
    } else if (!((0 <= timep->tm_wday) && (timep->tm_wday <= 6))) {
        rc = -7;
    } else if (!((0<= timep->tm_yday) && (timep->tm_yday <= days))) {
        rc = -8;
    } else {
        rc = 0;
    }

    return rc;
}
