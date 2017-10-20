/* vim: set ts=4 expandtab shiftwidth=4: */
#ifndef _COM_DIAG_OBELISK_DATABASES_H_
#define _COM_DIAG_OBELISK_DATABASES_H_

/**
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock<BR>
 * mailto:coverclock@diag.com<BR>
 * http://github.com/coverclock/com-diag-obelisk<BR>
 */

static const const char * TOKEN[] = {
    "ZERO",     /* OBELISK_TOKEN_ZERO */
    "ONE",      /* OBELISK_TOKEN_ONE */
    "MARKER",   /* OBELISK_TOKEN_MARKER */
    "INVALID",  /* OBELISK_TOKEN_INVALID */
};

static const char * STATE[] = {
    "START",    /* OBELISK_STATE_START */
    "WAIT",     /* OBELISK_STATE_WAIT */
    "SYNC",     /* OBELISK_STATE_SYNC */
    "DATA",     /* OBELISK_STATE_DATA */
    "MARK",     /* OBELISK_STATE_MARK */
    "END",      /* OBELISK_STATE_END */
    "BEGIN",    /* OBELISK_STATE_BEGIN */
    "LEAP",     /* OBELISK_STATE_LEAP */
};

static const char * STATUS[] = {
    "WAITING",  /* OBELISK_STATUS_WAITING */
    "NOMINAL",  /* OBELISK_STATUS_NOMINAL */
    "INVALID",  /* OBELISK_STATUS_INVALID */
    "TIME",     /* OBELISK_STATUS_TIME */
    "FRAME",    /* OBELISK_STATUS_FRAME */
    "LEAP",     /* OBELISK_STATUS_LEAP */
};

static const char * DAY[] = {
    "SUN",	/* Sunday */
    "MON",	/* Monday */
    "TUE",	/* Tuesday */
    "WED",	/* Wednesday */
    "THU",	/* THursday */
    "FRI",	/* Friday */
    "SAT",	/* Saturday */
};

#endif /*  _COM_DIAG_OBELISK_DATABASES_H_ */
