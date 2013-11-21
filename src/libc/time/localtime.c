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

/* localtime.c - Convert calendar time to broken-down time */

/* Includes */
#include <stdlib.h>
#include <time.h>
#include <private/timeP.h>
#include <vmx.h>

/* Defines */

/* Locals */

/* Globals */

/* Functions */

/******************************************************************************
 * localtime - Convert calendar time to broken-down time
 *
 * RETURNS: Pointer to time struct
 */

struct tm* localtime(
    const time_t *timer
    )
{
    static struct tm timeBuffer;

    localtime_r(timer, &timeBuffer);

    return &timeBuffer;
}

/******************************************************************************
 * localtime_r - Convert calendar time to broken-down time
 *
 * RETURNS: OK
 */

int localtime_r(
    const time_t *timer,
    struct tm    *timeBuffer
    )
{
    int dstOffset;
    char zoneBuffer[sizeof(ZONEBUFFER)];

    /* Get zone info */
    __getZoneInfo(zoneBuffer, TIMEOFFSET, __loctime);

    /* Create borken-down time structure */
    __getTime(*timer - ((atoi(zoneBuffer)) * SECSPERMIN), timeBuffer);

    /* Correct for daylight savings */
    dstOffset = __getDstInfo(timeBuffer, __loctime);
    timeBuffer->tm_isdst = dstOffset;
    if (dstOffset)
    {
        __getTime((*timer - ((atoi(zoneBuffer)) * SECSPERMIN)) +
                  (dstOffset * SECSPERHOUR), timeBuffer);
    }

    return OK;
}

