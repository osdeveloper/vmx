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

/* mmuALib.s - Memory mapping unit for 386+ assmebler routines */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>

        /* Internals */
        .globl  GTEXT(mmuI386Enable)
        .globl  GTEXT(mmuI386PdbrSet)
        .globl  GTEXT(mmuI386PdbrGet)
        .globl  GTEXT(mmuI386TlbFlush)

        .text
        .balign 16

/******************************************************************************
 * mmuI386Enable - Enable/Disable MMU
 *
 * RETURNS TRUE or FALSE
 */

        .balign 16, 0x90

FUNC_LABEL(mmuI386Enable)
        pushfl                          /* Store flags */
        cli                             /* Disable interrupts */
        movl    SP_ARG1+4(%esp), %edx
        movl    %cr0, %eax
        movl    %edx, FUNC(mmuEnabled)
        cmpl    $0, %edx
        je      mmuDisable
        orl     $0x80010000, %eax       /* Set PG/WP */
        jmp     mmuEnable0

mmuDisable:
        andl    $0x7ffeffff, %eax       /* Disable PG/WP */

mmuEnable0:
        movl    %eax,%cr0
        jmp     mmuEnable1

mmuEnable1:
        movl    $0, %eax
        popfl                           /* Enable interrupts */
        ret

/******************************************************************************
 * mmuI386PdbrSet - Setup page directory register
 *
 * RETURNS N/A
 */

        .balign 16, 0x90

FUNC_LABEL(mmuI386PdbrSet)
        pushfl                          /* Store flags */
        cli                             /* Disable interrupts */
        movl    SP_ARG1+4(%esp), %eax
        movl    (%eax), %eax
        movl    %cr3, %edx
        movl    $0xfffff000, %ecx
        andl    $0x00000fff, %edx

#if     (CPU==I80386)
        jmp     mmuPdbrSet0
#endif

        movl    $0xffffffe0, %ecx
        andl    $0x00000007, %edx

mmuPdbrSet0:
        andl    %ecx, %eax
        orl     %edx, %eax
        movl    %eax, %cr3
        jmp     mmuPdbrSet1

mmuPdbrSet1:
        popfl                           /* Enable interrupts */
        ret

/******************************************************************************
 * mmuI386PdbrGet - Get page directory register
 *
 * RETURNS Pointer to mmu translation table
 */

        .balign 16, 0x90

FUNC_LABEL(mmuI386PdbrGet)
        movl    %cr3, %eax
        movl    $0xfffff000, %edx

#if     (CPU==I80386)
        jmp     mmuPdbrGet0
#endif

        movl    $0xffffffe0, %edx

mmuPdbrGet0:
        andl    %edx, %eax
        ret

/******************************************************************************
 * mmuI386TlbFlush - Flush MMU translation table
 *
 * RETURNS N/A
 */

        .balign 16, 0x90

FUNC_LABEL(mmuI386TlbFlush)
        pushfl                          /* Store flags */
        cli                             /* Disable interrputs */
        movl    %cr3, %eax
        movl    %eax, %cr3               /* Flush */
        jmp     mmuTlbFlushEnd

mmuTlbFlushEnd:
        popfl                           /* Enable interrupts */
        ret

