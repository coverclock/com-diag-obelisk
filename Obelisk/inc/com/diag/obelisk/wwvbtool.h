/* vim: set ts=4 expandtab shiftwidth=4: */
#ifndef _COM_DIAG_OBELISK_WWVBTOOL_H_
#define _COM_DIAG_OBELISK_WWVBTOOL_H_

/**
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
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

static const char * EVENT[] = {
    "WAITING",  /* OBELISK_EVENT_WAITING */
    "NOMINAL",  /* OBELISK_EVENT_NOMINAL */
    "INVALID",  /* OBELISK_EVENT_INVALID */
    "TIME",     /* OBELISK_EVENT_TIME */
    "FRAME",    /* OBELISK_EVENT_FRAME */
    "LEAP",     /* OBELISK_EVENT_LEAP */
};

static const char * DAY[] = {
    "SUN",		/* Sunday */
    "MON",		/* Monday */
    "TUE",		/* Tuesday */
    "WED",		/* Wednesday */
    "THU",		/* Thursday */
    "FRI",		/* Friday */
    "SAT",		/* Saturday */
};

static const char DST[] = {
	'-',		/* OBELISK_DST_OFF */
	'<',		/* OBELISK_DST_ENDS */
	'>',		/* OBELISK_DST_BEGINS */
	'+',		/* OBELISK_DST_ON */
};

static const char SIGN[] = {
	'?',
	'?',
	'-',		/* OBELISK_SIGN_NEGATIVE */
	'?',
	'?',
	'+',		/* OBELISK_SIGN_POSITIVE */
	'?',
	'?',
};

#endif /*  _COM_DIAG_OBELISK_WWVBTOOL_H_ */
