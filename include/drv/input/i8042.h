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

/* i8042Kbd.h - Intel i8042 keyboard and mouse driver header */

#ifndef _i8042_h
#define _i8042_h

#include <sys/types.h>
#include <os/tyLib.h>

#define I8042_KBD_OBFULL  0x01
#define I8042_KBD_IBFULL  0x02
#define I8042_KBD_AUXB    0x20

#define I8042_WAIT_SEC  2

#define KBD_NORMAL      0x0000
#define KBD_STP         0x0001
#define KBD_NUM         0x0002
#define KBD_CAPS        0x0004
#define KBD_SHIFT       0x0008
#define KBD_CTRL        0x0010
#define KBD_EXT         0x0020
#define KBD_ESC         0x0040
#define KBD_EW          (KBD_EXT | KBD_ESC)
#define KBD_E1          0x0080
#define KBD_PRTSC       0x0100
#define KBD_BRK         0x0200

#define KBD_ON          0xff
#define KBD_OFF         0x00

#define KBD_EXT_SIZE    16
#define KBD_E0_BASE     0x80

#define KBA_NOR         0
#define KBA_SHI         1
#define KBA_CTL         2
#define KBA_NUM         3
#define KBA_CAP         4
#define KBA_STP         5
#define KBA_EXT         6
#define KBA_ESC         7

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Keyboard device */
typedef struct
{
    BOOL      cursorMode;
    u_int16_t flags;
    u_int16_t state;
    int       currConsole;
    FUNCPTR   consoleHook;
    BOOL      convertChar;
} KBD_CON_DEV;

/* Mouse device */
typedef struct
{
    TY_DEV    tyDev;
    u_int32_t dataReg;
    u_int32_t cmdReg;
} I8042_MSE_DEVICE;

/******************************************************************************
 * kbdHrdInit - Initialize keyboard hardware
 *
 * RETURNS: N/A
 */

void kbdHrdInit(
    void
    );

/******************************************************************************
 * kbdIntr - Keyboard interrupt callback
 *
 * RETURNS: N/A
 */

void kbdIntr(
    void
    );

/******************************************************************************
 * i8042MseDevCreate - Create mouse driver
 *
 * RETURNS: N/A
 */

int i8042MseDevCreate(
    char *name
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _i8042_h */

