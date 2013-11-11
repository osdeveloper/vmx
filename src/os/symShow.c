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

/* symShow.c - Symbol table show routines */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <a.out.h>
#include <vmx.h>
#include <usr/usrPage.c>
#include <os/classLib.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <os/symShow.h>

/* Defines */

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Locals */
LOCAL char *symTypeName[] =
{
    "???",
    "abs",
    "text",
    "data",
    "bss"
};

LOCAL char* symStringMatch(
    char *s1,
    char *s2
    );

LOCAL BOOL symPrint(
    char *name,
    int val,
    unsigned char type,
    char *str
    );

LOCAL BOOL symSysTablePrint(
    char *name,
    int val,
    unsigned char type,    
    char *str,
    unsigned short group
    );

/* Functions */

/******************************************************************************
 * symShowInit - Initialize symbol show facility
 *
 * RETURNS: OK or ERROR
 */

STATUS symShowInit(
    void
    )
{
    usrPageInit(24);

    return classShowConnect(symTableClassId, (FUNCPTR) symShow);
}

/******************************************************************************
 * symShow - Show a symbol
 *
 * RETURNS: OK or ERROR
 */

STATUS symShow(
    SYMTAB_ID symTableId,
    char *str
    )
{
    STATUS status;
    char nameClash[12];

    /* Verify object class */
    if (OBJ_VERIFY(symTableId, symTableClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* If search string given */
        if (str != NULL)
        {
            /* Seach for partial matching symbol name */

            /* If system symbol table */
            if (symTableId != sysSymTable)
            {
                symEach(symTableId, (FUNCPTR) symPrint, (ARG) str);
            }
            else
            {
                symEach(symTableId, (FUNCPTR) symSysTablePrint, (ARG) str);
                usrPageReset();
            }

            status = OK;
        }
        else
        {
            /* Get name clash string */
            if (symTableId->sameNameOk)
            {
                strcpy(nameClash, "Allowed");
            }
            else
            {
                strcpy(nameClash, "Disallowed");
            }

            /* Print symbol table summary */
            printf("Number of Symbols   : %-10d\n", symTableId->numSymbols);
            printf("Symbol Mutex Id     : %-#10x\n", symTableId->symMutex);
            printf("Symbol Hash Id      : %-#10x\n", symTableId->nameHashId);
            printf("Symbol memPartId    : %-#10x\n", symTableId->partId);
            printf("Name Clash Policy   : %-10s\n", nameClash);

            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * symStringMatch - Match strings for symbol table searcg
 *
 * RETURNS: NULL or match string
 */

LOCAL char* symStringMatch(
    char *s1,
    char *s2
    )
{
    int    len = strlen(s2);
    int    n   = strlen(s1) - len;
    char  *ret = NULL;

    /* While not all characters processed */
    for (; n >= 0; s1++, --n)
    {
        /* If symbol match */
        if (strncmp(s1, s2, len) == 0)
        {
            ret = s1;
            break;
        }

    }

    return ret;
}

/******************************************************************************
 * symPrint - Print symbol
 *
 * RETURNS: TRUE
 */

LOCAL BOOL symPrint(
    char *name,
    int val,
    unsigned char type,
    char *str
    )
{
    /* If null string or symbol name match */
    if ((str == NULL) || (symStringMatch(name, str) != NULL))
    {
        printf("-25s %#08x\n", name);
    }

    return TRUE;
}

/******************************************************************************
 * symSysTablePrint - Print symbol from system symbol table
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL symSysTablePrint(
    char *name,
    int val,
    unsigned char type,    
    char *str,
    unsigned short group
    )
{
    BOOL ret;
    char modName[80];

    /* If default module */
    if (group == symGroupDefault)
    {
        strcpy(modName, "");
    }
    else
    {
        sprintf(modName, "(%d)", group);
    }

    /* If null string or symbol match */
    if ((str == NULL) || (symStringMatch(name, str) != NULL))
    {
        printf(
            "%-25s %#08x %-8s %s",
            name,
            val,
            symTypeName[(type >> 1) & 7],
            modName
            );

        /* Print type */
        if ((type & N_EXT) == 0)
        {
            printf(" (local)");
        }

        printf("\n");

        /* Check page */
        ret = usrPageCheck();
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}

