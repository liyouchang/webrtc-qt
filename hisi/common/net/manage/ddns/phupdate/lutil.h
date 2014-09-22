/*
 * Copyright 1998,1999 The OpenLDAP Foundation, Redwood City, California, USA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted only
 * as authorized by the OpenLDAP Public License.  A copy of this
 * license is available at http://www.OpenLDAP.org/license.html or
 * in file LICENSE in the top-level directory of the distribution.
 */
#ifndef _LUTIL_H_
#define _LUTIL_H_
#include <sys/types.h>

/* base64.c */
extern int lutil_b64_ntop(unsigned char const *, size_t, char *, size_t);
extern int lutil_b64_pton(char const *, unsigned char *, size_t);
#endif

