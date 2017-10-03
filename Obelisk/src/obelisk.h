/* vim: set ts=4 expandtab shiftwidth=4: */
#ifndef _COM_DIAG_OBELISK_OBELISK_PRIVATE_H_
#define _COM_DIAG_OBELISK_OBELISK_PRIVATE_H_

/**
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock<BR>
 * mailto:coverclock@diag.com<BR>
 * http://github.com/coverclock/com-diag-obelisk<BR>
 */

/**
 * Convert Julian day of the year [1..365 or 366] to a Gregorian month
 * [1..12] and a day of the month [1..28, 29, 30, or 31]. If the function
 * fails (because the Julian day is out of range), the variables passed by
 * reference are not modified.
 * @param julian is the Julian day of the year [1..365 or 366].
 * @param lyi is true if it is a leap year, false otherwise.
 * @param monthp points to a field into which the month is stored.
 * @param dayp points to a field into which the day of the month is stored.
 * @return >= 0 for success, <0 otherwise.
 */
extern int obelisk_julian2gregorian(int julian, int lyi, int * monthp, int * dayp);

typedef enum ObeliskZeller {
    OBELISK_ZELLER_SUNDAY    = 0,
    OBELISK_ZELLER_MONDAY    = 1,
    OBELISK_ZELLER_TUESDAY   = 2,
    OBELISK_ZELLER_WEDNESDAY = 3,
    OBELISK_ZELLER_THURSDAY  = 4,
    OBELISK_ZELLER_FRIDAY    = 5,
    OBELISK_ZELLER_SATURDAY  = 6,
} obelisk_zeller_t;

/**
 * Computes a day of the week index [0..6] from a fully qualified year
 * (e.g. 2017), a month [1..12], and day of the month [1..28, 29, 30, or 31].
 * If the year, month, and day are not valid, the result is undefined.
 * @param year is the fully qualified year.
 * @param month is the month of the year.
 * @param day is the day of the month.
 * @return a day of the week index.
 */
extern obelisk_zeller_t obelisk_zeller(int year, int month, int day);

#endif /*  _COM_DIAG_OBELISK_OBELISK_PRIVATE_H_ */
