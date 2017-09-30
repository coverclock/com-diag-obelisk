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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_timer.h"
#include "com/diag/diminuto/diminuto_alarm.h"
#include "com/diag/diminuto/diminuto_cue.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_lock.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/obelisk/obelisk.h"

#define LOG(_FORMAT_, ...) fprintf(stderr, "%s: " _FORMAT_ "\n", program, ## __VA_ARGS__)

static const char ROOT[] = "/var/lock/";
static const size_t LIMIT = 64;
static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int HERTZ_DELAY = 2;
static const int HERTZ_TIMER = 100;
static const int HOUR_JULIET = 1;
static const int MINUTE_JULIET = 30;

static const const char * TOKEN[] = {
    "ZERO",     /* OBELISK_TOKEN_ZERO */
    "ONE",      /* OBELISK_TOKEN_ONE */
    "MARKER",   /* OBELISK_TOKEN_MARKER */
    "INVALID",  /* OBELISK_TOKEN_INVALID */
};

static const char * STATE[] = {
    "START",    /* OBELISK_STATE_START */
    "BEGIN",    /* OBELISK_STATE_BEGIN */
    "LEAP",     /* OBELISK_STATE_LEAP */
    "DATA",     /* OBELISK_STATE_DATA */
    "MARK",     /* OBELISK_STATE_MARK */
    "END",      /* OBELISK_STATE_END */
};

static const char * DAY[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
};

static const char * program = (const char *)0;

static int debug = 0;
static int reset = 0;
static int verbose = 0;
static int pin_out_p1 = -1;
static int pin_in_t = -1;
static int unexport = 0;
static int background = 0;
static int set = 0;
static int hour_juliet = -1;
static int minute_juliet = -1;

static void usage(void)
{
    fprintf(stderr, "usage: %s [ -H HOUR ] [ -M MINUTE ] [ -P PIN ] [ -T PIN ] [ -b ] [ -d ] [ -h ] [ -r ] [ -s ] [ -u ] [ -v ]\n", program);
    fprintf(stderr, "       -H HOUR         Set time of day at HOUR local (%d)\n", hour_juliet);
    fprintf(stderr, "       -M MINUTE       Set time of day at MINUTE local (%d).\n", minute_juliet);
    fprintf(stderr, "       -P PIN          Use P1 output GPIO PIN (%d).\n", pin_out_p1);
    fprintf(stderr, "       -T PIN          Use T input GPIO PIN (%d).\n", pin_in_t);
    fprintf(stderr, "       -b              Daemonize into the background.\n");
    fprintf(stderr, "       -d              Display debug output.\n");
    fprintf(stderr, "       -h              Display help menu and exit.\n");
    fprintf(stderr, "       -r              Reset device initially.\n");
    fprintf(stderr, "       -s              Set time of day when possible.\n");
    fprintf(stderr, "       -u              Unexport pins initially ignoring errors.\n");
    fprintf(stderr, "       -v              Display verbose output.\n");
}

int main(int argc, char ** argv)
{
    int rc = -1;
    pid_t pid = -1;
    char * path = (char *)0;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    diminuto_sticks_t ticks_frequency = -1;
    diminuto_ticks_t ticks_delay = -1;
    diminuto_ticks_t ticks_timer = -1;
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_cooked = -1;
    int milliseconds_cycle = -1;
    int milliseconds_pulse = -1;
    int cycles_limit = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state_before = (obelisk_state_t)-1;
    obelisk_state_t state_after = (obelisk_state_t)-1;
    obelisk_buffer_t buffer = (obelisk_buffer_t)-1;
    obelisk_frame_t frame = { 0 };
    struct tm time = { 0 };
    struct timeval value = { 0 };
    extern long timezone;
    extern int daylight;
    int field = -1;
    int length = -1;
    int leap = -1;
    int opt = -1;
    extern char * optarg;
    char * endptr = (char *)0;
    int armed = -1;
    int synchronized = -1;
    int year = -1;
    int month = -1;
    int day = -1;
    int hour = -1;
    int minute = -1;
    int second = -1;
    diminuto_ticks_t fraction = (diminuto_ticks_t)-1;
    int acquired = -1;
    int cycles = -1;
    int risings = -1;
    int fallings = -1;

    diminuto_log_setmask();

    /*
     * Process command line options.
     */

    program = strrchr(argv[0], '/');
    program = (program == (const char *)0) ? argv[0] : program + 1;

    pin_out_p1 = PIN_OUT_P1;
    pin_in_t = PIN_IN_T;
    hour_juliet = HOUR_JULIET;
    minute_juliet = MINUTE_JULIET;

    while ((opt = getopt(argc, argv, "H:M:P:T:bdhrsuv")) >= 0) {

        switch (opt) {

        case 'H':
            hour_juliet = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (hour_juliet < 0) || (hour_juliet > 23)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'M':
            minute_juliet = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (minute_juliet < 0) || (minute_juliet > 59)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'P':
            pin_out_p1 = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_p1 < 0)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'T':
            pin_in_t = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_in_t < 0)) {
                errno = EINVAL;
                perror(optarg);
                return 1;
            }
            break;

        case 'b':
            background = !0;
            break;

        case 'd':
            debug = !0;
            break;

        case 'h':
            usage();
            return 0;
            break;

        case 'r':
            reset = !0;
            break;

        case 's':
            set = !0;
            break;

        case 'u':
            unexport = !0;
            break;

        case 'v':
            verbose = !0;
            break;

        default:
            return 1;
            break;

        }

    }

    /*
     * Daemonize if so instructed. A lock file containing the daemon's
     * process identifier will be left in /var/lock.
     */

    if (background) {
        path = malloc(sizeof(ROOT) + strnlen(program, LIMIT));
        strcpy(path, ROOT);
        strncat(path, program, LIMIT);
        if (debug) { LOG("0 PATH \"%s\"", path); }
        rc = diminuto_daemon(program, path);
        if (rc < 0) { return 2; }
        pid = getpid();
        DIMINUTO_LOG_INFORMATION("%s: running pid=%d\n", program, getpid());
    } else {
        pid = getpid();
    }

    if (debug) { LOG("0 RUNNING %d.", pid); }

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

    /*
    ** Configure P1 output and T input pins.
    */

    if (unexport) {

        if (debug) { LOG("0 UNEXPORTING."); }

        (void)diminuto_pin_unexport(pin_out_p1);

        (void)diminuto_pin_unexport(pin_in_t);

    }

    if (debug) { LOG("0 EXPORTING."); }

     pin_out_p1_fp = diminuto_pin_output(pin_out_p1);
     assert(pin_out_p1_fp != (FILE *)0);

     pin_in_t_fp = diminuto_pin_input(pin_in_t);
     assert(pin_in_t_fp != (FILE *)0);

    /*
    ** Toggle P1 output pin (active low) if requested.
    */

    ticks_frequency = diminuto_frequency();
    assert(ticks_frequency > 0);

    if (reset) {

        if (debug) { LOG("0 RESETTING."); }

        ticks_delay = ticks_frequency / HERTZ_DELAY;

        rc = diminuto_pin_set(pin_out_p1_fp);
        assert(rc == 0);

        ticks_slack = diminuto_delay(ticks_delay, 0);
        assert(ticks_slack == 0);

        rc = diminuto_pin_clear(pin_out_p1_fp);
        assert(rc == 0);

        ticks_slack = diminuto_delay(ticks_delay, 0);
        assert(ticks_slack == 0);

    }

    /*
    ** Initialize state.
    */

    if (debug) { LOG("0 INITIALIZING."); }

    if (set) {
        tzset();
    }

    diminuto_cue_init(&cue, 0);

    milliseconds_pulse = 0;

    milliseconds_cycle  = 1000 / HERTZ_TIMER;

    cycles_limit = HERTZ_TIMER * 2;

    level_raw = OBELISK_LEVEL_ZERO;

    token = OBELISK_TOKEN_INVALID;

    state_after = OBELISK_STATE_START;

    rc = diminuto_alarm_install(!0);
    assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    assert(rc >= 0);

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    ticks_timer = ticks_frequency / HERTZ_TIMER;

    ticks_slack = diminuto_timer_periodic(ticks_timer);
    assert(ticks_slack == 0);

    risings = 0;
    fallings = 0;
    cycles = 0;

    armed = 0;
    synchronized = 0;
    acquired = 0;

    buffer = 0;

    /*
    ** Begin  work loop.
    */

    while (!0) {

        rc = pause();
        assert(rc == -1);

        if (diminuto_terminator_check()) {
            if (debug) { LOG("1 SIGTERM."); }
            break;
        }

        if (diminuto_hangup_check()) {
            DIMINUTO_LOG_INFORMATION("%s: hungup acquired=%d synchronized=%d armed=%d\n", program, acquired, synchronized, armed);
            synchronized = 0;
            if (debug) { LOG("1 SIGHUP."); }
        }

        if (!diminuto_alarm_check()) {
            continue;
        }
        if (verbose) { LOG("1 SIGALRM."); }

        /*
        ** Determine T input pin state.
        */

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);

        level_cooked = diminuto_cue_debounce(&cue, level_raw);

        /*
        ** Look for edge transitions and measure pulse duration.
        */

        edge = diminuto_cue_edge(&cue);

        switch (edge) {

        case DIMINUTO_CUE_EDGE_LOW:
            /* Do nothing. */
            break;

        case DIMINUTO_CUE_EDGE_RISING:
            risings += 1;
            milliseconds_pulse = milliseconds_cycle;
            if (debug) { LOG("3 RISING %dms.", milliseconds_pulse); }
            break;

        case DIMINUTO_CUE_EDGE_HIGH:
            milliseconds_pulse += milliseconds_cycle;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:
            fallings += 1;
            milliseconds_pulse += milliseconds_cycle;
            if (debug) { LOG("3 FALLING %dms.", milliseconds_pulse); }
            break;

        default:
            assert(edge != edge);
            milliseconds_pulse = 0;
            break;

        }

        /*
         * Try to detect a "stuck" GPIO pin, which could be a result of
         * a radio failure. What we really want is a timeout, but that's
         * harder than it sounds: our sense of time isn't nearly as good
         * as the radio transmitter that is synced to atomic clocks. Our
         * clock is sure to be running fast or slow. Mostly we resuire
         * that in a duration that we think is two seconds we see between
         * one and three pulses inclusive.
         */

        if ((++cycles) >= cycles_limit) {
            if (!acquired) {
                /* Do nothing. */
            } else if ((1 <= risings) && (risings <= 3)  && (1 <= fallings) && (fallings <= 3)) {
                /* Do nothing. */
            } else {
                acquired = 0;
                DIMINUTO_LOG_NOTICE("%s: lost %d %d\n", program, risings, fallings);
            }
            risings = 0;
            fallings = 0;
            cycles = 0;
        }

        /*
         * Wait for a complete pulse.
         */

        if (edge != DIMINUTO_CUE_EDGE_FALLING) {
            continue;
        }

        /*
        ** Classify pulse.
        */

        token = obelisk_tokenize(milliseconds_pulse);
        assert((0 <= token) && (token < countof(TOKEN)));

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_before = state_after;
        state_after = obelisk_parse(state_before, token, &field, &length, &leap, &buffer);
        assert((0 <= state_before) && (state_before < countof(STATE)));
        assert((0 <= state_after) && (state_after < countof(STATE)));

        if (debug) { LOG("6 PARSE %s %s %s %d %d %d 0x%llx.", STATE[state_before], TOKEN[token], STATE[state_after], field, length, leap, buffer); }

        /*
         * Detect an error that causes the state machine to return
         * to its start state.
         */

        if (!acquired) {
            /* Do nothing. */
        } else if (state_before == OBELISK_STATE_START) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_START) {
            /* Do nothing. */
        } else {
            acquired = 0;
            DIMINUTO_LOG_NOTICE("%s: lost %s %s\n", program, STATE[state_before], STATE[state_after]);
        }

        /*
         * Detect the beginning of the next minute so that we can
         * set the time if we so desire.
         */

        if (!set) {
            /* Do nothing. */
        } else if (!armed) {
            /* Do nothing. */
        } else if (synchronized) {
            /* Do nothing. */
        } else if (state_before != OBELISK_STATE_BEGIN) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_LEAP) {
            /* Do nothing. */
        } else {

            if ((rc = settimeofday(&value, (struct timezone *)0)) < 0) {
                perror("settimeodday");
            } else {

                synchronized = !0;

                ticks_now = diminuto_time_clock();
                assert(ticks_now >= 0);

                rc = diminuto_time_zulu(ticks_now, &year, &month, &day, &hour, &minute, &second, &fraction);
                assert(rc >= 0);

                DIMINUTO_LOG_NOTICE("%s: settimeofday %04d-%02d-%02dT%02d:%02d:%02d.%09llu+00:00\n",
                    program,
                    year, month, day,
                    hour, minute, second,
                    fraction / 1000
                );

            }

        }

        /*
         * Once we have a complete frame, decode it.
         */

        armed = 0;

        if (state_before != OBELISK_STATE_END) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_BEGIN) {
            /* Do nothing. */
        } else {
            
            obelisk_extract(&frame, buffer);

            if (debug) {
                LOG("7 FRAME 0x%016lld %d %d / %d %d %d T %d %d : %d %d - %d %d %d %d %d.",
                    buffer,
                    frame.year10, frame.year1,
                    frame.day100, frame.day10, frame.day1,
                    frame.hours10, frame.hours1,
                    frame.minutes10, frame.minutes1,
                    frame.dutonesign,
                    frame.dutone1,
                    frame.lyi,
                    frame.lsw,
                    frame.dst
                );
            }

            /*
             * Extract the binary coded digits from the frame and compute
             * a date and time. Validate the result.
             */

            rc = obelisk_validate(&time, &frame);
            if (rc < 0) {

                if (acquired) {
                    acquired = 0;
                    DIMINUTO_LOG_NOTICE("%s: lost %d\n", program, rc);
                }

            } else {

                if (!acquired) {
                    acquired = !0;
                    DIMINUTO_LOG_NOTICE("%s: acquired\n", program);
                }

                /*
                 * It took me a moment during testing to realize that this
                 * value will always be 59 seconds; at this point in the frame,
                 * we are always in the last second of the current minute. This
                 * is just for display purposes; we'll reset this to zero if we
                 * actually use it.
                 */

                time.tm_sec = 59;

                if (debug) {
                    assert((0 <= time.tm_wday) && (time.tm_wday < countof(DAY)));
                    LOG("7 TIME %d %04d-%02d-%02dT%02d:%02d:%02dZ %04d/%03d %s %s.",
                        rc,
                        time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                        time.tm_hour, time.tm_min, time.tm_sec,
                        time.tm_year + 1900, time.tm_yday + 1,
                        DAY[time.tm_wday],
                        time.tm_isdst ? "DST" : "!DST"
                    );
                }

                /*
                 * Derive the seconds since the POSIX Epoch that our time
                 * code represents. This will be time at the beginning of
                 * the next time frame, which will be at zero seconds past
                 * that minute (which we'll fix below).
                 */

                time.tm_sec = 0;
                value.tv_sec = mktime(&time);

                /*
                 * mktime(3) always assumes that the tm structure contains local
                 * time. So we have to adjust our value to account for the time
                 * zone of the host on which we are running. Similarly, we have
                 * to account for Daylight Saving Time in the local time zone.
                 */

                value.tv_sec -= timezone;
                if (!daylight) {
                    /* Do nothing. */
                } else if (!time.tm_isdst) {
                    /* Do nothing. */
                } else {
                    value.tv_sec += 3600;
                }
    
                /*
                 * Adjust the epoch seconds forward to the beginning of the
                 * next minute. That is when we'll set the system time (if
                 * we do so at all).
                 */

                value.tv_sec += 60;
    
                /*
                 * The microsecond offset accounts for the latency in the
                 * cue debouncer: two cycles of 10ms (10000usec) each.
                 */

                value.tv_usec = (2 * milliseconds_cycle) * 1000;

                if (debug) { LOG("7 EPOCH %ld.%06ld.", value.tv_sec, value.tv_usec); }

                armed = !0;

                /*
                 * If we think the system clock has been synchronized, then
                 * use it to determine the local time. If the local time is
                 * at the appropriate time, let the work loop set the time
                 * again.
                 */

                if (synchronized) {

                    ticks_now = diminuto_time_clock();
                    assert(ticks_now >= 0);

                    rc = diminuto_time_juliet(ticks_now, (int *)0, (int *)0, (int *)0, &hour, &minute, (int *)0, (diminuto_ticks_t *)0);
                    assert(rc >= 0);

                    if (hour != hour_juliet) {
                        /* Do nothing. */
                    } else if (minute != minute_juliet) {
                        /* Do nothing. */
                    } else {
                        synchronized = 0;
                        if (debug) { LOG("8 READY %02d:%02d:00J", hour, minute); }
                    }

                }

            }

        }

    }

    /*
    ** Release resources.
    */

    if (debug) { LOG("9 RELEASING."); }

    pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, pin_in_t);
    assert(pin_in_t_fp == (FILE *)0);

    pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, pin_out_p1);
    assert(pin_out_p1_fp == (FILE *)0);

    if (path != (char *)0) {
        (void)diminuto_lock_unlock(path);
    }

    /*
    ** Exit.
    */

    if (debug) { LOG("9 EXITING."); }

    return 0;
}
