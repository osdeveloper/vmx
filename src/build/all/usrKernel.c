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

/* usrKernel.c - User kernel initialize */

/* Includes */
#include <vmx.h>
#include <util/qLib.h>
#include <util/qFifoLib.h>
#include <util/qPrioLib.h>
#include <util/qPriBmpLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskShow.h>
#include <vmx/semLib.h>
#include <vmx/semShow.h>
#include <vmx/msgQLib.h>
#include <vmx/msgQShow.h>
#include <vmx/workQLib.h>
#include <vmx/tickLib.h>
#include <os/classLib.h>
#include <os/classShow.h>
#include <os/taskHookLib.h>
#include <os/taskHookShow.h>

#include "config.h"

/* Defines */

/* Imports */
IMPORT Q_HEAD activeQHead;
IMPORT Q_HEAD tickQHead;
IMPORT Q_HEAD readyQHead;

/* Locals */

/* Globals */
#ifdef INCLUDE_CONSTANT_RDY_Q
DL_LIST kernReadyLst[256];
unsigned kernReadyBmp[8];
#endif /* INCLUDE_CONSTANT_RDY_Q */

/* Functions */

/******************************************************************************
 * userKernelInit - Initialize user kernel
 *
 * RETURNS: N/A
 */

void usrKernelInit(void)
{
    classLibInit();

    taskLibInit();

    taskHookLibInit();
    taskHookShowInit();

    semLibInit();
    semBLibInit();
    semMLibInit();
    semCLibInit();

    msgQLibInit();
    msgQShowInit();

    classShowInit();
    taskShowInit();
    semShowInit();

    /* Initialize kernel queues */
    qInit(&activeQHead, qFifoClassId);
    qInit(&tickQHead, qPrioClassId);

#ifdef INCLUDE_CONSTANT_RDY_Q
    qInit(&readyQHead, qPriBmpClassId, 256, kernReadyLst, kernReadyBmp);
#else
    qInit(&readyQHead, qPrioClassId);
#endif /* INCLUDE_CONSTANT_RDY_Q */

    /* Initialize kernel work queue */
    workQLibInit();

    /* Initialize kernel clock to zero */
    tickLibInit();
}

