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

/* tcp.h - Tcp protocol header */

#ifndef _tcp_h
#define _tcp_h

#include <vmx.h>

/* Defines */
#define TCP_MSS                         512
#define TCP_MAXWIN                      65536
#define TCP_MAX_WINSHIFT                14

#define TCP_NODELAY                     0x01
#define TCP_MAXSEG                      0x02

#define TH_FIN                          0x01
#define TH_SYN                          0x02
#define TH_RST                          0x04
#define TH_PUSH                         0x08
#define TH_ACK                          0x10
#define TH_URG                          0x20

#define TCPOPT_EOL                      0
#define TCPOPT_NOP                      1
#define TCPOPT_MAXSEG                   2
#define TCPOLEN_MAXSEG                  4
#define TCPOPT_WINDOW                   3
#define TCPOLEN_WINDOW                  3
#define TCPOPT_SACK_PERMITTED           4
#define TCPOLEN_SACK_PERMITTED          2
#define TCPOPT_SACK                     5
#define TCPOPT_TIMESTAMP                8
#define TCPOLEN_TIMESTAMP               10
#define TCPOLEN_TSTAMP_APPA             (TCPOLEN_TIMESTAMP + 2)

#define TCPOPT_TSTAMP_HDR               (TCPOPT_NOP << 24 | TCPOPT_NOP << 16 | \
  TCPOPT_TIMESTAMP << 8 | TCPOLEN_TIMESTAMP)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef unsigned long tcp_seq;

/* Structs */
struct tcphdr {
  unsigned short                th_sport;               /* Source port */
  unsigned short                th_dport;               /* Destination port */
  tcp_seq                       th_seq;                 /* Seq. number */
  tcp_seq                       th_ack;                 /* Ack. number */

#if (_BYTE_ORDER == _LITTLE_ENDIAN)

  unsigned int                  th_x2:4,
                                th_off:4,               /* Data offset */

#elif (_BYTE_ORDER == _BIG_ENDIAN)

  unsigned int                  th_off:4,               /* Data offset */
                                th_x2:4,

#endif /* _BYTE_ORDER */

                                th_flags:8,             /* Flags */
                                th_win:16;              /* Window */
  unsigned short                th_sum;                 /* Checksum */
  unsigned short                th_urp;                 /* Urgent pointer */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcp_h */

