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

/* pcConsole.h - PC console header */

#ifndef _pcConsole_h
#define _pcConsole_h

#include <io/tyLib.h>
#include <drv/input/i8042Kbd.h>
#include <drv/video/m6845Vga.h>

#define INSERT_MODE_OFF         0
#define INSERT_MODE_ON          1
#define FORWARD                 1
#define BACKWARD                0

#define ESC_NORMAL              0x0001

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pcConDev
{
    TY_DEV       tyDev;
    BOOL         created;
    KBD_CON_DEV *ks;
    VGA_CON_DEV *vs;
} PC_CON_DEV;

/******************************************************************************
 * pcConDrvInit - Initialize console
 *
 * RETURNS: OK or ERROR
 */

STATUS pcConDrvInit(
    void
    );

/******************************************************************************
 * pcConDevCreate - Create device
 *
 * RETURNS: OK or ERROR
 */

STATUS pcConDevCreate(
    char *name,
    int channel,
    int readBufferSize,
    int writeBufferSize
    );

/******************************************************************************
 * pcConDrvNumber - Get console number
 *
 * RETURNS: Console number
 */

int pcConDrvNumber(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _pcConsole_h */

