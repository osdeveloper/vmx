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

/* gets.c - Get string from stdin */

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>

/******************************************************************************
 * gets - Get string from stdin
 *
 * RETURNS: Pointer to string
 */

char* gets(
    char *buf
    )
{
    int c;
    char *str = buf;
    char *ret = buf;

    /* While not end-of-line */
    while ((c = getchar()) != '\n')
    {
        /* If end-of-file */
        if (c == EOF)
        {
            if (str == buf)
            {
                ret = NULL;
            }

            break;
        }
        else
        {
            *str++ = c;
        }
    }

    if (ret != NULL)
    {
        /* Put end-of-string at end */
        *str = EOS;
    }

    return ret;
}

