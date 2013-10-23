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

/* regsI386.h - Registers */

#ifndef _regsI386_h
#define _regsI386_h

#define IDT_TRAP_GATE          0x0000ef00

#define REGS_EDI                     0x00
#define REGS_ESI                     0x04
#define REGS_EBP                     0x08
#define REGS_ESP                     0x0c
#define REGS_EBX                     0x10
#define REGS_EDX                     0x14
#define REGS_ECX                     0x18
#define REGS_EAX                     0x1c
#define REGS_EFLAGS                  0x20
#define REGS_PC                      0x24

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vmx.h>

/* Segement Descriptor */
typedef struct
{
    u_int16_t     limitLW;
    u_int16_t     baseLW;
    u_int8_t      baseMB;
    u_int8_t      type;
    u_int8_t      limitUB;
    u_int8_t      baseUB;
} __attribute__((packed)) GDT;

/* IDT table entry struct */
typedef struct
{
    u_int16_t     offsetLo;
    u_int16_t     selector;
    u_int8_t      params;
    u_int8_t      type;
    u_int16_t     offsetHi;
} __attribute__((packed)) CALL_GATE;

typedef u_int8_t INSTR;

/* Processor register storage */
typedef struct
{
    u_int32_t     edi;
    u_int32_t     esi;
    u_int32_t     ebp;
    u_int32_t     esp;
    u_int32_t     ebx;
    u_int32_t     edx;
    u_int32_t     ecx;
    u_int32_t     eax;
    u_int32_t     eflags;
    INSTR        *pc;
} __attribute__((packed)) REG_SET;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _regsI386_h */

