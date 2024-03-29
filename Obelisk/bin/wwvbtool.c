/* vim: set ts=4 expandtab shiftwidth=4: */

/**
 * @file
 *
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
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
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_lock.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/hazer/hazer.h"
#include "com/diag/obelisk/obelisk.h"
#include "com/diag/obelisk/wwvbtool.h"

#define LOG(_FORMAT_, ...) do { if (debug) { fprintf(stderr, "%s: " _FORMAT_ "\n", program, ## __VA_ARGS__); } } while (0)

static const char RUN_PATH[] = "/var/run/wwvbtool.pid";
static const char NMEA_PATH[] = "-";
static const int PIN_OUT_P1 = 23; /* output, radio enable, active low. */
static const int PIN_IN_T = 24; /* input, modulated pulse, active high */
static const int PIN_OUT_PPS = 25; /* output, pulse per second , active high */
static const int HERTZ_DELAY = 10;
static const int HERTZ_TIMER = 100;
static const int HOUR_JULIET = 1;
static const int MINUTE_JULIET = 30;
static const int NICE_MINIMUM = -20;
static const int NICE_MAXIMUM = 19;
static const int NICE_NONE = -21;

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
static int set_initially = 0;
static int set_daily = 0;
static int set_leap = 0;
static int hour_juliet = -1;
static int minute_juliet = -1;
static int nice_priority = 0;
static const char * run_path = (char *)0;
static char nmea_talker[sizeof("GP")] = { '\0', '\0', '\0' };
static const char * nmea_path = (char *)0;
static const char * nmea_endpoint = (char *)0;
static int serial_bitspersecond = DIMINUTO_SERIAL_BITSPERSECOND_NOMINAL;
static diminuto_serial_databits_t serial_databits = DIMINUTO_SERIAL_DATABITS_NOMINAL;
static diminuto_serial_paritybit_t serial_paritybit = DIMINUTO_SERIAL_PARITYBIT_NOMINAL;
static int serial_stopbits = DIMINUTO_SERIAL_STOPBITS_NOMINAL;
static int serial_modemcontrol = 0;
static int serial_xonxoff = 0;
static int serial_rtscts = 0;

static void usage(void)
{
    fprintf(stderr, "usage: %s [ -1 | -2 ] [ -7 | -8 ] [ -B BAUD ] [ -C NICE ] [ -H HOUR ] [ -L PATH ] [ -M MINUTE ] [ -N TALKER ] [ -O PATH ] [ -P PIN ] [ -S PIN ] [ -T PIN ] [ -U ENDPOINT ] [ -a ] [ -b ] [ -c ] [ -d ] [ -e | -o ] [ -g ] [ -h ] [ -i ] [ -k ] [ -l ] [ -m ] [ -n ] [ -p ]  [ -r ] [ -s ] [ -u ] [ -v ] [ -x ]\n", program);
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
    fprintf(stderr, "       -U ENDPOINT     Write NMEA sentences to UDP ENDPOINT.\n");
    fprintf(stderr, "       -a              Set time of day when leap second occurs.\n");
    fprintf(stderr, "       -b              Daemonize into the background.\n");
    fprintf(stderr, "       -c              Use RTS/CTS for OUTPUT.\n");
    fprintf(stderr, "       -d              Display debug output.\n");
    fprintf(stderr, "       -e              Use even parity for OUTPUT.\n");
    fprintf(stderr, "       -g              Send SIGHUP to the PID in the lock file and exit.\n");
    fprintf(stderr, "       -h              Display help menu and exit.\n");
    fprintf(stderr, "       -i              Set time of day initially when possible.\n");
    fprintf(stderr, "       -k              Send SIGTERM to the PID in the lock file and exit.\n");
    fprintf(stderr, "       -l              Remove the lock file initially ignoring errors.\n");
    fprintf(stderr, "       -m              Use modem control for OUTPUT.\n");
    fprintf(stderr, "       -n              Generate NMEA output.\n");
    fprintf(stderr, "       -o              Use odd parity for OUTPUT.\n");
    fprintf(stderr, "       -p              Generate PPS output.\n");
    fprintf(stderr, "       -r              Reset device initially.\n");
    fprintf(stderr, "       -s              Set time of day initially and also daily when possible.\n");
    fprintf(stderr, "       -u              Unexport pins initially ignoring errors.\n");
    fprintf(stderr, "       -v              Display verbose output.\n");
    fprintf(stderr, "       -x              Use XON/XOFF for OUTPUT.\n");
}

static void emit(FILE *fp, const char * bb)
{
    size_t current = 0;
    int end = 0;

    while (*bb != '\0') {
        diminuto_phex_emit(fp, *(bb++), ~(size_t)0, 0, 0, 0, &current, &end, 0);
    }
}


int main(int argc, char ** argv)
{
    int rc = -1;
    pid_t pid = -1;
    FILE * pin_out_p1_fp = (FILE *)0;
    FILE * pin_out_pps_fp = (FILE *)0;
    FILE * pin_in_t_fp = (FILE *)0;
    FILE * nmea_out_fp = (FILE *)0;
    diminuto_ipc_endpoint_t nmea_out_endpoint = { 0 };
    int nmea_out_sock4 = -1;
    int nmea_out_sock6 = -1;
    diminuto_sticks_t ticks_frequency = -1;
    diminuto_ticks_t ticks_delay = -1;
    diminuto_ticks_t ticks_timer = -1;
    diminuto_sticks_t ticks_slack = -1;
    diminuto_sticks_t ticks_now = -1;
    diminuto_sticks_t ticks_begin = -1;
    diminuto_sticks_t ticks_end = -1;
    diminuto_cue_state_t cue = { 0 };
    diminuto_cue_edge_t edge = (diminuto_cue_edge_t)-1;
    int level_raw = -1;
    int level_cooked = -1;
    int level_old = -1;
    int initialized = -1;
    int milliseconds_cycle = -1;
    int milliseconds_pulse = -1;
    int cycles_limit = -1;
    obelisk_token_t token = (obelisk_token_t)-1;
    obelisk_state_t state = (obelisk_state_t)-1;
    obelisk_state_t state_old = (obelisk_state_t)-1;
    obelisk_event_t event = (obelisk_event_t)-1;
    obelisk_buffer_t buffer = -1;
    obelisk_frame_t frame = { 0 };
    hazer_buffer_t sentence = { 0 };
    struct tm time = { 0 };
    struct tm * timep = (struct tm *)0;
    struct timeval epoch = { 0 };
    extern long timezone;
    extern int daylight;
    int field = -1;
    int length = -1;
    int opt = -1;
    extern char * optarg;
    char * endptr = (char *)0;
    int error = -1;
    int armed = -1;
    int disciplined = -1;
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
    int fd = -1;
    float dut1 = 0.0;
    diminuto_ipv4_t address4 = 0;
    diminuto_ipv6_t address6 = { 0 };
    char printable[sizeof("XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX")];
    diminuto_port_t port = 0;

    assert(sizeof(obelisk_buffer_t) == sizeof(uint64_t));
    assert(sizeof(obelisk_frame_t) == sizeof(uint64_t));

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
    strncpy(nmea_talker, HAZER_TALKER_NAME[HAZER_TALKER_RADIO], sizeof(nmea_talker) - 1);
    nmea_path = NMEA_PATH;
    nice_priority = NICE_NONE;

    error = 0;

    while ((opt = getopt(argc, argv, "1278B:C:H:L:M:N:O:P:S:T:U:abcdeghiklmonprsuvx")) >= 0) {

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
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'C':
            nice_priority = strtoul(optarg, &endptr, 0);
            if ((*endptr != '\0') || (nice_priority < NICE_MINIMUM) || (nice_priority > NICE_MAXIMUM)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'H':
            hour_juliet = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (hour_juliet < 0) || (hour_juliet > 23)) {
                errno = EINVAL;
                diminuto_perror(optarg);
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
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'N':
            if (strlen(optarg) == (sizeof(nmea_talker) - 1)) {
                strncpy(nmea_talker, optarg, sizeof(nmea_talker) - 1);
            } else {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'O':
            nmea_path = optarg;
            nmea_endpoint = (const char *)0;
            break;

        case 'P':
            pin_out_p1 = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_p1 < 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'S':
            pin_out_pps = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_out_pps < 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'T':
            pin_in_t = strtol(optarg, &endptr, 0);
            if ((*endptr != '\0') || (pin_in_t < 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;

        case 'U':
            nmea_endpoint = optarg;
            nmea_path = (const char *)0;
            break;

        case 'a':
            set_leap = !0;
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

        case 'i':
            set_initially = !0;
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
            set_daily = !0;
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

    LOG("LOCKPATH \"%s\".", run_path);

    if (unlock) {

        /*
         * Remove the lock file.
         */

        LOG("UNLOCKING.");
        (void)diminuto_lock_unlock(run_path);
    }

    /*
     * Figure out what mode we're running in.
     */

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
            diminuto_perror("kill");
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
            diminuto_perror("kill");
            return 2;
        }
        return 0;

    } else if (background) {

        /*
         * Daemonize if so instructed. A lock file containing the daemon's
         * process identifier will be left in /var/lock. Continue running
         * as the background child process.
         */

        LOG("PRELOCKING.");
        rc = diminuto_lock_prelock(run_path);
        if (rc < 0) { return 2; }

        LOG("DAEMONIZING.");
        rc = diminuto_daemon(program);
        if (rc < 0) {
            LOG("UNLOCKING.");
            (void)diminuto_lock_unlock(run_path);
            return 2;
        }

        LOG("POSTLOCKING.");
        rc = diminuto_lock_postlock(run_path);
        if (rc < 0) {
            LOG("UNLOCKING.");
            (void)diminuto_lock_unlock(run_path);
            return 2;
        }

    } else {

        /*
         * Create a lock file containing our PID. Continue running as
         * the original foreground process.
         */

        LOG("LOCKING.");
        rc = diminuto_lock_lock(run_path);
        if (rc < 0) { return 2; }

    }

    pid = getpid();
    DIMINUTO_LOG_NOTICE("%s: running pid=%d.\n", program, getpid());

    if (nice_priority != NICE_NONE) {
        LOG("PRIORITY %d.\n", nice_priority);
        rc = setpriority(PRIO_PROCESS, 0, nice_priority);
        if (rc < 0) { diminuto_perror("setpriority"); }
        assert(rc >= 0);
    }

    /*
    ** Configure P1 and PPS output pins and T input pin.
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

    pin_in_t_fp = diminuto_pin_input(pin_in_t);
    assert(pin_in_t_fp != (FILE *)0);

    if (pps) {
        pin_out_pps_fp = diminuto_pin_output(pin_out_pps);
        assert(pin_out_p1_fp != (FILE *)0);
    }

    /*
     * Initially clear PPS output pin.
     */

    if (pps) {
        rc = diminuto_pin_clear(pin_out_pps_fp);
        assert(rc == 0);
    }

    /*
    * Toggle P1 output pin (active low) to restart radio
    * receiver.
    */

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

    }

    /*
     * Open NMEA output file if requested. We open for reading and
     * writing ("a+") so that we don't get a SIGPIPE in the event
     * the user has specified a FIFO and the reader (e.g. gpsd)
     * hasn't opened its end for reading yet. That's also why
     * we delay the fopen(3) until after the GPIO radio initialization
     * above.
     */

    if (!nmea) {

        /* Do nothing. */

    } else if (nmea_path != (const char *)0) {

        LOG("NMEAPATH \"%s\".", nmea_path);

        if (strcmp(nmea_path, NMEA_PATH) == 0) {
            nmea_out_fp = stdout;
        } else if ((nmea_out_fp = fopen(nmea_path, "a+")) == (FILE *)0) {
            diminuto_perror(nmea_path);
            assert(nmea_out_fp != (FILE *)0);
        } else if ((fd = fileno(nmea_out_fp)) < 0) {
            diminuto_perror(nmea_path);
            assert(fd >= 0);
        } else if ((rc = isfdtype(fd, S_IFCHR)) < 0) {
            diminuto_perror(nmea_path);
            assert(rc >= 0);
        } else if (rc) {
            rc = diminuto_serial_set(fd, serial_bitspersecond, serial_databits, serial_paritybit, serial_stopbits, serial_modemcontrol, serial_xonxoff, serial_rtscts);
            assert(rc >= 0);
            rc = diminuto_serial_raw(fd);
            assert(rc >= 0);
        } else {
            /* Do nothing. */
        }

    } else if (nmea_endpoint != (const char *)0) {

        LOG("NMEAENDPOINT \"%s\".", nmea_endpoint);

        if ((rc = diminuto_ipc_endpoint(nmea_endpoint, &nmea_out_endpoint)) < 0) {
            /* Do nothing. */
        } else if (nmea_out_endpoint.udp <= 0) {
            /* Do nothing. */
        } else if (!diminuto_ipc6_is_unspecified(&nmea_out_endpoint.ipv6)) {
            nmea_out_sock6 = diminuto_ipc6_datagram_peer(0);
        } else if (!diminuto_ipc4_is_unspecified(&nmea_out_endpoint.ipv4)) {
            nmea_out_sock4 = diminuto_ipc4_datagram_peer(0);
        } else {
            /* Do nothing. */
        }

        if (nmea_out_sock6 >= 0) {
            diminuto_ipc6_nearend(nmea_out_sock6, &address6, &port);
            diminuto_ipc6_address2string(address6, printable, sizeof(printable));
            LOG("NMEANEAREND [%s]:%d.", printable, port);
            diminuto_ipc6_address2string(nmea_out_endpoint.ipv6, printable, sizeof(printable));
            LOG("NMEAFARREND [%s]:%d.", printable, nmea_out_endpoint.udp);
        } else if (nmea_out_sock4 >= 0) {
            diminuto_ipc4_nearend(nmea_out_sock4, &address4, &port);
            diminuto_ipc4_address2string(address4, printable, sizeof(printable));
            LOG("NMEANEAREND %s:%d.", printable, port);
            diminuto_ipc4_address2string(nmea_out_endpoint.ipv4, printable, sizeof(printable));
            LOG("NMEAFARREND %s:%d.", printable, nmea_out_endpoint.udp);
        } else {
            errno = EINVAL;
            diminuto_perror(nmea_endpoint);
            assert((nmea_out_sock6 >= 0) || (nmea_out_sock4 >= 0));
        }

    } else {

        /* SHould be impossible */

        nmea = 0;

    }

    /*
    ** Initialize state.
    */

    LOG("INITIALIZE.");

    tzset();

    milliseconds_pulse = 0;

    milliseconds_cycle  = 1000 / HERTZ_TIMER;

    cycles_limit = HERTZ_TIMER * 2;

    level_raw = OBELISK_LEVEL_ZERO;

    token = OBELISK_TOKEN_INVALID;
    state = OBELISK_STATE_START;

    rc = diminuto_alarm_install(!0);
    assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    assert(rc >= 0);

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    rc = diminuto_interrupter_install(0);
    assert(rc >= 0);

    ticks_timer = ticks_frequency / HERTZ_TIMER;
    if (ticks_timer == 0) {
    	ticks_timer = 1;
    }

    LOG("PERIODIC %lldticks.", (long long int)ticks_timer);

    ticks_slack = diminuto_timer_periodic(ticks_timer);
    assert(ticks_slack == 0);

    risings = 0;
    fallings = 0;
    cycles = 0;

    initialized = 0;
    synchronized = !0;
    acquired = 0;
    armed = 0;
    disciplined = 0;

    buffer = 0;

    /*
    ** Begin  work loop.
    */

    while (!0) {

    	/*
    	 * We wait for a timer interrupt. I implemented this as polling, but
    	 * have thought a lot about using the interrupt GPIO feature, supported
    	 * in the Raspberry Pi HW, the Raspbian SW, and by the Diminuto library.
    	 * But having lived with this for a while, I see a *lot* of noise on
    	 * the GPIO line caused by EMF on the AM band. This can be caused, in my
    	 * home office, by lots of stuff, most notably the nearby electric
    	 * pencil sharpener. In any case, it's very susceptible to EMF, and
    	 * I am concerned that it could overwhelm the Pi/Raspbian with
    	 * interrupts. This need for SW debouncing also limits the period that
    	 * we can poll the GPIO line. (If you want a really good inexpensive
    	 * NTP time source, you should be using GPS instead of WWVB, anyway.
    	 * It's actually much simpler, too. Note that the very low power CDMA
    	 * transmissions of GPS can be easily jammed, as well.)
    	 */

        rc = pause();
        assert(rc == -1);

        /*
         * Check for SIGTERM and if seen leave work loop.
         */

        if (diminuto_terminator_check()) {
            DIMINUTO_LOG_NOTICE("%s: terminated.\n", program);
            break;
        }

        /*
         * Check for SIGINT and if seen leave work loop.
         */

        if (diminuto_interrupter_check()) {
            DIMINUTO_LOG_NOTICE("%s: interrupted.\n", program);
            break;
        }

        /*
         * Check for SIGALRM and if NOT seen we're done.
         */

        if (!diminuto_alarm_check()) {
            continue;
        }

        if (verbose) { LOG("SIGALRM."); }

        /*
         * Poll T input pin state and submit to the debouncer. We also keep
         * track of the raw undebounced change and remember when it occurred
         * so we can compute the latency later.
         */

        if (initialized) {
            level_old = level_raw;
        }

        level_raw = diminuto_pin_get(pin_in_t_fp);
        assert(level_raw >= 0);
        level_raw = !!level_raw;

        if (!initialized) {
            level_old = level_raw;
            diminuto_cue_init(&cue, level_raw);
            initialized = !0;
        }

        if (level_raw == level_old) {
            /* Do nothing. */
        } else if (level_raw) {
            ticks_begin = diminuto_time_elapsed();
            assert(ticks_begin >= 0);
        } else {
            /* Do nothing. */
        }

        level_cooked = diminuto_cue_debounce(&cue, level_raw);

        /*
         * Look for edge transitions and measure pulse duration.
         */

        edge = diminuto_cue_edge(&cue);

        switch (edge) {

        case DIMINUTO_CUE_EDGE_LOW:
            /* Do nothing. */
            break;

        case DIMINUTO_CUE_EDGE_RISING:

            /*
             * Take PPS high on output pin. As a side effect, this
             * mimics the duration of the T pin - smoothed by our
             * sampling rate of 100Hz - and so can be used to
             * measure the modulationed pulses from the radio
             * receiver. Unfortunately, this will exhibit any scheduling
             * jitter in our sampling timer.
             */

            if (!pps) {
                /* Do nothing. */
            } else if (!synchronized) {
                /* Do nothing. */
            } else {
                rc = diminuto_pin_set(pin_out_pps_fp);
                assert(rc >= 0);
            }

            /*
             * Advance the epoch second by one second. Each pulse indicates
             * the start of the next second. This has the useful side effect
             * of keeping the epoch updated in case we want to set the time,
             * and also in the event a leap second was inserted. We also
             * compute the debounce latency, which may not be useful if it's
             * less than a hundredth of a second, the resolution of the NMEA
             * timestamp.
             */

            if (acquired) {
                epoch.tv_sec += 1;
            	ticks_end = diminuto_time_elapsed();
            	assert(ticks_end >= 0);
                epoch.tv_usec = diminuto_frequency_ticks2units(ticks_end - ticks_begin, 1000000);
                LOG("TOTAL %ld.%06lds.", epoch.tv_sec, epoch.tv_usec);
            }

            /*
             * Generate a synthesized NMEA RMC timestamp. Since we
             * do this as the rise of the T pulse, we generate a
             * timestamp every second, phaselocked - subject to measured
             * latency in the debouncer - to WWVB.
             */

            if (!nmea) {
                /* Do nothing. */
            } else if (!acquired) {
                /* Do nothing. */
            } else {
                timep = gmtime_r(&epoch.tv_sec, &time);
                assert(timep == &time);
                rc = snprintf(
                    sentence, sizeof(sentence) - 1,
                    "%c%2.2s%3.3s,%02d%02d%02d.%02ld,A,,,,,,,%02d%02d%02d,,,D%cXX\r\n",
                    HAZER_STIMULUS_START,
                    nmea_talker,
                    HAZER_NMEA_SENTENCE_RMC,
                    time.tm_hour,
                    time.tm_min,
                    time.tm_sec,
                    epoch.tv_usec / (1000000 / 100), /* Round down. */
                    time.tm_mday,
                    time.tm_mon + 1,
                    (time.tm_year + 1900) % 100,
                    HAZER_STIMULUS_CHECKSUM
                );
                assert(rc < (sizeof(sentence) - 1));
                sentence[sizeof(sentence) - 1] = '\0';
                assert(sentence[rc - 4] == 'X');
                assert(sentence[rc - 3] == 'X');
                hazer_checksum_buffer(sentence, sizeof(sentence), &sentence[rc - 4], &sentence[rc - 3]);
                assert(rc >= 0);
                if (debug) {
                    fprintf(stderr, "%s: NMEA \"", program);
                    emit(stderr, sentence);
                    fputs("\".\n", stderr);
                }
                if (nmea_out_fp != (FILE *)0) {
                    fputs(sentence, nmea_out_fp);
                    fflush(nmea_out_fp);
                } else if (nmea_out_sock6 >= 0) {
                    rc = diminuto_ipc6_datagram_send(nmea_out_sock6, sentence, strlen(sentence), nmea_out_endpoint.ipv6, nmea_out_endpoint.udp);
                    assert(rc >= 0);
                } else if (nmea_out_sock4 >= 0) {
                    rc = diminuto_ipc4_datagram_send(nmea_out_sock4, sentence, strlen(sentence), nmea_out_endpoint.ipv4, nmea_out_endpoint.udp);
                    assert(rc >= 0);
                } else {
                    /* Do nothing. */
                }
            }

            /*
             * Handle pulse rise.
             */

            risings += 1;
            milliseconds_pulse = milliseconds_cycle;
            LOG("RISING %dms.", milliseconds_pulse);
            break;

        case DIMINUTO_CUE_EDGE_HIGH:

            /*
             * Keep counting off milliseconds of pulse.
             */

            milliseconds_pulse += milliseconds_cycle;
            break;

        case DIMINUTO_CUE_EDGE_FALLING:

            if (pps) {
                rc = diminuto_pin_clear(pin_out_pps_fp);
                assert(rc >= 0);
            }

            /*
             * Handle pulse fall.
             */


            fallings += 1;
            milliseconds_pulse += milliseconds_cycle;
            LOG("FALLING %dms.", milliseconds_pulse);
            break;

        default:
            assert(edge != edge);
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
         * Check for SIGHUP and if seen report and dediscipline.
         *
         * initialized:    true if the debouncer has been initialized.
         * synchronized:    true if we are synced to the IRIQ framing.
         * acquired:        true if we have received a complete valid frame.
         * armed:           true if we have constructed a valid time stamp.
         * disciplined:     true if we have set the system clock.
         */

        if (diminuto_hangup_check()) {
            DIMINUTO_LOG_NOTICE("%s: hungup initialized=%d synchronized=%d acquired=%d disciplined=%d armed=%d risings=%d fallings=%d cycles=%d.\n", program, initialized, synchronized, acquired, disciplined, armed, risings, fallings, cycles);
            disciplined = 0;
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

        /*
        ** Parse grammar by transitioning state based on token.
        */

        state_old = state;

        event = obelisk_parse(&state, token, &field, &length, &buffer, &frame);

        assert((0 <= state_old) && (state_old < countof(STATE)));
        assert((0 <= state) && (state < countof(STATE)));
        assert((0 <= token) && (token < countof(TOKEN)));
        assert((0 <= event) && (event < countof(EVENT)));

        LOG("PARSE %s %s %s %s %d %d 0x%llx.", STATE[state_old], TOKEN[token], STATE[state], EVENT[event], field, length, (long long unsigned int)buffer);

        switch (event) {

        case OBELISK_EVENT_WAITING:

            /*
             * If our event indicates that we've synchronized with the framing,
             * we can enable stuff like PPS.
            */

            if (synchronized) {
                synchronized = 0;
                DIMINUTO_LOG_NOTICE("%s: synchronizing.\n", program);
            }

            armed = 0;

            break;

        case OBELISK_EVENT_INVALID:

            /*
             * Detect an error that causes the state machine to return
             * to its start state.
             */

            if (acquired) {
                acquired = 0;
                DIMINUTO_LOG_NOTICE("%s: lost state=%s token=%s state=%s.\n", program, STATE[state_old], TOKEN[token], STATE[state]);
            }

            armed = 0;

            break;
 
        case OBELISK_EVENT_TIME:

            /*
             * Detect the beginning of the minute so that we can set the
             * time if we so desire.
             */

            if (!(set_initially || set_daily || set_leap)) {
                /* Do nothing. */
            } else if (!armed) {
                /* Do nothing. */
            } else if (disciplined) {
                /* Do nothing. */
            } else {
        
                /*
                 * Set the system clock. For most (larger) UNIX systems,
                 * this is a rare and potentially violent operation.
                 * That's because a lot of subsystems depend on the
                 * system clock to know when to do things. Changing the
                 * system clock can cause unexpected results.
                 */
    
                if ((rc = settimeofday(&epoch, (struct timezone *)0)) < 0) {
                    diminuto_perror("settimeodday");
                } else {
    
                    disciplined = !0;
    
                    ticks_now = diminuto_time_clock();
                    assert(ticks_now >= 0);
    
                    rc = diminuto_time_zulu(ticks_now, &year, &month, &day, &hour, &minute, &second, &fraction);
                    assert(rc >= 0);
    
                    DIMINUTO_LOG_NOTICE("%s: set zulu=%04d-%02d-%02dT%02d:%02d:%02d.%09llu.\n",
                        program,
                        year, month, day,
                        hour, minute, second,
                        (long long unsigned int)(fraction / 1000)
                    );
    
                }
    
            }

            armed = 0;

            break;

        case OBELISK_EVENT_FRAME:

            armed = 0;

            /*
             * Once we have a complete frame, extract it from the buffer.
             */

            LOG("FRAME 0x%016llx %d %d %d %d %d %d %d %d %d %d %d %d %d %d.",
                 (long long unsigned int)buffer,
                 frame.year10, frame.year1,
                 frame.day100, frame.day10, frame.day1,
                 frame.hours10, frame.hours1,
                 frame.minutes10, frame.minutes1,
                 frame.dut1sign,
                 frame.dut1magnitude,
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

                /*
                 * If the frame doesn't pass the validity check, it's at least
                 * corrupt; if we had acquired the signal, we've lost it.
                 */

                if (acquired) {
                    acquired = 0;
                    DIMINUTO_LOG_NOTICE("%s: lost rc=%d.\n", program, rc);
                } else if (!disciplined) {
                    DIMINUTO_LOG_NOTICE("%s: corrupt rc=%d.\n", program, rc);
                } else {
                    /* Dothing. */
                }

            } else {

                /*
                 * Seconds will always be 59 at this point in the frame.
                 */

                time.tm_sec = 59;

                /*
                 * Calculate the difference from UT1. This will always be
                 * in the range [-0.9 .. +0.9]; any more, and another leap
                 * second would have been inserted.
                 */

                dut1 = frame.dut1magnitude;
                dut1 /= 10.0;
                if (frame.dut1sign == OBELISK_SIGN_NEGATIVE) { dut1 = -dut1; }

                assert((0 <= time.tm_wday) && (time.tm_wday < countof(DAY)));
                LOG("TIME %d %04d-%02d-%02dT%02d:%02d:%02dZ %04d/%03d %s %s %+4.1f.",
                    rc,
                    time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec,
                    time.tm_year + 1900, time.tm_yday + 1,
                    DAY[time.tm_wday],
                    time.tm_isdst ? "DST" : "!DST",
                    dut1
                );

                /*
                 * Derive the seconds since the POSIX Epoch that our time
                 * code represents.
                 */

                epoch.tv_sec = timegm(&time);

                LOG("EPOCH %lds.", epoch.tv_sec);

                if (!acquired) {
                    acquired = !0;
                    DIMINUTO_LOG_NOTICE("%s: acquired.\n", program);
                }

                /*
                 * Logging the received one per hour doesn't overrun
                 * the logging system. And doing so at the 59th minute
                 * captures the leap second at :59:60 if it occurs.
                 */

                if (!disciplined || (time.tm_min == 59)) {
                    DIMINUTO_LOG_NOTICE("%s: time zulu=%04d-%02d-%02dT%02d:%02d:%02d julian=%04d/%03d day=%s dst=%c dUT1=%+4.1fs lyi=%d lsw=%d epoch=%lds.",
                        program,
                        time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                        time.tm_hour, time.tm_min, time.tm_sec,
                        time.tm_year + 1900, time.tm_yday + 1,
                        DAY[time.tm_wday],
						DST[frame.dst],
						dut1,
                        frame.lyi,
                        frame.lsw,
						epoch.tv_sec
                    );
                }

                /*
                 * This time stamp is usable only in the very next
                 * TIME event.
                 */

                armed = !0;

                if (!disciplined) {
                    LOG("READY.");
                }

                /*
                 * If we think the system clock has been disciplined, then
                 * use it to determine the local time. If the local time is
                 * at the appropriate time, let the work loop set the time
                 * again.
                 */

                if (!set_daily) {
                    /* Do nothing. */
                } else if (!disciplined) {
                    /* Do nothing. */
                } else {

                    ticks_now = diminuto_time_clock();
                    assert(ticks_now >= 0);

                    rc = diminuto_time_juliet(ticks_now, (int *)0, (int *)0, (int *)0, &hour, &minute, (int *)0, (diminuto_ticks_t *)0);
                    assert(rc >= 0);

                    if (hour != hour_juliet) {
                        /* Do nothing. */
                    } else if (minute != minute_juliet) {
                        /* Do nothing. */
                    } else {
                        disciplined = 0;
                        LOG("READY %02d:%02d:00J.", hour, minute);
                    }

                }

            }

            break;

        case OBELISK_EVENT_LEAP:

            /*
             * Extra leap second marker encountered. The mere reception of
             * this marker advanced the second in the epoch.
             */

            DIMINUTO_LOG_NOTICE("%s: leap lsw=%d.", program, frame.lsw);

            if (!set_leap) {
                /* Do nothing. */
            } else if (!disciplined) {
                /* Do nothing. */
            } else {
                disciplined = 0;
                LOG("READY %ld.", epoch.tv_sec);
            }

            armed = 0;

            break;

        case OBELISK_EVENT_NOMINAL:

            /*
             * If we're collecting data, we are synchronized.
             */

            if (!synchronized) {
                synchronized = !0;
                DIMINUTO_LOG_NOTICE("%s: synchronized.\n", program);
            }

            armed = 0;

            break;

        default:
            assert(event != event);
            break;

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
        if (rc != 0) { diminuto_perror(nmea_path); }
        assert(rc == 0);
    }

    if (nmea_out_sock6 >= 0) {
        rc = diminuto_ipc6_close(nmea_out_sock6);
        if (rc < 0) { diminuto_perror(nmea_endpoint); }
        assert(rc >= 0);
    }

    if (nmea_out_sock4 >= 0) {
        rc = diminuto_ipc4_close(nmea_out_sock4);
        if (rc < 0) { diminuto_perror(nmea_endpoint); }
        assert(rc >= 0);
    }

    (void)diminuto_lock_unlock(run_path);

    /*
    ** Exit.
    */

    DIMINUTO_LOG_NOTICE("%s: exiting.\n", program);

    return 0;
}
