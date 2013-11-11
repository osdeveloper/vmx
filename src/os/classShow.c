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

/* classShow.c - Class show utilities */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <a.out.h>
#include <vmx.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <os/classLib.h>
#include <os/classShow.h>

/* Defines */

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Locals */
LOCAL void classShowSymbol(
    void *value
    );

/* Globals */

/* Functions */

/******************************************************************************
 * classShowInit - Initialize class show utilities
 *
 * RETURNS: OK or ERROR
 */

STATUS classShowInit(
    void
    )
{
    return classShowConnect(rootClassId, (FUNCPTR) classShow);
}

/******************************************************************************
 * classShowSymbol - Show class symbolic value
 *
 * RETURNS: N/A
 */

LOCAL void classShowSymbol(
    ARG value
    )
{
    SYMBOL_ID symId;
    char *name = NULL;
    ARG symValue = (ARG) 0;

    /* If no value */
    if (value == (ARG) 0)
    {
        fprintf(stderr, "No routine attached.\n");
    }
    else
    {
        /* If symbol found */
        if (symFindSymbol(
                sysSymTable,
                NULL,
                value,
                SYM_MASK_NONE,
                SYM_MASK_NONE,
                &symId
                ) == OK)
        {
            /* Get name and value */
            symNameGet(symId, &name);
            symValueGet(symId, &symValue);
        }

        if (symValue == value)
        {
            printf(" (%s)", name);
        }

        printf("\n");
    }
}

/******************************************************************************
 * classShow - Show class status
 *
 * RETURNS: OK or ERROR
 */

STATUS classShow(
    CLASS_ID classId,
    int mode
    )
{
    STATUS status;

    /* Verify object class */
    if (OBJ_VERIFY(classId, rootClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        printf("\n");
        printf("Memory partition id : %#-10x", classId->objPartId);
        classShowSymbol(classId->objPartId);

        printf("Object Size         : %-10d\n", classId->objSize);
        printf("Objects Allocated   : %-10d\n", classId->objAllocCount);
        printf("Objects Deallocated : %-10d\n", classId->objFreeCount);
        printf("Objects Initialized : %-10d\n", classId->objInitCount);
        printf("Objects Terminated  : %-10d\n", classId->objTerminateCount);

        printf("Create Routine      : %-#10x", classId->createMethod);
        classShowSymbol(classId->createMethod);

        printf("Init Routine        : %-#10x", classId->initMethod);
        classShowSymbol(classId->initMethod);

        printf("Destroy Routine     : %-#10x", classId->destroyMethod);
        classShowSymbol(classId->destroyMethod);

        printf("Show Routine        : %-#10x", classId->showMethod);
        classShowSymbol(classId->showMethod);
        status = OK;
    }

    return status;
}

