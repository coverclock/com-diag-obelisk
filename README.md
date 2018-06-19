# com-diag-obelisk
A home brew NIST WWVB radio clock.
## Copyright
Copyright 2017-2018 by the Digital Aggregates Corporation, Arvada Colorado U.S.A.
## License
Licensed under the terms of the FSF GNU GPL v2.0.
## Contact
Chip Overclock  
<mailto:coverclock@diag.com>  
Digital Aggregates Corporation  
<http://www.diag.com>  
3440 Youngfield Street  
Suite 209  
Wheat Ridge CO 80033  
U.S.A.  
## Abstract
Obelisk is my excuse to learn how to use the NIST WWVB radio time
signal. Obelisk, a.k.a. O-3, consists of a small library and an
application. The application runs on a Raspberry Pi. It decodes the
amplitude-modulated (AM) pulse-duration-encoded binary-coded-decimal
(BCD) data stream from a SYM-RFT-60 radio receiver. WWVB is a 60KHz
transmitter near Fort Collins Colorado. The facility is managed by the
U.S. National Institute of Standards and Technology. WWVB transmits the
current time every minute. This time is disciplined by atomic clocks
at the transmitter site. These clocks are ultimately synchronized to
the master atomic clock at the NIST laboratories in Boulder Colorado.

The application, wwbvtool, is built on top of the Obelisk library, the
Hazer library, and the Diminuto library. Obelisk implements functions to
demodulate, decode, and parse the data frames transmitted by WWVB. Hazer
implements functions to both parse and generate the NMEA sentences
associated with the typical GPS device. Diminuto is a general purpose
systems programming library. The wwvbtool application, Obelisk, Hazer,
and Diminuto are all written in C, are all products of the Digital
Aggregates Corporation, and are all open source.

Unlike Hourglass, a.k.a. O-1, which is a stratum-1 GPS-disciplined desk
clock, and Astrolabe, a.k.a. O-2, which is a stratum-0 GPS-disciplined
mantel clock with a chip-scale cesium atomic clock, Obelisk may not be
useful as a reference clock for an NTP server. You will find evidence
in this repository that I have experimented with using wwvbtool to
generate both GPS (NMEA) and PPS reference clocks, with mixed results.
My user-space approach suffers from a lot of sampling jitter and
scheduling latency.

But regardless of its poor behavior as an NTP reference clock, Obelisk
makes a fine desk clock just by synchronizing its own clock to WWVB.
The current options applied to wwvbtool in the timeservice init script
cause the clock to set itself as soon as it receives a correct frame
from WWVB, and to correct itself at 1:30AM local time ("juliet" in
NATO speak) when it receives another correct frame. This strategy
was borrowed from my Casio Wave Captor Multi Band 6, a solar-powered
wristwatch with its own WWVB receiver.

Reception of the long-wave AM signal can affected by the position of the
antenna, interference from perhaps inadvertent radio frequency sources in
the neighborhood, other electronic equipment (particularly LCD displays),
and even a sunny day. This is true even in Denver Colorado, where the WWVB
long-wave signal is strong enough to "pick up in the fillings of your
teeth" as my short-wave friends say. The signal is frequently best late
at night, which is the reason for the 1:30AM local time strategy. The
clock should be oriented such that the long direction (lateral) of the
ferrite antenna core is at right angles (orthogonal) to the direction
of the WWVB transmitter in Fort Collins Colorado.

The Obelisk software is quite paranoid about checking for errors,
rejecting invalid frames and logging signal loss and acquision in the
system log. The software processing the modulated signal is in effect a
kind of language interpreter: a tokenizer classifies pulses into one of
four symbols, ZERO, ONE, MARKER or INVALID; and a finite state machine
parses the symbols according to a simple grammer, rejecting any input
that violates what is in effect a language specification.

Part of this paranoia prevented me from using the blocking select(2)
system call, available in the Diminuto API, to wait for the GPIO pin
from the radio receiver to change. Instead, wwvbtool samples the pin
according to an interval timer and uses a software signal level debouncer
to deal with periods of high interference. My concern is that with the
more efficient former approach, interrupts might overwhelm the system.

## Repositories
<https://github.com/coverclock/com-diag-obelisk>    
<https://github.com/coverclock/com-diag-hazer>    
<https://github.com/coverclock/com-diag-diminuto>    
<https://git.savannah.gnu.org/git/gpsd.git>    
<https://gitlab.com/NTPsec/ntpsec.git>    
## Articles
<http://coverclock.blogspot.com/2017/10/my-wwvb-radio-clock.html>    
## Images
<https://www.flickr.com/photos/johnlsloan/albums/72157689295451755>    
## References
<https://en.wikipedia.org/wiki/WWVB>    
<http://www.pvelectronics.co.uk/rftime/SYM-RFT-XX.pdf>    
<https://universal-solder.com/product/60khz-wwvb-atomic-radio-clock-receiver-replaces-c-max-cmmr-6p-60/>    
<http://tinkersphere.com/sensors/1517-wwvb-nist-radio-time-receiver-kit.html>    
<http://www.popsci.com/diy/article/2010-03/build-clock-uses-atomic-timekeeping>    
<https://github.com/spuder/WWVB-Clock>    
<https://www.rs-online.com/designspark/atomic-time-for-the-raspberry-pi>    
## Targets
"obelisk"    
Raspberry Pi 3    
Raspbian 9    
gcc 6.3.0    
Linux 4.9.41    
SYM-RFT-60    
## Usage
    usage: wwvbtool [ -1 | -2 ] [ -7 | -8 ] [ -B BAUD ] [ -C NICE ] [ -H HOUR ] [ -L PATH ] [ -M MINUTE ] [ -N TALKER ] [ -O PATH ] [ -P PIN ] [ -S PIN ] [ -T PIN ] [ -U ENDPOINT ] [ -a ] [ -b ] [ -c ] [ -d ] [ -e | -o ] [ -g ] [ -h ] [ -i ] [ -k ] [ -l ] [ -m ] [ -n ] [ -p ]  [ -r ] [ -s ] [ -u ] [ -v ] [ -x ]
           -1              Use one stop bit for OUTPUT (default).
           -2              Use two stop bits for OUTPUT.
           -7              Use seven data bits for OUTPUT.
           -8              Use eight data bits for OUTPUT (default).
           -B BAUD         Use BAUD bits per second for OUTPUT (115200).
           -C NICE         Set scheduling priority to NICE (-20..19).
           -H HOUR         Set time of day at HOUR local (1).
           -L PATH         Use PATH for lock file ("/var/run/wwvbtool.pid").
           -M MINUTE       Set time of day at MINUTE local (30).
           -N TALKER       Set NMEA TALKER ("ZV").
           -O OUTPUT       Write NMEA sentences to OUTPUT ("-").
           -P PIN          Use P1 output GPIO PIN (23).
           -S PIN          Use PPS output GPIO PIN (25).
           -T PIN          Use T input GPIO PIN (24).
           -U ENDPOINT     Write NMEA sentences to UDP ENDPOINT.
           -a              Set time of day when leap second occurs.
           -b              Daemonize into the background.
           -c              Use RTS/CTS for OUTPUT.
           -d              Display debug output.
           -e              Use even parity for OUTPUT.
           -g              Send SIGHUP to the PID in the lock file and exit.
           -h              Display help menu and exit.
           -i              Set time of day initially when possible.
           -k              Send SIGTERM to the PID in the lock file and exit.
           -l              Remove the lock file initially ignoring errors.
           -m              Use modem control for OUTPUT.
           -n              Generate NMEA output.
           -o              Use odd parity for OUTPUT.
           -p              Generate PPS output.
           -r              Reset device initially.
           -s              Set time of day daily when possible.
           -u              Unexport pins initially ignoring errors.
           -v              Display verbose output.
           -x              Use XON/XOFF for OUTPUT.
## Notes
Here is the SYM-RFT-60 pinouts and to what they are connected.

    A1                          --> antenna
    A2                          --> antenna

    Vdd IN                      --> Pi pin 1 3.3V OUT
    Tcon OUT                    --> Pi pin 18 GPIO 24 IN
    Pon IN                      --> Pi pin 16 GPIO 23 OUT
    GND IN                      --> Pi pin 14 GND OUT
    N/C                         --> N/A

Here are the additional pins used on the Raspberry Pi.

    Pi pin 22 GPIO 25 OUT      --> Pi pin 12 GPIO 18 CLK IN

Here is a partial list of additional packages needed.

    sudo apt-get install pps-tools
    sudo apt-get install scons
    sudo apt-get install bison
    sudo apt-get install flex
    sudo apt-get install libssl-dev
    sudo apt-get install libcap-dev

Clone, build, and install Diminuto 48.12.0 in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-diminuto
    cd com-diag-diminuto/Diminuto
    git checkout 48.12.0
    make pristine all
    sudo make install

Clone, build, and install Hazer 1.4.1 in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-hazer
    cd com-diag-hazer/Hazer
    git checkout 1.4.1
    make pristine all
    sudo make install

Clone, build, and install Obelisk 3.1.4 in /usr/local. (The latest version
will probably work just fine, if you want to omit the git checkout.)

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-obelisk
    cd com-diag-obelisk/Obelisk
    git checkout 3.1.4
    make pristine all
    sudo make install

Clone, build, and install NTPsec daemon in /usr/local. Note that the
version of ntpsec that I used in Obelisk requires the refclock configuration
option; the earlier version I used in Hourglass and Astrolabe did not require
this (as far as I can remember anyway).

    cd ~
    mkdir -p src
    cd src
    git clone https://gitlab.com/NTPsec/ntpsec.git
    cd ntpsec
    sudo ./buildprep
    ./waf configure --refclock=gpsd,pps,nmea,shm
    ./waf build
    sudo ./waf install

Clone, build, and install GPS daemon in /usr/local. We have to enable the NETFEED
option so that we can sent it UDP datagrams. The timeservice start up script is
set to use UDP port 60180; this is trivial to edit to change it.

    cd ~
    mkdir -p src
    cd src
    git clone https://git.savannah.gnu.org/git/gpsd.git
    cd gpsd
    scons timeservice=yes nmea0183=yes prefix="/usr/local" pps=yes ntpshm=yes netfeed=yes
    sudo scons install

These user IDs in /etc/passwd may be required by the GPS daemon and the
NTP daemon; Obelisk runs as root. Change the user ID values as necessary.

    gpsd:x:111:20:GPSD system user,,,:/run/gpsd:/bin/false
    ntp:x:112:65534:,,,:/home/ntp:/bin/false

Run interactively for debugging.

    sudo su
    . out/host/bin/setup
    out/host/bin/wwvbtool -d -n -p -l -u -r -i -s -a

Run as a daemon in the background.

    sudo su
    . out/host/bin/setup
    out/host/bin/wwvbtool -b -n -p -l -u -r -i -s -a

Send SIGHUP to resynchronize (equivalent commands).

    sudo kill -HUP `cat /var/run/wwvbtool.pid`

    sudo wwvbtool -g

Send SIGTERM to terminate (equivalent commands).

    sudo kill -TERM `cat /var/run/wwvbtool.pid`

    sudo wwvbtool -k

Disable gpsd and enable time service (only needed once ever).

    sudo systemctl disable gpsd
    sudo systemctl enable timeservice

Start time service.

    service timeservice start

Stop time service.

    service timeservice stop

Process Obelisk wwvbtool NMEA output using Hazer gpstool.

    wwvbtool -r -s -u -l -n -p | gpstool -R
   
    2017-10-05T21:08:17.934603Z 6 [8209] wwvbtool: running pid=8209.
    2017-10-05T21:09:59.847936Z 5 [8209] wwvbtool: acquired.
    2017-10-05T21:09:59.848004Z 6 [8209] wwvbtool: time zulu=2017-10-05T21:09:59 julian=2017/278 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    $ZVRMC,211000.00,A,,,,,,,051017,,,D*7B\r\n
    $ZVRMC,211000.00,A,,,,,,,051017,,,D*7B\r\n
    MAP 2017-10-05T21:10:00Z  0*00'00.00"N,  0*00'00.00"E     0.00' N     0.000mph
    RMC  0.000000,  0.000000     0.000m   0.000*    0.000knots [00] 0 0 0 0 0
    2017-10-05T21:10:00.027201Z 5 [8209] wwvbtool: now zulu=2017-10-05T21:10:00.000027175.
    $ZVRMC,211100.00,A,,,,,,,051017,,,D*7A\r\n
    $ZVRMC,211100.00,A,,,,,,,051017,,,D*7A\r\n
    MAP 2017-10-05T21:11:00Z  0*00'00.00"N,  0*00'00.00"E     0.00' N     0.000mph
    RMC  0.000000,  0.000000     0.000m   0.000*    0.000knots [00] 0 0 0 0 0
    $ZVRMC,211200.00,A,,,,,,,051017,,,D*79\r\n
    $ZVRMC,211200.00,A,,,,,,,051017,,,D*79\r\n
    MAP 2017-10-05T21:12:00Z  0*00'00.00"N,  0*00'00.00"E     0.00' N     0.000mph
    RMC  0.000000,  0.000000     0.000m   0.000*    0.000knots [00] 0 0 0 0 0 

Make a FIFO (First In First Out), that is, a named pipe in the file
system Run wwvbtool to write to the FIFO, gpsd to read from the FIFO,
and gpsmon to read from gpsd. gpsd, wwvbtool, and gpsmon are all run in
the foreground from three different windows.

    mkfifo wwvb.fifo
    gpsd -b -N -D 2 -n ./wwvb.fifo

    wwvbtool -N GP -r -s -u -l -n -p -O ./wwvb.fifo

    gpsmon

Similarly, use a UDP socket for gpsd and wwvbtool to communicate. This
requires a gpsd built with the NETFEED option.

    gpsd -b -N -D 2 -n udp://localhost:60180

    wwvbtool -N GP -r -s -u -l -n -p -U localhost:60180

    gpsmon

Use socat instead of gpsd to receive UDP datagrams from wwvbtool.

    wwvbtool -N GP -r -s -u -l -n -p -U localhost:60180

    socat udp-recv:60180 -

Configure and test Pulse Per Second (PPS) when using -p flag on wwvbtool. Note
that in this example gpiopin=18 is GPIO18 a.k.a. physical pin 12.

    $ sudo apt-get install pps-tools
    :
    $ grep pps /boot/config.txt
    dtoverlay=pps-gpio,gpiopin=18
    $ lsmod | grep pps
    pps_gpio                3293  0
    pps_core                9164  1 pps_gpio
    $ sudo ppstest /dev/pps0
    trying PPS source "/dev/pps0"
    found PPS source "/dev/pps0"
    ok, found 1 source(s), now start fetching data...
    source 0 - assert 1507391765.063823536, sequence: 71286 - clear  0.000000000, sequence: 0
    source 0 - assert 1507391766.063842614, sequence: 71287 - clear  0.000000000, sequence: 0
    source 0 - assert 1507391767.063818786, sequence: 71288 - clear  0.000000000, se

Run wwvbtool using a serial port as the NMEA output device. The serial
options are only applied if the NMEA output parameter is a character
device. The device below is the Raspberry Pi console port. The port is
configured below for 9600 baud, 8 data bits, 1 stop bit, and no parity
(by not specifying -e or -o).

    wwvbtool -d -O /dev/ttyAMA0 -B 9600 -8 -1 -n -p -r -s -u -N GP

Before enabling the NTP peerstats capability in /etc/ntp.conf, do this.

    sudo mkdir -p -m 0777 /var/log/ntpstats

Sometimes NTP seems relatively happy with the GPS NMEA and PPS synthesized
by wwvbtool.

    $ ntpq -c peer -c as -c rl localhost
         remote           refid      st t when poll reach   delay   offset   jitter
    ===============================================================================
    *SHM(1)          .PPS.            0 l   36   64  377   0.0000 -12.9271  39.0038
    +SHM(0)          .GPS.            0 l   35   64  377   0.0000   5.8287  38.9642
     us.pool.ntp.org .POOL.          16 p    -  256    0   0.0000   0.0000   0.0019
    -b1-66er.matrix. 129.6.15.30      2 u   62   64  377  64.1515  85.1565  65.9280
    +coopnet.cc      216.218.254.202  2 u    -   64  377  23.3387  40.9690  34.3539
    +soft-sea-01.ser 209.51.161.238   2 u   66   64  377  38.2534  -1.2305  54.6508
    -clock.xmission. 150.143.81.69    2 u   61   64  377  38.3154  70.3081  58.3872
    +biisoni.miuku.n 207.224.49.219   2 u   15   64  377  38.6064 -11.1295  68.6863
    +y.ns.gin.ntt.ne 249.224.99.213   2 u   10   64  377  24.9829 -10.5371  67.2388
    +209.208.79.69   130.207.244.240  2 u   12   64  377  53.0668  -8.5697  69.2658
    +251.228.185.35. 169.254.169.254  3 u   12   64  377  39.4476 -11.8270  66.9695
    
    ind assid status  conf reach auth condition  last_event cnt
    ===========================================================
      1 39979  962a   yes   yes  none  sys.peer    sys_peer  2
      2 39980  941a   yes   yes  none candidate    sys_peer  1
      3 39981  8811   yes  none  none    reject    mobilize  1
      4 39982  131a    no   yes  none   outlier    sys_peer  1
      5 39983  141a    no   yes  none candidate    sys_peer  1
      6 39984  1414    no   yes  none candidate   reachable  1
      7 39985  1314    no   yes  none   outlier   reachable  1
      8 39986  1414    no   yes  none candidate   reachable  1
      9 39987  1414    no   yes  none candidate   reachable  1
     10 39988  1414    no   yes  none candidate   reachable  1
     11 39989  1414    no   yes  none candidate   reachable  1
    associd=0 status=0415 leap_none, sync_uhf_radio, 1 event, clock_sync,
    version="ntpd ntpsec-0.9.7+1473 2017-10-06T16:43:31Z", processor="armv7l",
    system="Linux/4.9.41-v7+", leap=00, stratum=1, precision=-19, rootdelay=0.0,
    rootdisp=54.926, refid=PPS,
    reftime=dd8761c6.9842a985 2017-10-10T15:12:06.594Z,
    clock=dd8761ea.e9000a04 2017-10-10T15:12:42.910Z, peer=39979, tc=6, mintc=0,
    offset=-12.927091, frequency=-144.019516, sys_jitter=39.003832,
    clk_jitter=27.85568, clk_wander=4.046512

Sometimes not so much, for the synthesized PPS at least.

    $ ntpq -c peer -c as -c rl localhost
         remote           refid      st t when poll reach   delay   offset   jitter
    ===============================================================================
    xSHM(1)          .PPS.            1 l   29   64  377   0.0000 -30.0354   4.6376
    *SHM(0)          .GPS.            1 l   27   64  377   0.0000 -15.7650   4.6256
     us.pool.ntp.org .POOL.          16 p    -  256    0   0.0000   0.0000   0.0010
    -mirror1.sjc02.s 162.213.2.253    2 u   64   64  377  41.6896  27.2100   6.2153
    +ns2.intrax.com  .GPSD.           1 u   35   64  377  76.4996  34.6403   4.0932
    -palpatine.steve 132.239.1.6      2 u   52   64  377  45.5836  28.6521   5.0903
     zombie.frizzen. 200.130.17.247   3 u  53m 1024    0  54.9129  58.2962  29.1178
    -paladin.latt.ne 204.123.2.72     2 u   32  128  377  58.2541  41.3394   4.2073
    -dev.smatwebdesi 192.168.204.60   3 u  101  128  377  34.9645  39.0102   3.2400
    +test.diarizer.c 216.239.35.4     2 u  100   64  376  40.2588  35.6555   2.4089
    
    ind assid status  conf reach auth condition  last_event cnt
    ===========================================================
      1 58034  91ca   yes   yes  none falsetick    sys_peer 12
      2 58035  96ea   yes   yes  none  sys.peer    sys_peer 14
      3 58036  8811   yes  none  none    reject    mobilize  1
      4 58037  1314    no   yes  none   outlier   reachable  1
      5 58038  14ca    no   yes  none candidate    sys_peer 12
      6 58039  131a    no   yes  none   outlier    sys_peer  1
      7 58041  0013    no    no  none    reject unreachable  1
      8 58042  1314    no   yes  none   outlier   reachable  1
      9 58043  1314    no   yes  none   outlier   reachable  1
     10 58044  141a    no   yes  none candidate    sys_peer  1
    associd=0 status=0415 leap_none, sync_uhf_radio, 1 event, clock_sync,
    version="ntpd ntpsec-0.9.7+1473 2017-10-06T16:43:31Z", processor="armv7l",
    system="Linux/4.9.41-v7+", leap=00, stratum=2, precision=-20, rootdelay=0.0,
    rootdisp=57.119, refid=127.127.28.0,
    reftime=dd9b196b.ad43ea60 2017-10-25T14:08:43.676Z,
    clock=dd9b1b07.1bdc7d73 2017-10-25T14:15:35.108Z, peer=58035, tc=6, mintc=0,
    offset=1.6395, frequency=-6.460846, sys_jitter=5.240931,
    clk_jitter=11.573969, clk_wander=0.676028

The wwvbtool utility writes some stuff to the system log at the "notice"
level when signficant events occur, or periodically at fifty-nine minutes
after the hour, or whenever it receives a SIGHUP signal. My version of
Raspbian extracts messages at this level from the /var/log/syslog and
saves them in /var/log/messages. Here are an extended example.

    Oct 24 15:50:32 obelisk wwvbtool[21005]: wwvbtool: running pid=21005.
    Oct 24 15:50:33 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 24 15:51:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 24 15:51:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-24T21:51:59 julian=2017/297 day=TUE dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 15:51:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 24 15:52:00 obelisk wwvbtool[21005]: wwvbtool: set zulu=2017-10-24T21:52:00.000020113.
    Oct 24 15:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-24T21:59:59 julian=2017/297 day=TUE dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 16:01:13 obelisk wwvbtool[21005]: wwvbtool: hungup synchronized=1 acquired=1 disciplined=1 armed=0 risings=1 fallings=1 cycles=60.
    Oct 24 16:01:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-24T22:01:59 julian=2017/297 day=TUE dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 16:02:00 obelisk wwvbtool[21005]: wwvbtool: set zulu=2017-10-24T22:02:00.000023835.
    Oct 24 16:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-24T22:59:59 julian=2017/297 day=TUE dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 17:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-24T23:59:59 julian=2017/297 day=TUE dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 18:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T00:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 19:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T01:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 20:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T02:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 21:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T03:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 22:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T04:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 24 23:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T05:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 00:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T06:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 01:31:00 obelisk wwvbtool[21005]: wwvbtool: set zulu=2017-10-25T07:31:00.000020050.
    Oct 25 01:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T07:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 02:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T08:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 03:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T09:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 04:56:28 obelisk wwvbtool[21005]: wwvbtool: lost risings=1 fallings=0.
    Oct 25 04:56:33 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 25 06:05:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 25 06:05:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 25 06:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T12:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 07:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T13:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 08:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T14:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 09:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T15:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 10:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T16:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 11:19:10 obelisk wwvbtool[21005]: wwvbtool: lost risings=1 fallings=0.
    Oct 25 11:19:17 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 25 11:25:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 25 11:25:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 25 11:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T17:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 12:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T18:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 13:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T19:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 14:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T20:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 15:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T21:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 16:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T22:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 17:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-25T23:59:59 julian=2017/298 day=WED dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 18:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T00:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 19:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T01:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 20:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T02:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 21:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T03:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 22:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T04:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 25 23:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T05:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 00:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T06:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 01:31:00 obelisk wwvbtool[21005]: wwvbtool: set zulu=2017-10-26T07:31:00.000020056.
    Oct 26 01:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T07:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 02:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T08:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 03:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T09:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 04:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T10:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 05:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T11:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 06:25:02 obelisk liblogging-stdlog:  [origin software="rsyslogd" swVersion="8.24.0" x-pid="352" x-info="http://www.rsyslog.com"] rsyslogd was HUPed
    Oct 26 06:56:14 obelisk wwvbtool[21005]: wwvbtool: lost state=DATA token=INVALID state=START.
    Oct 26 06:56:15 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 06:57:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 06:57:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 26 06:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T12:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 07:28:05 obelisk wwvbtool[21005]: wwvbtool: lost state=DATA token=INVALID state=START.
    Oct 26 07:28:06 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 07:29:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 07:29:18 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 07:29:20 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 07:29:29 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 07:30:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 07:30:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 26 07:39:18 obelisk wwvbtool[21005]: wwvbtool: lost risings=0 fallings=0.
    Oct 26 07:39:21 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 07:40:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 07:40:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 26 07:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T13:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 08:10:14 obelisk wwvbtool[21005]: wwvbtool: lost risings=1 fallings=0.
    Oct 26 08:10:18 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 08:11:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 08:11:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 26 08:42:37 obelisk wwvbtool[21005]: wwvbtool: lost state=DATA token=MARKER state=START.
    Oct 26 08:42:38 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 08:43:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 08:43:51 obelisk wwvbtool[21005]: wwvbtool: synchronizing.
    Oct 26 08:45:01 obelisk wwvbtool[21005]: wwvbtool: synchronized.
    Oct 26 08:45:59 obelisk wwvbtool[21005]: wwvbtool: acquired.
    Oct 26 08:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T14:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.
    Oct 26 09:59:59 obelisk wwvbtool[21005]: wwvbtool: time zulu=2017-10-26T15:59:59 julian=2017/299 day=THU dst=+ dUT1=+0.3 lyi=0 lsw=0.

You can see that the system looses the WWVB signal on several occasions and
eventually reacquires it. When I catch the system in this state, I can see
the LED on the radio board blinking sporadically and apparently randomly.
