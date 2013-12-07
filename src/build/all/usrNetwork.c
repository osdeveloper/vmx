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

/* usrNetwork.c - User network configuration */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <os/unixLib.h>
#include <netinet/in.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/netBufLib.h>
#include <net/netLib.h>
#include <net/mbuf.h>
#include <net/if.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/netShow.h>
#include <net/socketvar.h>
#include <net/sockLib.h>
#include <net/unixDomainLib.h>
#include <netinet/inetLib.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

/* Defines */
#define SOMAXCONN_CFG			5
#define SOCKET_MAX_FD			10

/* Imports */
SOCK_FUNC* bsdSockLibAdd(int domain);

M_CL_CONFIG mClBlkConfig = {
/*num. mBlk		num. clkBlk		memArea		memSize
  ---------		-----------		-------		------- */
  NUM_NET_MBLKS,	NUM_CL_BLKS,		NULL,		0
};

CL_DESC clDescTable[] = {
/*clusterSize		num		memArea		memSize
  -----------		---		-------		------- */
  {64,			NUM_64,		NULL,		0},
  {128,			NUM_128,	NULL,		0},
  {256,			NUM_256,	NULL,		0},
  {512,			NUM_512,	NULL,		0},
  {1024,		NUM_1024,	NULL,		0},
  {2048,		NUM_2048,	NULL,		0},
  {4096,		NUM_4096,	NULL,		0},
  {8192,		NUM_8192,	NULL,		0},
  {16384,		NUM_16384,	NULL,		0},
  {32768,		NUM_32768,	NULL,		0},
  {65536,		NUM_65536,	NULL,		0}
};

int clDescTableNumEntries;

M_CL_CONFIG sysMclBlkConfig = {
/*num. mBlk		num. clkBlk		memArea		memSize
  ---------		-----------		-------		-------*/
  NUM_SYS_MBLKS,	NUM_SYS_CL_BLKS,	NULL,		0
};

CL_DESC sysClDescTable[] = {
/*clusterSize		num		memArea		memSize
  -----------		---		-------		------- */
  {64,			NUM_SYS_64,	NULL,		0},
  {128,			NUM_SYS_128,	NULL,		0},
  {256,			NUM_SYS_256,	NULL,		0},
  {512,			NUM_SYS_512,	NULL,		0}
};

int sysClDescTableNumEntries;

IP_CFG_PARAMS ipCfgParams = {
  IP_FLAGS_DFLT,
  IP_TTL_DFLT,
  IP_QLEN_DFLT,
  IP_FRAG_TTL_DFLT
};

ICMP_CFG_PARAMS icmpCfgParams = {
  ICMP_FLAGS_DFLT
};

RAWIP_CFG_PARAMS rawipCfgParams = {
  RAWIP_SND_SIZE_DFLT,
  RAWIP_RCV_SIZE_DFLT
};

UDP_CFG_PARAMS udpCfgParams = {
  UDP_FLAGS_DFLT,
  UDP_SND_SIZE_DFLT,
  UDP_RCV_SIZE_DFLT
};

TCP_CFG_PARAMS tcpCfgParams = {
  TCP_FLAGS_DFLT,
  TCP_SND_SIZE_DFLT,
  TCP_RCV_SIZE_DFLT,
  TCP_CON_TIMEO_DFLT,
  TCP_REXMT_THLD_DFLT,
  TCP_MSS_DFLT,
  TCP_RND_TRIP_DFLT,
  TCP_IDLE_TIMEO_DFLT,
  TCP_MAX_PROBE_DFLT,
  TCP_RAND_FUNC,
  TCP_MSL_DFLT
};

/*******************************************************************************
 * usrNetInit - Initialize network
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS usrNetInit(char *bootString)
{
  int i;
  CL_DESC  *pClDesc;
  struct protosw protoSwitch;

  /* Initialize network buffers library */
  netBufLibInit();

  /* Initialize number of entries */
  clDescTableNumEntries = NELEMENTS(clDescTable);

  /* Initialize memory */
  mClBlkConfig.memSize = (mClBlkConfig.mBlkNum * (MSIZE + sizeof(long)) +
			  mClBlkConfig.clBlkNum * CL_BLK_SZ);
  mClBlkConfig.memArea = malloc(mClBlkConfig.memSize);
  if (mClBlkConfig.memArea == NULL) {
    return ERROR;
  }

  /* For all elements in descriptor table */
  for (pClDesc = clDescTable, i = 0;
       i < clDescTableNumEntries;
       i++, pClDesc++) {

    pClDesc->memSize = pClDesc->clNum * (pClDesc->clSize * sizeof(long));
    pClDesc->memArea = malloc(pClDesc->memSize);
    if (pClDesc->memArea == NULL) {
      return ERROR;
    }

  } /* End for all elements in descriptor table */

  /* Initialize number of entries */
  sysClDescTableNumEntries = NELEMENTS(sysClDescTable);

  /* Initialize memory */
  sysMclBlkConfig.memSize = (sysMclBlkConfig.mBlkNum * (MSIZE + sizeof(long)) +
			     sysMclBlkConfig.clBlkNum * CL_BLK_SZ);
  sysMclBlkConfig.memArea = malloc(sysMclBlkConfig.memSize);
  if (sysMclBlkConfig.memArea == NULL) {
    return ERROR;
  }

  /* For all elements in descriptor table */
  for (pClDesc = sysClDescTable, i = 0;
       i < sysClDescTableNumEntries;
       i++, pClDesc++) {

    pClDesc->memSize = pClDesc->clNum * (pClDesc->clSize * sizeof(long));
    pClDesc->memArea = malloc(pClDesc->memSize);
    if (pClDesc->memArea == NULL) {
      return ERROR;
    }

  } /* End for all elements in descriptor table */

  /* Initialize set processor level */
  if ( unixLibInit() != OK) {
    return ERROR;
  }

  /* Initialize network buffers */
  if ( mbufLibInit() != OK ) {
    return ERROR;
  }

  /* Start net task */
  netLibInit();

  /* Initialize domains */
  unixDomainLibInit(256);
  inetLibInit();

  /* Initialize socket confuguration */
  soLibInit(SOMAXCONN_CFG);
  sbLibInit(SB_MAX);
  inLibInit();

  /* Initialize socket library */
  if ( sockLibInit(SOCKET_MAX_FD) != OK ) {
    return ERROR;
  }

  /* Add bsd socket library */
  if ( bsdSockLibAdd(AF_UNIX) != OK) {
    return ERROR;
  }

  /* Add bsd socket library */
  if ( bsdSockLibAdd(AF_INET) != OK) {
    return ERROR;
  }

  if ( ipLibInit(&ipCfgParams) != OK ) {
    return ERROR;
  }

  rawLibInit(8192, 8192);

  if ( rawIpLibInit(&rawipCfgParams) != OK ) {
    return ERROR;
  }

  if (icmpLibInit(&icmpCfgParams) != OK) {
    return ERROR;
  }

  if ( radixLibInit(sizeof(struct sockaddr_in)) != OK ) {
    return ERROR;
  }

  routeLibInit();

  if ( ifLibInit(IFQ_MAXLEN) != OK ) {
    return ERROR;
  }

  if ( loattach() != OK) {
    return ERROR;
  }

  if ( udpLibInit(&udpCfgParams) != OK) {
    return ERROR;
  }

  if ( tcpLibInit(&tcpCfgParams) != OK ) {
    return ERROR;
  }

  domaininit();

  return OK;
}

