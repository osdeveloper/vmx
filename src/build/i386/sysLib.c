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
#include <vmx/memPartLib.h>

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

void sysHwInit0(
    void
    )
{
    segBaseSet(sysGdt);
}

void sysHwInit(
    void
    )
{
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

