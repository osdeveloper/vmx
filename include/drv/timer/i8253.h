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

/* i8253.h - Intel 8253 Programmable Interrupt Timer (PIT) */

#ifndef _i8253_h
#define _i8253_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */
#define PIT_ADRS(base, reg)             (base + reg * PIT_REG_ADDR_INTERVAL)
#define PIT_CNT0(base)                  PIT_ADRS(base, 0x00)
#define PIT_CNT1(base)                  PIT_ADRS(base, 0x01)
#define PIT_CNT2(base)                  PIT_ADRS(base, 0x02)
#define PIT_CMD(base)                   PIT_ADRS(base, 0x03)

/* Functions */
/******************************************************************************
 * sysClockConnect - Connect an interrupt handler routine to clock
 *
 * RETURNS: OK
 */

STATUS sysClockConnect(
    FUNCPTR func,
    int arg
    );

/******************************************************************************
 * sysClockInt - System interrupt clock handler routine
 *
 * RETURNS: N/A
 */

void sysClockInt(
    void
    );

/******************************************************************************
 * sysClockEnable - Start interrupt clock
 *
 * RETURNS: N/A
 */

void sysClockEnable(
    void
    );

/******************************************************************************
 * sysClockDisable - Disable interrput clock
 *
 * RETURNS: N/A
 */

void sysClockDisable(
    void
    );

/******************************************************************************
 * sysClockRateSet - Set system clock rate in ticks per second
 *
 * RETURNS: OK or ERROR
 */

STATUS sysClockRateSet(
    int ticksPerSecond
    );

/******************************************************************************
 * sysClockRateGet - Get system clock rate in ticks per second
 *
 * RETURNS: Ticks per second
 */

int sysClockRateGet(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _i8253_h */

