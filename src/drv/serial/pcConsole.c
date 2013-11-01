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

/* pcConsole.c - PC console */

#include <sys/types.h>
#include <vmx.h>
#include <io/iosLib.h>
#include <io/ioLib.h>
#include <io/tyLib.h>
#include <drv/input/i8042Kbd.h>
#include <drv/video/m6845Vga.h>
#include <drv/serial/pcConsole.h>
#include <stdlib.h>

/* Locals */
LOCAL int pcNumber = 0;

LOCAL void pcConDrvHrdInit(
    void
    );

LOCAL int pcConDrvOpen(
    PC_CON_DEV *pc,
    char *name,
    int flags,
    int mode
    );

LOCAL int pcConDrvCreat(
    PC_CON_DEV *pc,
    char *name,
    int mode,
    const char *symlink
    );

LOCAL int pcConDrvRead(
    PC_CON_DEV *pc,
    void *buffer,
    int maxBytes
    );

LOCAL int pcConDrvWrite(
    PC_CON_DEV *pc,
    void *buffer,
    int maxBytes
    );

LOCAL STATUS pcConDrvIoctl(
    PC_CON_DEV *pc,
    int req,
    int arg
    );

LOCAL int pcConDrvDelete(
    PC_CON_DEV *pc,
    const char *filename,
    mode_t mode
    );

LOCAL int pcConDrvClose(
    PC_CON_DEV *pc
    );

/******************************************************************************
 * pcConDrvInit - Initialize console
 *
 * RETURNS: OK or ERROR
 */

STATUS pcConDrvInit(
    void
    )
{
    STATUS status;

    if (pcNumber > 0)
    {
        status = OK;
    }
    else
    {
        pcConDrvHrdInit();

        /* Install driver */
        pcNumber = iosDrvInstall(
                       (FUNCPTR) pcConDrvOpen,
                       (FUNCPTR) pcConDrvDelete,
                       (FUNCPTR) pcConDrvOpen,
                       (FUNCPTR) pcConDrvClose,
                       (FUNCPTR) pcConDrvRead,
                       (FUNCPTR) pcConDrvWrite,
                       (FUNCPTR) pcConDrvIoctl
                       );
        if (pcNumber == ERROR)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * pcConDevCreate - Create device
 *
 * RETURNS: OK or ERROR
 */

STATUS pcConDevCreate(
    char *name,
    int channel,
    int readBufferSize,
    int writeBufferSize
    )
{
    STATUS status;
    PC_CON_DEV *pc;

    if (pcNumber <= 0)
    {
        status = ERROR;
    }
    else
    {
        if ((channel < 0)  || (channel >= N_VIRTUAL_CONSOLES))
        {
            status = ERROR;
        }
        else
        {
            pc = &pcConDev[channel];
            if (pc->created == TRUE)
            {
                status = ERROR;
            }
            else
            {
                if (tyDevInit(
                        &pc->tyDev,
                        readBufferSize,
                        writeBufferSize,
                        vgaWriteString
                        ) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    /* Enable interrupt level for keyboard */
                    sysIntEnablePIC(KBD_INT_LVL);
                    pc->created = TRUE;
                    status = iosDevAdd(&pc->tyDev.devHeader, name, pcNumber);
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * pcConDrvNumber - Get console number
 *
 * RETURNS: Console number
 */

int pcConDrvNumber(
    void
    )
{
    return pcNumber;
}

/******************************************************************************
 * pcConDrvHrdInit - Initalize hardware
 *
 * RETURNS: N/A
 */

LOCAL void pcConDrvHrdInit(
    void
    )
{
    int level;

    INT_LOCK(level);

    kbdHrdInit();
    vgaHrdInit();

    INT_UNLOCK(level);
}

/******************************************************************************
 * pcConDrvOpen - Open a file to console
 *
 * RETURNS: Device number
 */

LOCAL int pcConDrvOpen(
    PC_CON_DEV *pc,
    char *name,
    int flags,
    int mode
    )
{
    return (int) pc;
}

/******************************************************************************
 * pcConDrvCreat - Creat a file to console
 *
 * RETURNS: ERROR
 */

LOCAL int pcConDrvCreat(
    PC_CON_DEV *pc,
    char *name,
    int mode,
    const char *symlink
    )
{
    return ERROR;
}

/******************************************************************************
 * pcConDrvRead - Read from console
 *
 * RETURNS: number of bytes read
 */

LOCAL int pcConDrvRead(
    PC_CON_DEV *pc,
    void *buffer,
    int maxBytes
    )
{
    return tyRead(&pc->tyDev, buffer, maxBytes);
}

/******************************************************************************
 * pcConDrvWrite - Write to console
 *
 * RETURNS: number of bytes written
 */

LOCAL int pcConDrvWrite(
    PC_CON_DEV *pc,
    void *buffer,
    int maxBytes
    )
{
    return tyWrite(&pc->tyDev, buffer, maxBytes);
}

/******************************************************************************
 * pcConDrvIoctl - I/O control
 *
 * RETURNS: OR or ERROR
 */

LOCAL STATUS pcConDrvIoctl(
    PC_CON_DEV *pc,
    int req,
    int arg
    )
{
    STATUS status;

    switch(req)
    {
      default:
          status = tyIoctl(&pc->tyDev, req, arg);
          break;
    }

    return status;
}

/******************************************************************************
 * pcConDrvDelete - Delete console
 *
 * RETURNS: ERROR
 */

LOCAL int pcConDrvDelete(
    PC_CON_DEV *pc,
    const char *filename,
    mode_t mode
    )
{
    return ERROR;
}

/******************************************************************************
 * pcConDrvClose - Close console
 *
 * RETURNS: OK
 */

LOCAL int pcConDrvClose(
    PC_CON_DEV *pc
    )
{
    return OK;
}

