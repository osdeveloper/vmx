/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
 *
 *   Real VMX is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Real VMX is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TIME_H
#define _TIME_H

/* includes */

#include <sys/types.h>

/* defines */
#define CLOCK_REALTIME                  0x00            /* System time */
#define CLOCK_ABSTIME                   0x01            /* Absolute time */

/* typedefs */

/* structs */

struct tm
{
    int  tm_sec;    /* seconds           - [0..60]  */
    int  tm_min;    /* minutes           - [0..59]  */
    int  tm_hour;   /* hours             - [0..23]  */
    int  tm_mday;   /* day of the month  - [1..31]  */
    int  tm_mon;    /* month of the year - [0..11]  */
    int  tm_year;   /* years since 1900.            */
    int  tm_wday;   /* day of the week   - [0..6]   */
    int  tm_yday;   /* day of the year   - [0..365] */
    int  tm_isdst;  /* daylight savings flag        */
};

struct timespec
{
    time_t  tv_sec;
    long    tv_nsec;
};

struct itimerspec
{
    struct timespec  it_interval;    /* timer period */
    struct timespec  it_value;       /* timer expiration */
};

#endif

