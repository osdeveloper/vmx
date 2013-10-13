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

/* memset.c - Fill memory */

#include <vmx.h>
#include <sys/types.h>

/******************************************************************************
 * memset - Set memory location with data
 *
 * RETURNS: Pointer to destination
 */

void* memset(
    void *dest,
    char data,
    size_t size
    )
{
    char *dp = (char *) dest;

    for (; size; size--)
    {
        *dp++ = data;
    }

    return dest;
}

/******************************************************************************
 * memsetw - Set memory location int work chunks with data
 *
 * RETURNS: Pointer to destination
 */

void* memsetw(
    short *dest,
    short data,
    size_t size
    )
{
    short *dp = (short *) dest;

    for (; size; size--)
    {
        *dp++ = data;
    }

    return dest;
}

