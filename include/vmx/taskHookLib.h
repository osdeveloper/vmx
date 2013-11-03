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

/* taskHookLib.h - Hooks called when kenrel does task switch */

#ifndef _taskHookLib_h
#define _taskHookLib_h

#define MAX_TASK_CREATE_HOOKS   16
#define MAX_TASK_SWITCH_HOOKS   16
#define MAX_TASK_DELETE_HOOKS   16
#define MAX_TASK_SWAP_HOOKS     16

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * taskHookLibInit - Initialize task switch hook library
 *
 * RETURNS: OK 
 */

STATUS taskHookLibInit(
    void
    );

/******************************************************************************
 * taskCreateHookAdd - Add a task create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskCreateHookAdd(
    FUNCPTR hook
    );

/******************************************************************************
 * taskCreateHookDelete - Delete a task create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskCreateHookDelete(
    FUNCPTR hook
    );

/******************************************************************************
 * taskSwitchHookAdd - Add a task switch hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwitchHookAdd(
    FUNCPTR hook
    );

/******************************************************************************
 * taskSwitchHookDelete - Delete a task switch hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwitchHookDelete(
    FUNCPTR hook
    );

/******************************************************************************
 * taskDeleteHookAdd - Add a task delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteHookAdd(
    FUNCPTR hook
    );

/******************************************************************************
 * taskDeleteHookDelete - Delete a task switch delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteHookDelete(
    FUNCPTR hook
    );

/******************************************************************************
 * taskSwapHookAdd - Add a task swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookAdd(
    FUNCPTR hook
    );

/******************************************************************************
 * taskSwapHookAttach - Attach a task to swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookAttach(
    FUNCPTR hook,
    int taskId,
    BOOL swapIn,
    BOOL swapOut
    );

/******************************************************************************
 * taskSwapHookDetach - Detach a task to swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookDetach(
    FUNCPTR hook,
    int taskId,
    BOOL swapIn,
    BOOL swapOut
    );

/******************************************************************************
 * taskSwapHookDelete - Delete a task swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookDelete(
    FUNCPTR hook
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskHookLib_h */

