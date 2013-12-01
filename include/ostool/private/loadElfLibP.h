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

/* loadElfLibP.h - ELF loader library private header */

#ifndef _loadElfLibP_h
#define _loadElfLibP_h

#include <elf.h>
#include <vmx.h>
#include <sys/types.h>
#include <vmx.h>
#include <os/memPartLib.h>
#include <os/symbol.h>
#include <ostool/private/loadLibP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef void **SYM_ADDR_TABLE;
typedef void  *SECT_ADDR;
typedef void **SECT_ADDR_TABLE;

typedef struct
{
    u_int32_t     *pLoadSectHeaderIndex;
    u_int32_t     *pSymTableSectHeaderIndex;
    u_int32_t     *pRelSectHeaderIndex;
    u_int32_t     *pStrTableSectHeaderIndex;
} INDEX_TABLES;

typedef struct
{
    void          *pAddrSdata;                  /* Address of sda data seg. */
    void          *pAddrSbss;                   /* Address of sda bss seg. */
    int            sizeSdata;                   /* Size of sda data seg. */
    int            sizeSbss;                    /* Size of sda bss seg. */
    int            flagsSdata;                  /* Flags of sda data seg. */
    int            flagsSbss;                   /* Flags of sda bss seg. */
    PART_ID        sdaMemPartId;                /* Memory partition for sda */
    void          *sdaBaseAddr;                 /* Base address for sda */
    int            sdaAreaSize;                 /* Size of sda area */
    void          *pAddrSdata2;                 /* Address of sda2 data seg. */
    void          *pAddrSbss2;                  /* Address of sda2 bss seg. */
    int            sizeSdata2;                  /* Size of sda2 data seg. */
    int            sizeSbss2;                   /* Size of sda2 bss seg. */
    int            flagsSdata2;                 /* Flags of sda2 data seg. */
    int            flagsSbss2;                  /* Flags of sda2 bss seg. */
    PART_ID        sda2MemPartId;               /* Memory partition for sda2 */
    void          *sda2BaseAddr;                /* Base address for sda2 */
    int            sda2AreaSize;                /* Size of sda2 area */
} SDA_INFO;

typedef struct
{
    SYM_ADDR      pAddr;                        /* Symbol address */
    SYM_TYPE      type;                         /* Symbol type */
} SYM_INFO;

typedef Elf32_Sym       **SYMTABLE_REFS;
typedef void           ***SYMADDR_REFS;
typedef SYM_INFO         *SYM_INFO_TABLE;
typedef SYM_INFO_TABLE   *SYMINFO_REFS;

/* Macros */

#define CHECK_2_ALIGN(x)                (((x) & ((x) - 1)) == 0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _loadElfLibP_h */

