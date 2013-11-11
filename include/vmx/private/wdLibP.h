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

/* wdLibP.h - Whatchdog private header */

#ifndef _wdLibP_h
#define _wdLibP_h

#include <vmx.h>
#include <util/qLib.h>
#include <os/classLib.h>

#define WDOG_OUT_OF_Q           0x00            /* Watchdog not in tick queue */
#define WDOG_IN_Q               0x01            /* Watchdog in tick queue */
#define WDOG_DEAD               0x02            /* Watchdog terminated */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wdog
{
    OBJ_CORE       objCore;             /* Object class */
    Q_NODE         tickNode;            /* Tick queue node */
    unsigned short status;              /* Status */
    unsigned short dfrStartCount;       /* Number of wdog starts */
    FUNCPTR        wdFunc;              /* Function to call */
    ARG            wdArg;               /* Argument to function */
} WDOG;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _wdLibP_h */

