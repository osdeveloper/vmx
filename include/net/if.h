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

/* if.h - Interface library header */

#ifndef _if_h
#define _if_h

#include <vmx.h>
#include <sys/socket.h>
#include <net/mbuf.h>
#include <net/socketvar.h>

/* Defines */
#define IFF_UP			0x001		/* Interface is up */
#define IFF_BROADCAST		0x002		/* Broadcast address valid */
#define IFF_DEBUG		0x004		/* Debugging */
#define IFF_LOOPBACK		0x008		/* Loopback address */
#define IFF_POINTOPOINT		0x010		/* Point-to-point link */
#define IFF_NOTRAILERS		0x020		/* Avoid trailers */
#define IFF_RUNNING		0x040		/* Interface allocated */
#define IFF_NOARP		0x080		/* No arp protocol */
#define IFF_PROMISC		0x100		/* Receive all pkts */
#define IFF_ALLMULTI		0x200		/* Receive all multicast pkts */
#define IFF_OACTIVE		0x400		/* Transmition if progress */
#define IFF_SIMPLEX		0x800		/* Can't hear transmissions */
#define IFF_LINK0		0x1000		/* Link 0 */
#define IFF_LINK1		0x2000		/* Link 1 */
#define IFF_LINK2		0x4000		/* Link 2 */
#define IFF_MULTICAST		0x8000		/* Supports multicasting */
#define IFF_RFC2233		0x10000		/* Supports RFC 2233 */
#define IFF_UNNUMBERED		IFF_POINTOPOINT

#define IFF_CANTCHANGE		(IFF_BROADCAST | IFF_POINTOPOINT |             \
				 IFF_RUNNING | IFF_OACTIVE | IFF_SIMPLEX |     \
				 IFF_MULTICAST | IFF_ALLMULTI)

#define IFF_FP_ENABLE		IFF_LINK0	/* Fastpath enabled */
#define IFF_DONT_FORWARD	IFF_LINK1	/* Forwarding disabled */
#define IFF_PROXY_ALL		IFF_LINK2	/* Enable proxy all */

#define IFA_ROUTE		RTF_UP		/* Interface got route */

#define IFNAMSIZ		16		/* Interface name size */
#define IFQ_MAXLEN		50		/* Max queue length */
#define IFNET_SLOWHZ		1		/* Slow timeout */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Interface queue structure */
struct ifqueue {
  struct mbuf			*ifq_head;		/* Head of queue */
  struct mbuf			*ifq_tail;		/* Tail of queue */
  int				ifq_len;		/* Length */
  int 				ifq_maxlen;		/* Maximum length */
  int				ifq_drops;		/* Packet drops */
};

/* Interface data structure */
struct if_data {
  unsigned char			ifi_type;		/* Newtowk type */
  unsigned char			ifi_addrlen;		/* Address length */
  unsigned char			ifi_hdrlen;		/* Header length */
  unsigned long			ifi_mtu;		/* Max trans unit */
  unsigned long			ifi_metric;		/* Routing metric */
  unsigned long			ifi_baudrate;		/* Linespeed */
  unsigned long			ifi_ipackets;		/* Packets received */
  unsigned long			ifi_ierrors;		/* Input errors */
  unsigned long			ifi_opackets;		/* Packets sent */
  unsigned long			ifi_oerrors;		/* Output errors */
  unsigned long			ifi_collisions;		/* Collisions */
  unsigned long			ifi_ibytes;		/* Num octs received */
  unsigned long			ifi_obytes;		/* Num octs sent */
  unsigned long			ifi_imcasts;		/* Pkt rcvd multicast */
  unsigned long			ifi_omcasts;		/* Pkt sent multicast */
  unsigned long			ifi_iqdrops;		/* Dropped on input */
  unsigned long			ifi_noproto;		/* Not supp proto */
  unsigned long			ifi_lastchange;		/* Last updated */
};

#define if_mtu			if_data.ifi_mtu
#define if_type			if_data.ifi_type
#define if_addlen		if_data.ifi_addlen
#define if_hdrlen		if_data.ifi_hdrlen
#define if_metric		if_data.ifi_metric
#define if_baudrate		if_data.ifi_baudrate
#define if_ipackets		if_data.ifi_ipackets
#define if_ierrors		if_data.ifi_ierrors
#define if_opackets		if_data.ifi_opackets
#define if_oerrors		if_data.ifi_oerrors
#define if_collisions		if_data.ifi_collisions
#define if_ibytes		if_data.ifi_ibytes
#define if_obytes		if_data.ifi_obytes
#define if_imcasts		if_data.ifi_imcasts
#define if_omcasts		if_data.ifi_omcasts
#define if_iqdrops		if_data.ifi_iqdrops
#define if_noproto		if_data.ifi_noproto
#define if_lastchange		if_data.ifi_lastchange

/* Interface structure */
struct ifnet {
  char				*if_name;		/* Interface name */
  struct ifnet 			*if_next;		/* Next element */
  struct ifaddr			*if_addrlist;		/* Address list */
  int				if_pcount;		/* Num plisteners */
  unsigned char			*if_bpf;		/* Pkt filter struct */
  unsigned short		if_index;		/* Num interface idx */
  short				if_unit;		/* Unit lower lvl drv */
  short				if_timer;		/* Timer */
  short				if_flags;		/* Flags */
  struct if_data		if_data;		/* Interface stat */
  struct mBlk			*pInmBlk;		/* Chn muticast addr */

  FUNCPTR			if_init;		/* Initialize func */
  FUNCPTR			if_output;		/* Output func */
  FUNCPTR			if_start;		/* Initialize func */
  FUNCPTR			if_ioctl;		/* Ioctl func */
  FUNCPTR			if_reset;		/* Reset func */
  FUNCPTR			if_resolve;		/* Arp resolve func */
  FUNCPTR			if_watchdog;		/* Timer func */

  struct ifqueue		if_snd;			/* Output queue */
  void				*pCookie;		/* Data ip mux attach */
};

#define if_mtu			if_data.ifi_mtu
#define if_type			if_data.ifi_type
#define if_addrlen		if_data.ifi_addrlen
#define if_hdrlen		if_data.ifi_hdrlen
#define if_metric		if_data.ifi_metric

/* Interface address structure */
struct ifaddr {
  struct sockaddr		*ifa_addr;		/* Interface address */
  struct sockaddr		*ifa_dstaddr;		/* End of p2p link */
  struct sockaddr		*ifa_netmask;		/* Subnet mask */
  struct ifnet			*ifa_ifp;		/* Backptr interface */
  struct ifaddr			*ifa_next;		/* Next addr */

  FUNCPTR			ifa_rtrequest;		/* Check/clean routes */

  unsigned short		ifa_flags;		/* Flags */
  short				ifa_refcnt;		/* Reference count */
  int				ifa_metric;		/* Metric */
};

#define ifa_broadaddr		ifa_dstaddr		/* Broadcast address */

struct ifreq {
  char				ifr_name[IFNAMSIZ];	/* Interface name */
  union {
    struct sockaddr		  ifru_addr;
    struct sockaddr		  ifru_dstaddr;
    struct sockaddr		  ifru_broadaddr;
    short			  ifru_flags;
    int				  ifru_metric;
    char			  *ifru_data;
  } ifr_ifru;
};

#define ifr_addr		ifr_ifru.ifru_addr
#define ifr_dstaddr		ifr_ifru.ifru_dstaddr
#define ifr_broadaddr		ifr_ifru.ifru_broadaddr
#define ifr_flags		ifr_ifru.ifru_flags
#define ifr_metric		ifr_ifru.ifru_metric
#define ifr_data		ifr_ifru.ifru_data

struct ifconf {
  int				ifc_len;		/* Size of buffer */
  union {
    void *			  ifcu_buf;
    struct ifreq		  *ifcu_req;
  } ifc_ifcu;
};

#define ifc_buf			ifc_ifcu.ifcu_buf
#define ifc_req			ifc_ifcu.ifcu_req

struct ifaliasreq {
  char				ifra_name[IFNAMSIZ];	/* Name */
  struct sockaddr		ifra_addr;
  struct sockaddr		ifra_broadaddr;
  struct sockaddr		ifra_mask;
};

/* Macros */

/*******************************************************************************
 * IFAFREE - Free interface address
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IFAFREE(ifa)							       \
  ((ifa)->ifa_refcnt <= 0) ? (ifafree(ifa)) : ((ifa)->ifa_refcnt--)

/*******************************************************************************
 * IF_QFULL - Check if interface output queue is full
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define IF_QFULL(ifq)		((ifq)->ifq_len >= (ifq)->ifq_maxlen)

/*******************************************************************************
 * IF_DROP - Increase drop counter for interface queue
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_DROP(ifq)		((ifq)->ifq_drops++)

/*******************************************************************************
 * IF_ENQUEUE - Put on interface queue
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_ENQUEUE(ifq, m)						       \
{									       \
  (m)->m_nextpkt = NULL;						       \
  if ( (ifq)->ifq_tail == NULL )					       \
    (ifq)->ifq_head = (m);						       \
  else									       \
    (ifq)->ifq_tail->m_nextpkt = (m);					       \
  (ifq)->ifq_tail = (m);						       \
  (ifq)->ifq_len++;							       \
}

/*******************************************************************************
 * IF_PREPEND - Put on interface queue head
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_PREPEND(ifq, m)						       \
{									       \  (m)->m_nextpkt = NULL;						       \
  if ( (ifq)->ifq_tail == NULL )					       \
    (ifq)->ifq_tail = (m);						       \
  (ifq)->ifq_head = (m);						       \
  (ifq)->ifq_len++;							       \
}

/*******************************************************************************
 * IF_ADJ - Adjust packet after removed from interface queue
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_ADJ(m)							       \
{									       \
  (m)->m_off += sizeof(struct ifnet *);					       \
  (m)->m_len -= sizeof(struct ifnet *);					       \
  if ( (m)->m_len == 0 )						       \
    (m) = m_free(m);							       \
}

/*******************************************************************************
 * IF_DEQUEUEIF - Remove from interface queue and update interface pointer
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_DEQUEUEIF(ifq, m, ifp)					       \
{									       \
  (m) = (ifq)->ifq_head;						       \
  if (m != NULL) {							       \
    if ( ((ifq)->ifq_head = (m)->m_nextpkt) == NULL )			       \
      (ifq)->ifq_tail = NULL;						       \
    (m)->m_nextpkt = NULL;						       \
    (ifq)->ifq_len--;							       \
    (ifp) = *( mtod((m)), struct ifnet *) );				       \
    IF_ADJ(m);								       \
  }									       \
}

/*******************************************************************************
 * IF_DEQUEUE - Remove from interface queue
 *
 * RETURNS: N/A
 ******************************************************************************/

#define IF_DEQUEUE(ifq, m)						       \
{									       \
  (m) = (ifq)->ifq_head;						       \
  if (m != NULL) {							       \
    if ( ((ifq)->ifq_head = (m)->m_nextpkt) == NULL )			       \
      (ifq)->ifq_tail = NULL;						       \
    (m)->m_nextpkt = NULL;						       \
    (ifq)->ifq_len--;							       \
  }									       \
}

/* Functions */
IMPORT STATUS ifLibInit(int ifqMaxLen);
IMPORT STATUS ifIndexAlloc(void);
IMPORT BOOL ifIndexVerify(int index);
IMPORT struct ifnet* ifIndexToIfp(int index);

IMPORT STATUS if_attach(struct ifnet *ifp);

IMPORT struct ifnet* ifunit(char *ifname);

IMPORT struct ifaddr* ifa_ifwithaddr(struct sockaddr *addr);
IMPORT struct ifaddr* ifa_ifwithdstaddr(struct sockaddr *addr);
IMPORT struct ifaddr* ifa_ifwithnet(struct sockaddr *addr);
IMPORT struct ifaddr* ifa_ifwithaf(int af);
IMPORT struct ifaddr* ifaof_ifpforaddr(struct sockaddr *addr,
				       struct ifnet *ifp);

IMPORT void if_down(struct ifnet *ifp);
IMPORT void if_up(struct ifnet *ifp);
IMPORT void if_qflush(struct ifqueue *ifq);
IMPORT void if_slowtimo(void);

IMPORT int ifioctl(struct socket *so, int cmd, void *data);
IMPORT int ifconf(int cmd, void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _if_h */

