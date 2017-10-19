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
 * Reference:   MAS, "MAS6180C AM Receiver IC", DA6180C.001, Micro Analog
 *              Systems, 2014-09-17, p. 7, t. 5
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

/*
 * Exposed for unit testing.
 */
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

/**
 * Reference:   Wikipedia, "WWVB", https://en.wikipedia.org/wiki/WWVB,
 *              2017-05-02
 */
static const int8_t LENGTH[] = {
    8,  /* minutes10, minutes1 */
    9,  /* hours10, hours1 */
    9,  /* day100, day10 */
    9,  /* day1, dutonesign */
    9,  /* dutone1, year10 */
    9,  /* year1, lyi, lsw, dst */
};

obelisk_status_t obelisk_parse(obelisk_state_t * statep, obelisk_token_t token, int * fieldp, int * lengthp, obelisk_buffer_t * bufferp, obelisk_frame_t * framep)
{
    obelisk_status_t status = OBELISK_STATUS_NOMINAL;
    obelisk_state_t state = OBELISK_STATE_START;
    obelisk_action_t action = OBELISK_ACTION_NONE;

    assert(statep != (obelisk_state_t *)0);
    assert((OBELISK_TOKEN_FIRST <= token) && (token <= OBELISK_TOKEN_LAST));
    assert(fieldp != (int *)0);
    assert(lengthp != (int *)0);
    assert(bufferp != (obelisk_buffer_t *)0);
    assert(framep != (obelisk_frame_t *)0);

    /*
     * The finite state machine synchronizes at the start of a frame by
     * looking for the sequence
     *
     *   DATA   DATA   END    BEGIN    LEAP
     * ( ZERO | ONE  ) MARKER MARKER [ MARKER ]
     *
     * which unambiguously signals the beginning of a new frame with an
     * optional third MARKER indicating a leap second. We have to start
     * by looking for the final ZERO or ONE data bits at the end of the
     * prior frame to distinguish between the sequence of BEGIN and END
     * MARKERs (which we want) and BEGIN and LEAP MARKERS. The latter
     * case causes us to be one second off in our classification of
     * the current minute.
     */

    state = *statep;

    assert((OBELISK_STATE_FIRST <= state) && (state <= OBELISK_STATE_LAST));

    switch (state) {

    case OBELISK_STATE_START:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
            status = OBELISK_STATUS_WAITING;
            break;

        case OBELISK_TOKEN_MARKER:
            /* First MARKER could be END, BNEGIN, or LEAP. */
            status = OBELISK_STATUS_WAITING;
            state = OBELISK_STATE_WAIT;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
            break;

        }

        break;

    case OBELISK_STATE_WAIT:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
        case OBELISK_TOKEN_ONE:
            status = OBELISK_STATUS_WAITING;
            state = OBELISK_STATE_START;
            break;

        case OBELISK_TOKEN_MARKER:
            /* Second MARKER could be BEGIN or LEAP. */
            status = OBELISK_STATUS_WAITING;
            action = OBELISK_ACTION_CLEAR;
            state = OBELISK_STATE_SYNC;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_SYNC:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            status = OBELISK_STATUS_NOMINAL;
            action = OBELISK_ACTION_ZERO;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_ONE:
            status = OBELISK_STATUS_NOMINAL;
            action = OBELISK_ACTION_ONE;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_MARKER:
            /* Ignore third MARKER which must be LEAP. */
            status = OBELISK_STATUS_NOMINAL;
            state = OBELISK_STATE_DATA;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
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

        default:
            status = OBELISK_STATUS_INVALID;
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

        default:
            status = OBELISK_STATUS_INVALID;
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_END:

        switch (token) {

        case OBELISK_TOKEN_MARKER:
            status = OBELISK_STATUS_FRAME;
            action = OBELISK_ACTION_FINAL;
            state = OBELISK_STATE_BEGIN;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_BEGIN:

        switch (token) {

        case OBELISK_TOKEN_MARKER:
            status = OBELISK_STATUS_TIME;
            action = OBELISK_ACTION_CLEAR;
            state = OBELISK_STATE_LEAP;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
            state = OBELISK_STATE_START;
            break;

        }

        break;

    case OBELISK_STATE_LEAP:

        switch (token) {

        case OBELISK_TOKEN_ZERO:
            action = OBELISK_ACTION_ZERO;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_ONE:
            action = OBELISK_ACTION_ONE;
            state = OBELISK_STATE_DATA;
            break;

        case OBELISK_TOKEN_MARKER:
            status = OBELISK_STATUS_LEAP;
            state = OBELISK_STATE_DATA;
            break;

        default:
            status = OBELISK_STATUS_INVALID;
            state = OBELISK_STATE_START;
            break;

        }

        break;

    default:
        status = OBELISK_STATUS_INVALID;
        state = OBELISK_STATE_START;
        break;

    }

    assert((OBELISK_STATE_FIRST <= state) && (state <= OBELISK_STATE_LAST));

    *statep = state;

    switch (action) {

    case OBELISK_ACTION_NONE:
        /* Do nothing. */
        break;

    case OBELISK_ACTION_CLEAR:
        *bufferp = 0;
        *fieldp = 0;
        assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
        *lengthp = LENGTH[*fieldp];
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

    case OBELISK_ACTION_MARK:
        *bufferp <<= 1;
        *fieldp += 1;
        assert((0 <= *fieldp) && (*fieldp < countof(LENGTH)));
        *lengthp = LENGTH[*fieldp];
        break;

    case OBELISK_ACTION_FINAL:
        *bufferp <<= 1;
        obelisk_extract(framep, *bufferp);
        break;

    default:
        /* Do nothing. */
        break;

    }

    assert((OBELISK_STATUS_FIRST <= status) && (status <= OBELISK_STATUS_LAST));

    return status;
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

    if (julian > 0) {
        for (int month = 0; month < countof(DAYS[0]); ++month) {
            if (julian <= DAYS[lyi][month]) {
                *monthp = month + 1;
                *dayp = julian;
                rc = 0;
                break;
            }
            julian -= DAYS[lyi][month];
        }
    }

    return rc;
}

/*
 * This exists just to avoid an enum cast.
 */
static const obelisk_zeller_t ZELLER[] = {
    OBELISK_ZELLER_SUNDAY,
    OBELISK_ZELLER_MONDAY,
    OBELISK_ZELLER_TUESDAY,
    OBELISK_ZELLER_WEDNESDAY,
    OBELISK_ZELLER_THURSDAY,
    OBELISK_ZELLER_FRIDAY,
    OBELISK_ZELLER_SATURDAY,
};

/*
 * Exposed for unit testing.
 */
obelisk_zeller_t obelisk_zeller(int year, int month, int day)
{
    int index = -1;
    int ye = -1;
    int a = -1;
    int y = -1;
    int m = -1;
    int d = -1;

    /*
     * Reference:   C. Overclock, Date::weekday, Date.cpp,
     *              https://github.com/coverclock/com-diag-grandote,
     *              2017-09-27
     *
     * Reference:   Wikipedia, "Zeller's congruence",
     *              https://en.wikipedia.org/wiki/Zeller%27s_congruence,
     *              2017-08-31
     */
    ye = ((year - 1) % 400) + 1;
    a = (14 - month) / 12;
    y = ye - a;
    m = month + (12 * a) - 2;
    d = day + y + (y / 4) - (y / 100) + (y / 400) + ((31 * m) / 12);

    index = (((((d % 7) + 6) % 7) + 1) % 7);
    assert((0 <= index) && (index < countof(ZELLER)));

    return ZELLER[index];
}

int obelisk_validate(const obelisk_frame_t * framep)
{
    int rc = 0;

    if (!((0 <= framep->minutes10) && (framep->minutes10 <= 5))) {
        rc = -2;
    } else if (!((0 <= framep->minutes1) && (framep->minutes1 <= 9))) {
        rc = -3;
    } else if (!((0 <= framep->hours10) && (framep->hours10 <= 2))) {
        rc = -3;
    } else if (!((0 <= framep->hours1) && (framep->hours1 <= 9))) {
        rc = -5;
    } else if (!((0 <= framep->day100) && (framep->day100 <= 3))) {
        rc = -6;
    } else if (!((0 <= framep->day10) && (framep->day10 <= 9))) {
        rc = -7;
    } else if (!((0 <= framep->day1) && (framep->day1 <= 9))) {
        rc = -8;
    } else if (!((framep->dutonesign == OBELISK_SIGN_NEGATIVE) || (framep->dutonesign == OBELISK_SIGN_POSITIVE))) {
        rc = -9;
    } else if (!((0 <= framep->dutone1) && (framep->dutone1 <= 9))) {
        rc = -10;
    } else if (!((0 <= framep->year10) && (framep->year10 <= 9))) {
        rc = -11;
    } else if (!((0 <= framep->year1) && (framep->year1 <= 9))) {
        rc = -12;
    } else if (!((framep->dst == OBELISK_DST_OFF) || (framep->dst == OBELISK_DST_ENDS) || (framep->dst == OBELISK_DST_BEGINS) || (framep->dst == OBELISK_DST_ON))) {
        rc = -13;
    } else {
        /* Do nothing. */
    }

    return rc;
}

int obelisk_decode(struct tm * timep, const obelisk_frame_t * framep)
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
    
    rc = obelisk_julian2gregorian(days, framep->lyi, &(timep->tm_mon), &(timep->tm_mday));

    timep->tm_yday = days - 1; /* tm_yday is zero based. */

    timep->tm_wday = obelisk_zeller(timep->tm_year + 1900, timep->tm_mon, timep->tm_mday); /* tm_wday is zero based but so is zeller(). */

    timep->tm_mon -= 1; /* tm_mon is zero based. */

    return rc;
}

int obelisk_revalidate(const struct tm * timep)
{
    int rc = 0;
    int year = -1;
    int leap = -1;
    int days = -1;

    year = timep->tm_year + 1900;
    leap = ((year % 4) == 0) && ((year % 400) != 0);
    days = leap ? 366 : 365;

    /*
     * Reference:   ctime(3)
     *
     * Reference:   /usr/include/time.h
     */
    if (!((0 <= timep->tm_sec) && (timep->tm_sec <= 60))) {
        rc = -14;
    } else if (!((0 <= timep->tm_min) && (timep->tm_min <= 59))) {
        rc = -15;
    } else if (!((0 <= timep->tm_hour) && (timep->tm_hour <= 23))) {
        rc = -16;
    } else if (!((0 <= timep->tm_mon) && (timep->tm_mon <= 11))) {
        rc = -17;
    } else if (!((1 <= timep->tm_mday) && (timep->tm_mday <= DAYS[leap][timep->tm_mon]))) {
        rc = -18;
    } else if (!((117 <= timep->tm_year) && (timep->tm_year <= 216))) {
        rc = -19;
    } else if (!((0 <= timep->tm_wday) && (timep->tm_wday <= 6))) {
        rc = -20;
    } else if (!((0 <= timep->tm_yday) && (timep->tm_yday <= days))) {
        rc = -21;
    } else if (!((-1 <= timep->tm_isdst) && (timep->tm_isdst <= 1))) {
        rc = -22;
    } else {
        rc = 0;
    }

    return 0;
}
