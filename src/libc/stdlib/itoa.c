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

/* itoa.c - Integer to ascii */

#include <stdlib.h>
#include <vmx.h>

/***************************************************************************
 * itoa_r - recursive internal support routine for itoa()
 *
 * RETURNS: N/A
 */

LOCAL void itoa_r(
    unsigned  num,             /* known to be >= 0 */
    char **   string,          /* known to be non-NULL */
    unsigned  radix            /* 2 <= <radix> <= 36 */
    )
{
    unsigned  digit = num;

    if (num >= radix)                          /* Recursively call itoa_r() */
    {
        digit = num % radix;                   /* until <num> is 1-digit */
        itoa_r (num / radix, string, radix);
    }

    /* Put digit into string, advance ptr */
    **string = (digit < 10) ? '0' + digit : 'a' + digit;
    (*string)++;
}

/******************************************************************************
 * itoa - integer to ascii
 *
 * RETURNS: ptr to <string>
 */

char* itoa(
    int value,
    char *string,
    int radix
    )
{
    char      *ptr = string;
    unsigned  num;

    if (string == NULL)                  /* Do nothing if <string> is NULL */
    {
        return (NULL);
    }

    if ((radix < 2) || (radix > 36))     /* Return empty string if <radix> */
    {
        *ptr = '\0';                     /* is out of range. */
        return (string);
    }

    num = (unsigned) value;
    if ((radix == 10) && (value < 0))    /* Only base-10 may get a */
    {
        *ptr++ = '-';                    /* negative sign. */
        num = (unsigned) (-value);
    }

    itoa_r (num, &ptr, radix);
    *ptr = '\0';                         /* NULL terminate string. */

    return (string);
}

