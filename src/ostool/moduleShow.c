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

/* moduleShow.c - Module show routines */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <os/classLib.h>
#include <ostool/loadLib.h>
#include <ostool/moduleLib.h>
#include <ostool/moduleShow.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL char *moduleShowHeader[] =
{
    "\n"
    "MODULE NAME     MODULE ID  GROUP #    TEXT START DATA START  BSS START\n"
    "--------------- ---------- ---------- ---------- ----------  ---------\n"
};
LOCAL char moduleShowFormat[] = "%15s %#10x %10d %#10x %#10x %#10x\n";

LOCAL BOOL moduleDisplay(
    MODULE_ID modId,
    int       options
    );

/* Globals */

/* Functions */

/******************************************************************************
 * moduleShowInit - Inititalize module show
 *
 * RETURNS OK or ERROR
 */

STATUS moduleShowInit(
    void
    )
{
    return classShowConnect(moduleClassId, (FUNCPTR) moduleShow);
}

/******************************************************************************
 * moduleShow - Show module info
 *
 * RETURNS OK or ERROR
 */

STATUS moduleShow(
    ARG modNameOrId,
    int options
    )
{
    STATUS    status;
    MODULE_ID modId;

    /* If id or name given */
    if (modNameOrId != (ARG) 0)
    {
        modId = moduleIdFigure(modNameOrId); 
        if (modId == NULL)
        {
            fprintf(stderr, "Module not found\n");
            status = ERROR;
        }
        else
        {
            /* Print header */
            printf("%s", moduleShowHeader[0]);

            /* Default options */
            if (options == 0)
            {
                options = MODDISPLAY_ALL;
            }

            /* Show module info */
            if (moduleDisplay(modId, options) == TRUE)
            {
                status = OK;
            }
            else
            {
                status = ERROR;
            }
        }
    }
    else
    {
        /* Print header */
        printf("%s", moduleShowHeader[0]);

        if (moduleEach((FUNCPTR) moduleDisplay, (ARG) options) == NULL)
        {
            status = OK;
        }
        else
        {
            status = ERROR;
        }
    }

    return status;
}

/******************************************************************************
 * moduleDisplay - Display module information
 *
 * RETURNS TRUE or FALSE
 */

LOCAL BOOL moduleDisplay(
    MODULE_ID modId,
    int       options
    )
{
    BOOL            result;
    MODULE_SEG_INFO segInfo;

    /* If hidden module */
    if (modId->flags & HIDDEN_MODULE)
    {
        result = TRUE;
    }
    else
    {
        /* Get module info */
        if (moduleSegInfoGet(modId, &segInfo) != OK)
        {
            fprintf(stderr, "Can't get information about module %#x\n", modId);
            result = FALSE;
        }
        else
        {
            /* Print */
            printf(
                moduleShowFormat,
                modId->name,
                (int) modId,
                modId->group,
                segInfo.textAddr,
                segInfo.dataAddr,
                segInfo.bssAddr
                );

            /* If codesize display */
            if (options & MODDISPLAY_CODESIZE)
            {
                printf("\n");
                printf("Size of text segment: %8d\n", segInfo.textSize);
                printf("Size of data segment: %8d\n", segInfo.dataSize);
                printf("Size of bss  segment: %8d\n", segInfo.bssSize);
                printf(
                    "Total size          : %8d\n\n",
                    segInfo.textSize + segInfo.dataSize + segInfo.bssSize
                    );
            }

            result = TRUE;
        }
    }

    return result;
}

