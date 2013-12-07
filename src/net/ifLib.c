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

/* ifLib.c - Network interface library */

/*
 * Copyright (c) 1980, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *      @(#)if.c        8.3 (Berkeley) 1/4/94
 */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <vmx/wdLib.h>
#include <os/unixLib.h>
#include <sys/socket.h>
#include <net/netLib.h>
#include <net/socketvar.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/inetLib.h>
#include <net/if_dl.h>
#include <net/if.h>

/* Defines */
#define IF_MAX_INDEX            10

/* Imports */
IMPORT VOIDFUNCPTR rtIfaceMsgHook;

/* Locals */
LOCAL SEM_ID ifIndexSem;
LOCAL int ifIndexValue = 0;
LOCAL WDOG_ID ifSlowWd;

LOCAL STATUS ifIoctlSet(char *ifname, int cmd, int val);
LOCAL STATUS ifIoctlGet(char *ifname, int cmd, int *val);
LOCAL STATUS ifIoctlCall(int cmd, struct ifreq *ifrp);

LOCAL char* sprint_d(unsigned int n, char *buf, int buflen);
LOCAL void link_rtrequest(int cmd, struct rtentry *rt, struct sockaddr *sa);

/* Globals */
struct ifnet *ifnet = NULL;
int ifqmaxlen = 0;

/* Macros */

#define equal(a1, a2)                                                          \
        ( memcmp( (a1), (a2), ((struct sockaddr *) (a1))->sa_len ) == 0 )

#define roundup(a) ( 1 + ( ((a) - 1) | (sizeof(long) - 1)) )

/* Functions */

/*******************************************************************************
 * ifLibInit - Initialize network interface library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS ifLibInit(int ifqMaxLen)
{
  struct ifnet *ifp;

  /* Create semaphore */
  ifIndexSem = semMCreate(SEM_Q_PRIORITY);
  if (ifIndexSem == NULL)
    return ERROR;

  /* Set start index */
  ifIndexValue = 1;

  /* Set max len */
  ifqmaxlen = ifqMaxLen;

  /* If interface set */
  if (ifnet != NULL) {

    /* For all interfaces */
    for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

      /* If queue zero */
      if (ifp->if_snd.ifq_maxlen == 0)
        ifp->if_snd.ifq_maxlen = ifqmaxlen;

    } /* End for all interfaces */

  } /* End if interface set */

  /* Initialize watchdog timer for interface */
  ifSlowWd = wdCreate();
  if (ifSlowWd == NULL)
    return ERROR;

  /* Start slow timeout */
  if_slowtimo();

  return OK;
}

/*******************************************************************************
 * ifIndexAlloc - Allocte network interface index
 *
 * RETURNS: Index or ERROR
 ******************************************************************************/

int ifIndexAlloc(void)
{
  if (ifIndexValue == 0)
    return ERROR;

  semTake(ifIndexSem, WAIT_FOREVER);

  if (ifIndexValue == -1) {

    semGive(ifIndexSem);
    return ERROR;

  }

  /* If owerflow */
  if (++ifIndexValue >= IF_MAX_INDEX) {

    semGive(ifIndexSem);
    ifIndexValue = -1;
    return ERROR;

  } /* End if overflow */

  semGive(ifIndexSem);

  return ifIndexValue;
}

/*******************************************************************************
 * ifIndexVerify - Veirfy a network interface index
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

BOOL ifIndexVerify(int index)
{
  /* Not installed or overflowed */
  if ( (ifIndexValue == -1) || (ifIndexValue == 0) )
    return FALSE;

  semTake(ifIndexSem, WAIT_FOREVER);

  /* If installed */
  if (index < ifIndexValue)  {

    semGive(ifIndexSem);
    return TRUE;

  } /* End if installed */

  semGive(ifIndexSem);

  return FALSE;
}

/*******************************************************************************
 * ifIndexToIfp - Interface index to interface pointer
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

struct ifnet* ifIndexToIfp(int index)
{
  struct ifnet *ifp;
  int s;

  /* Verify index */
  if (!ifIndexVerify(index))
    return NULL;

  /* Setup locals */
  ifp = NULL;

  /* Get processor level */
  s = splnet();

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    if (ifp->if_index == index)
      break;

  } /* End for all interfaces */

  /* Restore processor level */
  splx(s);

  return ifp;
}

/*******************************************************************************
 * if_attach - Attach network interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS if_attach(struct ifnet *ifp)
{
  unsigned int socksize, ifasize;
  int i, s, namelen, unitlen, masklen;
  char buf[12], *unitname;
  struct ifnet **p;
  struct sockaddr_dl *sdl;
  struct ifaddr *ifa;

  /* Initialize locals */
  p = &ifnet;

  /* Initialize default flags */
  ifp->if_flags |= IFF_NOTRAILERS;

  /* Initialize queue len */
  if (ifp->if_snd.ifq_maxlen == 0)
    ifp->if_snd.ifq_maxlen = ifqmaxlen;

  /* Precalculate variables */
  unitname = sprint_d((unsigned int) ifp->if_unit, buf, sizeof(buf));
  namelen = strlen(ifp->if_name);
  unitlen = strlen(unitname);

  masklen = OFFSET(struct sockaddr_dl, sdl_data[0]) + unitlen + namelen;

  socksize = masklen + ifp->if_addrlen;
  socksize = roundup(socksize);
  if (socksize < sizeof(struct sockaddr_dl))
    socksize = sizeof(struct sockaddr_dl);

  /* Size of ifaddr struct plus 2 sockets */
  ifasize = sizeof(struct ifaddr) + 2 * socksize;

  /* Allocate memory */
  ifa = mb_alloc(ifasize, MT_IFADDR, M_WAIT);
  if (ifa == NULL)
    return ERROR;

  /* Get processor level */
  s = splnet();

  /* While interfaces left */
  while (*p != NULL)
    p = &((*p)->if_next);

  /* Set processor level */
  splx(s);

  /* Create new interface index */
  i = ifIndexAlloc();
  if (i == ERROR) {

    mb_free(ifa);
    return ERROR;

  }

  /* Set interface index */
  ifp->if_index = i;

  /* Set pointer to interface */
  *p = ifp;

  /* Clear all memory allocted */
  memset(ifa, 0, ifasize);

  /* Initialize link socket structure */
  sdl = (struct sockaddr_dl *) (ifa + 1);
  sdl->sdl_len = socksize;
  sdl->sdl_family = AF_LINK;
  memcpy(sdl->sdl_data, ifp->if_name, namelen);
  memcpy(namelen + (char *) sdl->sdl_data, unitname, unitlen);
  sdl->sdl_nlen = (namelen += unitlen);
  sdl->sdl_index = ifp->if_index;
  sdl->sdl_type = ifp->if_type;

  /* Setup inteface address structure */
  ifa->ifa_ifp = ifp;
  ifa->ifa_next = ifp->if_addrlist;
  ifa->ifa_rtrequest = (FUNCPTR) link_rtrequest;

  /* Setup interface structure */
  ifp->if_addrlist = ifa;

  /* Setup interface address addr field */
  ifa->ifa_addr = (struct sockaddr *) sdl;

  /* Setup interface mask, 2:nd socket allocated */
  sdl = (struct sockaddr_dl *) (socksize + (char *) sdl);
  ifa->ifa_netmask = (struct sockaddr *) sdl;
  sdl->sdl_len = masklen;

  while (namelen != 0)
    sdl->sdl_data[--namelen] = 0xff;

  return OK;
}

/*******************************************************************************
 * ifunit - Get interface with name
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

struct ifnet* ifunit(char *ifname)
{
  struct ifnet *ifp;
  int unit;
  unsigned len;
  char c;
  char *cp, *ep;
  char name[IFNAMSIZ];

  /* If null name given */
  if (ifname == NULL)
    return NULL;

  /* Copy string to local var */
  strncpy(name, ifname, IFNAMSIZ);

  /* Move to interface number */
  for (cp = name; (cp < name + IFNAMSIZ) && (*cp); cp++)
    if ( (*cp >= '0') && (*cp <= '9') )
      break;

  /* If no interface number */
  if ( (*cp = '\0') || (cp == name + IFNAMSIZ) )
    return NULL;

  len = cp - name + 1;
  c = *cp;
  ep = cp;
  for (unit = 0; (*cp >= '0') && (*cp <= '9'); )
    unit = unit * 10 + *cp++ - '0';

  *ep = 0;

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    if ( memcmp(ifp->if_name, name, len) )
      continue;

    if (unit == ifp->if_unit)
      break;

  } /* End for all interfaces */

  *ep = c;

  return ifp;
}
  
/*******************************************************************************
 * ifa_ifwithaddr - Locate inteface with complete address
 *
 * RETURNS: Pointer to interface address or NULL
 ******************************************************************************/

struct ifaddr* ifa_ifwithaddr(struct sockaddr *addr)
{
  struct ifnet *ifp;
  struct ifaddr *ifa;

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    /* For all addresses */
    for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

      /* If wrong family */
      if (ifa->ifa_addr->sa_family != addr->sa_family)
        continue;

      /* If equal */
      if (equal(addr, ifa->ifa_addr))
        return ifa;

      /* If broadcast match */
      if ( (ifp->if_flags & IFF_BROADCAST) &&
           (ifa->ifa_broadaddr != NULL) &&
           equal(ifa->ifa_broadaddr, addr) )
        return ifa;

    } /* End for all addresses */

  } /* End for all interfaces */

  return NULL;
}

/*******************************************************************************
 * ifa_ifwithdstaddr - Locate point-to-point inteface with destination address
 *
 * RETURNS: Pointer to interface address or NULL
 ******************************************************************************/

struct ifaddr* ifa_ifwithdstaddr(struct sockaddr *addr)
{
  struct ifnet *ifp;
  struct ifaddr *ifa;

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    /* If point-to-point */
    if (ifp->if_flags & IFF_POINTOPOINT) {

      /* For all addresses */
      for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

        /* If wrong family */
        if (ifa->ifa_addr->sa_family != addr->sa_family)
          continue;

        /* If equal */
        if (equal(addr, ifa->ifa_dstaddr))
          return ifa;

      } /* End for all addresses */

    } /* End if point-to-point */

  } /* End for all interfaces */

  return NULL;
}

/*******************************************************************************
 * ifa_ifwithnet - Locate inteface on a specific network
 *
 * RETURNS: Pointer to interface address or NULL
 ******************************************************************************/

struct ifaddr* ifa_ifwithnet(struct sockaddr *addr)
{
  struct ifnet *ifp;
  struct ifaddr *ifa, *ifa_match;
  struct sockaddr_dl *sdl;
  unsigned int af;
  char *addr_data, *cplim, *cp1, *cp2, *cp3;

  /* Initialize locals */
  ifa_match = NULL;
  af = addr->sa_family;
  addr_data = addr->sa_data;

  /* If link address family */
  if (af == AF_LINK) {

    sdl = (struct sockaddr_dl *) addr;
    ifp = ifIndexToIfp(sdl->sdl_index);
    if (ifp == NULL)
      return NULL;

    /* For all addresses */
    for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

      if (ifa->ifa_addr->sa_family == af)
        break;

    } /* End for all addresses */

    return ifa;

  } /* End if link address family */

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    /* For all addresses */
    for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

      /* If wrong family or zero netmask */
      if ( (ifa->ifa_addr->sa_family != af) || (ifa->ifa_netmask == NULL) ) {

next:

        continue;

      } /* End if wrong family or zero netmask */

      cp1 = addr_data;
      cp2 = ifa->ifa_addr->sa_data;
      cp3 = ifa->ifa_netmask->sa_data;
      cplim = ifa->ifa_netmask->sa_len + (char *) ifa->ifa_netmask;

      while (cp3 < cplim) {

        if ( (*cp1++ ^ *cp2++) & *cp3++ )
          goto next;

      }

      if ( (ifa_match == NULL) ||
           (rn_refines(ifa->ifa_netmask, ifa_match->ifa_netmask)) )
        ifa_match = ifa;

    } /* End for all addresses */

  } /* End for all interfaces */

  return ifa_match;
}

/*******************************************************************************
 * ifa_ifwithaf - Find an interface using a specific address family
 *
 * RETURNS: Pointer to interface address
 ******************************************************************************/

struct ifaddr *
ifa_ifwithaf(int af)
{
        struct ifnet *ifp;
        struct ifaddr *ifa;

        for (ifp = ifnet; ifp; ifp = ifp->if_next)
            for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
                if (ifa->ifa_addr->sa_family == af)
                        return (ifa);
        return ((struct ifaddr *)0);
}

/*******************************************************************************
 * ifaof_ifpforaddr -
 * Find an interface address specific to an interface best matching
 * a given address.
 *
 * RETURNS: Pointer to interface address
 ******************************************************************************/

struct ifaddr *
ifaof_ifpforaddr(struct sockaddr *addr, struct ifnet *ifp)
{
        struct ifaddr *ifa;
        char *cp, *cp2, *cp3;
        char *cplim;
        struct ifaddr *ifa_maybe = 0;
        u_int af = addr->sa_family;

        if (af >= AF_MAX)
                return (0);
        for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
                if (ifa->ifa_addr->sa_family != af)
                        continue;
                ifa_maybe = ifa;
                if (ifa->ifa_netmask == 0) {
                        if (equal(addr, ifa->ifa_addr) ||
                            (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
                                return (ifa);
                        continue;
                }
                cp = addr->sa_data;
                cp2 = ifa->ifa_addr->sa_data;
                cp3 = ifa->ifa_netmask->sa_data;
                cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
                for (; cp3 < cplim; cp3++)
                        if ((*cp++ ^ *cp2++) & *cp3)
                                break;
                if (cp3 == cplim)
                        return (ifa);
        }
        return (ifa_maybe);
}

/*******************************************************************************
 * if_down
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 *
 * RETURNS: N/A
 ******************************************************************************/

void
if_down(struct ifnet *ifp)
{
        struct ifaddr *ifa;

        ifp->if_flags &= ~IFF_UP;
        for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
                pfctlinput(PRC_IFDOWN, ifa->ifa_addr);

        if_qflush(&ifp->if_snd);

        if (rtIfaceMsgHook != NULL)
                ( *rtIfaceMsgHook) (ifp);
}

/*******************************************************************************
 * if_up -
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 *
 * RETURNS: N/A
 ******************************************************************************/

void
if_up(struct ifnet *ifp)
{
        struct ifaddr *ifa;

        ifp->if_flags |= IFF_UP;
#ifdef notyet
        /* this has no effect on IP, and will kill all iso connections XXX */
        for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
                pfctlinput(PRC_IFUP, ifa->ifa_addr);
#endif
#if 0
        rt_ifmsg(ifp);
#endif
}

/*******************************************************************************
 * if_qflush - Flush an interface queue.
 *
 * RETURNS: N/A
 ******************************************************************************/

void
if_qflush(struct ifqueue *ifq)
{
        struct mbuf *m, *n;

        n = ifq->ifq_head;
        while (m = n) {
                n = m->m_act;
                m_freem(m);
        }
        ifq->ifq_head = 0;
        ifq->ifq_tail = 0;
        ifq->ifq_len = 0;
}

/*******************************************************************************
 * if_slowtimeo -
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 *
 * RETURNS: N/A
 ******************************************************************************/

void if_slowtimo(void)
{
        struct ifnet *ifp;
        int s = splimp();

        if (ifnet != NULL) {

                for (ifp = ifnet; ifp; ifp = ifp->if_next) {

                        if (ifp->if_timer == 0 || --ifp->if_timer)
                                continue;

                        if (ifp->if_watchdog)
                                (*ifp->if_watchdog)(ifp->if_unit);
                }

        }

        splx(s);

        /* Start slow timeout */
        wdStart(ifSlowWd,
                sysClockRateGet() / IFNET_SLOWHZ,
                (FUNCPTR) netJobAdd,
                (ARG) if_slowtimo);
}

/*******************************************************************************
 * ifioctl - Interface ioctls.
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int
ifioctl(struct socket *so, int cmd, void *data)
{
        struct ifnet *ifp;
        struct ifreq *ifr;
        int error;

        switch (cmd) {

        case SIOCGIFCONF:
        case OSIOCGIFCONF:
                return (ifconf(cmd, data));
        }
        ifr = (struct ifreq *)data;
        ifp = ifunit(ifr->ifr_name);
        if (ifp == 0)
                return (ENXIO);
        switch (cmd) {

        case SIOCGIFFLAGS:
                ifr->ifr_flags = ifp->if_flags;
                break;

        case SIOCGIFMETRIC:
                ifr->ifr_metric = ifp->if_metric;
                break;

        case SIOCSIFFLAGS:
                if (ifp->if_flags & IFF_UP && (ifr->ifr_flags & IFF_UP) == 0) {
                        int s = splimp();
                        if_down(ifp);
                        splx(s);
                }
                if (ifr->ifr_flags & IFF_UP && (ifp->if_flags & IFF_UP) == 0) {
                        int s = splimp();
                        if_up(ifp);
                        splx(s);
                }
                ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
                        (ifr->ifr_flags &~ IFF_CANTCHANGE);
                if (ifp->if_ioctl)
                        (void) (*ifp->if_ioctl)(ifp, cmd, data);
                break;

        case SIOCSIFMETRIC:
                ifp->if_metric = ifr->ifr_metric;
                break;

        case SIOCADDMULTI:
        case SIOCDELMULTI:
                if (ifp->if_ioctl == NULL)
                        return (EOPNOTSUPP);
                return ((*ifp->if_ioctl)(ifp, cmd, data));

        default:
                if (so->so_proto == 0)
                        return (EOPNOTSUPP);
#ifndef COMPAT_43
                return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
                        cmd, data, ifp));
#else
            {
                int ocmd = cmd;

                switch (cmd) {

                case SIOCSIFDSTADDR:
                case SIOCSIFADDR:
                case SIOCSIFBRDADDR:
                case SIOCSIFNETMASK:
#if BYTE_ORDER != BIG_ENDIAN
                        if (ifr->ifr_addr.sa_family == 0 &&
                            ifr->ifr_addr.sa_len < 16) {
                                ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
                                ifr->ifr_addr.sa_len = 16;
                        }
#else
                        if (ifr->ifr_addr.sa_len == 0)
                                ifr->ifr_addr.sa_len = 16;
#endif
                        break;

                case OSIOCGIFADDR:
                        cmd = SIOCGIFADDR;
                        break;

                case OSIOCGIFDSTADDR:
                        cmd = SIOCGIFDSTADDR;
                        break;

                case OSIOCGIFBRDADDR:
                        cmd = SIOCGIFBRDADDR;
                        break;

                case OSIOCGIFNETMASK:
                        cmd = SIOCGIFNETMASK;
                }

                error =  ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
                                                            cmd, data, ifp));

                switch (ocmd) {

                case OSIOCGIFADDR:
                case OSIOCGIFDSTADDR:
                case OSIOCGIFBRDADDR:
                case OSIOCGIFNETMASK:
                        *(u_short *)&ifr->ifr_addr = ifr->ifr_addr.sa_family;
                }
                return (error);

            }
#endif
        }
        return (0);
}

/*******************************************************************************
 * ifconf -
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int
ifconf(int cmd, void *data)
{
        struct ifconf *ifc = (struct ifconf *)data;
        struct ifnet *ifp = ifnet;
        struct ifaddr *ifa;
        char *cp, *ep;
        struct ifreq ifr, *ifrp;
        int space = ifc->ifc_len, error = 0;

        ifrp = ifc->ifc_req;
        ep = ifr.ifr_name + sizeof (ifr.ifr_name) - 2;
        for (; space > sizeof (ifr) && ifp; ifp = ifp->if_next) {
                strncpy(ifr.ifr_name, ifp->if_name, sizeof (ifr.ifr_name) - 2);
                for (cp = ifr.ifr_name; cp < ep && *cp; cp++)
                        continue;
                *cp++ = '0' + ifp->if_unit; *cp = '\0';
                if ((ifa = ifp->if_addrlist) == 0) {
                        memset((caddr_t)&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
                        memcpy(ifrp, &ifr, sizeof(ifr));
                        space -= sizeof (ifr), ifrp++;
                } else 
                    for ( ; space > sizeof (ifr) && ifa; ifa = ifa->ifa_next) {
                        struct sockaddr *sa = ifa->ifa_addr;
#ifdef COMPAT_43
                        if (cmd == OSIOCGIFCONF) {
                                struct osockaddr *osa =
                                         (struct osockaddr *)&ifr.ifr_addr;
                                ifr.ifr_addr = *sa;
                                osa->sa_family = sa->sa_family;
                                memcpy(ifrp, &ifr, sizeof(ifr));
                                ifrp++;
                        } else
#endif
                        if (sa->sa_len <= sizeof(*sa)) {
                                ifr.ifr_addr = *sa;
                                memcpy(ifrp, &ifr, sizeof(ifr));
                                ifrp++;
                        } else {
                                space -= sa->sa_len - sizeof(*sa);
                                if (space < sizeof (ifr))
                                        break;
                                memcpy(ifrp, &ifr, sizeof(ifr.ifr_name));
                                memcpy(&ifrp->ifr_addr, sa, sa->sa_len);
                                ifrp = (struct ifreq *)
                                        (sa->sa_len + (caddr_t)&ifrp->ifr_addr);
                        }
                        if (error)
                                break;
                        space -= sizeof (ifr);
                }
        }
        ifc->ifc_len -= space;
        return (error);
}

/*******************************************************************************
 * link_rtrequest -
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void
link_rtrequest(int cmd, struct rtentry *rt, struct sockaddr *sa)
{
        struct ifaddr *ifa;
        struct sockaddr *dst;
        struct ifnet *ifp;

        if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
            ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
                return;
        if (ifa = ifaof_ifpforaddr(dst, ifp)) {
                IFAFREE(rt->rt_ifa);
                rt->rt_ifa = ifa;
                ifa->ifa_refcnt++;
                if (ifa->ifa_rtrequest &&
                    ifa->ifa_rtrequest != (FUNCPTR) link_rtrequest)
                        ifa->ifa_rtrequest(cmd, rt, sa);
        }
}

/*******************************************************************************
 * sprint_d - Print decimal value to string
 *
 * RETURNS: Pointer to string
 ******************************************************************************/

LOCAL char* sprint_d(unsigned int n, char *buf, int buflen)
{
  char str[] = "0123456789";
  char *cp;

  /* Initialize locals */
  cp = buf + buflen - 1;
  *cp = 0;

  /* While n non-zero */
  do {

    cp--;
    *cp = str[n % 10];
    n /= 10;

  } while (n != 0);

  return cp;
}

