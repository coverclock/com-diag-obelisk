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
signal. It is a work in progress. Obelisk, a.k.a. O-3, consists
of a small library and an application. The application runs on a
Raspberry Pi. It decodes the amplitude-modulated pulse-duration-encoded
binary-coded-decimal data stream from a SYM-RFT-60 radio receiver. WWVB is
a 60KHz long-wave transmitter near Fort Collins Colorado. The facility is
managed by the U.S. National Institute of Standards and Technology. WWVB
transmits the current time every minute. This time is disciplined by
atomic clocks at the transmitter site. These clocks are ultimately
synchronized to the master atomic clock at the NIST laboratories in
Boulder Colorado.  The application is built on top of the Obelisk library
and the Diminuto library. Both Obelisk and Diminuto are written in C.
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
## Usage
    $ wwvbtool -h
    usage: wwvbtool [ -H HOUR ] [ -L PATH ] [ -M MINUTE ] [ -P PIN ] [ -S PIN ] [ -T PIN ] [ -b ] [ -d ] [ -g ] [ -h ] [ -k ] [ -l ] [ -p ]  [ -r ] [ -s ] [ -u ] [ -v ]
           -H HOUR         Set time of day at HOUR local (1).
           -L PATH         Use PATH for lock directory ("/var/run/").
           -M MINUTE       Set time of day at MINUTE local (30).
           -P PIN          Use P1 output GPIO PIN (23).
           -S PIN          Use PPS output GPIO PIN (25).
           -T PIN          Use T input GPIO PIN (24).
           -b              Daemonize into the background.
           -d              Display debug output.
           -g              Send SIGHUP to the PID in the lock file and exit.
           -h              Display help menu and exit.
           -k              Send SIGTERM to the PID in the lock file and exit.
           -l              Remove the lock file initially ignoring errors.
           -p              Generate PPS output.
           -r              Reset device initially.
           -s              Set time of day when possible.
           -u              Unexport pins initially ignoring errors.
           -v              Display verbose output.
## Notes

    sudo wwvbtool -d -r -s -u -l

    sudo kill -HUP `cat /var/run/wwvbtool`

    sudo kill -TERM `cat /var/run/wwvbtool`

    sudo wwvbtool -b -r -s -u -l

    sudo wwvbtool -g

    sudo wwvbtool -k

    systemctl enable timeservice

    service timeservice start

    service timeservice stop

