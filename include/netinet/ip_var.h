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

/* ip_var.h - Ip internal header */

#ifndef _ip_var_h
#define _ip_var_h

#include <sys/types.h>
#include <vmx.h>
#include <util/listLib.h>
#include <net/netLib.h>
#include <net/mbuf.h>
#include <net/socketvar.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>

/* Defines */
#define IP_FORWARDING           0x01                    /* Most of ip header */
#define IP_RAWOUTPUT            0x02                    /* Raw ip header */
#define IP_ROUTETOIF            SO_DONTROUTE            /* Bypass routing */
#define IP_ALLOWBROADCAST       SO_BROADCAST            /* Can broadcast */

#define MAX_IPOPTLEN            40

#ifndef _ASMLANGUAGE

/* Structs */
struct ipq {
  struct ipq                    *next, *prev;           /* List index */
  unsigned char                 ipq_ttl;                /* Time-to-live */
  unsigned char                 ipq_p;                  /* Protocol frag */
  unsigned short                ipq_id;                 /* Id */
  struct mbuf                   *ipq_hdr;               /* Mbuf header */
  struct mbuf                   *ipq_pkt;               /* Mbuf packet */
  struct in_addr                ipq_src;                /* Source address */
  struct in_addr                ipq_dst;                /* Dest. address */
};

struct ipasfrag {

#if (_BYTE_ORDER == _LITTLE_ENDIAN)

  unsigned int                  ip_hl:4,
                                ip_v:4,

#elif (_BYTE_ORDER == _BIG_ENDIAN)

  unsigned int                  ip_v:4,
                                ip_hl:4,

#endif /* _BYTE_ORDER */

                                ipf_mff:8,
                                ip_len:16;
  unsigned short                ip_id;                  /* Id */
  short                         ip_off;                 /* Offset */
  unsigned char                 ip_ttl;                 /* Time-to-live */
  unsigned char                 ip_p;
  unsigned short                ip_sum;                 /* Sum */
  struct in_addr                ip_src;                 /* Source address */
  struct in_addr                ip_dst;                 /* Dest. address */
};

struct ipoption {
  struct in_addr                ipopt_dst;              /* Destination addr */
  char                          ipopt_list[MAX_IPOPTLEN];
};

struct ipstat {
  unsigned long                 ips_total;              /* Total packages */
  unsigned long                 ips_badsum;             /* Checksums bad */
  unsigned long                 ips_tooshort;           /* Packets to short */
  unsigned long                 ips_toosmall;           /* Packets to small */
  unsigned long                 ips_badhlen;            /* Ip headers lenth */
  unsigned long                 ips_badlen;             /* Ips invalid lenght */
  unsigned long                 ips_fragments;          /* Fragmets recvd */
  unsigned long                 ips_fragdropped;        /* Framets droped */
  unsigned long                 ips_fragtimeout;        /* Fragments timeo */
  unsigned long                 ips_forward;            /* Packets forwarded */
  unsigned long                 ips_cantforward;        /* Packets unforward */
  unsigned long                 ips_redirectsent;       /* Redirections sent */
  unsigned long                 ips_noproto;            /* Unknown protocols */
  unsigned long                 ips_delivered;          /* Packets delivered */
  unsigned long                 ips_localout;           /* Packets gen. here */
  unsigned long                 ips_odropped;           /* Packets lost */
  unsigned long                 ips_reassembled;        /* Packets reassembl. */
  unsigned long                 ips_fragmented;         /* Packets framentiz. */
  unsigned long                 ips_ofragments;         /* Output fragments */
  unsigned long                 ips_cantfrag;           /* Unable to frag */
  unsigned long                 ips_badoptions;         /* Bad options */
  unsigned long                 ips_noroute;            /* Packets no route */
  unsigned long                 ips_badvers;            /* Packets bad ver. */
  unsigned long                 ips_rawout;             /* Raw packets */
};

struct ipovly {
  void                          *ih_next, *ih_prev;     /* List */
  unsigned char                 ih_x1;
  unsigned char                 ih_pr;                  /* Protocol */
  short                         ih_len;                 /* Length */
  struct in_addr                ih_src;                 /* Source address */
  struct in_addr                ih_dst;                 /* Destination */
};

/* Functions */
IMPORT STATUS ipLibInit(IP_CFG_PARAMS *params);

IMPORT n_time iptime(void);

IMPORT STATUS rawIpLibInit(RAWIP_CFG_PARAMS *cfg);
IMPORT void rip_input(struct mbuf *m);
IMPORT int rip_output(struct mbuf *m, struct socket *so, unsigned long dst);
IMPORT int rip_ctloutput(int op,
                         struct socket *so,
                         int level,
                         int optname,
                         struct mbuf **m);
IMPORT int rip_usrreq(struct socket *so,
                      int req,
                      struct mbuf *m,
                      struct mbuf *name,
                      struct mbuf *ctrl);

IMPORT int ip_output(struct mbuf *m0,
                     struct mbuf *opt,
                     struct route *ro,
                     int flags,
                     struct ip_moptions *imo);
IMPORT int ip_optcopy(struct ip *ip, struct ip *jp);
IMPORT int ip_ctloutput(int op,
                        struct socket *so,
                        int level,
                        int optname,
                        struct mbuf **mp);
IMPORT int in_pcbopts(struct mbuf **pcbopt,
                      struct mbuf *m);
IMPORT int ip_getmoptions(int optname,
                          struct ip_moptions *imo,
                          struct mbuf **mp);
IMPORT void ip_freemoptions(struct ip_moptions *imo);

IMPORT void ipintr(struct mbuf *m);
IMPORT void ip_freef(struct ipq *fp);
IMPORT void ip_slowtimo(void);
IMPORT void ip_drain(void);
IMPORT int ip_dooptions(struct mbuf *m);
IMPORT struct in_ifaddr* ip_rtaddr(struct in_addr dst);
IMPORT void ip_stripoptions(struct mbuf *m, struct mbuf *mopt);
IMPORT void ip_forward(struct mbuf *m, int srcrt);
IMPORT int ip_sysctl(int *name,
                     unsigned int namelen,
                     void *oldp,
                     size_t *oldlenp,
                     void *newp,
                     size_t newlen);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ip_var_h */

