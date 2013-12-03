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

/* xbdBlkDev.h - BLK_DEV to XBD interface converter */

#ifndef _xbdBlkDev_h
#define _xbdBlkDev_h

#ifndef _ASMLANGUAGE

#include <fs/xbd.h>
#include <fs/blkIo.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/***************************************************************************
 *
 * xbdBlkDevCreate - create a XBD BLK interface wrapper device
 *
 * RETURNS: XBD's device ID on success, NULLDEV on error
 */

device_t xbdBlkDevCreate (
    BLK_DEV    *  bd,          /* block device */
    const char *  name         /* device name */
    );

/***************************************************************************
 *
 * xbdBlkDevDelete - delete a XBD BLK interface wrapper device
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdBlkDevDelete (
    device_t    device,
    BLK_DEV **  ppbd
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _xbdBlkDev_h */

