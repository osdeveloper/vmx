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

/* memcmp.h - Compare memory */

#include <stdlib.h>
#include <vmx.h>
#include <sys/types.h>

/******************************************************************************
 * memcmp - Comapre contents in two different memory locations
 *
 * RETURNS: The dirrerance between match or zero
 */

int memcmp(
    const void *s1,
    const void *s2,
    size_t len
    )
{
    unsigned char *c1 = (unsigned char *)s1;
    unsigned char *c2 = (unsigned char *)s2;

    while (len--)
    {
        if (*c1 != *c2)
        {
            return *c1 - *c2;
        }
        c1++;
        c2++;
    }

    return 0;
}

