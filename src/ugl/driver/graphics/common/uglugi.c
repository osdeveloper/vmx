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

/* uglugi.c - Universal graphics library common device */

#include <ugl/ugl.h>

#define UGL_SCRATCH_BUF_BLK_SIZE    64

/******************************************************************************
 *
 * uglUgiDevInit - Initialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevInit (
    UGL_DEVICE_ID  devId
    ) {

    devId->lockId = uglOSLockCreate ();
    if (devId->lockId == UGL_NULL) {
        return UGL_STATUS_ERROR;
    }

    /* Initialize basic fields */
    devId->defaultGc              = UGL_NULL;
    devId->pScratchBuf            = UGL_NULL;
    devId->scratchBufFree         = UGL_TRUE;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglUgiDevDeinit - Deinitialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevDeinit (
    UGL_DEVICE_ID  devId
    ) {

    if (uglOSLockDestroy (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGraphicsDevDestroy - Free graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglGraphicsDevDestroy (
    UGL_DEVICE_ID  devId
    ) {
    UGL_STATUS  status;

    if (devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Free scratch buffer */
    if (devId->pScratchBuf != UGL_NULL) {
        UGL_FREE (devId->pScratchBuf);
    }

    /* Destroy default graphics context */
    if (devId->defaultGc != UGL_NULL) {
        uglGcDestroy (devId->defaultGc);
    }

    /* Call driver specific device destroy method */
    status = (*devId->destroy) (devId);

    return (status);
}

/******************************************************************************
 *
 * uglScratchBufferAlloc - Allocate scratch memory
 *
 * RETURNS: Pointer to memory or UGL_NULL
 */

void * uglScratchBufferAlloc (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       memSize
    ) {
    UGL_SIZE  reqMemSize;
    void *    pMem;

    /* Check if buffer is not already in use */
    if (devId->scratchBufFree != UGL_TRUE) {
        return (UGL_NULL);
    }

    /* Calucate size of requested memory in blocks */
    reqMemSize = (memSize + UGL_SCRATCH_BUF_BLK_SIZE - 1) /
                 UGL_SCRATCH_BUF_BLK_SIZE * UGL_SCRATCH_BUF_BLK_SIZE;

    /* Reallocate */
    pMem = UGL_REALLOC (devId->pScratchBuf, reqMemSize);
    if (pMem == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Store new buffer */
    devId->pScratchBuf = pMem;

    return (pMem);
}

/******************************************************************************
 *
 * uglScratchBufferFree - Release scratch memory
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglScratchBufferFree (
    UGL_DEVICE_ID  devId,
    void *         pMem
    ) {

    /* Check buffer */
    if (pMem != devId->pScratchBuf) {
        return (UGL_STATUS_ERROR);
    }

    /* Mark as free */
    devId->scratchBufFree = UGL_TRUE;

    return (UGL_STATUS_OK);
}

