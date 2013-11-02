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

/* kernelLibP.h - Private header for kernel library */

#ifndef _kernelLibP_h
#define _kernelLibP_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>
#include <vmx/taskLib.h>

IMPORT BOOL      kernelInitialized;
IMPORT BOOL      kernelState;
IMPORT BOOL      roundRobinOn;
IMPORT unsigned  roundRobinSlice;

IMPORT char     *pRootMemStart;
IMPORT unsigned  rootMemNBytes;
IMPORT int       rootTaskId;

IMPORT char     *pExcStackBase;
IMPORT char     *pExcStackEnd;

IMPORT TCB_ID    taskIdCurrent;
IMPORT Q_HEAD    activeQHead;
IMPORT Q_HEAD    tickQHead;
IMPORT Q_HEAD    readyQHead;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _kernelLibP_h */

