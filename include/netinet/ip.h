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

/* ip.h - Internet protocol header */

#ifndef _ip_h
#define _ip_h

#include <vmx.h>
#include <netinet/in.h>

/* Defines */
#define IPVERSION                       4

#define IP_DF                           0x4000          /* No fragment flag */
#define IP_MF                           0x2000          /* More fragment flag */
#define IP_OFFMASK                      0x1fff          /* Mask for fragment */
#define IP_MAXPACKET                    65535           /* Max packet size */

#define MAXTTL                          255             /* Max time-to-live */
#define IPDEFTTL                        64              /* Def time-to-live */
#define IPFRAGTTL                       60              /* Frag. time-to-live */
#define IPTTLDEC                        1               /* Substract ttl */

#define IPOPT_CONTROL                   0x00
#define IPOPT_RESERED1                  0x20
#define IPOPT_DEBMEAS                   0x40
#define IPOPT_RESERVED2                 0x60

#define IPOPT_EOL                       0
#define IPOPT_NOP                       1

#define IPOPT_RR                        7
#define IPOPT_TS                        68
#define IPOPT_SECURITY                  130
#define IPOPT_LSRR                      131
#define IPOPT_SATID                     136
#define IPOPT_SSRR                      137
#define IPOPT_RA                        148

#define IPOPT_OPTVAL                    0
#define IPOPT_OLEN                      1
#define IPOPT_OFFSET                    2
#define IPOPT_MINOFF                    4

#define IPOPT_TS_TSONLY                 0               /* Timestamp only */
#define IPOPT_TS_TSANDADDR              1               /* Timestmp & address */
#define IPOPT_TS_PRESPEC                3               /* Specefied modules */

#ifndef _ASMLANGUAGE

#define IPOPT_COPIED(o)                 ((o) & 0x80)
#define IPOPT_CLASS(o)                  ((o) & 0x60)
#define IPOPT_NUMBER(o)                 ((o) & 0x1f)

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct ip {

#if ( _BYTE_ORDER == _LITTLE_ENDIAN )

  unsigned int          ip_hl:4,                        /* Header length */
                        ip_v:4,                         /* Version */

#elif ( _BYTE_ORDER == _BIG_ENDIAN )

  unsigned int          ip_v:4,                         /* Version */
                        ip_hl:4,                        /* Header length */

#endif /* _BYTE_ORDER */

                        ip_tos:8,                       /* Type of service */
                        ip_len:16;                      /* Total length */
  unsigned short        ip_id;                          /* Id */
  short                 ip_off;                         /* Fragment offset */
  unsigned char         ip_ttl;                         /* Time to live */
  unsigned char         ip_p;                           /* Protocol */
  unsigned short        ip_sum;                         /* Checksum */
  struct in_addr        ip_src;                         /* Source address */
  struct in_addr        ip_dst;                         /* Dest. address */
};

struct ipt_ta {
  struct in_addr        ipt_addr;                       /* Address */
  unsigned long         ipt_time;                       /* Time */
};

struct ip_timestamp {
  unsigned int          ipt_code:8,                     /* IPOPT_TS */
                        ipt_len:8,                      /* Size of struct */
                        ipt_ptr:8,                      /* Index of curr ent */

#if ( _BYTE_ORDER == _LITTLE_ENDIAN )

                        ipt_flg:4,                      /* Flags */
                        ipt_oflw:4;                     /* Overflow count */

#elif ( _BYTE_ORDER == _BIG_ENDIAN )

                        ipt_oflw:4,                     /* Overflow count */
                        ipt_flg:4;                      /* Flags */

#endif /* _BYTE_ORDER */

  union ipt_timestamp {
    unsigned long         ipt_time[1];
    struct ipt_ta         ipt_ta[1];
  } ipt_timestamp;

};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ip_h */

