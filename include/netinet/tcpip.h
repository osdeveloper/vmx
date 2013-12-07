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

/* tcpip.h - Tcp/ip header */

#ifndef _tcpip_h
#define _tcpip_h

#include <vmx.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct tcpiphdr {
  struct ipovly			ti_i;			/* Overlaid ip struct */
  struct tcphdr			ti_t;			/* Tcp header */
};

#define ti_next			ti_i.ih_next
#define ti_prev			ti_i.ih_prev
#define ti_x1			ti_i.ih_x1
#define ti_pr			ti_i.ih_pr
#define ti_len			ti_i.ih_len
#define ti_src			ti_i.ih_src
#define ti_dst			ti_i.ih_dst

#define ti_sport		ti_t.th_sport
#define ti_dport		ti_t.th_dport
#define ti_seq			ti_t.th_seq
#define ti_ack			ti_t.th_ack
#define ti_x2			ti_t.th_x2
#define ti_off			ti_t.th_off
#define ti_flags		ti_t.th_flags
#define ti_win			ti_t.th_win
#define ti_sum			ti_t.th_sum
#define ti_urp			ti_t.th_urp

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcpip_h */

