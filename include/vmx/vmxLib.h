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

/* vmxLib.h - Task scheduling library */

#ifndef _vmxLib_h
#define _vmxLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <util/qLib.h>
#include <vmx/taskLib.h>

IMPORT void vmxSpawn(TCB_ID pTcb);
IMPORT STATUS vmxDelete(TCB_ID pTcb);
IMPORT void vmxSuspend(TCB_ID pTcb);
IMPORT void vmxResume(TCB_ID pTcb);
IMPORT void vmxTickAnnounce(void);
IMPORT STATUS vmxDelay(unsigned timeout);
IMPORT STATUS vmxUndelay(TCB_ID pTcb);
IMPORT void vmxPriorityNormalSet(TCB_ID pTcb, unsigned priority);
IMPORT void vmxPrioritySet(TCB_ID pTcb, unsigned priority);

IMPORT void vmxPendQGet(Q_HEAD *pQHead);
IMPORT void vmxReadyQPut(TCB_ID pTcb);
IMPORT void vmxReadQRemove(Q_HEAD *pQHead, unsigned timeout);
IMPORT void vmxPendQFlush(Q_HEAD *pQHead);
IMPORT STATUS vmxPendQPut(Q_HEAD *pQHead, unsigned timeout);
IMPORT STATUS vmxPendQRemove(TCB_ID pTcb);
IMPORT void vmxPendQTerminate(Q_HEAD *pQHead);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vmxLib_h */

