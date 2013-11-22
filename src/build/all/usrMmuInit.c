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

/* usrMmuInit.c - Virual memory initialization */

/* Includes */
#include "config.h"

#include <stdlib.h>
#include <vmx.h>
#include <arch/sysArchLib.h>
#include <arch/mmuArchLib.h>
#include <os/vmLib.h>

/* Defines */

/* Imports */
IMPORT PHYS_MEM_DESC sysPhysMemDesc[];
IMPORT int           sysPhysMemDescNumEntries;

/* Locals */

/* Globals */

/* Functions */

/******************************************************************************
 * usrMmuInit - Initialize virtual memory
 *
 * RETURNS: N/A
 */

void usrMmuInit(void)
{
    /* If initialization of mmu unit fails */
    if (mmuPro32LibInit(VM_PAGE_SIZE) != OK)
    {
        printExc(
            "usrRoot: MMU configuration failed, errno = %#x", errno,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
        sysReboot();
    }

    /* In initialization of virual memory */
    if ((vmLibInit(VM_PAGE_SIZE) != OK) ||
        (vmGlobalMapInit(
             &sysPhysMemDesc[0],
             sysPhysMemDescNumEntries,
             TRUE
             ) == NULL))
    {
        printExc(
            "usrRoot: MMU configuration failed, errno = %#x", errno,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
        sysReboot();
    }
}

