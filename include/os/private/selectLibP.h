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

/* selectLibP.h - Private select library header */

#ifndef _selectLibP_h
#define _selectLibP_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <util/listLib.h>
#include <vmx/semLib.h>
#include <os/ioLib.h>

typedef struct selWakeupNode
{
    LIST_NODE   listHooks;
    BOOL        dontFree;
    int         taskId;
    int         fd;
    SELECT_TYPE type;
} SEL_WAKEUP_NODE;

typedef struct
{
    SEMAPHORE       mutex;
    SEL_WAKEUP_NODE first;
    LIST            wakeupList;
} SEL_WAKEUP_LIST;

typedef struct selContext
{
    SEMAPHORE  wakeupSync;
    BOOL       pendedOnSelect;
    fd_set    *pReadFds;
    fd_set    *pWriteFds;
    fd_set    *pOrigReadFds;
    fd_set    *pOrigWriteFds;
    int        width;
} SEL_CONTEXT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _selectLibP_h */

