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

/* taskHookShow.h - Task hook show header */

#ifndef _taskHookShow_h
#define _taskHookShow_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * taskHookShowInit - Initialize show hooks module
 *
 * RETURNS: N/A
 */

void taskHookShowInit(
    void
    );

/******************************************************************************
 * taskCreateHookShow - Show task create hooks
 *
 * RETURNS: N/A
 */

void taskCreateHookShow(
    void
    );

/******************************************************************************
 * taskSwitchHookShow - Show task switch hooks
 *
 * RETURNS: N/A
 */

void taskSwitchHookShow(
    void
    );

/******************************************************************************
 * taskDeleteHookShow - Show task delete hooks
 *
 * RETURNS: N/A
 */

void taskDeleteHookShow(
    void
    );

/******************************************************************************
 * taskSwapHookShow - Show task swap hooks
 *
 * RETURNS: N/A
 */

void taskSwapHookShow(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskHookShow_h */

