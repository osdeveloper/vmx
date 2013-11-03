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

/* pathLib.h - Path name functions */

#ifndef _pathLib_h
#define _pathLib_h

#include <vmx.h>
#include <sys/types.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * pathLibInit - initialize the path library
 *
 * RETURNS: OK
 */

STATUS pathLibInit(
    void
    );

/******************************************************************************
 * ioDefPathSet - Set current working directory
 *
 * RETURNS: OK or ERROR
 */

STATUS ioDefPathSet(
    char *path
    );

/******************************************************************************
 * ioDefPathGet - Get current working directory
 *
 * RETURNS: N/A
 */

void ioDefPathGet(
    char *path
    );

/******************************************************************************
 * ioDefPathCat - Conatenate path to current working directory
 *
 * RETURNS: OK or ERROR
 */

STATUS ioDefPathCat(
    char *path
    );

/******************************************************************************
 * pathCwdLen - Get length of working directory
 *
 * RETURNS: Length of current working directory path
 */

int pathCwdLen(
    void
    );

/******************************************************************************
 * pathPrependCwd - Prepend current working directory for filename
 *
 * RETURNS: OK or ERROR
 */

STATUS pathPrependCwd(
    char *filename,
    char *fullPath
    );

/******************************************************************************
 * pathSplit - Split path into directory and filename
 *
 * RETURNS: N/A
 */

void pathSplit(
    char *path,
    char *dirname,
    char *filename
    );

/******************************************************************************
 * pathCondense - Process path
 *
 * RETURNS: N/A
 */

void pathCondense(
    char *path
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _pathLib_h */

