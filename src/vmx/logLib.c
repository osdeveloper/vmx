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

/* logLib.c - Logging library */

#include <stdio.h>
#include <vmx/logLib.h>

unsigned logFlags = 0;
unsigned logLevel = 0;

/******************************************************************************
 * setLogFlags - Setup  logging flags
 *
 * RETURNS: N/A
 */

void setLogFlags(
    unsigned flags
    )
{
    logFlags = flags;
}

/******************************************************************************
 * setLogLevel - Setup  logging level
 *
 * RETURNS: N/A
 */

void setLogLevel(
    int level
    )
{
    logLevel = level;
}

/******************************************************************************
 * logString - Print log string
 *
 * RETURNS: N/A
 */

void logString(
    const char *str,
    unsigned flags,
    int level
    )
{
    if ((flags & logFlags) && (level & logLevel))
    {
        printf("%s\n", str);
    }
}

/******************************************************************************
 * logStringAndInteger - Print log string followed by integer
 *
 * RETURNS: N/A
 */

void logStringAndInteger(
    const char *str,
    int value,
    unsigned flags,
    int level
    )
{
    if ((flags & logFlags) && (level & logLevel))
    {
        printf("%s: %d\n", str, value);
    }
}

/******************************************************************************
 * logStringAndAddress - Print log string followed by an address
 *
 * RETURNS: N/A
 */

void logStringAndAddress(
    const char *str,
    unsigned value,
    unsigned flags,
    int level
    )
{
    if ((flags & logFlags) && (level & logLevel))
    {
        printf("%s @%x\n", str, value);
    }
}

