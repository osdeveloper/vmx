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

/* fsEventUtilLib.h - file system event utility library */

#ifndef __FS_EVENT_UTIL_LIB_H
#define __FS_EVENT_UTIL_LIB_H

#include <vmx.h>
#include <vmx/semLib.h>
#include <ostool/moduleNumber.h>

#define S_fsEventUtil_INVALID_PARAMETER (M_fsEventUtil | 0x0001)

typedef struct {
    SEM_ID  semId;
    char *  path;
} FS_PATH_WAIT_STRUCT;

/* functions */

/***************************************************************************
 *
 * fsEventUtilInit - initialize the file system event utility library
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsEventUtilInit (
    void
    );

/***************************************************************************
 *
 * fsPathAddedEventSetup - prepare to wait for a path
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsPathAddedEventSetup (
    FS_PATH_WAIT_STRUCT *  pWaitData,
    char *                 path
    );

/***************************************************************************
 *
 * fsPathAddedEventRaise - raise a path added event
 *
 * RETURNS: N/A
 */

STATUS fsPathAddedEventRaise (
    char *  path
    );

/***************************************************************************
 *
 * fsWaitForPath - wait for the path added event
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS  fsWaitForPath (
    FS_PATH_WAIT_STRUCT *  pWaitData
    );

#endif

