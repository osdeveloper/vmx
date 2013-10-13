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

/* sysALib.s - System dependent assebler code */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>
#include <stdlib.h>

#include "config.h"

	/* Globals */
	.global	GTEXT(sysInit)
	.global	GTEXT(intPrioTable)
	.global	GTEXT(intPrioTableSize)

	/* Externals */
	.extern	GTEXT(usrInit)
	.extern	GDATA(intLockTaskSR)

	.text

/******************************************************************************
* sysInit - OS start point
*
* RETURNS: N/A
******************************************************************************/

	.align	ALIGN_TEXT
	.type	FUNC(sysInit),@function

FUNC_LABEL(sysInit)
	/* Disable interrupts */
	mov.l	IntLockTaskSR,r1
	mov.l	@r1,r0
	ldc	r0,sr

	/* Setup system startup stack */
	mov.l	SysInit,sp

	/* Jump to usrInit */
	mov.l	UsrInit,r0
	xor	r1,r1
	jsr	@r0

	/* Just for niceness */
	ldc	r1,gbr

	.align	ALIGN_DATA
IntLockTaskSR:	.long	FUNC(intLockTaskSR)
SysInit:	.long	FUNC(sysInit)
UsrInit:	.long	FUNC(usrInit)

#ifndef INT_LVL_SYSCLK
#error  INT_LVL_SYSCLK not defined
#endif

#ifndef INT_LVL_AUXCLK
#error  INT_LVL_AUXCLK not defined
#endif

#ifndef INT_LVL_TSTAMP
#error  INT_LVL_TSTAMP not defined
#endif

#ifndef INT_LVL_SCI
#error  INT_LVL_SCI not defined
#endif

#ifndef INT_LVL_SCIF
#error  INT_LVL_SCIF not defined
#endif

/*******************************************************************************
* intPrioTable - Interrupts priotity table
*******************************************************************************/

	.align	ALIGN_DATA
	.type	FUNC(intPrioTable),@object

FUNC_LABEL(intPrioTable)			/* EXPEVT INTEVT */
	.long	NULL				/* 0x000: NOT USED */
	.long	NULL				/* 0x020: NOT USED */
	.long	0x400000f0			/* 0x040: TLB instr/data(r) */
	.long	0x400000f0			/* 0x060: TBL instr/data(w) */
	.long	0x400000f0			/* 0x080: Init page write */
	.long	0x400000f0			/* 0x0a0: TBL data(r) prot */
	.long	0x400000f0			/* 0x0c0: TLB data(w) prot */
	.long	0x400000f0			/* 0x0e0: Err addr(r) */
	.long	0x400000f0			/* 0x100: Err addr(w) */
	.long	0x400000f0			/* 0x120: FPU exc */
	.long	0x400000f0			/* 0x140: TBL instr/data(m) */
	.long	0x400000f0			/* 0x160: Uncond trap */
	.long	0x400000f0			/* 0x180: Illegal instr */
	.long	0x400000f0			/* 0x1a0: Illegal slot instr */
	.long	NULL				/* 0x1c0: NMI */
	.long	0x400000f0			/* 0x1e0: Usr break trap */
	.long	0x400000f0			/* 0x200: IRL15 */
	.long	0x400000e0			/* 0x220: IRL14 */
	.long	0x400000d0			/* 0x240: IRL13 */
	.long	0x400000c0			/* 0x260: IRL12 */
	.long	0x400000b0			/* 0x280: IRL11 */
	.long	0x400000a0			/* 0x2a0: IRL10 */
	.long	0x40000090			/* 0x2c0: IRL9 */
	.long	0x40000080			/* 0x2e0: IRL8 */
	.long	0x40000070			/* 0x300: IRL7 */
	.long	0x40000060			/* 0x320: IRL6 */
	.long	0x40000050			/* 0x340: IRL5 */
	.long	0x40000040			/* 0x360: IRL4 */
	.long	0x40000030			/* 0x380: IRL3 */
	.long	0x40000020			/* 0x3a0: IRL2 */
	.long	0x40000010			/* 0x3c0: IRL1 */
	.long	0x400000f0			/* 0x3e0: Reserved */
	.long	0x40000000|(INT_LVL_SYSCLK<<4)	/* 0x400: TMU0_UNDEFLOW */
	.long	0x40000000|(INT_LVL_AUXCLK<<4)	/* 0x420: TMU1_UNDEFLOW */
	.long	0x40000000|(INT_LVL_TSTAMP<<4)	/* 0x440: TMU2_UNDEFLOW */
	.long	0x400000f0			/* 0x460: TMU2_INPUT_CAPTURE */
	.long	0x400000f0			/* 0x480: RTC_ALARM */
	.long	0x400000f0			/* 0x4a0: RTC_PERIODIC */
	.long	0x400000f0			/* 0x4c0: RTC_CARRY */
	.long	0x40000000|(INT_LVL_SCI<<4)	/* 0x4e0: SCI_RX_ERROR */
	.long	0x40000000|(INT_LVL_SCI<<4)	/* 0x500: SCI_RX */
	.long	0x40000000|(INT_LVL_SCI<<4)	/* 0x520: SCI_TX */
	.long	0x40000000|(INT_LVL_SCI<<4)	/* 0x540: SCI_TX_ERROR */
	.long	0x400000f0			/* 0x560: WDT_INTERVAL_TIMER */
	.long	0x400000f0			/* 0x580: BSC_REFRESH_CMI */
	.long	0x400000f0			/* 0x5a0: BSC_REFRESH_OVF */
	.long	0x400000f0			/* 0x5c0: Reserved */
	.long	0x400000f0			/* 0x5e0: Reserved */
	.long	0x400000f0			/* 0x600: Hitachi-UDI */
	.long	0x400000f0			/* 0x620: Reserved */
	.long	0x400000f0			/* 0x640: DMAC DMTE0 */
	.long	0x400000f0			/* 0x660: DMAC DMTE1 */
	.long	0x400000f0			/* 0x680: DMAC DMTE2 */
	.long	0x400000f0			/* 0x6a0: DMAC DMTE3 */
	.long	0x400000f0			/* 0x6c0: DMAC DMAE */
	.long	0x400000f0			/* 0x6e0: Reserved */
	.long	0x40000000|(INT_LVL_SCIF<<4)	/* 0x700: SCIF_RX_ERROR */
	.long	0x40000000|(INT_LVL_SCIF<<4)	/* 0x720: SCIF_RX */
	.long	0x40000000|(INT_LVL_SCIF<<4)	/* 0x740: SCIF_TX */
	.long	0x40000000|(INT_LVL_SCIF<<4)	/* 0x760: SCIF_TX_ERROR */
	.long	0x400000f0			/* 0x780: Reserved */
	.long	0x400000f0			/* 0x7a0: Reserved */
	.long	0x400000f0			/* 0x7c0: Reserved */
	.long	0x400000f0			/* 0x7e0: Reserved */
	.long	0x400000f0			/* 0x800: FPU disable */
	.long	0x400000f0			/* 0x820: FPU slot disable */
FUNC_LABEL(intPrioTableEnd)			/* EXPEVT INTEVT */

	.align	ALIGN_DATA
	.type	FUNC(intPrioTableSize),@object
	.size	FUNC(intPrioTableSize),4
FUNC_LABEL(intPrioTableSize)
	.long	FUNC(intPrioTableEnd) - FUNC(intPrioTable)

/*******************************************************************************
* Include rest of assembler files
*******************************************************************************/

/* Basic architecure dependent stuff */
#include <arch/sh/excALib.s>
#include <arch/sh/intALib.s>
#include <arch/sh/kernALib.s>
#include <arch/sh/sysALib.s>

