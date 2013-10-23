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

/* i8253Timer.c - Intel 8253 Programmable Interrupt Timer (PIT) */

/* Includes */
#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <drv/timer/i8253.h>

/* Defines */

/* Imports */
IMPORT void sysHwInit2(void);

/* Locals */
LOCAL FUNCPTR sysClockFunc = NULL;
LOCAL int sysClockArg = 0;
LOCAL BOOL sysClockRunning = FALSE;
LOCAL BOOL sysClockConnected = FALSE;
LOCAL int sysClockTicksPerSecond = 60;

/* Globals */

/* Functions */

/******************************************************************************
 * sysClockConnect - Connect an interrupt handler routine to clock
 *
 * RETURNS: OK
 */

STATUS sysClockConnect(
    FUNCPTR func,
    int arg
    )
{
    /* If first call */
    if (sysClockConnected != TRUE)
    {
        sysHwInit2();
    }

    /* Store interrupt routine and argument */
    sysClockFunc = func;
    sysClockArg = arg;

    /* Mark as connected */
    sysClockConnected = TRUE;

    return OK;
}

/******************************************************************************
 * sysClockInt - System interrupt clock handler routine
 *
 * RETURNS: N/A
 */

void sysClockInt(
    void
    )
{
    /* Call interrupt acknoledge routine if set */
    if (sysClockFunc != NULL)
    {
        (*sysClockFunc)(sysClockArg);
    }
}

/******************************************************************************
 * sysClockEnable - Start interrupt clock
 *
 * RETURNS: N/A
 */

void sysClockEnable(
    void
    )
{
    u_int32_t tr;
    int level;

    /* If clock is not running */
    if (sysClockRunning != TRUE)
    {
        tr = PIT_CLOCK / sysClockTicksPerSecond;

        /* Lock interrupts */
        INT_LOCK(level);

        /* Setup clock */
        sysOutByte(PIT_CMD(PIT_BASE_ADR), 0x36);
        sysOutByte(PIT_CNT0(PIT_BASE_ADR), LSB(tr));
        sysOutByte(PIT_CNT0(PIT_BASE_ADR), MSB(tr));

        /* Unlock interrupts */
        INT_UNLOCK(level);

        /* Enable clock interrupt */
        sysIntEnablePIC(PIT0_INT_LVL);

        /* Mark clock as running */
        sysClockRunning = TRUE;

    }
}

/******************************************************************************
 * sysClockDisable - Disable interrput clock
 *
 * RETURNS: N/A
 */

void sysClockDisable(
    void
    )
{
    int level;

    /* If clock running */
    if (sysClockRunning == TRUE)
    {
        /* Disable interrupts */
        INT_LOCK(level);

        /* Stop clock */
        sysOutByte( PIT_CMD(PIT_BASE_ADR), 0x38);
        sysOutByte( PIT_CNT0(PIT_BASE_ADR), LSB(0));
        sysOutByte( PIT_CNT0(PIT_BASE_ADR), MSB(0));

        /* Note that interrupts won't occur util sysClockEnable() is called */

        /* Disable clock interrupt */
        sysIntDisablePIC(PIT0_INT_LVL);

        /* Mark clock as not running */
        sysClockRunning = FALSE;
  }
}

/******************************************************************************
 * sysClockRateSet - Set system clock rate in ticks per second
 *
 * RETURNS: OK or ERROR
 */

STATUS sysClockRateSet(
    int ticksPerSecond
    )
{
    STATUS status;

    /* Check clock rate interval */
    if ((ticksPerSecond < SYS_CLOCK_RATE_MIN) ||
        (ticksPerSecond > SYS_CLOCK_RATE_MAX))
    {
        status = ERROR;
    }
    else
    {
        /* Update global tick rate variable */
        sysClockTicksPerSecond = ticksPerSecond;

        /* If clock is running */
        if (sysClockRunning == TRUE)
        {
            sysClockDisable();
            sysClockEnable();
        }
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sysClockRateGet - Get system clock rate in ticks per second
 *
 * RETURNS: Ticks per second
 */

int sysClockRateGet(
    void
    )
{
    return sysClockTicksPerSecond;
}

