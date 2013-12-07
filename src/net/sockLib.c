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

/* sockLib.c - Socket library */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <os/iosLib.h>
#include <os/ioLib.h>
#include <net/sockLib.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL BOOL sockLibInstalled = FALSE;
LOCAL SOCK_MAP *pSockMapHead = NULL;
LOCAL unsigned int sockFdMax = 0;

/* Globals */
SOCK_FUNC **pSockFdMap = NULL;

/* Functions */

/*******************************************************************************
 * sockLibInit - Initalize socket library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS sockLibInit(int fdMax)
{
  /* Check if already installed */
  if (sockLibInstalled)
    return OK;

  /* Allocate fd map array */
  pSockFdMap = (SOCK_FUNC **) malloc( fdMax * sizeof(SOCK_FUNC **) );
  if (pSockFdMap == NULL)
    return ERROR;

  /* Zero array */
  memset( pSockFdMap, 0, fdMax * sizeof(SOCK_FUNC **) );

  /* Set global max fd count */
  sockFdMax = fdMax;

  /* Mark as installed */
  sockLibInstalled = TRUE;

  return OK;
}

/*******************************************************************************
 * sockMapAdd - Add a socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS sockMapAdd(FUNCPTR sockMapInitFunc, int domainMap, int domainReal)
{
  SOCK_MAP *pSockMap;

  /* Check if library is installed */
  if (!sockLibInstalled)
    return ERROR;

  /* Allocate memory for data */
  pSockMap = (SOCK_MAP *) malloc( sizeof(SOCK_MAP) );
  if (pSockMap == NULL)
    return ERROR;

  /* Check if functions supplied */
  if (sockMapInitFunc == NULL) {

    free(pSockMap);
    return ERROR;

  }

  /* Get socket functions from init func */
  pSockMap->pSockFunc = (SOCK_FUNC *) (* sockMapInitFunc) ();
  if (pSockMap->pSockFunc == NULL) {

    free(pSockMap);
    return ERROR;

  }

  /* Setup rest of structure */
  pSockMap->domainMap = domainMap;
  pSockMap->domainReal = domainReal;
  pSockMap->pNext = pSockMapHead;

  /* Update list head */
  pSockMapHead = pSockMap;

  return OK;
}

/*******************************************************************************
 * sockFdtosockFunc - Get socket functions for file descriptor
 *
 * RETURNS: Pointer to socket functions
 ******************************************************************************/

SOCK_FUNC* sockFdtosockFunc(int sockfd)
{
  SOCK_FUNC *pSockFunc;

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return NULL;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  return pSockFunc;
}

/*******************************************************************************
 * socket - Open a socket
 *
 * RETURNS: Socket descriptor or ERROR
 ******************************************************************************/

int socket(int domain, int type, int protocol)
{
  SOCK_MAP *pSockMap;
  int fd;

  /* Check if library is installed */
  if (!sockLibInstalled)
    return ERROR;

  /* Find domain in socket list */
  for (pSockMap = pSockMapHead;
       (pSockMap != NULL) && (pSockMap->domainMap != domain);
       pSockMap = pSockMap->pNext);

  /* If socket function not found */
  if ( (pSockMap == NULL) || (pSockMap->pSockFunc->socketFunc == NULL) )
    return ERROR;

  /* Get file descriptor from socket function */
  fd = ( *pSockMap->pSockFunc->socketFunc) (pSockMap->domainReal,
					    type, protocol);
  if (fd == ERROR)
    return ERROR;

  /* Set socket functions in global map array */
  pSockFdMap[fd] = pSockMap->pSockFunc;

  return fd;
}

/*******************************************************************************
 * bind - Bind name to socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket */
  if (my_addr == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->bindFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->bindFunc) (sockfd, my_addr, addrlen);
}

/*******************************************************************************
 * listen - Listen for connections on a socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int listen(int sockfd, int backlog)
{
  SOCK_FUNC *pSockFunc;

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->listenFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->listenFunc) (sockfd, backlog);
}

/*******************************************************************************
 * accept - Accept a conncetion from socket
 *
 * RETURNS: Socket descriptor or ERROR
 ******************************************************************************/

int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
  SOCK_FUNC *pSockFunc;
  int fd;

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->acceptFunc == NULL) )
    return ERROR;

  /* Get fd from accept function */
  fd = ( *pSockFunc->acceptFunc) (sockfd, cliaddr, addrlen);
  if (fd == ERROR)
    return ERROR;

  /* Store fd in array */
  pSockFdMap[fd] = pSockFunc;

  return fd;
}

/*******************************************************************************
 * connect - Connect to a socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check socket addr */
  if (serv_addr == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->connectFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->connectFunc) (sockfd, serv_addr, addrlen);
}

/*******************************************************************************
 * connectWithTimeout - Connect to a socket with timeout
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int connectWithTimeout(int sockfd, struct sockaddr *serv_addr,
	               socklen_t addrlen, struct timeval *timeout)
{
  SOCK_FUNC *pSockFunc;

  /* Check socket addr */
  if (serv_addr == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->connectWithTimeoutFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->connectWithTimeoutFunc) (sockfd, serv_addr,
					        addrlen, timeout);
}

/*******************************************************************************
 * getpeername - Get name of connected peer
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check parameters */
  if (addr == NULL || addrlen == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->getpeernameFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->getpeernameFunc) (sockfd, addr, addrlen);
}

/*******************************************************************************
 * getsockname - Get socket name
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check parameters */
  if (addr == NULL || addrlen == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->getsocknameFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->getsocknameFunc) (sockfd, addr, addrlen);
}

/*******************************************************************************
 * sendto - Send message to socket
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

int sendto(int sockfd, void *buf, int buflen, int flags,
	   struct sockaddr *to_addr, int tolen)
{
  SOCK_FUNC *pSockFunc;

  /* Check socket addr */
  if (to_addr == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check buffer */
  if (buf == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->sendtoFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->sendtoFunc) (sockfd, buf, buflen,
				    flags, to_addr, tolen);
}

/*******************************************************************************
 * sendto - Send message on socket
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

int send(int sockfd, void *buf, int buflen, int flags)
{
  SOCK_FUNC *pSockFunc;

  /* Check buffer */
  if (buf == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->sendFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->sendFunc) (sockfd, buf, buflen, flags);
}

/*******************************************************************************
 * sendmsg - Send message
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

int sendmsg(int sockfd, struct msghdr *msg, int flags)
{
  SOCK_FUNC *pSockFunc;

  /* Check message */
  if (msg == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->sendmsgFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->sendmsgFunc) (sockfd, msg, flags);
}

/*******************************************************************************
 * recvfrom - Receive message from socket
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

int recvfrom(int sockfd, void *buf, int buflen, int flags,
	     struct sockaddr *from_addr, int *fromlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check from value */
  if ( (from_addr == NULL && fromlen != NULL) ||
       (from_addr != NULL && fromlen == NULL) ) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check buffer */
  if (buf == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->recvfromFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->recvfromFunc) (sockfd, buf, buflen, flags,
				      from_addr, fromlen);
}

/*******************************************************************************
 * recv - Receive message on socket
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

int recv(int sockfd, void *buf, int buflen, int flags)
{
  SOCK_FUNC *pSockFunc;

  /* Check buffer */
  if (buf == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->recvFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->recvFunc) (sockfd, buf, buflen, flags);
}

/*******************************************************************************
 * recvmsg - Receive message
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

int recvmsg(int sockfd, struct msghdr *msg, int flags)
{
  SOCK_FUNC *pSockFunc;

  /* Check message */
  if (msg == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->recvmsgFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->recvmsgFunc) (sockfd, msg, flags);
}

/*******************************************************************************
 * shutdown - Close a connection
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int shutdown(int sockfd, int how)
{
  SOCK_FUNC *pSockFunc;

  /* Check method */
  if (how > 2) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->shutdownFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->shutdownFunc) (sockfd, how);
}

/*******************************************************************************
 * setsockopt - Set socket options
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int setsockopt(int sockfd, int level, int optname,
	       void *optval, int optlen)
{
  SOCK_FUNC *pSockFunc;

  if (optval == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->setsockoptFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->setsockoptFunc) (sockfd, level, optname,
				        optval, optlen);
}

/*******************************************************************************
 * getsockopt - Get socket options
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int getsockopt(int sockfd, int level, int optname,
	       void *optval, int *optlen)
{
  SOCK_FUNC *pSockFunc;

  /* Check optvalue */
  if (optval == NULL || optlen == NULL) {

    errnoSet(EINVAL);
    return ERROR;

  }

  /* Check file descriptor value */
  if ( iosFdValue(sockfd) == NULL)
    return ERROR;

  /* Get socket functions */
  pSockFunc = pSockFdMap[sockfd];

  /* Check socket function */
  if ( (pSockFunc == NULL) || (pSockFunc->getsockoptFunc == NULL) )
    return ERROR;

  return ( *pSockFunc->getsockoptFunc) (sockfd, level, optname,
					optval, optlen);
}

