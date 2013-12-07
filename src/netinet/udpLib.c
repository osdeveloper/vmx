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

/* udpLib.c - Udp datagram protocol */

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)udp_usrreq.c	8.6 (Berkeley) 5/23/95
 *	$Id: udpLib.c,v 1.3 2010/06/04 08:34:14 alexuspol Exp $
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <vmx.h>
#include <util/listLib.h>
#include <os/unixLib.h>
#include <net/mbuf.h>
#include <net/protosw.h>
#include <net/socketvar.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

/* Imports */
IMPORT struct in_addr zeroin_addr;
IMPORT int ip_defttl;
IMPORT unsigned char inetctlerrmap[];

/* Locals */
LOCAL int udp_sendspace;
LOCAL int udp_recvspace;

LOCAL void udp_detach(struct inpcb *inp);
LOCAL void udp_notify(struct inpcb *inp, int errno);
LOCAL struct mbuf* udp_saveopt(char *p, int size, int type);

BOOL udpDoCkSumSnd = FALSE;
BOOL udpDoCkSumRcv = FALSE;

struct inpcbinfo udbinfo;
struct inpcb *udp_last_inpcb = NULL;

unsigned short udp_pcbhashsize = 64;

struct	udpstat udpstat;	/* from udp_var.h */
struct	sockaddr_in udp_in;

/*******************************************************************************
 * udpLibInit - Inititalize udp library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS udpLibInit(UDP_CFG_PARAMS *cfg)
{
  /* Setup globals */
  udp_sendspace = cfg->udpSndSpace;
  udp_recvspace = cfg->udpRcvSpace;

  if (cfg->udpCfgFlags & UDP_DO_CKSUM_SND)
    udpDoCkSumSnd = TRUE;
  else
    udpDoCkSumSnd = FALSE;

  if (cfg->udpCfgFlags & UDP_DO_CKSUM_RCV)
    udpDoCkSumRcv = TRUE;
  else
    udpDoCkSumRcv = FALSE;

  udp_in.sin_len = sizeof(struct sockaddr_in);
  udp_in.sin_family = AF_INET;

  /* Inititalize list */
  udbinfo.listhead = malloc( sizeof(LIST) );
  if (udbinfo.listhead == NULL)
    return ERROR;

  listInit(udbinfo.listhead);

  /* Inititalize hash table */
  udbinfo.hashbase = hashinit(udp_pcbhashsize, MT_PCB, &udbinfo.hashmask);
  if (udbinfo.hashbase == NULL) {

    free(udbinfo.listhead);
    return ERROR;

  }

  return OK;
}

/*******************************************************************************
 * udp_input - Udp datagram input
 *
 * RETURNS: N/A
 ******************************************************************************/

void udp_input(struct mbuf *m, int iphlen)
{
	struct ip *ip;
	struct udphdr *uh;
	struct inpcb *inp;
	struct mbuf *opts = NULL;
	int len;
	struct ip save_ip;
	struct socket *last;
	struct mbuf *n;
	struct mbuf **mp;

	udpstat.udps_ipackets++;

	/*
	 * Strip IP options, if any; should skip this,
	 * make available to user, and use on returned packets,
	 * but we don't yet have a way to check the checksum
	 * with options still present.
	 */
	if (iphlen > sizeof (struct ip)) {

		ip_stripoptions(m, (struct mbuf *)0);
		iphlen = sizeof(struct ip);

	}

	/*
	 * Get IP and UDP header together in first mbuf.
	 */
	ip = mtod(m, struct ip *);
	if (m->m_len < iphlen + sizeof(struct udphdr)) {

		if ((m = m_pullup(m, iphlen + sizeof(struct udphdr))) == NULL) {

			udpstat.udps_hdrops++;
			return;

		}

		ip = mtod(m, struct ip *);

	}

	uh = (struct udphdr *)((caddr_t)ip + iphlen);

	/*
	 * Make mbuf data length reflect UDP length.
	 * If not enough data to reflect UDP length, drop.
	 */
	len = ntohs((u_short)uh->uh_ulen);
	if (len < sizeof(struct udphdr)) {

		udpstat.udps_badlen++;
		goto bad;

	}

	if (ip->ip_len != len) {

		if (len > ip->ip_len) {

			udpstat.udps_badlen++;
			goto bad;

		}

		m_adj(m, len - ip->ip_len);
		/* ip->ip_len = len; */

	}

	/*
	 * Save a copy of the IP header in case we want restore it
	 * for sending an ICMP error message in response.
	 */
	save_ip = *ip;

	/*
	 * Checksum extended UDP header and data.
	 */
	if (udpDoCkSumRcv && uh->uh_sum) {

		((struct ipovly *)ip)->ih_next = NULL;
		((struct ipovly *)ip)->ih_prev = NULL;
		((struct ipovly *)ip)->ih_x1 = 0;
		((struct ipovly *)ip)->ih_len = uh->uh_ulen;
		uh->uh_sum = in_cksum(m, len + sizeof (struct ip));
		if (uh->uh_sum) {

			udpstat.udps_badsum++;
			m_freem(m);
			return;

		}

	}

	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) ||
	    in_broadcast(ip->ip_dst, m->m_pkthdr.rcvif)) {

		/*
		 * Deliver a multicast or broadcast datagram to *all* sockets
		 * for which the local and remote addresses and ports match
		 * those of the incoming datagram.  This allows more than
		 * one process to receive multi/broadcasts on the same port.
		 * (This really ought to be done for unicast datagrams as
		 * well, but that would cause problems with existing
		 * applications that open both address-specific sockets and
		 * a wildcard socket listening to the same port -- they would
		 * end up receiving duplicates of every unicast datagram.
		 * Those applications open the multiple sockets to overcome an
		 * inadequacy of the UDP socket interface, but for backwards
		 * compatibility we avoid the problem here rather than
		 * fixing the interface.  Maybe 4.5BSD will remedy this?)
		 */

		/*
		 * Construct sockaddr format source address.
		 */
		udp_in.sin_port = uh->uh_sport;
		udp_in.sin_addr = ip->ip_src;
		m->m_len -= sizeof (struct udpiphdr);
		m->m_data += sizeof (struct udpiphdr);

		/*
		 * Locate pcb(s) for datagram.
		 * (Algorithm copied from raw_intr().)
		 */
		last = NULL;
		for (inp = (struct inpcb *) LIST_HEAD(udbinfo.listhead);
		     inp != NULL;
		     inp = (struct inpcb *) LIST_NEXT(&inp->inp_list)) {

			if (inp->inp_lport != uh->uh_dport)
				continue;

			if (inp->inp_laddr.s_addr != INADDR_ANY) {

				if (inp->inp_laddr.s_addr !=
				    ip->ip_dst.s_addr)
					continue;

			}

			if (inp->inp_faddr.s_addr != INADDR_ANY) {

				if (inp->inp_faddr.s_addr !=
				    ip->ip_src.s_addr ||
				    inp->inp_fport != uh->uh_sport)
					continue;

			}

			if (last != NULL) {

				n = m_copy(m, 0, M_COPYALL);
				if (n != NULL) {

					if (sbappendaddr(&last->so_rcv,
						(struct sockaddr *)&udp_in,
						n, (struct mbuf *)0) == 0) {

						m_freem(n);
						udpstat.udps_fullsock++;

					}

					else {

						sorwakeup(last);

					}

				}

			}

			last = inp->inp_socket;

			/*
			 * Don't look for additional matches if this one does
			 * not have either the SO_REUSEPORT or SO_REUSEADDR
			 * socket options set.  This heuristic avoids searching
			 * through all pcbs in the common case of a non-shared
			 * port.  It * assumes that an application will never
			 * clear these options after setting them.
			 */
			if ((last->so_options&(SO_REUSEPORT|SO_REUSEADDR) == 0))
				break;

		}

		if (last == NULL) {

			/*
			 * No matching pcb found; discard datagram.
			 * (No need to send an ICMP Port Unreachable
			 * for a broadcast or multicast datgram.)
			 */
			udpstat.udps_noportbcast++;
			goto bad;

		}

		if (sbappendaddr(&last->so_rcv, (struct sockaddr *)&udp_in,
		     m, (struct mbuf *)0) == 0) {

			udpstat.udps_fullsock++;
			goto bad;

		}

		sorwakeup(last);
		return;

	}

	inp = udp_last_inpcb;
	if ( (inp != NULL) &&
	     (inp->inp_laddr.s_addr == ip->ip_dst.s_addr) &&
	     (inp->inp_lport == uh->uh_dport) &&
	     (inp->inp_faddr.s_addr == ip->ip_src.s_addr) &&
	     (inp->inp_fport == uh->uh_sport) )
		goto matchFound;

	udpstat.udps_pcbhashmiss++;

	/*
	 * Locate pcb for datagram. First look for an exact match.
	 */
	inp = in_pcblookuphash(&udbinfo, ip->ip_src, uh->uh_sport,
	    ip->ip_dst, uh->uh_dport, 1);
	if (inp == NULL) {

		udpstat.udps_pcbhashmiss++;
		udpstat.udps_noport++;

		if (m->m_flags & (M_BCAST | M_MCAST)) {

			udpstat.udps_noportbcast++;
			goto bad;

		}

		*ip = save_ip;
		icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PORT, 0, 0);
		return;

	}

	/* Store last pcb */
	udp_last_inpcb = inp;

matchFound:

	/*
	 * Construct sockaddr format source address.
	 * Stuff source address and datagram in user buffer.
	 */
	udp_in.sin_port = uh->uh_sport;
	udp_in.sin_addr = ip->ip_src;
	if (inp->inp_flags & INP_CONTROLOPTS) {

		mp = &opts;

		if (inp->inp_flags & INP_RECVDSTADDR) {

			*mp = udp_saveopt((caddr_t) &ip->ip_dst,
			    sizeof(struct in_addr), IP_RECVDSTADDR);
			if (*mp != NULL)
				mp = &(*mp)->m_next;

		}
#ifdef notyet
		/* options were tossed above */
		if (inp->inp_flags & INP_RECVOPTS) {
			*mp = udp_saveopt((caddr_t) opts_deleted_above,
			    sizeof(struct in_addr), IP_RECVOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
		/* ip_srcroute doesn't do what we want here, need to fix */
		if (inp->inp_flags & INP_RECVRETOPTS) {
			*mp = udp_saveopt((caddr_t) ip_srcroute(),
			    sizeof(struct in_addr), IP_RECVRETOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
#endif
	}

	iphlen += sizeof(struct udphdr);
	m->m_len -= iphlen;
	m->m_pkthdr.len -= iphlen;
	m->m_data += iphlen;
	if (sbappendaddr(&inp->inp_socket->so_rcv, (struct sockaddr *)&udp_in,
	    m, opts) == 0) {

		udpstat.udps_fullsock++;
		goto bad;

	}

	sorwakeup(inp->inp_socket);

	return;
bad:

	m_freem(m);

	if (opts)
		m_freem(opts);

}

/*******************************************************************************
 * upd_saveopt - Create a "control" mbuf containing the specified data
 * with the specified type for presentation with a datagram.
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL struct mbuf* udp_saveopt(char *p, int size, int type)
{
	struct cmsghdr *cp;
	struct mbuf *m;

	if ((m = m_get(M_DONTWAIT, MT_CONTROL, CL_SIZE_128, TRUE)) == NULL)
		return ((struct mbuf *) NULL);

	cp = (struct cmsghdr *) mtod(m, struct cmsghdr *);
	memcpy(CMSG_DATA(cp), p, size);
	size += sizeof(*cp);
	m->m_len = size;
	cp->cmsg_len = size;
	cp->cmsg_level = IPPROTO_IP;
	cp->cmsg_type = type;

	return (m);
}

/*******************************************************************************
 * udp_notify - Notify a udp user of an asynchronous error;
 * just wake up so that he can collect error status.
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void udp_notify(struct inpcb *inp, int errno)
{
	inp->inp_socket->so_error = errno;
	sorwakeup(inp->inp_socket);
	sowwakeup(inp->inp_socket);
}

/*******************************************************************************
 * udp_ctlinput - Control input
 *
 * RETURNS: N/A
 ******************************************************************************/

void udp_ctlinput(int cmd, struct sockaddr *sa, struct ip *ip)
{
	struct udphdr *uh;

	if (!PRC_IS_REDIRECT(cmd) &&
	    ((unsigned)cmd >= PRC_NCMDS || inetctlerrmap[cmd] == 0))
		return;

	if (ip != NULL) {

		uh = (struct udphdr *)((caddr_t)ip + (ip->ip_hl << 2));
		in_pcbnotify(udbinfo.listhead,
			     sa,
			     uh->uh_dport,
			     ip->ip_src,
			     uh->uh_sport,
			     cmd,
			     udp_notify);

	}

	else {

		in_pcbnotify(udbinfo.listhead,
			     sa,
			     0,
			     zeroin_addr,
			     0,
			     cmd,
			     udp_notify);

	}

}

/*******************************************************************************
 * udp_output - Udp datagram output
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int udp_output(struct inpcb *inp,
	       struct mbuf *m,
	       struct mbuf *addr,
	       struct mbuf *control)
{
	struct rtentry *rt;
	struct udpiphdr *ui;
	int len = m->m_pkthdr.len;
	struct in_addr laddr;
	int s = 0, error = 0;

	if (control != NULL)
		m_freem(control);		/* XXX */

	if (addr != NULL) {

		laddr = inp->inp_laddr;

		if (inp->inp_faddr.s_addr != INADDR_ANY) {

			error = EISCONN;
			goto release;

		}

		/*
		 * Must block input while temporarily connected.
		 */
		s = splnet();
		error = in_pcbconnect(inp, addr);
		if (error) {

			splx(s);
			goto release;

		}

	}

	else {

		if (inp->inp_faddr.s_addr == INADDR_ANY) {

			error = ENOTCONN;
			goto release;

		}

	}

	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */
	M_PREPEND(m, sizeof(struct udpiphdr), M_DONTWAIT);
	if (m == NULL) {

		error = ENOBUFS;

		if (addr != NULL) {

			in_pcbdisconnect(inp);
			splx(s);

		}

		goto release;
	}

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	ui = mtod(m, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = NULL;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)len + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;

	/*
	 * Stuff checksum and output datagram.
	 */
	ui->ui_sum = 0;
	if (udpDoCkSumSnd) {

	    if ((ui->ui_sum = in_cksum(m, sizeof (struct udpiphdr) + len)) == 0)
		ui->ui_sum = 0xffff;

	}

	((struct ip *)ui)->ip_len = sizeof (struct udpiphdr) + len;
	((struct ip *)ui)->ip_ttl = inp->inp_ip.ip_ttl;	/* XXX */
	((struct ip *)ui)->ip_tos = inp->inp_ip.ip_tos;	/* XXX */
	udpstat.udps_opackets++;

	rt = inp->inp_route.ro_rt;
	if ( (rt != NULL) &&
	     (rt->rt_flags & RTF_UP) &&
	     ((rt->rt_rmx.rmx_locks & RTV_MTU) == 0) ) {

		if (inp->inp_socket->so_options & SO_USEPATHMTU)
			((struct ip *) ui)->ip_off = IP_DF;

        }

	error = ip_output(m, inp->inp_options, &inp->inp_route,
	    inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST),
	    inp->inp_moptions);

	if (addr != NULL) {

		in_pcbdisconnect(inp);
		inp->inp_laddr = laddr;
		splx(s);

	}

	return (error);

release:

	m_freem(m);

	return (error);
}

/*******************************************************************************
 * udp_usrreq - Upd datagram user request handle
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int udp_usrreq(struct socket *so,
	       int req,
	       struct mbuf *m,
	       struct mbuf *addr,
	       struct mbuf *control)
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;
	int s;

	if (req == PRU_CONTROL)
		return (in_control(so, (u_long)m, (caddr_t)addr,
			(struct ifnet *)control));

	if ( (inp == NULL) &&
	     (req != PRU_ATTACH) ) {

		error = EINVAL;
		goto release;

	}

	/*
	 * Note: need to block udp_input while changing
	 * the udp pcb queue and/or pcb addresses.
	 */
	switch (req) {

	case PRU_ATTACH:

		if (inp != NULL) {

			error = EINVAL;
			break;

		}

		s = splnet();
		error = in_pcballoc(so, &udbinfo);
		splx(s);
		if (error)
			break;

		error = soreserve(so, udp_sendspace, udp_recvspace);
		if (error)
			break;

		((struct inpcb *) so->so_pcb)->inp_ip.ip_ttl = ip_defttl;

	break;

	case PRU_DETACH:

		udp_detach(inp);

	break;

	case PRU_BIND:

		s = splnet();
		error = in_pcbbind(inp, addr);
		splx(s);

	break;

	case PRU_LISTEN:

		error = EOPNOTSUPP;

	break;

	case PRU_CONNECT:

		if (inp->inp_faddr.s_addr != INADDR_ANY) {

			error = EISCONN;
			break;

		}

		s = splnet();
		error = in_pcbconnect(inp, addr);
		splx(s);
		if (error == 0)
			soisconnected(so);

	break;

	case PRU_CONNECT2:

		error = EOPNOTSUPP;

	break;

	case PRU_ACCEPT:

		error = EOPNOTSUPP;

	break;

	case PRU_DISCONNECT:

		if (inp->inp_faddr.s_addr == INADDR_ANY) {

			error = ENOTCONN;
			break;

		}

		s = splnet();
		in_pcbdisconnect(inp);
		inp->inp_laddr.s_addr = INADDR_ANY;
		splx(s);
		so->so_state &= ~SS_ISCONNECTED;		/* XXX */

	break;

	case PRU_SHUTDOWN:

		socantsendmore(so);

	break;

	case PRU_SEND:

		return (udp_output(inp, m, addr, control));

	case PRU_ABORT:

		soisdisconnected(so);
		udp_detach(inp);

	break;

	case PRU_SOCKADDR:

		in_setsockaddr(inp, addr);

	break;

	case PRU_PEERADDR:

		in_setpeeraddr(inp, addr);

	break;

	case PRU_SENSE:

		/*
		 * stat: don't bother with a blocksize.
		 */
		return (0);

	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:

		error =  EOPNOTSUPP;

	break;

	case PRU_RCVD:
	case PRU_RCVOOB:

		return (EOPNOTSUPP);	/* do not free mbuf's */

	default:

		panic("udp_usrreq");

	}

release:

	if (control != NULL) {

		printf("udp control data unexpectedly retained\n");
		m_freem(control);

	}

	if (m != NULL)
		m_freem(m);

	return (error);
}

/*******************************************************************************
 * udp_detach - Detach udp pcb
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void udp_detach(struct inpcb *inp)
{
	int s = splnet();

	if (inp == udp_last_inpcb)
		in_pcbdetach(inp);

	splx(s);
}

