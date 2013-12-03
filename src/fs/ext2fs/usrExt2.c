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

/* usrExt2.c - user interface for ext2 filesystem */

#include <stdlib.h>
#include <vmx.h>
#include <fs/fsMonitor.h>
#include <fs/ext2fsLib.h>

/******************************************************************************
 *
 * usrExt2fsInit - Initialize ext2 filesystem
 *
 * RETURNS: OK on success, otherwise ERROR
 */

STATUS usrExt2fsInit (
    int  maxBufs,
    int  maxFiles,
    int  reserved1,
    int  reserved2 
    ) {
    int  error;

    /* initialize library */
    ext2fsLibInit (maxBufs, maxFiles, reserved1, reserved2);

    /* install filesystem probe */
    error = fsmProbeInstall (ext2fsDiskProbe, ext2fsDevCreate2);
    if (error != OK) {
        errnoSet (error);
        return (ERROR);
    }

    return (OK);
}

