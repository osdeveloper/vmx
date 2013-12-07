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

/* ipLib.c - Ip library */

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *      @(#)ip_output.c 8.3 (Berkeley) 1/21/94
 */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vmx.h>
#include <vmx/tickLib.h>
#include <os/unixLib.h>
#include <net/netLib.h>
#include <net/mbuf.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/socketvar.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>

/* Defines */
#ifndef IPFORWARDING
#ifdef GATEWAY
#define IPFORWARDING    1       /* forward IP packets not for us */
#else /* GATEWAY */
#define IPFORWARDING    0       /* don't forward IP packets not for us */
#endif /* GATEWAY */
#endif /* IPFORWARDING */
#ifndef IPSENDREDIRECTS
#define IPSENDREDIRECTS 1
#endif

/* Imports */
IMPORT struct domain inetdomain;
IMPORT struct protosw inetsw[];
IMPORT struct in_ifaddr *in_ifaddr;
IMPORT int max_linkhdr;
IMPORT struct ifnet *loif;

IMPORT int sysClockRateGet();

/* Locals */
LOCAL int ip_setmoptions(int optname,
                         struct inpcb *pInpcb,
                         struct mbuf *m);
LOCAL struct mbuf* ip_insertoptions(struct mbuf *m,
                                    struct mbuf *opt,
                                    int *phlen);
LOCAL void ip_mloopback(struct ifnet *ifp,
                        struct mbuf *m,
                        struct sockaddr_in *dst);
LOCAL struct mbuf* ipReAssemble(struct mbuf *pMbuf,
                                struct ipq *pIpFragQueue);

/* Globals */
struct ipstat ipstat;
unsigned char ip_protox[IPPROTO_MAX];

int ip_flags = IP_DO_FORWARDING | IP_DO_REDIRECT |
               IP_DO_CHECKSUM_SND | IP_DO_CHECKSUM_RCV;
int ip_defttl = IPDEFTTL;
int ipqmaxlen = IFQ_MAXLEN;
int ipfragttl = IPFRAGTTL;
int ip_nhops = 0;

struct ipq ipq;
unsigned short ip_id;
struct ifqueue ipintrq;
struct sockaddr_in input_ipaddr;
struct route ipforward_rt;

unsigned char inetctlerrmap[PRC_NCMDS] = {
        0,              0,              0,              0,
        0,              EMSGSIZE,       EHOSTDOWN,      EHOSTUNREACH,
        EHOSTUNREACH,   EHOSTUNREACH,   ECONNREFUSED,   ECONNREFUSED,
        EMSGSIZE,       EHOSTUNREACH,   0,              0,
        0,              0,              0,              0,
        ENOPROTOOPT
};

#ifdef DIAGNOSTIC
int     ipprintfs = 0;
#endif

/* Functions */

/*******************************************************************************
 * ipLibInit - Initialize ip library
 *
 * RETURNS: N/A
 ******************************************************************************/

STATUS ipLibInit(IP_CFG_PARAMS *params)
{
  int i;
  struct protosw *pr;

  /* If params set */
  if (params != NULL) {

    ip_flags  = params->ipCfgFlags;
    ip_defttl = params->ipDefTtl;
    ipqmaxlen = params->ipIntrQueueLen;
    ipfragttl = params->ipFragTtl;
    ip_nhops = 0;

  } /* End if params set */

  input_ipaddr.sin_len = sizeof(struct sockaddr_in);
  input_ipaddr.sin_family = AF_INET;

  pr = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
  if (pr == NULL)
    return ERROR;

  /* For all protocols */
  for (i = 0; i < IPPROTO_MAX; i++)
    ip_protox[i] = pr - inetsw;

  /* For all protocols */
  for (pr = inetdomain.dom_protosw;
       pr < inetdomain.dom_nprotosw;
       pr++) {

    if ( (pr->pr_domain != NULL) &&
         (pr->pr_domain->dom_family == PF_INET) &&
         (pr->pr_protocol) &&
         (pr->pr_protocol != IPPROTO_RAW) )
      ip_protox[pr->pr_protocol] = pr - inetsw;

  } /* End for all protocols */

  /* Initialize ip queue */
  ipq.next = &ipq;
  ipq.prev = &ipq;
  ip_id = iptime() & 0xffff;

  /* Initialize ip queue */
  memset(&ipq, 0, sizeof(struct ipq));

  /* Initialize interface queue */
  memset(&ipintrq, 0, sizeof(struct ifqueue));
  ipintrq.ifq_maxlen = ipqmaxlen;

  return OK;
}

/*******************************************************************************
 * iptime - Get time
 *
 * RETURNS: Time
 ******************************************************************************/

n_time iptime(void)
{
  unsigned long t, tim;

  /* Get time */
  tim = tickGet();
  t = tim;
  tim *= 1000;

  if ( t != (tim / 1000) )
    t = ( t / sysClockRateGet() ) * 1000;
  else
    t = tim / sysClockRateGet();

  return ( htonl(t) );
}

/*******************************************************************************
 * IP output.  The packet in mbuf chain m contains a skeletal IP
 * header (with len, off, ttl, proto, tos, src, dst).
 * The mbuf chain containing the packet will be freed.
 * The mbuf opt, if present, will not be freed.
 ******************************************************************************/

int ip_output(struct mbuf *m0,
              struct mbuf *opt,
              struct route *ro,
              int flags,
              struct ip_moptions *imo)
{
        struct ip *ip, *mhip;
        struct ifnet *ifp = NULL;
        struct mbuf *m = m0;
        int hlen = sizeof (struct ip);
        int len, off, error = 0;
        struct route iproute;
        struct sockaddr_in *dst;
        struct in_ifaddr *ia = NULL;
        struct in_multi *inm;
        int mtu = 0;

#ifdef  DIAGNOSTIC
        if ((m->m_flags & M_PKTHDR) == 0)
                panic("ip_output no header");
#endif

        if (opt) {
                m = ip_insertoptions(m, opt, &len);
                hlen = len;
        }

        /* Get pointer to ip data */
        ip = mtod(m, struct ip *);

        /*
         * Fill in IP header.
         */
        if ((flags & (IP_FORWARDING|IP_RAWOUTPUT)) == 0) {

                ip->ip_v = IPVERSION;
                ip->ip_off &= IP_DF;
                ip->ip_id = htons(ip_id++);
                ip->ip_hl = hlen >> 2;
                ipstat.ips_localout++;

        }

        else {

                hlen = ip->ip_hl << 2;
        }

        /*
         * Route packet.
         */
        if (ro == NULL) {

                ro = &iproute;
                memset( ro, 0, sizeof(*ro) );

        }

        /* Get destination */
        dst = (struct sockaddr_in *)&ro->ro_dst;

#ifdef DEBUG_IP
        printf("IP: ");
        inLogSock(dst);
#endif

        /*
         * If there is a cached route,
         * check that it is to the same destination
         * and is still up.  If not, free it and try again.
         */
        if ((ro->ro_rt != NULL) &&
            ((ro->ro_rt->rt_flags & RTF_UP) == 0 ||
            dst->sin_addr.s_addr != ip->ip_dst.s_addr)) {

                RTFREE(ro->ro_rt);
                ro->ro_rt = NULL;

        }

        if (ro->ro_rt == NULL) {

                dst->sin_family = AF_INET;
                dst->sin_len = sizeof(*dst);
                dst->sin_addr = ip->ip_dst;
                TOS_SET(dst, ip->ip_tos);

        }

        /*
         * If routing to interface only,
         * short circuit routing lookup.
         */

#define ifatoia(ifa)    ((struct in_ifaddr *)(ifa))
#define sintosa(sin)    ((struct sockaddr *)(sin))

        if (flags & IP_ROUTETOIF) {

                if (ro->ro_rt == NULL)
                        TOS_SET(dst, 0);

                if ((ia = ifatoia(ifa_ifwithdstaddr(sintosa(dst)))) == NULL &&
                    (ia = ifatoia(ifa_ifwithnet(sintosa(dst)))) == NULL) {

                        ipstat.ips_noroute++;
                        error = ENETUNREACH;
                        goto bad;

                }

                if (ro->ro_rt == NULL)
                        TOS_SET(dst, ip->ip_tos);

                ifp = ia->ia_ifp;
                ip->ip_ttl = 1;

        }

        else if ( !(IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) &&
                     (imo != NULL) &&
                     ((imo->imo_multicast_ifp != NULL))) ) {

                if (ro->ro_rt == NULL)
                        rtalloc(ro);

                if (ro->ro_rt == NULL) {

#ifdef DEBUG_IP
                        printf("FAIL!\n");
#endif
                        ipstat.ips_noroute++;
                        error = EHOSTUNREACH;
                        goto bad;

                }

#ifdef DEBUG_IP
                printf("SUCCESS!\n");
#endif
                ia = ifatoia(ro->ro_rt->rt_ifa);
                ifp = ro->ro_rt->rt_ifp;
                ro->ro_rt->rt_use++;
                if (ro->ro_rt->rt_flags & RTF_GATEWAY)
                        dst = (struct sockaddr_in *)ro->ro_rt->rt_gateway;

        }

        if ( IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) ) {

                m->m_flags |= M_MCAST;

                /*
                 * IP destination address is multicast.  Make sure "dst"
                 * still points to the address in "ro".  (It may have been
                 * changed to point to a gateway address, above.)
                 */

                dst = (struct sockaddr_in *)&ro->ro_dst;

                if ( (imo != NULL) &&
                     (imo->imo_multicast_ifp != NULL) ) {

                  for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next) {

                    if (ia->ia_ifp == ifp) {

                      dst->sin_addr.s_addr = IA_SIN(ia)->sin_addr.s_addr;
                      break;

                    }

                  }

                  if (ro->ro_rt != NULL) {

                    RTFREE(ro->ro_rt);
                    ro->ro_rt = NULL;

                  }

                }

                /*
                 * See if the caller provided any multicast options
                 */
                if (imo != NULL) {

                  ip->ip_ttl = imo->imo_multicast_ttl;

                  if (imo->imo_multicast_ifp != NULL)
                    ifp = imo->imo_multicast_ifp;

                }

                else {

                  ip->ip_ttl = IP_DEFAULT_MULTICAST_TTL;

                }

                /* Confirm that outgoing interface support multicasting */
                if ( (ifp->if_flags & IFF_MULTICAST) == 0 ) {

                  ipstat.ips_noroute++;
                  error = ENETUNREACH;
                  goto bad;

                }

                /*
                 * If source address not specified yet, use address
                 * of outgoing interface.
                 */
                if (ip->ip_src.s_addr == INADDR_ANY) {

                        for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)

                                if (ia->ia_ifp == ifp) {

                                        ip->ip_src = IA_SIN(ia)->sin_addr;
                                        break;

                                }
                }

                IN_LOOKUP_MULTI(ip->ip_dst, ifp, inm);

                if ( (inm != NULL) &&
                     (imo == NULL || imo->imo_multicast_loop) ) {

                        /*
                         * If we belong to the destination multicast group
                         * on the outgoing interface, and the caller did not
                         * forbid loopback, loop back a copy.
                         */
                        ip_mloopback(ifp, m, dst);

                }

#ifdef MROUTING
                else {
                        /*
                         * If we are acting as a multicast router, perform
                         * multicast forwarding as if the packet had just
                         * arrived on the interface to which we are about
                         * to send.  The multicast forwarding function
                         * recursively calls this function, using the
                         * IP_FORWARDING flag to prevent infinite recursion.
                         *
                         * Multicasts that are looped back by ip_mloopback(),
                         * above, will be forwarded by the ip_input() routine,
                         * if necessary.
                         */
                        extern struct socket *ip_mrouter;
                        if (ip_mrouter && (flags & IP_FORWARDING) == 0) {
                                if (ip_mforward(m, ifp) != 0) {
                                        m_freem(m);
                                        goto done;
                                }
                        }
                }
#endif
                /*
                 * Multicasts with a time-to-live of zero may be looped-
                 * back, above, but must not be transmitted on a network.
                 * Also, multicasts addressed to the loopback interface
                 * are not sent -- the above call to ip_mloopback() will
                 * loop back a copy if this host actually belongs to the
                 * destination group on the loopback interface.
                 */
                if (ip->ip_ttl == 0 || ifp == loif) {

                        m_freem(m);
                        goto done;

                }

                /* Store interface mtu */
                mtu = ifp->if_mtu;

                goto sendit;

        }

#ifndef notdef
        /*
         * If source address not specified yet, use address
         * of outgoing interface.
         */
        if (ip->ip_src.s_addr == INADDR_ANY)
                ip->ip_src = IA_SIN(ia)->sin_addr;
#endif

        /* Get mtu size */
        if (ifp->if_type == IFT_PMP) {

        }

        else {

          mtu = ifp->if_mtu;

        }

        /*
         * Look for broadcast address and
         * and verify user is allowed to send
         * such a packet.
         */
        if ( in_broadcast(dst->sin_addr, ifp) ) {

                if ((ifp->if_flags & IFF_BROADCAST) == 0) {

                        error = EADDRNOTAVAIL;
                        goto bad;

                }

                if ((flags & IP_ALLOWBROADCAST) == 0) {

                        error = EACCES;
                        goto bad;

                }

                /* don't allow broadcast messages to be fragmented */
                if ( ((u_short)ip->ip_len > mtu) &&
                     !(ip_flags & IP_DO_LARGE_BCAST) ) {

                        error = EMSGSIZE;
                        goto bad;

                }

                m->m_flags |= M_BCAST;

        }

        else {

                m->m_flags &= ~M_BCAST;

        }

sendit:

        /*
         * If small enough for interface, can just send directly.
         */
        if ( (u_short) ip->ip_len <= ifp->if_mtu) {

                ip->ip_len = htons((u_short)ip->ip_len);
                ip->ip_off = htons((u_short)ip->ip_off);

                ip->ip_sum = 0;
                if (ip_flags & IP_DO_CHECKSUM_SND)
                  ip->ip_sum = in_cksum(m, hlen);

                error = (*ifp->if_output)(ifp, m,
                                (struct sockaddr *)dst, ro->ro_rt);

                goto done;

        }

        /*
         * Too large for interface; fragment if possible.
         * Must be able to put at least 8 bytes per fragment.
         */
        if (ip->ip_off & IP_DF) {

                error = EMSGSIZE;

                if ( (ro->ro_rt != NULL) &&
                     (ro->ro_rt->rt_flags & (RTF_UP | RTF_HOST)) &&
                     !(ro->ro_rt->rt_rmx.rmx_locks & RTV_MTU) &&
                     (ro->ro_rt->rt_rmx.rmx_mtu > mtu) )
                  ro->ro_rt->rt_rmx.rmx_mtu = mtu;

                ipstat.ips_cantfrag++;
                goto bad;

        }

        /* Get length */
        len = (ifp->if_mtu - hlen) &~ 7;
        if (len < 8) {

                error = EMSGSIZE;
                goto bad;

        }

    {
        int mhlen, firstlen = len;
        struct mbuf **mnext = &m->m_nextpkt;

        /*
         * Loop through length of segment after first fragment,
         * make new header and copy data of each part and link onto chain.
         */
        m0 = m;
        mhlen = sizeof (struct ip);
        for (off = hlen + len; off < (u_short) ip->ip_len; off += len) {

                m = m_gethdr(M_DONTWAIT, MT_HEADER, CL_SIZE_128, TRUE);
                if (m == 0) {

                        error = ENOBUFS;
                        ipstat.ips_odropped++;
                        goto sendorfree;

                }

                m->m_flags = m0->m_flags;
                m->m_data += max_linkhdr;

                /* Get pointer to ip data */
                mhip = mtod(m, struct ip *);
                *mhip = *ip;

                if (hlen > sizeof (struct ip)) {

                        mhlen = ip_optcopy(ip, mhip) + sizeof (struct ip);
                        mhip->ip_hl = mhlen >> 2;

                }

                m->m_len = mhlen;
                mhip->ip_off = ((off - hlen) >> 3) + (ip->ip_off & ~IP_MF);

                if (ip->ip_off & IP_MF)
                        mhip->ip_off |= IP_MF;

                if (off + len >= (u_short) ip->ip_len)
                        len = (u_short) ip->ip_len - off;
                else
                        mhip->ip_off |= IP_MF;

                mhip->ip_len = htons((u_short)(len + mhlen));

                m->m_next = m_copy(m0, off, len);
                if (m->m_next == 0) {

                        m_free(m);
                        error = ENOBUFS;        /* ??? */
                        ipstat.ips_odropped++;
                        goto sendorfree;

                }

                m->m_pkthdr.len = mhlen + len;
                m->m_pkthdr.rcvif = (struct ifnet *)0;
                mhip->ip_off = htons((u_short)mhip->ip_off);

                mhip->ip_sum = 0;
                if (ip_flags & IP_DO_CHECKSUM_SND)
                  mhip->ip_sum = in_cksum(m, mhlen);

                *mnext = m;
                mnext = &m->m_nextpkt;
                ipstat.ips_ofragments++;
        }

        /*
         * Update first fragment by trimming what's been copied out
         * and updating header, then send each fragment (in order).
         */
        m = m0;
        m_adj(m, hlen + firstlen - (u_short)ip->ip_len);
        m->m_pkthdr.len = hlen + firstlen;
        ip->ip_len = htons((u_short)m->m_pkthdr.len);
        ip->ip_off = htons((u_short)(ip->ip_off | IP_MF));

        ip->ip_sum = 0;
        if (ip_flags & IP_DO_CHECKSUM_SND)
          ip->ip_sum = in_cksum(m, hlen);

sendorfree:

        for (m = m0; m != NULL; m = m0) {

                m0 = m->m_nextpkt;
                m->m_nextpkt = NULL;

                if (error == 0)
                        error = (*ifp->if_output)(ifp, m,
                            (struct sockaddr *)dst, ro->ro_rt);
                else
                        m_freem(m);

        }

        if (error == 0)
                ipstat.ips_fragmented++;
    }

done:

        if ( (ro == &iproute) &&
             ((flags & IP_ROUTETOIF) == 0) &&
             (ro->ro_rt != NULL) )
                RTFREE(ro->ro_rt);

        return (error);
bad:

        m_freem(m0);

        goto done;
}

/*******************************************************************************
 * Insert IP options into preformed packet.
 * Adjust IP destination as required for IP source routing,
 * as indicated by a non-zero in_addr at the start of the options.
 ******************************************************************************/

LOCAL struct mbuf* ip_insertoptions(struct mbuf *m,
                                    struct mbuf *opt,
                                    int *phlen)
{
        struct ipoption *p = mtod(opt, struct ipoption *);
        struct mbuf *n;
        struct ip *ip = mtod(m, struct ip *);
        unsigned optlen;

        optlen = opt->m_len - sizeof(p->ipopt_dst);
        if (optlen + (u_short)ip->ip_len > IP_MAXPACKET)
                return (m);             /* XXX should fail */
        if (p->ipopt_dst.s_addr)
                ip->ip_dst = p->ipopt_dst;

        n = m_gethdr(M_DONTWAIT, MT_HEADER, CL_SIZE_128, TRUE);
        if (n == 0)
                return (m);

        n->m_pkthdr.len = m->m_pkthdr.len + optlen;
        m->m_len -= sizeof(struct ip);
        m->m_data += sizeof(struct ip);
        n->m_next = m;
        m = n;
        m->m_len = optlen + sizeof(struct ip);
        m->m_data += max_linkhdr;
        memcpy( mtod(m, char *), ip, sizeof(struct ip) );

        ip = mtod(m, struct ip *);
        memcpy( (ip + 1), p->ipopt_list, (unsigned) optlen );
        *phlen = sizeof(struct ip) + optlen;
        ip->ip_len += optlen;
        return (m);
}

/*******************************************************************************
 * Copy options from ip to jp,
 * omitting those not copied during fragmentation.
 ******************************************************************************/

int ip_optcopy(struct ip *ip, struct ip *jp)
{
        u_char *cp, *dp;
        int opt, optlen, cnt;

        cp = (u_char *)(ip + 1);
        dp = (u_char *)(jp + 1);
        cnt = (ip->ip_hl << 2) - sizeof (struct ip);
        for (; cnt > 0; cnt -= optlen, cp += optlen) {
                opt = cp[0];
                if (opt == IPOPT_EOL)
                        break;
                if (opt == IPOPT_NOP) {
                        /* Preserve for IP mcast tunnel's LSRR alignment. */
                        *dp++ = IPOPT_NOP;
                        optlen = 1;
                        continue;
                } else
                        optlen = cp[IPOPT_OLEN];
                /* bogus lengths should have been caught by ip_dooptions */
                if (optlen > cnt)
                        optlen = cnt;
                if (IPOPT_COPIED(opt)) {
                        memcpy(dp, cp, (unsigned) optlen);
                        dp += optlen;
                }
        }
        for (optlen = dp - (u_char *)(jp+1); optlen & 0x3; optlen++)
                *dp++ = IPOPT_EOL;
        return (optlen);
}

/*******************************************************************************
 * IP socket option processing.
 ******************************************************************************/

int ip_ctloutput(int op,
                 struct socket *so,
                 int level,
                 int optname,
                 struct mbuf **mp)
{
        struct inpcb *inp = sotoinpcb(so);
        struct mbuf *m = *mp;
        int optval;
        int error = 0;

        if (level != IPPROTO_IP) {
                error = EINVAL;
                if (op == PRCO_SETOPT && *mp)
                        (void) m_free(*mp);
        } else switch (op) {

        case PRCO_SETOPT:
                switch (optname) {
                case IP_OPTIONS:
#ifdef notyet
                case IP_RETOPTS:
                        return (ip_pcbopts(optname, &inp->inp_options, m));
#else
                        return (ip_pcbopts(&inp->inp_options, m));
#endif

                case IP_TOS:
                case IP_TTL:
                case IP_RECVOPTS:
                case IP_RECVRETOPTS:
                case IP_RECVDSTADDR:
                        if (m->m_len != sizeof(int))
                                error = EINVAL;
                        else {
                                optval = *mtod(m, int *);
                                switch (optname) {

                                case IP_TOS:
                                        inp->inp_ip.ip_tos = optval;
                                        break;

                                case IP_TTL:
                                        inp->inp_ip.ip_ttl = optval;
                                        break;
#define OPTSET(bit) \
        if (optval) \
                inp->inp_flags |= bit; \
        else \
                inp->inp_flags &= ~bit;

                                case IP_RECVOPTS:
                                        OPTSET(INP_RECVOPTS);
                                        break;

                                case IP_RECVRETOPTS:
                                        OPTSET(INP_RECVRETOPTS);
                                        break;

                                case IP_RECVDSTADDR:
                                        OPTSET(INP_RECVDSTADDR);
                                        break;
                                }
                        }
                        break;
#undef OPTSET

                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_setmoptions(optname, inp, m);
                        break;

                default:
                        error = ENOPROTOOPT;
                        break;
                }
                if (m)
                        (void)m_free(m);
                break;

        case PRCO_GETOPT:
                switch (optname) {
                case IP_OPTIONS:
                case IP_RETOPTS:
                        *mp = m = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
                        if (inp->inp_options) {
                                m->m_len = inp->inp_options->m_len;
                                memcpy( mtod(m, char *),
                                        mtod(inp->inp_options, char *),
                                        (unsigned) m->m_len );
                        } else
                                m->m_len = 0;
                        break;

                case IP_TOS:
                case IP_TTL:
                case IP_RECVOPTS:
                case IP_RECVRETOPTS:
                case IP_RECVDSTADDR:
                        *mp = m = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
                        m->m_len = sizeof(int);
                        switch (optname) {

                        case IP_TOS:
                                optval = inp->inp_ip.ip_tos;
                                break;

                        case IP_TTL:
                                optval = inp->inp_ip.ip_ttl;
                                break;

#define OPTBIT(bit)     (inp->inp_flags & bit ? 1 : 0)

                        case IP_RECVOPTS:
                                optval = OPTBIT(INP_RECVOPTS);
                                break;

                        case IP_RECVRETOPTS:
                                optval = OPTBIT(INP_RECVRETOPTS);
                                break;

                        case IP_RECVDSTADDR:
                                optval = OPTBIT(INP_RECVDSTADDR);
                                break;
                        }
                        *mtod(m, int *) = optval;
                        break;

                case IP_MULTICAST_IF:
                case IP_MULTICAST_TTL:
                case IP_MULTICAST_LOOP:
                case IP_ADD_MEMBERSHIP:
                case IP_DROP_MEMBERSHIP:
                        error = ip_getmoptions(optname, inp->inp_moptions, mp);
                        break;

                default:
                        error = ENOPROTOOPT;
                        break;
                }
                break;
        }
        return (error);
}

/*******************************************************************************
 * Set up IP options in pcb for insertion in output packets.
 * Store in mbuf with pointer in pcbopt, adding pseudo-option
 * with destination address if source routed.
 ******************************************************************************/

int ip_pcbopts(struct mbuf **pcbopt, struct mbuf *m)
{
        int cnt, optlen;
        u_char *cp;
        u_char opt;

        /* turn off any old options */
        if (*pcbopt)
                (void)m_free(*pcbopt);
        *pcbopt = 0;
        if (m == (struct mbuf *)0 || m->m_len == 0) {
                /*
                 * Only turning off any previous options.
                 */
                if (m)
                        (void)m_free(m);
                return (0);
        }

        if (m->m_len % sizeof(long))
                goto bad;
        /*
         * IP first-hop destination address will be stored before
         * actual options; move other options back
         * and clear it when none present.
         */
        if (m->m_data + m->m_len + sizeof(struct in_addr) >=
            (m->m_extBuf + m->m_extSize))
          goto bad;

        cnt = m->m_len;
        m->m_len += sizeof(struct in_addr);
        cp = mtod(m, u_char *) + sizeof(struct in_addr);
        memcpy(cp, mtod(m, char *), (unsigned) cnt );
        memset( mtod(m, char *), 0, sizeof(struct in_addr) );

        for (; cnt > 0; cnt -= optlen, cp += optlen) {
                opt = cp[IPOPT_OPTVAL];
                if (opt == IPOPT_EOL)
                        break;
                if (opt == IPOPT_NOP)
                        optlen = 1;
                else {
                        optlen = cp[IPOPT_OLEN];
                        if (optlen <= IPOPT_OLEN || optlen > cnt)
                                goto bad;
                }
                switch (opt) {

                default:
                        break;

                case IPOPT_LSRR:
                case IPOPT_SSRR:
                        /*
                         * user process specifies route as:
                         *      ->A->B->C->D
                         * D must be our final destination (but we can't
                         * check that since we may not have connected yet).
                         * A is first hop destination, which doesn't appear in
                         * actual IP option, but is stored before the options.
                         */
                        if (optlen < IPOPT_MINOFF - 1 + sizeof(struct in_addr))
                                goto bad;
                        m->m_len -= sizeof(struct in_addr);
                        cnt -= sizeof(struct in_addr);
                        optlen -= sizeof(struct in_addr);
                        cp[IPOPT_OLEN] = optlen;
                        /*
                         * Move first hop before start of options.
                         */
                        memcpy( mtod(m, char *),
                                &cp[IPOPT_OFFSET + 1],
                                sizeof(struct in_addr) );
                        /*
                         * Then copy rest of options back
                         * to close up the deleted entry.
                         */
                        memcpy(&cp[IPOPT_OFFSET + 1],
                               (&cp[IPOPT_OFFSET + 1] + sizeof(struct in_addr)),
                               (unsigned) cnt + sizeof(struct in_addr) );
                        break;
                }
        }
        if (m->m_len > MAX_IPOPTLEN + sizeof(struct in_addr))
                goto bad;
        *pcbopt = m;
        return (0);

bad:
        (void)m_free(m);
        return (EINVAL);
}

/*******************************************************************************
 * Set the IP multicast options in response to user setsockopt().
 ******************************************************************************/

LOCAL int ip_setmoptions(int optname, struct inpcb *pInpcb, struct mbuf *m)
{
        int error = 0;
        u_char loop;
        int i;
        struct in_addr addr;
        struct ip_mreq *mreq;
        struct ifnet *ifp = NULL;
        struct ip_moptions **imop = &pInpcb->inp_moptions;
        struct ip_moptions *imo = *imop;
        struct route ro;
        struct sockaddr_in *dst;

        if (imo == NULL) {

                /*
                 * No multicast option buffer attached to the pcb;
                 * allocate one and initialize to default values.
                 */
                imo = (struct ip_moptions *)
                        mb_alloc(sizeof(*imo), MT_IPMOPTS, M_WAIT);

                if (imo == NULL)
                        return (ENOBUFS);

                *imop = imo;
                imo->imo_multicast_ifp = NULL;
                imo->imo_multicast_ttl = IP_DEFAULT_MULTICAST_TTL;
                imo->imo_multicast_loop = IP_DEFAULT_MULTICAST_LOOP;
                imo->imo_num_memberships = 0;
                imo->imo_mbuf = NULL;
        }

        switch (optname) {

        case IP_MULTICAST_IF:
                /*
                 * Select the interface for outgoing multicast packets.
                 */
                if (m == NULL || m->m_len != sizeof(struct in_addr)) {

                        error = EINVAL;
                        break;

                }

                addr = *(mtod(m, struct in_addr *));
                /*
                 * INADDR_ANY is used to remove a previous selection.
                 * When no interface is selected, a default one is
                 * chosen every time a multicast packet is sent.
                 */

                if (addr.s_addr == INADDR_ANY) {

                        imo->imo_multicast_ifp = NULL;
                        break;

                }

                /*
                 * The selected interface is identified by its local
                 * IP address.  Find the interface and confirm that
                 * it supports multicasting.
                 */
                INADDR_TO_IFP(addr, ifp);
                if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {

                        error = EADDRNOTAVAIL;
                        break;

                }

                imo->imo_multicast_ifp = ifp;
                break;

        case IP_MULTICAST_TTL:
                /*
                 * Set the IP time-to-live for outgoing multicast packets.
                 */
                if (m == NULL || m->m_len != 1) {

                        error = EINVAL;
                        break;

                }

                imo->imo_multicast_ttl = *(mtod(m, u_char *));
                break;

        case IP_MULTICAST_LOOP:
                /*
                 * Set the loopback flag for outgoing multicast packets.
                 * Must be zero or one.
                 */
                if (m == NULL || m->m_len != 1 ||
                   (loop = *(mtod(m, u_char *))) > 1) {

                        error = EINVAL;
                        break;

                }

                imo->imo_multicast_loop = loop;
                break;

        case IP_ADD_MEMBERSHIP:
                /*
                 * Add a multicast group membership.
                 * Group must be a valid IP multicast address.
                 */
                if (m == NULL || m->m_len != sizeof(struct ip_mreq)) {

                        error = EINVAL;
                        break;

                }

                mreq = mtod(m, struct ip_mreq *);
                if (!IN_MULTICAST(ntohl(mreq->imr_multiaddr.s_addr))) {

                        error = EINVAL;
                        break;

                }

                /*
                 * If no interface address was provided, use the interface of
                 * the route to the given multicast address.
                 */
                if (mreq->imr_interface.s_addr == INADDR_ANY) {

                        ro.ro_rt = NULL;
                        dst = (struct sockaddr_in *)&ro.ro_dst;
                        dst->sin_len = sizeof(*dst);
                        dst->sin_family = AF_INET;
                        dst->sin_addr = mreq->imr_multiaddr;
                        rtalloc(&ro);
                        if (ro.ro_rt == NULL) {

                                error = EADDRNOTAVAIL;
                                break;

                        }

                        ifp = ro.ro_rt->rt_ifp;
                        rtfree(ro.ro_rt);
                }

                else {

                        INADDR_TO_IFP(mreq->imr_interface, ifp);

                }

                /*
                 * See if we found an interface, and confirm that it
                 * supports multicast.
                 */
                if (ifp == NULL || (ifp->if_flags & IFF_MULTICAST) == 0) {

                        error = EADDRNOTAVAIL;
                        break;

                }

                /*
                 * See if the membership already exists or if all the
                 * membership slots are full.
                 */

#ifdef NOTE_FIX_ME

                for (i = 0; i < imo->imo_num_memberships; ++i) {
                        if (imo->imo_membership[i]->inm_ifp == ifp &&
                            imo->imo_membership[i]->inm_addr.s_addr
                                                == mreq->imr_multiaddr.s_addr)
                                break;
                }
                if (i < imo->imo_num_memberships) {
                        error = EADDRINUSE;
                        break;
                }
                if (i == IP_MAX_MEMBERSHIPS) {
                        error = ETOOMANYREFS;
                        break;
                }
                /*
                 * Everything looks good; add a new record to the multicast
                 * address list for the given interface.
                 */
                if ((imo->imo_membership[i] =
                    in_addmulti(&mreq->imr_multiaddr, ifp)) == NULL) {
                        error = ENOBUFS;
                        break;
                }

#endif /* NOTE_FIX_ME */

                ++imo->imo_num_memberships;
                break;

        case IP_DROP_MEMBERSHIP:
                /*
                 * Drop a multicast group membership.
                 * Group must be a valid IP multicast address.
                 */
                if (m == NULL || m->m_len != sizeof(struct ip_mreq)) {

                        error = EINVAL;
                        break;

                }

                mreq = mtod(m, struct ip_mreq *);
                if (!IN_MULTICAST(ntohl(mreq->imr_multiaddr.s_addr))) {

                        error = EINVAL;
                        break;

                }

                /*
                 * If an interface address was specified, get a pointer
                 * to its ifnet structure.
                 */
                if (mreq->imr_interface.s_addr == INADDR_ANY) {

                        ifp = NULL;

                }

                else {

                        INADDR_TO_IFP(mreq->imr_interface, ifp);
                        if (ifp == NULL) {

                                error = EADDRNOTAVAIL;
                                break;

                        }

                }

                /*
                 * Find the membership in the membership array.
                 */

#ifdef NOTE_FIX_ME

                for (i = 0; i < imo->imo_num_memberships; ++i) {
                        if ((ifp == NULL ||
                             imo->imo_membership[i]->inm_ifp == ifp) &&
                             imo->imo_membership[i]->inm_addr.s_addr ==
                             mreq->imr_multiaddr.s_addr)
                                break;
                }
                if (i == imo->imo_num_memberships) {
                        error = EADDRNOTAVAIL;
                        break;
                }
                /*
                 * Give up the multicast address record to which the
                 * membership points.
                 */
                in_delmulti(imo->imo_membership[i]);
                /*
                 * Remove the gap in the membership array.
                 */
                for (++i; i < imo->imo_num_memberships; ++i)
                        imo->imo_membership[i-1] = imo->imo_membership[i];
#endif /* NOTE_FIX_ME */

                --imo->imo_num_memberships;
                break;

        default:
                error = EOPNOTSUPP;
                break;

        }

        /*
         * If all options have default values, no need to keep the mbuf.
         */
        if (imo->imo_multicast_ifp == NULL &&
            imo->imo_multicast_ttl == IP_DEFAULT_MULTICAST_TTL &&
            imo->imo_multicast_loop == IP_DEFAULT_MULTICAST_LOOP &&
            imo->imo_num_memberships == 0) {

                mb_free(*imop);
                *imop = NULL;

        }

        return (error);
}

/*******************************************************************************
 * Return the IP multicast options in response to user getsockopt().
 ******************************************************************************/

int ip_getmoptions(int optname, struct ip_moptions *imo, struct mbuf **mp)
{
        u_char *ttl;
        u_char *loop;
        struct in_addr *addr;
        struct in_ifaddr *ia;

        *mp = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);

        switch (optname) {

        case IP_MULTICAST_IF:
                addr = mtod(*mp, struct in_addr *);
                (*mp)->m_len = sizeof(struct in_addr);
                if (imo == NULL || imo->imo_multicast_ifp == NULL)
                        addr->s_addr = INADDR_ANY;
                else {
                        IFP_TO_IA(imo->imo_multicast_ifp, ia);
                        addr->s_addr = (ia == NULL) ? INADDR_ANY
                                        : IA_SIN(ia)->sin_addr.s_addr;
                }
                return (0);

        case IP_MULTICAST_TTL:
                ttl = mtod(*mp, u_char *);
                (*mp)->m_len = 1;
                *ttl = (imo == NULL) ? IP_DEFAULT_MULTICAST_TTL
                                     : imo->imo_multicast_ttl;
                return (0);

        case IP_MULTICAST_LOOP:
                loop = mtod(*mp, u_char *);
                (*mp)->m_len = 1;
                *loop = (imo == NULL) ? IP_DEFAULT_MULTICAST_LOOP
                                      : imo->imo_multicast_loop;
                return (0);

        default:
                return (EOPNOTSUPP);
        }
}

/*******************************************************************************
 * Discard the IP multicast options.
 ******************************************************************************/

void
ip_freemoptions(struct ip_moptions *imo)
{

#ifdef NOTE_FIX_ME

        int i;

        if (imo != NULL) {
                for (i = 0; i < imo->imo_num_memberships; ++i)
                        in_delmulti(imo->imo_membership[i]);
                free(imo, M_IPMOPTS);
        }

#endif /* NOTE_FIX_ME */

}

/*******************************************************************************
 * Routine called from ip_output() to loop back a copy of an IP multicast
 * packet to the input queue of a specified interface.  Note that this
 * calls the output routine of the loopback "driver", but with an interface
 * pointer that might NOT be &loif -- easier than replicating that code here.
 ******************************************************************************/

LOCAL void ip_mloopback(struct ifnet *ifp,
                        struct mbuf *m,
                        struct sockaddr_in *dst)
{
        struct ip *ip;
        struct mbuf *copym;

        copym = m_copy(m, 0, M_COPYALL);
        if (copym != NULL) {
                /*
                 * We don't bother to fragment if the IP length is greater
                 * than the interface's MTU.  Can this possibly matter?
                 */
                ip = mtod(copym, struct ip *);
                ip->ip_len = htons((u_short)ip->ip_len);
                ip->ip_off = htons((u_short)ip->ip_off);

                ip->ip_sum = 0;
                if (ip_flags & IP_DO_CHECKSUM_SND)
                  ip->ip_sum = in_cksum(copym, ip->ip_hl << 2);

                looutput(ifp, copym, (struct sockaddr *)dst, NULL);
        }
}

/*******************************************************************************
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 ******************************************************************************/

void ipintr(struct mbuf *m)
{
        struct ip *ip;
        struct ipq *fp;
        struct in_ifaddr *ia;
        int hlen, s;
        unsigned long t;
        struct in_multi *inm;

        /* If mbuf null */
        if (m == NULL)
          return;

        /* Get processor level */
        s = splnet();

#ifdef  DIAGNOSTIC
        if ((m->m_flags & M_PKTHDR) == 0)
                panic("ipintr no header");
#endif
        /*
         * If no IP addresses have been set yet but the interfaces
         * are receiving, can't do anything with incoming packets yet.
         */
        if (in_ifaddr == NULL)
                goto bad;

        ipstat.ips_total++;

        if (m->m_len < sizeof (struct ip) &&
            (m = m_pullup(m, sizeof (struct ip))) == NULL) {

                ipstat.ips_toosmall++;
                goto done;

        }

        /* Get Pointer to data */
        ip = mtod(m, struct ip *);

        if (ip->ip_v != IPVERSION) {

                ipstat.ips_badvers++;
                goto bad;

        }

        /* Get header length */
        hlen = ip->ip_hl << 2;
        if (hlen < sizeof(struct ip)) { /* minimum header length */

                ipstat.ips_badhlen++;
                goto bad;

        }

        if (hlen > m->m_len) {

                if ((m = m_pullup(m, hlen)) == NULL) {

                        ipstat.ips_badhlen++;
                        goto done;

                }

                /* Get pointer to ip data */
                ip = mtod(m, struct ip *);

        }

        /* If do receive checksum */
        if (ip_flags & IP_DO_CHECKSUM_RCV) {

          /* Do checksum for packet */
          if (ip->ip_sum = in_cksum(m, hlen)) {

            ipstat.ips_badsum++;
            goto bad;

          }

        } /* End if do receive checksum */

        /*
         * Convert fields to host representation.
         */
        NTOHS(ip->ip_len);
        if (ip->ip_len < hlen) {

                ipstat.ips_badlen++;
                goto bad;

        }

        NTOHS(ip->ip_id);
        NTOHS(ip->ip_off);

        /*
         * Check that the amount of data in the buffers
         * is as at least much as the IP header would have us expect.
         * Trim mbufs if longer than we expect.
         * Drop packet if shorter than we expect.
         */
        if (m->m_pkthdr.len < ip->ip_len) {

                ipstat.ips_tooshort++;
                goto bad;

        }

        if (m->m_pkthdr.len > ip->ip_len) {

                if (m->m_len == m->m_pkthdr.len) {

                        m->m_len = ip->ip_len;
                        m->m_pkthdr.len = ip->ip_len;

                }

                else {
        
                        m_adj(m, ip->ip_len - m->m_pkthdr.len);

                }

        }

        /*
         * Process options and, if not destined for us,
         * ship it on.  ip_dooptions returns 1 when an
         * error was detected (causing an icmp message
         * to be sent and the original packet to be freed).
         */
        ip_nhops = 0;           /* for source routed packets */
        if (hlen > sizeof (struct ip) &&
            ip_dooptions(m))
                goto done;

        /*
         * Check our list of addresses, to see if the packet is for us.
         */
        for (ia = in_ifaddr; ia; ia = ia->ia_next) {

#define satosin(sa)     ((struct sockaddr_in *)(sa))

                if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr) {

                  if ( ((ip_flags & IP_DO_IF_STATUS_CHECK) == 0) ||
                       (ia->ia_ifp->if_flags & IFF_UP) )
                    goto ours;
                  else
                    goto bad;

                }

                if (
#ifdef  DIRECTED_BROADCAST
                    ia->ia_ifp == m->m_pkthdr.rcvif &&
#endif
                    (ia->ia_ifp->if_flags & IFF_BROADCAST)) {

                        if (satosin(&ia->ia_broadaddr)->sin_addr.s_addr ==
                            ip->ip_dst.s_addr)
                                goto ours;
                        if (ip->ip_dst.s_addr == ia->ia_netbroadcast.s_addr)
                                goto ours;

                        /*
                         * Look for all-0's host part (old broadcast addr),
                         * either for subnet or net.
                         */
                        t = ntohl(ip->ip_dst.s_addr);
                        if (t == ia->ia_subnet)
                                goto ours;

                        if (t == ia->ia_net)
                                goto ours;

                }

        }

        if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) {
#ifdef MROUTING
                extern struct socket *ip_mrouter;

                if (ip_mrouter) {
                        /*
                         * If we are acting as a multicast router, all
                         * incoming multicast packets are passed to the
                         * kernel-level multicast forwarding function.
                         * The packet is returned (relatively) intact; if
                         * ip_mforward() returns a non-zero value, the packet
                         * must be discarded, else it may be accepted below.
                         *
                         * (The IP ident field is put in the same byte order
                         * as expected when ip_mforward() is called from
                         * ip_output().)
                         */
                        ip->ip_id = htons(ip->ip_id);
                        if (ip_mforward(m, m->m_pkthdr.rcvif) != 0) {
                                ipstat.ips_cantforward++;
                                m_freem(m);
                                goto next;
                        }
                        ip->ip_id = ntohs(ip->ip_id);

                        /*
                         * The process-level routing demon needs to receive
                         * all multicast IGMP packets, whether or not this
                         * host belongs to their destination groups.
                         */
                        if (ip->ip_p == IPPROTO_IGMP)
                                goto ours;
                        ipstat.ips_forward++;
                }
#endif
                /*
                 * See if we belong to the destination multicast group on the
                 * arrival interface.
                 */
                IN_LOOKUP_MULTI(ip->ip_dst, m->m_pkthdr.rcvif, inm);
                if (inm == NULL) {

                        ipstat.ips_cantforward++;
                        m_freem(m);
                        goto done;

                }

                goto ours;

        }

        if (ip->ip_dst.s_addr == (u_long)INADDR_BROADCAST)
                goto ours;

        if (ip->ip_dst.s_addr == INADDR_ANY)
                goto ours;

        /*
         * Not for us; forward if possible and desirable.
         */
        if ( ((ip_flags & IP_DO_FORWARDING) == 0) &&
             ((m->m_pkthdr.rcvif->if_flags & IFF_DONT_FORWARD) != 0) ){

            ipstat.ips_cantforward++;
            m_freem(m);

        }

        else {

                ip_forward(m, 0);

        }

        goto done;

ours:
        /*
         * If offset or IP_MF are set, must reassemble.
         * Otherwise, nothing need be done.
         * (We could look in the reassembly queue to see
         * if the packet was previously fragmented,
         * but it's not worth the time; just let them time out.)
         */
        if (ip->ip_off &~ IP_DF) {

                /*
                 * Look for queue of fragments
                 * of this datagram.
                 */
                for (fp = ipq.next; fp != &ipq; fp = fp->next)
                        if (ip->ip_id == fp->ipq_id &&
                            ip->ip_src.s_addr == fp->ipq_src.s_addr &&
                            ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
                            ip->ip_p == fp->ipq_p)
                                goto found;
                fp = NULL;
found:

                /*
                 * Adjust ip_len to not reflect header,
                 * set ip_mff if more fragments are expected,
                 * convert offset of this to bytes.
                 */
                ip->ip_len -= hlen;
                ((struct ipasfrag *)ip)->ipf_mff &= ~1;

                if (ip->ip_off & IP_MF)
                        ((struct ipasfrag *)ip)->ipf_mff |= 1;

                ip->ip_off <<= 3;

                /*
                 * If datagram marked as having more fragments
                 * or if this is not the first fragment,
                 * attempt reassembly; if it succeeds, proceed.
                 */
                if (((struct ipasfrag *)ip)->ipf_mff & 1 ||
                    ip->ip_off) {

                        ipstat.ips_fragments++;

                        m = ipReAssemble(m, fp);
                        if (m == NULL)
                          goto done;

                        ip = mtod(m, struct ip *);
                        hlen = ip->ip_hl << 2;

                }

                else {
                        if (fp)
                                ip_freef(fp);

                }

        }

        else {

                ip->ip_len -= hlen;

        }

        /*
         * Switch out to protocol's input routine.
         */
        ipstat.ips_delivered++;
        (*inetsw[ip_protox[ip->ip_p]].pr_input)(m, hlen);

        /* Restore processor level and return */
        splx(s);

        return;

bad:
        m_freem(m);

done:
        /* Restore processor level */
        splx(s);
}

/*******************************************************************************
 * Take incoming datagram fragment and try to
 * reassemble it into whole datagram.  If a chain for
 * reassembly of this datagram already exists, then it
 * is given as fp; otherwise have to make a chain.
 ******************************************************************************/

LOCAL struct mbuf* ipReAssemble(struct mbuf *pMbuf,
                                struct ipq *pIpFragQueue)
{
  struct mbuf **pPtrMbuf;
  struct mbuf *pMbPktFrag, *pMbufTmp;
  struct ip *pIpHeader;
  struct ipasfrag *pIpHeaderFrag;
  int len;

  /* Initialize locals */
  pMbPktFrag = NULL;
  pIpHeaderFrag = NULL;
  pIpHeader = mtod(pMbuf, struct ip *);

  /* Set next packet as null */
  pMbuf->m_nextpkt = NULL;

  /* If buffer not allocated */
  if (pIpFragQueue == NULL) {

    pMbufTmp = m_get(M_DONTWAIT, MT_FTABLE, sizeof(struct ipq), TRUE);
    if (pMbufTmp == NULL)
      goto drop;

    /* Get pointer to data area */
    pIpFragQueue = mtod(pMbufTmp, struct ipq *);

    /* Insert into queue */
    insque(pIpFragQueue, &ipq);

    /* Setup struct */
    pIpFragQueue->ipq_ttl = ipfragttl;
    pIpFragQueue->ipq_p = pIpHeader->ip_p;
    pIpFragQueue->ipq_id = pIpHeader->ip_id;
    pIpFragQueue->ipq_src = ((struct ip *) pIpHeader)->ip_src;
    pIpFragQueue->ipq_dst = ((struct ip *) pIpHeader)->ip_dst;
    pIpFragQueue->ipq_hdr = pMbufTmp;
    pIpFragQueue->ipq_pkt = pMbuf;

    goto check;

  } /* End if buffer not allocated */

  /* For all mbuf packets */
  for (pPtrMbuf = &(pIpFragQueue->ipq_pkt);
       pPtrMbuf != NULL;
       pPtrMbuf = &(*pPtrMbuf)->m_nextpkt) {

    pMbPktFrag = *pPtrMbuf;
    pIpHeaderFrag = mtod(pMbPktFrag, struct ipasfrag *);

    /* If header ip offset gt. frag header offset */
    if ( (unsigned short) pIpHeader->ip_off >
         (unsigned short) pIpHeaderFrag->ip_off ) {

      /* Get length */
      len = (int) ((unsigned short) pIpHeaderFrag->ip_off +
                   pIpHeaderFrag->ip_len -
                   (unsigned short) pIpHeader->ip_off);

      /* If length gt. zero */
      if (len > 0) {

        /* If length to big */
        if (len >= pIpHeader->ip_len)
          goto drop;

        /* Decrease fragment length */
        pIpHeaderFrag->ip_len -= len;
        m_adj(pMbPktFrag, -len);

      } /* End if length gt. zero */

      /* If null next packet */
      if (pMbPktFrag->m_nextpkt == NULL) {

        pMbPktFrag->m_nextpkt = pMbuf;
        pMbuf->m_nextpkt = NULL;
        break;

      } /* End if null next packet */

    } /* End if header ip offset gt. frag header offset */

    /* Else header ip offset le. frag header offset */
    else {

      /* While full overlap */
      while (((unsigned short) pIpHeader->ip_off + pIpHeader->ip_len) >=
             ((unsigned short) pIpHeaderFrag->ip_off + pIpHeaderFrag->ip_len)) {

        *pPtrMbuf = (*pPtrMbuf)->m_nextpkt;
        pMbPktFrag->m_nextpkt = NULL;

        m_freem(pMbPktFrag);
        if (pMbPktFrag == NULL)
          break;

        /* Get pointer to data area */
        pIpHeaderFrag = mtod(pMbPktFrag, struct ipasfrag *);

      } /* End while full overlap */

      /* If packet fragment non-null */
      if (pMbPktFrag != NULL) {

        /* Get length */
        len = (int) (((unsigned short) pIpHeader->ip_off + pIpHeader->ip_len) -
                     ((unsigned short) pIpHeaderFrag->ip_off));

        /* If partial overlap */
        if (len > 0) {

          pIpHeader->ip_len -= len;
          m_adj(pMbuf, -len);

        } /* End if partial overlap */

      } /* End if packet fragment non-null */

      pMbuf->m_nextpkt = pMbPktFrag;
      *pPtrMbuf = pMbuf;
      break;

    } /* End else header ip offset le. frag header offset */

  } /* End for all mbuf packets */

check:

  len = 0;

  /* For all packets */
  for (pMbPktFrag = pIpFragQueue->ipq_pkt;
       pMbPktFrag != NULL;
       pMbPktFrag = pMbPktFrag->m_nextpkt) {

    /* Get pointer to data */
    pIpHeaderFrag = mtod(pMbPktFrag, struct ipasfrag *);

    /* If invalid */
    if ( (unsigned short) pIpHeaderFrag->ip_off != len )
      return NULL;

    /* Advace */
    len += pIpHeaderFrag->ip_len;

  } /* End for all packets */

  /* If mff bit still set */
  if (pIpHeaderFrag->ipf_mff & 1)
    return NULL;

  /* Assemble all fragments */
  pMbuf = pIpFragQueue->ipq_pkt;
  pMbufTmp = pMbuf->m_nextpkt;
  pMbuf->m_nextpkt = NULL;

  /* While more fragments */
  while (pMbufTmp != NULL) {

    /* Get current */
    pMbPktFrag = pMbufTmp;

    /* Get pointer to data */
    pIpHeaderFrag = mtod(pMbPktFrag, struct ipasfrag *);

    /* Assemble */
    pMbPktFrag->m_data += pIpHeaderFrag->ip_hl << 2;
    pMbPktFrag->m_len -= pIpHeaderFrag->ip_hl << 2;

    /* Get next */
    pMbufTmp = pMbPktFrag->m_nextpkt;

    /* Set terminate */
    pMbPktFrag->m_nextpkt = NULL;

   /* Concatenate */
   m_cat(pMbuf, pMbPktFrag);

  } /* End while more fragments */

  /* Get pointer to data */
  pIpHeaderFrag = mtod(pMbuf, struct ipasfrag *);

  /* Set length */
  pIpHeaderFrag->ip_len = len;
  if ( len > 0xffff - (pIpHeaderFrag->ip_hl << 2) )
    goto drop;

  /* Reset mff bit */
  pIpHeaderFrag->ipf_mff &= ~1;

  /* Remove from queue */
  remque(pIpFragQueue);

  /* Free */
  m_free(pIpFragQueue->ipq_hdr);

  /* If packet header */
  if (pMbuf->m_flags & M_PKTHDR) {

    /* Adjust header length */
    len = 0;
    for (pMbufTmp = pMbuf; pMbufTmp != NULL; pMbufTmp = pMbufTmp->m_next)
      len += pMbufTmp->m_len;

    pMbuf->m_pkthdr.len = len;

  } /* End if packet header */

  return pMbuf;

drop:

  /* Update status */
  ipstat.ips_fragdropped--;

  /* Free */
  m_freem(pMbuf);

  return NULL;
}

/*******************************************************************************
 * Free a fragment reassembly header and all
 * associated datagrams.
 ******************************************************************************/

void ip_freef(struct ipq *fp)
{
  struct mbuf **pPtrMbuf;
  struct mbuf *pMbuf;

  /* Initialize locals */
  pPtrMbuf = &(fp->ipq_pkt);

  /* While non-null */
  while (*pPtrMbuf != NULL) {

    pMbuf = (*pPtrMbuf)->m_nextpkt;
    m_freem(*pPtrMbuf);
    *pPtrMbuf = pMbuf;

  } /* End while non-null */

  /* Remove from queue and free */
  remque(fp);
  m_free(fp->ipq_hdr);
}

/*******************************************************************************
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 ******************************************************************************/

void ip_slowtimo(void)
{
        struct ipq *fp;
        int s = splnet();

        fp = ipq.next;
        if (fp == 0) {
                splx(s);
                return;
        }
        while (fp != &ipq) {
                --fp->ipq_ttl;
                fp = fp->next;
                if (fp->prev->ipq_ttl == 0) {
                        ipstat.ips_fragtimeout++;
                        ip_freef(fp->prev);
                }
        }
        splx(s);
}

/*******************************************************************************
 * Drain off all datagram fragments.
 ******************************************************************************/

void ip_drain(void)
{

        while (ipq.next != &ipq) {
                ipstat.ips_fragdropped++;
                ip_freef(ipq.next);
        }
}

/*******************************************************************************
 * Do option processing on a datagram,
 * possibly discarding it if bad options are encountered,
 * or forwarding it if source-routed.
 * Returns 1 if packet has been forwarded/freed,
 * 0 if the packet should be processed further.
 ******************************************************************************/

int ip_dooptions(struct mbuf *m)
{
        struct ip *ip = mtod(m, struct ip *);
        u_char *cp;
        struct ip_timestamp *ipt;
        struct in_ifaddr *ia;
        int opt, optlen, cnt, off, code, type = ICMP_PARAMPROB, forward = 0;
        struct in_addr *sin, dst;
        n_time ntime;

        dst = ip->ip_dst;
        cp = (u_char *)(ip + 1);
        cnt = (ip->ip_hl << 2) - sizeof (struct ip);
        for (; cnt > 0; cnt -= optlen, cp += optlen) {
                opt = cp[IPOPT_OPTVAL];
                if (opt == IPOPT_EOL)
                        break;
                if (opt == IPOPT_NOP)
                        optlen = 1;
                else {
                        optlen = cp[IPOPT_OLEN];
                        if (optlen <= 0 || optlen > cnt) {
                                code = &cp[IPOPT_OLEN] - (u_char *)ip;
                                goto bad;
                        }
                }
                switch (opt) {

                default:
                        break;

                /*
                 * Source routing with record.
                 * Find interface with current destination address.
                 * If none on this machine then drop if strictly routed,
                 * or do nothing if loosely routed.
                 * Record interface address and bring up next address
                 * component.  If strictly routed make sure next
                 * address is on directly accessible net.
                 */
                case IPOPT_LSRR:
                case IPOPT_SSRR:
                        if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
                                code = &cp[IPOPT_OFFSET] - (u_char *)ip;
                                goto bad;
                        }
                        input_ipaddr.sin_addr = ip->ip_dst;
                        ia = (struct in_ifaddr *)
                                ifa_ifwithaddr((struct sockaddr *)&input_ipaddr);
                        if (ia == 0) {
                                if (opt == IPOPT_SSRR) {
                                        type = ICMP_UNREACH;
                                        code = ICMP_UNREACH_SRCFAIL;
                                        goto bad;
                                }
                                /*
                                 * Loose routing, and not at next destination
                                 * yet; nothing to do except forward.
                                 */
                                break;
                        }
                        off--;                  /* 0 origin */
                        if (off > optlen - sizeof(struct in_addr)) {
                                /*
                                 * End of source route.  Should be for us.
                                 */
                                break;
                        }
                        /*
                         * locate outgoing interface
                         */
                        memcpy(&input_ipaddr.sin_addr,
                               cp + off,
                               sizeof(input_ipaddr.sin_addr));
                        if (opt == IPOPT_SSRR) {
#define INA     struct in_ifaddr *
#define SA      struct sockaddr *
                            if ((ia = (INA)ifa_ifwithdstaddr((SA)&input_ipaddr)) == 0)
                                ia = (INA)ifa_ifwithnet((SA)&input_ipaddr);
                        } else
                                ia = ip_rtaddr(input_ipaddr.sin_addr);
                        if (ia == 0) {
                                type = ICMP_UNREACH;
                                code = ICMP_UNREACH_SRCFAIL;
                                goto bad;
                        }
                        ip->ip_dst = input_ipaddr.sin_addr;
                        memcpy(cp + off,
                               &(IA_SIN(ia)->sin_addr),
                               sizeof(struct in_addr));
                        cp[IPOPT_OFFSET] += sizeof(struct in_addr);
                        /*
                         * Let ip_intr's mcast routing check handle mcast pkts
                         */
                        forward = !IN_MULTICAST(ntohl(ip->ip_dst.s_addr));
                        break;

                case IPOPT_RR:
                        if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
                                code = &cp[IPOPT_OFFSET] - (u_char *)ip;
                                goto bad;
                        }
                        /*
                         * If no space remains, ignore.
                         */
                        off--;                  /* 0 origin */
                        if (off > optlen - sizeof(struct in_addr))
                                break;
                        memcpy(&input_ipaddr.sin_addr,
                               &ip->ip_dst,
                               sizeof(input_ipaddr.sin_addr));
                        /*
                         * locate outgoing interface; if we're the destination,
                         * use the incoming interface (should be same).
                         */
                        if ((ia = (INA)ifa_ifwithaddr((SA)&input_ipaddr)) == 0 &&
                            (ia = ip_rtaddr(input_ipaddr.sin_addr)) == 0) {
                                type = ICMP_UNREACH;
                                code = ICMP_UNREACH_HOST;
                                goto bad;
                        }
                        memcpy(cp + off,
                               &(IA_SIN(ia)->sin_addr),
                               sizeof(struct in_addr));
                        cp[IPOPT_OFFSET] += sizeof(struct in_addr);
                        break;

                case IPOPT_TS:
                        code = cp - (u_char *)ip;
                        ipt = (struct ip_timestamp *)cp;
                        if (ipt->ipt_len < 5)
                                goto bad;
                        if (ipt->ipt_ptr > ipt->ipt_len - sizeof (long)) {
                                if (++ipt->ipt_oflw == 0)
                                        goto bad;
                                break;
                        }
                        sin = (struct in_addr *)(cp + ipt->ipt_ptr - 1);
                        switch (ipt->ipt_flg) {

                        case IPOPT_TS_TSONLY:
                                break;

                        case IPOPT_TS_TSANDADDR:
                                if (ipt->ipt_ptr + sizeof(n_time) +
                                    sizeof(struct in_addr) > ipt->ipt_len)
                                        goto bad;
                                input_ipaddr.sin_addr = dst;
                                ia = (INA)ifaof_ifpforaddr((SA)&input_ipaddr,
                                                            m->m_pkthdr.rcvif);
                                if (ia == 0)
                                        continue;
                                memcpy(sin,
                                       &IA_SIN(ia)->sin_addr,
                                       sizeof(struct in_addr));
                                ipt->ipt_ptr += sizeof(struct in_addr);
                                break;

                        case IPOPT_TS_PRESPEC:
                                if (ipt->ipt_ptr + sizeof(n_time) +
                                    sizeof(struct in_addr) > ipt->ipt_len)
                                        goto bad;
                                memcpy(&input_ipaddr.sin_addr,
                                       sin,
                                       sizeof(struct in_addr));
                                if (ifa_ifwithaddr((SA)&input_ipaddr) == 0)
                                        continue;
                                ipt->ipt_ptr += sizeof(struct in_addr);
                                break;

                        default:
                                goto bad;
                        }
                        ntime = iptime();
                        memcpy(cp + ipt->ipt_ptr - 1,
                               &ntime,
                               sizeof(n_time));
                        ipt->ipt_ptr += sizeof(n_time);
                }
        }
        if (forward) {
                ip_forward(m, 1);
                return (1);
        }
        return (0);
bad:
        ip->ip_len -= ip->ip_hl << 2;   /* XXX icmp_error adds in hdr length */
        icmp_error(m, type, code, 0, 0);
        ipstat.ips_badoptions++;

        return (1);
}

/*******************************************************************************
 * Given address of next destination (final or next hop),
 * return internet address info of interface to be used to get there.
 ******************************************************************************/

struct in_ifaddr* ip_rtaddr(struct in_addr dst)
{
        struct sockaddr_in *sin;

        sin = (struct sockaddr_in *) &ipforward_rt.ro_dst;

        if ( (ipforward_rt.ro_rt == NULL) ||
             ((ipforward_rt.ro_rt->rt_flags & RTF_UP) == 0) ||
             (ipforward_rt.ro_rt->rt_flags & RTF_BLACKHOLE) ||
             (dst.s_addr != sin->sin_addr.s_addr) ) {

                if (ipforward_rt.ro_rt != NULL) {

                        RTFREE(ipforward_rt.ro_rt);
                        ipforward_rt.ro_rt = 0;

                }

                sin->sin_family = AF_INET;
                sin->sin_len = sizeof(*sin);
                sin->sin_addr = dst;

                ipforward_rt.ro_rt = rtalloc1(&ipforward_rt.ro_dst, 2, 1);

        }

        if (ipforward_rt.ro_rt == NULL)
                return NULL;

        return ((struct in_ifaddr *) ipforward_rt.ro_rt->rt_ifa);
}

/*******************************************************************************
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 * XXX should be deleted; last arg currently ignored.
 ******************************************************************************/

void ip_stripoptions(struct mbuf *m, struct mbuf *mopt)
{
        int i;
        struct ip *ip = mtod(m, struct ip *);
        caddr_t opts;
        int olen;

        olen = (ip->ip_hl<<2) - sizeof (struct ip);
        opts = (caddr_t)(ip + 1);
        i = m->m_len - (sizeof (struct ip) + olen);
        memcpy(opts, opts + olen, (unsigned) i);
        m->m_len -= olen;
        if (m->m_flags & M_PKTHDR)
                m->m_pkthdr.len -= olen;
        ip->ip_hl = sizeof(struct ip) >> 2;
}

/*******************************************************************************
 * Forward a packet.  If some error occurs return the sender
 * an icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repertoire
 * of codes and types.
 *
 * If not forwarding, just drop the packet.  This could be confusing
 * if ipforwarding was zero but some routing protocol was advancing
 * us as a gateway to somewhere.  However, we must let the routing
 * protocol deal with that.
 *
 * The srcrt parameter indicates whether the packet is being forwarded
 * via a source route.
 ******************************************************************************/

void ip_forward(struct mbuf *m, int srcrt)
{
        struct ip *ip = mtod(m, struct ip *);
        struct sockaddr_in *sin;
        struct rtentry *rt;
        int error, type = 0, code;
        struct mbuf *mcopy;
        n_long dest;
        struct ifnet *destifp;

        dest = 0;

#ifdef DIAGNOSTIC
        if (ipprintfs)
                printf("forward: src %x dst %x ttl %x\n", ip->ip_src,
                        ip->ip_dst, ip->ip_ttl);
#endif

        if ( (m->m_flags & M_BCAST) ||
             (m->m_flags & M_MCAST) ||
             (in_canforward(ip->ip_dst) == 0) ) {

                ipstat.ips_cantforward++;
                m_freem(m);
                return;

        }

        HTONS(ip->ip_id);
        if (ip->ip_ttl <= IPTTLDEC) {

                icmp_error(m, ICMP_TIMXCEED, ICMP_TIMXCEED_INTRANS, dest, 0);

                return;

        }

        ip->ip_ttl -= IPTTLDEC;

        sin = (struct sockaddr_in *)&ipforward_rt.ro_dst;

        rt = ipforward_rt.ro_rt;
        if ( (rt == NULL) ||
             ((rt->rt_flags & RTF_UP) == 0) ||
             (ipforward_rt.ro_rt->rt_flags & RTF_BLACKHOLE) ||
             (ip->ip_dst.s_addr != sin->sin_addr.s_addr) ) {

                if (ipforward_rt.ro_rt != NULL) {

                        RTFREE(ipforward_rt.ro_rt);
                        ipforward_rt.ro_rt = NULL;

                }

                sin->sin_family = AF_INET;
                sin->sin_len = sizeof(*sin);
                sin->sin_addr = ip->ip_dst;

                ipforward_rt.ro_rt = rtalloc1(&ipforward_rt.ro_dst, 2, TRUE);

                if (ipforward_rt.ro_rt == NULL) {

                        ipstat.ips_noroute++;
                        icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_HOST, dest, 0);

                        return;

                }

                rt = ipforward_rt.ro_rt;

        }

        if ((rt->rt_ifp->if_flags & IFF_DONT_FORWARD) != 0) {

                ipstat.ips_cantforward++;
                m_freem(m);
                return;

        }

        /*
         * Save at most 64 bytes of the packet in case
         * we need to generate an ICMP message to the src.
         */
        mcopy = m_copy(m, 0, min((int)ip->ip_len, 64));

#ifdef GATEWAY
        ip_ifmatrix[rt->rt_ifp->if_index +
             if_index * m->m_pkthdr.rcvif->if_index]++;
#endif
        /*
         * If forwarding packet using same interface that it came in on,
         * perhaps should send a redirect to sender to shortcut a hop.
         * Only send redirect if source is sending directly to us,
         * and if packet was not source routed (or has any options).
         * Also, don't send redirect if forwarding using a default route
         * or a route modified by a redirect.
         */
#define satosin(sa)     ((struct sockaddr_in *)(sa))
        if (rt->rt_ifp == m->m_pkthdr.rcvif &&
            (rt->rt_flags & (RTF_DYNAMIC|RTF_MODIFIED)) == 0 &&
            satosin(rt_key(rt))->sin_addr.s_addr != 0 &&
            (ip_flags & IP_DO_REDIRECT) && !srcrt) {

#define RTA(rt) ((struct in_ifaddr *)(rt->rt_ifa))

                u_long src = ntohl(ip->ip_src.s_addr);

                if (RTA(rt) &&
                    (src & RTA(rt)->ia_subnetmask) == RTA(rt)->ia_subnet) {
                    if (rt->rt_flags & RTF_GATEWAY)
                        dest = satosin(rt->rt_gateway)->sin_addr.s_addr;
                    else
                        dest = ip->ip_dst.s_addr;

                    /* Router requirements says to only send host redirects */
                    type = ICMP_REDIRECT;
                    code = ICMP_REDIRECT_HOST;
#ifdef DIAGNOSTIC
                    if (ipprintfs)
                        printf("redirect (%d) to %lx\n", code, (u_long)dest);
#endif
                }
        }

        m->m_flags |= M_FORWARD;

        error = ip_output(m, (struct mbuf *)0, &ipforward_rt, IP_FORWARDING
#ifdef DIRECTED_BROADCAST
                            | IP_ALLOWBROADCAST
#endif
                                                , 0);
        if (error) {

                ipstat.ips_cantforward++;

        }

        else {

                ipstat.ips_forward++;

                if (type) {

                        ipstat.ips_redirectsent++;

                }

                else {
                        if (mcopy != NULL)
                                m_freem(mcopy);
                        return;
                }

        }

        if (mcopy == NULL)
                return;

        destifp = NULL;

        switch (error) {

        case 0:                         /* forwarded, but need redirect */
                /* type, code set above */
                break;

        case ENETUNREACH:               /* shouldn't happen, checked above */
        case EHOSTUNREACH:
        case ENETDOWN:
        case EHOSTDOWN:
        default:
                type = ICMP_UNREACH;
                code = ICMP_UNREACH_HOST;
                break;

        case EMSGSIZE:
                type = ICMP_UNREACH;
                code = ICMP_UNREACH_NEEDFRAG;
                if (ipforward_rt.ro_rt != NULL)
                        destifp = ipforward_rt.ro_rt->rt_ifp;
                ipstat.ips_cantfrag++;
                break;

        case ENOBUFS:
                type = ICMP_SOURCEQUENCH;
                code = 0;
                break;
        }

        icmp_error(mcopy, type, code, dest, destifp);
}

