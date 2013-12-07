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

/* if_loop.c - Lookback network interface */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <vmx.h>
#include <vmx/tickLib.h>
#include <os/unixLib.h>
#include <os/logLib.h>
#include <net/mbuf.h>
#include <net/netLib.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

/* Defines */
#define NLOOP			1			/* Just one for now */
#define LOMTU			(1024 * 32)

/* Imports */
IMPORT struct ifqueue ipintrq;

/* Locals */
LOCAL void lodequeue(void);

LOCAL char ifName[] = "lo";

/* Globals */
struct ifnet loif[NLOOP];

int looutput(struct ifnet *ifp,
	     struct mbuf *m,
	     struct sockaddr *dst,
	     struct rtentry *rt);

/* Functions */

/*******************************************************************************
 * loattach - Attach loopback interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS loattach(void)
{
  int i;
  struct ifnet *ifp;

  for (i = 0; i < NLOOP; i++) {

    ifp = &loif[i];

    memset(ifp, 0, sizeof(struct ifnet));

    ifp->if_unit = i;
    ifp->if_name = malloc( strlen(ifName) );
    if (ifp->if_name == NULL)
      return ERROR;

    strcpy(ifp->if_name, ifName);

    ifp->if_mtu = LOMTU;
    ifp->if_flags = IFF_LOOPBACK | IFF_MULTICAST;
    ifp->if_type = IFT_LOOP;
    ifp->if_hdrlen = 0;
    ifp->if_addrlen = 0;
    ifp->pCookie = NULL;

    ifp->if_ioctl = (FUNCPTR) NULL;
    ifp->if_output = (FUNCPTR) looutput;

    if (if_attach(ifp) != OK) {

      free(ifp->if_name);
      return ERROR;

    }

  }

  return OK;
}

/*******************************************************************************
 * looutput - Output on loopback interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int looutput(struct ifnet *ifp,
	     struct mbuf *m,
	     struct sockaddr *dst,
	     struct rtentry *rt)
{
  int s;
  struct ifqueue *ifq;
  BOOL netJobFlag;

  /* Initialize locals */
  ifq = NULL;
  netJobFlag = FALSE;

  /* If no packet header */
  if ( (m->m_flags & M_PKTHDR) == 0 )
    panic("looutput no header");

  /* Get current time */
  ifp->if_lastchange = tickGet();

  /* Set receive interface */
  m->m_pkthdr.rcvif = ifp;

  /* If route reject or blackhole */
  if ( (rt != NULL) &&
       (rt->rt_flags & (RTF_REJECT | RTF_BLACKHOLE)) ) {

    m_freem(m);
    if (rt->rt_flags & RTF_BLACKHOLE)
      return 0;

    if (rt->rt_flags & RTF_HOST)
      return EHOSTUNREACH;

    return ENETUNREACH;

  } /* End if route reject or blackhole */

  /* Update interface output status */
  ifp->if_opackets++;
  ifp->if_obytes += m->m_pkthdr.len;

  /* Select address family */
  switch(dst->sa_family) {

    case AF_INET:

      ifq = &ipintrq;

    break;

    default:

      logMsg("lo%d: can't handle af%d\n",
             (ARG) (int) ifp->if_unit,
             (ARG) (int) dst->sa_family,
             0, 0, 0, 0);
      m_freem(m);
      return EAFNOSUPPORT;

  } /* End select address family */

  /* Get processor level */
  s = splimp();

  /* If interface queue full */
  if ( IF_QFULL(ifq) ) {

    IF_DROP(ifq);
    m_freem(m);
    return ENOBUFS;

  } /* End if interface queue full */

  /* If anything on queue */
  if (ifq->ifq_head != NULL)
    netJobFlag = TRUE;

  /* Put it on queue */
  IF_ENQUEUE(ifq, m);

  /* Put work on net task if needed */
  if (!netJobFlag)
    netJobAdd((FUNCPTR) lodequeue,
	      (ARG) 0,
	      (ARG) 0,
	      (ARG) 0,
	      (ARG) 0,
	      (ARG) 0);

  /* Update interface input status */
  ifp->if_ipackets++;
  ifp->if_ibytes += m->m_pkthdr.len;

  /* Restore processor level */
  splx(s);

  return 0;
}

/*******************************************************************************
 * lodequeue - Transfer packet from ip queue to ipitr
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void lodequeue(void)
{
  int s;
  struct mbuf *m;

  /* Get processor level */
  s = splnet();

  /* If anything on queue */
  if (ipintrq.ifq_head != NULL) {

    /* Remove it from queue */
    IF_DEQUEUE(&ipintrq, m);

    /* Send it to ipLib */
    ipintr(m);

  } /* End if anything on queue */

  /* Restore processor level */
  splx(s);
}

