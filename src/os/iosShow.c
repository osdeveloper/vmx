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

/* iosShow.h - Show I/O drivers and devices */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vmx.h>
#include <util/dllLib.h>
#include <os/iosLib.h>
#include <os/private/iosLibP.h>
#include <os/iosShow.h>

/******************************************************************************
 * iosShowInit - Initialize I/O show
 *
 * RETURNS: N/A
 */

void iosShowInit(
    void
    )
{
}

/******************************************************************************
 * iosDevShow - Show I/O devices
 *
 * RETURNS: N/A
 */

void iosDevShow(
    void
    )
{
    DEV_HEADER *pDevHeader;

    /* Print header */
    printf("%3s %-20s\n", "drv", "name");

    for (pDevHeader = (DEV_HEADER *) DLL_HEAD(&iosDevList);
         pDevHeader != NULL;
         pDevHeader = (DEV_HEADER *) DLL_NEXT(&pDevHeader->node))
    {
        printf("%3d %-20s\n", pDevHeader->drvNumber, pDevHeader->name);
    }
}

/******************************************************************************
 * iosFdShow - Show filedesciptors
 *
 * RETURNS: N/A
 */

void iosFdShow(
    void
    )
{
    int i, fd;
    FD_ENTRY *pFd;
    char inStr[3], outStr[3], errStr[3];
    char name[80];

    printf("%3s %-20s %3s\n", "fd", "name", "drv");

    for (i = 0; i < iosMaxFd; i++)
    {
        pFd = &iosFdTable[i];
        if (pFd->used == TRUE)
        {
            fd = STD_FIX(i);

            /* Get filedescriptor names */
            if (fd == ioGlobalStdGet(STDIN_FILENO))
            {
                strcpy(inStr, "in");
            }
            else
            {
                strcpy(inStr, "");
            }

            if (fd == ioGlobalStdGet(STDOUT_FILENO))
            {
                strcpy(outStr, "out");
            }
            else
            {
                strcpy(outStr, "");
            }

            if (fd == ioGlobalStdGet(STDERR_FILENO))
            {
                strcpy(errStr, "err");
            }
            else
            {
                strcpy(errStr, "");
            }

            if (pFd->name == NULL)
            {
                strcpy(name, "(unknown)");
            }
            else
            {
                strcpy(name, pFd->name);
            }

            printf(
                "%3d %-20s %3d %s %s %s\n",
                fd,
                name,
                pFd->pDevHeader->drvNumber,
                inStr,
                outStr,
                errStr
                );
        }
    }
}

