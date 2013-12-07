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

/* raw_cb.h - Raw sockets control block */

#ifndef _raw_cb_h
#define _raw_cb_h

#include <vmx.h>
#include <util/listLib.h>
#include <sys/socket.h>
#include <net/mbuf.h>
#include <net/socketvar.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct rawcb {
  LIST_NODE                     listNode;               /* List node */
  struct socket                 *rcb_socket;            /* Pointer to socket */
  struct sockaddr               *rcb_faddr;             /* Foreign addr */
  struct sockaddr               *rcb_laddr;             /* Local address */
  struct sockproto              rcb_proto;              /* Protocol */
};

/* Macros */

/*******************************************************************************
 * sotorawcb - Get raw socket control block from socket
 *
 * RETURNS: Pointer to raw socket control block
 ******************************************************************************/

#define sotorawcb(so)           ((struct rawcb *) (so)->so_pcb)

/* Functions */

IMPORT void rawLibInit(int sendSpace, int recvSpace);
IMPORT int raw_attach(struct socket *so, int proto);
IMPORT void raw_detach(struct rawcb *rp);
IMPORT void raw_disconnect(struct rawcb *rp);
IMPORT void raw_input(struct mbuf *m0,
                      struct sockproto *proto,
                      struct sockaddr *src,
                      struct sockaddr *dst);
IMPORT int raw_usrreq(struct socket *so,
                      int req,
                      struct mbuf *m,
                      struct mbuf *name,
                      struct mbuf *ctrl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _raw_cb_h */

