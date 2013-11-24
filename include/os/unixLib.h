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

/* unixLib.h - Unix routines library header */

#ifndef _unixLib_h
#define _unixLib_h

#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * unixLibInit - Inititalize unix library
 *
 * RETURNS: OK or ERROR
 */

STATUS unixLibInit(
    void
    );

/******************************************************************************
 * splnet - Get network processor level
 *
 * RETURNS: TRUE or FALSE
 */

int splnet(
    void
    );

/******************************************************************************
 * splimp - Set imp processor level
 *
 * RETURNS: TRUE or FALSE
 */

int splimp(
    void
    );

/******************************************************************************
 * splx - Set processor level
 *
 * RETURNS: N/A
 */

void splx(
    int x
    );

/******************************************************************************
 * ksleep - Got to sleep
 *
 * RETURNS: N/A
 */

void ksleep(
    SEM_ID semId
    );

/******************************************************************************
 * wakeup - Wakeup
 *
 * RETURNS: N/A
 */

void wakeup(
    SEM_ID semId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _unixLib_h */

