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

/* excI386Lib.h - Exception handeling */

#ifndef _excI386Lib_h
#define _excI386Lib_h

#include <arch/regs.h>

/* Define bottom and top of interrupt vector */

#define LOW_VEC                 0x00
#define HIGH_VEC                0xff

/* Exception info validation bits */
#define EXC_VEC_NUM             0x01            /* Vector number valid */
#define EXC_ERROR_CODE          0x02            /* Error code valid */
#define EXC_ESP_SS              0x04            /* ESP and SS valid */
#define EXC_CR2                 0x08            /* CR2 valid */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/esf.h>

typedef struct
{
    u_int16_t  valid;
    u_int16_t  vecNum;
    u_int32_t  errCode;
    INSTR     *pc;
    u_int32_t  eflags;
    u_int16_t  cs;
    u_int16_t  pad;
    u_int32_t  esp;
    u_int32_t  ss;
    u_int32_t  esp0;
    u_int32_t  cr2;
    u_int32_t  cr3;
    u_int32_t  esp00;
    u_int32_t  esp01;
    u_int32_t  esp02;
    u_int32_t  esp03;
    u_int32_t  esp04;
    u_int32_t  esp05;
    u_int32_t  esp06;
    u_int32_t  esp07;
    u_int32_t  reserved0;
    u_int32_t  reserved1;
    u_int32_t  reserved2;
    u_int32_t  reserved3;
} __attribute__((packed)) EXC_INFO;

/* Exception call tables */
IMPORT u_int8_t         excCallTbl[];
IMPORT VOIDFUNCPTR      intCallTbl[];

/* Functions */

/******************************************************************************
 * excExcHandle - Default exception handler
 *
 * RETURNS: N/A
 */

void excExcHandle(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    BOOL error
    );

/******************************************************************************
 * excIntHandle - Uninitialized interrput handler
 *
 * RETURNS: N/A
 */

void excIntHandle(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    BOOL error
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* _excI386Lib_h */

