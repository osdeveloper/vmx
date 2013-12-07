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

/* netLib.h - Network interface library */

#ifndef _netLib_h
#define _netLib_h

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/socketvar.h>
#include <net/mbuf.h>
#include <vmx.h>
#include <vmx/taskLib.h>

/* Defines */
#define NETISR_IP		1
#define NETISR_ARP		2
#define NETISR_RARP		3
#define NETISR_IMP		4
#define NETISR_NS		5
#define NETISR_ISO		6
#define NETISR_CCITT		7

#define IP_INTR_MASK		( 1 << NETISR_IP )
#define ARP_INTR_MASK		( 1 << NETISR_ARP )
#define RARP_INTR_MASK		( 1 << NETISR_RARP )
#define IMP_INTR_MASK		( 1 << NETISR_IMP )
#define NS_INTR_MASK		( 1 << NETISR_NS )
#define ISO_INTR_MASK		( 1 << NETISR_ISO )
#define CCITT_INTR_MASK		( 1 << NETISR_CCITT )

#define IP_NO_FORWARDING	0x00000000	/* no IP forwading */
#define IP_DO_FORWARDING	0x00000001	/* IP forwadring */
#define IP_DO_REDIRECT		0x00000002	/* IP redirecting */
#define IP_DO_LARGE_BCAST	0x00000004	/* IP broadcasting */
#define IP_DO_CHECKSUM_SND	0x00000008	/* Send IP checksum */
#define IP_DO_CHECKSUM_RCV	0x00000010	/* Receive IP checksum */
#define IP_DO_ARP_ON_FLAGCHG	0x00000020	/* Send when status changes */
#define IP_DO_IF_STATUS_CHECK	0x00000040	/* Check status of interface */

#define IP_PROTO_NUM_MAX	16		/* Max size of protcol switch */

#ifndef IP_FLAGS_DFLT				/* IP default flags */
#define IP_FLAGS_DFLT		( IP_DO_FORWARDING   | IP_DO_REDIRECT | \
				  IP_DO_CHECKSUM_SND | IP_DO_CHECKSUM_RCV )
#endif /* IP_FLAGS_DFLT */

#ifndef IP_TTL_DFLT
#define IP_TTL_DFLT		64		/* Default time to live */
#endif /* IP_TTL_DFLT */

#ifndef IP_QLEN_DFLT
#define IP_QLEN_DFLT		50		/* Default ip queue length */
#endif /* IP_QLEN_DFLT */

#ifndef IP_FRAG_TTL_DFLT
#define IP_FRAG_TTL_DFLT	60		/* Default ip fragment ttl */
#endif /* IP_FRAG_TTL_DFLT */

#ifndef IP_FRAG_TTL_DFLT
#define IP_FRAM_TTL_DFLT	60		/* Default ip fragment ttl */
#endif /* IP_FRAG_TTL_DFLT */

#define ICMP_NO_MASK_REPLY	0x00000000	/* no ICMP mask reply support */
#define ICMP_DO_MASK_REPLY	0x00000001	/* ICMP mask rply support */

#ifndef RAWIP_SND_SIZE_DFLT
#define RAWIP_SND_SIZE_DFLT	8192		/* Default send buf size */
#endif

#ifndef RAWIP_RCV_SIZE_DFLT
#define RAWIP_RCV_SIZE_DFLT	8192		/* Default send buf size */
#endif

#ifndef ICMP_FLAGS_DFLT				/* ICMP default flags */
#define ICMP_FLAGS_DFLT		( ICMP_NO_MASK_REPLY )
#endif /* ICMP_FLAGS_DFLT */

#define UDP_DO_NO_CKSUM		0x00000000	/* no UDP checksum */
#define UDP_DO_CKSUM_SND	0x00000001	/* UDP checksum */
#define UDP_DO_CKSUM_RCV	0x00000002	/* Send UDP checksum */

#ifndef UDP_FLAGS_DFLT				/* UDP default flags */
#define UDP_FLAGS_DFLT		( UDP_DO_CKSUM_SND | UDP_DO_CKSUM_RCV )
#endif /* UDP_FLAGS_DFLT */

#ifndef UDP_SND_SIZE_DFLT
#define UDP_SND_SIZE_DFLT	8192		/* Default send buf size */
#endif /* UDP_SND_SIZE_DFLT */

#ifndef UDP_RCV_SIZE_DFLT
#define UDP_RCV_SIZE_DFLT	8192		/* Default receive buf size */
#endif /* UDP_RCV_SIZE_DFLT */

#define TCP_NO_RFC1323		0x00000000	/* no TCP RFC 1323 support */
#define TCP_DO_RFC1323		0x00000001	/* TCP RFC 1323 support */

#ifndef TCP_FLAGS_DFLT				/* TCP default flags */
#define TCP_FLAGS_DFLT		( TCP_DO_RFC1323 )
#endif /* TCP_FLAGS_DFLT */

#ifndef TCP_SND_SIZE_DFLT
#define TCP_SND_SIZE_DFLT	8192		/* Default send buf size */
#endif /* TCP_SND_SIZE_DFLT */

#ifndef TCP_RCV_SIZE_DFLT
#define TCP_RCV_SIZE_DFLT	8192		/* Default receive buf size */
#endif /* TCP_RCV_SIZE_DFLT */

#ifndef TCP_CON_TIMEO_DFLT
#define TCP_CON_TIMEO_DFLT	150		/* Default connection timeout */
#endif /* TCP_CON_TIMEO_DFLT */

#ifndef TCP_REXMT_THLD_DFLT
#define TCP_REXMT_THLD_DFLT	3		/* Default threshold */
#endif /* TCP_REXMT_THLD_DFLT */

#ifndef TCP_MSS_DFLT
#define TCP_MSS_DFLT		512		/* Default max segment size */
#endif /* TCP_MSS_DFLT */

#ifndef TCP_MSL_DFLT
#define TCP_MSL_DFLT		30		/* Default segment lifetime */
#endif /* TCP_MSL_DFLT */

#ifndef TCP_RND_TRIP_DFLT
#define TCP_RND_TRIP_DFLT	3		/* Time if no data available */
#endif /* TCP_RND_TRIP_DFLT */

#ifndef TCP_IDLE_TIMEO_DFLT
#define TCP_IDLE_TIMEO_DFLT	14400		/* Default idle timeout */
#endif /* TCP_IDLE_TIMEO_DFLT */

#ifndef TCP_MAX_PROBE_DFLT
#define TCP_MAX_PROBE_DFLT	8		/* Num reties before drop */
#endif /* TCP_MAX_PROBE_DFLT */

#ifndef TCP_RAND_FUNC
#define TCP_RAND_FUNC		(FUNCPTR) rand
#endif /* TCP_RAND_FUNC */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Typedefs */
typedef struct ipcfgparams {
  int		ipCfgFlags;			/* IP configuration flags */
  int		ipDefTtl;			/* IP packet def time to live */
  int		ipIntrQueueLen;			/* IP interrupt queue length */
  int		ipFragTtl;			/* IP fragment time to live */
} IP_CFG_PARAMS;

typedef struct icmpcfgparams {
  int		icmpCfgFlags;			/* ICMP configuration flags */
} ICMP_CFG_PARAMS;

typedef struct rawipcfgparams {
  int		rawipSndSpace;			/* Raw ip send space */
  int		rawipRcvSpace;			/* Raw ip receive space */
} RAWIP_CFG_PARAMS;

typedef struct udpcfgparams {
  int		udpCfgFlags;			/* UDP configuration flags */
  int		udpSndSpace;			/* UDP send space */
  int		udpRcvSpace;			/* UDP receive space */
} UDP_CFG_PARAMS;

typedef struct tcpcfgparams {
  int		tcpCfgFlags;			/* TCP configuration flags */
  int		tcpSndSpace;			/* TCP send space */
  int		tcpRcvSpace;			/* TCP reveive space */
  int		tcpConnectTime;			/* TCP connection timeout */
  int		tcpReTxThresh;			/* TCP dup ack to trig reTx */
  int		tcpMssDflt;			/* TCP def max segment size */
  int		tcpRttDflt;			/* TCP def rtt in no data */
  int		tcpKeepIdle;			/* TCP idle time */
  int		tcpKeepCount;			/* TCP max retry before drop */
  FUNCPTR	pTcpRandFunc;			/* TCP random func tcp_init */
  u_int32_t	tcpMsl;				/* TCP max segment lifetime */
} TCP_CFG_PARAMS;

/* Local defines */
#define NET_JOB_MAX_ARGS	5

/* Local typedefs */
typedef struct {
  FUNCPTR func;
  ARG args[NET_JOB_MAX_ARGS];
} NET_JOB_NODE;

/* Local macros */
#define NET_LIB_RING_SIZE	( 85 * sizeof(NET_JOB_NODE) )

/* Functions */
IMPORT STATUS netLibInit(void);
IMPORT void netTask(void);
IMPORT STATUS netJobAdd(FUNCPTR func, ARG param0, ARG param1, ARG param2,
		 		      ARG param3, ARG param4);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _netLib_h */

