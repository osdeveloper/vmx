/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* sysLib.c - System dependent C code */

#include "config.h"

#include <sys/types.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <arch/mmuArchLib.h>
#include <os/vmLib.h>

u_int32_t sysIntIdtType	= SYS_INT_TRAPGATE;
u_int32_t sysVectorIRQ0	= INT_NUM_IRQ0;
GDT	  *sysGdt	= (GDT *) (LOCAL_MEM_LOCAL_ADRS + GDT_BASE_OFFSET);
CALL_GATE *sysIdt	= (CALL_GATE *) (VEC_BASE_ADRS);

#include <drv/intrCtl/i8259Pic.c>
#include <drv/timer/i8253Timer.c>

/* Console */
#ifdef   INCLUDE_PC_CONSOLE
#include <drv/input/englishKeymap.c>
#include <drv/input/i8042Kbd.c>
#include <drv/video/latin1CharMap.c>
#include <drv/video/m6845Vga.c>
#include <drv/serial/pcConsole.c>
PC_CON_DEV pcConDev[N_VIRTUAL_CONSOLES];
#endif   /* INCLUDE_PC_CONSOLE */

#ifdef INCLUDE_RAM_DRV
#include <drv/disk/ramDrv.c>
#endif /* INCLUDE_RAM_DRV */

/* Dma driver */
#include <drv/dma/i8237Dma.c>

/* Floppy driver */
#ifdef INCLUDE_FD_DRV
unsigned int sysFdBuf     = FD_DMA_BUF_ADDR;
unsigned int sysFdBufSize = FD_DMA_BUF_SIZE;
#include <drv/fdisk/nec765Fd.c>
#endif /* INCLUDE_FD_DRV */

#ifdef INCLUDE_NET
#include <drv/netif/if_loop.c>
#endif /* INCLUDE_NET */

int sysPhysMemDescNumEntries   = 0;
PHYS_MEM_DESC sysPhysMemDesc[] =
{
    /* First memory page */
    {
        /* Virtual address */
        (void *) 0x00000000,

        /* Physical address */
        (void *) 0x00000000,

        /* Length */
        VM_PAGE_OFFSET,

#ifdef DEBUG_NULL_ACCESS
        /* Initial mask */
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,

        /* Initial state */
        VM_STATE_NOT_VALID  | VM_STATE_NOT_WRITABLE  | VM_STATE_NOT_CACHEABLE
#else
        /* Initial mask */
        VM_STATE_MASK_FOR_ALL,

        /* Initial state */
        VM_STATE_FOR_MEM_OS
#endif /* DEBUG_NULL_ACCESS */

    },

    /* Lower memory valid access */
    {
        /* Virtual address */
        (void *) VM_PAGE_OFFSET,

        /* Physical address */
        (void *) VM_PAGE_OFFSET,

        /* Length */
        0xa0000 - VM_PAGE_OFFSET,

        /* Initial mask */
        VM_STATE_MASK_FOR_ALL,

        /* Initial state */
        VM_STATE_FOR_MEM_OS
    },

    /* Video ram */
    {
        /* Virtual address */
        (void *) 0x000a0000,

        /* Physical address */
        (void *) 0x000a0000,

        /* Length */
        0x00060000,

        /* Initial mask */
        VM_STATE_MASK_FOR_ALL,

        /* Initial state */
        VM_STATE_FOR_IO
    },

    /* Operating system memory */
    {
        /* Virtual address */
        (void *) LOCAL_MEM_LOCAL_ADRS,

        /* Physical address */
        (void *) LOCAL_MEM_LOCAL_ADRS,

        /* Length */
        LOCAL_MEM_SIZE_OS,

        /* Initial mask */
        VM_STATE_MASK_FOR_ALL,

        /* Initial state */
        VM_STATE_FOR_MEM_OS
    },

    /* Application memory */
    {
        /* Virtual address */
        (void *) LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE_OS,

        /* Physical address */
        (void *) LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE_OS,

        /* Length */
        LOCAL_MEM_SIZE - LOCAL_MEM_SIZE_OS,

        /* Initial mask */
        VM_STATE_MASK_FOR_ALL,

        /* Initial state */
        VM_STATE_FOR_MEM_APPLICATION
    },

    {
        (void *) VM_INVALID_ADDR,
        (void *) VM_INVALID_ADDR,
        0,
        0,
        0
    }
};

void sysHwInit0(
    void
    )
{
    regsGdtInit(sysGdt);
}

void sysHwInit(
    void
    )
{
#ifdef INCLUDE_MMU
    int            i;
    PHYS_MEM_DESC *pVm;

    /* Initialize number of virtual memory descriptos */
    pVm = &sysPhysMemDesc[0];

    /* For all virtual memory descriptos */
    for (i = 0; i < NELEMENTS(sysPhysMemDesc); i++)
    {
        if ((int) pVm->vAddr != (int) VM_INVALID_ADDR)
        {
            pVm++;
        }
        else
        {
            break;
        }
    }

    /* Store number of descriptors */
    sysPhysMemDescNumEntries = i;
#endif /* INCLUDE_MMU */

    sysIntInitPIC();
    sysIntEnablePIC(PIT0_INT_LVL);
}

void sysHwInit2(
    void
    )
{
    /* Connect system timer interrupt handler */
    intConnectDefault(0x20, sysClockInt, NULL);

#ifdef INCLUDE_PC_CONSOLE
    intConnectDefault(0x21, kbdIntr, (void *) 0);
#endif /* INCLUDE_PC_CONSOLE */
}

/******************************************************************************
 * sysDelay - Delay routine
 *
 * RETURNS: N/A
 */

void sysDelay(
    void
    )
{
    (void) sysInByte(UNUSED_ISA_IO_ADDRESS);
}

