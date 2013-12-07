/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
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

/* in.h - Internet header file */

#ifndef _in_h
#define _in_h

#include <arch/arch.h>
#include <net/socketvar.h>
#include <net/if.h>

/* Defines */
#define IPPROTO_IP			0	/* Start of protocols */
#define IPPROTO_ICMP			1	/* Control message protocol */
#define IPPROTO_IGMP			2	/* Group management protocol */
#define IPPROTO_GGP			3	/* Gateway (deprecated) */
#define IPPROTO_TCP			6	/* TCP protocol */
#define IPPROTO_EGP			8	/* Exteriour gateway protocol */
#define IPPROTO_PUP			12	/* PUP protocol */
#define IPPROTO_UDP			17	/* User datagram protocol */
#define IPPROTO_IDP			22	/* XNS idp protocol */
#define IPPROTO_TP			29	/* TP-4 w. class negotiation */
#define IPPROTO_EON			80	/* ISO cnlp */
#define IPPROTO_OSPF			89	/* OSPF ver. 2 */
#define IPPROTO_ENCAP			98	/* Encapsulation head */
#define IPPROTO_RAW			255	/* RAW IP packet */
#define IPPROTO_MAX			256	/* End of protocols */

#define IPPROTO_MAXID			( IPOROTO_IDP + 1 )

#define IPPORT_RESERVED			1024	/* Reserved ports */
#define IPPORT_USERRESERVED		5000

#define IP_OPTIONS			1	/* IP options */
#define IP_HDRINCL			2	/* Header included with data */
#define IP_TOS				3	/* IP type of service preceed */
#define IP_TTL				4	/* IP time to live */
#define IP_RECVOPTS			5	/* IP receive opts. */
#define IP_RECVRETOPTS			6	/* IP receive response opts. */
#define IP_RECVDSTADDR			7	/* IP receive dest with dgram */
#define IP_RETOPTS			8	/* IP options set/get */
#define IP_MULTICAST_IF			9	/* IP multicast interface */
#define IP_MULTICAST_TTL		10	/* IP multicast time to live */
#define IP_MULTICAST_LOOP		11	/* IP multicast loopback */
#define IP_ADD_MEMBERSHIP		12	/* IP add group membership */
#define IP_DROP_MEMBERSHIP		13	/* IP drop group membership */
#define IP_MULTICAST_IFINDEX		14	/* IP multicast if */
#define IP_ADD_MEMBERSHIP_INDEX		15	/* IP add memhership index */
#define IP_DROP_MEMBERSHIP_INDEX	16	/* IP drop membership index */

#define IP_DEFAULT_MULTICAST_TTL	1
#define IP_DEFAULT_MULTICAST_LOOP	1
#define IP_MAX_MEMBERSHIPS		20

#define IPCTL_FORWARDING		1	/* Act as a router */
#define IPCTL_SENDREDIRECTS		2	/* Might send redir when forw */
#define IPCTL_DEFTTL			3	/* Default time to live */
#define IPCTL_MAXID			4

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct in_addr {
  unsigned long		s_addr;
};

struct sockaddr_in {
  unsigned char		sin_len;
  unsigned char		sin_family;
  unsigned short	sin_port;
  struct in_addr	sin_addr;
  char			sin_zero[8];
};

struct sockaddr_rt {
  unsigned char		srt_len;
  unsigned char		srt_family;
  unsigned char		srt_proto;
  unsigned char		srt_tos;
  struct in_addr	srt_addr;
  char			srt_zero[8];
};

struct ip_opts {
  struct in_addr	ip_dst;
  char			opts[40];
};

struct ip_mreq {
  struct in_addr	imr_multiaddr;
  union {
    struct in_addr	  u_imr_interface;
#ifdef ROUTER_STACK
    int			  u_imr_index;
#endif /* ROUTER_STACK */
  } imr_arg;
};

/* Macros */
#define IN_CLASSA(i)		( ((long)(i) & 0x80000000) == 0 )
#define IN_CLASSA_NET		0xff000000
#define IN_CLASSA_NSHIFT	24
#define IN_CLASSA_HOST		0x00ffffff
#define IN_CLASSA_MAX		128

#define IN_CLASSB(i)		( ((long)(i) & 0xc0000000) == 0x80000000 )
#define IN_CLASSB_NET		0xffff0000
#define IN_CLASSB_NSHIFT	16
#define IN_CLASSB_HOST		0x0000ffff
#define IN_CLASSB_MAX		65536

#define IN_CLASSC(i)		( ((long)(i) & 0xe0000000) == 0xc0000000 )
#define IN_CLASSC_NET		0xffffff00
#define IN_CLASSC_NSHIFT	8
#define IN_CLASSC_HOST		0x000000ff

#define IN_CLASSD(i)		( ((long)(i) & 0xf0000000) == 0xe0000000 )
#define IN_CLASSD_NET		0xf0000000
#define IN_CLASSD_NSHIFT	28
#define IN_CLASSD_HOST		0x0fffffff
#define IN_MULTICAST(i)		IN_CLASSD(i)

#define IN_EXPERIMENTAL(i)	( ((long)(i) & 0xf0000000) == 0xf0000000 )
#define IN_BADCLASS(i)		( ((long)(i) & 0xf0000000) == 0xf0000000 )

#define INADDR_ANY		(unsigned long) 0x00000000
#define	INADDR_LOOPBACK		(unsigned long) 0x7f000001
#define INADDR_BROADCAST	(unsigned long) 0xffffffff
#define INADDR_UNSPEC_GROUP	(unsigned long) 0xe0000000
#define INADDR_ALLHOSTS_GROUP	(unsigned long) 0xe0000001
#define INADDR_ALLRTRS_GROUP	(unsigned long) 0xe0000002
#define INADDR_MAX_LOCAL_GROUP	(unsigned long) 0xe00000ff
#define INADDR_NONE		(unsigned long) 0xffffffff

#define IN_LOOPBACKNET		127

#define TOS_SET(s, v)		( ((struct sockaddr_rt *)(s))->srt_tos = (v) )
#define TOS_GET(s)		( ((struct sockaddr_rt *)(s))->srt_tos )
#define RT_PROTO_SET(s, v)	( ((struct sockaddr_rt *)(s))->srt_proto = (v) )
#define RT_PROTO_GET(s)		( ((struct sockaddr_rt *)(s))->srt_proto )

#define imr_interface		imr_arg.u_imr_interface
#ifdef ROUTER_STACK
#define imr_index		imr_arg.u_imr_index
#endif /* ROUTER_STACK */

#if ( _BYTE_ORDER == _BIG_ENDIAN )

#define ntohl(x)		(x)
#define ntohs(x)		(x)
#define htonl(x)		(x)
#define htons(x)		(x)

#elif ( _BYTE_ORDER == _LITTLE_ENDIAN )

#define ntohl(x)		(  ( ((x) & 0x000000ff) << 24 ) |	       \
				   ( ((x) & 0x0000ff00) << 8  ) |	       \
				   ( ((x) & 0x00ff0000) >> 8  ) |	       \
				   ( ((x) & 0xff000000) >> 24 )   )

#define ntohs(x)		(  ( ((x) & 0x00ff) << 8 ) |		       \
				   ( ((x) & 0xff00) >> 8 )   )

#define htonl(x)		(  ( ((x) & 0x000000ff) << 24 ) |	       \
				   ( ((x) & 0x0000ff00) << 8  ) |	       \
				   ( ((x) & 0x00ff0000) >> 8  ) |	       \
				   ( ((x) & 0xff000000) >> 24 )   )

#define htons(x)		(  ( ((x) & 0x00ff) << 8 ) |		       \
				   ( ((x) & 0xff00) >> 8 )   )

#endif

#define NTOHL(x)		(x) = ntohl( (unsigned long)(x) )
#define NTOHS(x)		(x) = ntohs( (unsigned short)(x) )
#define HTONL(x)		(x) = htonl( (unsigned long)(x) )
#define HTONS(x)		(x) = htons( (unsigned short)(x) )

/* Functions */

IMPORT void inLibInit(void);
IMPORT unsigned long in_netof(struct in_addr in);
IMPORT int in_localaddr(struct in_addr in);
IMPORT void in_socktrim(struct sockaddr_in *sin);
IMPORT int in_control(struct socket *so,
		      unsigned long cmd,
		      void *data,
		      struct ifnet *ifp);
IMPORT int in_broadcast(struct in_addr in, struct ifnet *ifp);
IMPORT int in_canforward(struct in_addr in);
IMPORT int in_cksum(struct mbuf *m, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _in_h */

