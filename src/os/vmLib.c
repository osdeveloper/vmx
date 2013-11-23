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

/* vmLib.c - Virtual memory library */

#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/mmuArchLib.h>
#include <arch/cacheArchLib.h>
#include <vmx/semLib.h>
#include <os/memPartLib.h>
#include <os/classLib.h>
#include <os/objLib.h>
#include <os/cacheLib.h>
#include <os/vmLib.h>

/* Mactos */
#define NUM_PAGE_STATES 256

#define NOT_PAGE_ALIGNED(addr) \
    (((unsigned int) (addr)) & ((unsigned int) vmPageSize - 1))

#define IS_IN_GLOBAL_SPACE(vAddr) \
    (globalPageBlockTable[(unsigned) vAddr / mmuPageBlockSize])

/* Locals */
LOCAL unsigned    vmStateTransTable[NUM_PAGE_STATES];
LOCAL unsigned    vmMaskTransTable[NUM_PAGE_STATES];
LOCAL VM_CONTEXT  vmSysContext;
LOCAL OBJ_CLASS   vmContextClass;
LOCAL SEMAPHORE   globalMemMutex;
LOCAL int         vmPageSize     = ERROR;
LOCAL VM_CONTEXT *vmCurrContext  = NULL;
LOCAL BOOL        vmLibInstalled = FALSE;

/* Globals */
char     *globalPageBlockTable;
CLASS_ID  vmContextClassId = &vmContextClass;
int       vmMutexOptions   = SEM_Q_PRIORITY | SEM_DELETE_SAFE;

/******************************************************************************
 * vmLibInit - Initialize virtual memory library
 *
 * RETURNS: OK or ERROR
 */

STATUS vmLibInit(
    int pageSize
    )
{
    STATUS              status;
    int                 i;
    int                 j;
    int                 tableSize;
    unsigned            state;
    unsigned            mask;
    VM2MMU_STATE_TRANS *thisElement;
    unsigned            vmState;
    unsigned            mmuState;
    unsigned            vmMask;
    unsigned            mmuMask;

    /* Check if already installed */
    if (vmLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Check for zero pagesize */
        if (pageSize == 0)
        {
            status = ERROR;
        }
        else
        {
            /* Store page size */
            vmPageSize = pageSize;

            for (i = 0; i < NUM_PAGE_STATES; i++)
            {
                state = 0;
                for (j = 0; j < mmuStateTransTableSize; j++)
                {
                    thisElement = &mmuStateTransTable[j];
                    vmState     = thisElement->vmState;
                    mmuState    = thisElement->mmuState;
                    vmMask      = thisElement->vmMask;

                    if ((i & vmMask) == vmState)
                    {
                        state |= mmuState;
                    }
                }

                vmStateTransTable[i] = state;
            }

            for (i = 0; i < NUM_PAGE_STATES; i++)
            {
                mask = 0;
                for (j = 0; j < mmuStateTransTableSize; j++)
                {
                    thisElement = &mmuStateTransTable[j];
                    vmMask      = thisElement->vmMask;
                    mmuMask     = thisElement->mmuMask;

                    if ((i & vmMask) == vmMask)
                    {
                        mask |= mmuMask;
                    }
                }

                vmMaskTransTable[i] = mask;
            }

            /* Global page block table size */
            tableSize = (unsigned) 0x80000000 / (mmuPageBlockSize / 2);
            globalPageBlockTable = (char *) calloc(
                                                tableSize,
                                                sizeof(globalPageBlockTable[0])
                                                );
            if (globalPageBlockTable == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Initialize class */
                if (classInit(
                        vmContextClassId,
                        sizeof(VM_CONTEXT),
                        OFFSET(VM_CONTEXT, objCore),
                        memSysPartId,
                        (FUNCPTR) vmContextCreate,
                        (FUNCPTR) vmContextInit,
                        (FUNCPTR) vmContextDestroy) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    semMInit(&globalMemMutex, vmMutexOptions);

                    /* Enable mmu for cache library */
                    cacheMmuAvailable = TRUE;
                    cacheFuncsSet();

                    /* Set as installed */
                    vmLibInstalled = TRUE;
                    status = OK;
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * vmGlobalMapInit - Initialize global page map
 *
 * RETURNS: VM_CONTEXT_ID for global vmContext or NULL
 */

VM_CONTEXT_ID vmGlobalMapInit(
    PHYS_MEM_DESC *pMemDescTable,
    int            numElements,
    BOOL           enable
    )
{
    VM_CONTEXT_ID  ret;
    PHYS_MEM_DESC *thisDesc;
    int            i;
    BOOL           error;

    if (vmLibInstalled != TRUE)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        ret = NULL;
    }
    else
    {
        error = FALSE;

        /* Setup default map, copied by all new maps */
        for (i = 0; i < numElements; i++)
        {
            thisDesc = &pMemDescTable[i];

            if (vmGlobalMap(
                    thisDesc->vAddr,
                    thisDesc->pAddr,
                    thisDesc->len
                    ) != OK)
            {
                ret   = NULL;
                error = TRUE;
                break;
            }
        }

        if (error == FALSE)
        {
            /* Intialize system virtual context */
            if (vmContextInit(&vmSysContext) != OK)
            {
                ret   = NULL;
                error = TRUE;
            }
            else
            {
                /* Setup page states for elements */
                for (i = 0; i < numElements; i++)
                {
                    thisDesc = &pMemDescTable[i];

                    if (vmStateSet(
                            &vmSysContext,
                            thisDesc->vAddr,
                            thisDesc->len,
                            thisDesc->initialMask,
                            thisDesc->initialState
                            ) != OK)
                    {
                        ret   = NULL;
                        error = TRUE;
                        break;
                    }
                }

                if (error == FALSE)
                {
                    /* Set current context */
                    vmCurrContext = &vmSysContext;
                    MMU_CURRENT_SET(vmSysContext.mmuTransTable);

                    if (enable == TRUE)
                    {
                        if (vmEnable(TRUE) != OK)
                        {
                            ret   = NULL;
                            error = TRUE;
                        }
                    }
                }

                if (error == FALSE)
                {
                    ret = &vmSysContext;
                }
            }
        }
    }

    return ret;
}

/******************************************************************************
 * vmContextCreate - Create virtual memory context
 *
 * RETURNS: VM_CONTEXT_ID virtual memory context or NULL
 */

VM_CONTEXT_ID vmContextCreate(
    void
    )
{
    VM_CONTEXT_ID context;

    if (vmLibInstalled != TRUE)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        context = NULL;
    }
    else
    {
        /* Allocate object */
        context = (VM_CONTEXT *) objAlloc(vmContextClassId);
        if (context != NULL)
        {
            /* Initialize structure */
            if (vmContextInit(context) != OK)
            {
                objFree(vmContextClassId, context);
                context = NULL;
            }
        }
    }

    return context;
}

/******************************************************************************
 * vmContextInit - Initialize virtual memory context
 *
 * RETURNS: OK or ERROR
 */

STATUS vmContextInit(
    VM_CONTEXT_ID context
    )
{
    STATUS status;

    if (vmLibInstalled != TRUE)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        context->mmuTransTable = MMU_TRANS_TABLE_CREATE();
        if (context->mmuTransTable == NULL)
        {
            status = ERROR;
        }
        else
        {
            semMInit(&context->sem, vmMutexOptions);
            objCoreInit(&context->objCore, vmContextClassId);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * vmContextDestroy - Destroy virtual memory context
 *
 * RETURNS: OK or ERROR
 */

STATUS vmContextDestroy(
    VM_CONTEXT_ID context
    )
{
    STATUS status;

    /* Verify object class */
    if (OBJ_VERIFY(context, vmContextClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        semTake(&context->sem, WAIT_FOREVER);

        if (MMU_TRANS_TABLE_DESTROY(context->mmuTransTable) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Free datastructure */
            objFree(vmContextClassId, context);
            status = OK;
        }
    }

    return status;
}

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
    )
{
    STATUS    status;
    unsigned  mmuState;
    unsigned  mmuMask;
    char     *thisPage     = vAddr;
    int       pageSize     = vmPageSize;
    unsigned  numBytesDone = 0;

    if (vmLibInstalled != TRUE)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        /* Check if context should be current */
        if (context == NULL)
        {
            context = vmCurrContext;
        }

        /* Verify object class */
        if (OBJ_VERIFY(context, vmContextClassId) != OK)
        {
            status = ERROR;
        }
        else
        {
            if (NOT_PAGE_ALIGNED(thisPage))
            {
                errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
                status = ERROR;
            }
            else if (NOT_PAGE_ALIGNED(len))
            {
                errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
                status = ERROR;
            }
            else if (state > NUM_PAGE_STATES)
            {
                errnoSet(S_vmLib_INVALID_STATE);
                status = ERROR;
            }
            else if (mask > NUM_PAGE_STATES)
            {
                errnoSet(S_vmLib_INVALID_STATE_MASK);
                status = ERROR;
            }
            else
            {
                status = OK;

                /* Get mmu state */
                mmuState = vmStateTransTable[state];
                mmuMask  = vmMaskTransTable[mask];

                /* Setup states */
                while (numBytesDone < len)
                {
                    if (MMU_STATE_SET(
                            context->mmuTransTable,
                            thisPage,
                            mmuMask,
                            mmuState
                            ) != OK)
                    {
                        status = ERROR;
                        break;
                    }

                    /* Move to next page */
                    thisPage     += pageSize;
                    numBytesDone += pageSize;
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * vmStateGet - Get pages states
 *
 * RETURNS: OK or ERROR
 */

STATUS vmStateGet(
    VM_CONTEXT_ID  context,
    void          *vAddr,
    unsigned      *pState
    )
{
    STATUS              status;
    unsigned            mmuState;
    VM2MMU_STATE_TRANS *thisState;
    int                 i;
    unsigned            mmuTMask;
    unsigned            mmuTState;
    unsigned            vmTState;

    /* Check if context should be current */
    if (context == NULL)
    {
        context = vmCurrContext;
    }

    /* Verify object class */
    if (OBJ_VERIFY(context, vmContextClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        *pState = 0;

        if (MMU_STATE_GET(
                context->mmuTransTable,
                vAddr,
                &mmuState
                ) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Translate state */
            for (i = 0; i < mmuStateTransTableSize; i++)
            {
                thisState = &mmuStateTransTable[i];
                mmuTMask  = thisState->mmuMask;
                mmuTState = thisState->mmuState;
                vmTState  = thisState->vmState;

                if ((mmuState & mmuTMask) == mmuTState)
                {
                    *pState |= vmTState;
                }
            }

            status = OK;
        }
    }

    return status;
}

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
    )
{
    STATUS  status;
    int     pageSize     = vmPageSize;
    char   *thisVirtPage = (char *) vAddr;
    char   *thisPhysPage = (char *) pAddr;
    int     numBytesDone = 0;

    /* Check if context should be current */
    if (context == NULL)
    {
        context = vmCurrContext;
    }

    /* Verify object class */
    if (OBJ_VERIFY(context, vmContextClassId) != OK)
    {
        status = ERROR;
    }
    else if (NOT_PAGE_ALIGNED(thisVirtPage))
    {
        errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
        status = ERROR;
    }
    else if (NOT_PAGE_ALIGNED(len))
    {
        errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
        status = ERROR;
    }
    else
    {
         status = OK;
         semTake(&context->sem, WAIT_FOREVER);

         while (numBytesDone < len)
         {
             if (IS_IN_GLOBAL_SPACE(thisVirtPage))
             {
                 errnoSet(S_vmLib_ADDR_IN_GLOBAL_SPACE);
                 status = ERROR;
                 break;
             }

             /* Setup page */
             if (MMU_PAGE_MAP(
                     context->mmuTransTable,
                     thisVirtPage,
                     thisPhysPage
                     ) != OK)
             {
                 status = ERROR;
                 break;
             }

             /* Set page state */
             if (vmStateSet(
                     context,
                     thisVirtPage,
                     pageSize,
                     VM_STATE_MASK_VALID    |
                     VM_STATE_MASK_WRITABLE |
                     VM_STATE_MASK_CACHEABLE,
                     VM_STATE_VALID         |
                     VM_STATE_WRITABLE      |
                     VM_STATE_CACHEABLE
                     ) != OK)
            {
                status = ERROR;
                break;
            }

            /* Advance */
            thisVirtPage += pageSize;
            thisPhysPage += pageSize;
            numBytesDone += pageSize;
        }

        semGive(&context->sem);
    }

    return status;
}

/******************************************************************************
 * vmGlobalMap - Map physical page(s) to global page map
 *
 * RETURNS: OK or ERROR
 */

STATUS vmGlobalMap(
    void     *vAddr,
    void     *pAddr,
    unsigned  len
    )
{
    STATUS status;
    int   pageSize     = vmPageSize;
    char *thisVirtPage = (char *) vAddr;
    char *thisPhysPage = (char *) pAddr;
    int   numBytesDone = 0;

    if (vmLibInstalled != TRUE)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {

        if (NOT_PAGE_ALIGNED(thisVirtPage))
        {
            errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
            status = ERROR;
        }
        else if (NOT_PAGE_ALIGNED(len))
        {
            errnoSet(S_vmLib_NOT_PAGE_ALIGNED);
            status = ERROR;
        }
        else
        {
            status = OK;

            semTake(&globalMemMutex, WAIT_FOREVER);

            while (numBytesDone < len)
            {
                /* Setup page */
                if (MMU_GLOBAL_PAGE_MAP(thisVirtPage, thisPhysPage) != OK)
                {
                    status = ERROR;
                    break;
                }

                /* Mark page as used */
                globalPageBlockTable[
                    (unsigned) thisVirtPage / mmuPageBlockSize
                    ] = TRUE;

                /* Advance */
                thisVirtPage += pageSize;
                thisPhysPage += pageSize;
                numBytesDone += pageSize;
            }

            semGive(&globalMemMutex);
        }
    }

    return status;
}

/******************************************************************************
 * vmGlobalInfoGet - Get global page block table
 *
 * RETURNS: Pointer to page block table
 */

char* vmGlobalInfoGet(
    void
    )
{
    return globalPageBlockTable;
}

/******************************************************************************
 * vmPageBlockSizeGet - Get page block size
 *
 * RETURNS: Global page block size
 */

int vmPageBlockSizeGet(
    void
    )
{
    return mmuPageBlockSize;
}

/******************************************************************************
 * vmCurrentSet - Set current page map
 *
 * RETURNS: OK or ERROR
 */

STATUS vmCurrentSet(
    VM_CONTEXT_ID context
    )
{
    STATUS status;

    /* Verify object class */
    if (OBJ_VERIFY(context, vmContextClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Set as current */
        vmCurrContext = context;
        MMU_CURRENT_SET(context->mmuTransTable);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * vmCurrentGet - Get current page map
 *
 * RETURNS: Pointer to current page map
 */

VM_CONTEXT_ID vmCurrentGet(
    void
    )
{
    return vmCurrContext;
}

/******************************************************************************
 * vmEnable - Enable MMU
 *
 * RETURNS: OK or ERROR
 */

STATUS vmEnable(
    BOOL enable
    )
{
    return MMU_ENABLE(enable);
}

/******************************************************************************
 * vmPageSizeGet - Get virual memory page size
 *
 * RETURNS: Page size
 */

int vmPageSizeGet(
    void
    )
{
    return vmPageSize;
}

/******************************************************************************
 * vmTranslate - Translate from virtual to physical memory address
 *
 * RETURNS: OK or ERROR
 */

STATUS vmTranslate(
    VM_CONTEXT_ID   context,
    void           *vAddr,
    void          **pAddr
    )
{
    STATUS status;

    /* Check if context should be current */
    if (context == NULL)
    {
        context = vmCurrContext;
    }
    else
    {
        /* Verify object class */
        if (OBJ_VERIFY(context, vmContextClassId) != OK)
        {
            status = ERROR;
        }
        else
        {
            status = MMU_TRANSLATE(context->mmuTransTable, vAddr, pAddr);
        }
    }

    return status;
}

