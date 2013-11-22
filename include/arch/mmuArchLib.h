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

/* mmuLib.h - Memory mapping unit */

#ifndef _mmuArchLib_h
#define _mmuArchLib_h

#include <ostool/moduleNumber.h>

#include <types/vmxCpu.h>

#if      CPU==PENTIUM2
#include <arch/i386/mmuPro32Lib.h>
#endif

#define S_mmuLib_NOT_INSTALLED           (M_mmuLib | 0x0001)
#define S_mmuLib_UNSUPPORTED_PAGE_SIZE   (M_mmuLib | 0x0002)
#define S_mmuLib_UNABLE_TO_GET_PTE       (M_mmuLib | 0x0003)
#define S_mmuLib_PAGE_NOT_PRESENT        (M_mmuLib | 0x0004)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

/* Types */
typedef struct
{
    unsigned vmMask;
    unsigned mmuMask;
    unsigned vmState;
    unsigned mmuState;
} VM2MMU_STATE_TRANS;

typedef struct
{
    FUNCPTR            mmuLibInit;
    MMU_TRANS_TABLE_ID (*mmuTransTableCreate)();
    FUNCPTR            mmuTransTableDestroy;
    FUNCPTR            mmuEnable;
    FUNCPTR            mmuStateSet;
    FUNCPTR            mmuStateGet;
    FUNCPTR            mmuPageMap;
    FUNCPTR            mmuGlobalPageMap;
    FUNCPTR            mmuTranslate;
    VOIDFUNCPTR        mmuCurrentSet;
} MMU_LIB_FUNCTIONS;

/* Imports */

IMPORT VM2MMU_STATE_TRANS *mmuStateTransTable;
IMPORT int                 mmuStateTransTableSize;
IMPORT int                 mmuPageBlockSize;
IMPORT MMU_LIB_FUNCTIONS   mmuLibFunctions;

/* Macros */

/******************************************************************************
 * MMU_LIB_INIT - Initialize MMU library
 *
 * RETURNS: OK or ERROR
 */

#define MMU_LIB_INIT(pgsize) \
    (*(mmuLibFunctions.mmuLibInit))((pgsize))

/******************************************************************************
 * MMU_TRANS_TABLE_CREATE - Create MMU table
 *
 * RETURNS: MMU_TRANS_TABLE pointer or NULL
 */

#define MMU_TRANS_TABLE_CREATE \
    (*(mmuLibFunctions.mmuTransTableCreate))

/******************************************************************************
 * MMU_TRANS_TABLE_DESTROY - Destroy MMU table
 *
 * RETURNS: OK or ERROR
 */

#define MMU_TRANS_TABLE_DESTROY(table) \
    (*(mmuLibFunctions.mmuTransTableDestroy))((table))

/******************************************************************************
 * MMU_ENABLE
 *
 * RETURNS: TRUE or FALSE
 */

#define MMU_ENABLE(enable) \
    (*(mmuLibFunctions.mmuEnable))((enable))

/******************************************************************************
 * MMU_STATE_SET - Set MMU state
 *
 * RETURNS: OK or ERROR
 */

#define MMU_STATE_SET(table, addr, mask, state) \
    (*(mmuLibFunctions.mmuStateSet))((table), (addr), (mask), (state))

/******************************************************************************
 * MMU_STATE_GET - Get MMU state
 *
 * RETURNS: OK or ERROR
 */

#define MMU_STATE_GET(table, addr, state)\
    (*(mmuLibFunctions.mmuStateGet))((table), (addr), (state))

/******************************************************************************
 * MMU_PAGE_MAP - Map page
 *
 * RETURNS: OK or ERROR
 */

#define MMU_PAGE_MAP(table, vaddr, paddr) \
    (*(mmuLibFunctions.mmuPageMap))((table), (vaddr), (paddr))

/******************************************************************************
 * MMU_GLOBAL_PAGE_MAP - Map global page
 *
 * RETURNS: OK or ERROR
 */

#define MMU_GLOBAL_PAGE_MAP(vaddr, paddr) \
    (*(mmuLibFunctions.mmuGlobalPageMap))((vaddr), (paddr))

/******************************************************************************
 * MMU_TRANSLATE - Translate virtual to physical address
 *
 * RETURNS: OK or ERROR
 */

#define MMU_TRANSLATE(table, vaddr, paddr) \
    (*(mmuLibFunctions.mmuTranslate))((table), (vaddr), (paddr))

/******************************************************************************
 * MMU_CURRENT_SET - Set current MMU table
 *
 * RETURNS: N/A
 */

#define MMU_CURRENT_SET(table) \
    (*(mmuLibFunctions.mmuCurrentSet))((table))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUANGE */

#endif /* _mmuArchLib_h */

