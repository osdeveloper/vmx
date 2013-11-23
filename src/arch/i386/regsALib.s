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

/* regsALib.s - System dependent registers */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>

        /* Internals */
        .globl  GTEXT(regsGdtSet)
        .globl  GTEXT(regsCr4Get)
        .globl  GTEXT(regsCr4Set)

        .text
        .balign 16

/******************************************************************************
 * regsGdtSet - Set global descriptor table
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(regsGdtSet)
        movl    SP_ARG1(%esp), %eax
        lgdt    (%eax)
        movw    $0x0010, %ax    /* Offset in GDT to data segment */
        movw    %ax, %dx
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        movw    %ax, %ss
        jmp     $0x08, $flushCs
flushCs:
        ret

/******************************************************************************
 * regsCr4Get - Get contents of CR4 register
 *
 * RETURNS: Contents in CR4 register
 */

        .balign 16,0x90

FUNC_LABEL(regsCr4Get)
#if     (CPU == I80386 || CPU == I80486)
        jmp     cr4GetExit
#endif

        movl    %cr4, %eax

cr4GetExit:
        ret

/******************************************************************************
 * regsCr4Set - Set contents of CR4 register
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(regsCr4Set)
#if     (CPU == I80386 || CPU == I80486)
        jmp     cr4SetExit
#endif

        movl    SP_ARG1(%esp), %eax
        movl    %eax, %cr4

cr4SetExit:
        ret

