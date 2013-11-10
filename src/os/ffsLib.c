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

/* ffsLib.c - find first set bit library */

#include <vmx.h>
#include <os/ffsLib.h>

LOCAL unsigned char msbTable[256] =
{
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,   /* 0x00 - 0x0f */
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,   /* 0x10 - 0x1f */
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,   /* 0x20 - 0x2f */
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,   /* 0x30 - 0x3f */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   /* 0x40 - 0x4f */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   /* 0x50 - 0x5f */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   /* 0x60 - 0x6f */
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   /* 0x70 - 0x7f */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0x80 - 0x8f */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0x90 - 0x9f */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0xa0 - 0xaf */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0xb0 - 0xbf */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0xc0 - 0xcf */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0xd0 - 0xdf */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,   /* 0xe0 - 0xef */
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8    /* 0xf0 - 0xff */
};

/******************************************************************************
 * ffsMsb - find first set most significant bit
 *
 * 1 indicates the least significant bit, 32 the most.
 *
 * RETURNS: position of most-significant bit, 0 if no bits set
 */

int ffsMsb(
    unsigned  value
    )
{
    unsigned short upper;
    unsigned char high;
    int result;

    if ((upper = (value >> 16)) == 0)
    {
        /* Bit in lower 16 bits may be set. */
        if ((high = (value >> 8)) == 0)
        {
            result = msbTable[value];
        }
        else
        {
            result = msbTable[high] + 8;
        }
    }
    else
    {
        /* Bit in upper 16 bits is set */
        if ((high = (upper >> 8)) == 0)
        {
            result = msbTable[upper] + 16;
        }
        else
        {
            result = msbTable[high] + 24;
        }
    }

    return result;
}

/******************************************************************************
 * ffsLsb - find first set least significant bit
 *
 * 1 indicates the least significant bit, 32 the most.
 *
 * RETURNS: position of least-significant bit, 0 if no bits set
 */

int ffsLsb(
    unsigned value
    )
{
    unsigned tmp = value;

    value ^= (value - 1);          /* Wipe out all bits above LSB */

    /* If <value> was 0, it is now <0xffffffff>.  ANDing fixes that. */

    return ffsMsb(value & tmp);
}

