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
#include <vmx/memPartLib.h>

/* Basic architecture dependent stuff */
#include <arch/sysArchLib.h>
#include <arch/i386/segArchLib.c>
#include <arch/i386/intArchLib.c>
#include <arch/i386/excArchLib.c>
#include <arch/i386/taskArchLib.c>

/* Kernel */
#include <vmx/kernLib.c>
#include <vmx/kernQLib.c>
#include <vmx/vmxLib.c>
#include <vmx/taskLib.c>
#include <vmx/kernHookLib.c>

#include <util/dllLib.c>
#include <util/qLib.c>
#include <util/qPrioLib.c>
#include <util/qFifoLib.c>

/* Os */
#include <vmx/errnoLib.c>
#include <vmx/objLib.c>
#include <vmx/classLib.c>
#include <vmx/listLib.c>
#include <vmx/semLib.c>
#include <vmx/semBLib.c>
#include <vmx/memPartLib.c>
#include <vmx/logLib.c>

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

#ifdef	 INCLUDE_LIBC
#include <libc/stdio/puts.c>
#include <libc/stdlib/itoa.c>
#include <libc/stdlib/itox.c>
#include <libc/stdlib/malloc.c>
#include <libc/stdlib/free.c>
#include <libc/string/memset.c>
#include <libc/string/memcpy.c>
#include <libc/string/strlen.c>
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

