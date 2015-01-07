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

/* i8042KbdMse.c - Intel 8042 keyboard and mouse driver */

#include <stdlib.h>
#include <vmx.h>
#include <arch/sysArchLib.h>
#include <vmx/taskLib.h>
#include <vmx/wdLib.h>
#include <drv/input/i8042.h>

IMPORT int sysClockRateGet();

LOCAL BOOL    i8042Timeout;
LOCAL int     i8042TimeoutCount;
LOCAL WDOG_ID i8042Wdid;

LOCAL void i8042MseHwInit(
    I8042_MSE_DEVICE *pDev
    );

LOCAL int i8042MseOpen(
    I8042_MSE_DEVICE *pDev,
    char             *name,
    int               flags,
    int               mode
    );

LOCAL int i8042MseCreat(
    I8042_MSE_DEVICE *pDev,
    char             *name,
    int               mode,
    const char       *symlink
    );

LOCAL int i8042MseRead(
    I8042_MSE_DEVICE *pDev,
    void             *buffer,
    int               maxBytes
    );

LOCAL int i8042MseWrite(
    I8042_MSE_DEVICE *pDev,
    void             *buffer,
    int               maxBytes
    );

LOCAL STATUS i8042MseIoctl(
    I8042_MSE_DEVICE *pDev,
    int               req,
    int               arg
    );

LOCAL int i8042MseDelete(
    I8042_MSE_DEVICE *pDev,
    const char       *filename,
    mode_t            mode
    );

LOCAL int i8042MseClose(
    I8042_MSE_DEVICE *pDev
    );

LOCAL void i8042MseTxStart(
    I8042_MSE_DEVICE *pDev
    );

LOCAL void i8042Wd(
    void
    );

LOCAL STATUS i8042Command(
    u_int32_t reg,
    u_int8_t  cmd
    );

LOCAL STATUS i8042Read(
    u_int32_t cmdReg,
    u_int32_t dataReg,
    u_int8_t *pData
    );

LOCAL STATUS i8042Write(
    u_int32_t cmdReg,
    u_int32_t dataReg,
    u_int8_t  dData
    );

LOCAL void i8042Intr(
    I8042_MSE_DEVICE *pDev
    );

/******************************************************************************
 * i8042MseDevCreate - Create mouse driver
 *
 * RETURNS: N/A
 */

int i8042MseDevCreate(
    char *name
    )
{
    int               result;
    int               drvNum;
    I8042_MSE_DEVICE *pDev;

    drvNum = iosDrvInstall(
        (FUNCPTR) i8042MseOpen,
        (FUNCPTR) i8042MseDelete,
        (FUNCPTR) i8042MseOpen,
        (FUNCPTR) i8042MseClose,
        (FUNCPTR) i8042MseRead,
        (FUNCPTR) i8042MseWrite,
        (FUNCPTR) i8042MseIoctl
        );
    if (drvNum == ERROR)
    {
       result = ERROR;
    }
    else
    {
        pDev = (I8042_MSE_DEVICE *) malloc(sizeof(I8042_MSE_DEVICE));
        if (pDev == NULL)
        {
            result = ERROR;
        }
        else
        {
            if (tyDevInit(
                    &pDev->tyDev,
                    512,
                    512,
                    (FUNCPTR) i8042MseTxStart) != OK)
            {
                free(pDev);
                result = ERROR;
            }
            else
            {
                /* TODO: Initialize device cmdReg and dataReg */

                /* TODO: Connect interrupt handler */

                i8042MseHwInit(pDev);

                /* TODO: Enable interrupt */

                if (iosDevAdd(&pDev->tyDev.devHeader, name, drvNum) != OK)
                {
                    free(pDev);
                    result = ERROR;
                }
                else
                {
                    result = drvNum;
                }
            }
        }
    }

    return result;
}

/******************************************************************************
 * i8042MseHwInit - Initialize hardware
 *
 * RETURNS: N/A
 */

LOCAL void i8042MseHwInit(
    I8042_MSE_DEVICE *pDev
    )
{
    /* TODO */
}

/******************************************************************************
 * i8042MseOpen - Open file on device
 *
 * RETURNS: Device number
 */

LOCAL int i8042MseOpen(
    I8042_MSE_DEVICE *pDev,
    char             *name,
    int               flags,
    int               mode
    )
{
    return (int) pDev;
}

/******************************************************************************
 * i8042MseCreat - Create file on device
 *
 * RETURNS: ERROR
 */

LOCAL int i8042MseCreat(
    I8042_MSE_DEVICE *pDev,
    char             *name,
    int               mode,
    const char       *symlink
    )
{
    return ERROR;
}

/******************************************************************************
 * i8042MseRead - Read from device
 *
 * RETURNS: Number of bytes read
 */

LOCAL int i8042MseRead(
    I8042_MSE_DEVICE *pDev,
    void             *buffer,
    int               maxBytes
    )
{
    return tyRead(&pDev->tyDev, buffer, maxBytes);
}

/******************************************************************************
 * i8042MseWrite - Write to device
 *
 * RETURNS: Number of bytes written
 */

LOCAL int i8042MseWrite(
    I8042_MSE_DEVICE *pDev,
    void             *buffer,
    int               maxBytes
    )
{
    return tyWrite(&pDev->tyDev, buffer, maxBytes);
}

/******************************************************************************
 * i8042MseIoctl - I/O settings
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS i8042MseIoctl(
    I8042_MSE_DEVICE *pDev,
    int               req,
    int               arg
    )
{
    return tyIoctl(&pDev->tyDev, req, arg);
}

/******************************************************************************
 * i8042MseDelete - Delete file on device
 *
 * RETURNS: ERROR
 */

LOCAL int i8042MseDelete(
    I8042_MSE_DEVICE *pDev,
    const char       *filename,
    mode_t            mode
    )
{
    return ERROR;
}

/******************************************************************************
 * i8042MseClose - Close device
 *
 * RETURNS: OK
 */

LOCAL int i8042MseClose(
    I8042_MSE_DEVICE *pDev
    )
{
   return OK;
}

/******************************************************************************
 * i8042MseTxStart - Transmit data to mouse
 *
 * RETURNS: N/A
 */

LOCAL void i8042MseTxStart(
    I8042_MSE_DEVICE *pDev
    )
{
    char ch;

    while (tyIntTx(&pDev->tyDev, &ch) == OK);
}

/******************************************************************************
 * i8042Wd - Keyboard and mouse watchdog handler
 *
 * RETURNS: N/A
 */

LOCAL void i8042Wd(
    void
    )
{
    i8042Timeout = TRUE;
    i8042TimeoutCount++;
}

/******************************************************************************
 * i8042Command - Send command to keyboard and mouse controller chip
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS i8042Command(
    u_int32_t reg,
    u_int8_t  cmd
    )
{
    STATUS status = ERROR;

    /* Start watchdog */
    i8042Timeout = FALSE;
    wdStart(i8042Wdid,
            sysClockRateGet() * I8042_WAIT_SEC,
            (FUNCPTR) i8042Wd,
            0);

    /* Wait for input buffer */
    while ((sysInByte(reg) & I8042_KBD_IBFULL) && (i8042Timeout == FALSE));

    /* Send command */
    sysOutByte(reg, cmd);

    /* Wait for command completion */
    while ((sysInByte(reg) & I8042_KBD_IBFULL) && (i8042Timeout == FALSE));
    wdCancel(i8042Wdid);

    if (i8042Timeout == FALSE)
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * i8042Read - Read data from keyboard and mouse controller
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS i8042Read(
    u_int32_t cmdReg,
    u_int32_t dataReg,
    u_int8_t *pData
    )
{
    STATUS status = ERROR;

    /* Start watchdog */
    i8042Timeout = FALSE;
    wdStart(i8042Wdid,
            sysClockRateGet() * I8042_WAIT_SEC,
            (FUNCPTR) i8042Wd,
            0);

    /* Wait for output buffer to be ready */
    while (((sysInByte(cmdReg) & I8042_KBD_OBFULL)  == 0) &&
            (i8042Timeout == FALSE));
    wdCancel(i8042Wdid);
    taskDelay(sysClockRateGet() >> 4);

    /* Read data */
    *pData = sysInByte(dataReg);

    if (i8042Timeout == FALSE)
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * i8042Write - Write data to keyboard and mouse controller
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS i8042Write(
    u_int32_t cmdReg,
    u_int32_t dataReg,
    u_int8_t  data
    )
{
    STATUS status = ERROR;

    /* Start watchdog */
    i8042Timeout = FALSE;
    wdStart(i8042Wdid,
            sysClockRateGet() * I8042_WAIT_SEC,
            (FUNCPTR) i8042Wd,
            0);

    /* Wait for output buffer to be ready */
    while ((sysInByte(cmdReg) & I8042_KBD_IBFULL) && (i8042Timeout == FALSE));
    wdCancel(i8042Wdid);

    /* Write data */
    sysOutByte(dataReg, data);

    if (i8042Timeout == FALSE)
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * i8042Intr - Keyboard and mouse controller interrupt handler
 *
 * RETURNS: N/A
 */

LOCAL void i8042Intr(
    I8042_MSE_DEVICE *pDev
    )
{
    u_int8_t inByte;

    if (sysInByte(pDev->cmdReg) & (I8042_KBD_OBFULL | I8042_KBD_AUXB))
    {
        inByte = sysInByte(pDev->dataReg);
        tyIntRd(&pDev->tyDev, inByte);
    }
}

