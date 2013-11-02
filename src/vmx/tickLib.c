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

/* tickLib.c - Tick library */

/* includes */

#include <sys/types.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/workQLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/tickLib.h>

/* globals */

volatile unsigned long sysTicks     = 0;
volatile u_int64_t     sysAbsTicks  = 0;

/******************************************************************************
 * tickLibInit - Initialize kernel tick library
 *
 * RETURNS: N/A
 */

void tickLibInit(
    void
    )
{
     /* Initialize variables */
     sysTicks = 0;
     sysAbsTicks = 0;
}

/***************************************************************************
 * tickAnnounce - timer tick handler
 *
 * RETURNS: N/A
 */

void tickAnnounce(
    void
    )
{
    if (kernelState == TRUE)
    {
        workQAdd0((FUNCPTR) vmxTickAnnounce);
    }
    else
    {
        kernelState = TRUE;
        vmxTickAnnounce();
        vmxExit();
    }
}

/******************************************************************************
 * tickSet - Set time
 *
 * RETURNS: N/A
 */

void tickSet(
    unsigned long ticks
    )
{
    int level;

    INT_LOCK(level);
    sysAbsTicks = ticks;
    INT_UNLOCK(level);
}

/******************************************************************************
 * tickGet - Get time
 *
 * RETURNS: N/A
 */

unsigned long tickGet(
    void
    )
{
    return (unsigned long) (sysAbsTicks & 0xffffffffull);
}

/******************************************************************************
 * tick64Set - Set time
 *
 * RETURNS: N/A
 */

void tick64Set(
    u_int64_t ticks
    )
{
    int level;

    INT_LOCK(level);
    sysAbsTicks = ticks;
    INT_UNLOCK(level);
}

/******************************************************************************
 * tick64Get - Get time
 *
 * RETURNS: N/A
 */

u_int64_t tick64Get(
    void
    )
{
    int level;
    u_int64_t ticks;

    INT_LOCK(level);
    ticks = sysAbsTicks;
    INT_UNLOCK(level);

    return ticks;
}

