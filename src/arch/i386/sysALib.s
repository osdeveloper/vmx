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

/* sysALib.s - System routines  */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>

        /* Internals */
        .globl  GTEXT(sysInByte)
        .globl  GTEXT(sysInWord)
        .globl  GTEXT(sysInLong)
        .globl  GTEXT(sysOutByte)
        .globl  GTEXT(sysOutWord)
        .globl  GTEXT(sysOutLong)
        .globl  GTEXT(sysInWordString)
        .globl  GTEXT(sysInLongString)
        .globl  GTEXT(sysOutLongString)
        .globl  GTEXT(sysOutWordString)
        .globl  GTEXT(sysWait)
        .globl  GTEXT(sysReboot)

        .text
        .balign 16

/******************************************************************************
 * sysInByte - Read one byte from I/O space
 *
 * RETURNS: Byte data from I/O port
 */

        .balign 16,0x90

FUNC_LABEL(sysInByte)
        movl    SP_ARG1(%esp),%edx
        movl    $0,%eax
        inb     %dx,%al
        jmp     sysInByte0

sysInByte0:
        ret

/******************************************************************************
 * sysInWord - Read one word from I/O space
 *
 * RETURNS: Word data from I/O port
 */

        .balign 16,0x90

FUNC_LABEL(sysInWord)
        movl    SP_ARG1(%esp),%edx
        movl    $0,%eax
        inw     %dx,%ax
        jmp     sysInWord0
sysInWord0:
        ret

/******************************************************************************
 * sysInLong - Read one long-word from I/O space
 *
 * RETURNS: Long-word data from I/O port
 */

        .balign 16,0x90

FUNC_LABEL(sysInLong)
        movl    SP_ARG1(%esp),%edx
        movl    $0,%eax
        inl     %dx,%eax
        jmp     sysInLong0

sysInLong0:
        ret

/******************************************************************************
 * sysOutByte - Output one byte to I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysOutByte)
        movl    SP_ARG1(%esp),%edx
        movl    SP_ARG2(%esp),%eax
        outb    %al,%dx
        jmp     sysOutByte0

sysOutByte0:
        ret

/******************************************************************************
 * sysOutWord - Output one word to I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysOutWord)
        movl    SP_ARG1(%esp),%edx
        movl    SP_ARG2(%esp),%eax
        outw    %ax,%dx
        jmp     sysOutWord0

sysOutWord0:
        ret

/******************************************************************************
 * sysOutLong - Output one long-word to I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysOutLong)
        movl    SP_ARG1(%esp),%edx
        movl    SP_ARG2(%esp),%eax
        outl    %eax,%dx
        jmp     sysOutLong0

sysOutLong0:
        ret

/******************************************************************************
 * sysInWordString - Input word string from I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysInWordString)
        pushl   %edi
        movl    SP_ARG1+4(%esp),%edx
        movl    SP_ARG2+4(%esp),%edi
        movl    SP_ARG3+4(%esp),%ecx
        cld
        rep
        insw    %dx,(%edi)
        movl    %edi,%eax
        popl    %edi
        ret

/*****************************************************************************
 * sysInLongString - Input long string from I/O space
 *
 * RETURNS:   N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysInLongString)
        pushl   %edi
        movl    SP_ARG1+4(%esp),%edx
        movl    SP_ARG2+4(%esp),%edi
        movl    SP_ARG3+4(%esp),%ecx
        cld
        rep
        insl    %dx,(%edi)
        movl    %edi,%eax
        popl    %edi
        ret

/******************************************************************************
 * sysOutWordString - Output word string to I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysOutWordString)
        pushl   %esi
        movl    SP_ARG1+4(%esp),%edx
        movl    SP_ARG2+4(%esp),%esi
        movl    SP_ARG3+4(%esp),%ecx
        cld
        rep
        outsw   (%esi),%dx
        movl    %esi,%eax
        popl    %esi
        ret

/******************************************************************************
 * sysOutLongString - Output word string to I/O space
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysOutLongString)
        pushl   %esi
        movl    SP_ARG1+4(%esp),%edx
        movl    SP_ARG2+4(%esp),%esi
        movl    SP_ARG3+4(%esp),%ecx
        cld
        rep
        outsl   (%esi),%dx
        movl    %esi,%eax
        popl    %esi
        ret

/******************************************************************************
 * sysWait - Wait for input buffer to become empty
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysWait)
        xorl    %ecx,%ecx

sysWait0:
        movl    $0x64,%edx              /* Check if its ready to write */
        inb     %dx,%al
        andb    $2,%al
        loopnz  sysWait0
        ret

/******************************************************************************
 * sysReboot - Reset system
 *
 * RETURNS:   N/A
 */

        .balign 16,0x90

FUNC_LABEL(sysReboot)
        movl    $0,%eax
        lgdt    (%eax)                  /* Messup GDT */
        ret

