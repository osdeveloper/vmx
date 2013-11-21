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

/* archI386.h - Architecture specific stuff */

#ifndef _archI386_h_
#define _archI386_h_

#define _BYTE_ORDER             _LITTLE_ENDIAN
#define _ALLOC_ALIGN_SIZE       4
#define _STACK_ALIGN_SIZE       4
#define _STACK_DIR              _STACK_GROWS_DOWN
#define _CACHE_ALIGN_SIZE       32

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>

typedef struct
{
    u_int32_t reserved0;
    u_int32_t reserved1;
    u_int32_t reserved2;
    u_int32_t reserved3;
    u_int32_t reserved4;
    u_int32_t reserved5;
    u_int32_t reserved6;
    u_int32_t reserved7;
} __attribute__((packed)) X86_EXT;
        
/* Global variables to hold segment pointers */
IMPORT u_int32_t sysCsSuper;
IMPORT u_int32_t sysCsExc;
IMPORT u_int32_t sysCsInt;

/* Functions */

/******************************************************************************
 * sysGDTSet - Routine to be called by C program to setup GDT Segment table
 *
 * RETURNS:   N/A
 */

void sysGDTSet(
    GDT *baseAddr
    );

/******************************************************************************
 * segBaseSet - Setup special GDT pointer and Segments
 *
 * RETURNS: OK
 */

STATUS segBaseSet(
    GDT *baseAddr
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif

