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

/* atKbd.c - PC keyboard driver */

#include <string.h>
#include <vmx.h>
#include <sys/types.h>
#include <arch/regs.h>
#include <arch/esf.h>
#include <arch/iv.h>
#include <arch/sysArchLib.h>
#include <arch/intArchLib.h>

#include <drv/input/atKbd.h>

unsigned char kbdon[128];

/******************************************************************************
 * atKbdHandler - Keyboard interrupt handler
 *
 * RETURNS: N/A
 */

LOCAL void atKbdHandler(
    void *param
    )
{
    u_int8_t scancode, status;

    /* Read scancode from keyboard I/O port */
    scancode = sysInByte(0x60);
    status = sysInByte(0x61);

    /* Set bit 7 and write */
    sysOutByte(0x61, status | 0x80);
    sysOutByte(0x61, status);

    /* Check key up/down */
    if (scancode & 0x80)
    {
        /* Key was released */
        kbdon[scancode - 128] = 0;

    }
    else
    {
        /* Key was pressed */
        kbdon[scancode] = 1;
    }
}

/******************************************************************************
 * atKbdInit - Initialize keyboard
 *
 * RETURNS: N/A
 */

void atKbdInit(
    void
    )
{
    int i;

    /* Clear all keys */
    for (i = 0; i < 128; i++)
    {
        kbdon[i] = 0;
    }

    /* Install interrupt handler */
    intConnectFunction(0x21, atKbdHandler, (void *) 0);
}

