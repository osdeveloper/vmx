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

/* mmuTest.c - Testing of memory mapping unit */

#include <stdio.h>
#include <stdlib.h>
#include <a.out.h>
#include <vmx.h>
#include <arch/mmuArchLib.h>
#include <os/errnoLib.h>
#include <os/logLib.h>
#include <os/symLib.h>
#include <os/vmLib.h>
#include "config.h"

/* Imports */
IMPORT SYMTAB_ID sysSymTable;
STATUS mmuPro32LibInit();

int mmuInit(void)
{
    /* If initialization of mmu unit fails */
    if (mmuPro32LibInit(VM_PAGE_SIZE) != OK)
    {
        logMsg(
            "mmuInit: MMU configuration failed, errno = %#x",
            (ARG) errno,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
        return ERROR;
    }

    /* In initialization of virual memory */
    if (vmLibInit(VM_PAGE_SIZE) != OK)
    {
        logMsg(
            "mmuInit: MMU configuration failed, errno = %#x",
            (ARG) errno,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
        return ERROR;
    }

    return OK;
}

int mmuGlobalMapTest(
    void *vAddr,
    void *pAddr,
    unsigned len
    )
{
    PHYS_MEM_DESC memDesc[1];

    memDesc[0].vAddr       = vAddr;
    memDesc[0].pAddr       = pAddr;
    memDesc[0].len         = len;
    memDesc[0].initialMask = VM_STATE_MASK_VALID|
                             VM_STATE_MASK_WRITABLE|
                             VM_STATE_MASK_CACHEABLE;
    memDesc[0].initialState = VM_STATE_VALID|
                              VM_STATE_WRITABLE|
                              VM_STATE_CACHEABLE;

    return (int) vmGlobalMapInit(memDesc, 1, TRUE);
}

int mmuTestInit(
    void
    )
{
    static SYMBOL symTableMmuTest[] =
    {
        {NULL, "_mmuGlobalMapTest", mmuGlobalMapTest, 0, N_TEXT | N_EXT}
    };

    int i;

    if (mmuInit() != OK)
    {
        fprintf(stderr, "Error - Unable to initialize mmu.\n");
        return ERROR;
    }

    for (i = 0; i < NELEMENTS(symTableMmuTest); i++)
    {
        symTableAdd(sysSymTable, &symTableMmuTest[i]);
    }

    return i;
}

