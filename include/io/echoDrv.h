/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2008 Surplus Users Ham Society
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

/* echoDrv.h - I/O Echo driver header */

#ifndef _echoDrv_h
#define _echoDrv_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>
#include <io/ttyLib.h>

typedef struct
{
    TTY_DEV ttyDev;
} ECHO_DEV;

/******************************************************************************
 * echoDrvInit - Initialize echo device
 *
 * RETURNS: OK or ERROR
 */

STATUS echoDrvInit(
    void
    );

/******************************************************************************
 * echoDevCreate - Create echo device
 *
 * RETURNS: OK or ERROR
 */

STATUS echoDevCreate(
    char *name,
    int readBufferSize,
    int writeBufferSize
    );

/******************************************************************************
 * echoDrvNumber - Get echo device number
 *
 * RETURNS: Driver number
 */

int echoDrvNumber(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _echoDrv_h */

