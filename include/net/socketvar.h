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

/* socketvar.h - Socket struct header */

#ifndef _socketvar_h
#define _socketvar_h

#include <vmx.h>
#include <os/selectLib.h>
#include <net/mbuf.h>

/* Defines */
#define SB_MAX		(256 * 1024)		/* Default max chars in buf */

#define SB_LOCK		0x01			/* Lock on data queue */
#define SB_WANT		0x02			/* Waiting to look */
#define SB_WAIT		0x04			/* Waiting for data */
#define SB_SEL		0x08			/* Selecting */
#define SB_ASYNC	0x10			/* Async i/o */
#define SB_NOINTR	0x40			/* Not interruptable */
#define SB_NOTIFY	(SB_WAIT | SB_SEL | SB_ASYNC)

#define SS_NOFDREF	0x001			/* No file table ref */
#define SS_ISCONNECTED	0x002			/* Socket connected to peer */
#define SS_ISCONNECTING	0x004			/* In process of connect */
#define SS_ISDISCONNECTING 0x008		/* In process of disconnect */
#define SS_CANTSENDMORE	0x010			/* Can't send more */
#define SS_CANTRCVMORE	0x020			/* Can't receive more */
#define SS_RCVATMARK	0x040			/* Mark on input */
#define SS_PRIV		0x080			/* Priviledged broadcast */
#define SS_NBIO		0x100			/* Non-blocking operations */
#define SS_ASYNC	0x200			/* Async i/o notify */
#define SS_ISCONFIRMING 0x400			/* Deciding to accept */

#ifndef _ASMLANGUAGE

#include <vmx/semLib.h>
#include <net/protosw.h>
#include <net/mbuf.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <os/selectLib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct socket {
  short			so_type;		/* Socket type */
  short			so_options;		/* Socket options */
  short			so_linger;		/* Time to linger on closing */
  short			so_state;		/* Socket state */
  void			*so_pcb;		/* Protocol control block */
  struct protosw	*so_proto;		/* Protocol handle */

  ARG			so_userArg;		/* Argument */
  SEM_ID		so_timeoSem;		/* Timeout semaphore */
  int			so_fd;			/* Socket file descriptor */

  struct socket		*so_head;		/* Back pointer to socket */
  struct socket		*so_q0;			/* Queue for partial con. */
  struct socket		*so_q;			/* Queue for incomplete con. */
  short			so_q0len;		/* Length of partials */
  short			so_qlen;		/* Length of incompletes */
  short			so_qlimit;		/* Max number of con. */
  short			so_timeo;		/* Connection timeout */
  unsigned short	so_error;		/* Error for connection */
  short			so_pgrp;		/* Pgrp for signals */
  unsigned long		so_oobmark;		/* Chars to oob mark */

  struct sockbuf {
    unsigned long	  sb_cc;		  /* Chars in buffer */
    unsigned long	  sb_hiwat;		  /* Max chars in buffer */
    unsigned long	  sb_mbcnt;		  /* Chars of mbufs used */
    unsigned long	  sb_mbmax;		  /* Max chars of mbufs used */
    unsigned long	  sb_lowat;		  /* Low watermark */
    struct mbuf		  *sb_mb;		  /* mbuf chain */
    int			  sb_sel;		  /* Task selecting r/w */
    short		  sb_timeo;		  /* Timeout */
    short		  sb_flags;		  /* Flags */

    SEM_ID		  sb_sem;		  /* Semaphore */
    int			  sb_want;		  /* Want semaphore */
  } so_rcv, so_snd;

  FUNCPTR		so_upcall;		/* Upcall function */
  void			*so_upcallarg;		/* Argument for upcall */

  BOOL			selectFlag;		/* Select available */
  SEL_WAKEUP_LIST	so_selWakeupList;	/* Select wakeup list */
  int			vsid;			/* Virtual stack id */
};

/* Macros */

/*******************************************************************************
 * sosendallatonce - Check send all at once for socket
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define sosendallatonce(so)						       \
  ( (so)->so_proto->pr_flags & PR_ATOMIC )

/*******************************************************************************
 * sowwakeup - Wakeup socket
 *
 * RETURNS: N/A
 ******************************************************************************/

#define sowwakeup(so)							       \
  sowakeup( (so), &(so)->so_snd, SELWRITE )

/*******************************************************************************
 * sorwakeup - Wakeup socket
 *
 * RETURNS: N/A
 ******************************************************************************/

#define sorwakeup(so)							       \
{									       \
  sowakeup( (so), &(so)->so_rcv, SELREAD );				       \
  if ( (so)->so_upcall != NULL )					       \
    ( *((so)->so_upcall) ) ( (so)->so_upcallarg, M_DONTWAIT );		       \
}

/*******************************************************************************
 * sbspace - Get send receive buffer space
 *
 * RETURNS: Minimum space
 ******************************************************************************/

#define sbspace(sb)							       \
  ( (long) min( (int)((sb)->sb_hiwat - (sb)->sb_cc),			       \
	        (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)) )

/*******************************************************************************
 * sbfree - Free send receive buffer
 *
 * RETURNS: Minimum space
 ******************************************************************************/

#define sbfree(sb, m)							       \
{									       \
  (sb)->sb_cc -= (m)->m_len;						       \
  (sb)->sb_mbcnt -= MSIZE;						       \
  if ( (m)->m_flags & M_EXT )						       \
    (sb)->sb_mbcnt -= (m)->m_extSize;					       \
}

/*******************************************************************************
 * sballoc - Allocate socket buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

#define sballoc(sb, m)							       \
{									       \
  (sb)->sb_cc += (m)->m_len;						       \
  (sb)->sb_mbcnt += MSIZE;						       \
  if ( (m)->m_flags & M_EXT )						       \
    (sb)->sb_mbcnt += (m)->m_extSize;					       \
}

/* Functions */

IMPORT void soLibInit(int maxConn);

IMPORT int socreate(int domain, struct socket **aso, int type, int proto);
IMPORT int soaccept(struct socket *so, struct mbuf *cliaddr);
IMPORT int sobind(struct socket *so, struct mbuf *my_addr);
IMPORT int soconnect(struct socket *so, struct mbuf *serv_addr);
IMPORT int soconnect2(struct socket *so1, struct socket *so2);
IMPORT int solisten(struct socket *so, int backlog);
IMPORT void sorflush(struct socket *so);
IMPORT int sosend(struct socket *so, struct mbuf *addr,
		  struct uio *uio, struct mbuf *top, struct mbuf *ctrl,
		  int flags);
IMPORT int soreceive(struct socket *so, struct mbuf **paddr, struct uio *uio,
		     struct mbuf **mp0, struct mbuf **ctrlp, int *flagsp);
IMPORT int sosetopt(struct socket *so, int level, int optname, struct mbuf *m);
IMPORT int sogetopt(struct socket *so, int level, int optname, struct mbuf **m);
IMPORT int sodisconnect(struct socket *so);
IMPORT int soclose(struct socket *so);
IMPORT int soshutdown(struct socket *so, int how);
IMPORT int soabort(struct socket *so);
IMPORT void sofree(struct socket *so);
IMPORT void sowakeup(struct socket *so, struct sockbuf *sb,
		     SELECT_TYPE selType);
IMPORT void soisconnecting(struct socket *so);
IMPORT void soisconnected(struct socket *so);
IMPORT void socantsendmore(struct socket *so);
IMPORT void socantrcvmore(struct socket *so);
IMPORT void soisdisconnecting(struct socket *so);
IMPORT void soisdisconnected(struct socket *so);
IMPORT struct socket* sonewconn(struct socket *head, int connstatus);
IMPORT void soqinsque(struct socket *head, struct socket *so, int q);
IMPORT int soqremque(struct socket *so, int q);
IMPORT int soreserve(struct socket *so,
		     unsigned long sndcc,
		     unsigned long rcvcc);
IMPORT void sohasoutofband(struct socket *so);
IMPORT int soo_unselect(struct socket *so, SEL_WAKEUP_NODE *wakeupNode);
IMPORT int soo_ioctl(struct socket *so, int cmd, void *data);

IMPORT void sbcompress(struct sockbuf *sb, struct mbuf *m, struct mbuf *n);
IMPORT void sbflush(struct sockbuf *sb);
IMPORT void sbdrop(struct sockbuf *sb, int len);
IMPORT void sbdroprecord(struct sockbuf *sb);
IMPORT void sbrelease(struct sockbuf *sb);
IMPORT void sbseldequeue(struct socket *so,
			 struct sockbuf *sb,
			 SEL_WAKEUP_NODE *wakeupNode);
IMPORT void sbwakeup(struct socket *so, struct sockbuf *sb,
		     SELECT_TYPE selType);
IMPORT void sbwait(struct sockbuf *sb);
IMPORT int sbreserve(struct sockbuf *sb, unsigned long cc);
IMPORT void sbappend(struct sockbuf *sb, struct mbuf *m);
IMPORT void sbappendrecord(struct sockbuf *sb, struct mbuf *m0);
IMPORT int sbappendaddr(struct sockbuf *sb,
			struct sockaddr *asa,
			struct mbuf *m0,
			struct mbuf *ctrl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _socketvar_h */

