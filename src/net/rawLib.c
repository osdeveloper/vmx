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

/* rawLib.c - Raw socket library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <vmx.h>
#include <util/listLib.h>
#include <os/unixLib.h>
#include <net/mbuf.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/socketvar.h>
#include <net/raw_cb.h>

/* Defines */

/* Imports */

/* Locals */

/* Globals */
unsigned long raw_sendspace;
unsigned long raw_recvspace;
LIST rawList;

/* Functions */

/*******************************************************************************
 * rawLibInit - Initialize raw sockets libraryr
 *
 * RETURNS: N/A
 ******************************************************************************/

void rawLibInit(int sendSpace, int recvSpace)
{
  raw_sendspace = sendSpace;
  raw_recvspace = recvSpace;
  listInit(&rawList);
}

/*******************************************************************************
 * raw_attach - Attach raw control block
 *
 * RETURNS: Zero or error
 ******************************************************************************/

int raw_attach(struct socket *so, int proto)
{
  int err;
  struct rawcb *rp;

  /* Setup locals */
  err = 0;
  rp = sotorawcb(so);

  /* If control block null */
  if (rp == NULL)
    return ENOBUFS;

  /* Reserve space for socket */
  err = soreserve(so, raw_sendspace, raw_recvspace);
  if (err)
    return err;

  /* Setup struct */
  rp->rcb_socket = so;
  rp->rcb_proto.sp_family = so->so_proto->pr_domain->dom_family;
  rp->rcb_proto.sp_protocol = proto;

  /* Insert into list */
  listInsert(&rawList, NULL, &rp->listNode);

  return 0;
}

/*******************************************************************************
 * raw_detach - Detach raw socket control block
 *
 * RETURNS: N/A
 ******************************************************************************/

void raw_detach(struct rawcb *rp)
{
  struct socket *so;

  /* Initialize locals */
  so = rp->rcb_socket;

  /* Free */
  so->so_pcb = NULL;
  sofree(so);

  /* Remove from list */
  listRemove(&rawList, &rp->listNode);

  Free(rp);
}

/*******************************************************************************
 * raw_disconnect  - Disconnect raw control block
 *
 * RETURNS: N/A
 ******************************************************************************/

void raw_disconnect(struct rawcb *rp)
{
  if (rp->rcb_socket->so_state & SS_NOFDREF);
    raw_detach(rp);
}

/*******************************************************************************
 * raw_input - Raw protocol input routine
 *
 * RETURNS: N/A
 ******************************************************************************/

void raw_input(struct mbuf *m0,
	       struct sockproto *proto,
	       struct sockaddr *src,
	       struct sockaddr *dst)
{
  struct rawcb *rp;
  struct mbuf *m, *n;
  struct socket *last;
  int sockets;

  /* Initialize locals */
  m = m0;
  last = NULL;
  sockets = 0;

  /* For all nodes in list */
  for (rp = (struct rawcb *) LIST_HEAD(&rawList);
       rp != NULL;
       rp = (struct rawcb *) LIST_NEXT(&rp->listNode) ) {

    /* If protocol family not equal */
    if (rp->rcb_proto.sp_family != proto->sp_family)
      continue;

    /* If protocol not equal */
    if ( (rp->rcb_proto.sp_protocol) &&
	 (rp->rcb_proto.sp_protocol != proto->sp_protocol) )
      continue;

    /* If local address not equal to destination */
    if ( (rp->rcb_laddr != NULL) &&
	 (memcmp(rp->rcb_laddr, dst, rp->rcb_laddr->sa_len) != 0) )
      continue;

    /* If foreign address not equal to source*/
    if ( (rp->rcb_faddr != NULL) &&
	 (memcmp(rp->rcb_faddr, src, rp->rcb_faddr->sa_len) != 0) )
      continue;

    /* If last non-null */
    if (last != NULL) {

      n = m_copy(m, 0, M_COPYALL);

      /* If copy okay */
      if (n != NULL) {

        /* If append fails */
        if ( sbappendaddr(&last->so_rcv, src, n, (struct mbuf *) NULL) == 0 ) {

          m_freem(n);

        } /* End if append fails */

        /* Else append succeeds */
        else {

          sorwakeup(last);
          sockets++;

        } /* End else append succeeds */

      } /* End if copy okay */

    } /* End if last non-null */

    /* Store last node */
    last = rp->rcb_socket;

  } /* End for all nodes in list */

  /* If last node found */
  if (last != NULL) {

    /* If append fails */
    if ( sbappendaddr(&last->so_rcv, src, m, (struct mbuf *) NULL) == 0 ) {

      m_freem(m);

    } /* End if append fails */

    /* Else append succeeds */
    else {

      sorwakeup(last);
      sockets++;

    } /* End else append succeeds */

  } /* End if last node found */

  /* Else last node not found */
  else {

    m_freem(m);

  } /* End else last node not found */
}

/*******************************************************************************
 * raw_usrreq - Raw socket protocol user requests
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int raw_usrreq(struct socket *so,
	       int req,
	       struct mbuf *m,
	       struct mbuf *name,
	       struct mbuf *ctrl)
{
  struct rawcb *rp;
  int err, len;

  /* Initialize locals */
  rp = sotorawcb(so);
  err = 0;

  /* If control message */
  if (req == PRU_CONTROL)
    return EOPNOTSUPP;

  /* If control length */
  if ( (ctrl != NULL) && (ctrl->m_len) ) {

    err = EOPNOTSUPP;
    goto release;

  } /* End if control length */

  /* If null rawcb */
  if (rp == NULL) {

    err = EINVAL;
    goto release;

  } /* End if null rawcb */

  /* Select request */
  switch (req) {

    case PRU_ATTACH:

      /* If privilege flag not set */
      if ( (so->so_state && SS_PRIV) == 0 ) {

        err = EACCES;
        break;

      } /* End if privilege flag not set */

      /* Attach raw control block */
      err = raw_attach(so, (int) name);

    break;

    case PRU_DETACH:

      raw_detach(rp);

    break;

    case PRU_CONNECT2:

      err = EOPNOTSUPP;

      goto release;

    case PRU_DISCONNECT:

      /* If null foreign address */
      if (rp->rcb_faddr == NULL) {

        err = ENOTCONN;
        break;

      } /* End if null foreign address */

      raw_disconnect(rp);
      soisdisconnected(so);

    break;

    case PRU_SHUTDOWN:

      socantsendmore(so);

    break;


    case PRU_SEND:

      /* If non-null name */
      if (name != NULL) {

        /* If foreign address non-null */
        if (rp->rcb_faddr != NULL) {

          err = EISCONN;
          break;

        } /* End if foreign address non-null */

        rp->rcb_faddr = mtod(name, struct sockaddr *);

      } /* End if non-null name */

      /* Else null name */
      else {

        err = ENOTCONN;
        break;

      } /* End else null name */

      if (so->so_proto->pr_output != NULL)
        err = ( *so->so_proto->pr_output) (m, so);

      m = NULL;
      if (name != NULL)
        rp->rcb_faddr = NULL;

    break;

    case PRU_ABORT:

      soisdisconnected(so);
      raw_disconnect(rp);

    break;

    case PRU_SENSE:
      return 0;

    case PRU_RCVOOB:
    case PRU_RCVD:
      return EOPNOTSUPP;

    case PRU_CONNECT:
    case PRU_BIND:
    case PRU_LISTEN:
    case PRU_ACCEPT:
    case PRU_SENDOOB:

      err = EOPNOTSUPP;

    break;

    case PRU_SOCKADDR:

      /* If null local address */
      if (rp->rcb_laddr == NULL) {

        err = EINVAL;
        break;

      } /* End if null local address */

      len = rp->rcb_laddr->sa_len;
      memcpy( mtod(name, char *), rp->rcb_laddr, len);
      name->m_len = len;

    break;

    case PRU_PEERADDR:

      /* If null foreign address */
      if (rp->rcb_faddr == NULL) {

        err = ENOTCONN;
        break;

      } /* End if null foreign address */

      len = rp->rcb_faddr->sa_len;
      memcpy( mtod(name, char *), rp->rcb_faddr, len);
      name->m_len = len;

    break;

    default:

      panic("raw_usrreq");

    break;

  } /* End select request */

release:

  /* If mbuf non-null */
  if (m != NULL)
    m_freem(m);

  return err;
}

