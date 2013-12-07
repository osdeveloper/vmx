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

/* in_pcb.h - Internet control block */

#ifndef _in_pcb_h
#define _in_pcb_h

#include <vmx.h>
#include <util/listLib.h>
#include <net/socketvar.h>
#include <net/if.h>
#include <net/mbuf.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>

/* Defines */
#define INPLOOKUP_WILDCARD		1

#define INP_RECVOPTS			0x01	/* Receive incoming ip opts */
#define INP_RECVRETOPTS			0x02	/* Receive ip opts */
#define INP_RECVDSTADDR			0x04	/* Receie dest addr */
#define INP_HDRINCL			0x80	/* Supplied ip header */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#define INP_CONTROLOPTS		(INP_RECVOPTS |				       \
				 INP_RECVRETOPTS |			       \
			         INP_RECVDSTADDR)

/* Structs */
struct ip_moptions {
  struct ifnet		*imo_multicast_ifp;	/* If for outgoiung multicast */
  unsigned char		imo_multicast_ttl;	/* Time to live */
  unsigned char		imo_multicast_loop;	/* Header send if member */
  unsigned short	imo_num_memberships;	/* Num memberships */
  struct mbuf		*imo_mbuf;		/* Chain of addresses */
  unsigned long		imo_multicast_vif;	/* Multicas vif to send to */
};

struct inpcbinfo {
  LIST			*listhead;		/* Head of list */
  LIST			*hashbase;		/* Hash list array */
  unsigned long		hashmask;		/* Hash table mask */
  unsigned short	lastport;		/* Last port */
};

struct inpcb {
  LIST_NODE		inp_list;		/* List of all pcbs */
  LIST_NODE		inp_hash;		/* Hash list */
  struct inpcbinfo	*inp_pcbinfo;		/* Pcb info */
  struct in_addr	inp_faddr;		/* Foreign address */
  unsigned short	inp_fport;		/* Foreign port */
  struct in_addr	inp_laddr;		/* Local address */
  unsigned short	inp_lport;		/* Local port */
  struct socket		*inp_socket;		/* Back pointer to socket */
  void			*inp_ppcb;		/* Pointer to pcb */
  struct route		inp_route;		/* Routing entry */
  int			inp_flags;		/* Flags */
  struct ip		inp_ip;			/* IP packet */
  struct mbuf		*inp_options;		/* IP options */
  struct ip_moptions	*inp_moptions;		/* IP multicast options */
};

/* Macros */

/*******************************************************************************
 * sotoinpcb - Get pcb from socket
 *
 * RETURNS: Pointer to control block
 ******************************************************************************/

#define sotoinpcb(so)	((struct inpcb *) (so)->so_pcb)

/*******************************************************************************
 * INP_PCBHASH - Get pcb hash bucket
 *
 * RETURNS: Hash bucket
 ******************************************************************************/

#define INP_PCBHASH(faddr, lport, fport, mask)				       \
  ( ( (faddr) ^ ((faddr) >> 16) ^ (lport) ^ (fport) ) & (mask) )

/* Functions */
IMPORT int in_pcballoc(struct socket *so, struct inpcbinfo *pcbinfo);
IMPORT int in_pcbbind(struct inpcb *inp, struct mbuf *name);
IMPORT int in_pcbladdr(struct inpcb *inp,
		       struct mbuf *name,
		       struct sockaddr_in **plocal_sin);
IMPORT int in_pcbconnect(struct inpcb *inp, struct mbuf *name);
IMPORT void in_pcbdisconnect(struct inpcb *inp);
IMPORT void in_pcbdetach(struct inpcb *inp);
IMPORT void in_pcbinshash(struct inpcb *inp);
IMPORT int in_pcbhash(struct inpcb *inp);
IMPORT void in_pcbchghash(struct inpcb *inp, int oi);
IMPORT void in_pcbrehash(struct inpcb *inp);
IMPORT void in_pcbremhash(struct inpcb *inp);
IMPORT struct inpcb* in_pcblookuphash(struct inpcbinfo *pcbinfo,
				      struct in_addr faddr,
				      unsigned int fport_arg,
				      struct in_addr laddr,
				      unsigned int lport_arg,
				      int wildcard);
IMPORT struct inpcb* in_pcblookup(struct inpcbinfo *pcbinfo,
				  struct in_addr faddr,
				  unsigned int fport_arg,
				  struct in_addr laddr,
				  unsigned int lport_arg,
				  int wild_okay);
IMPORT void in_pcbnotify(LIST *pList,
		         struct sockaddr *dst,
		         unsigned int fport_arg,
		         struct in_addr laddr,
		         unsigned int lport_arg,
		         int cmd,
		         VOIDFUNCPTR notify);
IMPORT void in_losing(struct inpcb *inp);
IMPORT void in_setsockaddr(struct inpcb *inp, struct mbuf *name);
IMPORT void in_setpeeraddr(struct inpcb *inp, struct mbuf *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _in_pcb_h */

