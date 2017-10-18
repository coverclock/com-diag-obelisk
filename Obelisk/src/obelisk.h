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
 * When we parse the IRIQ timecode frame using a finite state machine,
 * these are the actions that the FSM may perform.
 */
typedef enum ObeliskAction {
    OBELISK_ACTION_NONE,    /* Do nothing. */
    OBELISK_ACTION_CLEAR,   /* Empty frame. */
    OBELISK_ACTION_ZERO,    /* Insert 0. */
    OBELISK_ACTION_ONE,     /* Insert 1. */
    OBELISK_ACTION_LEAP,    /* Leap second indicated. */
    OBELISK_ACTION_MARK,    /* Insert MARK. */
    OBELISK_ACTION_FINAL,   /* Complete frame. */
    OBELISK_ACTION_FIRST = OBELISK_ACTION_NONE,
    OBELISK_ACTION_LAST = OBELISK_ACTION_FINAL,
} obelisk_action_t;

/**
 * These are the bit offsets of the fields in the IRIQ timecode buffer.
 */
typedef enum ObeliskOffset {
     OBELISK_OFFSET_MINUTES10   = 56,
     OBELISK_OFFSET_MINUTES1    = 51,
     OBELISK_OFFSET_HOURS10     = 46,
     OBELISK_OFFSET_HOURS1      = 41,
     OBELISK_OFFSET_DAY100      = 36,
     OBELISK_OFFSET_DAY10       = 31,
     OBELISK_OFFSET_DAY1        = 26,
     OBELISK_OFFSET_DUTONESIGN  = 21,
     OBELISK_OFFSET_DUTONE1     = 16,
     OBELISK_OFFSET_YEAR10      = 11,
     OBELISK_OFFSET_YEAR1       = 6,
     OBELISK_OFFSET_LYI         = 4,
     OBELISK_OFFSET_LSW         = 3,
     OBELISK_OFFSET_DST         = 1,
} obelisk_offset_t;

/**
 * These are the bit masks of the fields in the IRIQ timecode buffer.
 */
typedef enum ObeliskMask {
    OBELISK_MASK_MINUTES10    = 0x7,
    OBELISK_MASK_MINUTES1     = 0xf,
    OBELISK_MASK_HOURS10      = 0x3,
    OBELISK_MASK_HOURS1       = 0xf,
    OBELISK_MASK_DAY100       = 0x3,
    OBELISK_MASK_DAY10        = 0xf,
    OBELISK_MASK_DAY1         = 0xf,
    OBELISK_MASK_DUTONESIGN   = 0x7,
    OBELISK_MASK_DUTONE1      = 0xf,
    OBELISK_MASK_YEAR10       = 0xf,
    OBELISK_MASK_YEAR1        = 0xf,
    OBELISK_MASK_LYI          = 0x1,
    OBELISK_MASK_LSW          = 0x1,
    OBELISK_MASK_DST          = 0x3,
} obelisk_mask_t;

/**
 * @def OBELISK_EXTRACT
 * This generates code to extract a single field from the buffer. The field
 * name is turned into an bit offset and a bit mask.
 */
#define OBELISK_EXTRACT(_BUFFER_, _FIELD_) ((_BUFFER_ >> OBELISK_OFFSET_ ## _FIELD_) & OBELISK_MASK_ ## _FIELD_)

/**
 * Extract the individual IRIQ timecode fields from the buffer and store
 * them in a frame.
 * @param framep points to the output frame structure.
 * @param buffer is the input buffer.
 */
extern void obelisk_extract(obelisk_frame_t * framep, obelisk_buffer_t buffer);

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

/**
 * This describes how the Zeller algorithm encodes the day of the week.
 */
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
