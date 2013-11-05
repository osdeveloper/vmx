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

/* pipeDrv.h - Pipe device header */

#ifndef _pipeDrv_h
#define _pipeDrv_h

#include <vmx.h>
#include <vmx/msgQLib.h>
#include <os/iosLib.h>
#include <os/selectLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct
{
    DEV_HEADER      devHeader;
    MSG_Q           msgQ;
    SEL_WAKEUP_LIST selWakeupList;
    unsigned int    numOpens;
} PIPE_DEV;

/******************************************************************************
 * pipeDrvInit- Install pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDrvInit(
    void
    );

/*******************************************************************************
 * pipeDevCreate - Create pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDevCreate(
    char *name,
    int maxMsg,
    int maxBytes
    );

/******************************************************************************
 * pipeDevDelete - Delete pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDevDelete(
    char *name,
    BOOL force
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _pipeDrv_h */

