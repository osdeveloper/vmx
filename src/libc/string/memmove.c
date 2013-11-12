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

/* memmove.c - Move between memory areas */

#include <string.h>
#include <vmx.h>

/******************************************************************************
 * memmove - Move between memory areas
 *
 * RETURNS: Pointer to destdination
 */

void* memmove(
    void *dst,
    const void *src,
    size_t len
    )
{
    char *s1 = dst, *s2 = (char *) src;

    /* This bit of sneakyness c/o Glibc, it assumes the test is unsigned */
    if (s1 - s2 >= len)
    {
        return memcpy(dst, src, len);
    }

    /* This reverse copy only used if we absolutly have to */
    s1 += len;
    s2 += len;
    while (len-- > 0)
    {
        *(--s1) = *(--s2);
    }

    return dst;
}

