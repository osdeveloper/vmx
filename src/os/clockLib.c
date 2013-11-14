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

/* clockLib.c - Clock library */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <vmx/tickLib.h>
#include <os/private/timerLibP.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL BOOL clockLibInstalled = FALSE;

/* Globals */
CLOCK _clockRealtime;

/* Functions */

/******************************************************************************
 * clockLibInit - Initialize clock library
 *
 * RETURNS: OK or ERROR
 */

STATUS clockLibInit(
    void
    )
{
    STATUS status;

    if (clockLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        if (sysClockRateGet() < 1)
        {
            status = ERROR;
        }

        memset(&_clockRealtime, 0, sizeof(CLOCK));
        _clockRealtime.tickBase         = tick64Get();
        _clockRealtime.timeBase.tv_sec  = 0;
        _clockRealtime.timeBase.tv_nsec = 0;

        clockLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * clock_settime - Set time
 *
 * RETURNS: OK or ERROR
 */

int clock_settime(
    clockid_t              id,
    const struct timespec *tp
    )
{
    int ret;

    if (id != CLOCK_REALTIME)
    {
        errnoSet(EINVAL);
        ret = ERROR;
    }
    else
    {
        if ((tp == NULL) || (tp->tv_sec < 0) || (tp->tv_nsec >= BILLION))
        {
            errnoSet(EINVAL);
            ret = ERROR;
        }
        else
        {
            _clockRealtime.tickBase = tick64Get();
            _clockRealtime.timeBase = *tp;
            ret = OK;
        }
    }

    return ret;
}

/******************************************************************************
 * clock_gettime - Get time
 *
 * RETURNS: OK or ERROR
 */

int clock_gettime(
    clockid_t        id,
    struct timespec *tp
    )
{
    int       ret;
    u_int64_t diff;

    if (id != CLOCK_REALTIME)
    {
        errnoSet(EINVAL);
        ret = ERROR;
    }
    else
    {
        if (tp == NULL)
        {
            errnoSet(EINVAL);
            ret = ERROR;
        }
        else
        {
            diff = tick64Get() - _clockRealtime.tickBase;
            TV_CONVERT_TO_SEC(*tp, diff);
            TV_ADD(*tp, _clockRealtime.timeBase);
            ret = OK;
        }
    }

    return ret;
}

