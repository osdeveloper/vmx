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

/* mmuArchLib.c - Memory mapping unit for Pentium Pro (II) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <sys/types.h>
#include <os/memPartLib.h>
#include <os/memLib.h>
#include <os/vmLib.h>
#include <arch/regs.h>
#include <arch/intArchLib.h>
#include <arch/cacheArchLib.h>
#include <arch/mmuArchLib.h>

/* Globals */
BOOL                mmuPro32Enabled = FALSE;
VM2MMU_STATE_TRANS *mmuStateTransTable;
int                 mmuStateTransTableSize;
int                 mmuPageBlockSize;
MMU_LIB_FUNCTIONS   mmuLibFunctions;

/* Locals */
LOCAL BOOL             firstTime = TRUE;
LOCAL int              mmuPageSize;
LOCAL MMU_TRANS_TABLE  mmuGlobalTransTable;
LOCAL MMU_TRANS_TABLE *mmuCurrentTransTable = &mmuGlobalTransTable;
LOCAL BOOL            *globalPageBlock;

/* State transitions */
LOCAL VM2MMU_STATE_TRANS mmuStateTransTableLocal[] =
{
    /* Validity of a page */
    {
        VM_STATE_MASK_VALID, MMU_STATE_MASK_VALID,
        VM_STATE_VALID,      MMU_STATE_VALID
    },
    {
        VM_STATE_MASK_VALID, MMU_STATE_MASK_VALID,
        VM_STATE_NOT_VALID,  MMU_STATE_NOT_VALID
    },

    /* Read/Write */
    {
        VM_STATE_MASK_WRITABLE, MMU_STATE_MASK_WRITABLE,
        VM_STATE_WRITABLE,      MMU_STATE_WRITABLE
    },
    {
        VM_STATE_MASK_WRITABLE, MMU_STATE_MASK_WRITABLE,
        VM_STATE_NOT_WRITABLE,  MMU_STATE_NOT_WRITABLE
    },

    /* Caching */
    {
        VM_STATE_MASK_CACHEABLE, MMU_STATE_MASK_CACHEABLE,
        VM_STATE_CACHEABLE,      MMU_STATE_CACHEABLE
    },
    {
        VM_STATE_MASK_CACHEABLE, MMU_STATE_MASK_CACHEABLE,
        VM_STATE_NOT_CACHEABLE,  MMU_STATE_NOT_CACHEABLE
    },

    /* Writeback */
    {
        VM_STATE_MASK_WBACK, MMU_STATE_MASK_WBACK,
        VM_STATE_WBACK,      MMU_STATE_WBACK
    },
    {
        VM_STATE_MASK_WBACK, MMU_STATE_MASK_WBACK,
        VM_STATE_NOT_WBACK,  MMU_STATE_NOT_WBACK
    },

    /* Global */
    {
        VM_STATE_MASK_GLOBAL, MMU_STATE_MASK_GLOBAL,
        VM_STATE_GLOBAL,      MMU_STATE_GLOBAL
    },
    {
        VM_STATE_MASK_GLOBAL, MMU_STATE_MASK_GLOBAL,
        VM_STATE_NOT_GLOBAL,  MMU_STATE_NOT_GLOBAL
    }
};

/* Forward declare library functions */
LOCAL STATUS mmuPteGet(
    MMU_TRANS_TABLE  *transTable,
    void             *vAddr,
    PTE             **result
    );

LOCAL MMU_TRANS_TABLE* mmuTransTableCreate(
    void
    );

LOCAL STATUS mmuTransTableInit(
    MMU_TRANS_TABLE *transTable
    );

LOCAL STATUS mmuTransTableDestroy(
    MMU_TRANS_TABLE *transTable
    );

LOCAL STATUS mmuStateSet(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    u_int32_t        stateMask,
    u_int32_t        state
    );

LOCAL STATUS mmuStateGet(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    u_int32_t       *state
    );

LOCAL STATUS mmuPageCreate(
    MMU_TRANS_TABLE *thisTable,
    void            *vPageAddr
    );

LOCAL STATUS mmuPageMap(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    void            *pAddr
    );

LOCAL STATUS mmuGlobalPageMap(
    void *vAddr,
    void *pAddr
    );

LOCAL STATUS mmuTranslate(
    MMU_TRANS_TABLE  *transTable,
    void             *vAddr,
    void            **pAddr
    );

LOCAL void mmuCurrentSet(
    MMU_TRANS_TABLE *transTable
    );

LOCAL void mmuMemPagesWriteDisable(
    MMU_TRANS_TABLE *transTable
    );

LOCAL MMU_LIB_FUNCTIONS mmuLibFunctionsLocal =
{
    mmuPro32LibInit,
    mmuTransTableCreate,
    mmuTransTableDestroy,
    mmuPro32Enable,
    mmuStateSet,
    mmuStateGet,
    mmuPageMap,
    mmuGlobalPageMap,
    mmuTranslate,
    mmuCurrentSet
};

/* Macros */
#define MMU_ON() \
  do { int tempReg; __asm__ __volatile__ ("movl %%cr0,%0; orl $0x80010000,%0; \
  movl %0,%%cr0; jmp 0f; 0:" : "=r" (tempReg) : : "memory"); } while (0)

#define MMU_OFF() \
  do { int tempReg; __asm__ __volatile__ ("movl %%cr0,%0; andl $0x7ffeffff,%0; \
  movl %0,%%cr0; jmp 0f; 0:" : "=r" (tempReg) : : "memory"); } while (0)

#define MMU_TLB_FLUSH() \
  do { int tempReg; __asm__ __volatile__ ("pushfl; cli; movl %%cr3,%0; \
  movl %0,%%cr3; jmp 0f; 0: popfl; " : "=r" (tempReg) : : "memory"); \
  } while (0)

#define MMU_UNLOCK(wasEnabled, oldLevel) \
  do { if (((wasEnabled) = mmuPro32Enabled) == TRUE) \
      {INT_LOCK (oldLevel); MMU_OFF (); } \
  } while(0)

#define MMU_LOCK(wasEnabled, oldLevel) \
  do { if ((wasEnabled) == TRUE) \
      {MMU_ON (); INT_UNLOCK (oldLevel);} \
  } while (0)

/******************************************************************************
 * mmuPro32LibInit - Intialize mmu library
 *
 * RETURNS: OK or ERROR
 */

STATUS mmuPro32LibInit(
    int pageSize
    )
{
    STATUS  status;
    int     i;
    PTE    *pDirTable;

    /* Assume 4k or 4M page size */
    if ((pageSize != PAGE_SIZE_4KB) && (pageSize != PAGE_SIZE_4MB))
    {
        errnoSet(S_mmuLib_UNSUPPORTED_PAGE_SIZE);
        status = ERROR;
    }
    else
    {
        /* Intitialize vmLib globals */
        mmuStateTransTable     = &mmuStateTransTableLocal[0];
        mmuStateTransTableSize =
            sizeof(mmuStateTransTableLocal) / sizeof(VM2MMU_STATE_TRANS);
        mmuPageBlockSize = PAGE_BLOCK_SIZE;
        mmuLibFunctions  = mmuLibFunctionsLocal;

        /* Initialy disable mmu */
        mmuPro32Enabled = FALSE;

        /* Store page size */
        mmuPageSize = pageSize;

        /* Allocate memory for global page block array */
        globalPageBlock = memalign(pageSize, pageSize);
        if (globalPageBlock == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Clear page block */
            memset(globalPageBlock, 0, pageSize);

            /* Allocate one page to hold directory table */
            pDirTable = memalign(pageSize, pageSize);
            if (pDirTable == NULL)
            {
                free(globalPageBlock);
                status = ERROR;
            }
            else
            {
                /* Set as global translation table */
                mmuGlobalTransTable.pDirTable = pDirTable;

                /* Clear page table */
                for (i = 0; i < (PD_SIZE / sizeof(PTE)); i++)
                {
                    pDirTable[i].entry.present  = 0;
                    pDirTable[i].entry.rw       = 0;
                    pDirTable[i].entry.us       = 0;
                    pDirTable[i].entry.pwt      = 0;
                    pDirTable[i].entry.pcd      = 0;
                    pDirTable[i].entry.access   = 0;
                    pDirTable[i].entry.dirty    = 0;
                    pDirTable[i].entry.pagesize =
                        (pageSize == PAGE_SIZE_4KB) ? 0 : 1;
                    pDirTable[i].entry.global   = 0;
                    pDirTable[i].entry.avail    = 0;
                    pDirTable[i].entry.page     = -1;
                }

                /* Set CR4 register to PAE = 0, PSE = 1 */
                regsCr4Set((regsCr4Get() & ~CR4_PAE) | CR4_PSE);
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * mmuPteGet - Get page table entry
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuPteGet(
    MMU_TRANS_TABLE  *transTable,
    void             *vAddr,
    PTE             **result
    )
{
    STATUS  status;
    PTE    *pDte;
    PTE    *pPageTable;

    /* Get directory table entry */
    pDte = &transTable->pDirTable[((u_int32_t) vAddr & DIR_BITS) >> DIR_INDEX];

    /* Get page table according to page size */
    if (mmuPageSize == PAGE_SIZE_4KB)
    {
        pPageTable = (PTE *) (pDte->bits & PTE_TO_ADDR_4KB);

        if ((u_int32_t) pPageTable == (0xffffffff & PTE_TO_ADDR_4KB))
        {
            errnoSet(S_mmuLib_UNABLE_TO_GET_PTE);
            status = ERROR;
        }
        else
        {
            *result =
                &pPageTable[((u_int32_t) vAddr & TABLE_BITS) >> TABLE_INDEX];
            status = OK;
        }
    }
    else
    {
        if ((u_int32_t) pDte == (0xffffffff & PTE_TO_ADDR_4MB))
        {
            errnoSet(S_mmuLib_UNABLE_TO_GET_PTE);
            status = ERROR;
        }
        else
        {
            *result = pDte;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * mmuTransTableCreate - Create an translation table
 *
 * RETURNS: MMU_TRANS_TABLE *pTable
 */

LOCAL MMU_TRANS_TABLE* mmuTransTableCreate(
    void
    )
{
    MMU_TRANS_TABLE *newTransTable;

    newTransTable = (MMU_TRANS_TABLE *) malloc(sizeof(MMU_TRANS_TABLE));
    if (newTransTable != NULL)
    {
        /* Initialize table */
        if (mmuTransTableInit(newTransTable) != OK)
        {
            free(newTransTable);
            newTransTable = NULL;
        }
    }

    return newTransTable;
}

/******************************************************************************
 * mmuTransTableInit - Initialize a new translation table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuTransTableInit(
    MMU_TRANS_TABLE *transTable
    )
{
    STATUS  status;
    PTE    *pDirTable;
  
    /* Allocate memory */
    pDirTable = memalign(mmuPageSize, mmuPageSize);
    if (pDirTable == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Set as global translation table */
        transTable->pDirTable = pDirTable;

        /* Copy global directory table */
        memcpy(pDirTable, mmuGlobalTransTable.pDirTable, PD_SIZE);

        /* Write protect directory table */
        (void) mmuStateSet(
                   &mmuGlobalTransTable,
                   pDirTable,
                   MMU_STATE_MASK_WRITABLE,
                   MMU_STATE_NOT_WRITABLE
                   );

        /* Ignore status from mmuStateSet */
        status = OK;
    }

    return status;
}

/******************************************************************************
 * mmuTransTableDestroy - Destroy translation table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuTransTableDestroy(
    MMU_TRANS_TABLE *transTable
    )
{
    int     i;
    PTE    *pDte;
    PTE    *pPageTable;

    /* Unlock page for writing */
    (void) mmuStateSet(
               &mmuGlobalTransTable,
               transTable->pDirTable,
               MMU_STATE_MASK_WRITABLE,
               MMU_STATE_WRITABLE
               );

    /* Free non global page memory */
    if (mmuPageSize == PAGE_SIZE_4KB)
    {
        pDte = transTable->pDirTable;

        for (i = 0; i < (PT_SIZE / sizeof(PTE)); i++, pDte++)
        {
            if ((pDte->entry.present == 1) && (!globalPageBlock[i]))
            {
                pPageTable = (PTE *) (pDte->bits & PTE_TO_ADDR_4KB);

                /* Unlock page to table writing */
                (void) mmuStateSet(
                           &mmuGlobalTransTable,
                           pPageTable,
                           MMU_STATE_MASK_WRITABLE,
                           MMU_STATE_WRITABLE
                           );
                free(pPageTable);
            }
        }
    }

    /* Free page with directory table and storage */
    free(transTable->pDirTable);
    free(transTable);

    return OK;
}

/******************************************************************************
 * mmuPageCreate - Create a virtual memory page
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuPageCreate(
    MMU_TRANS_TABLE *thisTable,
    void            *vPageAddr
    )
{
    STATUS  status;
    PTE    *pDte;
    PTE    *pPageTable;
    int     i;
    PTE    *dummy;

    if (mmuPteGet(thisTable, vPageAddr, &dummy) == OK)
    {
        /* Page already exists. */
        status = OK;
    }
    else
    {
        /* Select page size */
        if (mmuPageSize == PAGE_SIZE_4KB)
        {
            pPageTable = (PTE *) memalign(mmuPageSize, mmuPageSize);
            if (pPageTable == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Clear page table */
                for (i = 0; i < (PT_SIZE / sizeof(PTE)); i++)
                {
                    pPageTable[i].entry.present  = 0;
                    pPageTable[i].entry.rw       = 0;
                    pPageTable[i].entry.us       = 0;
                    pPageTable[i].entry.pwt      = 0;
                    pPageTable[i].entry.pcd      = 0;
                    pPageTable[i].entry.access   = 0;
                    pPageTable[i].entry.dirty    = 0;
                    pPageTable[i].entry.pagesize = 0;
                    pPageTable[i].entry.global   = 0;
                    pPageTable[i].entry.avail    = 0;
                    pPageTable[i].entry.page     = -1;
                }

                /* Write protect page table */
                (void) mmuStateSet(
                           &mmuGlobalTransTable,
                           pPageTable,
                           MMU_STATE_MASK_WRITABLE,
                           MMU_STATE_NOT_WRITABLE
                           );
            }
        }
        else
        {
            pPageTable = (PTE *) ((u_int32_t) vPageAddr & ADDR_TO_PAGEBASE);
            status = OK;
        }

        if (status == OK)
        {
            /* Unlock directory table page for writing */
            (void) mmuStateSet(
                       &mmuGlobalTransTable,
                       thisTable->pDirTable,
                       MMU_STATE_MASK_WRITABLE,
                       MMU_STATE_WRITABLE
                       );

            pDte = &thisTable->pDirTable[
                       ((u_int32_t) vPageAddr & DIR_BITS) >> DIR_INDEX
                       ];

            /* Set directory table entry to new page table */
            pDte->entry.page    = (u_int32_t) pPageTable >> ADDR_TO_PAGE;
            pDte->entry.present = 1;
            pDte->entry.rw = 1;

            /* Write protect directory table page again, after update */
            (void) mmuStateSet(
                       &mmuGlobalTransTable,
                       thisTable->pDirTable,
                       MMU_STATE_MASK_WRITABLE,
                       MMU_STATE_NOT_WRITABLE
                       );

            if (firstTime == FALSE)
            {
                MMU_TLB_FLUSH();
            }
        }
    }

    return status;
}

/******************************************************************************
 * mmuSetState - Set state of a virtual memory page
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuStateSet(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    u_int32_t        stateMask,
    u_int32_t        state
    )
{
    STATUS  status;
    PTE    *pPte;
    BOOL    wasEnabled;
    BOOL    level = 0;

    /* Get page table entry */
    if (mmuPteGet(transTable, vAddr, &pPte) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Turn off MMU and modify page table entry */
        MMU_UNLOCK(wasEnabled, level);
        pPte->bits = (pPte->bits & ~stateMask) | (state & stateMask);
        MMU_LOCK(wasEnabled, level);

        /* Flush if not first time */
        if (firstTime == FALSE)
        {
            MMU_TLB_FLUSH();
            cacheArchClearEntry(DATA_CACHE, pPte);
        }

        status = OK;
    }

    return status;
}

/******************************************************************************
 * mmuStateGet - Get state of a virtual memory page
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuStateGet(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    u_int32_t       *state
    )
{
    STATUS  status;
    PTE    *pPte;

    /* Get page table entry */
    if (mmuPteGet(transTable, vAddr, &pPte) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Store state */
        *state = pPte->bits;

        status = OK;
    }

    return status;
}

/******************************************************************************
 * mmuPageMap - Map a physical page to virtual address
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuPageMap(
    MMU_TRANS_TABLE *transTable,
    void            *vAddr,
    void            *pAddr
    )
{
    STATUS  status;
    PTE    *pPte;
    BOOL    wasEnabled;
    int     level = 0;

    /* Get current status */
    status = mmuPteGet(transTable, vAddr, &pPte);

    /* Do if page size is 4kB and non existant */
    if ((mmuPageSize == PAGE_SIZE_4KB) && (status != OK))
    {
        /* Page does not exist yet */
        if (mmuPageCreate(transTable, vAddr) != OK)
        {
            status = ERROR;
        }
        else
        {
            status = mmuPteGet(transTable, vAddr, &pPte);
        }
    }

    if (status == OK)
    {
        MMU_UNLOCK(wasEnabled, level);

        /* Do if page size is 4MB */
        if (mmuPageSize == PAGE_SIZE_4MB)
        {
            pAddr = (void *) ((u_int32_t) pAddr & ADDR_TO_PAGEBASE);
        }

        pPte->entry.page = (u_int32_t) pAddr >> ADDR_TO_PAGE;

        MMU_LOCK(wasEnabled, level);

        MMU_TLB_FLUSH();
        cacheArchClearEntry(DATA_CACHE, pPte);
    }

    return status;
}

/******************************************************************************
 * mmuGlobalPageMap - Map a memory page in global table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuGlobalPageMap(
    void *vAddr,
    void *pAddr
    )
{
    STATUS  status;
    PTE    *pPte;
    BOOL    wasEnabled;
    int     level = 0;

    /* Get status of page table entry */
    status = mmuPteGet(&mmuGlobalTransTable, vAddr, &pPte);

    /* If page does not exist, and page size is 4k  */
    if ((mmuPageSize == PAGE_SIZE_4KB) && (status != OK))
    {
        /* Build a new virtual page */
        if (mmuPageCreate(&mmuGlobalTransTable, vAddr) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* See if insert was successfull */
            if (mmuPteGet(&mmuGlobalTransTable, vAddr, &pPte) != OK)
            {
                status = ERROR;
            }
            else
            {
                /* Unlock global page block for writing */
                (void) mmuStateSet(
                           &mmuGlobalTransTable,
                           globalPageBlock,
                           MMU_STATE_MASK_WRITABLE,
                           MMU_STATE_WRITABLE
                           );

                /* Mark page as used */
                globalPageBlock[
                    ((u_int32_t) vAddr & DIR_BITS) >> DIR_INDEX
                    ] = TRUE;

                /* Write protect global page block again */
                (void) mmuStateSet(
                           &mmuGlobalTransTable,
                           globalPageBlock,
                           MMU_STATE_MASK_WRITABLE,
                           MMU_STATE_NOT_WRITABLE
                           );
            }
        }
    }

    if (status == OK)
    {
        MMU_UNLOCK(wasEnabled, level);

        if (mmuPageSize == PAGE_SIZE_4MB)
        {
            pAddr = (void *) ((u_int32_t) pAddr & ADDR_TO_PAGEBASE);
        }

        pPte->entry.page = (u_int32_t) pAddr >> ADDR_TO_PAGE;

        MMU_LOCK(wasEnabled, level);

        /* Flush if needed */
        if (firstTime == FALSE)
        {
            MMU_TLB_FLUSH();
            cacheArchClearEntry(DATA_CACHE, pPte);
        }
    }

    return status;
}

/******************************************************************************
 * mmuTranslate - Translate a virtual address to physical
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS mmuTranslate(
    MMU_TRANS_TABLE  *transTable,
    void             *vAddr,
    void            **pAddr
    )
{
    STATUS  status;
    PTE    *pPte;

    /* Get page table entry */
    if (mmuPteGet(transTable, vAddr, &pPte) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Check if it exists */
        if (pPte->entry.present == 0)
        {
            errnoSet(S_mmuLib_PAGE_NOT_PRESENT);
            status = ERROR;
        }
        else
        {
            /* Find page */
            if (mmuPageSize == PAGE_SIZE_4KB)
            {
                *pAddr = (void *) ((u_int32_t)(pPte->bits & PTE_TO_ADDR_4KB) +
                                  ((u_int32_t) vAddr & OFFSET_BITS_4KB));
            }
            else
            {
                *pAddr = (void *) ((u_int32_t)(pPte->bits & PTE_TO_ADDR_4MB) +
                                  ((u_int32_t) vAddr & OFFSET_BITS_4MB));
            }

            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * mmuCurrentSet - Set current translation table
 *
 * RETURNS: N/A
 */

LOCAL void mmuCurrentSet(
    MMU_TRANS_TABLE *transTable
    )
{
    int  level = 0;

    /* If first time, write disable global translation table */
    if (firstTime == TRUE)
    {
        mmuMemPagesWriteDisable(&mmuGlobalTransTable);
        mmuMemPagesWriteDisable(transTable);

        /* Invalidate caches */
#if (CPU != I80386)
        __asm__ __volatile__("wbinvd");
#endif

        firstTime = FALSE;
    }

    /* Disable interrputs */
    INT_LOCK(level);

    /* Store translation table as current */
    mmuCurrentTransTable = transTable;

    /* Setup mmu register */
    mmuPro32PdbrSet(transTable);

    /* Enable interrupts */
    INT_UNLOCK(level);
}

/******************************************************************************
 * mmuMemPagesWriteDisable - Write disable a series of pages
 *
 * RETURNS: N/A
 */

LOCAL void mmuMemPagesWriteDisable(
    MMU_TRANS_TABLE *transTable
    )
{
    void   *thisPage;
    int     i;

    if (mmuPageSize == PAGE_SIZE_4KB)
    {
        for (i = 0; i < (PD_SIZE / sizeof(PTE)); i++)
        {
            /* Write protect individual pages */
            thisPage = (void *) (transTable->pDirTable[i].bits &
                                 PTE_TO_ADDR_4KB);

            if ((u_int32_t) thisPage != (0xffffffff & PTE_TO_ADDR_4KB))
            {
                (void) mmuStateSet(
                           transTable,
                           thisPage,
                           MMU_STATE_MASK_WRITABLE,
                           MMU_STATE_NOT_WRITABLE
                           );
            }
        }
    }

    /* Write protect translation table */
    (void) mmuStateSet(
               transTable,
               transTable->pDirTable,
               MMU_STATE_MASK_WRITABLE,
               MMU_STATE_NOT_WRITABLE
               );
}

