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

/* mmuPro32Lib.h - Memory mapping unit for Pentium Pro (II) */

#ifndef _mmuPro32Lib_h
#define _mmuPro32Lib_h

/* MMU definitions */
#define PAGE_SIZE_4KB           0x1000
#define PAGE_SIZE_4MB           0x400000

#define PD_SIZE                 0x1000
#define PT_SIZE                 0x1000
#define PAGE_BLOCK_SIZE         0x400000

#define DIR_BITS                0xffc00000
#define TABLE_BITS              0x003ff000
#define DIR_INDEX               22
#define TABLE_INDEX             12
#define PTE_TO_ADDR_4KB         0xfffff000
#define PTE_TO_ADDR_4MB         0xffc00000
#define ADDR_TO_PAGE            12
#define ADDR_TO_PAGEBASE        0xffc00000
#define OFFSET_BITS_4KB         0x00000fff
#define OFFSET_BITS_4MB         0x003fffff

/* State definitions */
#define MMU_STATE_MASK_VALID            0x001
#define MMU_STATE_MASK_WRITABLE         0x002
#define MMU_STATE_MASK_WBACK            0x008
#define MMU_STATE_MASK_CACHEABLE        0x010
#define MMU_STATE_MASK_GLOBAL           0x100

#define MMU_STATE_VALID                 0x001
#define MMU_STATE_NOT_VALID             0x000
#define MMU_STATE_WRITABLE              0x002
#define MMU_STATE_NOT_WRITABLE          0x000
#define MMU_STATE_WBACK                 0x000
#define MMU_STATE_NOT_WBACK             0x008
#define MMU_STATE_CACHEABLE             0x000
#define MMU_STATE_NOT_CACHEABLE         0x010
#define MMU_STATE_GLOBAL                0x100
#define MMU_STATE_NOT_GLOBAL            0x000
#define MMU_STATE_CACHEABLE_WT          0x008

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef struct pteField
{
    unsigned      present :1;
    unsigned      rw      :1;
    unsigned      us      :1;
    unsigned      pwt     :1;
    unsigned      pcd     :1;
    unsigned      access  :1;
    unsigned      dirty   :1;
    unsigned      pagesize:1;
    unsigned      global  :1;
    unsigned      avail   :3;
    unsigned      page    :20;
} PTE_ENTRY;

typedef union pte
{
    PTE_ENTRY     entry;
    unsigned int  bits;
} PTE;

typedef struct mmuTransTable
{
    PTE           *pDirTable;
} MMU_TRANS_TABLE;

typedef MMU_TRANS_TABLE *MMU_TRANS_TABLE_ID;

/* Imports */

IMPORT BOOL mmuPro32Enabled;

/* Functions */

/******************************************************************************
 * mmuPro32LibInit - Intialize mmu library
 *
 * RETURNS: OK or ERROR
 */

STATUS mmuPro32LibInit(
    int pageSize
    );

/******************************************************************************
 * mmuPro32Enable - Enable/Disable MMU
 *
 * RETURNS TRUE or FALSE
 */

BOOL mmuPro32Enable(
    BOOL enable
    );

/******************************************************************************
 * mmuPro32On - Turn on memory mapping unit
 * 
 * Assumes that interrupts are looked out
 *
 * RETURNS N/A
 */

void mmuPro32On(
    void
    );

/******************************************************************************
 * mmuPro32Off - Turn off memory mapping unit
 *
 * Assumes that interrupts are looked out
 *
 * RETURNS N/A
 */

void mmuPro32Off(
    void
    );

/******************************************************************************
 * mmuPro32PdbrSet - Setup page directory register
 *
 * RETURNS N/A
 */

void mmuPro32PdbrSet(
    void *transTable
    );

/******************************************************************************
 * mmuPro32PdbrGet - Get page directory register
 *
 * RETURNS Pointer to mmu translation table
 */

MMU_TRANS_TABLE* mmuPro32PdbrGet(
    void
    );

/******************************************************************************
 * mmuPro32TlbFlush - Flush MMU translation table
 *
 * RETURNS N/A
 */

void mmuPro32TlbFlush(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _mmuPro32Lib_h */

