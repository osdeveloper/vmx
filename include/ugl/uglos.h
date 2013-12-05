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

/* uglos.h - Universal graphics library os calls */

#ifndef _uglos_h
#define _uglos_h

#include <ugl/os/uglvmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Locks */

/******************************************************************************
 *
 * uglOSLockCreate - Create a locking mechanism
 * 
 * RETURNS: Lock identifier
 */

UGL_LOCK_ID uglOsLockCreate (
    void
    );

/******************************************************************************
 *
 * uglOSLockDestroy - Free a locking mechanism
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOsLockDestroy (
    UGL_LOCK_ID  lockId
    );

/******************************************************************************
 *
 * uglOSLock - Lock
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOsLock (
    UGL_LOCK_ID  lockId
    );

/******************************************************************************
 *
 * uglOSUnLock - Unlock
 * 
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOsUnLock (
    UGL_LOCK_ID  lockId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglos_h */

