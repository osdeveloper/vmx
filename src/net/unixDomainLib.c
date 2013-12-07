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

/* unixDomainLib.c - Unix domain library */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <vmx.h>
#include <os/iosLib.h>
#include <os/pipeDrv.h>
#include <net/protosw.h>
#include <net/mbuf.h>
#include <net/socketvar.h>
#include <net/domain.h>
#include <netinet/in.h>
#include <net/unixDomainLib.h>

/* Defines */
#define MAX_SERVERS		10

/* Structs */
struct unixDomainNode {
  BOOL used;
  int sockFd;
  int pipeFd;
  int connFd;
  char name[255];
  struct socket *so;
};

/* Imports */

/* Locals */
LOCAL int unixDomainPacketSize = 256;
LOCAL char *unixPipePrefix = "/pipe/unixDomain/";
LOCAL struct unixDomainNode unixDomainNodes[MAX_SERVERS];
LOCAL struct socket *unixDomainSocket = NULL;

LOCAL void unixDomainProtoInit(struct protosw *pr);
LOCAL int unixDomainUsrreq(struct socket *so, int req,
			   struct mbuf *arg0,
			   struct mbuf *arg1,
			   struct mbuf *arg2);

/* Globals */
char unixDomainName[] = "unix";
struct protosw unixDomainProto[1];
struct domain unixDomain;

/* Functions */

/*******************************************************************************
 * unixDomainLibInit - Initialize unix domain library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS unixDomainLibInit(int pktSize)
{
  int i;

  /* Check packet size range */
  if (pktSize < 64 || pktSize > 512)
    pktSize = 256;

  /* Set packet size */
  unixDomainPacketSize = pktSize;

  /* Initialize array */
  for (i = 0; i < MAX_SERVERS; i++) {

    unixDomainNodes[i].used = FALSE;
    unixDomainNodes[i].connFd = -1;
    unixDomainNodes[i].so = NULL;

  }

  /* Initialize unix domain */
  memset(&unixDomain, 0, sizeof(struct domain) );

  unixDomain.dom_family = AF_UNIX;
  unixDomain.dom_name = unixDomainName;

  unixDomain.dom_init = (FUNCPTR) NULL;
  unixDomain.dom_externalize = (FUNCPTR) NULL;
  unixDomain.dom_dispose = (FUNCPTR) NULL;

  /* Initialize protocol */
  unixDomainProtoInit(&unixDomainProto[0]);
  unixDomain.dom_protosw = &unixDomainProto[0];
  unixDomain.dom_nprotosw = &unixDomainProto[1];

  unixDomain.dom_next = NULL;
  unixDomain.dom_rtattach = (FUNCPTR) NULL;
  unixDomain.dom_rtoffset = 0;
  unixDomain.dom_maxrtkey = 0;

  /* Add domain */
  addDomain(&unixDomain);
}

/*******************************************************************************
 * unixDomainProtoInit - Initialize unix domain sockets
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void unixDomainProtoInit(struct protosw *pr)
{
  /* Initialize unix domain sockets protocol switch */
  memset( pr, 0, sizeof(struct protosw) );

  pr->pr_type = SOCK_STREAM;
  pr->pr_domain = &unixDomain;
  pr->pr_protocol = IPPROTO_TCP;
  pr->pr_flags = PR_CONNREQUIRED | PR_WANTRCVD;

  pr->pr_input = (FUNCPTR) NULL;
  pr->pr_output = (FUNCPTR) NULL;
  pr->pr_ctlinput = (FUNCPTR) NULL;
  pr->pr_ctloutput = (FUNCPTR) NULL;
  pr->pr_usrreq = (FUNCPTR) unixDomainUsrreq;
  pr->pr_init = (FUNCPTR) NULL;
  pr->pr_fasttimo = (FUNCPTR) NULL;
  pr->pr_slowtimo = (FUNCPTR) NULL;
  pr->pr_drain = (FUNCPTR) NULL;
  pr->pr_sysctl = (FUNCPTR) NULL;
}

/*******************************************************************************
 * unixDomainUsrreq - Protocol request function
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL int unixDomainUsrreq(struct socket *so, int req,
			   struct mbuf *arg0,
			   struct mbuf *arg1,
			   struct mbuf *arg2)
{
  char str[64];
  int i, j, fd;
  struct socket *aso;
  struct sockaddr *sa;

  switch(req) {

    case PRU_BIND:
      sa = mtod(arg1, struct sockaddr *);

      printf("PRU_BIND: to %s...", sa->sa_data);

      so->so_snd.sb_hiwat = unixDomainPacketSize;
      so->so_snd.sb_cc = 0;
      so->so_snd.sb_mbmax = unixDomainPacketSize;
      so->so_snd.sb_mbcnt = 0;

      for (i = 0; i < MAX_SERVERS; i++)
        if (!unixDomainNodes[i].used)
          break;

      if ( i >= (MAX_SERVERS - 1) ) {

        printf("PRU_BIND: Error - No more servers available.\n");
        return ERROR;

      }

      strcpy(unixDomainNodes[i].name, unixPipePrefix);
      strcat(unixDomainNodes[i].name, sa->sa_data);

      pipeDevCreate(unixDomainNodes[i].name, 10, unixDomainPacketSize);

      unixDomainNodes[i].used = TRUE;
      unixDomainNodes[i].sockFd = so->so_fd;

      printf("PRU_BIND: Created server #%d for fd: %d\n",
	     i, unixDomainNodes[i].sockFd);

      printf("PRU_BIND COMPLETED SUCCESSFULLY!\n");
    break;

    case PRU_LISTEN:

      printf("PRU_LISTEN...");

      for (i = 0; i < MAX_SERVERS; i++)
        if ( (unixDomainNodes[i].used) &&
             (unixDomainNodes[i].sockFd == so->so_fd) )
          break;

      if ( i >= (MAX_SERVERS - 1) ) {

        printf("PRU_LISTEN: Error - Server hasn't bound yet.\n");
        return ERROR;

      }

      /* Set server socket */
      unixDomainNodes[i].so = so;

      printf("PRU_LISTEN: Listening for connections on server #%d fd: %d\n",
	     i, unixDomainNodes[i].sockFd);

      printf("PRU_LISTEN COMPLETED SUCCESSFULLY!\n");

    break;

    case PRU_ACCEPT:

      printf("PRU_ACCEPT for: %d...", so->so_fd);

      sa = mtod(arg1, struct sockaddr *);

      for (i = 0; i < MAX_SERVERS; i++)
        if ( (unixDomainNodes[i].used) &&
	     (unixDomainNodes[i].connFd == so->so_fd) )
          break;

      if ( i >= (MAX_SERVERS - 1) ) {

        printf("PRU_ACCEPT: Error - No connection to server from: %d\n",
	       so->so_fd);
        return ERROR;

      }

      printf("PRU_ACCEPT: Accepted connction to server #%d from fd: %d\n",
	     i, unixDomainNodes[i].connFd);

      sa->sa_family = AF_UNIX;
      strcpy(sa->sa_data, "unixDomain");
      sa->sa_len = strlen(sa->sa_data);

      printf("PRU_ACCEPT COMPLETED SUCCESSFULLY!\n");

    break;

    case PRU_CONNECT:
      sa = mtod(arg1, struct sockaddr *);

      printf("PRU_CONNECT: to %s...", sa->sa_data);

      so->so_snd.sb_hiwat = unixDomainPacketSize;
      so->so_snd.sb_cc = 0;
      so->so_snd.sb_mbmax = unixDomainPacketSize;
      so->so_snd.sb_mbcnt = 0;

      strcpy(str, unixPipePrefix);
      strcat(str, sa->sa_data);

      for (i = 0; i < MAX_SERVERS; i++)
        if ( (unixDomainNodes[i].used) &&
	     (strcmp(str, unixDomainNodes[i].name) == 0) )
          break;

      if ( i >= (MAX_SERVERS - 1) ) {

        printf("PRU_CONNECT: Error - Can't find server: %s\n", str);
        return ERROR;

      }

      if ( (fd = open(str, O_RDWR, 0)) == ERROR ) {

        printf("PRU_CONNECT: Error - Unable to open unix domain: %s\n", str);
        return ERROR;

      }

      if (unixDomainNodes[i].so != NULL) {

        if ( socreate(PF_UNIX, &aso, SOCK_STREAM, 0) != 0 ) {

          printf("PRU_CONNECT: Error - Unable to create new socket.\n");
          return ERROR;

        }

        soisconnecting(so);
        unixDomainNodes[i].pipeFd = fd;
        unixDomainNodes[i].connFd = so->so_fd;

        memcpy(aso, so, sizeof(struct socket));
        soqinsque(unixDomainNodes[i].so, aso, 1);
        aso->so_state |= SS_NOFDREF;

        soisconnected(so);
        soisconnected(aso);

        printf("PRU_CONNECT: Connected to server #%d fd: %d from fd: %d\n",
	       i, unixDomainNodes[i].sockFd, unixDomainNodes[i].connFd);

      }

      printf("PRU_CONNECT COMPLETED SUCCESSFULLY!\n");

    break;

    case PRU_SEND:
      printf("PRU_SEND: from fd: %d\n", so->so_fd);

      for (i = 0; i < MAX_SERVERS; i++)
        if ( (unixDomainNodes[i].used) &&
             (unixDomainNodes[i].connFd == so->so_fd) )
          break;

      if ( i >= (MAX_SERVERS - 1) ) {

        printf("PRU_SEND: Error - Server has not been connected to from %d\n",
	       so->so_fd);
        return ERROR;

      }

      printf("PRU_SEND: Got connection to server #%d from fd: %d\n",
	     i, unixDomainNodes[i].connFd);

      j = write(unixDomainNodes[i].pipeFd,
		mtod(arg0, char *), arg0->m_len);

      if (j != arg0->m_pkthdr.len) {

        printf("PRU_SEND: Error - %d or %d bytes sent.\n",
	       j, arg0->m_len);
        return ERROR; 

      }

      for (j = 0; j < arg0->m_len; j++)
        printf("%c", *(mtod(arg0, char *) + j));

      printf("%d byte(s) sent\n", arg0->m_len);

      so->so_rcv.sb_mb = m_copy(arg0, 0, arg0->m_len);
      so->so_rcv.sb_cc += j;

      printf("PRU_SEND COMPLETED SUCCESSFULLY!\n");
    break;

    case PRU_SHUTDOWN:
      printf("PRU_SHUTDOWN... ", sa->sa_data);
      printf("PRU_SHUTDOWN COMPLETED SUCCESSFULLY!\n");
    break;

    default:
      printf("unixDomain: Unknown request %d\n", req);
    break;

  }

  return 0;
}

