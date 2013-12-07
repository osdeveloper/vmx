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

/* ip_icmp.h - Ip icmp header */

#ifndef _ip_icmp_h
#define _ip_icmp_h

#include <netinet/in.h>
#include <netinet/ip.h>

#define ICMP_ECHOREPLY                          0       /* Echo reply: */

#define ICMP_UNREACH                            3       /* Unreachable: */
#define ICMP_UNREACH_NET                        0       /* Bad net */
#define ICMP_UNREACH_HOST                       1       /* Bad host */
#define ICMP_UNREACH_PROTOCOL                   2       /* Bad protocol */
#define ICMP_UNREACH_PORT                       3       /* Bad port */
#define ICMP_UNREACH_NEEDFRAG                   4       /* Caused drop */
#define ICMP_UNREACH_SRCFAIL                    5       /* Source route fail */
#define ICMP_UNREACH_NET_UNKNOWN                6       /* Unknown net */
#define ICMP_UNREACH_HOST_UNKNOWN               7       /* Unknown host */
#define ICMP_UNREACH_ISOLATED                   8       /* Src host isolated */
#define ICMP_UNREACH_NET_PROHIB                 9       /* Prohibited access */
#define ICMP_UNREACH_HOST_PROHIB                10      /* Prohibited access */
#define ICMP_UNREACH_TOSNET                     11      /* Bad tos for net */
#define ICMP_UNREACH_TOSHOST                    12      /* Bad tos for host */

#define ICMP_SOURCEQUENCH                       4       /* Packet lost */

#define ICMP_REDIRECT                           5       /* Redirect: */
#define ICMP_REDIRECT_NET                       0       /* Network */
#define ICMP_REDIRECT_HOST                      1       /* Host */
#define ICMP_REDIRECT_TOSNET                    2       /* Tos & net */
#define ICMP_REDIRECT_TOSHOST                   3       /* Tos & host */

#define ICMP_ECHO                               8       /* Echo */

#define ICMP_ROUTERADVERT                       9       /* Router advertise */

#define ICMP_ROUTERSOLICIT                      10      /* Router solocitate */

#define ICMP_TIMXCEED                           11      /* Time exceeded: */
#define ICMP_TIMXCEED_INTRANS                   0       /* Transition */
#define ICMP_TIMXCEED_REASS                     1       /* Reassamble */

#define ICMP_PARAMPROB                          12      /* Ip header bad */
#define ICMP_PARAMPROB_OPTABSENT                1       /* Opt. absent */

#define ICMP_TSTAMP                             13      /* Timestamp request */

#define ICMP_TSTAMPREPLY                        14      /* Temestamp reply */

#define ICMP_IREQ                               15      /* Info request */

#define ICMP_IREQREPLY                          16      /* Info reply */

#define ICMP_MASKREQ                            17      /* Mask request */

#define ICMP_MASKREPLY                          18      /* Mask reply */

#define ICMP_MAXTYPE                            18

#define ICMP_MINLEN                             8       /* Abs minimum */
#define ICMP_MASKLEN                            12

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct ih_idseq {
  unsigned short        icd_id;
  unsigned short        icd_seq;
};

struct id_ts {
  unsigned long         its_otime;
  unsigned long         its_rtime;
  unsigned long         its_ttime;
};

struct id_ip {
  struct ip             idi_ip;
};

struct icmp {

  unsigned char         icmp_type;                      /* Type of message */
  unsigned char         icmp_code;                      /* Type sub code */
  unsigned short        icmp_cksum;                     /* Checksum */

  union {

    unsigned char         ih_pptr;                      /* ICMP_PARAMPROB */
    struct in_addr        ih_gwaddr;                    /* ICMP_REDIRECT */
    struct ih_idseq       ih_idseq;
    int                   ih_void;

    struct ih_pmtu {
      unsigned short        ipm_void;
      unsigned short        ipm_nextmtu;

    } ih_pmtu;

  } icmp_hun;

  union {

    struct id_ts          id_ts;
    struct id_ip          id_ip;
    unsigned long         id_mask;
    char                  id_data[1];

  } icmp_dun;

};

#define icmp_pptr       icmp_hun.ih_pptr
#define icmp_gwaddr     icmp_hun.ih_gwaddr
#define icmp_id         icmp_hun.ih_idseq.icd_id
#define icmp_seq        icmp_hun.ih_idseq.icd_seq
#define icmp_void       icmp_hun.ih_void
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_otime      icmp_dun.id_ts.its_otime
#define icmp_rtime      icmp_dun.id_ts.its_rtime
#define icmp_ttime      icmp_dun.id_ts.its_ttime
#define icmp_ip         icmp_dun.id_ip.idi_ip
#define icmp_mask       icmp_dun.id_mask
#define icmp_data       icmp_dun.id_data

/* Macros */
#define ICMP_TSLEN      (8 + 3 * sizeof(unsigned short))
#define ICMP_ADVLENMIN  (8 + sizeof(struct ip) + 8)
#define ICMP_ADVLEN(p)  (8 + ((p)->icmp_ip.ip_hl << 2) + 8)

#define ICMP_INFOTYPE(type)                                                    \
  ( (type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO ||                         \
    (type) == ICMP_ROUTERADVERT || (type) == ICMP_ROUTERSOLICIT ||             \
    (type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY ||                     \
    (type) == ICMP_IREQ || (type) == ICMP_IREQREPLY ||                         \
    (type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY )

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ip_icmp_h */

