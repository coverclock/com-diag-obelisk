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
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
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
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/obelisk/hazer.h"
#include "com/diag/obelisk/obelisk.h"

#define LOG(_FORMAT_, ...) do { if (debug) { fprintf(stderr, "%s: " _FORMAT_ "\n", program, ## __VA_ARGS__); } } while (0)

static const char RUN_PATH[] = "/var/run/wwvbtool.pid";
static const char NMEA_PATH[] = "-";
static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int PIN_OUT_PPS = 25; /* output, pulse per second , active high */
static const int HERTZ_DELAY = 2;
static const int HERTZ_TIMER = 100;
static const int HOUR_JULIET = 1;
static const int MINUTE_JULIET = 30;
static const int NICE_MINIMUM = -20;
static const int NICE_MAXIMUM = 19;
static const int NICE_NONE = -21;

static const const char * TOKEN[] = {
    "ZERO",     /* OBELISK_TOKEN_ZERO */
    "ONE",      /* OBELISK_TOKEN_ONE */
    "MARKER",   /* OBELISK_TOKEN_MARKER */
    "INVALID",  /* OBELISK_TOKEN_INVALID */
};

static const char * STATE[] = {
    "START",    /* OBELISK_STATE_START */
    "WAIT",     /* OBELISK_STATE_WAIT */
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
static int terminate = 0;
static int unlock = 0;
static int pps = 0;
static int nmea = 0;
static int hangup = 0;
static int pin_out_p1 = -1;
static int pin_in_t = -1;
static int pin_out_pps = -1;
static int unexport = 0;
static int background = 0;
static int set = 0;
static int hour_juliet = -1;
static int minute_juliet = -1;
static int nice_priority = 0;
static const char * run_path = (char *)0;
static char nmea_talker[sizeof("GP")] = { '\0', '\0', '\0' };
static const char * nmea_path = (char *)0;
static int serial_bitspersecond = DIMINUTO_SERIAL_BITSPERSECOND_NOMINAL;
static diminuto_serial_databits_t serial_databits = DIMINUTO_SERIAL_DATABITS_NOMINAL;
static diminuto_serial_paritybit_t serial_paritybit = DIMINUTO_SERIAL_PARITYBIT_NOMINAL;
static int serial_stopbits = DIMINUTO_SERIAL_STOPBITS_NOMINAL;
static int serial_modemcontrol = 0;
static int serial_xonxoff = 0;
static int serial_rtscts = 0;

static void usage(void)
{
    fprintf(stderr, "usage: %s [ -1 | -2 ] [ -7 | -8 ] [ -B BAUD ] [ -C NICE ] [ -H HOUR ] [ -L PATH ] [ -M MINUTE ] [ -N TALKER ] [ -O PATH ] [ -P PIN ] [ -S PIN ] [ -T PIN ] [ -b ] [ -c ] [ -d ] [ -e | -o ] [ -g ] [ -h ] [ -k ] [ -l ] [ -m ] [ -n ] [ -p ]  [ -r ] [ -s ] [ -u ] [ -v ] [ -x ]\n", program);
    fprintf(stderr, "       -1              Use one stop bit for OUTPUT (default).\n");
    fprintf(stderr, "       -2              Use two stop bits for OUTPUT.\n");
    fprintf(stderr, "       -7              Use seven data bits for OUTPUT.\n");
    fprintf(stderr, "       -8              Use eight data bits for OUTPUT (default).\n");
    fprintf(stderr, "       -B BAUD         Use BAUD bits per second for OUTPUT (%d).\n", serial_bitspersecond);
    fprintf(stderr, "       -C NICE         Set scheduling priority to NICE (%d..%d).\n", NICE_MINIMUM, NICE_MAXIMUM);
    fprintf(stderr, "       -H HOUR         Set time of day at HOUR local (%d).\n", hour_juliet);
    fprintf(stderr, "       -L PATH         Use PATH for lock file (\"%s\").\n", run_path);
    fprintf(stderr, "       -M MINUTE       Set time of day at MINUTE local (%d).\n", minute_juliet);
    fprintf(stderr, "       -N TALKER       Set NMEA TALKER (\"%s\").\n", nmea_talker);
    fprintf(stderr, "       -O OUTPUT       Write NMEA sentences to OUTPUT (\"%s\").\n", nmea_path);
    fprintf(stderr, "       -P PIN          Use P1 output GPIO PIN (%d).\n", pin_out_p1);
    fprintf(stderr, "       -S PIN          Use PPS output GPIO PIN (%d).\n", pin_out_pps);
    fprintf(stderr, "       -T PIN          Use T input GPIO PIN (%d).\n", pin_in_t);
    fprintf(stderr, "       -b              Daemonize into the background.\n");
    fprintf(stderr, "       -c              Use RTS/CTS for OUTPUT.\n");
    fprintf(stderr, "       -d              Display debug output.\n");
    fprintf(stderr, "       -e              Use even parity for OUTPUT.\n");
    fprintf(stderr, "       -g              Send SIGHUP to the PID in the lock file and exit.\n");
    fprintf(stderr, "       -h              Display help menu and exit.\n");
    fprintf(stderr, "       -k              Send SIGTERM to the PID in the lock file and exit.\n");
    fprintf(stderr, "       -l              Remove the lock file initially ignoring errors.\n");
    fprintf(stderr, "       -m              Use modem control for OUTPUT.\n");
    fprintf(stderr, "       -n              Generate NMEA output.\n");
    fprintf(stderr, "       -o              Use odd parity for OUTPUT.\n");
    fprintf(stderr, "       -p              Generate PPS output.\n");
    fprintf(stderr, "       -r              Reset device initially.\n");
    fprintf(stderr, "       -s              Set time of day when possible.\n");
    fprintf(stderr, "       -u              Unexport pins initially ignoring errors.\n");
    fprintf(stderr, "       -v              Display verbose output.\n");
    fprintf(stderr, "       -x              Use XON/XOFF for OUTPUT.\n");
}

int main(int argc, char ** argv)
{
    int rc = -1;
    pid_t pid = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_out_pps_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    FILE * nmea_out_fp = (FILE *)0;
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
    obelisk_buffer_t buffer = -1;
    obelisk_frame_t frame = { 0 };
    hazer_buffer_t sentence = { 0 };
    struct tm time = { 0 };
    struct tm * timep = (struct tm *)0;
    struct timeval value = { 0 };
    extern long timezone;
    extern int daylight;
    int field = -1;
    int length = -1;
    int leap = -1;
    int opt = -1;
    extern char * optarg;
    char * endptr = (char *)0;
    int error = -1;
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
    ssize_t limit = -1;
    uint8_t checksum = -1;
    char msb = -1;
    char lsb = -1;
    int fd = -1;

    diminuto_log_setmask();

    /*
     * Process command line options.
     */

    program = strrchr(argv[0], '/');
    program = (program == (const char *)0) ? argv[0] : program + 1;

    run_path = RUN_PATH;
    pin_out_p1 = PIN_OUT_P1;
    pin_out_pps = PIN_OUT_PPS;
    pin_in_t = PIN_IN_T;
    hour_juliet = HOUR_JULIET;
    minute_juliet = MINUTE_JULIET;
    strncpy(nmea_talker, HAZER_NMEA_RADIO_TALKER, sizeof(nmea_talker) - 1);
    nmea_path = NMEA_PATH;
    nice_priority = NICE_NONE;

    error = 0;

    while ((opt = getopt(argc, argv, "1278B:C:H:L:M:N:O:P:S:T:bcdeghklmonprsuvx")) >= 0) {

        switch (opt) {

        case '1':
            serial_stopbits = DIMINUTO_SERIAL_STOPBITS_1;
            break;

        case '2':
            serial_stopbits = DIMINUTO_SERIAL_STOPBITS_2;
            break;

        case '7':
            serial_databits = DIMINUTO_SERIAL_DATABITS_7;
            break;

        case '8':
            serial_databits = DIMINUTO_SERIAL_DATABITS_8;
            break;

        case 'e':
            serial_paritybit = DIMINUTO_SERIAL_PARITYBIT_EVEN;
            break;

        case 'o':
            serial_paritybit = DIMINUTO_SERIAL_PARITYBIT_ODD;
            break;

        case 'x':
            serial_xonxoff = !0;
            break;

        case 'c':
            serial_rtscts = !0;
            break;

        case 'm':
            serial_modemcontrol = !0;
            break;

        case 'B':
            serial_bitspersecond = strtoul(optarg, &endptr, 0);
            if ((*endptr != '\0') || (serial_bitspersecond < 0)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'C':
            nice_priority = strtoul(optarg, &endptr, 0);
            if ((*endptr != '\0') || (nice_priority < NICE_MINIMUM) || (nice_priority > NICE_MAXIMUM)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'H':
            hour_juliet = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (hour_juliet < 0) || (hour_juliet > 23)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'L':
            run_path = optarg;
            break;

        case 'M':
            minute_juliet = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (minute_juliet < 0) || (minute_juliet > 59)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'N':
            if (strlen(optarg) == (sizeof(nmea_talker) - 1)) {
                strncpy(nmea_talker, optarg, sizeof(nmea_talker) - 1);
            } else {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'O':
            nmea_path = optarg;
            break;

        case 'P':
            pin_out_p1 = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_p1 < 0)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'S':
            pin_out_pps = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_pps < 0)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'T':
            pin_in_t = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_in_t < 0)) {
                errno = EINVAL;
                perror(optarg);
                error = !0;
            }
            break;

        case 'b':
            background = !0;
            break;

        case 'd':
            debug = !0;
            break;

        case 'g':
            hangup = !0;
            break;

        case 'h':
            usage();
            return 0;
            break;

        case 'k':
            terminate = !0;
            break;

        case 'l':
            unlock = !0;
            break;

        case 'n':
            nmea = !0;
            break;

        case 'p':
            pps = !0;
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
            error = !0;
            break;

        }

    }

    if (error) {
        return 1;
    }

    LOG("LOCK \"%s\".", run_path);

    if (unlock) {

        /*
         * Remove the lock file.
         */

        LOG("UNLOCKING.");
        (void)diminuto_lock_unlock(run_path);
    }

    if (terminate) {

        /*
         * Send a SIGTERM to the process whose PID is in the lock file
         * and then exit.
         */

        pid = diminuto_lock_check(run_path);
        if (pid < 0) { return 2; }
        LOG("TERMINATING %d.", pid);
        rc = kill(pid, SIGTERM);
        if (rc < 0) {
            perror("kill");
            return 2;
        }
        return 0;

    } else if (hangup) {

        /*
         * Send a SIGHUP to the process whose PID is in the lock file
         * and then exit.
         */

        pid = diminuto_lock_check(run_path);
        if (pid < 0) { return 2; }
        LOG("HANGINGUP %d.", pid);
        rc = kill(pid, SIGHUP);
        if (rc < 0) {
            perror("kill");
            return 2;
        }
        return 0;

    } else if (background) {

        /*
         * Daemonize if so instructed. A lock file containing the daemon's
         * process identifier will be left in /var/lock.
         */

        LOG("DAEMONIZING.");
        rc = diminuto_daemon(program, run_path);
        if (rc < 0) { return 2; }

    } else {

        /*
         * Create a lock file containing our PID.
         */

        LOG("LOCKING.");
        rc = diminuto_lock_lock(run_path);
        if (rc < 0) { return 2; }

    }

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

    pid = getpid();
    DIMINUTO_LOG_INFORMATION("%s: running pid=%d.\n", program, getpid());

    if (nice_priority != NICE_NONE) {
        LOG("PRIORITY %d.\n", nice_priority);
        rc = setpriority(PRIO_PROCESS, 0, nice_priority);
        if (rc < 0) { perror("setpriority"); }
        assert(rc >= 0);
    }

    /*
    ** Configure P1 output and T input pins.
    */

    if (unexport) {

        LOG("UNEXPORT.");

        if (reset) {
            (void)diminuto_pin_unexport(pin_out_p1);
        }

        (void)diminuto_pin_unexport(pin_in_t);

        if (pps) {
            (void)diminuto_pin_unexport(pin_out_pps);
        }

    }

    LOG("EXPORT.");

    if (reset) {
        pin_out_p1_fp = diminuto_pin_output(pin_out_p1);
        assert(pin_out_p1_fp != (FILE *)0);
    }

    if (pps) {
        pin_out_pps_fp = diminuto_pin_output(pin_out_pps);
        assert(pin_out_p1_fp != (FILE *)0);
    }

    pin_in_t_fp = diminuto_pin_input(pin_in_t);
    assert(pin_in_t_fp != (FILE *)0);

    /*
    ** Toggle P1 output pin (active low) if requested.
    */

    if (pps) {
        rc = diminuto_pin_clear(pin_out_pps_fp);
        assert(rc == 0);
    }

    ticks_frequency = diminuto_frequency();
    assert(ticks_frequency > 0);

    if (reset) {

        LOG("RESET.");

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
     * Open NMEA output file if requested. We open for reading and
     * writing ("a+") so that we don't get a SIGPIPE in the event
     * the user has specified a FIFO and the reader (e.g. gpsd)
     * hasn't opened its end for reading yet. That's also why
     * we delay the fopen(3) until after the GPIO radio initialization
     * above.
     */

    if (nmea) {
        LOG("NMEA \"%s\".", nmea_path);
        if (strcmp(nmea_path, NMEA_PATH) == 0) {
            nmea_out_fp = stdout;
        } else if ((nmea_out_fp = fopen(nmea_path, "a+")) == (FILE *)0) {
            perror(nmea_path);
            assert(nmea_out_fp != (FILE *)0);
        } else if ((fd = fileno(nmea_out_fp)) < 0) {
            perror(nmea_path);
            assert(fd >= 0);
        } else if ((rc = isfdtype(fd, S_IFCHR)) < 0) {
            perror(nmea_path);
            assert(rc >= 0);
        } else if (rc) {
            rc = diminuto_serial_set(fd, serial_bitspersecond, serial_databits, serial_paritybit, serial_stopbits, serial_modemcontrol, serial_xonxoff, serial_rtscts);
            assert(rc >= 0);
            rc = diminuto_serial_raw(fd);
            assert(rc >= 0);
        } else {
            /* Do nothing. */
        }
    }

    /*
    ** Initialize state.
    */

    LOG("INITIALIZE.");

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
            DIMINUTO_LOG_INFORMATION("%s: terminated.\n", program);
            break;
        }

        if (diminuto_hangup_check()) {
            DIMINUTO_LOG_INFORMATION("%s: hungup acquired=%d synchronized=%d armed=%d risings=%d fallings=%d cycles=%d.\n", program, acquired, synchronized, armed, risings, fallings, cycles);
            synchronized = 0;
        }

        if (!diminuto_alarm_check()) {
            continue;
        }

        if (verbose) { LOG("SIGALRM."); }

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
            if (!pps) {
                /* Do nothing. */
            } else if (!acquired) {
                /* Do nothing. */
            } else {
                rc = diminuto_pin_set(pin_out_pps_fp);
                assert(rc >= 0);
            }
            risings += 1;
            milliseconds_pulse = milliseconds_cycle;
            LOG("RISING %dms.", milliseconds_pulse);
            break;

        case DIMINUTO_CUE_EDGE_HIGH:
            milliseconds_pulse += milliseconds_cycle;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:
            if (pps) {
                rc = diminuto_pin_clear(pin_out_pps_fp);
                assert(rc >= 0);
            }
            fallings += 1;
            milliseconds_pulse += milliseconds_cycle;
            LOG("FALLING %dms.", milliseconds_pulse);
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
                DIMINUTO_LOG_NOTICE("%s: lost risings=%d fallings=%d.\n", program, risings, fallings);
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
        assert((OBELISK_TOKEN_ZERO <= token) && (token <= OBELISK_TOKEN_INVALID));
        assert((0 <= token) && (token < countof(TOKEN)));

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_before = state_after;
        assert((OBELISK_STATE_START <= state_before) && (state_before <= OBELISK_STATE_END));
        assert((0 <= state_before) && (state_before < countof(STATE)));

        state_after = obelisk_parse(state_before, token, &field, &length, &leap, &buffer);
        assert((OBELISK_STATE_START <= state_after) && (state_after <= OBELISK_STATE_END));
        assert((0 <= state_after) && (state_after < countof(STATE)));

        LOG("PARSE %s %s %s %d %d %d 0x%llx.", STATE[state_before], TOKEN[token], STATE[state_after], field, length, leap, buffer);

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
            DIMINUTO_LOG_NOTICE("%s: lost state_before=%s state_after=%s.\n", program, STATE[state_before], STATE[state_after]);
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

                DIMINUTO_LOG_NOTICE("%s: now zulu=%04d-%02d-%02dT%02d:%02d:%02d.%09llu.\n",
                    program,
                    year, month, day,
                    hour, minute, second,
                    fraction / 1000
                );

            }

        }

        /*
         * Once we have a complete frame, extract it from the buffer.
         */

        armed = 0;

        if (state_before != OBELISK_STATE_END) {
            /* Do nothing. */
        } else if (state_after != OBELISK_STATE_BEGIN) {
            /* Do nothing. */
        } else {
            
            obelisk_extract(&frame, buffer);

            LOG("FRAME 0x%016lld %d %d %d %d %d %d %d %d %d %d %d %d %d %d.",
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

            /*
             * Decode the binary coded digits from the frame and compute
             * a POSIX-compatible date and time. Validate the result.
             */

            rc = obelisk_validate(&frame);
            if (rc >= 0) {
                rc = obelisk_decode(&time, &frame);
                if (rc >= 0) {
                    rc = obelisk_revalidate(&time);
                }
            }

            if (rc < 0) {

                if (acquired) {
                    acquired = 0;
                    DIMINUTO_LOG_NOTICE("%s: lost rc=%d.\n", program, rc);
                } else if (!synchronized) {
                    DIMINUTO_LOG_NOTICE("%s: corrupt rc=%d.\n", program, rc);
                } else {
                    /* Dothing. */
                }

            } else {

                if (!acquired) {
                    acquired = !0;
                    DIMINUTO_LOG_NOTICE("%s: acquired.\n", program);
                }

                /*
                 * Seconds will always be 59 at this point in the frame
                 * unless a leap second has been inserted.
                 */

                time.tm_sec = leap ? 60 : 59;

                assert((0 <= time.tm_wday) && (time.tm_wday < countof(DAY)));
                LOG("TIME %d %04d-%02d-%02dT%02d:%02d:%02dZ %04d/%03d %s %s.",
                    rc,
                    time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec,
                    time.tm_year + 1900, time.tm_yday + 1,
                    DAY[time.tm_wday],
                    time.tm_isdst ? "DST" : "!DST"
                );

                /*
                 * Logging the received one per hour doesn't overrun
                 * the logging system. And doing so at the 59th minute
                 * captures the leap second at :59:60 if it occurs.
                 */

                if (!synchronized || (time.tm_min == 59)) {
                    DIMINUTO_LOG_INFORMATION("%s: time zulu=%04d-%02d-%02dT%02d:%02d:%02d julian=%04d/%03d day=%s dst=%c dUT1=%c0.%d lyi=%d lsw=%d.",
                        program,
                        time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                        time.tm_hour, time.tm_min, time.tm_sec,
                        time.tm_year + 1900, time.tm_yday + 1,
                        DAY[time.tm_wday],
                        (frame.dst == OBELISK_DST_OFF) ? '-'
                            : (frame.dst == OBELISK_DST_ENDS) ? '<'
                            : (frame.dst == OBELISK_DST_BEGINS) ? '>'
                            : (frame.dst == OBELISK_DST_ON) ? '+'
                            : '?',
                        (frame.dutonesign == OBELISK_SIGN_NEGATIVE) ? '-'
                            : (frame.dutonesign == OBELISK_SIGN_POSITIVE) ? '+'
                            : '?',
                        frame.dutone1,
                        frame.lyi,
                        frame.lsw
                    );
                }

                /*
                 * Derive the seconds since the POSIX Epoch that our time
                 * code represents. This will be time at the beginning of
                 * the next time frame, which will be at zero seconds past
                 * that subsequent minute (which we'll fix below).
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

                LOG("EPOCH %ld.%06ld.", value.tv_sec, value.tv_usec);

                armed = !0;

                /*
                 * Generate NMEA output if so requested.
                 */

                if (nmea) {
 
                    timep = gmtime_r(&value.tv_sec, &time);
                    if (timep != &time) {
                        perror("gmtime_r");
                    } else {

                        rc = snprintf(
                            sentence, sizeof(sentence) - 1,
                            "%c%2.2s%3.3s,%02d%02d%02d.%02d,A,,,,,,,%02d%02d%02d,,,D%c",
                            HAZER_STIMULUS_START,
                            nmea_talker,
                            HAZER_NMEA_GPS_MESSAGE_RMC,
                            time.tm_hour,
                            time.tm_min,
                            time.tm_sec,
                            value.tv_usec / (1000000000 / 100),
                            time.tm_mday,
                            time.tm_mon + 1,
                            (time.tm_year + 1900) % 100,
                            HAZER_STIMULUS_CHECKSUM
                        );
                        assert(rc < (sizeof(sentence) - 1));
                        sentence[sizeof(sentence) - 1] = '\0';

                        checksum = hazer_checksum(sentence, sizeof(sentence));
                        rc = hazer_checksum2characters(checksum, &msb, &lsb);
                        assert(rc >= 0);

                        LOG("NMEA %s%c%c\\r\\n", sentence, msb, lsb);

                        fprintf(nmea_out_fp, "%s%c%c\r\n", sentence, msb, lsb);
                        fflush(nmea_out_fp);

                    }

                }

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
                        LOG("READY %02d:%02d:00J", hour, minute);
                    }

                }

            }

        }

    }

    /*
    ** Release resources.
    */

    LOG("RELEASING.");

    if (pin_in_t_fp != (FILE *)0) {
        pin_in_t_fp = diminuto_pin_unused(pin_in_t_fp, pin_in_t);
        assert(pin_in_t_fp == (FILE *)0);
    }

    if (pin_out_p1_fp != (FILE *)0) {
        pin_out_p1_fp = diminuto_pin_unused(pin_out_p1_fp, pin_out_p1);
        assert(pin_out_p1_fp == (FILE *)0);
    }

    if (pin_out_pps_fp != (FILE *)0) {
        pin_out_pps_fp = diminuto_pin_unused(pin_out_pps_fp, pin_out_pps);
        assert(pin_out_pps_fp == (FILE *)0);
    }

    if (nmea_out_fp != (FILE *)0) {
        rc = fclose(nmea_out_fp);
        if (rc != 0) { perror(nmea_path); }
        assert(rc == 0);
    }

    (void)diminuto_lock_unlock(run_path);

    /*
    ** Exit.
    */

    DIMINUTO_LOG_INFORMATION("%s: exiting.\n", program);

    return 0;
}
