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

/* excALib.h - Exception handling in I80X86 assambly routines */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>
#include <arch/iv.h>

	/* Externals */
	.globl	GTEXT(excExcHandle)
	.globl	GTEXT(kernIntEnt)
	.globl	GTEXT(kernIntExit)
	
	/* Internals */
	.globl	GTEXT(excCallTbl)
	.globl	GTEXT(excStub)
	.globl	GTEXT(excIntStub)

	.text
	.balign	16

/*************************************************************************
* excCallTbl - Table of calls
*
**************************************************************************/

FUNC_LABEL(excCallTbl)			/* Description:		     Err:*/
        call    FUNC(excStub)           /* Division By Zero	     N/A */
        call    FUNC(excStub)           /* Debug 		     N/A */
        call    FUNC(excStub)           /* Non Maskable Interrupt    N/A */
        call    FUNC(excStub)           /* Breakpoint 		     N/A */
        call    FUNC(excStub)           /* Into Detected Overflow    N/A */
        call    FUNC(excStub)           /* Out of Bounds 	     N/A */
        call    FUNC(excStub)           /* Invalid Opcode	     N/A */
        call    FUNC(excStub)           /* Device Not Avilable	     N/A */
        call    FUNC(excStub)           /* Double Fault		     YES */
        call    FUNC(excStub)           /* Co-Processor Seg. Overrun N/A */
        call    FUNC(excStub)           /* Bad TSS	       	     YES */
        call    FUNC(excStub)           /* Segment Not Present 	     YES */
        call    FUNC(excStub)           /* Stack Fault		     YES */
        call    FUNC(excStub)           /* General Protection Fault  YES */
        call    FUNC(excStub)           /* Page Fault		     YES */
        call    FUNC(excStub)           /* Unknown Interrupt	     N/A */
        call    FUNC(excStub)           /* Co-processor Fault	     N/A */
        call    FUNC(excStub)           /* Alignment Check 486	     N/A */
        call    FUNC(excStub)           /* Machine Check Pentium     N/A */
        call    FUNC(excStub)           /* Streaming SIMD 	     N/A */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excStub)           /* unassigned reserved 		 */
        call    FUNC(excIntStub)        /* User interrputs */
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)
        call    FUNC(excIntStub)

/*************************************************************************
* excStub - Exception handler stub
*
**************************************************************************/

	.balign	16,0x90
FUNC_LABEL(excStub)
	pushfl				/* Push flags register */
	pushal				/* Push CPU regs */
	movl	%esp, %ebx		/* Store regs pointer */

	/* Calculate offset in Call table */

	movl	0x24(%esp),%eax		/* Get Call return address */
	subl	$4,%eax			/* Make return addr call addr */

	subl	$ FUNC(excCallTbl),%eax	/* Get offset in Call table */

	movl	$5,%ecx			/* Calculate exception num */

	cltd
	idivl	%ecx			/* %eax = exception num */

	/* Branch depending on if exception pushes an error code */

	cmpl	$ IN_DOUBLE_FAULT,%eax
	jl	excStub1		/* vecNum < 8 */
	cmpl	$ IN_CP_OVERRUN,%eax
	je	excStub1		/* vecNum == 9 */
	cmpl	$ IN_RESERVED,%eax
	je	excStub1		/* vecNum == 15 */
	cmpl	$ IN_CP_ERROR,%eax
	je	excStub1		/* vecNum == 16 */
	cmpl	$ IN_MACHINE_CHECK,%eax
	je	excStub1		/* vecNum == 18 */
	cmpl	$ IN_SIMD,%eax
	je	excStub1		/* vecNum == 19

	/* Exception stack has error-code */

	movl	0x2c(%esp),%edx		/* Get pc */
	movl	%edx,0x24(%esp)		/* replace return addr by pc */
	pushl	$ TRUE			/* Push error code flag */
	pushl	%ebx			/* Push REG_SET pointer */
	addl	$0x28,%ebx
	pushl	%ebx			/* Push pointer to ESF */
	pushl	%eax			/* Push exception num */
	call	FUNC(excExcHandle)	/* Call exception handler */
	addl	$16,%esp		/* Clean up pushed arguments */

	popal				/* Restore regs */
	addl	$12,%esp		/* Get ESF pointer */
	iret

	.balign	16,0x90
excStub1:
	/* Exception has no error code */

	movl	0x28(%esp),%edx		/* Get pc */
	movl	%edx,0x24(%esp)		/* Replace return addr with pc */

	pushl	$ FALSE			/* Push no error code */
	pushl	%ebx			/* Push REG_SET pointer */
	addl	$0x28, %ebx
	pushl	%ebx			/* Push ESF pointer */
	pushl	%eax			/* Push exception number */
	call	FUNC(excExcHandle)	/* Call exception handler */
	addl	$16,%esp		/* Clean up pushed arguments */

	popal				/* Restore regs */
	addl	$8,%esp			/* Get ESF pointer */
	iret

/*************************************************************************
* excIntStub - Exception handler stub
*
**************************************************************************/

	.balign	16,0x90
FUNC_LABEL(excIntStub)
	popl	%eax			/* Save return address */
	call	FUNC(kernIntEnt)	/* Call interrupt entry func */
	pushl	%eax			/* Push pc return address */

	/* Create REG_SET struct */

	pushfl				/* Push eflags */
	pushal				/* Push CPU regs */
	movl	%esp,%ebx		/* Save pointer to regs */

	/* Calculate interrupt number from address in Call table */

	movl	0x24(%esp),%eax		/* Get Call table ret addr */
	subl	$4,%eax			/* Adjust return to be Call addr */

	subl	$ FUNC(excCallTbl),%eax	/* Get offset fron Call table */

	movl	$5,%ecx			/* Turn vector offset to intnum */

	cltd
	idivl	%ecx			/* %eax = interrupt num */

	/* Interrupts pushes no error code */

	movl	0x28+4(%esp),%edx	/* Get pc from ESF */
	movl	%edx,0x24(%esp)		/* Replace return addr by pc */

	pushl	$ FALSE			/* No error code */
	pushl	%ebx			/* Push pointer to REG_SET */
	addl	$0x28+4,%ebx		/* Get address to ESF */

	pushl	%ebx			/* Push pointer to ESF */
	pushl	%eax			/* Push interrupt number */
	call	FUNC(intIntHandle)	/* Call default handler */
	addl	$16,%esp		/* Clean up args */

	popal				/* Restore regs */
	addl	$8,%esp			/* Jump over eflags */
	jmp	FUNC(kernIntExit)
	
