#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Arvada Colorado USA.
# Compute the average offset for the GPS (0) and PPS (1) reference clock.
# <http://catb.org/gpsd/gpsd-time-service-howto.html>
# <https://docs.ntpsec.org/latest/ntp_conf.html>
ROOT=${1-"/var/log/ntpstats/peerstats"}
cat ${ROOT}.* | awk '
    BEGIN { tot0 = 0; num0 = 0; tot1 = 0; num1 = 0; }
    $3 == "SHM(0)" { tot0 += $5; num0++; avg0 = tot0 / num0; print $3, $5, tot0, num0, avg0; }
    $3 == "SHM(1)" { tot1 += $5; num1++; avg1 = tot1 / num1; print $3, $5, tot1, num1, avg1; }
    END { print "SHM(0) time1", 0 - avg0; print "SHM(1) time1", 0 - avg1; }
'
