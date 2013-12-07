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

/* udp_var.h - Upd datagram internal header */

#ifndef _udp_var_h
#define _udp_var_h

#include <vmx.h>
#include <sys/socket.h>
#include <net/netLib.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>

#define UDP_TTL		30			/* Default time-to-live */

#define UDPCTL_CHECKSUM	1			/* Checksum udp packets */
#define UDPCTL_MAXID	2

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct udpiphdr {
  struct ipovly		ui_i;			/* Ip structure */
  struct udphdr		ui_u;			/* Udp header */
};

#define ui_next		ui_i.ih_next
#define ui_prev		ui_i.ih_prev
#define ui_x1		ui_i.ih_x1
#define ui_pr		ui_i.ih_pr
#define ui_len		ui_i.ih_len
#define ui_src		ui_i.ih_src
#define ui_dst		ui_i.ih_dst
#define ui_sport	ui_u.uh_sport
#define ui_dport	ui_u.uh_dport
#define ui_ulen		ui_u.uh_ulen
#define ui_sum		ui_u.uh_sum

struct udpstat {
  unsigned long		udps_ipackets;		/* Total input packets */
  unsigned long		udps_hdrops;		/* Packets shorter than head */
  unsigned long		udps_badsum;		/* Checksum error */
  unsigned long		udps_badlen;		/* Length bigger than packet */
  unsigned long		udps_noport;		/* Invalid port */
  unsigned long		udps_noportbcast;	/* No broadcast port */
  unsigned long		udps_fullsock;		/* Input socket full */
  unsigned long		udps_pcbcachemiss;	/* Input packet cache miss */
  unsigned long		udps_pcbhashmiss;	/* Input packet hash miss */
  unsigned long		udps_opackets;		/* Total output packets */
};

/* Functions */
IMPORT STATUS udpLibInit(UDP_CFG_PARAMS *cfg);
IMPORT void udp_input(struct mbuf *m, int iphlen);
IMPORT void udp_ctlinput(int cmd, struct sockaddr *sa, struct ip *ip);
IMPORT int udp_output(struct inpcb *inp,
		      struct mbuf *m,
		      struct mbuf *addr,
		      struct mbuf *control);
IMPORT int udp_usrreq(struct socket *so,
		      int req,
		      struct mbuf *m,
		      struct mbuf *addr,
		      struct mbuf *control);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udp_var_h */

