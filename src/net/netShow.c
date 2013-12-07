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

/* netShow.c - Show network information */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <net/mbuf.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/inetLib.h>

/* Defines */
#define RT_SHOW_NET                             0x001
#define RT_SHOW_HST                             0x010
#define RT_SHOW_ARP                             0x100

/* Imports */
IMPORT struct domain *domains;
IMPORT struct ifnet *ifnet;
IMPORT struct radix_node_head *rt_tables[];

/* Locals */
LOCAL char afName[AF_MAX][14] = {
  "AF_UNSPEC",
  "AF_LOCAL",
  "AF_INET",
  "AF_IMPLINK",
  "AF_PUP",
  "AF_CHAOS",
  "AF_NS",
  "AF_OSI",
  "AF_ECMA",
  "AF_DATAKIT",
  "AF_CCITT",
  "AF_SNA",
  "AF_DECnet",
  "AF_DLI",
  "AF_LAT",
  "AF_HYLINK",
  "AF_APPLETALK",
  "AF_ROUTE",
  "AF_LINK",
  "AF_XTP",
  "AF_COIP",
  "AF_CNT",
  "AF_RTIP",
  "AF_IPX",
  "AF_SIP",
  "AF_PIP"
};

LOCAL char sockName[SOCK_MAX][14] = {
  "SOCK_UNSPEC",
  "SOCK_STREAM",
  "SOCK_DGRAM",
  "SOCK_RAW",
  "SOCK_RDM",
  "SOCK_SEQPACKET"
};

LOCAL char sepLine[] = "-------------------------------------------------------"
                       "------------------------\n";

LOCAL void clPoolShow(NET_POOL_ID poolId);
LOCAL void ifInetAddrPrint(char *str, struct sockaddr *addr);
LOCAL BOOL routeEntryPrint(struct rtentry *pRoute,
                           void *pRouteType,
                           BOOL ipRouteFlag);
LOCAL int routeNodeShow(struct radix_node *pNode, void *w);
LOCAL void routeTableShow(int routeType, int af);

/******************************************************************************
* clPoolShow - Print cluster pool information
*
* RETURNS: N/A
******************************************************************************/

LOCAL void clPoolShow(NET_POOL_ID poolId)
{
  unsigned char type;
  CL_POOL_ID clPoolId;

  /* Print header */
  printf("__________________\n");
  printf("CLUSTER POOL TABLE\n");
  printf(sepLine);
  printf("size     clusters  free      usage\n");
  printf(sepLine);

  /* For all cluster types */
  for (type = poolId->clLg2Min; type <= poolId->clLg2Max; type++) {

    /* Get cluster pool */
    clPoolId = netClPoolIdGet(poolId, CL_LOG2_TO_CL_SIZE(type), TRUE);

    /* Print if available */
    if (clPoolId != NULL) {

      printf("%-9d", clPoolId->clSize);
      printf("%-10d", clPoolId->clNum);
      printf("%-10d", clPoolId->clNumFree);
      printf("%-14d\n", clPoolId->clUsage);
    }

    printf(sepLine);

  } /* End for all cluster types */

}

/******************************************************************************
* netPoolShow - Print network pool information
*
* RETURNS: N/A
******************************************************************************/

void netPoolShow(NET_POOL_ID poolId)
{
  static int mtTypes[NUM_MBLK_TYPES] = {
    MT_FREE,    MT_DATA,    MT_HEADER,  MT_SOCKET,
    MT_PCB,     MT_RTABLE,  MT_HTABLE,  MT_ATABLE,
    MT_SONAME,  MT_ZOMBIE,  MT_SOOPTS,  MT_FTABLE,
    MT_RIGHTS,  MT_IFADDR,  MT_CONTROL, MT_OOBDATA,
    MT_IPMOPTS, MT_IPMADDR, MT_IFMADDR, MT_MRTABLE
  };

  static char mtNames[NUM_MBLK_TYPES][12] = {
    "FREE",    "DATA",    "HEADER",  "SOCKET",
    "PCB",     "RTABLE",  "HTABLE",  "ATABLE",
    "SONAME",  "ZOMBIE",  "SOOPTS",  "FTABLE",
    "RIGHTS",  "IFADDR",  "CONTROL", "OOBDATA",
    "IPMOPTS", "IPMADDR", "IFMADDR", "MRTABLE"
  };

  int i, sum;

  /* Return if nothing to print */
  if (poolId == NULL || poolId->pPoolStat == NULL)
    return ;

  /* Initialize local variables */
  sum = 0;

  /* Print header */
  printf("type        number\n");
  printf("---------   ------\n");

  /* For all block types */
  for (i = 0; i < NUM_MBLK_TYPES; i++) {

    printf("%-8s:    %3ld\n",
        mtNames[i], poolId->pPoolStat->mTypes[mtTypes[i]]);
    sum += poolId->pPoolStat->mTypes[mtTypes[i]];

  } /* End for all block types */

  /* Print sum of blocks */
  printf("%-8s:    %3d\n", "TOTAL", sum);

  /* Print other net pool info */
  printf("number of mbufs: %ld\n", poolId->pPoolStat->mNum);
  printf("number of times failed to find space: %ld\n",
        poolId->pPoolStat->mDrops);
  printf("number of times waited for space: %ld\n",
        poolId->pPoolStat->mDrops);
  printf("number of times drained protocols for space: %ld\n",
        poolId->pPoolStat->mDrain);

  /* Print cluster pool info */
  clPoolShow(poolId);
}

/******************************************************************************
* ifAddrPrint - Print interface address
*
* RETURNS: N/A
******************************************************************************/

LOCAL void ifInetAddrPrint(char *str, struct sockaddr *addr)
{
  char tmp[INET_ADDR_LEN];

  /* Get internet address string representation */
  inet_ntoa_b(((struct sockaddr_in *) addr)->sin_addr, tmp);

  /* Print address */
  printf("     %s: %s\n", str, tmp);
}

/******************************************************************************
* ifFlagsPrint - Print interface flags
*
* RETURNS: N/A
******************************************************************************/

LOCAL void ifFlagsPrint(unsigned short flags)
{
  printf("     Flags: (%#x) ", flags);

  if (flags & IFF_UP)
    printf("UP ");
  else
    printf("DOWN ");

  if (flags & IFF_LOOPBACK)
    printf("LOOPBACK ");

  if (flags & IFF_POINTOPOINT)
    printf("POINT-TO-POINT ");

  if (flags & IFF_BROADCAST)
    printf("BROADCAST ");

  if (flags & IFF_MULTICAST)
    printf("MULTICAST ");

  if (flags & IFF_PROMISC)
    printf("PROMISCUOUS ");

  if ( (flags & IFF_NOTRAILERS) == 0 )
    printf("TRAILERS ");

  if ( (flags & IFF_NOARP) == 0 )
    printf("ARP ");

  if (flags & IFF_RUNNING)
    printf("RUNNING ");

  if (flags & IFF_DEBUG)
    printf("DEBUG ");

  if (flags & IFF_FP_ENABLE)
    printf("FASTPATH ");

  if (flags & IFF_DONT_FORWARD)
    printf("NOFORWARD ");

  if ( ((flags & IFF_NOARP) == 0) && (flags & IFF_PROXY_ALL) )
    printf("PROXY-ALL");

  printf("\n");
}

/******************************************************************************
* ifShow - Show network interface info
*
* RETURNS: N/A
******************************************************************************/

void ifShow(char *ifName)
{
  struct ifnet *ifp;
  struct ifaddr *ifa;
  struct in_ifaddr *ia;
  int len, unit;
  BOOL found;

  /* Initialize locals */
  ia = NULL;
  found = FALSE;

  /* For all interfaces */
  for (ifp = ifnet; ifp != NULL; ifp = ifp->if_next) {

    /* Get name length */
    len = strlen(ifp->if_name);

    /* If interface argument given */
    if (ifName != NULL) {

      /* If names do not match */
      if ( strncmp(ifName, ifp->if_name, len) != 0 )
        continue;

      /* If interface name length is lt. argument */
      if ( len < strlen(ifName) ) {

        /* Get unit number */
        sscanf(&ifName[len], "%d", &unit);

        /* If unit number mismatch */
        if (ifp->if_unit != unit)
          continue;

      } /* End if interface name length is lt. argument */

    } /* End if interface argument given */

    /* If here intraface found */
    found = TRUE;

    /* Print interface header */
    printf("%s (unit number %d):\n", ifp->if_name, ifp->if_unit);

    /* Print flags */
    ifFlagsPrint(ifp->if_flags);

    /* For all addresses for interface */
    for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next) {

      /* If internet address family */
      if (ifa->ifa_addr->sa_family == AF_INET) {

        /* Get internet address */
        ia = (struct in_ifaddr *) ifa;

        /* Print address */
        ifInetAddrPrint("Internet address", ifa->ifa_addr);

        /* If destination address */
        if (ifp->if_flags & IFF_POINTOPOINT)
          ifInetAddrPrint("Destination Internet address", ifa->ifa_dstaddr);

        /* Else if broadcast address */
        else if (ifp->if_flags & IFF_BROADCAST)
          ifInetAddrPrint("Broadcast address", ifa->ifa_broadaddr);

        /* Print netmask */
        printf("     Netmask 0x%lx Subnetmask 0x%lx\n",
               ia->ia_netmask, ia->ia_subnetmask);

      } /* End if internet address family */

    } /* End for all addresses for interface */

    /* Print metric and mtu */
    printf("     Metric is %lu\n", ifp->if_metric);
    printf("     Maximum Transfer Unit size is %lu\n", ifp->if_mtu);

    /* Print status */
    printf("     %lu packets received; %lu packets sent\n",
           ifp->if_ipackets, ifp->if_opackets);
    printf("     %lu multicast packets received\n", ifp->if_imcasts);
    printf("     %lu multicast packets sent\n", ifp->if_omcasts);
    printf("     %lu input errors; %lu output errors\n",
           ifp->if_ierrors, ifp->if_oerrors);

  } /* End for all interfaces */

  /* If interface not found */
  if (!found) {

    if (ifName != NULL)
      fprintf(stderr, "\"%s\" - No such interface\n", ifName);
    else
      fprintf(stderr, "No network interface active.\n");

  } /* End if interface not found */
}

/******************************************************************************
* routeEntryPrint - Print route entry
*
* RETURNS: TRUE or FALSE
******************************************************************************/

LOCAL BOOL routeEntryPrint(struct rtentry *pRoute,
                           void *pRouteType,
                           BOOL ipRouteFlag)
{
  struct sockaddr *dest, *gateway;
  int type;
  char str[INET_ADDR_LEN];
  unsigned long mask;

  /* Initialize locals */
  dest = NULL;
  type = *((int *) pRouteType);

  /* If host routes and only network requested */
  if ( (type == RT_SHOW_NET) && (pRoute->rt_flags & RTF_HOST) )
    return FALSE;

  /* Get values */
  dest = rt_key(pRoute);
  gateway = pRoute->rt_gateway;

  /* If host but not arp requested */
  if ( (type & RT_SHOW_HST) &&
       (gateway->sa_family == AF_LINK) &&
       (pRoute->rt_flags & RTF_HOST) )
    return FALSE;

  /* If only host requested */
  if ( (type == RT_SHOW_HST) &&
       ((pRoute->rt_flags & RTF_HOST) == 0) )
    return FALSE;

  /* If arp requested and invalid link address */
  if ( (type & RT_SHOW_ARP) &&
       ((gateway->sa_family != AF_LINK) ||
        (((struct sockaddr_dl *) gateway)->sdl_alen == 0)) )
    return FALSE;

  /* If net requested and gateway address family is link */
  if ( (type & RT_SHOW_NET) &&
       (gateway->sa_family == AF_LINK) )
    gateway = pRoute->rt_ifa->ifa_addr;

  /* If ip route request */
  if (ipRouteFlag) {

    /* If internet address */
    if (dest->sa_family == AF_INET) {

      inet_ntoa_b(((struct sockaddr_in *) dest)->sin_addr, str);
      printf("%-16s ", str);

    } /* End if internet address */

    /* Else non-internet address */
    else {

      printf("%-16s ", "not AF_INET");

    } /* End else non-internet address */

  } /* End if ip route request */

  /* Else non-ip route request */
  else {

    printf("                 ");

  } /* End else non-ip route request */

  /* If both net and host requested */
  if ( (type & RT_SHOW_NET) &&
       (type & RT_SHOW_HST) ) {

    /* If ip route request */
    if (ipRouteFlag) {

      /* If null mask */
      if ( rt_mask(pRoute) == NULL) {

        mask = 0x0;

      } /* End if null mask */

      /* Else non-null mask */
      else {

        mask = ((struct sockaddr_in *) rt_mask(pRoute))->sin_addr.s_addr;
        mask = ntohl(mask);

      } /* End else non-null mask */

      /* Print mask */
      printf("%#-10lx ", mask);

    } /* End if ip route request */

    /* Else non-ip route request */
    else {

      printf("           ");

    } /* End else non-ip route request */

  } /* End if both net and host requested */

  /* If link level gateway */
  if (gateway->sa_family == AF_LINK) {

    printf("                 ");

  } /* End if link level gateway */

  /* Else non-link gateway */
  else {

    /* If internet address */
    if (gateway->sa_family == AF_INET) {

      inet_ntoa_b(((struct sockaddr_in *) gateway)->sin_addr, str);
      printf("%-16s ", str);

    } /* End if internet address */

    /* Else non-internet address */
    else {

      printf("%-16s ", "not AF_INET");

    } /* End else non-internet address */

  } /* End else non-link gateway */

  /* Print the rest */
  printf("%#-6hx ", pRoute->rt_flags);
  printf("%-5d  ", pRoute->rt_refcnt);
  printf("%-10ld ", pRoute->rt_use);
  printf("%s%d\n", pRoute->rt_ifp->if_name, pRoute->rt_ifp->if_unit);

  return TRUE;
}

/******************************************************************************
* routeNodeShow - Show route node
*
* RETURNS: OK or ERROR
******************************************************************************/

LOCAL int routeNodeShow(struct radix_node *pRoute, void *w)
{
  ROUTE_ENTRY *pHead;
  BOOL printFlag;

  /* Get head node */
  pHead = (ROUTE_ENTRY *) pRoute;

  /* Print node */
  printFlag = routeEntryPrint( (struct rtentry *) pHead, w, TRUE);
  if (!printFlag)
    return OK;

  return OK;
}

/******************************************************************************
* routeTableShow - Show route table
*
* RETURNS: N/A
******************************************************************************/

LOCAL void routeTableShow(int routeType, int af)
{
  int type, s;
  struct radix_node_head *rnh;

  /* Setup locals */
  type = routeType;
  rnh = rt_tables[af];

  /* If head dont exists */
  if (rnh == NULL)
    return;

  /* Get processor level */
  s = splnet();

  /* Show routes */
  rn_walktree(rnh, routeNodeShow, &type);

  /* Restore processor level */
  splx(s);
}

/******************************************************************************
* routeShow - Show all routes
*
* RETURNS: N/A
******************************************************************************/

void routeShow(int af)
{
  char *headLine = "Destination      Gateway          Flags  Refcnt Use"
                   "        Interface\n";

  printf("\nROUTE NET TABLE\n");
  printf(headLine);
  printf(sepLine);

  routeTableShow(RT_SHOW_NET, af);

  printf(sepLine);
  printf("\nROUTE HOST TABLE\n");
  printf(headLine);
  printf(sepLine);

  routeTableShow(RT_SHOW_HST, af);

  printf(sepLine);
}

/*******************************************************************************
 * domainShow - Show domains
 *
 * RETURNS: N/A
 ******************************************************************************/

void domainShow(void)
{
  struct domain *dp;
  struct protosw *pr;

  printf(sepLine);

  /* For all domains */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    printf("%s: %s\n", dp->dom_name, afName[dp->dom_family]);

    for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      printf("%s %x\n", sockName[pr->pr_type], pr->pr_usrreq);

    }

    printf(sepLine);

  } /* End for all domains */
}

