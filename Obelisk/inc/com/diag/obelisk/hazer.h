/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _COM_DIAG_OBELISK_HAZER_H_
#define _COM_DIAG_OBELISK_HAZER_H_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file was derived frmo the Digital Aggregates Corporation Hazer package.
 * Hazer is a simple C-based parser of the National Marine Electronics
 * Association (NMEA) strings produced by typical consumer GPS devices.
 *
 * REFERENCES
 *
 * "NMEA 0183 Standard for Interfacing Marine Electronic Devices", version 4.10,
 * NMEA 0183, National Marine Electronics Association, 2012-06
 *
 * "BU-353S4 GPS Receiver Data Sheet", BU353S4-DS08212013B, USGLobalSat Inc.,
 * 2013
 *
 * "NMEA Reference Manual", Revision 2.2, 1050-0042, SiRF Technology, Inc.,
 * 2008-11
 *
 * "SiRF Binary Protocol Reference Manual", revision 2.4, 1040-0041, SiRF
 * Technology, Inc., 2008-11
 *
 * "GP-2106 SiRF Star IV GPS module with antenna", version 0.2, ADH Technology
 * Co. Ltd., 2010-12-08
 *
 * Electronic Doberman, "Modern GPS Teardown - GlobalSat BU-353S4 SiRF Star
 * IV USB GPS", https://www.youtube.com/watch?v=8xn8FspJDnY
 *
 * E. Kaplan, ed., UNDERSTANDING GPS PRINCIPLES AND APPLICATIONS, Artech House,
 * 1996
 *
 * "Geographic coordinate system", Wikipedia,
 * https://en.wikipedia.org/wiki/Geographic_coordinate_system, 2017-01-24
 *
 * "Decimal degrees", Wikipedia,
 * https://en.wikipedia.org/wiki/Decimal_degrees, 2016-11-04
 *
 * "Points of the compass", Wikipedia,
 * https://en.wikipedia.org/wiki/Points_of_the_compass, 2017-01-17
 */

#include <stddef.h>
#include <stdint.h>

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 * NMEA 0183 4.10, 5.3
 *
 * SiRF NMEA, p. 2-2 has an example which appears to violate the
 * NMEA spec as to the length of the message ID.
 *
 * The USGlobalSat ND-105C routinely violates the NMEA spec as to
 * the maximum message length of 79 characters between the initial
 * '$' and the terminating \r\n by (so far) one character.
 */
enum HazerConstant {
    HAZER_CONSTANT_NMEA_LONGEST     = 85,
    HAZER_CONSTANT_NMEA_TALKER      = sizeof("GP") - 1,
};

/**
 * This buffer is large enough to contain the largest NMEA sentence,
 * according to the NMEA spec, plus a trailing NUL.
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef char (hazer_buffer_t)[HAZER_CONSTANT_NMEA_LONGEST + 1]; /* plus NUL */

/**
 * NMEA state machine stimuli. This is just the special characters that
 * the state machine must take different action on, not all possible
 * characters that may be in an NMEA sentence.
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
enum HazerStimulus {
    HAZER_STIMULUS_START            = '$',
    HAZER_STIMULUS_CHECKSUM         = '*',
};

/**
 * @def HAZER_NMEA_GPS_TALKER
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_RADIO_TALKER "ZV"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_RMC
 * SiRF NMEA, Table 1-2, Time, date position, course, and speed data
 */
#define HAZER_NMEA_GPS_MESSAGE_RMC "RMC"

/**
 * Compute the checksum of an NMEA sentence.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @return the checksum.
 */
extern uint8_t hazer_checksum(const void * buffer, size_t size);

/**
 * Given an eight-bit checksum, concert into the two checksum characters.
 * @param ck is the checksum.
 * @param mnsp points where the most significant character is stored.
 * @param lnsp points where the least significant character is stored.
 * @return 0 for success, <0 if an error occurred.
 */
extern int hazer_checksum2characters(uint8_t ck, char * msnp, char * lsnp);

#endif
