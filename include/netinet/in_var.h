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

/* in_var.h - Internet variables */

#ifndef _in_var_h
#define _in_var_h

#include <vmx.h>
#include <net/if.h>
#include <netinet/in.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct in_ifaddr {
  struct ifaddr         ia_ifa;                 /* Protocol independent info */
  unsigned long         ia_net;                 /* Network interface number */
  unsigned long         ia_netmask;             /* Net mask */
  unsigned long         ia_subnet;              /* Subnet number */
  unsigned long         ia_subnetmask;          /* Subnet mask */
  struct in_addr        ia_netbroadcast;        /* Broadcast address */
  struct in_ifaddr      *ia_next;               /* Next interface in list */
  struct sockaddr_in    ia_addr;                /* Interface in addrss  */
  struct sockaddr_in    ia_dstaddr;             /* Destination in address */
  struct sockaddr_in    ia_sockmask;            /* General netmask */
};

#define ia_ifp          ia_ifa.ifa_ifp
#define ia_flags        ia_ifa.ifa_flags
#define ia_broadaddr    ia_dstaddr

struct in_aliasreq {
  char                  ifra_name[IFNAMSIZ];    /* Name */
  struct sockaddr_in    ifra_addr;
  struct sockaddr_in    ifra_broadaddr;
  struct sockaddr_in    ifra_mask;
};

#define ifra_dstaddr    ifra_broadaddr

/* Macros */

/*******************************************************************************
 * IA_SIN - Get internet socket address from interface
 *
 * RETURNS: Internet socket address
 ******************************************************************************/

#define IA_SIN(ia)                                                             \
  (&(((struct in_ifaddr *)(ia))->ia_addr))

/*******************************************************************************
 * IN_LOOKUP_MULTI - Lookup multicast record
 *
 * RETURNS: NULL
 ******************************************************************************/

#define IN_LOOKUP_MULTI(addr, ifp, inm)                                        \
  (NULL)

/*******************************************************************************
 * INADDR_TO_IFP - Get interface from inet address
 *
 * RETURNS: N/A
 ******************************************************************************/

#define INADDR_TO_IFP(addr, ifp)                                               \
  ((ifp) = in_ifwithaddr((addr)))

/*******************************************************************************
 * IFP_TO_IA - Get address from interface
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IFP_TO_IA(ifp, ia)                                                     \
  ((ia) = in_addrwithif((ifp)))

/* Functions */
IMPORT void in_ifscrub(struct ifnet *ifp, struct in_ifaddr *ia);
IMPORT struct ifnet* in_ifwithaddr(struct in_addr addr);
IMPORT struct in_ifaddr* in_addrwithif(struct ifnet *ifp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _in_var_h */

