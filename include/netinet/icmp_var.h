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

/* icmp_var.h - ICMP internal header */

#ifndef _icmp_var_h
#define _icmp_var_h

#include <vmx.h>
#include <net/mbuf.h>
#include <net/if.h>
#include <net/netLib.h>
#include <netinet/ip_icmp.h>

#define ICMPCTL_MASKREPL		1
#define ICMPCTL_MAXID			2

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct icmpstat {
  unsigned long		icps_error;		/* Num calls to icmp_error */
  unsigned long		icps_oldshort;		/* No error, old ip too short */
  unsigned long		icps_oldicmp;		/* No error, old was icmp */
  unsigned long		icps_outhist[ICMP_MAXTYPE + 1];
  unsigned long		icps_badcode;		/* Code out of range */
  unsigned long		icps_tooshort;		/* Too small packet */
  unsigned long		icps_checksum;		/* Bad checksum */
  unsigned long		icps_badlen;		/* Calculated bound mismatch */
  unsigned long		icps_reflect;		/* Number of responses */
  unsigned long		icps_inhist[ICMP_MAXTYPE + 1];
};

/* Functions */
IMPORT STATUS icmpLibInit(ICMP_CFG_PARAMS *cfg);
IMPORT void icmp_error(struct mbuf *n,
		       int type,
		       int code,
		       unsigned long dest,
		       struct ifnet *destifp);
IMPORT void icmp_input(struct mbuf *m, int hlen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _icmp_var_h */

