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

/* usrFsLib.h - User filesystem tools header */

#ifndef _usrFsLib_h
#define _usrFsLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <vmx.h>

/******************************************************************************
 * ioHelp - Print io help
 *
 * RETURNS: N/A
 */

void ioHelp(
    void
    );

/******************************************************************************
 * cd - Change working directory
 *
 * RETURNS: OK or ERROR
 */

STATUS cd(
    const char *path
    );

/******************************************************************************
 * pwd - Printf working directory
 *
 * RETURNS: N/A
 */

void pwd(
    void
    );

/******************************************************************************
 * dirList - List directory
 *
 * RETURNS: OK or ERROR
 */

STATUS dirList(
    FILE *fp,
    char *dirname,
    BOOL  doLong,
    BOOL  doTree
    );

/******************************************************************************
 * ls - List directory
 *
 * RETURNS: OK or ERROR
 */

STATUS ls(
    char *dirname,
    BOOL  doLong
    );

/******************************************************************************
 * ll - List directory (long version)
 *
 * RETURNS: OK or ERROR
 */

STATUS ll(
    char *dirname
    );

/******************************************************************************
 * lsr - List directory and subdirectories
 *
 * RETURNS: OK or ERROR
 */

STATUS lsr(
    char *dirname
    );

/******************************************************************************
 * llr - List directory and subdirectories (long version)
 *
 * RETURNS: OK or ERROR
 */

STATUS llr(
    char *dirname
    );

/******************************************************************************
 * copy - Copy a file to another file
 *
 * RETURNS: OK or ERROR
 */

STATUS copy(
    const char *inFilename,
    const char *outFilename
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _usrFsLib_h */

