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

/* routeLib.c - Routing library */

/*
 * Copyright (c) 1980, 1986, 1991, 1993
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
 *      @(#)route.c     8.3 (Berkeley) 1/9/95
 * $FreeBSD: src/sys/net/route.c,v 1.66 2001/10/17 11:10:55 ru Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vmx.h>
#include <vmx/tickLib.h>
#include <net/mbuf.h>
#include <net/domain.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <net/radix.h>
#include <net/m2Lib.h>
#include <net/route.h>

/* Imports */
IMPORT struct domain *domains;
IMPORT VOIDFUNCPTR rtMissMsgHook;
IMPORT VOIDFUNCPTR rtNewAddrMsgHook;

/* Locals */
LOCAL struct rtstat rtstat;
LOCAL int       rttrash;                /* routes not in table but not freed */

LOCAL void rtable_init(void ** table);
LOCAL int rt_fixdelete(struct radix_node *rn, void *vp);
LOCAL int rt_fixchange(struct radix_node *rn, void *vp);
LOCAL void rt_maskedcopy(struct sockaddr *src,
                         struct sockaddr *dst,
                         struct sockaddr *netmask);

/* Globals */
struct route_cb route_cb;
struct radix_node_head *rt_tables[AF_MAX + 1];

/* Macros */
#define SA(p) ((struct sockaddr *)(p))
#define ROUNDUP(a) (a>0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

/*******************************************************************************
 * rtable_init - Initialize routing tables
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void
rtable_init(void **table)
{
        struct domain *dom;

        for (dom = domains; dom; dom = dom->dom_next)
                if (dom->dom_rtattach)
                        dom->dom_rtattach(&table[dom->dom_family],
                            dom->dom_rtoffset);
}

/*******************************************************************************
 * routeLibInit - Initialize routing library
 *
 * RETURNS: N/A
 ******************************************************************************/

void
routeLibInit(void)
{
        rtable_init((void **)rt_tables);
}

/*******************************************************************************
 * rtalloc - Allocate route
 *
 * RETURNS: N/A
 ******************************************************************************/

void
rtalloc(struct route *ro)
{
        struct rtentry *rt;
        int s;

        rt = ro->ro_rt;
        if (rt != NULL) {

                if (rt->rt_ifp != NULL && rt->rt_flags & RTF_UP)
                        return;

                /* XXX - We are probably always at splnet here already. */
                s = splnet();
                RTFREE(rt);
                ro->ro_rt = NULL;
                splx(s);

        }

        ro->ro_rt = rtalloc1(&ro->ro_dst, 1, TRUE);
}

/*******************************************************************************
 * rtalloc1 -
 * Look up the route that matches the address given
 * Or, at least try.. Create a cloned route if needed.
 *
 * RETURNS: Pointer to routing entry
 ******************************************************************************/

struct rtentry *
rtalloc1(struct sockaddr *dst, int report, BOOL skipFlag)
{
        struct radix_node_head *rnh = rt_tables[dst->sa_family];
        struct rtentry *rt;
        struct radix_node *rn;
        struct rtentry *newrt = 0;
        struct rt_addrinfo info;
        unsigned long tosrtmask, newrtmask;
        unsigned char oldtos;
        struct rtentry *tosrt = NULL;
        int  s = splnet(), err = 0, msgtype = RTM_MISS;

        /* Save tos */
        oldtos = TOS_GET(dst);

        /* Set new tos */
        TOS_SET(dst, oldtos & 0x1f);

        /*
         * Look up the address in the table for that Address Family
         */

match:

        if ((rnh != NULL) &&
            (rn = (struct radix_node *) (* rnh->rnh_matchaddr)
                                        ((caddr_t)dst, rnh, skipFlag)) &&
            ((rn->rn_flags & RNF_ROOT) == 0)) {

                /*
                 * If we find it and it's not the root node, then
                 * get a refernce on the rtentry associated.
                 */
                newrt = rt = (struct rtentry *)rn;

                if ( (dst->sa_family == AF_INET) &&
                     (rt->rt_refcnt == 0) &&
                     (rt->rt_flags & RTF_DELETE) &&
                     (!rt->rt_flags & RTF_STATIC)) {

                  rt->rt_flags & ~RTF_DELETE;
                  rt->rt_rmx.rmx_expire = 0;

                }

                if ( (rt->rt_flags & RTF_CLONING) &&
                     (report == 1 ||
                      (report == 2 && !(rt->rt_flags & RTF_GATEWAY)))) {

                        /*
                         * We are apparently adding (report = 0 in delete).
                         * If it requires that it be cloned, do so.
                         * (This implies it wasn't a HOST route.)
                         */
                        err = rtrequest(RTM_RESOLVE, dst, SA(0),
                                              SA(0), 0, &newrt);
                        if (err) {
                                /*
                                 * If the cloning didn't succeed, maybe
                                 * what we have will do. Return that.
                                 */
                                newrt = rt;
                                rt->rt_refcnt++;
                                goto miss;
                        }
                        if ((rt = newrt) && (rt->rt_flags & RTF_XRESOLVE)) {
                                /*
                                 * If the new route specifies it be
                                 * externally resolved, then go do that.
                                 */
                                msgtype = RTM_RESOLVE;
                                goto miss;
                        }
                }

                else {

                        rt->rt_refcnt++;

                }

        }

        else {

                /*
                 * Either we hit the root or couldn't find any match,
                 * Which basically means
                 * "caint get there frm here"
                 */
                rtstat.rts_unreach++;

miss:

                if (report) {

                        /*
                         * If required, report the failure to the supervising
                         * Authorities.
                         * For a delete, this is not an error. (report == 0)
                         */
                        memset((caddr_t)&info, 0, sizeof(info));
                        info.rti_info[RTAX_DST] = dst;

                        if (rtMissMsgHook != NULL)
                                ( *rtMissMsgHook) (msgtype, &info, 0, err);

                }
        }

        /* If tos non-zero */
        if ( TOS_GET(dst) ) {

          TOS_SET(dst, 0);
          tosrt = newrt;
          newrt = NULL;
          goto match;

        } /* End if tos non-zero */

        /* If tosrt non-null */
        if (tosrt != NULL) {

          /* If new route non-null */
          if (newrt != NULL) {

            if ( rt_mask(tosrt) == NULL)
              tosrtmask = 0xffffffff;
            else
              tosrtmask =
                ((struct sockaddr_in *) rt_mask(tosrt))->sin_addr.s_addr;

            if ( rt_mask(newrt) == NULL)
              newrtmask = 0xffffffff;
            else
              newrtmask =
                ((struct sockaddr_in *) rt_mask(newrt))->sin_addr.s_addr;

            /* If tos route mask ge. new route mask */
            if ( tosrtmask >= newrtmask) {

              RTFREE(newrt);
              newrt = tosrt;

            } /* End if tos route mask ge. new route mask */

            /* Else newrt greater */
            else {

              RTFREE(tosrt);

            } /* End else newrt greater */

          } /* End if new route non-null */

          /* Else newrt null */
          else {

            newrt = tosrt;

          } /* End else newrt null */

        } /* End if tosrt non-null */

        /* Restore tos */
        TOS_SET(dst, oldtos);

        splx(s);

        return (newrt);
}

/*******************************************************************************
 * rtfree - Remove a reference count from an rtentry.
 * If the count gets low enough, take it out of the routing table
 *
 * RETURNS: N/A
 ******************************************************************************/

void
rtfree(struct rtentry *rt)
{
        /*
         * find the tree for that address family
         */
        struct radix_node_head *rnh =
                rt_tables[rt_key(rt)->sa_family];
        struct ifaddr *ifa;

        if (rt == NULL)
                panic("rtfree");

        /*
         * decrement the reference count by one and if it reaches 0,
         * and there is a close function defined, call the close function
         */
        rt->rt_refcnt--;

        if (rt_key(rt)->sa_family == AF_INET) {

                if ( (rt->rt_refcnt == 0) &&
                     (rt->rt_flags & RTF_UP) &&
                     (rt->rt_flags & RTF_HOST) &&
                     (rt->rt_flags & RTF_LLINFO) &&
                     (rt->rt_flags & RTF_CLONED) ) {

                        rt->rt_flags |= RTF_DELETE;
                        if (rt->rt_rmx.rmx_expire == 0)
                                rt->rt_rmx.rmx_expire == tickGet();

                }

        }

        /*
         * If we are no longer "up" (and ref == 0)
         * then we can free the resources associated
         * with the route.
         */
        if ( (rt->rt_refcnt <= 0) &&
             (rt->rt_flags & RTF_UP) == 0) {

                if (rt->rt_nodes->rn_flags & (RNF_ACTIVE | RNF_ROOT))
                        panic ("rtfree 2");

                /*
                 * the rtentry must have been removed from the routing table
                 * so it is represented in rttrash.. remove that now.
                 */

                rttrash--;

#ifdef  DIAGNOSTIC
                if (rt->rt_refcnt < 0) {

                        printf("rtfree: %p not freed (neg refs)\n", rt);
                        return;

                }

#endif

                /*
                 * release references on items we hold them on..
                 * e.g other routes and ifaddrs.
                 */
                ifa = rt->rt_ifa;
                if(ifa != NULL)
                        IFAFREE(ifa);

                if (rt->rt_parent != NULL)
                        RTFREE(rt->rt_parent);


                /*
                 * The key is separatly alloc'd so free it (see rt_setgate()).
                 * This also frees the gateway, as they are always malloc'd
                 * together.
                 */
                Free(rt_key(rt));

                /*
                 * and the rtentry itself of course
                 */
                Free(rt);

        }

}

/*******************************************************************************
 * ifafree - Free interface address
 *
 * RETURNS: N/A
 ******************************************************************************/

void
ifafree(struct ifaddr *ifa)
{
        if (ifa == NULL)
                panic("ifafree");
        if (ifa->ifa_refcnt == 0)
                //free(ifa, M_IFADDR);
                mb_free(ifa);
        else
                ifa->ifa_refcnt--;
}

/*******************************************************************************
 * rtredirect -
 * Force a routing table entry to the specified
 * destination to go through the given gateway.
 * Normally called as a result of a routing redirect
 * message from the network layer.
 *
 * RETURNS: N/A
 ******************************************************************************/

void
rtredirect(struct sockaddr *dst,
           struct sockaddr *gateway,
           struct sockaddr *netmask,
           int flags,
           struct sockaddr *src,
           struct rtentry **rtp)
{
        struct rtentry *rt;
        int error = 0;
        short *stat = 0;
        struct rt_addrinfo info;
        struct ifaddr *ifa;

        /* verify the gateway is directly reachable */
        if ((ifa = ifa_ifwithnet(gateway)) == 0) {
                error = ENETUNREACH;
                goto out;
        }
        rt = rtalloc1(dst, 0, 0UL);
        /*
         * If the redirect isn't from our current router for this dst,
         * it's either old or wrong.  If it redirects us to ourselves,
         * we have a routing loop, perhaps as a result of an interface
         * going down recently.
         */
#define equal(a1, a2) (memcmp((caddr_t)(a1), (caddr_t)(a2), (a1)->sa_len) == 0)
        if (!(flags & RTF_DONE) && rt &&
             (!equal(src, rt->rt_gateway) || rt->rt_ifa != ifa))
                error = EINVAL;
        else if (ifa_ifwithaddr(gateway))
                error = EHOSTUNREACH;
        if (error)
                goto done;
        /*
         * Create a new entry if we just got back a wildcard entry
         * or the the lookup failed.  This is necessary for hosts
         * which use routing redirects generated by smart gateways
         * to dynamically build the routing tables.
         */
        if ((rt == 0) || (rt_mask(rt) && rt_mask(rt)->sa_len < 2))
                goto create;
        /*
         * Don't listen to the redirect if it's
         * for a route to an interface.
         */
        if (rt->rt_flags & RTF_GATEWAY) {
                if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
                        /*
                         * Changing from route to net => route to host.
                         * Create new route, rather than smashing route to net.
                         */
                create:
                        flags |=  RTF_GATEWAY | RTF_DYNAMIC;
                        error = rtrequest((int)RTM_ADD, dst, gateway,
                                    netmask, flags,
                                    (struct rtentry **)0);
                        stat = &rtstat.rts_dynamic;
                } else {
                        /*
                         * Smash the current notion of the gateway to
                         * this destination.  Should check about netmask!!!
                         */
                        rt->rt_flags |= RTF_MODIFIED;
                        flags |= RTF_MODIFIED;
                        stat = &rtstat.rts_newgateway;
                        /*
                         * add the key and gateway (in one malloc'd chunk).
                         */
                        rt_setgate(rt, rt_key(rt), gateway);
                }
        } else
                error = EHOSTUNREACH;
done:
        if (rt) {
                if (rtp && !error)
                        *rtp = rt;
                else
                        rtfree(rt);
        }
out:
        if (error)
                rtstat.rts_badredirect++;
        else if (stat != NULL)
                (*stat)++;
        memset((caddr_t)&info, 0, sizeof(info));
        info.rti_info[RTAX_DST] = dst;
        info.rti_info[RTAX_GATEWAY] = gateway;
        info.rti_info[RTAX_NETMASK] = netmask;
        info.rti_info[RTAX_AUTHOR] = src;

        if (rtMissMsgHook != NULL)
                ( *rtMissMsgHook) (RTM_REDIRECT, &info, flags, error);

}

/*******************************************************************************
 * rtioctl - Routing table ioctl interface
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int
rtioctl(unsigned long req, void *data)
{
        return ENXIO;
}

/*******************************************************************************
 * ifa_ifwithroute -
 * Find interface with route
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

struct ifaddr *
ifa_ifwithroute(int flags, struct sockaddr *dst, struct sockaddr *gateway)
{
        struct ifaddr *ifa, *oifa;

        if ((flags & RTF_GATEWAY) == 0) {
                /*
                 * If we are adding a route to an interface,
                 * and the interface is a pt to pt link
                 * we should search for the destination
                 * as our clue to the interface.  Otherwise
                 * we can use the local address.
                 */
                ifa = 0;
                if (flags & RTF_HOST) {
                        ifa = ifa_ifwithdstaddr(dst);
                }
                if (ifa == 0)
                        ifa = ifa_ifwithaddr(gateway);
        } else {
                /*
                 * If we are adding a route to a remote net
                 * or host, the gateway may still be on the
                 * other end of a pt to pt link.
                 */
                ifa = ifa_ifwithdstaddr(gateway);
        }
        if (ifa == 0)
                ifa = ifa_ifwithnet(gateway);
        if (ifa == 0) {
                struct rtentry *rt = rtalloc1(gateway, 0, 0UL);
                if (rt == 0)
                        return (0);
                rt->rt_refcnt--;
                if ((ifa = rt->rt_ifa) == 0)
                        return (0);
        }

        if (ifa->ifa_addr->sa_family != dst->sa_family) {

                oifa = ifa;
                ifa = ifaof_ifpforaddr(dst, ifa->ifa_ifp);
                if (ifa == NULL)
                        ifa = oifa;

        }

        return (ifa);
}

struct rtfc_arg {
        struct rtentry *rt0;
        struct radix_node_head *rnh;
};

/*******************************************************************************
 * rtrequest -
 * Do appropriate manipulations of a routing tree given
 * all the bits of info needed
 *
 * RETURNS: Pointer to interface
 ******************************************************************************/

int
rtrequest(int req,
          struct sockaddr *dst,
          struct sockaddr *gateway,
          struct sockaddr *netmask,
          int flags,
          struct rtentry **ret_nrt)
{
        int s = splnet(); int error = 0;
        struct rtentry *rt;
        struct radix_node *rn;
        struct radix_node_head *rnh;
        struct ifaddr *ifa;
        struct sockaddr *ndst;
#define senderr(x) { error = x ; goto bad; }

        /*
         * Find the correct routing tree to use for this Address Family
         */
        if ((rnh = rt_tables[dst->sa_family]) == 0)
                senderr(ESRCH);
        /*
         * If we are adding a host route then we don't want to put
         * a netmask in the tree, nor do we want to clone it.
         */
        if (flags & RTF_HOST) {
                netmask = 0;
        }

        if (netmask != NULL)
          TOS_SET(netmask, 0x1f);

        switch (req) {
        case RTM_DELETE:
                /*
                 * Remove the item from the tree and return it.
                 * Complain if it is not there and do no more processing.
                 */
                if ((rn = (struct radix_node *) ( *rnh->rnh_deladdr)
                                                (dst, netmask, rnh)) == 0)
                        senderr(ESRCH);
                if (rn->rn_flags & (RNF_ACTIVE | RNF_ROOT))
                        panic ("rtrequest delete");
                rt = (struct rtentry *)rn;

                /*
                 * Now search what's left of the subtree for any cloned
                 * routes which might have been formed from this node.
                 */
                if ( (dst->sa_family == AF_INET) &&
                     (rt->rt_flags & RTF_CLONING) &&
                     (netmask != NULL) )
                        (* rnh->rnh_walktree_from)(rnh, dst, netmask,
                                                   rt_fixdelete, rt);

                /*
                 * NB: RTF_UP must be set during the search above,
                 * because we might delete the last ref, causing
                 * rt to get freed prematurely.
                 *  eh? then why not just add a reference?
                 * I'm not sure how RTF_UP helps matters. (JRE)
                 */
                rt->rt_flags &= ~RTF_UP;

                /*
                 * Remove any external references we may have.
                 * This might result in another rtentry being freed if
                 * we held its last reference.
                 */
                if (rt->rt_gwroute) {
                        rt = rt->rt_gwroute;
                        RTFREE(rt);
                        (rt = (struct rtentry *)rn)->rt_gwroute = 0;
                }

                /*
                 * give the protocol a chance to keep things in sync.
                 */
                if ((ifa = rt->rt_ifa) && ifa->ifa_rtrequest)
                        ( *ifa->ifa_rtrequest) (RTM_DELETE, rt, SA(0));

                /*
                 * one more rtentry floating around that is not
                 * linked to the routing table.
                 */
                rttrash++;

                /*
                 * If the caller wants it, then it can have it,
                 * but it's up to it to free the rtentry as we won't be
                 * doing it.
                 */
                if (ret_nrt)
                        *ret_nrt = rt;
                else if (rt->rt_refcnt <= 0) {
                        rt->rt_refcnt++; /* make a 1->0 transition */
                        rtfree(rt);
                }
                break;

        case RTM_RESOLVE:
                if (ret_nrt == 0 || (rt = *ret_nrt) == 0)
                        senderr(EINVAL);
                ifa = rt->rt_ifa;
                flags = rt->rt_flags & ~RTF_CLONING;
                gateway = rt->rt_gateway;
                if ((netmask = rt->rt_genmask) == 0)
                        flags |= RTF_HOST;
                goto makeroute;

        case RTM_ADD:

                if ((ifa = ifa_ifwithroute(flags, dst, gateway)) == 0)
                        senderr(ENETUNREACH);

        makeroute:

                R_Malloc(rt, struct rtentry *, sizeof(ROUTE_ENTRY));
                if (rt == 0)
                        senderr(ENOBUFS);
                memset(rt, 0, sizeof(ROUTE_ENTRY));
                rt->rt_flags = RTF_UP | flags;

                /*
                 * Add the gateway. Possibly re-malloc-ing the storage for it
                 * also add the rt_gwroute if possible.
                 */
                if ((error = rt_setgate(rt, dst, gateway)) != 0) {
                        Free(rt);
                        senderr(error);
                }

                /*
                 * point to the (possibly newly malloc'd) dest address.
                 */
                ndst = rt_key(rt);

                /*
                 * make sure it contains the value we want (masked if needed).
                 */
                if (netmask) {
                        rt_maskedcopy(dst, ndst, netmask);
                } else
                        memcpy(ndst, dst, dst->sa_len);

                if (ndst->sa_family == AF_INET &&
                    !IN_MULTICAST(
                     ntohl( ((struct sockaddr_in *) ndst)->sin_addr.s_addr)) &&
                    !(rt->rt_flags & RTF_HOST) ) {

                  if ( ((struct sockaddr_in *) netmask)->sin_addr.s_addr !=
                       0xffffffff)
                    rt->rt_flags |= RTF_CLONING;

                }

                /* Add route node */
                rn = (struct radix_node *) ( *rnh->rnh_addaddr)
                                           ((caddr_t)ndst, (caddr_t)netmask,
                                            rnh, rt->rt_nodes);

                /* Update protocol */
                if ( (req == RTM_ADD) && (gateway->sa_family == AF_INET) )
                  RT_PROTO_SET( ndst, RT_PROTO_GET(dst) );

                /*
                 * If it still failed to go into the tree,
                 * then un-make it (this should be a function)
                 */
                if (rn == 0) {
                        if (rt->rt_gwroute)
                                rtfree(rt->rt_gwroute);
                        Free(rt_key(rt));
                        Free(rt);
                        senderr(EEXIST);
                }

                /* Update interface */
                ifa->ifa_refcnt++;
                rt->rt_ifa = ifa;
                rt->rt_ifp = ifa->ifa_ifp;

                if ( (rt->rt_ifp->if_flags & IFF_UP) == 0 ) {

                  rt->rt_flags |= RTF_BLACKHOLE;
                  rn->rn_flags |= RNF_BLACKHOLE;

                }

                /*
                 * If we got here from RESOLVE, then we are cloning
                 * so clone the rest, and note that we
                 * are a clone (and increment the parent's references)
                 */
                if (req == RTM_RESOLVE) {
                        rt->rt_rmx = (*ret_nrt)->rt_rmx; /* copy metrics */
                        rt->rt_parent = (*ret_nrt);
                        (*ret_nrt)->rt_refcnt++;
                        rt->rt_flags |= RTF_CLONED;
                        ((ROUTE_ENTRY *) rt)->primaryRouteFlag = TRUE;
                }

                if (rt->rt_ifp->if_type == IFT_PMP)
                  rt->rt_flags &= ~RTF_CLONING;

                if (!rt->rt_rmx.rmx_mtu &&
                    !(rt->rt_rmx.rmx_locks & RTV_MTU) &&
                    rt->rt_ifp)
                  rt->rt_rmx.rmx_mtu = rt->rt_ifp->if_mtu;

                /*
                 * if this protocol has something to add to this then
                 * allow it to do that as well.
                 */
                if (ifa->ifa_rtrequest)
                        ( *ifa->ifa_rtrequest)
                                (req, rt, SA(ret_nrt ? *ret_nrt : 0));

                /*
                 * We repeat the same procedure from rt_setgate() here because
                 * it doesn't fire when we call it there because the node
                 * hasn't been added to the tree yet.
                 */
                if (ndst->sa_family == AF_INET) {

                  if (!(rt->rt_flags & RTF_HOST) &&
                      rt_mask(rt) != 0 &&
                      ((struct sockaddr_in *) ndst)->sin_addr.s_addr != 0)
                        ( *rnh->rnh_walktree_from)(rnh, ndst, netmask,
                                                   rt_fixchange, rt);

                }

                /*
                 * actually return a resultant rtentry and
                 * give the caller a single reference.
                 */
                if (ret_nrt) {
                        *ret_nrt = rt;
                        rt->rt_refcnt++;
                }

                rt->rt_mod = tickGet();

                break;
        }
bad:
        splx(s);
        return (error);
}

/*******************************************************************************
 * rt_fixdelete -
 * Called from rtrequest(RTM_DELETE, ...) to fix up the route's ``family''
 * (i.e., the routes related to it by the operation of cloning).  This
 * routine is iterated over all potential former-child-routes by way of
 * rnh->rnh_walktree_from() above, and those that actually are children of
 * the late parent (passed in as VP here) are themselves deleted.
 *
 * RETURNS: One or zero
 ******************************************************************************/

LOCAL int
rt_fixdelete(struct radix_node *rn, void *vp)
{
        struct rtentry *rt = (struct rtentry *)rn;
        struct rtentry *rt0 = vp;

        if (rt->rt_parent == rt0 && !(rt->rt_flags & RTF_PINNED)) {
                return rtrequest(RTM_DELETE, rt_key(rt),
                                 (struct sockaddr *)0, rt_mask(rt),
                                 rt->rt_flags, (struct rtentry **)0);
        }
        return 0;
}

/*
 * This routine is called from rt_setgate() to do the analogous thing for
 * adds and changes.  There is the added complication in this case of a
 * middle insert; i.e., insertion of a new network route between an older
 * network route and (cloned) host routes.  For this reason, a simple check
 * of rt->rt_parent is insufficient; each candidate route must be tested
 * against the (mask, value) of the new route (passed as before in vp)
 * to see if the new route matches it.
 *
 * XXX - it may be possible to do fixdelete() for changes and reserve this
 * routine just for adds.  I'm not sure why I thought it was necessary to do
 * changes this way.
 */
#ifdef DEBUG
LOCAL int rtfcdebug = 0;
#endif

/*******************************************************************************
 * rt_fixchange - Fix change
 *
 * RETURNS: One or zero
 ******************************************************************************/

LOCAL int
rt_fixchange(struct radix_node *rn, void *vp)
{
        struct rtentry *rt = (struct rtentry *)rn;
        struct rtfc_arg *ap = vp;
        struct rtentry *rt0 = ap->rt0;
        struct radix_node_head *rnh = ap->rnh;
        u_char *xk1, *xm1, *xk2, *xmp;
        int i, len, mlen;

#ifdef DEBUG
        if (rtfcdebug)
                printf("rt_fixchange: rt %p, rt0 %p\n", rt, rt0);
#endif

        if (!rt->rt_parent || (rt->rt_flags & RTF_PINNED)) {
#ifdef DEBUG
                if(rtfcdebug) printf("no parent or pinned\n");
#endif
                return 0;
        }

        if (rt->rt_parent == rt0) {
#ifdef DEBUG
                if(rtfcdebug) printf("parent match\n");
#endif
                return rtrequest(RTM_DELETE, rt_key(rt),
                                 (struct sockaddr *)0, rt_mask(rt),
                                 rt->rt_flags, (struct rtentry **)0);
        }

        /*
         * There probably is a function somewhere which does this...
         * if not, there should be.
         */
        len = min(((struct sockaddr *)rt_key(rt0))->sa_len,
                   ((struct sockaddr *)rt_key(rt))->sa_len);

        xk1 = (u_char *)rt_key(rt0);
        xm1 = (u_char *)rt_mask(rt0);
        xk2 = (u_char *)rt_key(rt);

        /* avoid applying a less specific route */
        xmp = (u_char *)rt_mask(rt->rt_parent);
        mlen = ((struct sockaddr *)rt_key(rt->rt_parent))->sa_len;
        if (mlen > ((struct sockaddr *)rt_key(rt0))->sa_len) {
#ifdef DEBUG
                if (rtfcdebug)
                        printf("rt_fixchange: inserting a less "
                               "specific route\n");
#endif
                return 0;
        }
        for (i = rnh->rnh_treetop->rn_off; i < mlen; i++) {
                if ((xmp[i] & ~(xmp[i] ^ xm1[i])) != xmp[i]) {
#ifdef DEBUG
                        if (rtfcdebug)
                                printf("rt_fixchange: inserting a less "
                                       "specific route\n");
#endif
                        return 0;
                }
        }

        for (i = rnh->rnh_treetop->rn_off; i < len; i++) {
                if ((xk2[i] & xm1[i]) != xk1[i]) {
#ifdef DEBUG
                        if(rtfcdebug) printf("no match\n");
#endif
                        return 0;
                }
        }

        /*
         * OK, this node is a clone, and matches the node currently being
         * changed/added under the node's mask.  So, get rid of it.
         */
#ifdef DEBUG
        if(rtfcdebug) printf("deleting\n");
#endif
        return rtrequest(RTM_DELETE, rt_key(rt), (struct sockaddr *)0,
                         rt_mask(rt), rt->rt_flags, (struct rtentry **)0);
}

/*******************************************************************************
 * rt_setgate - Set gateway
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int
rt_setgate(struct rtentry *rt0, struct sockaddr *dst, struct sockaddr *gate)
{
        caddr_t new, old;
        int dlen = ROUNDUP(dst->sa_len), glen = ROUNDUP(gate->sa_len);
        struct rtentry *rt = rt0;
        struct radix_node_head *rnh = rt_tables[dst->sa_family];

        /*
         * A host route with the destination equal to the gateway
         * will interfere with keeping LLINFO in the routing
         * table, so disallow it.
         */
        if (((rt0->rt_flags & (RTF_HOST|RTF_GATEWAY|RTF_LLINFO)) ==
                                        (RTF_HOST|RTF_GATEWAY)) &&
            (dst->sa_len == gate->sa_len) &&
            (memcmp(dst, gate, dst->sa_len) == 0))
                return EADDRNOTAVAIL;

        /*
         * Both dst and gateway are stored in the same malloc'd chunk
         * (If I ever get my hands on....)
         * if we need to malloc a new chunk, then keep the old one around
         * till we don't need it any more.
         */
        if (rt->rt_gateway == 0 || glen > ROUNDUP(rt->rt_gateway->sa_len)) {

                old = (caddr_t)rt_key(rt);

                R_Malloc(new, caddr_t, dlen + glen);
                if (new == 0)
                        return ENOBUFS;

                rt->rt_nodes->rn_key = new;

        }

        else {

                /*
                 * otherwise just overwrite the old one
                 */
                new = rt->rt_nodes->rn_key;
                old = 0;

        }

        /*
         * copy the new gateway value into the memory chunk
         */
        memcpy((rt->rt_gateway = (struct sockaddr *)(new + dlen)), gate, glen);

        /*
         * if we are replacing the chunk (or it's new) we need to
         * replace the dst as well
         */
        if (old) {

                memcpy(new, dst, dlen);
                Free(old);

        }

        /*
         * If there is already a gwroute, it's now almost definitly wrong
         * so drop it.
         */
        if (rt->rt_gwroute) {

                rt = rt->rt_gwroute;
                RTFREE(rt);
                rt = rt0; rt->rt_gwroute = 0;

        }

        if (rt->rt_flags & RTF_GATEWAY)
                rt->rt_gwroute = rtalloc1(gate, 1, 0);

        return 0;
}

/*******************************************************************************
 * rt_maskedcopy - Make a masked copy
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void
rt_maskedcopy(struct sockaddr *src,
              struct sockaddr *dst,
              struct sockaddr *netmask)
{
        u_char *cp1 = (u_char *)src;
        u_char *cp2 = (u_char *)dst;
        u_char *cp3 = (u_char *)netmask;
        u_char *cplim = cp2 + *cp3;
        u_char *cplim2 = cp2 + *cp1;

        *cp2++ = *cp1++; *cp2++ = *cp1++; /* copies sa_len & sa_family */
        cp3 += 2;
        if (cplim > cplim2)
                cplim = cplim2;
        while (cp2 < cplim)
                *cp2++ = *cp1++ & *cp3++;
        if (cp2 < cplim2)
                memset((caddr_t)cp2, 0, (unsigned)(cplim2 - cp2));
}

/*******************************************************************************
 * rtinit -
 * Set up a routing table entry, normally
 * for an interface.
 *
 * RETURNS: N/A
 ******************************************************************************/

int
rtinit(struct ifaddr *ifa, int cmd, int flags)
{
        struct rtentry *rt;
        struct sockaddr *dst;
        struct sockaddr *deldst;
        struct mbuf *m = 0;
        struct rtentry *nrt = 0;
        int error = 0;

        dst = flags & RTF_HOST ? ifa->ifa_dstaddr : ifa->ifa_addr;
        /*
         * If it's a delete, check that if it exists, it's on the correct
         * interface or we might scrub a route to another ifa which would
         * be confusing at best and possibly worse.
         */
        if (cmd == RTM_DELETE) {
                /*
                 * It's a delete, so it should already exist..
                 * If it's a net, mask off the host bits
                 * (Assuming we have a mask)
                 */
                if ((flags & RTF_HOST) == 0 && ifa->ifa_netmask) {
                        m = m_get(M_DONTWAIT, MT_SONAME, CL_SIZE_128, TRUE);
                        if (m == NULL)
                                return(ENOBUFS);
                        deldst = mtod(m, struct sockaddr *);
                        rt_maskedcopy(dst, deldst, ifa->ifa_netmask);
                        dst = deldst;
                }
                /*
                 * Get an rtentry that is in the routing tree and
                 * contains the correct info. (if this fails, can't get there).
                 * We set "report" to FALSE so that if it doesn't exist,
                 * it doesn't report an error or clone a route, etc. etc.
                 */
                rt = rtalloc1(dst, 0, FALSE);
                if (rt) {
                        /*
                         * Ok so we found the rtentry. it has an extra reference
                         * for us at this stage. we won't need that so
                         * lop that off now.
                         */
                        rt->rt_refcnt--;
                        if (rt->rt_ifa != ifa) {
                                /*
                                 * If the interface in the rtentry doesn't match
                                 * the interface we are using, then we don't
                                 * want to delete it, so return an error.
                                 * This seems to be the only point of
                                 * this whole RTM_DELETE clause.
                                 */
                                if (m)
                                        (void) m_free(m);
                                return (flags & RTF_HOST ? EHOSTUNREACH
                                                        : ENETUNREACH);
                        }
                }
        }

        /*
         * Do the actual request
         */

        /* Select command */
        switch(cmd) {

          case RTM_DELETE:

            error = rtrequest(cmd, dst, ifa->ifa_addr, ifa->ifa_netmask,
                              flags | ifa->ifa_flags, &nrt);

          break;

          case RTM_ADD:

            error = rtrequestAddEqui(dst, ifa->ifa_netmask, ifa->ifa_addr,
                                     flags | ifa->ifa_flags,
                                     M2_ipRouteProto_local, 0, FALSE, FALSE,
                                     (ROUTE_ENTRY **) &nrt);

            break;

        } /* End select command */

        if (m)
                (void) m_free(m);

        /*
         * If we are deleting, and we found an entry, then
         * it's been removed from the tree.. now throw it away.
         */
        if (cmd == RTM_DELETE && error == 0 && (rt = nrt)) {

                /*
                 * notify any listenning routing agents of the change
                 */
                if (rtNewAddrMsgHook != NULL)
                        ( *rtNewAddrMsgHook) (cmd, ifa, error, nrt);

                if (rt->rt_refcnt <= 0) {

                        rt->rt_refcnt++; /* need a 1->0 transition to free */
                        rtfree(rt);

                }

        }

        /*
         * We are adding, and we have a returned routing entry.
         * We need to sanity check the result.
         */
        if (cmd == RTM_ADD && error == 0 && (rt = nrt)) {
                /*
                 * We just wanted to add it.. we don't actually need a reference
                 */
                rt->rt_refcnt--;
                /*
                 * If it came back with an unexpected interface, then it must
                 * have already existed or something. (XXX)
                 */
                if (rt->rt_ifa != ifa) {
                        /*
                         * Ask that the protocol in question
                         * remove anything it has associated with
                         * this route and ifaddr.
                         */
                        if (rt->rt_ifa->ifa_rtrequest)
                            ( *rt->rt_ifa->ifa_rtrequest)
                                (RTM_DELETE, rt, SA(0));
                        /*
                         * Remove the reference to its ifaddr.
                         */
                        IFAFREE(rt->rt_ifa);
                        /*
                         * And substitute in references to the ifaddr
                         * we are adding.
                         */
                        rt->rt_ifa = ifa;
                        rt->rt_ifp = ifa->ifa_ifp;
                        ifa->ifa_refcnt++;
                        /*
                         * Now ask the protocol to check if it needs
                         * any special processing in its new form.
                         */
                        if (ifa->ifa_rtrequest)
                            ifa->ifa_rtrequest(RTM_ADD, rt, SA(0));
                }
                /*
                 * notify any listenning routing agents of the change
                 */
                if (rtNewAddrMsgHook != NULL)
                        ( *rtNewAddrMsgHook) (cmd, ifa, error, nrt);
        }
        return (error);
}

/*******************************************************************************
 * rtrequestAddEqui - Route creation
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS rtrequestAddEqui(struct sockaddr *pDest,
                        struct sockaddr *pNetMask,
                        struct sockaddr *pGateway,
                        short flags,
                        short protoId,
                        unsigned char weight,
                        BOOL notifyFlag,
                        BOOL socketFlag,
                        ROUTE_ENTRY **dpRouteEntry)
{
  int err;
  struct sockaddr dest;

  /* Copy address to struct */
  memcpy( &dest, pDest, sizeof(struct sockaddr) );

  /* Set protocol id */
  RT_PROTO_SET(&dest, protoId);

  /* Make general request */
  err = rtrequest(RTM_ADD, &dest, pGateway, pNetMask, flags,
                  (struct rtentry **) dpRouteEntry);
  if (err) {

    errnoSet(err);
    return ERROR;

  }

  return OK;
}

