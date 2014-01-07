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

/* udvmxlck.c - Universal graphics library lock support */

#include "ugl.h"

/******************************************************************************
 *
 * uglOSLockCreate - Create a locking mechanism
 *
 * RETURNS: Lock identifier
 */

UGL_LOCK_ID uglOSLockCreate (
    void
    ) {

    /* TODO */
    return (void *) 1;
}

/******************************************************************************
 *
 * uglOSLockDestroy - Free a locking mechanism
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSLockDestroy (
    UGL_LOCK_ID  lockId
    ) {

    /* TODO */

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglOSLock - Lock
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSLock (
    UGL_LOCK_ID  lockId
    ) {

    /* TODO */

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglOSUnlock - Unlock
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSUnlock (
    UGL_LOCK_ID  lockId
    ) {

    /* TODO */

    return (UGL_STATUS_OK);
}

