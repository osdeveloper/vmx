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

/* udvmxtsk.c - Universal graphics library task support */

#include <unistd.h>

#include "ugl.h"

/******************************************************************************
 *
 * uglOSTaskDelay - Put task to sleep
 *
 * RETURNS: N/A
 */

UGL_VOID uglOSTaskDelay (
    UGL_UINT32  msecs
    ) {

    /* Sleep */
    usleep (msecs * 1000);
}

/******************************************************************************
 *
 * uglOSTaskLock - Lock task
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSTaskLock (
    void
    ) {

    /* TODO */

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglOSTaskUnlock - Unlock task
 *
 * RETURNS: UGL_STATUS_OR or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSTaskUnlock (
    void
    ) {

    /* TODO */

    return (UGL_STATUS_OK);
}

