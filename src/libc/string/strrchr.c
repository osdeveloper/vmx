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

/* strrchr.c - Find character in string revesed */

#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>

/******************************************************************************
 * strrchr - Find character in string revesed
 *
 * RETURNS: Pointer to character or NULL
 */

char* strrchr(
    const  char *s,
    int c
    )
{
    const char *p;

    p = NULL;
    do
    {
        if (*s == (char) c)
        {
            p = s;
        }
    } while (*s++);

    return (char *) p;                     /* silence the warning */
}

