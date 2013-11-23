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

#define EFLAGS_BRANDNEW        0x00000200
#define EFLAGS_TF              0x00000100          /* Trap flag */
#define EFLAGS_IF              0x00000200          /* Interrupt enable flag */
#define IDT_TRAP_GATE          0x0000ef00

#define GREG_NUM                        8

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

#define spReg                        esp

/* Control registers */
#define CR0                            1
#define CR1                            2
#define CR2                            3
#define CR3                            4

/* Test registers */
#define TR3                            5
#define TR4                            6
#define TR5                            7
#define TR6                            8
#define TR7                            9

/* CR0 bits */
#define CR0_PE                  0x00000001      /* Protection enable */
#define CR0_MP                  0x00000002      /* Math present */
#define CR0_EM                  0x00000004      /* Emulation */
#define CR0_TS                  0x00000008      /* Task switch */
#define CR0_NE                  0x00000020      /* Numeric error */
#define CR0_WP                  0x00010000      /* Write protected */
#define CR0_AM                  0x00040000      /* Alignment mask */
#define CR0_NW                  0x20000000      /* No write trough */
#define CR0_CD                  0x40000000      /* Cache disable */
#define CR0_PG                  0x80000000      /* Paging */
#define CR0_NW_NOT              0xdfffffff      /* Write trought */
#define CR0_CD_NOT              0xbfffffff      /* Cache enable */

/* CR4 bits */
#define CR4_VME                 0x00000001      /* Virtual 8086 mode */
#define CR4_PVI                 0x00000002      /* Prot. virtual interrupts */
#define CR4_TSD                 0x00000004      /* Timestamp disable */
#define CR4_DE                  0x00000008      /* Debugging extension */
#define CR4_PSE                 0x00000010      /* Page size extension */
#define CR4_PAE                 0x00000020      /* Physical addr. extension */
#define CR4_MCE                 0x00000040      /* Machine check enable */
#define CR4_PGE                 0x00000080      /* Global page enable */
#define CR4_PCE                 0x00000100      /* Perf. monitoring enable */
#define CR_OSFXSR               0x00000200      /* Fxsave/fxstor instructions */
#define CR4_OSXMMEXCEPT         0x00000400      /* Streaming SIMD exception */

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

/* Macros */

#define G_REG_BASE              0x00
#define G_REG_OFFSET(n)         (G_REG_BASE + (n) * sizeof(u_int32_t))
#define SR_OFFSET               G_REG_OFFSET(GREG_NUM)
#define PC_OFFSET               (SR_OFFSET + sizeof(u_int32_t))

/* Imports */

IMPORT u_int32_t sysCsSuper;
IMPORT u_int32_t sysCsExc;
IMPORT u_int32_t sysCsInt;

/******************************************************************************
 * regsGdtSet - Set global descriptor table
 *
 * RETURNS: N/A
 */

void regsGdtSet(
    GDT *baseAddr
    );

/******************************************************************************
 * regsGdtInit - Initialize global descriptor table
 *
 * RETURNS: OK
 */

STATUS regsGdtInit(
    GDT *baseAddr
    );

/******************************************************************************
 * regsCr4Get - Get contents of CR4 register
 *
 * RETURNS: Contents in CR4 register
 */

u_int32_t regsCr4Get(
    void
    );

/******************************************************************************
 * regsCr4Set - Set contents of CR4 register
 *
 * RETURNS: N/A
 */

void regsCr4Set(
    u_int32_t value
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _regsI386_h */

