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

/* cksumLib.c - Checksum library */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <arch/arch.h>
#include <util/cksumLib.h>

/* Defines */

/* Imports */

/* Locals */

/* Globals */

/* Functions */

/******************************************************************************
 * checksum - Calculate checksum
 *
 * RETURNS: Checksum
 */

unsigned short checksum(
    unsigned short *addr,
    int             len
    )
{
    unsigned short  result;
    int             nLeft = len;
    int             sum   = 0;
    unsigned short *w     = addr;

    while (nLeft > 1)
    {
        sum   += *w++;
        nLeft -= 2;
    }

    if (nLeft == 1)
    {
#if _BYTE_ORDER == _BIG_ENDIAN
        sum += 0 | ((*(unsigned char *) w) << 8);
#else /* _BYTE_ORDER == _LITTLE_ENDIAN */
        sum += *(unsigned char *) w;
#endif /* _BYTE_ORDER */
    }

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = sum;

    return (~result & 0xffff);
}

