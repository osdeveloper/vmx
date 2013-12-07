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

/* inLib.c - Internet primitives */

/*
 * Copyright (c) 1988, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)in_cksum.c	8.1 (Berkeley) 6/10/93
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <vmx.h>
#include <util/listLib.h>
#include <os/unixLib.h>
#include <net/socketvar.h>
#include <net/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>

/* Defines */

/* Imports */
IMPORT struct ifnet loif[];
IMPORT unsigned char inetctlerrmap[];

/* Locals */

/* Globals */
int subnetsarelocal = 1;
int in_interfaces = 0;
struct in_ifaddr *in_ifaddr = NULL;
struct in_addr zeroin_addr = {0};

/* Macros */
#define satosin(sa)			((struct sockaddr_in *) (sa))
#define sintosa(sin)			((struct sockaddr *) (sin))
#define ifatoia(ifa)			((struct in_ifaddr *) (ifa))

#define ADDCARRY(x)  (x > 65535 ? x -= 65535 : x)
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}


/* Functions */

/*******************************************************************************
 * inLibInit - Initialize internet library
 *
 * RETURNS: N/A
 ******************************************************************************/

void inLibInit(void)
{
}

/*******************************************************************************
 * in_netof - Get network number from internet address
 *
 * RETURNS: Network number
 ******************************************************************************/

unsigned long in_netof(struct in_addr in)
{
  unsigned long i, net;
  struct in_ifaddr *ia;

  /* Setup locals */
  i = ntohl(in.s_addr);

  /* Get network number */
  if ( IN_CLASSA(i) )
    net = i & IN_CLASSA_NET;

  else if ( IN_CLASSB(i) )
    net = i & IN_CLASSB_NET;

  else if ( IN_CLASSC(i) )
    net = i & IN_CLASSC_NET;

  else if ( IN_CLASSD(i) )
    net = i & IN_CLASSD_NET;

  else
    return 0;

  /* Check if subnet */
  for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)
    if (net == ia->ia_net)
      return (i & ia->ia_subnetmask);

  return net;
}

/*******************************************************************************
 * in_localaddr - Determine if an address is local
 *
 * RETURNS: One or Zero
 ******************************************************************************/

int in_localaddr(struct in_addr in)
{
  unsigned long i;
  struct in_ifaddr *ia;

  /* If subnets are local */
  if (subnetsarelocal) {

    for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)
      if ( (i & ia->ia_netmask) == ia->ia_net )
        return 1;

  } /* End if subnets are local */

  /* Else subnets are not local */
  else {

    for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)
      if ( (i & ia->ia_subnetmask) == ia->ia_subnet )
        return 1;

  } /* End else subnets are not local */

  return 0;
}

/*******************************************************************************
 * in_socktrim - Trim socket address
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_socktrim(struct sockaddr_in *sin)
{
  char *cp, *cplim;

  /* Initialize locals */
  cplim = (char *) &sin->sin_addr;
  cp = (char *) (&sin->sin_addr + 1);

  sin->sin_len = 0;
  while (--cp >= cplim) {

    if (*cp) {

      sin->sin_len = cp - (char *) sin + 1;
      break;

    }

  }
}

/*******************************************************************************
 * in_ifscrub - Delete existing routes for interface
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_ifscrub(struct ifnet *ifp, struct in_ifaddr *ia)
{
  /* If no route */
  if ( (ia->ia_flags & IFA_ROUTE) == 0)
    return;

  if (ifp->if_flags & (IFF_LOOPBACK | IFF_POINTOPOINT) )
    rtinit(&ia->ia_ifa, RTM_DELETE, RTF_HOST);
  else
    rtinit(&ia->ia_ifa, RTM_DELETE, 0);

  ia->ia_flags &= ~IFA_ROUTE;

#if 0
  ifa_rtcleanup(&ia->ia_ifa);
#endif
}

/*******************************************************************************
 * in_ifinit - Initalize interface internet address
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int in_ifinit(struct ifnet *ifp,
	      struct in_ifaddr *ia,
	      struct sockaddr_in *sin,
	      int scrub)
{
  int s, flags, err;
  unsigned long i;
  struct sockaddr_in oldaddr;

  /* Initialize locals */
  err = 0;
  i = ntohl(sin->sin_addr.s_addr);
  flags = RTF_UP;

  /* Get processor level */
  s = splimp();

  /* Store old address */
  oldaddr = ia->ia_addr;

  /* Set new address */
  ia->ia_addr = *sin;

  /* If ioctl function */
  if (ifp->if_ioctl != NULL) {

    /* Initialize interface */
    err = ( *ifp->if_ioctl) (ifp, SIOCSIFADDR, ia);
    if (err) {

      splx(s);
      ia->ia_addr = oldaddr;
      return err;

    }

  } /* End if ioctl function */

  /* Restore processor level */
  splx(s);

  /* If scrub requested */
  if (scrub) {

    ia->ia_ifa.ifa_addr = (struct sockaddr *) &oldaddr;
    in_ifscrub(ifp, ia);
    ia->ia_ifa.ifa_addr = (struct sockaddr *) &ia->ia_addr;

  } /* End if scrub requested */

  /* Setup netmask */
  if ( IN_CLASSA(i) )
    ia->ia_netmask = IN_CLASSA_NET;
  else if ( IN_CLASSB(i) )
    ia->ia_netmask = IN_CLASSB_NET;
  else
    ia->ia_netmask = IN_CLASSC_NET;

  /* If subnet mask zero */
  if (ia->ia_subnetmask == 0) {

    ia->ia_subnetmask = ia->ia_netmask;
    ia->ia_sockmask.sin_addr.s_addr = htonl(ia->ia_subnetmask);

  } /* End if subnet mask zero */

  /* Else subnet mask non-zero */
  else {

    ia->ia_netmask &= ia->ia_subnetmask;

  } /* End else subnet mask non-zero */

  /* Set net */
  ia->ia_net = i & ia->ia_netmask;
  ia->ia_subnet = i & ia->ia_subnetmask;
  in_socktrim(&ia->ia_sockmask);

  /* Add route for network */
  ia->ia_ifa.ifa_metric = ifp->if_metric;

  /* If broadcast */
  if (ifp->if_flags & IFF_BROADCAST) {

    ia->ia_broadaddr.sin_addr.s_addr =
	htonl(ia->ia_subnet | ~ia->ia_subnetmask);

    ia->ia_netbroadcast.s_addr =
	htonl(ia->ia_net | ~ia->ia_netmask);

  } /* End if broadcast */

  /* Else if loopback */
  else if (ifp->if_flags & IFF_LOOPBACK) {

    ia->ia_ifa.ifa_dstaddr = ia->ia_ifa.ifa_addr;
    flags |= RTF_HOST;

  } /* End else if loopback */

  /* Else if pointopoint */
  else if (ifp->if_flags & IFF_POINTOPOINT) {

    if (ia->ia_dstaddr.sin_family != AF_INET)
      return 0;

    flags |= RTF_HOST;

  } /* End else if pointopoint */

  /* Initialize route */
  err = rtinit(&ia->ia_ifa, RTM_ADD, flags);
  if (err == 0)
    ia->ia_flags |= IFA_ROUTE;

  return err;
}

/*******************************************************************************
 * in_control - Ioct operations
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int in_control(struct socket *so,
	       unsigned long cmd,
	       void *data,
	       struct ifnet *ifp)
{
  struct ifreq *ifr;
  struct in_ifaddr *ia, *oia;
  struct ifaddr *ifa;
  struct in_aliasreq *ifra;
  struct sockaddr_in oldaddr;
  int err, newroute, newmask, newhost;
  unsigned long i;

  /* Initialize local */
  ifr = (struct ifreq *) data;
  ia = NULL;
  ifra = (struct in_aliasreq *) data;
  err = 0;
  newroute = 0;

  /* If interface argument sent */
  if (ifp != NULL) {
 
    /* For all interfaces */
    for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)
      if (ia->ia_ifp == ifp)
        break;

  } /* End if interface argument sent */

  /******************************************/
  /* Select command part1 - Initialize data */
  /******************************************/
  switch(cmd) {

    case SIOCAIFADDR:

      if (ia == NULL)
        newroute = 1;

      /* FALL TRU */

    case SIOCDIFADDR:

      /* If internet family */
      if (ifra->ifra_addr.sin_family == AF_INET) {

        /* For all interface addresses */
        for (oia = ia; ia != NULL; ia = ia->ia_next) {

          if ((ia->ia_ifp == ifp) &&
	      (ia->ia_addr.sin_addr.s_addr == ifra->ifra_addr.sin_addr.s_addr))
            break;

        } /* End for all interface addresses */

      } /* End if internet family */

      if ( (cmd == SIOCDIFADDR) && (ia == NULL) )
        return EADDRNOTAVAIL;
 
      /* FALL TRU */

    case SIOCSIFADDR:
    case SIOCSIFNETMASK:
    case SIOCSIFDSTADDR:

      if ( (so->so_state & SS_PRIV) == 0 )
        return EPERM;

      if (ifp == NULL)
        panic("in control");


      /* If need to alloc */
      if (ia == NULL) {

        oia = mb_alloc(sizeof(struct in_ifaddr), MT_IFADDR, M_WAIT);
        if (oia == NULL)
          return ENOBUFS;

      } /* End if need to alloc */

      /* Clear address */
      memset(oia, 0, sizeof(struct in_ifaddr));

      /* Add address to address list */
      ia = in_ifaddr;

      /* If any addresses exists */
      if (ia != NULL) {

        /* Go to end of address list */
        for (; ia->ia_next != NULL; ia = ia->ia_next)
          continue;

        /* Set address */
        ia->ia_next = oia;

      } /* End if any addresses exists */

      /* Else no addresses exists */
      else {

        /* Set address */
        in_ifaddr = oia;

      } /* End else no addresses exists */

      /* Add address to interface address list */
      ia = oia;
      ifa = ifp->if_addrlist;

      /* If any addresses in interface address list */
      if (ifa != NULL) {

        /* Move to last address */
        for ( ; ifa->ifa_next != NULL; ifa = ifa->ifa_next)
          continue;

        /* Set address */
        ifa->ifa_next = (struct ifaddr *) ia;

      } /* End if any addresses in interface address list */

      /* Else no addresses in interface address list */
      else {

        /* Set address */
        ifp->if_addrlist = (struct ifaddr *) ia;

      } /* End else no addresses in interface address list */

      /* Setup address struct */
      ia->ia_ifa.ifa_addr = (struct sockaddr *) &ia->ia_addr;
      ia->ia_ifa.ifa_dstaddr = (struct sockaddr *) &ia->ia_dstaddr;
      ia->ia_ifa.ifa_netmask = (struct sockaddr *) &ia->ia_netmask;
      ia->ia_sockmask.sin_len = 8;

      /* If broadcast available */
      if (ifp->if_flags & IFF_BROADCAST) {

        ia->ia_broadaddr.sin_len = sizeof(struct sockaddr_in);
        ia->ia_broadaddr.sin_family = AF_INET;

      } /* End if broadcast available */

      /* Set interface point in address */
      ia->ia_ifp = ifp;

      if (ifp != loif)
        in_interfaces++;

    break;

    case SIOCSIFBRDADDR:

      if ( (so->so_state & SS_PRIV) == 0 )
        return EPERM;

      /* FALL TRU */

    case SIOCGIFADDR:
    case SIOCGIFNETMASK:
    case SIOCGIFDSTADDR:
    case SIOCGIFBRDADDR:

      if (ia == NULL)
        return EADDRNOTAVAIL;

    break;

  } /* End select command part1 - Initialize data */

  /******************************************/
  /* Select command part2 - Execute command */
  /******************************************/
  switch(cmd) {

    case SIOCGIFADDR:

      /* Get address */
      *((struct sockaddr_in *)&ifr->ifr_addr) = ia->ia_addr;

    break;

    case SIOCGIFBRDADDR:

      if ( (ifp->if_flags & IFF_BROADCAST) == 0 )
        return EINVAL;

      /* Get address */
      *((struct sockaddr_in *) &ifr->ifr_dstaddr) = ia->ia_broadaddr;

    break;

    case SIOCGIFDSTADDR:

      if ( (ifp->if_flags & IFF_POINTOPOINT) == 0 )
        return EINVAL;

      /* Get address */
      *((struct sockaddr_in *) &ifr->ifr_dstaddr) = ia->ia_dstaddr;

    break;

    case SIOCGIFNETMASK:

      /* Get mask */
      *((struct sockaddr_in *) &ifr->ifr_addr) = ia->ia_sockmask;

    break;

    case SIOCSIFDSTADDR:

      if ( (ifp->if_flags & IFF_POINTOPOINT) == 0 )
        return EINVAL;

      /* Set address */
      oldaddr = ia->ia_dstaddr;
      ia->ia_dstaddr = *(struct sockaddr_in *) &ifr->ifr_dstaddr;

      /* If interface ioctl fails */
      if ( (ifp->if_ioctl != NULL) &&
	   (err = ( *ifp->if_ioctl) (ifp, SIOCSIFDSTADDR, ia)) ) {

        ia->ia_dstaddr = oldaddr;
        return err;

      } /* If interface ioctl fails */

      /* If route */
      if (ia->ia_flags & IFA_ROUTE) {

        /* Delete old route */
        ia->ia_ifa.ifa_dstaddr = (struct sockaddr *) &oldaddr;
        rtinit(&(ia->ia_ifa), RTM_DELETE, RTF_HOST);

        /* Add new route */
        ia->ia_ifa.ifa_dstaddr = (struct sockaddr *) &ia->ia_dstaddr;
        rtinit(&(ia->ia_ifa), RTM_ADD, RTF_HOST | RTF_UP);

      } /* If route */

    break;

    case SIOCSIFBRDADDR:

      if ( (ifp->if_flags & IFF_BROADCAST) == 0 )
        return EINVAL;

      /* Set address */
      ia->ia_broadaddr = *(struct sockaddr_in *) &ifr->ifr_broadaddr;

    break;

    case SIOCSIFADDR:

      return in_ifinit(ifp, ia, (struct sockaddr_in *) &ifr->ifr_addr, 1);

    case SIOCSIFNETMASK:

      i = ifra->ifra_addr.sin_addr.s_addr;
      ia->ia_sockmask.sin_addr.s_addr = i;
      ia->ia_subnetmask = ntohl(ia->ia_sockmask.sin_addr.s_addr);
      in_socktrim(&ia->ia_sockmask);

    break;

    case SIOCAIFADDR:

      newmask = 0;
      newhost = 1;

      /* If inet family */
      if (ia->ia_addr.sin_family == AF_INET) {

        /* If zero length */
        if (ifra->ifra_addr.sin_len == 0) {

          ifra->ifra_addr = ia->ia_addr;
          newhost = 0;

        } /* End if zero length */

        /* Else if address match */
        else if (ifra->ifra_addr.sin_addr.s_addr ==
		 ia->ia_addr.sin_addr.s_addr) {

          newhost = 0;

        } /* End else if address match */

      } /* End if inet family */

      /* If mask length non-zero */
      if (ifra->ifra_mask.sin_len) {

        in_ifscrub(ifp, ia);
        ia->ia_sockmask = ifra->ifra_mask;
        ia->ia_subnetmask = ntohl(ia->ia_sockmask.sin_addr.s_addr);
        newmask = 1;

      } /* End if mask length non-zero */

      /* If point-to-point and inet */
      if ( (ifp->if_flags & IFF_POINTOPOINT) &&
	   (ifra->ifra_dstaddr.sin_family == AF_INET) ) {

        in_ifscrub(ifp, ia);
        ia->ia_dstaddr = ifra->ifra_dstaddr;
        newmask = 1;

      } /* End if point-to-point and inet */

      if ( (ifra->ifra_addr.sin_family == AF_INET) &&
	   (newhost || newmask) )
        err = in_ifinit(ifp, ia, &ifra->ifra_addr, 0);

      if ( (err == EEXIST) && !newroute)
        err = 0;

      if ( (ifp->if_flags & IFF_BROADCAST) &&
	   (ifra->ifra_broadaddr.sin_family == AF_INET) &&
	   (ifra->ifra_broadaddr.sin_addr.s_addr) )
        ia->ia_broadaddr = ifra->ifra_broadaddr;

      return err;

    case SIOCDIFADDR:

      in_ifscrub(ifp, ia);

      /* Get interface address list */
      ifa = ifp->if_addrlist;

      /* If the same address list */
      if (ifa == (struct ifaddr *) ia) {

        ifp->if_addrlist = ifa->ifa_next;

      } /* End if the same address list */

      /* Else not the same address list */
      else {

        /* Move until end or match */
        while ( (ifa->ifa_next != NULL) &&
		(ifa->ifa_next != (struct ifaddr *) ia) )
          ifa = ifa->ifa_next;

        if (ifa->ifa_next != NULL)
          ifa->ifa_next = ((struct ifaddr *) ia)->ifa_next;
        else
          panic("link inifaddr");

      } /* End else not the same address list */

      oia = ia;
      ia = in_ifaddr;

      /* If old is first element */
      if (oia == ia) {

        in_ifaddr = ia->ia_next;

      } /* End if old is first element */

      /* Else old is not first element */
      else {

        /* Move until end or match */
        while ( (ia->ia_next != NULL) &&
		(ia->ia_next != oia) )
          ia = ia->ia_next;

        if (ia->ia_next != NULL)
          ia->ia_next = oia->ia_next;
        else
          panic("unlink inifaddr");

      } /* End else old is not first element */

      IFAFREE(&oia->ia_ifa);

    break;

    default:

      if ( (ifp == NULL) || (ifp->if_ioctl == NULL) )
        return EOPNOTSUPP;

      return ( *ifp->if_ioctl) (ifp, cmd, data);

  } /* End select command part2 - Execute command */

  return 0;
}

/*******************************************************************************
 * in_pcballoc - Allocate pcb
 *
 * RETURNS: Errno or zero
 ******************************************************************************/

int in_pcballoc(struct socket *so, struct inpcbinfo *pcbinfo)
{
  struct inpcb *inp;
  int s;

  inp = mb_alloc( sizeof(struct inpcb), MT_PCB, M_DONTWAIT);
  if (inp == NULL)
    return ENOBUFS;

  /* Clear struct */
  memset( inp, 0, sizeof(struct inpcb) );

  /* Setup struct */
  inp->inp_pcbinfo = pcbinfo;
  inp->inp_socket = so;

  /* Get processor level */
  s = splnet();

  /* Insert into info list head */
  listInsert(pcbinfo->listhead, NULL, &inp->inp_list);
  in_pcbinshash(inp);

  /* Restore processor level */
  splx(s);

  /* Setup address */
  so->so_pcb = inp;

  return 0;
}

/*******************************************************************************
 * in_pcbbind - Bind to name
 *
 * RETURNS: Errno or zero
 ******************************************************************************/

int in_pcbbind(struct inpcb *inp, struct mbuf *name)
{
  struct sockaddr_in *sin;
  struct socket *so;
  struct inpcb *t;
  short *lastport;
  unsigned short lport;
  int wild, reuseport, i;

  /* Setup locals */
  so = inp->inp_socket;
  lastport = &inp->inp_pcbinfo->lastport;
  reuseport = (so->so_options & SO_REUSEPORT);
  lport = 0;
  wild = 0;
  i = in_pcbhash(inp);

  if (in_ifaddr == NULL)
    return EADDRNOTAVAIL;

  /* If local port or any inet address */
  if ( (inp->inp_lport != 0) || (inp->inp_laddr.s_addr != INADDR_ANY) )
    return EINVAL;

  if ( ((so->so_options & (SO_REUSEADDR | SO_REUSEPORT)) == 0) &&
        ( ((so->so_proto->pr_flags & PR_CONNREQUIRED) == 0) ||
	  ((so->so_options & SO_ACCEPTCONN) == 0) ) )
    wild = INPLOOKUP_WILDCARD;

  /* If name non-null */
  if (name != NULL) {

    /* Get name */
    sin = mtod(name, struct sockaddr_in *);
    if ( name->m_len != sizeof(struct sockaddr_in) )
      return EINVAL;

#ifdef NET_DIAGNOSE

    /* Check family */
    if (sin->sin_family != AF_INET)
      return EAFNOSUPPORT;

#endif /* NET_DIAGNOSE */

    /* Get port */
    lport = sin->sin_port;

    /* If multicast address */
    if ( IN_MULTICAST( ntohl(sin->sin_addr.s_addr) ) ) {

      if (so->so_options & SO_REUSEADDR)
        reuseport = SO_REUSEADDR | SO_REUSEPORT;

    } /* End if multicast address */

    /* Else if not any address */
    else if (sin->sin_addr.s_addr != INADDR_ANY) {

      sin->sin_port = 0;

      if (ifa_ifwithaddr((struct sockaddr *) sin) == NULL)
        return EADDRNOTAVAIL;

    } /* End else if not any address */

    /* If local port */
    if (lport) {

      /* Lookup address */
      t = in_pcblookup(inp->inp_pcbinfo, zeroin_addr, 0,
		       sin->sin_addr, lport, wild);
      if ( (t != NULL) && ((reuseport & t->inp_socket->so_options) == 0) )
        return EADDRINUSE;

    } /* End if local port */

    /* Store local address */
    inp->inp_laddr = sin->sin_addr;

  } /* End if name non-null */

  /* If local port zero */
  if (lport == 0) {

    /* While more nodes in list */
    do {

      ++*lastport;
      if ( (*lastport < IPPORT_RESERVED) ||
	   (*lastport > IPPORT_USERRESERVED) )
        *lastport = IPPORT_RESERVED;

      lport = htons(*lastport);

    } while (in_pcblookup(inp->inp_pcbinfo, zeroin_addr, 0,
			  inp->inp_laddr, lport, wild) );

  } /* End if local port zero */

  /* Setup port and rehash */
  inp->inp_lport = lport;
  in_pcbchghash(inp, i);

  return 0;
}

/*******************************************************************************
 * in_pcbladdr - Assign local address
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int in_pcbladdr(struct inpcb *inp,
		struct mbuf *name,
		struct sockaddr_in **plocal_sin)
{
  struct in_ifaddr *ia;
  struct sockaddr_in *sin;
  struct route *ro;
  struct ip_moptions *imo;
  struct ifnet *ifp;
  unsigned short fport;

  /* Setup locals */
  sin = mtod(name, struct sockaddr_in *);

  if ( name->m_len != sizeof(struct sockaddr_in) )
    return EINVAL;

  if (sin->sin_family != AF_INET)
    return EAFNOSUPPORT;

  if (sin->sin_port == 0)
    return EADDRNOTAVAIL;

  /* If interface address non-null */
  if (in_ifaddr != NULL) {

    /* If inet address any */
    if (sin->sin_addr.s_addr == INADDR_ANY) {

      sin->sin_addr = IA_SIN(in_ifaddr)->sin_addr;

    } /* End if inet address any */

    /* Else if broadcast address */
    else if (sin->sin_addr.s_addr == (unsigned long) INADDR_BROADCAST) {

      /* For all nodes */
      for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next) {

        /* If broadcast */
        if (ia->ia_ifp->if_flags & IFF_BROADCAST) {

	  sin->sin_addr =
		satosin(&in_ifaddr->ia_next->ia_broadaddr)->sin_addr;

          break;

        } /* End if broadcast */

      } /* End for all nodes */

    } /* End else if broadcast address */

  } /* End if interface address non-null */

  /* If any address */
  if (inp->inp_laddr.s_addr == INADDR_ANY) {

    ia = (struct in_ifaddr *) NULL;
    ro = &inp->inp_route;

    if ( (ro->ro_rt != NULL) &&
	 ( (satosin(&ro->ro_dst)->sin_addr.s_addr != sin->sin_addr.s_addr) ||
	   (inp->inp_socket->so_options & SO_DONTROUTE) ) ) {

      RTFREE(ro->ro_rt);
      ro->ro_rt = NULL;

    }

    /* If route but no routing info */
    if ( ((inp->inp_socket->so_options & SO_DONTROUTE) == 0) &&
	 ( (ro->ro_rt == NULL) || (ro->ro_rt->rt_ifp == NULL) ) ) {

      ro->ro_dst.sa_family = AF_INET;
      ro->ro_dst.sa_len = sizeof(struct sockaddr_in);
      ((struct sockaddr_in *) &ro->ro_dst)->sin_addr = sin->sin_addr;
      TOS_SET((struct sockaddr_in *) &ro->ro_dst, inp->inp_ip.ip_tos);

      rtalloc(ro);

    } /* End if route but no routing info */

    /* Route found */

    if ( (ro->ro_rt != NULL) && !(ro->ro_rt->rt_ifp->if_flags & IFF_LOOPBACK) )
      ia = ifatoia(ro->ro_rt->rt_ifa);

    /* If interface null */
    if (ia == NULL) {

      fport = sin->sin_port;
      sin->sin_port = 0;

      ia = ifatoia(ifa_ifwithdstaddr(sintosa(sin)));
      if (ia == NULL)
        ia = ifatoia(ifa_ifwithnet(sintosa(sin)));

      /* Set foreign port */
      sin->sin_port = fport;

      if (ia == NULL)
        ia = in_ifaddr;

      if (ia == NULL)
        return EADDRNOTAVAIL;

    } /* End if interface null */

    /* If multicast address */
    if ( IN_MULTICAST(ntohl(sin->sin_addr.s_addr)) &&
         (inp->inp_moptions != NULL) ) {

      imo = inp->inp_moptions;

      /* If multicast */
      if (imo->imo_multicast_ifp != NULL) {

        ifp = imo->imo_multicast_ifp;
        for (ia = in_ifaddr; ia != NULL; ia = ia->ia_next)
          if (ia->ia_ifp == ifp)
            break;

        if (ia == NULL)
          return EADDRNOTAVAIL;

      } /* End if multicast */

    } /* End if multicast address */

    /* Set as local address */
    *plocal_sin = &ia->ia_addr;

  } /* End if any address */

  return 0;
}

/*******************************************************************************
 * in_pcbconnect - Pcb connect
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int in_pcbconnect(struct inpcb *inp, struct mbuf *name)
{
  struct sockaddr_in *ifaddr;
  struct sockaddr_in *sin;
  struct in_addr addr;
  int i, err;

  /* Setup locals */
  sin = mtod(name, struct sockaddr_in *);
  i = in_pcbhash(inp);

  /* Assign local address */
  err = in_pcbladdr(inp, name, &ifaddr);
  if (err)
    return err;

  /* Get local address from struct or automaically assigned */
  if (inp->inp_laddr.s_addr)
    addr = inp->inp_laddr;
  else
    addr = ifaddr->sin_addr;

  /* Check if it exists in table */
  if (in_pcblookuphash(inp->inp_pcbinfo, sin->sin_addr, sin->sin_port,
		       addr, inp->inp_lport, 0) != NULL)
    return EADDRINUSE;

  /* If any local address */
  if (inp->inp_laddr.s_addr == INADDR_ANY) {

    if (inp->inp_lport == 0)
      in_pcbbind(inp, NULL);

    i = in_pcbhash(inp);

    inp->inp_laddr = ifaddr->sin_addr;

  } /* End if any local address */

  /* Setup address and rehash */
  inp->inp_faddr = sin->sin_addr;
  inp->inp_fport = sin->sin_port;
  in_pcbchghash(inp, i);

  return 0;
}

/*******************************************************************************
 * in_pcbdisconnect - Disconnect control block
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbdisconnect(struct inpcb *inp)
{
  int i;

  /* Setup locals */
  i = in_pcbhash(inp);

  /* Change address */
  inp->inp_faddr.s_addr = INADDR_ANY;
  inp->inp_fport = 0;

  /* Insert into hash table */
  in_pcbchghash(inp, i);

  if (inp->inp_socket->so_state & SS_NOFDREF)
    in_pcbdetach(inp);
}

/*******************************************************************************
 * in_pcbdetach - Detach pcb
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbdetach(struct inpcb *inp)
{
  struct socket *so;
  int s;

  /* Setup locals */
  so = inp->inp_socket;

  so->so_pcb = NULL;
  sofree(so);

  if (inp->inp_options)
    m_free(inp->inp_options);

  if (inp->inp_route.ro_rt != NULL)
    rtfree(inp->inp_route.ro_rt);

  //ip_freemoptions(inp->inp_moptions, inp);

  /* Get processor level */
  s = splnet();

  /* Remove from lists */
  listRemove(inp->inp_pcbinfo->listhead, &inp->inp_list);
  in_pcbremhash(inp);

  /* Restore processor level */
  splx(s);

  mb_free(inp);
}

/*******************************************************************************
 * in_pcbinshash - Insert pcb into hash list
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbinshash(struct inpcb *inp)
{
  LIST *pHead;

  /* Get head from hash table */
  pHead = &inp->inp_pcbinfo->hashbase[INP_PCBHASH(inp->inp_faddr.s_addr,
	inp->inp_lport, inp->inp_fport, inp->inp_pcbinfo->hashmask)];

  /* Insert into list */
  listInsert(pHead, NULL, &inp->inp_hash);
}

/*******************************************************************************
 * in_pcbhash - Get hash bucket number
 *
 * RETURNS: Bucket number
 ******************************************************************************/

int in_pcbhash(struct inpcb *inp)
{
  return (INP_PCBHASH(inp->inp_faddr.s_addr,
  		      inp->inp_lport,
		      inp->inp_fport,
		      inp->inp_pcbinfo->hashmask));
}

/*******************************************************************************
 * in_pcbchgehash - Change place in hash table
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbchghash(struct inpcb *inp, int oi)
{
  LIST *pHead;
  int s;

  /* Get processor level */
  s = splnet();

  /* Remove from old place in hash list */
  pHead = &inp->inp_pcbinfo->hashbase[oi];
  listRemove(pHead, &inp->inp_hash);

  /* Insert into new place in hash list */
  pHead = &inp->inp_pcbinfo->hashbase[INP_PCBHASH(inp->inp_faddr.s_addr,
	inp->inp_lport, inp->inp_fport, inp->inp_pcbinfo->hashmask)];
  listInsert(pHead, NULL, &inp->inp_hash);

  /* Restore processor level */
  splx(s);
}

/*******************************************************************************
 * in_pcbrehash - Insert in correct place in hash table
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbrehash(struct inpcb *inp)
{
  LIST *pHead;
  int s;

  /* Get processor level */
  s = splnet();

  /* Remove from base list */
  listRemove(inp->inp_pcbinfo->hashbase, &inp->inp_hash);

  /* Get head from hash table */
  pHead = &inp->inp_pcbinfo->hashbase[INP_PCBHASH(inp->inp_faddr.s_addr,
	inp->inp_lport, inp->inp_fport, inp->inp_pcbinfo->hashmask)];

  /* Insert into hash list */
  listInsert(pHead, NULL, &inp->inp_hash);

  /* Restore procssor level */
  splx(s);
}

/*******************************************************************************
 * in_pcbremhash - Remove from hash table
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbremhash(struct inpcb *inp)
{
  LIST *pHead;

  /* Get head from hash table */
  pHead = &inp->inp_pcbinfo->hashbase[INP_PCBHASH(inp->inp_faddr.s_addr,
	inp->inp_lport, inp->inp_fport, inp->inp_pcbinfo->hashmask)];

  /* Remove from hash list */
  listRemove(pHead, &inp->inp_hash);
}

/*******************************************************************************
 * in_lookuphash - Find pcb in hash list
 *
 * RETURNS: Pointer to node
 ******************************************************************************/

struct inpcb* in_pcblookuphash(struct inpcbinfo *pcbinfo,
			       struct in_addr faddr,
			       unsigned int fport_arg,
			       struct in_addr laddr,
			       unsigned int lport_arg,
			       int wildcard)
{
  struct inpcb *inp, *inpw;
  LIST *pHead;
  LIST_NODE *pNode;
  unsigned short fport, lport;
  int s;

  /* Setup locals */
  fport = (unsigned short) fport_arg;
  lport = (unsigned short) lport_arg;

  /* Get processor level */
  s = splnet();

  /* Get head from hash table */
  pHead = &pcbinfo->hashbase[INP_PCBHASH(faddr.s_addr, lport, fport,
	pcbinfo->hashmask)];

  /* For all nodes in list */
  for (pNode = LIST_HEAD(pHead); pNode != NULL; pNode = LIST_NEXT(pNode)) {

    /* Calculate address of struct */
    inp = (struct inpcb *) ( ((char *) pNode) -
				OFFSET(struct inpcb, inp_hash) );

    /* If match */
    if ( (inp->inp_faddr.s_addr == faddr.s_addr) &&
	 (inp->inp_fport == fport) &&
         (inp->inp_laddr.s_addr == laddr.s_addr) &&
	 (inp->inp_lport == lport) )
      goto found;

  } /* End for all nodes in list */

  /* If wildcard */
  if (wildcard) {

    /* Reset partial match */
    inpw = NULL;

    /* Get head from hash table */
    pHead = &pcbinfo->hashbase[INP_PCBHASH(INADDR_ANY, lport, 0,
	pcbinfo->hashmask)];

    /* For all nodes in list */
    for (pNode = LIST_HEAD(pHead); pNode != NULL; pNode = LIST_NEXT(pNode)) {

      /* Calculate address of struct */
      inp = (struct inpcb *) ( ((char *) pNode) -
				OFFSET(struct inpcb, inp_hash) );

      /* If partial match */
      if ( (inp->inp_faddr.s_addr == INADDR_ANY) &&
	   (inp->inp_fport == 0) &&
	   (inp->inp_lport == lport) ) {

        if (inp->inp_laddr.s_addr == laddr.s_addr)
          goto found;
        else if (inp->inp_laddr.s_addr == INADDR_ANY)
          inpw = inp;

      } /* End if partial match */

    } /* End for all nodes in list */

    /* If partial match found */
    if (inpw != NULL) {

      inp = inpw;
      goto found;

    } /* End if partial match found */

  } /* End if wildcard */

  /* Restore procssor level */
  splx(s);

  return NULL;

found:

#if 0
  /* If not first node in list */
  if ( pNode != LIST_HEAD(pHead) ) {

    listRemove(pHead, pNode);
    listInsert(pHead, NULL, pNode);

  } /* End if not first node in list */
#endif

  /* Restore procssor level */
  splx(s);

  return inp;
}

/*******************************************************************************
 * in_pcblookup - Lookup pcb in list
 *
 * RETURNS: Pointer to node
 ******************************************************************************/

struct inpcb* in_pcblookup(struct inpcbinfo *pcbinfo,
			   struct in_addr faddr,
			   unsigned int fport_arg,
			   struct in_addr laddr,
			   unsigned int lport_arg,
			   int wild_okay)
{
  struct inpcb *inp, *match;
  int wildcard, matchwild;
  unsigned short fport, lport;
  int s;

  /* Initialize loclas */
  match = NULL;
  matchwild = 3;
  fport = (unsigned short) fport_arg;
  lport = (unsigned short) lport_arg;

  /* Get processor level */
  s = splnet();

  /* For all nodes in list */
  for (inp = (struct inpcb *) LIST_HEAD(pcbinfo->listhead);
       inp != NULL;
       inp = (struct inpcb *) LIST_NEXT((LIST_NODE *) &inp->inp_list) ) {

    /* If local port mismatch */
    if (inp->inp_lport != lport)
      continue;

    /* Reset wildcard */
    wildcard = 0;

    /* If not any foreign address */
    if (inp->inp_faddr.s_addr != INADDR_ANY) {

      if (faddr.s_addr == INADDR_ANY)
        wildcard++;
      else if ( (inp->inp_faddr.s_addr != faddr.s_addr) ||
		(inp->inp_fport != fport) )
        continue;

    } /* End if not any foreign address */

    /* Else any foreign address */
    else {

      if (faddr.s_addr != INADDR_ANY)
        wildcard++;

    } /* End else any foreign address */

    /* If not any local address */
    if (inp->inp_laddr.s_addr != INADDR_ANY) {

      if (laddr.s_addr == INADDR_ANY)
        wildcard++;
      else if (inp->inp_laddr.s_addr != laddr.s_addr)
        continue;

    } /* End if not any local address */

    /* Else any local address */
    else {

      if (laddr.s_addr != INADDR_ANY)
        wildcard++;

    } /* End else any local address */

    /* If wildcard */
    if ( (wildcard) && (wild_okay == 0) )
      continue;

    /* If wildcard lt. matched wildcard */
    if (wildcard < matchwild) {

      match = inp;
      matchwild = wildcard;
      if (matchwild == 0)
        break;

    } /* End if wildcard lt. matched wildcard */

  } /* End for all nodes in list */

  /* Restore processor level */
  splx(s);

  return match;
}

/*******************************************************************************
 * in_ifwithaddr - Get interface for inet address
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

struct ifnet* in_ifwithaddr(struct in_addr addr)
{
  struct in_ifaddr *ia;

  /* For all interface addresses */
  for (ia = in_ifaddr;
       (ia != NULL) && (IA_SIN(ia)->sin_addr.s_addr != addr.s_addr);
       ia = ia->ia_next)
    continue;

  if (ia == NULL)
    return NULL;

  return ia->ia_ifp;
}

/*******************************************************************************
 * in_addrwithif - Get interface for address
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

struct in_ifaddr* in_addrwithif(struct ifnet *ifp)
{
  struct in_ifaddr *ia;

  /* For all interface addresses */
  for (ia = in_ifaddr;
       (ia != NULL) && (ia->ia_ifp != ifp);
       ia = ia->ia_next)
    continue;

  return ia;
}

/*******************************************************************************
 * in_broadcast - Determine if an address might be a local broadcast address
 *
 * RETURNS: One if it is a local broadcast address
 ******************************************************************************/

int in_broadcast(struct in_addr in, struct ifnet *ifp)
{
  struct ifaddr *ifa;
  unsigned long t;

  /* Must be broadcast */
  if ( (in.s_addr == INADDR_BROADCAST) ||
       (in.s_addr == INADDR_ANY) )
    return 1;

  /* Can't be broadcast */
  if ( (ifp->if_flags & IFF_BROADCAST) == 0 )
    return 0;

  /* Get address */
  t = ntohl(in.s_addr);

  /* For all addresses for interface */
  for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

    if ((ifa->ifa_addr->sa_family == AF_INET) &&
	(in.s_addr == ((struct in_ifaddr *)ifa)->ia_broadaddr.sin_addr.s_addr ||
	 in.s_addr == ((struct in_ifaddr *)ifa)->ia_netbroadcast.s_addr ||
	  t == ((struct in_ifaddr *)ifa)->ia_subnet ||
	  t == ((struct in_ifaddr *)ifa)->ia_net) )
      return 1;

  } /* End for all addresses for interface */
}

/*******************************************************************************
 * in_canforward - Determine if an ip address is reseved not to forward
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

int in_canforward(struct in_addr in)
{
  unsigned long i, net;

  /* Setup locals */
  i = ntohl(in.s_addr);

  /* Can't forward */
  if ( IN_EXPERIMENTAL(i) || IN_MULTICAST(i) )
    return 0;

  /* If class A network */
  if ( IN_CLASSA(i) ) {

    net = i & IN_CLASSA_NET;
    if ( (net == 0) || (net == (IN_LOOPBACKNET << IN_CLASSA_NSHIFT)) )
      return 0;

  } /* End if class A network */

  return 1;
}

/*******************************************************************************
 * in_pcbnotify - Pass some modification to all connections of protocol
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_pcbnotify(LIST *pList,
		  struct sockaddr *dst,
		  unsigned int fport_arg,
		  struct in_addr laddr,
		  unsigned int lport_arg,
		  int cmd,
		  VOIDFUNCPTR notify)
{
  struct inpcb *inp, *oinp;
  struct in_addr faddr;
  unsigned short fport, lport;
  int errno, s;

  /* Inititalize locals */
  fport = (unsigned short) fport_arg;
  lport = (unsigned short) lport_arg;

  if ( (cmd > PRC_NCMDS) ||
       (dst->sa_family != AF_INET) )
    return;

  /* Get foreign address */
  faddr = ((struct sockaddr_in *) dst)->sin_addr;
  if (faddr.s_addr == INADDR_ANY)
    return;

  if (PRC_IS_REDIRECT(cmd) ||
      (cmd == PRC_HOSTDEAD) ) {

    fport = 0;
    lport = 0;
    laddr.s_addr = 0;

#if 0
    if (cmd != PRC_HOSTDEAD)
      notify = (VOIDFUNCPTR) in_rtchange;
#endif

  }

  /* Get errno */
  errno = inetctlerrmap[cmd];

  /* Get processor level */
  s = splnet();

  /* For all nodes */
  for (inp = (struct inpcb *) LIST_HEAD(pList);
       inp != NULL; ) {

    if ( ((inp->inp_faddr.s_addr != faddr.s_addr) &&
	  (((struct sockaddr_in *) &inp->inp_route.ro_dst)->sin_addr.s_addr !=
	    faddr.s_addr)) ||
	 inp->inp_socket == NULL ||
	 (lport && inp->inp_lport != lport) ||
	 (laddr.s_addr && inp->inp_laddr.s_addr != laddr.s_addr) ||
	 (fport && inp->inp_fport != fport) ) {

      inp = (struct inpcb *) LIST_NEXT(&inp->inp_list);
      continue;

    }

    oinp = inp;
    inp = (struct inpcb *) LIST_NEXT(&inp->inp_list);

    if (notify)
      ( *notify) (oinp, errno);

  } /* End for all nodes */

  /* Restore processor level */
  splx(s);
}

/*******************************************************************************
 * in_losing - Check for alternatived with higher level complains
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_losing(struct inpcb *inp)
{
  struct rtentry *rt;
  struct rt_addrinfo info;

  /* Initialize locals */
  rt = inp->inp_route.ro_rt;
  if (rt != NULL)
    return;

  inp->inp_route.ro_rt = NULL;
  memset(&info, 0, sizeof(info));

  info.rti_info[RTAX_DST] = (struct sockaddr *) &inp->inp_route.ro_dst;
  info.rti_info[RTAX_GATEWAY] = rt->rt_gateway;
  info.rti_info[RTAX_NETMASK] = rt_mask(rt);

  /* NOTE: Not complete */
}

/*******************************************************************************
 * in_setsockaddr - Set socket address
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_setsockaddr(struct inpcb *inp, struct mbuf *name)
{
  struct sockaddr_in *sin;

  name->m_len = sizeof(struct sockaddr_in);
  sin = mtod(name, struct sockaddr_in *);

  /* Clear struct */
  memset(sin, 0, sizeof(struct sockaddr_in));

  /* Setup struct */
  sin->sin_family = AF_INET;
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_port = inp->inp_lport;
  sin->sin_addr = inp->inp_laddr;
}

/*******************************************************************************
 * in_setpeer - Set peer address
 *
 * RETURNS: N/A
 ******************************************************************************/

void in_setpeeraddr(struct inpcb *inp, struct mbuf *name)
{
  struct sockaddr_in *sin;

  name->m_len = sizeof(struct sockaddr_in);
  sin = mtod(name, struct sockaddr_in *);

  /* Clear struct */
  memset(sin, 0, sizeof(struct sockaddr_in));

  /* Setup struct */
  sin->sin_family = AF_INET;
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_port = inp->inp_fport;
  sin->sin_addr = inp->inp_faddr;
}


/*******************************************************************************
 * Checksum routine for Internet Protocol family headers (Portable Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 ******************************************************************************/

int in_cksum(struct mbuf *m, int len)
{
	u_short *w;
	int sum = 0;
	int mlen = 0;
	int byte_swapped = 0;

	union {
		char	c[2];
		u_short	s;
	} s_util;
	union {
		u_short s[2];
		long	l;
	} l_util;

	for (;m && len; m = m->m_next) {
		if (m->m_len == 0)
			continue;
		w = mtod(m, u_short *);
		if (mlen == -1) {
			/*
			 * The first byte of this mbuf is the continuation
			 * of a word spanning between this mbuf and the
			 * last mbuf.
			 *
			 * s_util.c[0] is already saved when scanning previous 
			 * mbuf.
			 */
			s_util.c[1] = *(char *)w;
			sum += s_util.s;
			w = (u_short *)((char *)w + 1);
			mlen = m->m_len - 1;
			len--;
		} else
			mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		/*
		 * Force to even boundary.
		 */
		if ((1 & (int) w) && (mlen > 0)) {
			REDUCE;
			sum <<= 8;
			s_util.c[0] = *(u_char *)w;
			w = (u_short *)((char *)w + 1);
			mlen--;
			byte_swapped = 1;
		}
		/*
		 * Unroll the loop to make overhead from
		 * branches &c small.
		 */
		while ((mlen -= 32) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
			sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
			sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
			w += 16;
		}
		mlen += 32;
		while ((mlen -= 8) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			w += 4;
		}
		mlen += 8;
		if (mlen == 0 && byte_swapped == 0)
			continue;
		REDUCE;
		while ((mlen -= 2) >= 0) {
			sum += *w++;
		}
		if (byte_swapped) {
			REDUCE;
			sum <<= 8;
			byte_swapped = 0;
			if (mlen == -1) {
				s_util.c[1] = *(char *)w;
				sum += s_util.s;
				mlen = 0;
			} else
				mlen = -1;
		} else if (mlen == -1)
			s_util.c[0] = *(char *)w;
	}
	if (len)
		printf("cksum: out of data\n");
	if (mlen == -1) {
		/* The last mbuf has odd # of bytes. Follow the
		   standard (the odd byte may be shifted left by 8 bits
		   or not as determined by endian-ness of the machine) */
		s_util.c[1] = 0;
		sum += s_util.s;
	}
	REDUCE;
	return (~sum & 0xffff);
}

