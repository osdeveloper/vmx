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

/* fsMonitor.h - Filesystem monitor header */

#ifndef _fsMonitor_h
#define _fsMonitor_h

#include <ostool/moduleNumber.h>

#define S_fsMonitor_INVALID_PARAMETER           (M_fsMonitor | 0x0001)
#define S_fsMonitor_ITEM_NOT_FOUND              (M_fsMonitor | 0x0002)
#define S_fsMonitor_DRV_NOT_FOUND               (M_fsMonitor | 0x0002)

#include <vmx.h>
#include <fs/xbd.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* functions */

/***************************************************************************
 *
 * fsMonitorInit - initialize the FS monitor
 *
 * CAVEAT: This routine, if called, must be done AFTER xbdLibInit().
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsMonitorInit (
    void
    );

/***************************************************************************
 *
 * fsmNameMap - map an XBD device name to an IO path
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS  fsmNameMap (
    char *  xbdName,
    char *  ioPath
    );

extern STATUS fsmNameUninstall(char *xbdName);
extern STATUS fsmProbeInstall(int (*probe)(device_t),
                              STATUS (*inst)(device_t, char *));
/***************************************************************************
 *
 * fsmNameMap - map an XBD device name to an IO path
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS  fsmNameMap (
    char *  xbdName,
    char *  ioPath
    );

/***************************************************************************
 *
 * fsmNameUninstall - remove an XBD device name to IO path mapping
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmNameUninstall (
    char *  xbdName
    );

/***************************************************************************
 *
 * fsmProbeInstall - install file system probe function
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmProbeInstall (
    int     (*probe)(device_t),       /* function pointer to probe F/S type */
    STATUS  (*inst)(device_t, char *) /* function to create F/S type */
    );

/***************************************************************************
 *
 * fsmProbeUninstall - uninstall file system probe function
 *
 * RETURNS OK on success, ERROR otherwise
 */

STATUS fsmProbeUninstall (
    int     (*probe)(device_t)        /* function pointer to probe F/S type */
    );

/***************************************************************************
 *
 * fsmGetDriver - get the XBD device name given the IO path
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmGetDriver (
    const char *  ioPath,
    char *        driver,
    int           drvLen
    );

/***************************************************************************
 *
 * fsmGetVolume - get the pathname given the XBD device name
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmGetVolume (
    const char *  xbdName,
    char *        volume,
    int           volLen
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _fsMonitor_h */

