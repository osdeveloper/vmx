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

/* timeLib.c - Ansi time library */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <private/timeP.h>
#include <vmx.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL TIMELOCALE __ctime =
{
    {
        /* Days short name */
        "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",

        /* Days long name */
        "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY",
        "THURSDAY", "FRIDAY", "SATURDAY"
    },

    /* Moths short names */
    {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL",
        "AUG", "SEP", "OCT", "NOV", "DEC",

        /* Moths long name */
        "JANUARY", "FEBRUARI", "MARS", "APRIL", "MAY",
        "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER",
        "NOVEMBER", "DECEMBER"
    },

    /* Format */
    { "%b %d %H:%M:%S:%Y", "%b %d %Y", "%H:%M:%S" },

    /* AM/PM */
    { "AM", "PM" },

    /* Zones */
    { "UTC", "UTC", "" },

    /* Dst */
    { "", "" }
};

LOCAL void __tItostr(
    char *buffer,
    int   num,
    int   size
    );

/* Globals */
TIMELOCALE *__loctime = &__ctime;

/* Functions */

/******************************************************************************
 * timeLibInit - Initialize time library
 *
 * RETURNS: N/A
 */

void timeLibInit(
    void
    )
{
}

/******************************************************************************
 * __julday - CalculateJulian day given year, moth and day
 *
 * RETURNS: Julian day
 */

int __julday(
    int yr,
    int mon,
    int day
    )
{
    static int jdays[12] =
    {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    int leap = 0;

    if (isleap(yr + TM_YEAR_BASE))
    {
        if (mon > 1)
        {
            leap = 1;
        }
    }

    return (jdays[mon] + day + leap);
}

/******************************************************************************
 * __daysSinceEpoch - Calculate number days since ANSI C epoch
 *
 * RETURNS: Days since epoch
 */

int __daysSinceEpoch(
    int year,
    int yday
    )
{
    int ret;

    if (year >= 0)
    {
        ret = (365 * year) + (year + 1) / 4 + yday;
    }
    else
    {
        ret = (365 * year) + (year - 2) / 4 + yday;
    }

    return ret;
}

/******************************************************************************
 * __tmNormalize - Reduce time unit to range [0,base]
 *
 * RETURNS: N/A
 */

void __tmNormalize(
    int *tens,
    int *units,
    int  base
    )
{
    *tens += *units / base;

    *units %= base;
    if ((*units % base) < 0)
    {
        (*tens)--;
        *units += base;
    }
}

/******************************************************************************
 * __tmValidate - Validate broken down structure
 *
 * RETURNS: N/A
 */

void __tmValidate(
    struct tm *pTime
    )
{
    struct tm t;
    int       jday;
    int       mon;

    /* Set local structure */
    t = *pTime;

    /* Normalize */
    __tmNormalize(&t.tm_min, &t.tm_sec, SECSPERMIN);
    __tmNormalize(&t.tm_hour, &t.tm_min, MINSPERHOUR);
    __tmNormalize(&t.tm_mday, &t.tm_hour, HOURSPERDAY);
    __tmNormalize(&t.tm_year, &t.tm_mon, MONSPERYEAR);

    /* Calulcate julian day */
    jday = __julday(t.tm_year, t.tm_mon, t.tm_mday);

    /* Calulcate month and day */
    for (mon = 0;
         (jday > __julday(t.tm_year, mon + 1, 0)) && (mon < 11);
         mon++);

    t.tm_mon = mon;
    t.tm_mday = jday - __julday(t.tm_year, mon, 0);
    t.tm_wday = 0;
    t.tm_yday = 0;

    *pTime = t;
}

/******************************************************************************
 * __getDstInfo - Determine if daylight savings are in effect
 *
 * RETURNS: TRUE or FALSE
 */

int __getDstInfo(
    struct tm  *timeNow,
    TIMELOCALE *timeInfo
    )
{
    char  numstr[2];
    char  buffer[sizeof(ZONEBUFFER)];
    char *start    = NULL;
    char *end      = NULL;
    char *last     = "";
    char *tmp      = NULL;
    char *envInfo  = NULL;
    int   monStart = 0;
    int   monEnd   = 0;
    int   dayStart = 0;
    int   dayEnd   = 0;
    int   hrStart  = 0;
    int   hrEnd    = 0;

    /* If timezone set */
    envInfo = getenv("TIMEZONE");
    if (envInfo != NULL)
    {
        strcpy(buffer, envInfo);
        tmp   = strtok_r(buffer, ":", &last);
        tmp   = strtok_r(NULL, ":", &last);
        start = strtok_r(NULL, ":", &last);
        end   = strtok_r(NULL, ":", &last);
    }
    else
    {
        start = timeInfo->_Isdst[DSTON];
        end   = timeInfo->_Isdst[DSTOFF];

        if ((strcmp(start, "") == 0) || (strcmp(end, " ") == 0))
        {
            return FALSE;
        }
    }

    if ((start == NULL) || (end == NULL))
    {
        return FALSE;
    }

    /* Scan information of the form mmddhh */
    monStart = (atoi(strncpy(numstr, start, 2))) - 1;
    monEnd   = atoi(strncpy(numstr, end, 2)) - 1;
    if ((timeNow ->tm_mon < monStart) || (timeNow->tm_mon > monEnd))
    {
        return FALSE;
    }

    if ((timeNow->tm_mon == monStart) || (timeNow->tm_mon == monEnd))
    {
        dayStart = atoi(strncpy(numstr, start + 2, 2));
        dayEnd   = atoi(strncpy(numstr, end + 2, 2));

        if (((timeNow->tm_mon == monStart) && (timeNow->tm_mday < dayStart)) ||
            ((timeNow->tm_mon == monEnd) && (timeNow->tm_mday == dayEnd)))
        {
            hrStart = atoi(strncpy(numstr, start + 4, 2));
            hrEnd   = atoi(strncpy(numstr, end + 4, 2));
            if (((timeNow->tm_mon == monStart) &&
                 (timeNow->tm_mday == dayStart) &&
                 (timeNow->tm_hour < hrStart)) ||
                ((timeNow->tm_mon == monEnd) &&
                 (timeNow->tm_mday == dayEnd) &&
                 (timeNow->tm_hour > hrEnd)))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return TRUE;

}

/******************************************************************************
 * __getZoneInfo - Determine minute time difference from UTC
 *
 * RETURNS: TRUE or FALSE
 */

void __getZoneInfo(
    char       *buffer,
    int         option,
    TIMELOCALE *timeInfo
    )
{
    char *pLimitStart;
    char *pLimitEnd;
    char *envInfo = getenv("TIMEZONE");

    /* If timezone set */
    if ((envInfo != NULL) && (strcmp(envInfo, "") != 0))
    {
        pLimitEnd = strpbrk(envInfo, ":");
        if (option == NAME)
        {
            strcpy(buffer, envInfo);
            *(buffer + (pLimitEnd - envInfo)) = EOS;
            return;
        }

        pLimitStart = pLimitEnd + 1;
        pLimitEnd   = strpbrk(pLimitStart, ":");

        if (option == NAME2)
        {
            strcpy(buffer, pLimitStart);
            *(buffer + (pLimitEnd - pLimitStart)) = EOS;
            return;
        }

        pLimitStart = pLimitEnd + 1;
        pLimitEnd   = strpbrk(pLimitStart, ":");

        if (option == TIMEOFFSET)
        {
            strcpy(buffer, pLimitStart);
            if (pLimitEnd != NULL)
            {
                *(buffer + (pLimitEnd - pLimitStart)) = EOS;
            }

            return;
        }
    }
    else
    {
        if (strcmp(timeInfo->_Zone[option], "") != 0)
        {
            strcpy(buffer, timeInfo->_Zone[option]);
        }
        else
        {
            *buffer = EOS;
        }
    }
}

/******************************************************************************
 * __getTime - Convert calendar time to broken-down time
 *
 * RETURNS: OK
 */

int __getTime(
    const time_t timer,
    struct tm   *tmp
    )
{
    int    days;
    int    timeOfDay;
    int    year;
    int    mon;
    ldiv_t result;

    /* Calculate days since epoch */
    days = timer / SECSPERDAY;
    timeOfDay = timer % SECSPERDAY;
    if (timeOfDay < 0)
    {
        timeOfDay += SECSPERDAY;
        days      -= 1;
    }

    /* Calculate years since epoch */
    year = days / DAYSPERYEAR;
    while (__daysSinceEpoch(year, 0) > days)
    {
        year--;
    }

    /* Calculcate weekday */
    tmp->tm_wday = (days + EPOCH_WDAY) % DAYSPERWEEK;
    if (tmp->tm_wday < 0)
    {
        tmp->tm_wday += DAYSPERWEEK;
    }

    /* Find year and days left */
    days -= __daysSinceEpoch(year, 0);
    year += EPOCH_YEAR;

    /* Calculate month */
    for (mon = 0;
         (days >= __julday(year - TM_YEAR_BASE, mon + 1, 0)) && (mon < 11);
         mon++);

    /* Setup time structure */
    tmp->tm_year = year - TM_YEAR_BASE;
    tmp->tm_mon  = mon;
    tmp->tm_mday = (days - __julday(tmp->tm_year, mon, 0)) + 1;
    tmp->tm_yday = __julday(tmp->tm_year, mon, tmp->tm_mday) - 1;
    tmp->tm_hour = timeOfDay / SECSPERHOUR;

    timeOfDay %= SECSPERHOUR;
    ldiv_r((long) timeOfDay, (long) SECSPERMIN, &result);
    tmp->tm_min = result.quot;
    tmp->tm_sec = result.rem;

    return OK;
}

/******************************************************************************
 * __generateTime - Generate time string
 *
 * RETURNS: N/A
 */

void __generateTime(
    char            *buffer,
    const struct tm *tmp,
    TIMELOCALE      *timeInfo,
    int             *pos,
    const char      *fmt
    )
{
    /* Select format */
    switch(*fmt)
    {
        /* Day short name*/
        case 'a':
            __getDay(buffer, tmp->tm_wday, ABBR, timeInfo, pos);
            break;

        /* Day full name */
        case 'A':
            __getDay(buffer, tmp->tm_wday, FULL, timeInfo, pos);
            break;

        /* Month short name */
        case 'b':
            __getMonth(buffer, tmp->tm_mon, ABBR, timeInfo, pos);
            break;

        /* Month long name */
        case 'B':
            __getMonth(buffer, tmp->tm_mon, FULL, timeInfo, pos);
            break;

        /* Date and time */
        case 'c':
            __getLocale(buffer, DATETIME, tmp, timeInfo, pos);
            break;

        /* Day of month */
        case 'd':
            __tItostr(buffer, tmp->tm_mday, *pos = 2);
            break;

        /* Hour */
        case 'H':
            __tItostr(buffer, tmp->tm_hour, *pos = 2);
            break;

        /* Hour */
        case 'I':
            __tItostr(
                buffer,
                ((tmp->tm_hour % 12) ? (tmp->tm_hour % 12) : 12),
                *pos = 2
                );
            break;

        /* Day of year */
        case 'j':
            __tItostr(buffer, tmp->tm_yday + 1, *pos = 3);
            break;

        /* Month */
        case 'm':
            __tItostr(buffer, tmp->tm_mon, *pos = 2);
            break;

        /* Minute */
        case 'M':
            __tItostr(buffer, tmp->tm_min, *pos = 2);
            break;

        /* Am/pm */
        case 'p':
            __getLocale(buffer, AMPM, tmp, timeInfo, pos);
            break;

        /* Second */
        case 'S':
            __tItostr(buffer, tmp->tm_sec, *pos = 2);
            break;

        /* Week number */
        case 'U':
            __tItostr(
                buffer,
                __weekOfYear(TM_SUNDAY, tmp->tm_wday, tmp->tm_yday),
                *pos = 2
                );
            break;

        /* Weekday */
        case 'w':
            __tItostr(buffer, tmp->tm_wday, *pos = 1);
            break;

        /* Week number */
        case 'W':
            __tItostr(
                buffer,
                __weekOfYear(TM_MONDAY, tmp->tm_wday, tmp->tm_yday),
                *pos = 2
                );
            break;

        /* Date */
        case 'x':
            __getLocale(buffer, DATE, tmp, timeInfo, pos);
            break;

        /* Time */
        case 'X':
            __getLocale(buffer, TIMEO, tmp, timeInfo, pos);
            break;

        /* Year */
        case 'y':
            __tItostr(buffer, tmp->tm_year % CENTURY, *pos = 2);
            break;

        /* Year */
        case 'Y':
            __tItostr(buffer, tmp->tm_year + TM_YEAR_BASE, *pos = 4);
            break;

        /* Zone */
        case 'Z':
            __getLocale(buffer, ZONE, tmp, timeInfo, pos);
            break;

        /* Operator */
        case '%':
            memcpy(buffer, "%", 1);
            *pos = 1;
            break;

        default:
            *pos = 0;
            break;
    }
}

/******************************************************************************
 * __getLocale - Get locale presentation for requested item
 *
 * RETURNS: N/A
 */

void __getLocale(
    char            *buffer,
    int              desc,
    const struct tm *tmp,
    TIMELOCALE      *timeInfo,
    int *size
    )
{
    char  zoneBuffer[sizeof(ZONEBUFFER)];
    char *ptr = NULL;

    /* Select item */
    switch (desc)
    {
        case DATETIME:
            ptr = timeInfo->_Format[DATETIME];
            break;

        case DATE:
            ptr = timeInfo->_Format[DATE];
            break;

        case TIMEO:
            ptr = timeInfo->_Format[TIMEO];
            break;

        case AMPM:
            ptr = timeInfo->_Ampm[(tmp->tm_hour < 12) ? 0 : 1];
            break;

        case ZONE:
            __getZoneInfo(zoneBuffer, NAME, timeInfo);
            ptr = zoneBuffer;
            break;
    }

    /* Set size */
    *size = strlen(ptr);

    /* Get string */
    strcpy(buffer, ptr);
}

/******************************************************************************
 * __getMonth - Get locale presentation of month
 *
 * RETURNS: N/A
 */

void __getMonth(
    char       *buffer,
    int         index,
    int         abbr,
    TIMELOCALE *timeInfo,
    int        *size
    )
{
    char *pStr;

    /* Initialize locals */
    pStr = timeInfo->_Months[index];

    if (abbr != ABBR)
    {
        index += MONSPERYEAR;
    }

    *size = strlen(pStr);
    strcpy(buffer, pStr);
}

/******************************************************************************
 * __getDay - Get locale presentation of weekday
 *
 * RETURNS: N/A
 */

void __getDay(
    char       *buffer,
    int         index,
    int         abbr,
    TIMELOCALE *timeInfo,
    int        *size
    )
{
    char *pStr;

    /* Initialize locals */
    pStr = timeInfo->_Days[index];

    if (abbr != ABBR)
    {
        index += DAYSPERWEEK;
    }

    *size = strlen(pStr);
    strcpy(buffer, pStr);
}

/******************************************************************************
 * __weekOfYear - Calculate week number of the year
 *
 * RETURNS: Week number
 */

int __weekOfYear(
    int start,
    int wday,
    int yday
    )
{
    wday = (wday - start + DAYSPERWEEK) % DAYSPERWEEK;
    return (yday - wday + 12) / DAYSPERWEEK - 1;
}

/******************************************************************************
 * tItostr - Convert integer to string
 *
 * RETURNS: N/A
 */

LOCAL void __tItostr(
    char *buffer,
    int   num,
    int   size
    )
{
    if (num < 0)
    {
        num = 0;
    }

    for (buffer += size; 0 <= --size; num /= 10)
    {
        *--buffer = num % 10 + '0';
    }
}

