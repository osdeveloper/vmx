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

/* fsMonitor.c - file systems monitor library */

/* includes */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/ioLib.h>
#include <os/erfLib.h>
#include <fs/xbd.h>
#include <fs/fsMonitor.h>

/* defines */

/* structs */

struct fsm_probe;
struct fsm_mapping;

/* typedefs */

typedef struct fsm_probe {
    struct fsm_probe *  next;
    int    (*probe)(device_t);        /* function pointer to probe F/S type */
    STATUS (*inst)(device_t, char *); /* function to create F/S type */
} FSM_PROBE;

typedef struct fsm_mapping {
    struct fsm_mapping *  next;
    char *  xbdName;              /* XBD device name (NUL-terminated) */
    char *  path;                 /* path name (NUL-terminated) */
} FSM_MAPPING;

/* globals */

/* locals */

LOCAL BOOL           fsMonitorInstalled = FALSE;
LOCAL FSM_PROBE *    fsmProbes;
LOCAL FSM_MAPPING *  fsmMappings;
LOCAL SEM_ID         fsmMutex;

LOCAL void fsmHandler (int category, int type,
                       void *pEventData, void *pUserData);

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
    ) {
    if (fsMonitorInstalled) {
        return (OK);
    }
    if ((erfHandlerRegister (xbdEventCategory, xbdEventSecondaryInsert,
                             fsmHandler, NULL, 0) != OK) ||
        (erfHandlerRegister (xbdEventCategory, xbdEventSoftInsert,
                             fsmHandler, NULL, 0) != OK)) {
        /* errno set by ERF routines */
        return (ERROR);
    }

    fsmMutex = semMCreate (SEM_Q_PRIORITY);
    if (fsmMutex == NULL) {
        /* errno set by semMCreate() */
        return (ERROR);
    }

    fsmProbes   = NULL;
    fsmMappings = NULL;

    fsMonitorInstalled = TRUE;

    return (OK);
}

/***************************************************************************
 *
 * fsmNameMap - map an XBD device name to an IO path
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS  fsmNameMap (
    char *  xbdName,
    char *  ioPath
    ) {
    FSM_MAPPING *  pMapping;
    int  xbdNameLen;
    int  ioPathLen;

    if ((xbdName == NULL) || (ioPath == NULL)) {
        errnoSet (S_fsMonitor_INVALID_PARAMETER);
        return (ERROR);
    }

    xbdNameLen = strlen (xbdName) + 1;
    ioPathLen  = strlen (ioPath) + 1;

    pMapping = (FSM_MAPPING *)
               malloc (sizeof (FSM_MAPPING) + xbdNameLen + ioPathLen);
    if (pMapping == NULL) {
        /* errno set by malloc() */
        return (ERROR);
    }

    pMapping->xbdName = (char *) &pMapping[1];
    pMapping->path    = pMapping->xbdName + xbdNameLen;

    /* Copy name and path */
    strncpy (pMapping->xbdName, xbdName, xbdNameLen);
    strncpy (pMapping->path, ioPath, ioPathLen);

    if (semTake (fsmMutex, WAIT_FOREVER) != OK) {
        /* errno set by semTake() */
        return (ERROR);
    }

    pMapping->next = fsmMappings;
    fsmMappings = pMapping;

    return (semGive (fsmMutex));
}

/***************************************************************************
 *
 * fsmNameUninstall - remove an XBD device name to IO path mapping
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmNameUninstall (
    char *  xbdName
    ) {
    FSM_MAPPING *  prev;
    FSM_MAPPING *  pMapping;

    if (xbdName == NULL) {
        errnoSet (S_fsMonitor_INVALID_PARAMETER);
        return (ERROR);
    }

    if (semTake (fsmMutex, WAIT_FOREVER) != OK) {
        /* errno set by semTake() */
        return (ERROR);
    }

    prev = NULL;
    for (pMapping = fsmMappings; pMapping != NULL; pMapping = pMapping->next) {
        if (strcmp (xbdName, pMapping->xbdName) == 0) {
            if (prev == NULL) {
                fsmMappings = fsmMappings->next;
            } else {
                prev->next = pMapping->next;
            }
            semGive (fsmMutex);
            return (OK);
        }
        prev = pMapping;
    }

    semGive (fsmMutex);
    errnoSet (S_fsMonitor_ITEM_NOT_FOUND);
    return (ERROR);
}

/***************************************************************************
 *
 * fsmProbeInstall - install file system probe function
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fsmProbeInstall (
    int     (*probe)(device_t),       /* function pointer to probe F/S type */
    STATUS  (*inst)(device_t, char *) /* function to create F/S type */
    ) {
    FSM_PROBE *  pProbe;
    int          level;

    if ((probe == NULL) || (inst == NULL)) {
        errnoSet (S_fsMonitor_INVALID_PARAMETER);
        return (ERROR);
    }

    pProbe = (FSM_PROBE *) malloc (sizeof (FSM_PROBE));
    if (pProbe == NULL) {
        /* errno set by malloc() */
        return (ERROR);
    }

    /* New probe and instantiator routines are added to the head of the list. */

    pProbe->probe = probe;
    pProbe->inst  = inst;

    semTake (fsmMutex, WAIT_FOREVER);
    pProbe->next = fsmProbes;
    fsmProbes = pProbe;
    semGive (fsmMutex);

    return (OK);
}

/***************************************************************************
 *
 * fsmProbeUninstall - uninstall file system probe function
 *
 * RETURNS OK on success, ERROR otherwise
 */

STATUS fsmProbeUninstall (
    int     (*probe)(device_t)        /* function pointer to probe F/S type */
    ) {
    FSM_PROBE *  prev;
    FSM_PROBE *  pProbe;
    int          level;

    if (semTake (fsmMutex, WAIT_FOREVER) != OK) {
        /* errno set by semTake() */
        return (ERROR);
    } 

    prev  = NULL;
    for (pProbe = fsmProbes; pProbe != NULL; pProbe = pProbe->next) {
        if (pProbe->probe == probe) {
           if (prev != NULL) {
               prev->next = pProbe->next;
           } else {
               fsmProbes = fsmProbes->next;
           }
           break;
        }
        prev = pProbe;
    }

    semGive (fsmMutex);

    return (ERROR);
}

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
    ) {
    FSM_MAPPING *  pMapping;
    char *ioTail;
    int  len;

    if ((ioPath == NULL) || (driver == NULL) || (drvLen <= 0)) {
        errnoSet (S_fsMonitor_INVALID_PARAMETER);
        return (ERROR);
    }

    /*
     * Hmmm, should this step be necessary?  That is, should we impose a
     * restriction that the caller should have already identified the device
     * portion of the IO path and only pass that portion in?  If so, this
     * module can be simplified.
     */

    if (iosDevFind ((char *) ioPath, &ioTail) == NULL) {
        /* errno set by iosDevFind() */
        return (ERROR);
    }

    /* IO device name has been extracted from <ioPath> */
    len = (int) ioTail - (int) ioPath;

    if (semTake (fsmMutex, WAIT_FOREVER) != OK) {
        /* errno set by semTake() */
        return (ERROR);
    }

    for (pMapping = fsmMappings;
         pMapping != NULL;
         pMapping = pMapping->next) {
        if ((strncmp (pMapping->path, ioPath, len) == 0) &&
            (pMapping->path[len] == '\0')) {
            strncpy (driver, pMapping->xbdName, drvLen);
            semGive (fsmMutex);
            return (OK);
        }
    }

    semGive (fsmMutex);

    strncpy (driver, ioPath, len + 1);
    return (OK);
}

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
    ) {
    FSM_MAPPING *  pMapping;

    if ((xbdName == NULL) || (volume == NULL) || (volLen <= 0)) {
        errnoSet (S_fsMonitor_INVALID_PARAMETER);
        return (ERROR);
    }

    if (semTake (fsmMutex, WAIT_FOREVER) != OK) {
        /* errno set by semTake() */
        return (ERROR);
    }

    for (pMapping = fsmMappings;
         pMapping != NULL;
         pMapping = pMapping->next) {
        if (strcmp (pMapping->xbdName, xbdName) == 0) {
            strncpy (volume, pMapping->path, volLen);
            semGive (fsmMutex);
            return (OK);
        }
    }

    /*
     * ???
     * If the XBD device name was not mapped to a path, then the path shall
     * be assumed to be the same as the XBD device name.  It is thought that
     * this should be acceptable behaviour.
     * ???
     */

    strncpy (volume, xbdName, volLen);

    semGive (fsmMutex);
    return (OK);
}

/***************************************************************************
 *
 * fsmHandler -
 *
 * RETURNS: N/A
 */

LOCAL void fsmHandler (
    int     category,      /* event category */
    int     type,          /* event type */
    void *  pEventData,    /* device ID */
    void *  pUserData      /* not used--path extracted from XBD & mapping */
    ) {
    device_t     device;
    devname_t    devname;
    char         path[PATH_MAX];
    FSM_PROBE *  pProbe;

    if (category != xbdEventCategory) {
        return;
    }

    if (type == xbdEventSecondaryInsert) {
        /*
         * If one is comparing the use of the ERF in this module to its use
         * in the ERF, the path information is extracted differently.  The
         * event data only has room for the device ID.  The user data in this
         * module must be NULL, as there is no way of knowing in advance what
         * the path information is supposed to be.
         */

        device   = (device_t) pEventData;
        if (xbdDevName (device, devname) != OK) {
                return;
        }

        if (fsmGetVolume (devname, path, PATH_MAX) != OK) {
            /* errno set by fsmVolumeGet() */
            return;
        }

        semTake (fsmMutex, WAIT_FOREVER);

        /* Run the file system probes */
        for (pProbe = fsmProbes; pProbe != NULL; pProbe = pProbe->next) {
            if ((pProbe->probe (device) == OK) &&
                (pProbe->inst (device, path) == OK)) {
                break;
            }
        }

        /*
         * RawFS MUST be in the system and it must be the first file system
         * type registered with the file system monitor.  Since new file
         * system types are inserted at the head of the list, RawFS will always
         * be at the tail and it is to succeed in mounting, thus acting as
         * a sort of sentinel.
         */

        semGive (fsmMutex);

    } else if (type == xbdEventSoftInsert) {
        /*
         * A soft insertion event means that the usual action of probing and
         * instantiation and stuff is to be skipped.  Instead the device is to
         * be mounted with RawFS.  This means that this module will have a
         * hard dependency on RawFS.
         */

        device   = (device_t) pEventData;
        if (xbdDevName (device, devname) != OK) {
            return;
        }

        if (fsmGetVolume (devname, path, PATH_MAX) != OK) {
            /* errno set by fsmVolumeGet() */
            return;
        }

        semTake (fsmMutex, WAIT_FOREVER);

        /* rawFS must be the first filesystem registered */
        for (pProbe = fsmProbes; pProbe != NULL; pProbe = pProbe->next) {
            if (pProbe->next == NULL) {
                pProbe->inst (device, path);
                break;
            }
        }

        semGive (fsmMutex);

    }

    return;
}

