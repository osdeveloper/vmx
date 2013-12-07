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

/* soLib.c - Socket primitives library */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <vmx.h>
#include <util/listLib.h>
#include <vmx/semLib.h>
#include <os/unixLib.h>
#include <os/selectLib.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/netLib.h>
#include <net/mbuf.h>
#include <net/domain.h>
#include <net/route.h>
#include <net/socketvar.h>

/* Defines */

/* Imports */
IMPORT int max_hdr;

/* Locals */

/* Globals */
int somaxconn = 5;

/* Functions */

/*******************************************************************************
 * soLibInit - Inititalize socket library
 *
 * RETURNS: N/A
 ******************************************************************************/

void soLibInit(int maxConn)
{
  somaxconn = maxConn;
}

/*******************************************************************************
 * socreate - Create socket
 *
 * RETURNS: Error number or zero
 ******************************************************************************/

int socreate(int domain, struct socket **aso, int type, int proto)
{
  struct protosw *pr;
  struct socket *so;
  int err;

  /* Get protocol struct */
  if (proto)
    pr = pffindproto(domain, proto, type);
  else
    pr = pffindtype(domain, type);

  if ( (pr == NULL) || (pr->pr_usrreq == NULL) )
    return EPROTONOSUPPORT;

  if (pr->pr_type != type)
    return EPROTOTYPE;

  /* Allocate memory for socket */
  so = (struct socket *) mb_alloc( sizeof(struct socket), MT_SOCKET, M_WAIT );
  if (so == NULL)
    return ENOBUFS;

  /* Zero structure */
  memset( so, 0, sizeof(struct socket) );

  /* Initialize some fields */
  so->so_options = 0;
  so->so_type = type;
  so->so_state = SS_PRIV;
  so->so_proto = pr;

  /* Initialize semaphore */
  so->so_timeoSem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_timeoSem == NULL) {

    so->so_state |= SS_NOFDREF;
    sofree(so);
    return ENOMEM;

  }

  /* Initialize receive semaphore */
  so->so_rcv.sb_sem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_rcv.sb_sem == NULL) {

    so->so_state |= SS_NOFDREF;
    sofree(so);
    return ENOMEM;

  }

  /* Initialize send semaphore */
  so->so_snd.sb_sem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_snd.sb_sem == NULL) {

    so->so_state |= SS_NOFDREF;
    sofree(so);
    return ENOMEM;

  }

  so->so_rcv.sb_want = 0;
  so->so_snd.sb_want = 0;

  selWakeupListInit(&so->so_selWakeupList);
  so->selectFlag = TRUE;

  err = ( *pr->pr_usrreq) (so, PRU_ATTACH, (struct mbuf *) NULL,
		           (struct mbuf *) (long) proto, (struct mbuf *) NULL);
  if (err) {

    so->so_state |= SS_NOFDREF;
    sofree(so);
    return err;

  }

  /* Store socket in argument */
  *aso = so;

  return 0;
}

/*******************************************************************************
 * soaccept - Accept connection to socket
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int soaccept(struct socket *so, struct mbuf *cliaddr)
{
  int s, err;

  /* Get processor level */
  s = splnet();

  /* If no file desc */
  if ( (so->so_state & SS_NOFDREF) == 0)
    panic("soaccept: !NOFDREF");

  /* Reset state */
  so->so_state &= ~SS_NOFDREF;

  /* Call function */
  err = ( *so->so_proto->pr_usrreq) (so, PRU_ACCEPT, (struct mbuf *) NULL,
				     cliaddr, (struct mbuf *) NULL);

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * sobind - Bind a socket to an address
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int sobind(struct socket *so, struct mbuf *my_addr)
{
  int s, err;

  /* Get processor level */
  s = splnet();

  /* Call function */
  err = ( *so->so_proto->pr_usrreq) (so, PRU_BIND, (struct mbuf *) NULL,
				     my_addr, (struct mbuf *) NULL);

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * soconnect - Connet from socket
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int soconnect(struct socket *so, struct mbuf *serv_addr)
{
  int s, err;

  if (so->so_options & SO_ACCEPTCONN)
    return EOPNOTSUPP;

  /* Get processor level */
  s = splnet();

  /* If connected or connecting */
  if ( (so->so_state) & (SS_ISCONNECTED | SS_ISCONNECTING) &&
       ( (so->so_proto->pr_flags & PR_CONNREQUIRED) ||
	 (err = sodisconnect(so)) ) ) {

    err = EISCONN;

  } /* End if connected or connecting */

  /* Else not connected */
  else {

    /* Call function */
    err = ( *so->so_proto->pr_usrreq) (so, PRU_CONNECT, (struct mbuf *) NULL,
				       serv_addr, (struct mbuf *) NULL);

  } /* End else not connected */
 
  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * soconnect2 - Connet two sockets
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int soconnect2(struct socket *so1, struct socket *so2)
{
  int s, err;

  /* Get processor level */
  s = splnet();

  /* Call function */
  err = ( *so1->so_proto->pr_usrreq) (so1, PRU_CONNECT2, (struct mbuf *) NULL,
				     (struct mbuf *) so2, (struct mbuf *) NULL);

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * solisten - Listen for connections on a socket
 *
 * RETURNS: Protocol specific zero
 ******************************************************************************/

int solisten(struct socket *so, int backlog)
{
  int s, err;

  /* Get processor level */
  s = splnet();

  /* Call usrreq function */
  err = ( *so->so_proto->pr_usrreq) (so, PRU_LISTEN,
				     (struct mbuf *) NULL,
			             (struct mbuf *) NULL,
			             (struct mbuf *) NULL);
  if (err) {

    splx(s);
    return err;

  }

  /* Accept conncetion */
  if (so->so_q == NULL)
    so->so_options |= SO_ACCEPTCONN;

  if (backlog < 0)
    backlog = 0;

  so->so_qlimit = min(backlog, somaxconn);

  /* Restore processor level */
  splx(s);

  return 0;
}

/*******************************************************************************
 * sorflush - Flush socket
 *
 * RETURNS: N/A
 ******************************************************************************/

void sorflush(struct socket *so)
{
  struct protosw *pr;
  struct sockbuf *sb;
  int s;
  struct sockbuf asb;

  /* Get protocol switch */
  pr = so->so_proto;

  /* Get socket buffer */
  sb = &so->so_rcv;

  /* Set not interruptable flag */
  sb->sb_flags |= SB_NOINTR;

  /* Get processor level */
  s = splimp();

  /* Can't receive more */
  socantrcvmore(so);

  /* Get local copy */
  asb = *sb;

  /* If semaphore */
  if (sb->sb_sem != NULL) {

    semDelete(sb->sb_sem);
    sb->sb_sem = NULL;

  } /* End if semaphore */

  /* Zero buffer */
  memset( sb, 0, sizeof(struct sockbuf) );

  /* Restore processor level */
  splx(s);

  /* Call dispose if existing */
  if ( (pr->pr_flags & PR_RIGHTS) && (pr->pr_domain->dom_dispose != NULL) )
    ( *pr->pr_domain->dom_dispose) (asb.sb_mb);

  /* Release socket buffers */
  sbrelease(&asb);
}

/*******************************************************************************
 * sosend - Send data
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int sosend(struct socket *so, struct mbuf *addr,
	   struct uio *uio, struct mbuf *top, struct mbuf *ctrl,
	   int flags)
{
  struct mbuf **mp;
  struct mbuf *m;
  long space, len, resid;
  int ctlen, err, s, dontroute;
  int canWait, atomic, outcount;

  /* Reset output count */
  outcount = 0;

  /* Get atomic */
  atomic = sosendallatonce(so) || top;

  /* Check uio */
  if (uio != NULL)
    resid = uio->uio_resid;
  else
    resid = top->m_pkthdr.len;

  /* Check resid */
  if (resid < 0)
    return EINVAL;

  /* Get dontroute */
  dontroute = ( (flags & MSG_DONTROUTE) &&
                ( (so->so_options & SO_DONTROUTE) == 0 ) &&
		(so->so_proto->pr_flags & PR_ATOMIC) );

  /* Check control */
  if (ctrl != NULL)
    ctlen = ctrl->m_len;
  else
    ctlen = 0;

  /* Check can wait */
  if (so->so_state & SS_NBIO)
    canWait = M_DONTWAIT;
  else
    canWait = M_WAIT;

  /* Get processor level */
  s = splnet();

restart:

  /* Do while resid */
  do {

    /* If can't send more */
    if (so->so_state & SS_CANTSENDMORE) {

      err = EPIPE;
      splx(s);
      goto out;

    } /* End if can't send more */

    /* If socket error */
    if (so->so_error) {

      err = so->so_error;
      splx(s);
      goto out;

    } /* End if socket error */

    /* If not connected */
    if ( (so->so_state & SS_ISCONNECTED) == 0 ) {

      /* If connection required */
      if (so->so_proto->pr_flags & PR_CONNREQUIRED) {

        /* If not connected error */
        if ( ( (so->so_state & SS_ISCONFIRMING) == 0 ) &&
	     ( !((resid == 0) && (ctlen != 0)) ) ) {

          err = ENOTCONN;
          splx(s);
          goto out;

        } /* End if not connected error */

      } /* End if connection required */

      /* Else if zero address */
      else if (addr == NULL) {

        err = EDESTADDRREQ;
        splx(s);
        goto out;

      } /* End else if zero address */

    } /* End if not connected */

    /* Get send space */
    space = sbspace(&so->so_snd);

    /* If out of band data */
    if (flags & MSG_OOB)
      space += 1024;

    /* If message size error */
    if ( (atomic) && ( (resid > so->so_snd.sb_hiwat) ||
		       (ctlen > so->so_snd.sb_hiwat) ) ) {

      err = EMSGSIZE;
      splx(s);
      goto out;

    } /* End if message size error */

    /* If space left */
    if ( (space < resid + ctlen) &&
	 ( (atomic) || (space < so->so_snd.sb_lowat) || (space < ctlen) ) ) {

      /* If nbio state */
      if (so->so_state & SS_NBIO) {

        if (flags & MSG_MBUF)
          top = NULL;

        err = EWOULDBLOCK;
        splx(s);
        goto out;

      } /* End if nbio state */

      sbwait(&so->so_snd);
      goto restart;

    } /* End if space left */

    /* Advance */
    mp = &top;
    space -= ctlen;

    /* Do while resid and space left */
    do {

      /* If null uio */
      if (uio == NULL) {

        resid = 0;
        if (flags & MSG_EOR)
          top->m_flags |= M_EOR;

      } /* End if null uio */

      /* Else non-null uio */
      else {

        /* Do while space left and atomic */
        do {

          /* Get minimum length */
          len = min(resid, space);

          /* If null top */
          if (top == NULL) {

            /* Allocate memory block with cluster */
            m = m_get(canWait, MT_DATA, len + max_hdr, FALSE);
            if (m == NULL) {

              err = ENOBUFS;
              splx(s);
              goto out;

            }

            /* Calculate length */
            len = min(len, m->m_extSize);

            /* Setup packet */
            m->m_flags |= M_PKTHDR;
            m->m_pkthdr.len = 0;
            m->m_pkthdr.rcvif = NULL;

            /* If atomic and extsize gt. header */
            if ( (atomic) && (m->m_extSize > max_hdr) ) {

              len = min( (m->m_extSize - max_hdr), len );
              m->m_data += max_hdr;

            } /* End if atomic and extsizxe gt. header */

          } /* End if null top */

          /* Else non-null top */
          else {

            /* Allocate memory block with cluster */
            m = m_get(canWait, MT_DATA, len + max_hdr, FALSE);
            if (m == NULL) {

              err = ENOBUFS;
              splx(s);
              goto out;

            }

            /* Calculate length */
            len = min(len, m->m_extSize);

          } /* End else non-null top */

          /* Advance */
          space -= (m->m_extSize + MSIZE);

	  /* Uio move */
	  err = uiomove( mtod(m, char *), (int) len, uio );

          /* Get resid */
	  resid = uio->uio_resid;

          /* Advance */
          m->m_len = len;
          *mp = m;
          top->m_pkthdr.len += len;
          if (err)
            goto release;
          mp = &m->m_next;

          /* If resid */
          if (resid <= 0) {

            if (flags & MSG_EOR)
              top->m_flags |= M_EOR;

            break;

          } /* End if resid */

        } while( (space > 0) && (atomic) );

      } /* End else non-null uio */

      /* If no routing */
      if (dontroute)
        so->so_options |= SO_DONTROUTE;

      /* If not send-all-at-once */
      if ( !sosendallatonce(so) ) {

        top->m_flags &= ~M_EOB;

        /* If end */
        if ( ((resid == 0) || (space <= 0)) && (outcount > 0) )
          top->m_flags |= M_EOB;

          /* Advance */
          outcount++;

          if (resid)
            top->m_flags &= ~M_EOB;

      } /* End if not send-all-at-once */

      /* Do the send call to protocol */
      if (flags & MSG_OOB)
        err = ( *so->so_proto->pr_usrreq) (so, PRU_SENDOOB, top, addr, ctrl);
      else
        err = ( *so->so_proto->pr_usrreq) (so, PRU_SEND, top, addr, ctrl);

      /* If no routing */
      if (dontroute)
        so->so_options &= ~SO_DONTROUTE;

      /* Reset */
      ctlen = 0;
      ctrl = NULL;
      top = NULL;
      mp = &top;

      if (err)
        goto release;

    } while( (resid) &&  (space > 0) );

  } while (resid);

release:

  /* Restore processor level */
  splx(s);

out:

  if (top != NULL)
    m_freem(top);

  if (ctrl != NULL)
    m_freem(ctrl);

  if (err != 0)
    errnoSet(err);

  return err;
}

/*******************************************************************************
 * soreceive - Receive data
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int soreceive(struct socket *so, struct mbuf **paddr, struct uio *uio,
	      struct mbuf **mp0, struct mbuf **ctrlp, int *flagsp)
{
  struct mbuf *m, **mp, *next;
  struct protosw *pr;
  int flags, len, err, s, offset;
  int moff, type;
  int orig_resid;

  /* Initialize locals */
  err = 0;
  type = 0;
  pr = so->so_proto;
  orig_resid = uio->uio_resid;
  mp = mp0;

  if (paddr != NULL)
    *paddr = NULL;
  if (ctrlp != NULL)
    *ctrlp = NULL;

  if (flagsp != NULL)
    flags = *flagsp & ~MSG_EOR;
  else
    flags = 0;

  if (mp != NULL)
    *mp = NULL;

  /* If out-of-band message */
  if (flags & MSG_OOB) {

    /* Allocate buffer */
    m = m_get(M_WAIT, MT_DATA, CL_SIZE_128, TRUE);
    if (m == NULL)
      return ENOBUFS;

    /* Call protocol func */
    err = ( *pr->pr_usrreq) (so, PRU_RCVOOB, m,
			     (struct mbuf *) (long) (flags & MSG_PEEK),
			     (struct mbuf *) NULL);
    if (err)
      goto bad;

    /* If mbuf pointer non-null */
    if (mp != NULL) {

      /* While contents of mp non-null */
      do {

        uio->uio_resid -= m->m_len;
        *mp = m;
        mp = &m->m_next;
        m = m->m_next;

      } while(*mp != NULL);

    } /* End if mbuf pointer non-null */

    /* Else mp null */
    else {

      /* While redisual count */
      do {

        err = uiomove( mtod(m, char *), (int) min(uio->uio_resid, m->m_len),
		       uio );
        m = m_free(m);

      } while ( (uio->uio_resid) && (err == 0) && (m != NULL) );

    } /* End else mp null */

bad:

    if (m != NULL)
      m_freem(m);

    errnoSet(err); 
    return err;

  } /* End if out-of-band message */

  /* If is confirming and residual count */
  if ( (so->so_state & SS_ISCONFIRMING) && (uio->uio_resid) )
    ( *pr->pr_usrreq) (so, PRU_RCVD, (struct mbuf *) NULL,
		       (struct mbuf *) NULL, (struct mbuf *) NULL);

  /* Get processor level */
  s = splnet();

restart:

  m = so->so_rcv.sb_mb;

  /* If less than requested */
  if ( (m == NULL) || ( ((flags & MSG_DONTWAIT) == 0) &&
			(so->so_rcv.sb_cc < uio->uio_resid) &&
			(so->so_rcv.sb_cc < so->so_rcv.sb_lowat) )
		   ||
		      ( (flags & MSG_WAITALL) &&
			(uio->uio_resid <= so->so_rcv.sb_hiwat) &&
			(m->m_nextpkt == NULL) &&
		       ((pr->pr_flags & PR_ATOMIC) == 0) ) ) {

#ifdef NET_DIAGNOSE

    if ( (m == NULL) && (so->so_rcv.sb_cc) )
      panic("receive 1");

#endif /* NET_DIAGNOSE */

    /* If socket error */
    if (so->so_error) {

      if (m != NULL)
        goto dontblock;

      err = so->so_error;
      if ( (flags & MSG_PEEK) == 0)
        so->so_error = 0;

      goto release;

    } /* End if socket error */

    /* If can't receive more */
    if (so->so_state & SS_CANTRCVMORE) {

      if (m != NULL)
        goto dontblock;
      else
        goto release;

    } /* End if can't receive more */

    /* For all mbufs */
    for (; m != NULL; m = m->m_next) {

      /* If out-of-band data */
      if ( (m->m_type == MT_OOBDATA) || (m->m_flags & M_EOR) ) {

        m = so->so_rcv.sb_mb;
        goto dontblock;

      } /* End if out-of-band data */

    } /* End for all mbufs */

    /* If connection required and not connected */
    if ( (so->so_state & (SS_ISCONNECTED | SS_ISCONNECTING) == 0) &&
	 (so->so_proto->pr_flags & PR_CONNREQUIRED) ) {

      err = ENOTCONN;
      goto release;

    } /* End if connection required and not connected */

    /* If residual zero */
    if (uio->uio_resid == 0)
      goto release;

    /* If no block */
    if ( (so->so_state & SS_NBIO) || (flags & MSG_DONTWAIT) ) {

      err = EWOULDBLOCK;
      goto release;

    } /* End if no block */

    sbwait(&so->so_rcv);
    goto restart;

  } /* End if less than requested */

dontblock:

  /* Initialize default next */
  next = m->m_nextpkt;

  /* If protocol address flag */
  if (pr->pr_flags & PR_ADDR) {

#ifdef NET_DIAGNOSE

    if (m->m_type != MT_SONAME)
      panic("receive 1a");

#endif /* NET_DIAGNOSE */

    /* Reset original residual count */
    orig_resid = 0;

    /* If message peek */
    if (flags & MSG_PEEK) {

      if (paddr != NULL)
        *paddr = m_copy(m, 0, m->m_len);

      m = m->m_next;

    } /* End if message peek */

    /* Else not message peek */
    else {

      sbfree(&so->so_rcv, m);

      /* If paddr non-null */
      if (paddr != NULL) {

        *paddr = m;
        so->so_rcv.sb_mb = m->m_next;
        m->m_next = NULL;
        m = so->so_rcv.sb_mb;

      } /* End if paddr non-null */

      /* Else paddr null */
      else {

        so->so_rcv.sb_mb = m_free(m);
        m = so->so_rcv.sb_mb;

      } /* End else paddr null */

    } /* End else not message peek */

  } /* End if protocol address flag */

  /* While control type */
  while ( (m != NULL) && (m->m_type == MT_CONTROL) && (err == 0) ) {

    /* If message peek */
    if (flags & MSG_PEEK) {

      if (ctrlp)
        *ctrlp = m_copy(m, 0, m->m_len);

      m = m->m_next;

    } /* End if message peek */

    /* Else no message peek */
    else {

      sbfree(&so->so_rcv, m);

      /* If ctrl ptr non-null */
      if (ctrlp != NULL) {

        if ( (pr->pr_domain->dom_externalize != NULL) &&
	     (mtod(m, struct cmsghdr *)->cmsg_type == SCM_RIGHTS) )
          err = ( *pr->pr_domain->dom_externalize) (m);

        *ctrlp = m;
        so->so_rcv.sb_mb = m->m_next;
        m->m_next = NULL;
        m = so->so_rcv.sb_mb;

      } /* End if ctrl ptr non-null */

      /* Else ctrl ptr null */
      else {

        so->so_rcv.sb_mb = m_free(m);
        m = so->so_rcv.sb_mb;

      } /* End else ctrl ptr null */

    } /* End else no message peek */

    /* If control ptr non-null */
    if (ctrlp != NULL) {

      orig_resid = 0;
      ctrlp = &(*ctrlp)->m_next;

    } /* End if control ptr non-null */

  } /* End while control type */

  /* If mbuf non-null */
  if (m != NULL) {

    if ( (flags & MSG_PEEK) == 0 )
      m->m_nextpkt = next;

    if (type == MT_OOBDATA)
      flags |= MSG_OOB;
    
  } /* End if mbuf non-null */

  /* Reset counters */
  moff = 0;
  offset = 0;

  /* While residual gt. zero */
  while ( (m != NULL) && (uio->uio_resid > 0) && (err == 0) ) {

    /* If out-of-band data */
    if (m->m_type == MT_OOBDATA) {

      if (type != MT_OOBDATA)
        break;

    } /* End if out-of-band data */

    /* Else if req out-of-band data */
    else if (type == MT_OOBDATA) {
 
      break;

    } /* End else if req out-of-band data */

#ifdef NET_DIAGNOSE

    else if ( (m->m_type != MT_DATA) && (m->m_type != MT_HEADER) )
      panic("receive 3");

#endif /* NET_DIAGNOSE */

    so->so_state &= ~SS_RCVATMARK;
    len = uio->uio_resid;

    if ( (so->so_oobmark) && (len > (so->so_oobmark - offset)) )
      len = so->so_oobmark - offset;

    if (len > (m->m_len - moff))
      len = m->m_len - moff;

    if (mp == NULL)
      err = uiomove( mtod(m, char *) + moff, (int) len, uio);
    else
      uio->uio_resid -= len;

    /* If length equal */
    if (len == (m->m_len - moff)) {

      if (m->m_flags & M_EOR)
        flags |= MSG_EOR;

      /* If message peek */
      if (flags & MSG_PEEK) {

        m = m->m_next;
        moff = 0;

      } /* End if message peek */

      /* Else no message peek */
      else {

        next = m->m_nextpkt;
        sbfree(&so->so_rcv, m);

        /* If mbuf pointer non-null */
        if (mp != NULL) {

          *mp = m;
          mp = &m->m_next;
          so->so_rcv.sb_mb = m = m->m_next;
          *mp = NULL;

        } /* End if mbuf pointer non-null */

        /* Else mbuf pointer null */
        else {

          so->so_rcv.sb_mb = m_free(m);
          m = so->so_rcv.sb_mb;

        } /* End else mbuf pointer null */

        if (m != NULL)
          m->m_nextpkt = next;

      } /* End else no message peek */

    } /* End if length equal */

    /* Else length not equal */
    else {

      /* If message peek */
      if (flags & MSG_PEEK) {

        moff += len;

      } /* If message peek */

      /* Else no message peek */
      else {

        /* If mp non-null */
        if (mp != NULL) {

          *mp = m_copym(m, 0, len, M_WAIT);
          if (*mp == NULL) {

            err = ENOBUFS;
            goto release;

          }

        } /* If mp non-null */

        /* Advace */
        m->m_data += len;
        m->m_len -= len;
        so->so_rcv.sb_cc -= len;

      } /* End else no message peek */

    } /* End else length not equal */

   /* If out-of-band mark */
   if (so->so_oobmark) {

     /* If no message peek */
     if ( (flags & MSG_PEEK) == 0 ) {

       /* Advance */
       so->so_oobmark -= len;

       /* If out-of-band mark reached zero */
       if (so->so_oobmark == 0) {

         so->so_state |= SS_RCVATMARK;
         break;

       } /* End if out-of-band mark reached zero */

     } /* End if no message peek */

     /* Else message peek */
     else {

       /* Advance */
       offset += len;
       if (offset == so->so_oobmark)
         break;

     } /* End else message peek */

   } /* End if out-of-band mark */

   if (flags & MSG_EOR)
     break;

    /* If wait for all */
    while ( (flags & MSG_WAITALL) && (m == NULL) && (uio->uio_resid > 0) &&
         (!sosendallatonce(so)) && (next == NULL) ) {

      if ( (so->so_error) || (so->so_state & SS_CANTRCVMORE) )
        break;

      sbwait(&so->so_rcv);

      m = so->so_rcv.sb_mb;
      if (m != NULL)
        next = m->m_nextpkt;

    } /* End if wait for all */

  } /* End while residual gt. zero */

  /* If protocol atomic */
  if ( (m != NULL) && (pr->pr_flags & PR_ATOMIC) ) {

    flags |= MSG_TRUNC;
    if ( (flags & MSG_PEEK) == 0)
      sbdroprecord(&so->so_rcv);

  } /* End if protocol atomic */

  /* If no message peek */
  if ( (flags & MSG_PEEK) == 0 ) {

    if (m == NULL)
      so->so_rcv.sb_mb = next;

    if ( (pr->pr_flags & PR_WANTRCVD) && (so->so_pcb) )
      ( *pr->pr_usrreq) (so, PRU_RCVD, (struct mbuf *) NULL,
			 (struct mbuf *) (long) flags,
			 (struct mbuf *) NULL, (struct mbuf *) NULL);

  } /* End if no message peek */

  if ( (orig_resid == uio->uio_resid) && (orig_resid) &&
       ((flags & MSG_EOR) == 0) && ((so->so_state & SS_CANTRCVMORE) == 0) )
    goto restart;

  /* Store flags */
  if (flagsp != NULL)
    *flagsp |= flags;

release:

  /* Restore processor level */
  splx(s);

  if (err)
    errnoSet(err);

  return err;
}

/*******************************************************************************
 * sosetopt - Set socket options
 *
 * RETURNS: Errno
 ******************************************************************************/

int sosetopt(struct socket *so, int level, int optname, struct mbuf *m)
{
  int err;

  /* Setup locals */
  err = 0;

  /* If level not sol socket */
  if (level != SOL_SOCKET) {

    if ( (so->so_proto != NULL) && (so->so_proto->pr_ctloutput) )
      return ( *so->so_proto->pr_ctloutput) (PRCO_SETOPT, so, level,
					     optname, &m);

    err = ENOPROTOOPT;

  } /* End if level not sol socket */

  /* Else level sol socket */
  else {

    /* Select option */
    switch(optname) {

      case SO_LINGER:

        /* If null mbuf and linger size */
        if ( (m == NULL) || (m->m_len != sizeof(struct linger)) ) {

          err = EINVAL;
          goto bad;

        } /* End if null mbuf and linger size */

        /* Set linger time */
        so->so_linger = mtod(m, struct linger *)->l_linger;

        /* FALL THRU */

      case SO_DEBUG:
      case SO_KEEPALIVE:
      case SO_DONTROUTE:
      case SO_USELOOPBACK:
      case SO_BROADCAST:
      case SO_REUSEADDR:
      case SO_REUSEPORT:
      case SO_OOBINLINE:
      case SO_USEPATHMTU:

        /* If null mbuf and int size */
        if ( (m == NULL) || (m->m_len != sizeof(int)) ) {

          err = EINVAL;
	  goto bad;

        } /* End if null mbuf and int size */

        /* Set/reset option */
        if ( mtod(m, int *) != NULL )
          so->so_options |= optname;
        else
          so->so_options &= ~optname;

      break;

      case SO_SNDBUF:
      case SO_RCVBUF:
      case SO_SNDLOWAT:
      case SO_RCVLOWAT:

        /* If null mbuf and int size */
        if ( (m == NULL) || (m->m_len != sizeof(int)) ) {

          err = EINVAL;
	  goto bad;

        } /* End if null mbuf and int size */

        /* Select optname */
        switch(optname) {

          case SO_SNDBUF:
          case SO_RCVBUF:

          /* If reserve failed */
          if ( sbreserve( (optname == SO_SNDBUF) ? &so->so_snd : &so->so_rcv,
			  (unsigned long) *mtod(m, int *) ) == 0) {

            err = ENOBUFS;
            goto bad;

          } /* End if reserve failed */

          break;

          case SO_SNDLOWAT:
            so->so_snd.sb_lowat = *mtod(m, int *);
          break;

          case SO_RCVLOWAT:
            so->so_rcv.sb_lowat = *mtod(m, int *);
          break;

        } /* End select optname */

      break;

      case SO_SNDTIMEO:
      case SO_RCVTIMEO:

        /* Select optname */
        switch(optname) {

          case SO_SNDTIMEO:
            so->so_snd.sb_timeo = *mtod(m, int *);
          break;

          case SO_RCVTIMEO:
            so->so_rcv.sb_timeo = *mtod(m, int *);
          break;

        } /* End select optname */

      break;

      default:

        err = ENOPROTOOPT;

      break;

    } /* End select option */

    /* If zero error and ctl proto output func */
    if ( (err == 0) && (so->so_proto && so->so_proto->pr_ctloutput != NULL) ) {

      ( *so->so_proto->pr_ctloutput) (PRCO_SETOPT, so, level, optname, &m);
      m = NULL;

    } /* End if zero error and ctl proto output func */

  } /* End else level not sol socket */

bad:

  if (m != NULL)
    m_free(m);

  return err;
}

/*******************************************************************************
 * sogetopt - Get socket options
 *
 * RETURNS: Errno
 ******************************************************************************/

int sogetopt(struct socket *so, int level, int optname, struct mbuf **m)
{
  struct mbuf *mNew;
  int err;

  /* Setup locals */
  err = 0;

  /* If level not sol socket */
  if (level != SOL_SOCKET) {

    if ( (so->so_proto != NULL) && (so->so_proto->pr_ctloutput) )
      return ( *so->so_proto->pr_ctloutput) (PRCO_GETOPT, so, level,
					     optname, &m);

    err = ENOPROTOOPT;

  } /* End if level not sol socket */
  /* Else level sol socket */
  else {

    /* Allocate memory for data */
    mNew = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
    if (mNew == NULL)
      return ENOBUFS;

    /* Set size */
    mNew->m_len = sizeof(int);

    /* Select optname */
    switch(optname) {

      case SO_LINGER:

        mNew->m_len = sizeof(struct linger);
        mtod(mNew, struct linger *)->l_onoff = so->so_options & SO_LINGER;
        mtod(mNew, struct linger *)->l_linger = so->so_linger;

      break;

      case SO_USELOOPBACK:
      case SO_DONTROUTE:
      case SO_DEBUG:
      case SO_KEEPALIVE:
      case SO_REUSEADDR:
      case SO_REUSEPORT:
      case SO_BROADCAST:
      case SO_OOBINLINE:

        *mtod(mNew, int *) = so->so_options & optname;

      break;

      case SO_TYPE:

        *mtod(mNew, int *) = so->so_type;

      break;

      case SO_ERROR:

        *mtod(mNew, int *) = so->so_error;
        so->so_error = 0;

      break;

      case SO_SNDBUF:

        *mtod(mNew, int *) = so->so_snd.sb_hiwat;

      break;

      case SO_RCVBUF:

        *mtod(mNew, int *) = so->so_rcv.sb_hiwat;

      break;

      case SO_SNDLOWAT:

        *mtod(mNew, int *) = so->so_snd.sb_lowat;

      break;

      case SO_RCVLOWAT:

        *mtod(mNew, int *) = so->so_rcv.sb_lowat;

      break;

      case SO_SNDTIMEO:
      case SO_RCVTIMEO:

        if (optname == SO_SNDTIMEO)
          *mtod(mNew, int *) = so->so_snd.sb_timeo;
        else
          *mtod(mNew, int *) = so->so_rcv.sb_timeo;

      break;

      default:

        m_free(mNew);
        return ENOPROTOOPT;

    } /* End select optname */

    *m = mNew;
    return 0;

  } /* End else level not sol socket */

}

/*******************************************************************************
 * sodisconnect - Disconnet socket
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int sodisconnect(struct socket *so)
{
  int s, err;

  /* Get processor level */
  s = splnet();

  /* If not connected */
  if ( (so->so_state & SS_ISCONNECTED) == 0) {

    splx(s);
    return ENOTCONN;

  } /* End if not connected */

  /* If too early */
  if ( (so->so_state & SS_ISDISCONNECTING) == 0) {

    splx(s);
    return EALREADY;

  } /* End if too early */

  /* Call function */
  err = ( *so->so_proto->pr_usrreq) (so, PRU_DISCONNECT, (struct mbuf *) NULL,
				    (struct mbuf *) NULL, (struct mbuf *) NULL);

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * soclose - Close socket
 *
 * RETURNS: Protocol specific or zero
 ******************************************************************************/

int soclose(struct socket *so)
{
  int s, err, err2;
  SEL_WAKEUP_NODE *pWakeupNode;
  SEL_WAKEUP_LIST *pWakeupList;

  /* Setup locals */
  err = 0;

  /* Get processor level */
  s = splnet();

  /* If connection(s) accepted */
  if (so->so_options & SO_ACCEPTCONN) {

    /* Abort queues */
    while (so->so_q0 != NULL)
      soabort(so->so_q0);
    while (so->so_q != NULL)
      soabort(so->so_q);

  } /* End if connection(s) accepted */

  if (so->so_pcb == NULL)
    goto discard;

  /* If connected */
  if (so->so_state & SS_ISCONNECTED) {

    /* If not disconnecting */
    if ( (so->so_state & SS_ISDISCONNECTING) == 0) {

      err = sodisconnect(so);
      if (err)
        goto drop;

    } /* End if not disconnecting */

    /* If linger */
    if (so->so_options & SO_LINGER) {

      if ( (so->so_state & SS_ISDISCONNECTING) &&
	   (so->so_state & SS_NBIO) )
        goto drop;

      while (so->so_state & SS_ISCONNECTED)
        ksleep(so->so_timeoSem);

    } /* End if linger */

  } /* End if connected */

drop:

  /* If pcb */
  if (so->so_pcb != NULL) {

    err2 = (*so->so_proto->pr_usrreq) (so, PRU_DETACH,
				       (struct mbuf *) NULL,
				       (struct mbuf *) NULL,
				       (struct mbuf *) NULL);
    if (err == 0)
      err = err2;

  } /* End if pcb */

discard:

  /* Get wakeup list */
  pWakeupList = &so->so_selWakeupList;

  /* If nodes in wakeup list */
  if ( listCount(&pWakeupList->wakeupList) != 0 ) {

    semTake(&pWakeupList->mutex, WAIT_FOREVER);

    while ( (pWakeupNode = (SEL_WAKEUP_NODE *) LIST_HEAD(&pWakeupList)))
      soo_unselect(so, pWakeupNode);

    semGive(&pWakeupList->mutex);

  } /* End if nodes in wakeup list */

  /* If no fd reference */
  if (so->so_state & SS_NOFDREF)
    panic("soclose: NOFDREF");

  /* Invalidate fd reference */
  so->so_state |= SS_NOFDREF;

  /* Free socket */
  sofree(so);

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * soshutdown - Shutdown socket
 *
 * RETURNS: Protocol specific or zero
 ******************************************************************************/

int soshutdown(struct socket *so, int how)
{
  struct protosw *pr;

  /* Get protocol switch */
  pr = so->so_proto;

  how++;
  if (how & 1)
    sorflush(so);
  if (how & 2)
    return ( *pr->pr_usrreq) (so, PRU_SHUTDOWN, (struct mbuf *) NULL,
			      (struct mbuf *) NULL, (struct mbuf *) NULL);

  return 0;
}

/*******************************************************************************
 * soabort - Abort socket operation
 *
 * RETURNS: Protocol specific
 ******************************************************************************/

int soabort(struct socket *so)
{
  return ( *so->so_proto->pr_usrreq) (so, PRU_ABORT, (struct mbuf *) NULL,
				      (struct mbuf *) NULL,
				      (struct mbuf * ) NULL);
}

/*******************************************************************************
 * sofree - Free a socket
 *
 * RETURNS: N/A
 ******************************************************************************/

void sofree(struct socket *so)
{
  /* If all pointers must be checked */
  if (so->so_state & SS_NOFDREF) {

    if (so->so_timeoSem != NULL) {

      semDelete(so->so_timeoSem);
      so->so_timeoSem = NULL;

    }

    if (so->so_snd.sb_sem != NULL) {

      semDelete(so->so_snd.sb_sem);
      so->so_snd.sb_sem = NULL;

    }

    if (so->so_rcv.sb_sem != NULL) {

      semDelete(so->so_rcv.sb_sem);
      so->so_rcv.sb_sem = NULL;

    }

    if (so->selectFlag) {

      selWakeupListTerminate(&so->so_selWakeupList);
      so->selectFlag = FALSE;

    }

  } /* End if all pointers must be checked */

  /* Check if we are done here */
  if ( (so->so_pcb != NULL) || ((so->so_state & SS_NOFDREF) == 0) )
    return;

  /* If items on queue */
  if (so->so_head != NULL) {

    if ( (!soqremque(so, 0)) && (!soqremque(so, 1)) )
      panic("sofree dq");

     /* Reset queue head */
     so->so_head = NULL;

  } /* End if items on queue */

  /* Release send buffer */
  sbrelease(&so->so_snd);

  /* Flush */
  sorflush(so);

  /* Free memory */
  mb_free(so);
}

/*******************************************************************************
 * sowakeup - Wakeup socket
 *
 * RETURNS: N/A
 ******************************************************************************/

void sowakeup(struct socket *so, struct sockbuf *sb, SELECT_TYPE selType)
{
  sbwakeup(so, sb, selType);
}

/*******************************************************************************
 * soisconnecting - Is connecting
 *
 * RETURNS: N/A
 ******************************************************************************/

void soisconnecting(struct socket *so)
{
  /* Reset other flags */
  so->so_state &= ~(SS_ISCONNECTED | SS_ISDISCONNECTING);

  /* Set connecting flag */
  so->so_state |= SS_ISCONNECTING;
}

/*******************************************************************************
 * soisconnected - Is connected
 *
 * RETURNS: N/A
 ******************************************************************************/

void soisconnected(struct socket *so)
{
  struct socket *head;
  short prevState;

  /* Setup locals */
  head = so->so_head;
  prevState = so->so_state;

  /* Reset flags */
  so->so_state &= ~(SS_ISCONNECTING | SS_ISDISCONNECTING | SS_ISCONFIRMING);

  /* Set flags */
  so->so_state |= SS_ISCONNECTED;

  /* If queue not empty */
  if ( (head != NULL) && (soqremque(so, 0)) ) {

    soqinsque(head, so, 1);
    sorwakeup(head);
    wakeup(head->so_timeoSem);

  } /* End if queue not empty */

  /* Else queue empty */
  else {

    wakeup(so->so_timeoSem);

    if ( (prevState & SS_ISCONNECTING) == 0 )
      sorwakeup(so);

    sowwakeup(so);

  } /* End else queue empty */
}

/*******************************************************************************
 * socantsendmore - Can't send more
 *
 * RETURNS: N/A
 ******************************************************************************/

void socantsendmore(struct socket *so)
{
  so->so_state |= SS_CANTSENDMORE;
  sowwakeup(so);
}

/*******************************************************************************
 * socantrcvmore - Can't receive more
 *
 * RETURNS: N/A
 ******************************************************************************/

void socantrcvmore(struct socket *so)
{
  so->so_state |= SS_CANTRCVMORE;
  sowwakeup(so);
}

/*******************************************************************************
 * soisdiconnecting - Mark as disconnecting
 *
 * RETURNS: N/A
 ******************************************************************************/

void soisdisconnecting(struct socket *so)
{
  so->so_state &= ~SS_ISCONNECTING;
  so->so_state |= (SS_ISDISCONNECTING | SS_CANTRCVMORE | SS_CANTSENDMORE);

  if (so->so_timeoSem != NULL)
    wakeup(so->so_timeoSem);

  sowwakeup(so);
  sorwakeup(so);
}

/*******************************************************************************
 * soisdiconnected - Mark as disconnected
 *
 * RETURNS: N/A
 ******************************************************************************/

void soisdisconnected(struct socket *so)
{
  so->so_state &= ~(SS_ISCONNECTING | SS_ISCONNECTED | SS_ISDISCONNECTING);
  so->so_state |= (SS_CANTRCVMORE | SS_CANTSENDMORE);

  if (so->so_timeoSem != NULL)
    wakeup(so->so_timeoSem);

  sowwakeup(so);
  sorwakeup(so);
}

/*******************************************************************************
 * sonewconn - New socket connection
 *
 * RETURNS: Pointer to socket
 ******************************************************************************/

struct socket* sonewconn(struct socket *head, int connstatus)
{
  struct socket *so;
  int soqueue;

  /* Initialize locals */
  if (connstatus)
    soqueue = 1;
  else
    soqueue = 0;

  /* If no more room */
  if (head->so_qlen + head->so_q0len > 3 * head->so_qlimit / 2)
    return NULL;

  /* Allocate memory */
  so = mb_alloc(sizeof(struct socket), MT_SOCKET, M_DONTWAIT);
  if (so == NULL)
    return NULL;

  /* Clear struct */
  memset(so, 0, sizeof(struct socket));

  /* Setup struct */
  so->so_type = head->so_type;
  so->so_options = head->so_options & ~SO_ACCEPTCONN;
  so->so_linger = head->so_linger;
  so->so_state = head->so_state | SS_NOFDREF;
  so->so_proto = head->so_proto;
  so->so_timeo = head->so_timeo;
  so->so_pgrp = head->so_pgrp;

  /* Initialize a new semaphores */
  so->so_timeoSem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_timeoSem == NULL) {

    mb_free(so);
    return NULL;

  }

  so->so_rcv.sb_sem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_rcv.sb_sem == NULL) {

    semDelete(so->so_timeoSem);
    mb_free(so);
    return NULL;

  }

  so->so_snd.sb_sem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
  if (so->so_snd.sb_sem == NULL) {

    semDelete(so->so_timeoSem);
    semDelete(so->so_rcv.sb_sem);
    mb_free(so);
    return NULL;

  }

  selWakeupListInit(&so->so_selWakeupList);
  so->selectFlag = TRUE;

  /* Reserve space */
  soreserve(so, head->so_snd.sb_hiwat, head->so_rcv.sb_hiwat);

  /* Insert into queue */
  soqinsque(head, so, soqueue);

  /* If attach fails */
  if ( (* so->so_proto->pr_usrreq) (so, PRU_ATTACH, NULL, NULL, NULL) ) {

    soqremque(so, soqueue);
    semDelete(so->so_timeoSem);
    semDelete(so->so_rcv.sb_sem);
    semDelete(so->so_snd.sb_sem);

    if (so->selectFlag) {

      selWakeupListTerminate(&so->so_selWakeupList);
      so->selectFlag = FALSE;

    }

    mb_free(so);
    return NULL;

  } /* End if attack fails */

  /* If connected */
  if (connstatus) {

    sorwakeup(head);
    wakeup(head->so_timeoSem);
    so->so_state |= connstatus;

  } /* End if conncted */

  return so;
}

/*******************************************************************************
 * soqinsque - Insert into queue
 *
 * RETURNS: One or zero
 ******************************************************************************/

void soqinsque(struct socket *head, struct socket *so, int q)
{
  struct socket **prev;

  /* Set head */
  so->so_head = head;

  /* If zero queue arument */
  if (q == 0) {

    head->so_q0len++;
    so->so_q0 = NULL;

    /* Reverse walk in queue */
    for (prev = &head->so_q0; *prev != NULL; )
       prev = &((*prev)->so_q0);

  } /* End if null queue */

  /* Else non-zero queue */
  else {

    head->so_qlen++;
    so->so_q = NULL;

    /* Reverse walk in queue */
    for (prev = &head->so_q; *prev != NULL; )
      prev = &((*prev)->so_q);

  } /* End else non-zero queue */

  /* Set previous element in queue */
  *prev = so;
}

/*******************************************************************************
 * soqremque - Remove queue
 *
 * RETURNS: One or zero
 ******************************************************************************/

int soqremque(struct socket *so, int q)
{
  struct socket *head, *prev, *next;

  /* Get queue head */
  head = so->so_head;
  prev = head;

  /* Endless loop */
  while(1) {

    /* Get next element */
    if (q)
      next = prev->so_q;
    else
      next = prev->so_q0;

    /* If match */
    if (next == so)
      break;

    /* If next null */
    if (next == NULL)
      return 0;

    /* Store last element */
    prev = next;

  } /* End endless loop */

  /* If zero queue arument */
  if (q == 0) {

    prev->so_q0 = next->so_q0;
    head->so_q0len--;

  } /* End if null queue */

  /* Else non-null queue */
  else {

    prev->so_q = next->so_q;
    head->so_qlen--;

  } /* End else non-null queue */

  /* Zero all queue pointers */
  next->so_q0 = NULL;
  next->so_q = NULL;
  next->so_head = NULL;

  return 1;
}

/*******************************************************************************
 * soreserve - Reserve space for socket
 *
 * RETURNS: Zero or error
 ******************************************************************************/

int soreserve(struct socket *so,
	      unsigned long sndcc,
	      unsigned long rcvcc)
{

  /* Reserve send buffer */
  if ( sbreserve(&so->so_snd, sndcc) == 0 )
    return ENOBUFS;

  /* Reserve receive buffer */
  if ( sbreserve(&so->so_rcv, rcvcc) == 0 ) {

    sbrelease(&so->so_snd);
    return ENOBUFS;

  }

  if (so->so_rcv.sb_lowat == 0)
    so->so_rcv.sb_lowat = 1;

  if (so->so_snd.sb_lowat == 0)
    so->so_snd.sb_lowat = CL_SIZE_64;

  if (so->so_snd.sb_lowat > so->so_snd.sb_hiwat)
    so->so_snd.sb_lowat = so->so_snd.sb_hiwat;

  return 0;
}

/*******************************************************************************
 * sohasoutofband - Has out of band data
 *
 * RETURNS: N/A
 ******************************************************************************/

void sohasoutofband(struct socket *so)
{
}

/*******************************************************************************
 * soo_unselect - Unselect socket
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int soo_unselect(struct socket *so, SEL_WAKEUP_NODE *wakeupNode)
{
  /* If select type */
  switch(selWakeupType(wakeupNode)) {

    case SELREAD:
      sbseldequeue(so, &so->so_rcv, wakeupNode);
    break;

    case SELWRITE:
      sbseldequeue(so, &so->so_snd, wakeupNode);
    break;

    default:
      return ERROR;

  } /* End select type */

  return OK;
}

/*******************************************************************************
 * soo_ioctl - Socket ioctl
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

int soo_ioctl(struct socket *so, int cmd, void *data)
{
  int err;

  /* Initialize locals */
  err = 0;

  /* Select commnand */
  switch (cmd) {

    case FIONBIO:

      if ( *(int *) data)
        so->so_state |= SS_NBIO;
      else
        so->so_state &= ~SS_NBIO;

      return 0;

    case FIONREAD:

      *(int *) data = so->so_rcv.sb_cc;

      return 0;

    case SIOCSPGRP:

      so->so_pgrp = *(int *) data;

      return 0;

    case SIOCGPGRP:

      *(int *) data = so->so_pgrp;

      return 0;

    case SIOCATMARK:

      *(int *) data = (so->so_state & SS_RCVATMARK) != 0;

      return 0;

    case FIOSELECT:

      /* NOTE: Not implemented yet */

      return EOPNOTSUPP;

    case FIOUNSELECT:

      /* NOTE: Not implemented yet */

      return EOPNOTSUPP;

  } /* End select commnand */

  /* If command byte interface */
  if (IOCGROUP(cmd) == 'i') {

    err = ifioctl(so, cmd, data);

  } /* End if command byte interface */

  /* Else not interface command */
  else {

    if (IOCGROUP(cmd) == 'r')
      err = rtioctl(cmd, data);
    else
      err = ( *so->so_proto->pr_usrreq)
	      (so,
	      PRU_CONTROL,
	      (struct mbuf *) cmd,
	      (struct mbuf *) data,
	      (struct mbuf *) NULL);

  } /* End else not interface command */

  /* If error */
  if (err) {

    errnoSet(err);
    return ERROR;

  }

  return OK;
}

