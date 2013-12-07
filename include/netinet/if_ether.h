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

/* if_ether.h - Network interface ethernet header */

#ifndef _if_ether_h
#define _if_ether_h

#include <netinet/in.h>

/* Defines */
#define ETHERTYPE_PUP                   0x0200          /* PUP protocol */
#define ETHERTYPE_IP                    0x0800          /* IP protocol */
#define ETHERTYPE_ARP                   0x0806          /* ARP protocol */
#define ETHERTYPE_REVARP                0x8035          /* Reverse ARP proto. */

#define ETHERTYPE_TRAIL                 0x1000          /* Trailer packet */
#define ETHERTYPE_NTRAILER              16

#define ETHERMTU                        1500
#define ETHERMIN                        ( 60 - 14 )

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct ether_addr {
  unsigned char         ether_addr_octet[6];
} __attribute__((packed));

struct ether_header {
  unsigned char         ether_dhost[6];
  unsigned char         ether_shost[6];
  unsigned short        ether_type;
} __attribute__((packed));

struct ether_tag_header {
  unsigned char         ether_dhost[6];
  unsigned char         ether_shost[6];
  unsigned char         etherTag[4];
  unsigned short        ether_type;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _if_ether_h */

