/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#ifndef _ejs_date_h_
#define _ejs_date_h_

#include <time.h>
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};
#include "ejs-object.h"

typedef struct {
    /* object header */
    EJSObject obj;

    /* date specific data */
    EJSBool valid;
    struct timeval tv;
    struct timezone tz;
} EJSDate;


EJS_BEGIN_DECLS;

extern ejsval _ejs_Date;
extern ejsval _ejs_Date_prototype;
extern EJSSpecOps _ejs_Date_specops;

double _ejs_date_get_time (EJSDate* date);

void _ejs_date_init(ejsval global);

EJS_END_DECLS;

#endif /* _ejs_date_h */
