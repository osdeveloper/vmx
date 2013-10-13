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
******************************************************************************/

/* kernHookLib.h - Hooks called when kenrel does task switch */

#ifndef _kernHookLib_h
#define _kernHookLib_h

#define MAX_KERNEL_CREATE_HOOKS	16
#define MAX_KERNEL_SWITCH_HOOKS	16
#define MAX_KERNEL_DELETE_HOOKS	16
#define MAX_KERNEL_SWAP_HOOKS	16

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx/taskLib.h>

extern STATUS kernHookLibInit(void);
extern STATUS kernCreateHookAdd(FUNCPTR hook);
extern STATUS kernCreateHookDelete(FUNCPTR hook);
extern STATUS kernSwitchHookAdd(FUNCPTR hook);
extern STATUS kernSwitchHookDelete(FUNCPTR hook);
extern STATUS kernDeleteHookAdd(FUNCPTR hook);
extern STATUS kernDeleteHookDelete(FUNCPTR hook);
extern STATUS kernSwapHookAdd(FUNCPTR hook);
extern STATUS kernSwapHookAttach(FUNCPTR hook,
				 TCB_ID pTcb,
				 BOOL swapIn,
				 BOOL swapOut);
extern STATUS kernSwapHookDetach(FUNCPTR hook,
				 TCB_ID pTcb,
				 BOOL swapIn,
				 BOOL swapOut);
extern STATUS kernSwapHookDelete(FUNCPTR hook);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _kernHookLib_h */

