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

/* mktime.c - Convert broken-down time to calendar time */

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
 * mktime - Convert broken-down time to calendar time
 *
 * RETURNS: Time
 */

time_t mktime(
    struct tm *pTime
    )
{
    char zoneBuffer[sizeof(ZONEBUFFER)];
    time_t timeIs = 0;
    int    days   = 0;

    /* Validate time struct */
    __tmValidate(pTime);

    /* Calulcate time value */
    timeIs += pTime->tm_sec +
              pTime->tm_min * SECSPERMIN +
              pTime->tm_hour * SECSPERHOUR;

    /* Get date */
    days = __julday(pTime->tm_year, pTime->tm_mon, pTime->tm_mday);

    pTime->tm_yday = (days - 1);

    if ((pTime->tm_year + TM_YEAR_BASE) < EPOCH_YEAR)
    {
        timeIs = (time_t) ERROR;
    }
    else
    {
        /* Days previous year */
        days = __daysSinceEpoch(
                   pTime->tm_year - (EPOCH_YEAR - TM_YEAR_BASE),
                   pTime->tm_yday
                   );

        pTime->tm_wday = (days + EPOCH_YEAR) % DAYSPERWEEK;

        timeIs += days * SECSPERDAY;

        /* Correct for daylight savings */
        pTime->tm_isdst = __getDstInfo(pTime, __loctime);
        if (pTime->tm_isdst)
        {
            timeIs -= SECSPERHOUR;
            __tmValidate(pTime);
        }

        /* Correct from zone offset for UTC */
        __getZoneInfo(zoneBuffer, TIMEOFFSET, __loctime);
        timeIs += ( atoi(zoneBuffer) * SECSPERMIN );
    }

    return timeIs;
}

