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

/* bsdSockLib.c - BSD compatible socket library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <net/socketvar.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <os/unixLib.h>
#include <os/iosLib.h>
#include <os/ioLib.h>
#include <net/mbuf.h>
#include <net/sockLib.h>

/* Defines */

/* Imports */
IMPORT SOCK_FUNC **pSockFdMap;

/* Locals */
LOCAL DEV_HEADER bsdSockDev;
LOCAL int bsdSockDrvNum = 0;
LOCAL char *bsdSockName = "(socket)";

LOCAL SOCK_FUNC* bsdSockLibInit(void);
LOCAL int bsdSocket(int domain, int type, int protocol);
LOCAL int bsdBind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
LOCAL int bsdListen(int sockfd, int backlog);
LOCAL int bsdAccept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
LOCAL int bsdConnect(int sockfd, struct sockaddr *serv_addr,
                     socklen_t addrlen);
LOCAL int bsdConnectWithTimeout(int sockfd, struct sockaddr *serv_addr,
                                socklen_t addrlen, struct timeval *timeout);
LOCAL int bsdSendto(int sockfd, void *buf, int buflen, int flags,
                    struct sockaddr *to_addr, int tolen);
LOCAL int bsdSend(int sockfd, void *buf, int buflen, int flags);
LOCAL int bsdSendmsg(int sockfd, struct msghdr *msg, int flags);
LOCAL int bsdRecvfrom(int sockfd, void *buf, int buflen, int flags,
                    struct sockaddr *from_addr, int *fromlen);
LOCAL int bsdRecv(int sockfd, void *buf, int buflen, int flags);
LOCAL int bsdRecvmsg(int sockfd, struct msghdr *msg, int flags);
LOCAL int bsdSetsockopt(int sockfd, int level, int optname,
                        void *optval, int optlen);
LOCAL int bsdGetsockopt(int sockfd, int level, int optname,
                        void *optval, int *optlen);
LOCAL int bsdGetsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
LOCAL int bsdGetpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
LOCAL int bsdShutdown(int sockfd, int how);

LOCAL int bsdSockCreate(struct socket *so, const char *name, mode_t mode,
                        const char *symlink);
LOCAL int bsdSockDelete(struct socket *so, const char *name, mode_t mode);
LOCAL int bsdSockOpen(struct socket *so, const char *name, int flags,
                      mode_t mode);
LOCAL STATUS bsdSockClose(struct socket *so);
LOCAL int bsdSockWrite(struct socket *so, void *buf, int buflen);
LOCAL int bsdSockRead(struct socket *so, void *buf, int buflen);
LOCAL int bsdSockArgs(struct mbuf **pName, void *name,
                      socklen_t addrlen, int type);

/* Globals */

SOCK_FUNC bsdSockFunc = {
  (FUNCPTR) bsdSockLibInit,
  (FUNCPTR) bsdAccept,
  (FUNCPTR) bsdBind,
  (FUNCPTR) bsdConnect,
  (FUNCPTR) bsdConnectWithTimeout,
  (FUNCPTR) bsdGetpeername,
  (FUNCPTR) bsdGetsockname,
  (FUNCPTR) bsdListen,
  (FUNCPTR) bsdRecv,
  (FUNCPTR) bsdRecvfrom,
  (FUNCPTR) bsdRecvmsg,
  (FUNCPTR) bsdSend,
  (FUNCPTR) bsdSendto,
  (FUNCPTR) bsdSendmsg,
  (FUNCPTR) bsdShutdown,
  (FUNCPTR) bsdSocket,
  (FUNCPTR) bsdGetsockopt,
  (FUNCPTR) bsdSetsockopt,
  (FUNCPTR) NULL
};

/* Functions */

/*******************************************************************************
 * bsdSockLibAdd - Add bsd sockets to socket library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS bsdSockLibAdd(int domain)
{
  return sockMapAdd((FUNCPTR) bsdSockLibInit, domain, domain);
}

/*******************************************************************************
 * bsdSockLibInit - Initialize bsd socket library
 *
 * RETURNS: Socket functions
 ******************************************************************************/

LOCAL SOCK_FUNC* bsdSockLibInit(void)
{
  /* Check if already installed */
  if (bsdSockDrvNum > 0)
    return (&bsdSockFunc);

  /* Install driver */
  bsdSockDrvNum = iosDrvInstall((FUNCPTR) bsdSockCreate,/* Create */
                                (FUNCPTR) bsdSockClose, /* Delete */
                                (FUNCPTR) bsdSockOpen,  /* Open */
                                (FUNCPTR) bsdSockClose, /* Close */
                                (FUNCPTR) bsdSockRead,  /* Read */
                                (FUNCPTR) bsdSockWrite, /* Write */
                                (FUNCPTR) soo_ioctl);   /* Ioctl */
  if (bsdSockDrvNum == ERROR)
    return NULL;

  /* Store values in driver */
  bsdSockDev.drvNumber = bsdSockDrvNum;
  bsdSockDev.name = bsdSockName;

  return (&bsdSockFunc);
}

/*******************************************************************************
 * bsdSocket - Open a socket
 *
 * RETURNS: File descritptor or ERROR
 ******************************************************************************/

LOCAL int bsdSocket(int domain, int type, int protocol)
{
  int s, fd, status;
  struct socket *so;

  /* Get processor level */
  s = splnet();

  /* Create socket */
  status = socreate(domain, &so, type, protocol);

  /* Restore processor level */
  splx(s);

  /* If status failed */
  if (status) {

    errnoSet(status);
    return ERROR;

  } /* End if status failed */

  /* Create file descriptor for socket */
  fd = iosFdNew((DEV_HEADER *) &bsdSockDev, bsdSockName, (ARG) so);
  if (fd == ERROR) {

    bsdSockClose(so);
    return ERROR;

  }

  /* Save file descriptor in socket struct */
  so->so_fd = fd;

  return fd;
}

/*******************************************************************************
 * bsdBind - Bind a name to socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdBind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
  struct mbuf *name;
  struct socket *so;
  int status;

  /* Get socket file value */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* Pack aruments */
  status = bsdSockArgs(&name, my_addr, addrlen, MT_SONAME);
  if (status) {

    errnoSet(status);
    return ERROR;

  }

  /* Bind socket to name */
  status = sobind(so, name);
  if (status) {

    errnoSet(status);
    m_freem(name);
    return ERROR;

  }

  /* Free name */
  m_freem(name);

  return OK;
}

/*******************************************************************************
 * bsdListen - Listen for connections to a socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdListen(int sockfd, int backlog)
{
  struct socket *so;
  int status;

  /* Get socket from fd */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* Listen on socket */
  status = solisten(so, backlog);
  if (status) {

    errnoSet(status);
    return ERROR;

  }

  return OK;
}

/*******************************************************************************
 * bsdAccept - Accept a connection to socket
 *
 * RETURNS: File descriptor or ERROR
 ******************************************************************************/

LOCAL int bsdAccept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
  struct mbuf *name;
  struct socket *so, *aso;
  int s, namelen, fd;

  /* Setup locals */
  namelen = *addrlen;

  /* Get socket fd value */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* Get processor level */
  s = splnet();

  /* If not accept connection */
  if ( (so->so_options & SO_ACCEPTCONN) == 0 ) {

    errnoSet(EINVAL);
    splx(s);
    return ERROR;

  } /* End if not accept connection */

  /* If no block and no queue */
  if ( (so->so_state & SS_NBIO) && (so->so_qlen == 0) ) {

    errnoSet(EWOULDBLOCK);
    splx(s);
    return ERROR;

  } /* End if no block and no queue */

  /* While queue length zero and no socket error */
  while ( (so->so_qlen == 0) && (so->so_error == 0) ) {

    /* If can't receive more */
    if (so->so_state & SS_CANTRCVMORE) {

      so->so_error = ECONNABORTED;
      break;

    } /* End if can't receive more */

    ksleep(so->so_timeoSem);

  } /* End while queue length zero and no socket error */

  /* If socket error */
  if (so->so_error) {

    errnoSet(so->so_error);
    so->so_error = 0;
    splx(s);
    return ERROR;

  } /* End if socket error */

  /* Allocate memory for name */
  name = m_get(M_WAIT, MT_SONAME, CL_SIZE_128, TRUE);
  if (name == NULL) {

    errnoSet(ENOBUFS);
    splx(s);
    return ERROR;

  }

  /* Get first socket on queue */
  aso = so->so_q;
  if ( soqremque(aso, 1) == 0 )
    panic("accept");

  /* Set socket */
  so = aso;

  /* Call accept */
  soaccept(so, name);

  /* If client address not null */
  if (cliaddr != NULL) {

    if (namelen > name->m_len)
      namelen = name->m_len;

      /* Copy name */
      memcpy(cliaddr, mtod(name, char *), namelen);
      memcpy(addrlen, &namelen, sizeof(socklen_t));

  } /* End if client address not null */

  /* Free buffer */
  m_freem(name);

  /* Restore processor level */
  splx(s);

  /* Create a new file descsiptor for connection */
  fd = iosFdNew((DEV_HEADER *) &bsdSockDev, bsdSockName, (ARG) so);
  if (fd == ERROR) {

    bsdSockClose(so);
    return ERROR;

  }

  /* Save file descriptor in socket */
  so->so_fd = fd;

  return fd;
}

/*******************************************************************************
 * bsdConnect - Connect to a socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdConnect(int sockfd, struct sockaddr *serv_addr,
                     socklen_t addrlen)
{
  struct mbuf *name;
  int s, status;
  struct socket *so;

  /* Get socket from file descriptor */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* Get processor level */
  s = splnet();

  /* If nbio or connecting */
  if ( (so->so_state & SS_NBIO) &&
       (so->so_state & SS_ISCONNECTING) ) {

    splx(s);
    errnoSet(EALREADY);
    return ERROR;

  } /* End if nbio of connecting */

  /* Restore processor level */
  splx(s);

  /* Setup name argument */
  status = bsdSockArgs(&name, serv_addr, addrlen, MT_SONAME);
  if (status) {

    errnoSet(status);
    return ERROR;

  }

  /* Connect */
  status = soconnect(so, name);
  if (status) {

    errnoSet(status);
    so->so_state &= ~SS_ISCONNECTING;
    m_freem(name);
    return ERROR;

  }

  /* Get processor level */
  s = splnet();

  /* If connection in progress */
  if ( (so->so_state & SS_NBIO) &&
       (so->so_state & SS_ISCONNECTING) ) {

    errnoSet(EINPROGRESS);
    splx(s);
    m_freem(name);
    return ERROR;

  } /* End if connection in progress */

  /* While is connecting */
  while ( (so->so_state & SS_ISCONNECTING) && (so->so_error == 0) )
    ksleep(so->so_timeoSem);

  /* If socket error */
  if (so->so_error) {

    status = ERROR;
    errnoSet(so->so_error);
    so->so_error = 0;

  } /* End if socket error */

  /* Else no socket error */
  else {

    status = OK;

  } /* End else no socket error */

  /* Restore processor level */
  splx(s);
  m_freem(name);

  return status;
}

/*******************************************************************************
 * bsdConnectWithTimeout - Connect to a socket with timeout
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdConnectWithTimeout(int sockfd, struct sockaddr *serv_addr,
                                socklen_t addrlen, struct timeval *timeout)
{
  return ERROR;
}

/*******************************************************************************
 * bsdSendto - Send message to socket
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

LOCAL int bsdSendto(int sockfd, void *buf, int buflen, int flags,
                    struct sockaddr *to_addr, int tolen)
{
  struct socket *so;
  struct mbuf *m, *msgBuf;
  struct uio uio, *pUio;
  struct iovec iov;
  int s, len, status;

  /* Initialize locals */
  pUio = NULL;
  msgBuf = NULL;

  /* Get socket from file descriptor */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == (struct socket *) ERROR) {

    if ( (flags & MSG_MBUF) &&
         (buf != NULL) )
        m_freem((struct mbuf *) buf);

      return ERROR;

  }

  /* Get processor level */
  s = splnet();

  /* Store socket arguments */
  status = bsdSockArgs(&m, to_addr, tolen, MT_SONAME);
  if (status) {

    errnoSet(status);

    if ( (flags & MSG_MBUF) &&
         (buf != NULL) )
        m_freem((struct mbuf *) buf);

      splx(s);
      return ERROR;

  }

  /* If message buffer */
  if (flags & MSG_MBUF) {

    msgBuf = (struct mbuf *) buf;
    len = buflen;

  }

  /* Else no message buffer */
  else {

    uio.uio_iov = &iov;
    uio.uio_iovcnt = 1;
    uio.uio_offset = 0;
    uio.uio_resid = buflen;
    uio.uio_rw = UIO_WRITE;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    pUio = &uio;
    len = uio.uio_resid;

  }

  /* Send to socket */
  status = sosend(so, m, pUio, msgBuf, NULL, flags);

  m_free(m);

  /* Restore processor level */
  splx(s);

  /* Check status */
  if (status) {

    if ( (pUio != NULL) &&
         (pUio->uio_resid != len) &&
         ((status == EINTR) || (status != EWOULDBLOCK)) )
      status = OK;

  }

  if (status == OK) {

    if (pUio == NULL)
      return len;

    return (len - pUio->uio_resid);

  }

  return ERROR;
}

/*******************************************************************************
 * bsdSend - Send message on socket
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

LOCAL int bsdSend(int sockfd, void *buf, int buflen, int flags)
{
  return ERROR;
}

/*******************************************************************************
 * bsdSendmsg - Send message
 *
 * RETURNS: Bytes sent or ERROR
 ******************************************************************************/

LOCAL int bsdSendmsg(int sockfd, struct msghdr *msg, int flags)
{
  return ERROR;
}

/*******************************************************************************
 * bsdRecvfrom - Receive message from socket
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

LOCAL int bsdRecvfrom(int sockfd, void *buf, int buflen, int flags,
                    struct sockaddr *from_addr, int *fromlen)
{
  struct socket *so;
  struct mbuf *m;
  struct uio uio;
  struct iovec iov;
  struct mbuf **ppMb;
  int s, len, status, fromLen, soFlags;

  /* Inititalize locals */
  ppMb = NULL;

  /* Get socket from file descriptor */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == (struct socket *) ERROR)
    return ERROR;

  /* Get processor level */
  s = splnet();

  /* Setup uio struct */
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;
  uio.uio_offset = 0;
  uio.uio_resid = buflen;
  uio.uio_rw = UIO_READ;

  iov.iov_base = buf;
  iov.iov_len = buflen;

  /* Store locals */
  soFlags = flags;
  len = uio.uio_resid;

  /* If message buffer */
  if (flags & MSG_MBUF)
    ppMb = (struct mbuf **) buf;

  /* Receive data from socket */
  status = soreceive(so, &m, &uio, ppMb, NULL, &soFlags);

  /* If copy data */
  if ( (from_addr != NULL) &&
       (fromlen != NULL) ) {

    fromLen = *fromlen;
    if ( (fromLen <= 0) ||
         (m == NULL) ) {

      fromLen = 0;

     }

     else {

       if (fromLen > m->m_len)
         fromLen = m->m_len;

       memcpy(from_addr, mtod(m, char *), fromLen);

     }

     *fromlen = fromLen;

  }

  if (m != NULL)
    m_freem(m);

  /* Restore processor level */
  splx(s);

  /* Check status */
  if (status) {

    if ( (uio.uio_resid != len) &&
         ((status == EINTR) || (status == EWOULDBLOCK)) )
      status = OK;

  }

  if (status == OK)
    return (len - uio.uio_resid);

  return ERROR;
}

/*******************************************************************************
 * bsdRecv - Receive message on socket
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

LOCAL int bsdRecv(int sockfd, void *buf, int buflen, int flags)
{
  return ERROR;
}

/*******************************************************************************
 * bsdRecvmsg - Receive message
 *
 * RETURNS: Bytes received or ERROR
 ******************************************************************************/

LOCAL int bsdRecvmsg(int sockfd, struct msghdr *msg, int flags)
{
  return ERROR;
}

/*******************************************************************************
 * bsdSetsockopt - Set socket options
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdSetsockopt(int sockfd, int level, int optname,
                        void *optval, int optlen)
{
  struct mbuf *m;
  struct socket *so;
  int status;
  char *value;

  /* Initialize locals */
  m = NULL;

  /* Get socket */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* If options size to large */
  if (optlen > CL_SIZE_128) {

    errnoSet(EINVAL);
    return ERROR;

  } /* End if options size to large */

  /* Get Value */
  value = (char *) optval;

  /* If value sent */
  if (value != NULL) {

    /* Allocate buffer */
    m = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
    if (m == NULL) {

      errnoSet(ENOBUFS);
      return ERROR;

    }

    /* Copy data */
    memcpy( mtod(m, char *), optval, optlen);

    /* Set length */
    m->m_len = optlen;

  } /* End if value sent */

  /* Call socket set options */
  status = sosetopt(so, level, optname, m);
  if (status) {

    errnoSet(status);
    return ERROR;

  }

  return OK;
}

/*******************************************************************************
 * bsdGetsockopt - Get socket options
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdGetsockopt(int sockfd, int level, int optname,
                        void *optval, int *optlen)
{
  return ERROR;
}

/*******************************************************************************
 * bsdGetsockname - Get socket name
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdGetsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  return ERROR;
}

/*******************************************************************************
 * bsdGetpeername - Get peer name
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdGetpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  return ERROR;
}

/*******************************************************************************
 * bsdShutdown - Shutdown socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL int bsdShutdown(int sockfd, int how)
{
  struct socket *so;
  int status;

  /* Get socket file value */
  so = (struct socket *) iosFdValue(sockfd);
  if (so == NULL)
    return ERROR;

  /* Shutdown */
  status = soshutdown(so, how);
  if (status) {

    errnoSet(status);
    return ERROR;

  }

  return OK;
}

/*******************************************************************************
 * bsdSockCreate - Create socket
 *
 * RETURNS: ERROR
 ******************************************************************************/

LOCAL int bsdSockCreate(struct socket *so, const char *name, mode_t mode,
                        const char *symlink)
{
    return ERROR;
}

/*******************************************************************************
 * bsdSockDelete - Delete socket
 *
 * RETURNS: ERROR
 ******************************************************************************/

LOCAL int bsdSockDelete(struct socket *so, const char *name, mode_t mode)
{
    return ERROR;
}

/*******************************************************************************
 * bsdSockOpen - Open socket
 *
 * RETURNS: OK
 ******************************************************************************/

LOCAL int bsdSockOpen(struct socket *so, const char *name, int flags,
                      mode_t mode)
{
    return OK;
}

/*******************************************************************************
 * bsdSockClose - Close socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL STATUS bsdSockClose(struct socket *so)
{
  int s, fd, ret;

  /* Get processor level */
  s = splnet();

  /* Get file descriptor */
  fd = so->so_fd;

  /* Check fd reference */
  if (so->so_state & SS_NOFDREF)
    ret = 0;
  else
    ret = soclose(so);

  /* Restore processor level */
  splx(s);

  /* Remove fd reference */
  pSockFdMap[fd] = NULL;

  return ret;
}

/*******************************************************************************
 * bsdSockWrite - Write to socket
 *
 * RETURNS: Bytes written or ERROR
 ******************************************************************************/

LOCAL int bsdSockWrite(struct socket *so, void *buf, int buflen)
{
  int status, s, len;
  struct uio uio;
  struct iovec iovec;

  /* Get processor level */
  s = splnet();

  /* Setup uio struct */
  uio.uio_iov = &iovec;
  uio.uio_iovcnt = 1;
  uio.uio_offset = 0;
  uio.uio_resid = buflen;
  uio.uio_rw = UIO_WRITE;

  /* Setup iovec */
  iovec.iov_base = buf;
  iovec.iov_len = buflen;

  /* Get length */
  len = uio.uio_resid;

  /* Call socket send function */
  status = sosend(so, (struct mbuf *) NULL,
                      &uio,
                      (struct mbuf *) NULL,
                      (struct mbuf *) NULL,
                      0);
  if (status) {

    if ( (uio.uio_resid != len) &&
         ( (status == EINTR) || (status == EWOULDBLOCK) ) )
      status = OK;

  }

  /* Return bytes written or ERROR */
  if (status == OK)
    return (len - uio.uio_resid);

  return ERROR;
}

/*******************************************************************************
 * bsdSockRead - Read from socket
 *
 * RETURNS: Bytes read or ERROR
 ******************************************************************************/

LOCAL int bsdSockRead(struct socket *so, void *buf, int buflen)
{
  int s, status, len, flags;
  struct uio uio;
  struct iovec iovec;

  /* Reset flags */
  flags = 0;

  /* Get processor level */
  s = splnet();

  /* Setup uio struct */
  uio.uio_iov = &iovec;
  uio.uio_iovcnt = 1;
  uio.uio_offset = 0;
  uio.uio_resid = buflen;
  uio.uio_rw = UIO_READ;

  /* Setup uio vector */
  iovec.iov_base = buf;
  iovec.iov_len = buflen;

  /* Get length */
  len = uio.uio_resid;

  /* Call so receive */
  status = soreceive(so, (struct mbuf **) NULL, &uio, (struct mbuf **) NULL,
                     (struct mbuf **) NULL, &flags);

  /* Restore processor level */
  splx(s);

  /* If error */
  if (status) {

    if ( (uio.uio_resid != len) &&
         ( (status == EINTR) || (status == EWOULDBLOCK) ) )
      status = OK;

  } /* End if error */

  if (status == OK)
    return (len - uio.uio_resid);

  return ERROR;
}

/*******************************************************************************
 * bsdSockArgs - Store socket arguments into mbuf
 *
 * RETURNS: Bytes read or ERROR
 ******************************************************************************/

LOCAL int bsdSockArgs(struct mbuf **pName, void *name,
                      socklen_t addrlen, int type)
{
  struct mbuf *m;
  struct sockaddr *sa;

  if (addrlen > CL_SIZE_128)
    return EINVAL;

  /* Allocate memory block and cluster */
  m = m_get(M_WAIT, type, CL_SIZE_128, TRUE);
  if (m == NULL)
    return ENOBUFS;

  /* Copy name */
  m->m_len = addrlen;
  memcpy( mtod(m, char *), name, addrlen);
  *pName = m;

  /* If socket name */
  if (type == MT_SONAME) {

    sa = mtod(m, struct sockaddr *);
    sa->sa_len = addrlen;

  } /* End if socket name */

  return OK;
}

