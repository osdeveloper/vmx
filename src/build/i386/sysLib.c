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

#ifdef	DEBUG
#undef  INCLUDE_VGA
#define INCLUDE_VGA
#undef  INCLUDE_LIBC
#define INCLUDE_LIBC
#endif

#ifdef   INCLUDE_VGA
#include <drv/video/vga.c> 
#endif

#ifdef 	 INCLUDE_ATKBD
#include <drv/input/atKbd.c>
#endif

#ifdef 	 INCLUDE_PIC
#include <drv/intrCtl/i8259Pic.c>
#endif

u_int32_t sysIntIdtType	= SYS_INT_TRAPGATE;
u_int32_t sysVectorIRQ0	= INT_NUM_IRQ0;
SEGDESC   *sysGdt	= (SEGDESC *) (LOCAL_MEM_LOCAL_ADRS + GDT_BASE_OFFSET);
CALL_GATE *sysIdt	= (CALL_GATE *) (VEC_BASE_ADRS);

void sysHwInit(void)
{

#ifdef INCLUDE_VGA
  vgaInit();
#endif

#ifdef INCLUDE_ATKBD
  atKbdInit();
#endif

  segBaseSet(sysGdt);
  intVecBaseSet((FUNCPTR *) sysIdt);
  excVecInit();

#ifdef INCLUDE_PIC
  sysIntInitPIC();
#endif
}

