# com-diag-obelisk
A home brew NIST WWVB radio clock.
## Copyright
Copyright 2017 by the Digital Aggregates Corporation, Arvada Colorado U.S.A.
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
amplitude-modulated pulse-duration-encoded binary-coded-decimal data
stream from a SYM-RFT-60 radio receiver. WWVB is a 60KHz long-wave
transmitter near Fort Collins Colorado. The facility is managed by the
U.S. National Institute of Standards and Technology. WWVB transmits the
current time every minute. This time is disciplined by atomic clocks
at the transmitter site. These clocks are ultimately synchronized to
the master atomic clock at the NIST laboratories in Boulder Colorado.
The application is built on top of the Obelisk library and the Diminuto
library. Both Obelisk and Diminuto are written in C.

Unlike Hourglass, a.k.a. O-1, which is a stratum-1 GPS-disciplined desk
clock, and Astrolabe, a.k.a. O-2, which is a stratum-0 GPS-disciplined
mantel clock with a chip-scale cesium atomic clock, Obelisk may not be
useful as a reference clock for an NTP server. You will find evidence
in this repository that I am experimenting with using wwvbtool to
generate both GPS (NMEA) and PPS reference clocks, but that is a work in
progress. So far my user space approach doesn't produce precise enough
PPS, which I attribute to software latency of going through wwvbtool to
gpsd to ntpd. Regardless of its poor behavior as an NTP reference clock,
Obelisk makes a fine desk clock just by synchronizing its own clock to
WWVB as described below.

The current options applied to wwvbtool in the timeservice init script
cause the clock to set itself as soon as it receives a correct frame from
WWVB, and to correct itself at 1:30AM local time ("juliet" in NATO speak)
when it receives another correct frame. This strategy was borrowed from my
Casio Wave Captor Multi Band 6, a solar-powered wristwatch with its own
WWVB receiver. Here in Denver Colorado, the WWVB long-wave signal is strong
enough to "pick up in the fillings of your teeth" as my short-wave friends
say. But elsewhere the signal can be extremely finicky; the position of the
antenna, interference from radio frequency sources in the neighborhood, and
even a sunny day, can interfere with it. The signal is frequently best
late at night, which is the reason for the 1:30AM local time strategy.
## Links
<https://github.com/coverclock/com-diag-obelisk>    
<https://github.com/coverclock/com-diag-diminuto>    
<https://www.flickr.com/photos/johnlsloan/albums/72157689295451755>    
<https://en.wikipedia.org/wiki/WWVB>    
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
    usage: wwvbtool [ -1 | -2 ] [ -7 | -8 ] [ -B BAUD ] [ -H HOUR ] [ -L PATH ] [ -M MINUTE ] [ -N TALKER ] [ -O PATH ] [ -P PIN ] [ -S PIN ] [ -T PIN ] [ -b ] [ -c ] [ -d ] [ -e | -o ] [ -g ] [ -h ] [ -k ] [ -l ] [ -m ] [ -n ] [ -p ]  [ -r ] [ -s ] [ -u ] [ -v ] [ -x ]
           -1              Use one stop bit for OUTPUT (default).
           -2              Use two stop bits for OUTPUT.
           -7              Use seven data bits for OUTPUT.
           -8              Use eight data bits for OUTPUT (default).
           -B BAUD         Use BAUD bits per second for OUTPUT (115200).
           -H HOUR         Set time of day at HOUR local (1).
           -L PATH         Use PATH for lock file ("/var/run/wwvbtool.pid").
           -M MINUTE       Set time of day at MINUTE local (30).
           -N TALKER       Set NMEA TALKER ("ZV").
           -O OUTPUT       Write NMEA sentences to OUTPUT ("-").
           -P PIN          Use P1 output GPIO PIN (23).
           -S PIN          Use PPS output GPIO PIN (25).
           -T PIN          Use T input GPIO PIN (24).
           -b              Daemonize into the background.
           -c              Use RTS/CTS for OUTPUT.
           -d              Display debug output.
           -e              Use even parity for OUTPUT.
           -g              Send SIGHUP to the PID in the lock file and exit.
           -h              Display help menu and exit.
           -k              Send SIGTERM to the PID in the lock file and exit.
           -l              Remove the lock file initially ignoring errors.
           -m              Use modem control for OUTPUT.
           -n              Generate NMEA output.
           -o              Use odd parity for OUTPUT.
           -p              Generate PPS output.
           -r              Reset device initially.
           -s              Set time of day when possible.
           -u              Unexport pins initially ignoring errors.
           -v              Display verbose output.
           -x              Use XON/XOFF for OUTPUT.
## Notes
Here is a partial list of additional packages needed.

    sudo apt-get install pps-tools
    sudo apt-get install scons
    sudo apt-get install bison
    sudo apt-get install flex
    sudo apt-get install libssl-dev
    sudo apt-get install libcap-dev

Clone, build, and install Diminuto in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-diminuto
    cd com-diag-diminuto
    make
    make install

Clone, build, and install Obelisk in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-obelisk
    cd com-diag-obelisk
    make
    make install

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

Clone, build, and install GPS daemon in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://git.savannah.gnu.org/git/gpsd.git
    cd gpsd
    scons timeservice=yes nmea0183=yes prefix="/usr/local" pps=yes ntpshm=yes
    sudo scons install

These user identifiers in /etc/passwd may be required by the GPS daemon and
the NTP daemon; Obelisk runs as root.

    gpsd:x:111:20:GPSD system user,,,:/run/gpsd:/bin/false
    ntp:x:106:65534:,,,:/home/ntp:/bin/false

Run interactively for debugging.

    sudo wwvbtool -d -r -s -u -l

Run as a daemon in the background.

    sudo wwvbtool -b -r -s -u -l

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

Process Obelisk wwvbtool NMEA output using Hazer gpstool (for testing).

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
system. Run wwvbtool to write to the FIFO, gpsd to read from the FIFO,
and gpsmon to read from gpsd. gpsd, wwvbtool, and gpsmon are all run in
the foreground from three different windows.

    mkfifo wwvb.fifo
    gpsd -b -N -D 2 -n ./wwvb.fifo

    wwvbtool -N GP -r -s -u -l -n -p -O ./wwvb.fifo

    gpsmon

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
