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

/* inetLib.h - Internet library header */

#ifndef _inetLib_h
#define _inetLib_h

#include <vmx.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <ostool/moduleNumber.h>

#define S_inetLib_ILLEGAL_INTERNET_ADDRESS	(M_inetLib | 0x0001)

#define INET_ADDR_LEN				18	/* Max inet addr len */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

IMPORT struct protosw inetsw[];
IMPORT struct domain inetdomain;

IMPORT void inetLibInit(void);
IMPORT unsigned long inet_addr(char *inetString);
IMPORT void inet_ntoa(struct in_addr addr, char *str);
IMPORT STATUS ifAddrAdd(char *ifName, char *ifAddr,
			char *broadAddr,
			int subnetMask);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _inetLib_h */

