/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "ugl.h"
#include "driver/graphics/fbdev/udfb.h"

/* Locals */

UGL_LOCAL UGL_MODE modes[1];

UGL_LOCAL UGL_MODE *pModes = modes;

UGL_LOCAL UGL_STATUS uglFbModeAvailGet (
    UGL_DEVICE_ID     devId,
    UGL_UINT32 *      pNumModes,
    const UGL_MODE ** pModeArray
    );

UGL_LOCAL UGL_STATUS uglFbModeSet (
    UGL_DEVICE_ID  devId,
    UGL_MODE *     pMode
    );

/******************************************************************************
 *
 * uglFbDevCreate - Create framebuffer device
 *
 * RETURNS: Pointer to driver or UGL_NULL
 */

UGL_UGI_DRIVER * uglFbDevCreate (
    UGL_UINT32  arg0,
    UGL_UINT32  arg1,
    UGL_UINT32  arg2
    ) {
    UGL_FB_DRIVER *           pDrv;
    UGL_DEVICE_ID             devId;

    /* Allocate memory for driver */
    pDrv = (UGL_FB_DRIVER *) UGL_CALLOC (1, sizeof (UGL_FB_DRIVER));
    if (pDrv == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Open the file for reading and writing */
    pDrv->fd = open ("/dev/fb0", O_RDWR);
    if (pDrv->fd == -1) {
        UGL_FREE (pDrv);
        return (UGL_NULL);
    }

    /* Get fixed screen information */
    if (ioctl (pDrv->fd, FBIOGET_FSCREENINFO, &pDrv->finfo) == -1) {
        UGL_FREE (pDrv);
        close (pDrv->fd);
        return (UGL_NULL);
    }

    /* Get variable screen information */
    if (ioctl (pDrv->fd, FBIOGET_VSCREENINFO, &pDrv->vinfo) == -1) {
        UGL_FREE (pDrv);
        close (pDrv->fd);
        return (UGL_NULL);
    }

    /* Store mode info */
    modes[0].name        = strdup ("/dev/fb0");
    modes[0].width       = (UGL_UINT16) pDrv->vinfo.xres;
    modes[0].height      = (UGL_UINT16) pDrv->vinfo.yres;
    modes[0].colorDepth  = (UGL_UINT16) pDrv->vinfo.bits_per_pixel;
    modes[0].refreshRate = (UGL_UINT16) 60; /* TODO: Find out real value */
    modes[0].refreshRate = (UGL_UINT16) 60; /* TODO: Find out real value */
    modes[0].monitorType = UGL_MODE_CRT; /* TODO: Find out real value */
    modes[0].flags       = UGL_MODE_DIRECT_COLOR; /* TODO: Find out real val */

    /* Initialize device driver */
    uglUgiDevInit ((UGL_DEVICE_ID) pDrv);

    return (devId);
}

/******************************************************************************
 *
 * uglFbModeAvailGet - Get available graphics modes
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_LOCAL UGL_STATUS uglFbModeAvailGet (
    UGL_DEVICE_ID     devId,
    UGL_UINT32 *      pNumModes,
    const UGL_MODE ** pModeArray
    ) {

    *pModeArray = pModes;

    /* Calculate number of elements */
    *pNumModes = sizeof (modes) / sizeof (modes[0]);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglFbModeSet - Set graphics mode
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglFbModeSet (
    UGL_DEVICE_ID  devId,
    UGL_MODE *     pMode
    ) {
    UGL_FB_DRIVER *  pDrv;
    UGL_INT32        modeIndex;
    UGL_UINT32       fbSize;
    UGL_GEN_DDB *    pDdb;

    /* Get driver from device struct, since it it the first entry */
    pDrv = (UGL_FB_DRIVER *) devId;

    /* Get requested mode from list */
    modeIndex = uglGenericModeFind (modes, pMode,
                                    sizeof (modes) / sizeof (modes[0]));
    if (modeIndex == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    /* Store current mode */
    devId->pMode = &modes[modeIndex];

    /* Figure out the size of the screen in bytes */
    fbSize = devId->pMode->width * devId->pMode->height *
             devId->pMode->colorDepth / 8;

    /* Map the device to memory */
    pDrv->generic.fbAddress = mmap (0, fbSize,
                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                    pDrv->fd, 0);
    if ((int) pDrv->generic.fbAddress == -1) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup first drawing page */
    devId->pPageZero = (UGL_PAGE *) UGL_CALLOC (1, sizeof (UGL_PAGE) +
                                                sizeof (UGL_GEN_DDB));
    if (devId->pPageZero == UGL_NULL) {
        munmap (pDrv->generic.fbAddress, fbSize);
        return (UGL_STATUS_ERROR);
    }

    pDdb = (UGL_GEN_DDB *) &devId->pPageZero[1];
    pDdb->header.width  = devId->pMode->width;
    pDdb->header.height = devId->pMode->height;
    pDdb->header.type   = UGL_DDB_TYPE;
    pDdb->colorDepth    = devId->pMode->colorDepth;
    pDdb->stride        = pDrv->finfo.line_length;
    pDdb->pData         = pDrv->generic.fbAddress;

    devId->pPageZero->pDdb = (UGL_DDB *) pDdb;

    pDrv->generic.pDrawPage    = devId->pPageZero;
    pDrv->generic.pVisiblePage = devId->pPageZero;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglFbDevDestroy - Free frambuffer graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFbDevDestroy (
    UGL_DEVICE_ID  devId
    ) {
    UGL_UINT32      fbSize;
    UGL_FB_DRIVER * pDrv;

    /* Get framebuffer driver */
    pDrv = (UGL_FB_DRIVER *) devId;

    /* Call device deinit */
    if (uglUgiDevDeinit (devId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Figure out the size of the screen in bytes */
    fbSize = devId->pMode->width * devId->pMode->height *
             devId->pMode->colorDepth / 8;

    munmap (pDrv->generic.fbAddress, fbSize);
    close (pDrv->fd);

    /* Free memory */
    UGL_FREE (devId);

    return (UGL_STATUS_OK);
}

/* Temporary for test */
int main (void)
{
    UGL_DEVICE_ID    devId;
    UGL_MODE         gfxMode;
    UGL_FB_DRIVER *  pDrv;
    UGL_GEN_DDB *    pDdb;
    UGL_UINT8 *      pDest;
    UGL_UINT32       location;
    int              x = 100;
    int              y = 100;

    devId = uglFbDevCreate (0, 0, 0);
    if (pDrv == UGL_NULL) {
        printf("Error - Unable to open device.\n");
        return (1);
    }

    /* Enter video mode */
    gfxMode.width = 1024;
    gfxMode.height = 600;
    gfxMode.colorDepth = 32;
    gfxMode.refreshRate = 60;
    gfxMode.flags = UGL_MODE_DIRECT_COLOR;

    if (uglModeSet(devId, &gfxMode) != UGL_STATUS_OK) {
        uglFbDevDestroy (devId);
        printf("Error - Unable to set graphics mode.\n");
        return (1);
    }

    /* Get framebuffer driver */
    pDrv = (UGL_FB_DRIVER *) devId;

    /* Get drawing page */
    pDdb  = (UGL_GEN_DDB *) pDrv->generic.pDrawPage;
    pDest = (UGL_UINT8 *) pDdb->pData;

    /* Figure out where in memory to put the pixel */
    for (y = 100; y < 300; y++)
        for (x = 100; x < 300; x++) {

            location = (x + pDrv->vinfo.xoffset) *
                       (pDdb->colorDepth / 8) +
                       (y + pDrv->vinfo.yoffset) * pDdb->stride;

            if (pDdb->colorDepth == 32) {
                *(pDest + location) = 100; 
                *(pDest + location + 1) = 15 + (x - 100) / 2; 
                *(pDest + location + 2) = 200 - (y - 100) / 5; 
                *(pDest + location + 3) = 0;
                /* location += 4; */
            } else  { /* assume 16bpp */
                int b = 10;
                int g = (x - 100) / 6;
                int r = 31 - (y - 100) / 16;
                unsigned short int t = r << 11 | g << 5 | b;
                *((unsigned short int *) (pDest + location)) = t;
            }

        }

    uglFbDevDestroy (devId);

    return (0);
}
/* End temporary for test */

