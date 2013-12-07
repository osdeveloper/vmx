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

/* sbLib.c - Socket buffer primitives library */

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

/* Locals */

/* Globals */
int sb_max = SB_MAX;

/* Functions */

/*******************************************************************************
 * soLibInit - Inititalize socket buffer library
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbLibInit(int sbMax)
{
  sb_max = sbMax;
}

/*******************************************************************************
 * sbcompress - Compress send/receive buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbcompress(struct sockbuf *sb, struct mbuf *m, struct mbuf *n)
{
  int eor;
  struct mbuf *o;

  /* Initialize locals */
  eor = 0;

  /* While buffer non-null */
  while (m != NULL) {

    /* Get end-of-record */
    eor |= m->m_flags & M_EOR;

    /* If packet can be removed */
    if ( (m->m_len == 0) &&
         ( (eor == 0) ||
          ( (((o = m->m_next) != NULL) || ((o = n) != NULL)) &&
           (o->m_type == m->m_type) )) ) {

      m = m_free(m);
      continue;

    } /* End if packet can be removed */

    /* If data copy possible */
    if ( (n != NULL) &&
         ((n->m_flags & M_EOR) == 0) &&
         (m->m_len < CL_SIZE_MIN) &&
         (n->m_data + n->m_len + m->m_len) < (n->m_extBuf + n->m_extSize) &&
         (n->m_type == m->m_type) ) {

      memcpy( mtod(n, char *) + n->m_len, mtod(m, char *), m->m_len );
      n->m_len += m->m_len;
      sb->sb_cc += m->m_len;
      m = m_free(m);
      continue;

    } /* End if data copy possible */

    if (n != NULL)
      n->m_next = m;
    else
      sb->sb_mb = m;

    /* Allocate buffer */
    sballoc(sb, m);

    n = m;
    m->m_flags &= ~M_EOR;
    m = m->m_next;
    n->m_next = NULL;

  } /* End while buffer non-null */

  /* If end-of-record */
  if (eor) {

    if (n != NULL)
      n->m_flags |= eor;
    else
      printf("semi-panic: sbcompress\n");

  } /* End if end-of-record */
}

/*******************************************************************************
 * sbflush - Flush send receive buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbflush(struct sockbuf *sb)
{
  /* If locked */
  if (sb->sb_flags & SB_LOCK)
    panic("sbflush");

  /* While buffers to drop */
  while (sb->sb_mbcnt)
    sbdrop(sb, (int) sb->sb_cc);

  if ( (sb->sb_cc) || (sb->sb_mb != NULL) )
    panic("sbflush 2");
}

/*******************************************************************************
 * sbdrop - Send receive buffer drop
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbdrop(struct sockbuf *sb, int len)
{
  struct mbuf *m, *mn, *next;

  /* Get mbuf */
  m = sb->sb_mb;

  /* Get next packet */
  if (m != NULL)
    next = m->m_nextpkt;
  else
    next = NULL;

  /* While length gt. zero */
  while (len > 0) {

    /* If null mbuf */
    if (m == NULL) {

      if (next == NULL)
        panic("sbdrop");

      /* Advance */
      m = next;
      next = m->m_nextpkt;
      continue;

    } /* End if null mbuf */

    /* If more length */
    if (m->m_len > len) {

      /* Advance beyond lenth */
      m->m_len -= len;
      m->m_data += len;
      sb->sb_cc -= len;
      break;

    } /* End if more length */

    /* Advance length */
    len -= m->m_len;

    /* Free buffer */
    sbfree(sb, m);

    /* Free mbuf */
    mn = m_free(m);
    m = mn;

  } /* End while length gt. zero */

  /* While m not null and length zero */
  while ( (m != NULL) && (m->m_len == 0) )

    sbfree(sb, m);
    mn = m_free(m);
    m = mn;

  /* End wile m not null and length zero */

  /* If non-null mbuf */
  if (m != NULL) {

    sb->sb_mb = m;
    m->m_nextpkt = next;

  } /* End if non-null mbuf */

  /* Else null mbuf */
  else {

    sb->sb_mb = next;

  } /* End else null mbuf */
}

/*******************************************************************************
 * sbdroprecord - Send receive buffer drop record
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbdroprecord(struct sockbuf *sb)
{
  struct mbuf *m, *mn;

  /* Get buffer */
  m = sb->sb_mb;

  /* If non-null */
  if (m != NULL) {

    sb->sb_mb = m->m_nextpkt;

    do {

      sbfree(sb, m);
      mn = m_free(m);

    } while ( (m = mn) != NULL);

  } /* End if non-null */
}

/*******************************************************************************
 * sbrelease - Release send receive buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbrelease(struct sockbuf *sb)
{
  sbflush(sb);
  sb->sb_hiwat = 0;
  sb->sb_mbmax = 0;
}

/*******************************************************************************
 * sbwakeup - Wakeup send recive buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbwakeup(struct socket *so, struct sockbuf *sb, SELECT_TYPE selType)
{

  /* If want */
  if (sb->sb_want > 0) {

    sb->sb_want--;
    if (sb->sb_sem != NULL)
      wakeup(sb->sb_sem);

  } /* If want */

  /* Else noting wanted */
  else {

    sb->sb_want = 0;

  } /* End else nothing wanted */

  /* If selected */
  if (sb->sb_sel) {

    if (so->selectFlag)
      selWakeupAll(&so->so_selWakeupList, selType);

  } /* End if selected */
}

/*******************************************************************************
 * sbseldequeue - Dequeue select node
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbseldequeue(struct socket *so,
                  struct sockbuf *sb,
                  SEL_WAKEUP_NODE *wakeupNode)
{
  selNodeDelete(&so->so_selWakeupList, wakeupNode);

  if (selWakeupListLen(&so->so_selWakeupList) == 0)
    sb->sb_sel = NULL;
}

/*******************************************************************************
 * sbwait - Send receive buffer wait
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbwait(struct sockbuf *sb)
{
  sb->sb_want++;
  sb->sb_flags |= SB_WAIT;

  ksleep(sb->sb_sem);
}

/*******************************************************************************
 * sbreserve - Allot mbufs to a sockbuf
 *
 * RETURNS: One or zero
 ******************************************************************************/

int sbreserve(struct sockbuf *sb, unsigned long cc)
{
  /* If cc to large */
  if ( cc > sb_max * MCLBYTES / (MSIZE + MCLBYTES) )
    return 0;

  sb->sb_hiwat = cc;
  sb->sb_mbmax = min(cc * 5, sb_max);

  /* If lowat to large */
  if (sb->sb_lowat > sb->sb_hiwat)
    sb->sb_lowat = sb->sb_hiwat;

  return 1;
}

/*******************************************************************************
 * sbappend - Append mbuf chain to last record
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbappend(struct sockbuf *sb, struct mbuf *m)
{
  struct mbuf *n;

  if (m == NULL)
    return;

  n = sb->sb_mb;

  /* If buffer non-null */
  if (n != NULL) {

    /* Goto last packet */
    while (n->m_nextpkt != NULL)
      n = n->m_nextpkt;

    /* While more buffers */
    do {

      /* If end-of-record */
      if (n->m_flags & M_EOR) {

        sbappendrecord(sb, m);
        return;

      } /* End if end-of-record */

    } while ( (n->m_next != NULL) && ((n = n->m_next) != NULL) );

  } /* End if buffer non-null */

  sbcompress(sb, m, n);
}

/*******************************************************************************
 * sbappendrecord - Append record to mbuf chain
 *
 * RETURNS: N/A
 ******************************************************************************/

void sbappendrecord(struct sockbuf *sb, struct mbuf *m0)
{
  struct mbuf *m;

  if (m0 == NULL)
    return;

  m = sb->sb_mb;

  /* If non-null buffer */
  if (m != NULL) {

    while (m->m_nextpkt != NULL)
      m = m->m_nextpkt;

  } /* End if non-null buffer */

  /* Allocate buffer */
  sballoc(sb, m0);

  if (m != NULL)
    m->m_nextpkt = m0;
  else
    sb->sb_mb = m0;

  m = m0->m_next;
  m0->m_next = NULL;

  /* If end-of-record */
  if ( (m != NULL) &&
        (m0->m_flags & M_EOR) ) {

    m0->m_flags &= ~M_EOR;
    m->m_flags |= M_EOR;

  } /* End if end-of-record */

  sbcompress(sb, m, m0);
}

/*******************************************************************************
 * sbappendaddr - Append address and data
 *
 * RETURNS: Zero if space to big else non zero
 ******************************************************************************/

int sbappendaddr(struct sockbuf *sb,
                 struct sockaddr *asa,
                 struct mbuf *m0,
                 struct mbuf *ctrl)
{
  int space;
  struct mbuf *m, *n;

  if ( (m0 != NULL) && ((m0->m_flags & M_PKTHDR) == 0) )
    panic("sbappendaddr");

  /* Initialize locals */
  space = asa->sa_len;

  /* If m0 non-null */
  if (m0 != NULL)
    space += m0->m_pkthdr.len;

  /* For all control blocks */
  for (n = ctrl; n != NULL; n = n->m_next) {

    /* Advace */
    space += n->m_len;

    if (n->m_next == NULL)
      break;

  } /* End for all control blocks */

  /* If space full */
  if (space > sbspace(sb))
    return 0;

  /* If len to big */
  if (asa->sa_len > CL_SIZE_128)
    return 0;

  /* Get clust block */
  m = m_get(M_DONTWAIT, MT_SONAME, CL_SIZE_128, TRUE);
  if (m == NULL)
    return 0;

  m->m_len = asa->sa_len;
  memcpy( mtod(m, char *), asa, asa->sa_len );

  if (n != NULL)
    n->m_next = m0;
  else
    ctrl = m0;

  m->m_next = ctrl;

  /* Allocate buffers */
  for (n = m; n != NULL; n = n->m_next)
    sballoc(sb, n);

  /* Set local pointer */
  n = sb->sb_mb;

  /* If local pointer non-null */
  if (n != NULL) {

    while (n->m_nextpkt != NULL)
      n = n->m_nextpkt;

    n->m_nextpkt = m;

  } /* End if local pointer non-null */

  /* Else local pointer null */
  else {

    sb->sb_mb = m;

  } /* End else local pointer null */

  return 1;
}

