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

/* netDemo.c - Networking demo program for Real VMX */

#include "configAll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <a.out.h>
#include <elf.h>
#include <assert.h>

#include <vmx.h>
#include <os/symLib.h>

#include <net/netBufLib.h>
#include <net/mbuf.h>
#include <net/netShow.h>
#include <net/netLib.h>
#include <net/sockLib.h>

/* Imports */
IMPORT SYMTAB_ID sysSymTable;
IMPORT int sysClockRateGet(void);

/* Globals */
char bigString[] = "\n"
                   "******************************************************\n"
                   "Suddenly I awoke. I must have falled asleep under the \n"
                   "oak, where I used to sit and dream as a child.\n"
                   "I remebered my strange dream and knew the love and \n"
                   "strength in my heart to create something from my \n"
                   "life. The world was waiting, I was reborn...\n"
                   "******************************************************"
                   "\n";

int readSelect(int fd)
{
  char line[512];
  int result, len;
  fd_set readset;
  struct timeval tv;

  FD_ZERO(&readset);
  FD_SET(fd, &readset);

  tv.tv_sec = 10;
  tv.tv_usec = 0;

  printf("Read select on file %d...\n", fd);
  result = select(fd + 1, &readset, NULL, NULL, &tv);

  if (result > 0) {

    printf("Data is available now.\n");

    len = read(fd, line, 512);
    if (len)
      printf("Got %d bytes: %s\n", len, line);

  }
  else if (result == 0) {

    printf("No data within five seconds.\n");

  }

  else {

    printf("Error - Unable to select on file descriptor %d\n", fd);
    return -1;

  }

  return 0;
}

int netWrite(int fd, int pos)
{
  int nBytes;
  int len;
  int currPos;

  if (pos > 0)
    lseek(fd, pos, SEEK_SET);

  if (ioctl(fd, FIOWHERE, (int) &currPos) != OK)
    currPos = 0;

  len = strlen(bigString);

  printf("Writing to file %d at position %d...", fd, currPos);
  nBytes = write(fd, bigString, len + 1);
  if (nBytes != (len + 1)) {
    printf("Error - %d of %d bytes where written\n", nBytes, len + 1);
    return -1;
  }
  printf("Done.\n");

  return 0;
}

int netRead(int fd, int pos)
{
  char buf[512];
  int nBytes;
  int currPos;

  if (pos > 0)
    lseek(fd, pos, SEEK_SET);

  if (ioctl(fd, FIOWHERE, (int) &currPos) != OK)
    currPos = 0;

  printf("Reading from file %d at position %d...", fd, currPos);
  nBytes = read(fd, buf, 512);
  if (nBytes > 0)
    printf("Read data:\n%s\n", buf);

  return 0;
}

int netSocketNew(void)
{
  int fd;

  if ( (fd = socket(PF_UNIX, SOCK_STREAM, 0)) == ERROR ) {

    printf("Error - Unable to create socket.\n");
    return ERROR;

  }

  printf("Socket created with fd: %d\n", fd);

  return fd;
}

int netSocketDelete(int fd)
{
  if (shutdown(fd, 0x02) != OK) {

    printf("Error - Unable to shutdown socket.\n");
    return -1;

  }

  return close(fd);
}

int netSocketBind(int fd, char *name)
{
  struct sockaddr sa;

  strcpy(sa.sa_data, name);
  sa.sa_family = AF_UNIX;
  sa.sa_len = strlen(sa.sa_data);

  return bind(fd, &sa, sizeof(struct sockaddr));
}

int netSocketConnect(int fd, char *name)
{
  struct sockaddr sa;

  strcpy(sa.sa_data, name);
  sa.sa_family = AF_UNIX;
  sa.sa_len = strlen(sa.sa_data);

  return connect(fd, &sa, sizeof(struct sockaddr));
}

int netServer(char *name)
{
  struct sockaddr sa;
  int fd, con_fd, socklen;

  if ( (fd = socket(PF_UNIX, SOCK_STREAM, 0)) == ERROR ) {

    printf("netServer: Error - Unable to create socket.\n");
    return ERROR;

  }

  if ( netSocketBind(fd, name) == ERROR) {

    printf("netServer: Error - Unable to bind to address.\n");
    return ERROR;

  }

  socklen = sizeof(sa);
  listen(fd, 0);

  printf("netServer: Listening for connections.\n");
  while ( (con_fd = accept(fd, &sa, &socklen)) > 0 ) {

    printf("netServer: Got connection: with fd: %d (%s)\n",
	    con_fd, sa.sa_data);

  }
  printf("netServer: Stoped listening for connections.\n");

  return OK;
}

int netClient(char *name)
{
  int fd;

  if ( (fd = socket(PF_UNIX, SOCK_STREAM, 0)) == ERROR ) {

    printf("netClient: Error - Unable to create socket.\n");
    return ERROR;

  }

  if ( netSocketConnect(fd, name) != ERROR) {

    printf("netClient: Connected to server with fd: %d\n", fd);
    return fd;

  }

  return ERROR;
}

#include <net/un.h>

int unSock(char *path)
{
  struct sockaddr_un un;

  /* Clear struct */
  memset(&un, 0, sizeof(struct sockaddr_un));

  /* Fill in structure */
  un.sun_len = sizeof(struct sockaddr_un) + strlen(path);
  un.sun_family = AF_UNIX;
  strcpy(un.sun_path, path);

  printf("Created unix ipc domain with length: %d\n", SUN_LEN(&un));

  return OK;
}

#include <net/radix.h>
#include <net/route.h>
#include <net/if.h>
#include <net/raw_cb.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_var.h>
#include <netinet/ip_var.h>
#include <netinet/inetLib.h>
IMPORT struct inpcbinfo udbinfo;

int inAddRoute(char *adr, char *gate, int mask)
{
  int i, status;
  struct sockaddr_in addr;
  struct sockaddr_in gw;
  struct sockaddr_rt msk;

  addr.sin_len = sizeof(struct sockaddr_in);
  addr.sin_family = AF_INET;
  addr.sin_port = 80;
  addr.sin_addr.s_addr = inet_addr(adr);
  for (i = 0; i < 8; i++)
    addr.sin_zero[i] = 0;

  gw.sin_len = sizeof(struct sockaddr_in);
  gw.sin_family = AF_INET;
  gw.sin_port = 80;
  gw.sin_addr.s_addr = inet_addr(gate);
  for (i = 0; i < 8; i++)
    addr.sin_zero[i] = 0;

  if (mask != 0) {

    msk.srt_len = sizeof(struct sockaddr_rt);
    msk.srt_family = AF_INET;
    msk.srt_proto = SOCK_STREAM;
    msk.srt_tos = 0;
    msk.srt_addr.s_addr = mask;
    for (i = 0; i < 8; i++)
      msk.srt_zero[i] = 0;

    if ( (status = rtrequest(RTM_ADD,
			     (struct sockaddr *) &addr,
			     (struct sockaddr *) &gw,
			     (struct sockaddr *) &msk,
			     0,
			     NULL)) != 0) {

      printf("inAddRoute: Error - Unable to add route %s(%d).\n", adr, status);
      return status;

    }
    printf("inAddRoute: Added route: %s, gateway: %s, netmask: %d.\n",
	   adr, gate, mask);

  }

  else {

    if ( (status = rtrequest(RTM_ADD,
			     (struct sockaddr *) &addr,
			     (struct sockaddr *) &gw,
			     NULL,
			     0,
			     NULL)) != 0) {

      printf("inAddRoute: Error - Unable to add route %s(%d).\n", adr, status);
      return status;

    }

    printf("inAddRoute: Added route: %d, gateway: %s.\n", adr, gate);

  }

  return OK;
}

IMPORT unsigned short udp_pcbhashsize;
void inLogAddr(struct in_addr addr);
int udpCheck(void)
{
  int i;
  struct inpcb *inp;
  LIST_NODE *pNode;

  printf("UDP hashsize %d\n", udp_pcbhashsize);

  printf("UDP hash nodes:\n");
  for (inp = (struct inpcb *) LIST_HEAD(udbinfo.listhead);
       inp != NULL;
       inp = (struct inpcb *) LIST_NEXT(&inp->inp_list) ) {

    printf("Node: %#x\n", inp);
    inLogAddr(inp->inp_laddr);
    printf(":%d->", ntohs(inp->inp_lport));
    inLogAddr(inp->inp_faddr);
    printf(":%d ", ntohs(inp->inp_fport));
    printf("in bucket: %d\n", in_pcbhash(inp));

  }


  for (i = 0; i < udp_pcbhashsize; i++) {

    if (listCount(&udbinfo.hashbase[i])) {

      pNode = LIST_HEAD(&udbinfo.hashbase[i]);
      inp = (struct inpcb *) (((char *) pNode) -
				OFFSET(struct inpcb, inp_hash) );
      printf("%d [%d] @%#8x",
	     i,
	     listCount(&udbinfo.hashbase[i]),
	     inp);

      for (; pNode != NULL; pNode = LIST_NEXT(pNode))
        printf(".");

      printf("\n");

    }

  }

  return OK;
}

#define MAX_LINE 1000
int tcpServer(char *string, int port)
{
  int fd, len, con_fd, err, socklen;
  unsigned long addr;
  struct sockaddr_in sin, cliaddr;
  char buf[MAX_LINE + 1];

  addr = inet_addr(string);
  if (addr == ERROR) {

    printf("tcpServer: Error - Invalid internet address %s\n", string);
    return ERROR;

  }

  if ( (fd = socket(PF_INET, SOCK_STREAM, 0)) == ERROR ) {

    printf("tcpServer: Error - Unable to create socket.\n");
    return ERROR;

  }

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = addr;

  err = bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if (err) {

    printf("tcpServer: Error - Unable to bind to %s:%d\n", string, port);
    return err;

  }

  listen(fd, 5);
  socklen = sizeof(struct sockaddr_in);

  printf("tcpServer: Waiting for connections.\n");
  while ( (con_fd = accept(fd, (struct sockaddr *) &cliaddr, &socklen)) > 0 ) {

    printf("tcpServer: Got connection: with fd: %d\n", con_fd);

    printf("tcpServer: Waiting for data...");
    len = read(con_fd, buf, MAX_LINE);
    printf("Got data with length: %d.\n", len);

    printf("tcpServer: Echoing data...");
    len = write(con_fd, buf, len);
    printf("Done with length: %d.\n", len);

    printf("tcpServer: Closing connection...");
    close(con_fd);
    printf("Done.\n");

  }

  printf("tcpServer: Stoped listening for connections.\n");

  return OK;
}

int udpServer(char *string, int port)
{
  int fd, n, len, err;
  unsigned long addr;
  struct sockaddr_in sin, cliaddr;
  char buf[MAX_LINE + 1];

  addr = inet_addr(string);
  if (addr == ERROR) {

    printf("udpServer: Error - Invalid internet address %s\n", string);
    return ERROR;

  }

  if ( (fd = socket(PF_INET, SOCK_DGRAM, 0)) == ERROR ) {

    printf("udpServer: Error - Unable to create socket.\n");
    return ERROR;

  }

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = addr;

  err = bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if (err) {

    printf("udpServer: Error - Unable to bind to %s:%d\n", string, port);
    return err;

  }

  while (1) {

    printf("udpServer: Waiting for data...");
    len = sizeof(struct sockaddr_in);
    n = 0;
    n = recvfrom(fd, buf, MAX_LINE, 0, (struct sockaddr *) &cliaddr, &len);

    printf("Got data with length: %d.\n", n);

    printf("udpServer: Echoing data...");
    sendto(fd, buf, n, 0, (struct sockaddr *) &cliaddr, len);
    printf("Done with length: %d.\n", n);

  }

  return OK;
}

int tcpClient(char *string, int port)
{
  int fd, err;
  unsigned long addr;
  struct sockaddr_in sin;

  addr = inet_addr(string);
  if (addr == ERROR) {

    printf("tcpClient: Error - Invalid internet address %s\n", string);
    return ERROR;

  }

  if ( (fd = socket(PF_INET, SOCK_STREAM, 0)) == ERROR ) {

    printf("tcpClient: Error - Unable to create socket.\n");
    return ERROR;

  }

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = addr;

  printf("tcpClient: Connecting to: %s:%d...", string, port);

  err = connect(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if (err) {

    printf("tcpClient: Error - Unable to connect to %s:%d\n", string, port);
    return err;
  }

  printf("tcpClient: Connected to %s:%d with fd: %d\n", string, port, fd);

  return fd;
}

int udpClient(char *string, int port)
{
  int fd, n;
  unsigned long addr;
  struct sockaddr_in sin;
  char buf[MAX_LINE + 1];

  addr = inet_addr(string);
  if (addr == ERROR) {

    printf("udpClient: Error - Invalid internet address %s\n", string);
    return ERROR;

  }

  if ( (fd = socket(PF_INET, SOCK_DGRAM, 0)) == ERROR ) {

    printf("udpClient: Error - Unable to create socket.\n");
    return ERROR;

  }

  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = addr;

  printf("udpClient: Sending data...");
  sendto(fd, bigString, strlen(bigString), 0, (struct sockaddr *) &sin,
	 sizeof(struct sockaddr_in));
  printf("Done.\n");

  printf("udpClient: Receiving data...");
  n = recvfrom(fd, buf, MAX_LINE, 0, NULL, NULL);
  printf("Done.\n");

  buf[n] = '\0';

  printf("udpClient: Returned data: %s\n", buf);

  return OK;
}

int rawSocketNew(int on)
{
  int fd;

  if ( (fd = socket(PF_INET, SOCK_RAW, 0)) == ERROR ) {

    printf("Error - Unable to create socket.\n");
    return ERROR;

  }

  if ( setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0 ) {

    printf("Error - Unable to set socket option: IP_HDRINCL\n");
    return ERROR;

  }

  printf("Socket created with fd: %d\n", fd);

  return fd;
}

int rawSocketConnect(int fd, char *string)
{
  int i;
  unsigned long addr;
  struct sockaddr_in sin;

  printf("rawSocketConnect: Connect to internet address %s\n", string);

  addr = inet_addr(string);
  if (addr == ERROR) {

    printf("rawSocketConnect: Error - Invalid internet address %s\n", string);
    return ERROR;

  }

  printf("rawSocketConnect: To integer address: %d(%#x)\n", addr, addr);

  sin.sin_family = AF_INET;
  sin.sin_len = sizeof(struct sockaddr_in);
  sin.sin_port = 0;
  sin.sin_addr.s_addr = addr;
  for (i = 0; i < 8; i++)
    sin.sin_zero[i] = 0;

  return connect(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
}

int ipRead(int fd)
{
  char str[INET_ADDR_LEN];
  char buf[512];
  int nBytes;

  printf("Reading from file %d...", fd);
  nBytes = read(fd, buf, 512);
  if (nBytes > 0) {

    printf("%d bytes received\n", nBytes);

    struct ip *ip = (struct ip *) buf;
    printf("ip_len: %d\n", ip->ip_len);

    inet_ntoa_b(ip->ip_src.s_addr, str);
    printf("ip_src: %s\n", str);

    inet_ntoa_b(ip->ip_dst.s_addr, str);
    printf("ip_dst: %s\n", str);

    if (nBytes > sizeof(struct ip))
      printf("== MESSAGE ==\n%s\n", buf + sizeof(struct ip));

  }

  return 0;
}

void inLogAddr(struct in_addr addr)
{
  char str[16];

  inet_ntoa_b(addr.s_addr, str);
  printf("%s|%#x|", str, taskIdSelf());
}

void inLogSock(struct sockaddr *sa)
{
  struct sockaddr_in *sin;
  char str[16];

  sin = (struct sockaddr_in *) sa;

  inet_ntoa_b(sin->sin_addr.s_addr, str);
  printf("%s|%#x|", str, taskIdSelf());
}

int inSetAddr(char *string)
{
  int status;
  unsigned long addr;
  struct ifnet *ifnet;

  addr = inet_addr(string);
  if (addr == ERROR) {
    printf("inSetAddr: Error - Invalid internet address %s\n", string);
    return ERROR;
  }

  /* Bring up interface */
  ifnet = ifunit("lo0");
  if (ifnet == NULL) {
    printf("inSetAddr: Error - Unable to find interface 'lo0'\n");
    return ERROR;
  }

  if_up(ifnet);

  status = ifAddrAdd("lo0", string, NULL, 0);
  if (status != OK) {
    printf("inSetAddr: Error - Unable to set interface address %s\n", string);
    return ERROR;
  }

  return OK;
}

#define NUM_MBLK_LOCAL 256
#define NUM_CLBLK_LOCAL 256
#define NUM_MBLK_CL_LOCAL 256
#define NET_BUF_SIZE 256

M_BLK_ID mBlkId[NUM_MBLK_LOCAL + 1];
CL_BLK_ID clBlkId[NUM_CLBLK_LOCAL + 1];
M_BLK_ID mBlkClId[NUM_MBLK_CL_LOCAL + 1];
int mBlkCount = 0, clBlkCount = 0, clBufCount = 0, mBlkClCount = 0;
int canWait = M_WAIT;

int newMblk(void)
{

  if (mBlkCount >= NUM_MBLK_LOCAL) {
    printf("Warning - No more room for mBlk.\n");
    //return 0;
  }

  if ( (mBlkId[mBlkCount] = netMblkGet(netSysPoolId, canWait, MT_DATA)) == NULL) {
    printf("Error - Unable to get mBlk.\n");
    return 1;
  }

printf("\n\n--------------------------------------------------------------------------------\n");
  printf("GOT M_BLK: %x\n", (unsigned int) mBlkId[mBlkCount]);
printf("--------------------------------------------------------------------------------\n\n");

  mBlkCount++;

  return 0;
}

void deleteMblk(void)
{
  if (mBlkCount == 0) {
    printf("Warning - No mBlk allocated.\n");
    return;
  }

  netMblkFree(netSysPoolId, mBlkId[--mBlkCount]);
}

int newClBlk(void)
{
  if (clBlkCount >= NUM_CLBLK_LOCAL) {
    printf("Warning - No more room for ClBlk.\n");
    return 0;
  }

  if ( (clBlkId[clBlkCount] = netClBlkGet(netSysPoolId, canWait)) == NULL) {
    printf("Error - Unable to get clBlk.\n");
    return 1;
  }

printf("\n\n--------------------------------------------------------------------------------\n");
  printf("GOT CL_BLK: %x\n", (unsigned int) clBlkId[clBlkCount]);
printf("--------------------------------------------------------------------------------\n\n");

  clBlkCount++;

  return 0;
}

void deleteClBlk(void)
{
  if (clBlkCount == 0) {
    printf("Warning - No clBlk allocated.\n");
    return;
  }

  netClBlkFree(netSysPoolId, clBlkId[--clBlkCount]);
}

int newTuple(void)
{
  if (mBlkClCount >= NUM_MBLK_LOCAL) {
    printf("Warning - No more room for mBlk.\n");
    return 0;
  }

  if ( (mBlkClId[mBlkClCount] = netTupleGet(netSysPoolId, NET_BUF_SIZE, canWait, MT_HEADER, TRUE)) == NULL) {
    printf("Error - Unable to get net tuple.\n");
    return 1;
  }

printf("\n\n--------------------------------------------------------------------------------\n");
  printf("GOT NET_TUPLE: %x\n", (unsigned int) mBlkClId[mBlkClCount]);
printf("--------------------------------------------------------------------------------\n\n");

  mBlkClCount++;

  return 0;
}

void deleteTuple(void)
{
  if (mBlkClCount == 0) {
    printf("Warning - No tuples allocated.\n");
    return;
  }

  netMblkClFree(mBlkClId[--mBlkClCount]);
}

void printLocalStatus(void)
{
  printf("number of Mblks created: %d\n", mBlkCount);
  printf("number of ClBlks created: %d\n", clBlkCount);
  if (canWait == M_DONTWAIT)
    printf("Will not wait for block to become free.\n");
  else
    printf("Will wait for block to become free.\n");
}

void netTestShell(void)
{
  char str[256];
  char c;

  do {

    printf("\nSelect an option: \n");
    printf("1 - New mBlk\n");
    printf("2 - Delete mBlk\n");
    printf("3 - New clBlk\n");
    printf("4 - Delete clBlk\n");
    printf("5 - New netTuple\n");
    printf("6 - Delete netTuple\n");
    printf("x - Show info\n");
    printf("c - Toggle canWait flag\n");
    printf("\n->");

    scanf("%s", str);
    c = str[0];

    switch (c) {
      case '1' : newMblk();
	         break;

      case '2' : deleteMblk();
	         break;

      case '3' : newClBlk();
	         break;

      case '4' : deleteClBlk();
	         break;

      case '5' : newTuple();
	         break;

      case '6' : deleteTuple();
	         break;

      case 'c' : if (canWait == M_DONTWAIT)
		   canWait = M_WAIT;
		 else
		   canWait = M_DONTWAIT;
	         break;

      case 'x' : netPoolShow(netSysPoolId);
		 printLocalStatus();
	         break;
    }


  } while (c != 'q');

}

int netDemoInit(void)
{
static SYMBOL symTableDemo[] = {
  {NULL, "_readSelect", readSelect, 0, N_TEXT | N_EXT},
  {NULL, "_netWrite", netWrite, 0, N_TEXT | N_EXT},
  {NULL, "_netRead", netRead, 0, N_TEXT | N_EXT},
  {NULL, "_netSocketNew", netSocketNew, 0, N_TEXT | N_EXT},
  {NULL, "_netSocketDelete", netSocketDelete, 0, N_TEXT | N_EXT},
  {NULL, "_netSocketBind", netSocketBind, 0, N_TEXT | N_EXT},
  {NULL, "_netSocketConnect", netSocketConnect, 0, N_TEXT | N_EXT},
  {NULL, "_netServer", netServer, 0, N_TEXT | N_EXT},
  {NULL, "_netClient", netClient, 0, N_TEXT | N_EXT},
  {NULL, "_netTestShell", netTestShell, 0, N_TEXT | N_EXT},
  {NULL, "_unSock", unSock, 0, N_TEXT | N_EXT},
  {NULL, "_inAddRoute", inAddRoute, 0, N_TEXT | N_EXT},
  {NULL, "_udpCheck", udpCheck, 0, N_TEXT | N_EXT},
  {NULL, "_tcpServer", tcpServer, 0, N_TEXT | N_EXT},
  {NULL, "_udpServer", udpServer, 0, N_TEXT | N_EXT},
  {NULL, "_tcpClient", tcpClient, 0, N_TEXT | N_EXT},
  {NULL, "_udpClient", udpClient, 0, N_TEXT | N_EXT},
  {NULL, "_rawSocketNew", rawSocketNew, 0, N_TEXT | N_EXT},
  {NULL, "_rawSocketConnect", rawSocketConnect, 0, N_TEXT | N_EXT},
  {NULL, "_ipRead", ipRead, 0, N_TEXT | N_EXT},
  {NULL, "_inSetAddr", inSetAddr, 0, N_TEXT | N_EXT}
};

  struct tm startDate;
  struct timespec tp;
  int i;

  startDate.tm_sec = 0;
  startDate.tm_min = 56;
  startDate.tm_hour = 17;
  startDate.tm_mday = 8;
  startDate.tm_mon = 4;
  startDate.tm_year = 110;

  tp.tv_sec = mktime(&startDate);
  tp.tv_nsec = 0;
  clock_settime(CLOCK_REALTIME, &tp);

  /* Set ip address */
  inSetAddr("127.0.0.1");


  for (i = 0; i < NELEMENTS(symTableDemo); i++)
  {
    symTableAdd(sysSymTable, &symTableDemo[i]);
  }

  return 0;
}

