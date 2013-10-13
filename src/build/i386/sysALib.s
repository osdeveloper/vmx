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

#include "config.h"

	/* Internals */
	.globl	GTEXT(sysInit)
	.globl	GTEXT(sysInByte)
	.globl	GTEXT(sysInWord)
	.globl	GTEXT(sysInLong)
	.globl	GTEXT(sysOutByte)
	.globl	GTEXT(sysOutWord)
	.globl	GTEXT(sysOutLong)
	.globl	GTEXT(sysInWordString)
	.globl	GTEXT(sysInLongString)
	.globl	GTEXT(sysOutLongString)
	.globl	GTEXT(sysOutWordString)
	.globl	GTEXT(sysWait)
	.globl	GTEXT(sysReboot)

	/* Externals */
	.globl	GTEXT(usrInit)

	.text
	.balign	16

/*****************************************************************************
* sysInit - This is the os entry point, fix stack load gdt transfer
* control to usrInit in usrConfig.c
*
* RETURNS:   N/A
* PROTOTYPE: void sysInit(void)
*
*****************************************************************************/

FUNC_LABEL(sysInit)
	/* Initialize stack */

	cli				/* Disable interrupts */
	movl	$ FUNC(sysInit),%esp	/* Initialize stack pointer */
	xorl	%ebp,%ebp		/* Initialize stack frame */

	/* Start os */

	call	FUNC(usrInit)		/* Call usrInit */
	iret

/*****************************************************************************
* Include rest of assembler code
*****************************************************************************/

/* Basic architecure dependent stuff */
#include <arch/i386/sysALib.s>
#include <arch/i386/segALib.s>
#include <arch/i386/intALib.s>
#include <arch/i386/excALib.s>
#include <arch/i386/kernALib.s>

