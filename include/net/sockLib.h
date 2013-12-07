/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2010 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* sockLib.h - Socket library header */

#ifndef _sockLib_h
#define _sockLib_h

#include <vmx.h>
#include <sys/time.h>
#include <sys/socket.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct sockFunc {
  FUNCPTR	libInitFunc;
  FUNCPTR	acceptFunc;
  FUNCPTR	bindFunc;
  FUNCPTR	connectFunc;
  FUNCPTR	connectWithTimeoutFunc;
  FUNCPTR	getpeernameFunc;
  FUNCPTR	getsocknameFunc;
  FUNCPTR	listenFunc;
  FUNCPTR	recvFunc;
  FUNCPTR	recvfromFunc;
  FUNCPTR	recvmsgFunc;
  FUNCPTR	sendFunc;
  FUNCPTR	sendtoFunc;
  FUNCPTR	sendmsgFunc;
  FUNCPTR	shutdownFunc;
  FUNCPTR	socketFunc;
  FUNCPTR	getsockoptFunc;
  FUNCPTR	setsockoptFunc;
  FUNCPTR	zbufFunc;
} SOCK_FUNC;

typedef struct sockMap {
  int domainMap;			/* Mapping address family */
  int domainReal;			/* Real address family */
  SOCK_FUNC *pSockFunc;			/* Socket functions */
  struct sockMap *pNext;		/* Pointer to next structure */
} SOCK_MAP;

IMPORT STATUS sockLibInit(int fdMax);
IMPORT STATUS sockMapAdd(FUNCPTR sockMapInitFunc,
			 int domainMap,
			 int domainReal);
IMPORT SOCK_FUNC* sockFdtosockFunc(int sockfd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _sockLib_h */

