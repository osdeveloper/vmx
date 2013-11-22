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

/* vmLib.h - Virtual memory library */

#ifndef _vmLib_h
#define _vmLib_h

#include <ostool/moduleNumber.h>

#include <vmx.h>

#define S_vmLib_NOT_INSTALLED          (M_vmLib | 0x0001)
#define S_vmLib_ZERO_PAGE_SIZE         (M_vmLib | 0x0002)
#define S_vmLib_NOT_PAGE_ALIGNED       (M_vmLib | 0x0003)
#define S_vmLib_INVALID_STATE          (M_vmLib | 0x0004)
#define S_vmLib_INVALID_STATE_MASK     (M_vmLib | 0x0005)
#define S_vmLib_ADDR_IN_GLOBAL_SPACE   (M_vmLib | 0x0006)

/* State masks */
#define VM_STATE_MASK_VALID             0x03
#define VM_STATE_MASK_WRITABLE          0x0c
#define VM_STATE_MASK_CACHEABLE         0x30
#define VM_STATE_MASK_MEM_COHERENCY     0x40
#define VM_STATE_MASK_GUARDED           0x80

/* States */
#define VM_STATE_VALID                  0x01
#define VM_STATE_NOT_VALID              0x00
#define VM_STATE_WRITABLE               0x04
#define VM_STATE_NOT_WRITABLE           0x00
#define VM_STATE_CACHEABLE              0x10
#define VM_STATE_NOT_CACHEABLE          0x00
#define VM_STATE_MEM_COHERENCY          0x40
#define VM_STATE_NOT_MEM_COHERENCY      0x00
#define VM_STATE_GUARDED                0x80
#define VM_STATE_NOT_GUARDED            0x00

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/mmuArchLib.h>
#include <vmx/semLib.h>
#include <os/objLib.h>
#include <os/classLib.h>

typedef struct
{
    void     *vAddr;
    void     *pAddr;
    unsigned  len;
    unsigned  initialMask;
    unsigned  initialState;
} PHYS_MEM_DESC;

typedef struct
{
    OBJ_CORE           objCore;
    MMU_TRANS_TABLE_ID mmuTransTable;
    SEMAPHORE          sem;
} VM_CONTEXT;

typedef struct
{
    BOOL    vmLibInstalled;
    BOOL    protectTextSegs;
} VM_LIB_INFO;

/* Types */

typedef VM_CONTEXT *VM_CONTEXT_ID;

/* Imports */

IMPORT CLASS_ID vmContextClassId;
IMPORT VM_LIB_INFO vmLibInfo;

/* Functions */

/******************************************************************************
 * vmLibInit - Initialize virtual memory library
 *
 * RETURNS: OK or ERROR
 */

STATUS vmLibInit(
    int pageSize
    );

/******************************************************************************
 * vmGlobalMapInit - Initialize global page map
 *
 * RETURNS: VM_CONTEXT_ID for global vmContext or NULL
 */

VM_CONTEXT_ID vmGlobalMapInit(
    PHYS_MEM_DESC *pMemDescTable,
    int            numElements,
    BOOL           enable
    );

/******************************************************************************
 * vmContextCreate - Create virtual memory context
 *
 * RETURNS: VM_CONTEXT_ID virtual memory context or NULL
 */

VM_CONTEXT_ID vmContextCreate(
    void
    );

/******************************************************************************
 * vmContextInit - Initialize virtual memory context
 *
 * RETURNS: OK or ERROR
 */

STATUS vmContextInit(
    VM_CONTEXT_ID context
    );

/******************************************************************************
 * vmContextDestroy - Destroy virtual memory context
 *
 * RETURNS: OK or ERROR
 */

STATUS vmContextDestroy(
    VM_CONTEXT_ID context
    );

/******************************************************************************
 * vmStateSet - Setup pages states
 *
 * RETURNS: OK or ERROR
 */

STATUS vmStateSet(
    VM_CONTEXT_ID  context,
    void          *vAddr,
    int            len,
    unsigned       mask,
    unsigned       state
    );

/******************************************************************************
 * vmStateGet - Get pages states
 *
 * RETURNS: OK or ERROR
 */

STATUS vmStateGet(
    VM_CONTEXT_ID  context,
    void          *vAddr,
    unsigned      *pState
    );

/******************************************************************************
 * vmMap - Map physical page(s)
 *
 * RETURNS: OK or ERROR
 */

STATUS vmMap(
    VM_CONTEXT_ID  context,
    void          *vAddr,
    void          *pAddr,
    unsigned       len
    );

/******************************************************************************
 * vmGlobalMap - Map physical page(s) to global page map
 *
 * RETURNS: OK or ERROR
 */

STATUS vmGlobalMap(
    void     *vAddr,
    void     *pAddr,
    unsigned  len
    );

/******************************************************************************
 * vmGlobalInfoGet - Get global page block table
 *
 * RETURNS: Pointer to page block table
 */

char* vmGlobalInfoGet(
    void
    );

/******************************************************************************
 * vmPageBlockSizeGet - Get page block size
 *
 * RETURNS: Global page block size
 */

int vmPageBlockSizeGet(
    void
    );

/******************************************************************************
 * vmCurrentSet - Set current page map
 *
 * RETURNS: OK or ERROR
 */

STATUS vmCurrentSet(
    VM_CONTEXT_ID context
    );

/******************************************************************************
 * vmCurrentGet - Get current page map
 *
 * RETURNS: Pointer to current page map
 */

VM_CONTEXT_ID vmCurrentGet(
    void
    );

/******************************************************************************
 * vmEnable - Enable MMU
 *
 * RETURNS: OK or ERROR
 */

STATUS vmEnable(
    BOOL enable
    );

/******************************************************************************
 * vmPageSizeGet - Get virual memory page size
 *
 * RETURNS: Page size
 */

int vmPageSizeGet(
    void
    );

/******************************************************************************
 * vmTranslate - Translate from virtual to physical memory address
 *
 * RETURNS: OK or ERROR
 */

STATUS vmTranslate(
    VM_CONTEXT_ID   context,
    void           *vAddr,
    void          **pAddr
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vmLib_h */

