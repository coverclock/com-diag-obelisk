# com-diag-obelisk
Musings with WWVB and Inter-Range Instrumentation Group (IRIG) time codes.
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
## Notes

    systemctl enable timeservice

