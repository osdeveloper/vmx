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

/* shellLib.h - Shell library header */

#ifndef _shellLib_h
#define _shellLib_h

#include <vmx.h>
#include <ostool/private/shellLibP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * shellLibInit - Initialize shell library
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLibInit(
    int stackSize,
    ARG arg
    );

/******************************************************************************
 * shellSpawn - Respawn shell task
 *
 * RETURNS: OK or ERROR
 */

STATUS shellSpawn(
    int stackSize,
    ARG arg
    );

/******************************************************************************
 * shellLoginInstall - Install shell login function
 *
 * RETURNS: N/A
 */

void shellLoginInstall(
    FUNCPTR func,
    int value
    );

/******************************************************************************
 * shellLogin - Login using user login function
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLogin(
    int fd
    );

/******************************************************************************
 * shellLogoutInstall - Install shell logout function
 *
 * RETURNS: N/A
 */

void shellLogoutInstall(
    FUNCPTR func,
    int value
    );

/******************************************************************************
 * shellLogout - Logout using user logout function
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLogout(
    void
    );

/******************************************************************************
 * shellLock - Lock/unlock shell
 *
 * RETURNS: TRUE if mode changed else FALSE
 */

BOOL shellLock(
    BOOL lock
    );

/******************************************************************************
 * shell - Shell task
 *
 * RETURNS: N/A
 */

void shell(
    BOOL interactive
    );

/******************************************************************************
 * execute - Execute shell line
 *
 * RETURNS: N/A
 */

STATUS execute(
    char *line
    );

/* Functions */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _shellLib_h */

