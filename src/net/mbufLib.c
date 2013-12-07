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

/* mbufLib.c - Mbuf primitives */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/netBufLib.h>
#include <net/netLib.h>
#include <net/mbuf.h>

/* Defines */

/* Imports */
IMPORT struct domain *domains;
IMPORT int max_protohdr;

IMPORT M_CL_CONFIG mClBlkConfig;
IMPORT CL_DESC clDescTable[];
IMPORT int clDescTableNumEntries;

IMPORT M_CL_CONFIG sysMclBlkConfig;
IMPORT CL_DESC sysClDescTable[];
IMPORT int sysClDescTableNumEntries;

/* Locals */
LOCAL NET_POOL netSysDpool;
LOCAL NET_POOL netSysPool;

LOCAL void mbufCollect(M_STAT *poolStatId);

/* Globals */
NET_POOL_ID netSysDpoolId = &netSysDpool;
NET_POOL_ID netSysPoolId = &netSysPool;

/* Functions */

/*******************************************************************************
 * mbufLibInit - Initailize mbuf library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS mbufLibInit(void)
{
  /* Set collect function */
  netBufCollectFuncSet( (FUNCPTR) mbufCollect );

  /* Initialize network buffers */
  if ( netPoolInit(&netSysDpool, &mClBlkConfig, clDescTable,
		clDescTableNumEntries, NULL) != OK)
    return ERROR;

  if ( netPoolInit(&netSysPool, &sysMclBlkConfig, sysClDescTable,
		sysClDescTableNumEntries, NULL) != OK)
    return ERROR;

  return OK;
}

/*******************************************************************************
 * m_alloc - Allocate memory from network buffers
 *
 * RETURNS: Pointer to memory of NULL
 ******************************************************************************/

void* mb_alloc(size_t size, unsigned char type, int canWait)
{
  struct mbuf *pMblk;
  struct mbuf **ppMblk;

  /* Get network tuple */
  pMblk = netTupleGet(netSysPoolId, (size + sizeof(struct mbuf *)),
		      canWait, type, TRUE);
  if (pMblk == NULL)
    return NULL;

  /* Find address after mbuf struct */
  ppMblk = mtod(pMblk, struct mbuf **);
  *ppMblk = pMblk;
  pMblk->m_data += sizeof(struct mbuf **);

  /* Return address to data */
  return ( mtod(pMblk, char*) );
}

/*******************************************************************************
 * mb_free - Free  memory allocated from network buffers
 *
 * RETURNS: N/A
 ******************************************************************************/

void mb_free(void *buf)
{
  if (buf != NULL)
    m_free( *((struct mbuf **) ( buf - sizeof(struct mbuf **) )) );
}

/*******************************************************************************
 * m_get - Get a new memory block with a cluster
 *
 * RETURNS: Memory block or NULL
 ******************************************************************************/

struct mbuf* m_get(int canWait, unsigned char type,
		   int bufsize, BOOL bestFit)
{
  M_BLK_ID blkId;

  /* Allocate from data pool */
  blkId = M_BLK_GET(netSysDpoolId, canWait, type);
  if (blkId == NULL)
    return NULL;

  /* Allocate cluster for it */
  if ( M_CL_GET(netSysDpoolId, blkId, bufsize, canWait, bestFit) != OK ) {

    m_free(blkId);
    return NULL;

  }

  return blkId;
}

/*******************************************************************************
 * m_gethdr - Get a new header with a cluster
 *
 * RETURNS: Memory block or NULL
 ******************************************************************************/

struct mbuf* m_gethdr(int canWait, unsigned char type,
		      int bufsize, BOOL bestFit)
{
  M_BLK_ID blkId;

  /* Allocate from data pool */
  blkId = M_BLK_GET(netSysDpoolId, canWait, type);
  if (blkId == NULL)
    return NULL;

  /* Mark as header */
  blkId->m_flags |= M_PKTHDR;

  /* Allocate cluster for it */
  if ( M_CL_GET(netSysDpoolId, blkId, bufsize, canWait, bestFit) != OK ) {

    m_free(blkId);
    return NULL;

  }

  return blkId;
}

/*******************************************************************************
 * m_prepend - Prepend block
 *
 * RETURNS: Memory block or NULL
 ******************************************************************************/

struct mbuf* m_prepend(struct mbuf *m, int len, int how)
{
  struct mbuf *mn;

  /* Get cluster block */
  mn = m_get(how, m->m_type, len, TRUE);
  if (mn == NULL) {

    m_freem(m);
    return NULL;

  }

  /* If header */
  if (m->m_flags & M_PKTHDR) {

    mn->m_pkthdr = m->m_pkthdr;
    mn->m_flags = m->m_flags;
    m->m_flags &= ~M_PKTHDR;

  } /* End if header */

  /* Set next */
  mn->m_next = m;
  m = mn;

  /* Align if needed */
  if (len < m->m_extSize)
    MH_ALIGN(m, len);

  m->m_len = len;

  return m;
}

/*******************************************************************************
 * m_cat - Concatenate blocks
 *
 * RETURNS: N/A
 ******************************************************************************/

void m_cat(struct mbuf *m, struct mbuf *n)
{
  /* Goto last block */
  while (m->m_next != NULL)
    m = m->m_next;

  /* Paste in new block */
  if (n != NULL)
    m->m_next = n;
}

/*******************************************************************************
 * m_adj - Adjust data in mbuf
 *
 * RETURNS: N/A
 ******************************************************************************/

void m_adj(struct mbuf *mp, int req_len)
{
  int i, len;
  struct mbuf *m;

  /* Settup locals */
  len = req_len;
  m = mp;

  if (mp == NULL)
    return;

  /* If length ge. zero */
  if (len >= 0) {

    /* While m non-null and len gt. zero */
    while ( (m != NULL) && (len > 0) ) {

      /* If m_len le. length */
      if (m->m_len <= len) {

        len -= m->m_len;
        m->m_len = 0;
        m = m->m_next;

      } /* End if m_len le. length */

      /* Else m_len gt. length */
      else {

        m->m_len -= len;
        m->m_data += len;
        len = 0;

      } /* End else m_len gt. length */

    } /* End while m non-null and len gt. zero */

    /* Update local pointer */
    m = mp;

    /* If paket header */
    if (mp->m_flags & M_PKTHDR)
      m->m_pkthdr.len -= (req_len - len);

  } /* End if length ge. zero */

  /* Else length lt. zero */
  else {

    len = -len;
    i = 0;

    /* Endless loop */
    for (;;) {

      /* Update count */
      i += m->m_len;

      /* Break when next is null */
      if (m->m_next == NULL)
        break;

      /* Advance */
      m = m->m_next;

    } /* End endless loop */

    /* If packet length ge. length */
    if (m->m_len >= len) {

      /* Update packet lenght */
      m->m_len -= len;

      /* If header */
      if (mp->m_flags & M_PKTHDR)
        mp->m_pkthdr.len -= len;

      /* Done */
      return;

    } /* End if packet length ge. length */

    /* Adjust count */
    i -= len;
    if (i < 0)
      i = 0;

    /* Update local pointer */
    m = mp;

    /* If header */
    if (m->m_flags & M_PKTHDR)
      m->m_pkthdr.len = i;

    /* For all packets */
    for (; m != NULL; m = m->m_next) {

      /* If packet length ge. count */
      if (m->m_len >= i) {

        m->m_len = i;

        /* Break out of loop */
        break;

      } /* End if packet length ge. count */

      /* Advance */
      i -= m->m_len;

    } /* End for all packets */

    /* While packets left */
    while ( (m = m->m_next) )
      m->m_len = 0;

  } /* End else length lt. zero */
}

/*******************************************************************************
 * m_pullup - Make area continious at beginning of chain
 *
 * RETURNS: Memory block or NULL
 ******************************************************************************/

struct mbuf* m_pullup(struct mbuf *n, int len)
{
  struct mbuf *m;
  int i, space;

  if ( ((n->m_data + len) < (n->m_extBuf + n->m_extSize)) &&
       (n->m_next != NULL) ) {

    if (n->m_len >= len)
      return n;

    m = n;
    n = n->m_next;
    len -= m->m_len;

  }

  else {

    m = m_get(M_DONTWAIT, n->m_type, len, TRUE);
    if (m == NULL)
      goto bad;

    m->m_len = 0;
    if (n->m_flags & M_PKTHDR) {

      m->m_pkthdr = n->m_pkthdr;
      m->m_flags = n->m_flags;
      n->m_flags &= ~M_PKTHDR;

    }

  }

  space = (m->m_extBuf + m->m_extSize) - (m->m_data + m->m_len);

  do {

    i = min( min( max(len, max_protohdr), space), n->m_len);
    memcpy( mtod(m, char *) + m->m_len, mtod(n, char *), (unsigned) i );

    /* Advance */
    len -= i;
    m->m_len += i;
    n->m_len -= i;
    space -= i;

    if (n->m_len)
      n->m_data += i;
    else
      n = m_free(n);

  } while ( (len > 0) && (n != NULL) );

  if (len > 0) {

    m_free(m);
    goto bad;

  }

  m->m_next = n;

  return m;

bad:

  m_freem(n);

  return NULL;
}

/*******************************************************************************
 * mbufCollect - Network buffer collect function
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void mbufCollect(M_STAT *poolStatId)
{
  struct domain *dp;
  struct protosw *pr;
  int level;

  /* For all domains */
  for (dp = domains; dp != NULL; dp->dom_next) {

    /* For all protocols */
    for (dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      /* Call drain function if set */
      if (pr->pr_drain != NULL)
        ( *pr->pr_drain) ();

    } /* End for all protocols */

  } /* End for all domains */

  /* Lock interrupts */
  INT_LOCK(level);

  /* Increase drain counter */
  poolStatId->mDrain++;

  INT_UNLOCK(level);
}

