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

#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglOSLockCreate - Create a locking mechanism
 *
 * RETURNS: Lock identifier
 */

UGL_LOCK_ID uglOSLockCreate (
    void
    ) {

    return semMCreate (SEM_Q_PRIORITY);
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

    if (semDelete (lockId) == ERROR) {
        return (UGL_STATUS_ERROR);
    }

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

    if (semTake(lockId, WAIT_FOREVER) == ERROR) {
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglOSUnLock - Unlock
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSUnLock (
    UGL_LOCK_ID  lockId
    ) {

    if (semGive(lockId) == ERROR) {
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

