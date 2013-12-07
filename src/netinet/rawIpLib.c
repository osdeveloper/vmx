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

/* rawIpLib.c - Raw ip socket library */

/* Includes */
#include <stdlib.h>
#include <errno.h>
#include <vmx.h>
#include <util/listLib.h>
#include <os/unixLib.h>
#include <net/mbuf.h>
#include <net/socketvar.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

/* Defines */

/* Imports */
IMPORT struct ipstat ipstat;
IMPORT struct ifnet *ifnet;
IMPORT unsigned short ip_id;
IMPORT int ip_defttl;

/* Locals */
LOCAL BOOL rawIpLibInstalled = FALSE;
LOCAL int rip_sendspace;
LOCAL int rip_recvspace;
LOCAL struct inpcbinfo ripcbinfo;
LOCAL struct sockaddr_in ripsrc;

/* Globals */

/* Functions */

/*******************************************************************************
 * rawIpLibInit - Initialize raw ip sockets
 *
 * RETURNS: N/A
 ******************************************************************************/

STATUS rawIpLibInit(RAWIP_CFG_PARAMS *cfg)
{
  if (rawIpLibInstalled)
    return OK;

  rip_sendspace = cfg->rawipSndSpace;
  rip_recvspace = cfg->rawipRcvSpace;

  /* Allocate list */
  ripcbinfo.listhead = malloc( sizeof(LIST) );
  if (ripcbinfo.listhead == NULL)
    return ERROR;

  /* Initialize list */
  listInit(ripcbinfo.listhead);

  /* Initialize hash list */
  ripcbinfo.hashbase = hashinit(1, MT_PCB, &ripcbinfo.hashmask);
  if (ripcbinfo.hashbase == NULL) {

    free(ripcbinfo.listhead);
    return ERROR;

  }

  /* Initalize source address */
  ripsrc.sin_len = sizeof(struct sockaddr_in);
  ripsrc.sin_family = AF_INET;

  /* Mark as installed */
  rawIpLibInstalled = TRUE;

  return OK;
}

/*******************************************************************************
 * rip_input - Raw sockets input
 *
 * RETURNS: N/A
 ******************************************************************************/

void rip_input(struct mbuf *m)
{
  struct ip *ip;
  struct inpcb *inp;
  struct mbuf *n;
  struct socket *last;

  /* Initialize locals */
  ip = mtod(m, struct ip*);
  last = NULL;

  /* Get address from ip struct */
  ripsrc.sin_addr = ip->ip_src;
  *(int *) ripsrc.sin_zero = m->m_pkthdr.rcvif->if_index;

  /* For all nodes in list */
  for (inp = (struct inpcb *) LIST_HEAD(ripcbinfo.listhead);
       inp != NULL;
       inp = (struct inpcb *) LIST_NEXT(&inp->inp_list) ) {

    /* If protocol mismatch */
    if ( (inp->inp_ip.ip_p) &&
	 (inp->inp_ip.ip_p != ip->ip_p) )
      continue;

    /* If local address and destination address mismatch */
    if ( (inp->inp_laddr.s_addr) &&
	 (inp->inp_laddr.s_addr != ip->ip_dst.s_addr) )
      continue;

    /* If foreign address and soruce address mismatch */
    if ( (inp->inp_faddr.s_addr) &&
	 (inp->inp_faddr.s_addr != ip->ip_src.s_addr) )
      continue;

    /* If last match found */
    if (last != NULL) {

      /* Duplicate data */
      n = m_copy(m, 0, M_COPYALL);

      /* If copy succeeded */
      if (n != NULL) {

        if ( sbappendaddr(&last->so_rcv,
			 (struct sockaddr *) &ripsrc,
			 n,
			 (struct mbuf *) NULL) == 0 )
          m_freem(n);
        else
          sorwakeup(last);

      } /* End if copy succeeded */

    } /* End if last match found */

    /* Store last match */
    last = inp->inp_socket;

  } /* End for all nodes in list */

  /* If last match found */
  if (last != NULL) {

    if ( sbappendaddr(&last->so_rcv,
		     (struct sockaddr *) &ripsrc,
		     m,
		     (struct mbuf *) NULL) == 0 )
      m_freem(m);
    else
      sorwakeup(last);

  } /* End if last match found */

  /* Else last match not found */
  else {

    m_freem(m);

    /* Update ip stat */
    ipstat.ips_noproto++;
    ipstat.ips_delivered--;

  } /* End else last match not found */
}

/*******************************************************************************
 * rip_output - Generate ip header and pass packet to ip output
 *
 * RETURNS: Zero or error
 ******************************************************************************/

int rip_output(struct mbuf *m, struct socket *so, unsigned long dst)
{
  struct ip *ip;
  struct inpcb *inp;
  int flags, err;
  struct mbuf *opts;

  /* Initialize locals */
  inp = (struct inpcb *) sotoinpcb(so);
  flags = (so->so_options & SO_DONTROUTE) | IP_ALLOWBROADCAST;

  /* If header no include */
  if ( (inp->inp_flags & INP_HDRINCL) == 0) {

    M_PREPEND(m, sizeof(struct ip), M_WAIT);
    if (m == NULL)
      return ENOBUFS;

    /* Setup header */
    ip = mtod(m, struct ip *);
    ip->ip_tos = 0;
    ip->ip_off = 0;
    ip->ip_p = inp->inp_ip.ip_p;
    ip->ip_len = m->m_pkthdr.len;
    ip->ip_src = inp->inp_laddr;
    ip->ip_dst.s_addr = dst;
    ip->ip_ttl = inp->inp_ip.ip_ttl;

    /* Get options */
    opts = inp->inp_options;

  } /* End if header no include */

  /* Else header include */
  else {

    if ( (m->m_len < sizeof(struct ip)) &&
	 (m = m_pullup(m, sizeof(struct ip))) == NULL )
      return ENOBUFS;

    ip = mtod(m, struct ip *);
    if (ip->ip_id == 0)
      ip->ip_id = htons(ip_id++);

    opts = NULL;

    /* Update ip stat */
    ipstat.ips_rawout++;

  } /* Else header include */

  /* Make output */
  err = ip_output(m, opts, &inp->inp_route, flags, inp->inp_moptions);

  return err;
}

/*******************************************************************************
 * rip_ctloutput - Raw ip sockets set output options
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int rip_ctloutput(int op,
		  struct socket *so,
		  int level,
		  int optname,
		  struct mbuf **m)
{
  struct inpcb *inp;
  int err;

  /* Setup locals */
  inp = sotoinpcb(so);

  /* Select option */
  switch(optname) {

    case IP_HDRINCL:

      err = 0;

      /* If set option */
      if (op == PRCO_SETOPT) {

        if ( (*m == NULL) ||
	     (*m)->m_len < sizeof(int) )
	  err = EINVAL;
	else if ( *mtod(*m, int *) )
	  inp->inp_flags |= INP_HDRINCL;
	else
	  inp->inp_flags &= ~INP_HDRINCL;

	if (*m != NULL)
	  m_free(*m);

      } /* End if set option */

      /* Else get option */
      else {

        *m = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
        if (*m == NULL) {

          err = ENOBUFS;

        }

        else {

          (*m)->m_len = sizeof(int);
	  *mtod(*m, int *) = inp->inp_flags & INP_HDRINCL;

        }

      } /* End else get option */

      return err;

    default:
    break;

  } /* End select option */

  return ip_ctloutput(op, so, level, optname, m);
}

/*******************************************************************************
 * rip_usrreq - Protocol handler for raw ip sockets
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int rip_usrreq(struct socket *so,
	       int req,
	       struct mbuf *m,
	       struct mbuf *name,
	       struct mbuf *ctrl)
{
  int err;
  struct inpcb *inp;
  struct sockaddr_in *sin;
  unsigned long dst;

  /* Setup locals */
  err = 0;
  inp = sotoinpcb(so);

  /* Select request */
  switch (req) {

    case PRU_CONTROL:

      return in_control(so, (unsigned long) m, name, (struct ifnet *) ctrl);

    case PRU_ATTACH:

      if (inp != NULL)
        panic("rip_attach");

      /* If no privileged flag */
      if ( (so->so_state & SS_PRIV) == 0 ) {

        err = EACCES;
        break;

      } /* End if no privileged flag */

      /* If reserve space or pcb alloc fails */
      if ( (err = soreserve(so, rip_sendspace, rip_recvspace)) ||
	   (err = in_pcballoc(so, &ripcbinfo)) )
        break;

      /* Get allocated pcb */
      inp = (struct inpcb *) so->so_pcb;

      /* Setup ip struct */
      inp->inp_ip.ip_p = (int) name;
      inp->inp_ip.ip_ttl = ip_defttl;

    break;

    case PRU_DISCONNECT:

      /* If not connected */
      if ( (so->so_state & SS_ISCONNECTED) == 0 ) {

        err = ENOTCONN;
        break;

      } /* End if not connected */

      /* FALL TRU */

    case PRU_ABORT:

      soisdisconnected(so);

      /* FALL TRU */

    case PRU_DETACH:

      if (inp == NULL)
        panic("rip_detach");

      in_pcbdetach(inp);

    break;

    case PRU_BIND:

      /* Get name */
      sin = mtod(name, struct sockaddr_in *);

      /* If name size incorrect */
      if (name->m_len != sizeof(struct sockaddr_in ) ) {

        err = EINVAL;
        break;

      } /* End if name size incorrect */

      /* If no inet address  */
      if ( (ifnet == NULL) &&
	   ((sin->sin_family != AF_INET) &&
	    (sin->sin_family != AF_IMPLINK)) ||
	   (sin->sin_addr.s_addr &&
	    ifa_ifwithaddr((struct sockaddr *) sin) == 0) ) {

        err = EADDRNOTAVAIL;
	break;

      } /* End if no inet address */

      /* Set local address */
      inp->inp_laddr = sin->sin_addr;

    break;

    case PRU_CONNECT:

      /* Get name */
      sin = mtod(name, struct sockaddr_in *);

      /* If name size incorrect */
      if (name->m_len != sizeof(struct sockaddr_in ) ) {

        err = EINVAL;
        break;

      } /* End if name size incorrect */

      /* If no interface address  */
      if (ifnet == NULL) {

        err = EADDRNOTAVAIL;
	break;

      } /* End if no interface address  */

      /* If wrong family */
      if ( (sin->sin_family != AF_INET) &&
	   (sin->sin_family != AF_IMPLINK) ) {

        err = EAFNOSUPPORT;
        break;

      } /* End if wrong family */

      /* Set foreign address */
      inp->inp_faddr = sin->sin_addr;

      /* Mark as connected */
      soisconnected(so);

    break;


    case PRU_CONNECT2:

      err = EOPNOTSUPP;

    break;

    case PRU_SHUTDOWN:

      socantsendmore(so);

    break;

    case PRU_SEND:

      /* If connected */
      if (so->so_state & SS_ISCONNECTED) {

	/* If destination given */
        if (name != NULL) {

	  err = EISCONN;
	  break;

	} /* End if destination given */

	/* Get destination for connection */
        dst = inp->inp_faddr.s_addr;

      } /* End if connected */

      /* Else not connected */
      else {

        /* If destination not given */
        if (name == NULL) {

	  err = ENOTCONN;
	  break;

        } /* End if destination not given */

	/* Get give destination */
	dst = mtod(name, struct sockaddr_in *)->sin_addr.s_addr;

      } /* End else not connected */

      /* Do raw ip output */
      err = rip_output(m, so, dst);
      m = NULL;

    break;

    case PRU_SENSE:
      return 0;

    case PRU_RCVOOB:
    case PRU_RCVD:
    case PRU_LISTEN:
    case PRU_ACCEPT:
    case PRU_SENDOOB:
 
      err = EOPNOTSUPP;

    break;

    case PRU_SOCKADDR:

      in_setsockaddr(inp, name);

    break;

    case PRU_PEERADDR:

      in_setpeeraddr(inp, name);

    break;

    default:

      panic("rip_usrreq");

    break;

  } /* End select request */

  if (m != NULL)
    m_freem(m);

  return err;
}

