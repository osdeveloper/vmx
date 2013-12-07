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

/* mbuf.h - Network buffer library header */

#ifndef _mbuf_h
#define _mbuf_h

#include <vmx.h>
#include <net/netBufLib.h>

/* Cluster size related */
#define MCLBYTES                        2048
#define USE_CLUSTER(size)               ( (size) >= 512 ? TRUE : FALSE )

/* Constants */
#define M_MAXCOMPRESS                   (CL_SIZE_128 / 2)
#define mbuf                            mBlk
#define m_next                          mBlkHdr.mNext
#define m_len                           mBlkHdr.mLen
#define m_data                          mBlkHdr.mData
#define m_type                          mBlkHdr.mType
#define m_flags                         mBlkHdr.mFlags
#define m_nextpkt                       mBlkHdr.mNextPkt
#define m_act                           m_nextpkt
#define m_pkthdr                        mBlkPktHdr
#define m_ext                           pClBlk
#define m_extBuf                        m_ext->clNode.pClBuf
#define m_extFreeFunc                   m_ext->clFreeFunc
#define m_extSize                       m_ext->clSize
#define m_extRefCount                   m_ext->clRefCount
#define m_extArg1                       m_ext->clFreeArg1
#define m_extArg2                       m_ext->clFreeArg2
#define m_extArg3                       m_ext->clFreeArg3
#define m_mbufs                         mNum
#define m_drops                         mDrops
#define m_wait                          mWait
#define m_drain                         mDrain
#define m_mtypes                        mTypes

#define M_COPYFLAGS                     (M_PKTHDR | M_EOR | M_BCAST | M_MCAST)

#define MPULL_EXTRA                     32

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Type conversions */
#define MC_NO_CLUSTER                   ( (unsigned char) 0 )
#define MC_CLUSTER                      ( (unsigned char) 1 )
#define MC_LANCE                        ( (unsigned char) 2 )
#define MC_BACKPLANE                    ( (unsigned char) 3 )
#define MC_EI                           ( (unsigned char) 4 )
#define MC_UCLUSTER                     ( (unsigned char) 0x80 )

/* Macros */

/******************************************************************************
* mtod - Convert mbuf pointer to data pointer
*
* RETURNS: N/A
******************************************************************************/

#define mtod(m, t)                      ( (t)( (m)->m_data ) )

/******************************************************************************
* M_ALIGN - Set m_data to point to new mbuf
*
* RETURNS: N/A
******************************************************************************/

#define M_ALIGN(m, len)                                                       \
{                                                                             \
  (m)->m_data += ( m->m_extSize - (len) ) &~ ( sizeof(long) - 1 );            \
}

#define MH_ALIGN(m, len)                M_ALIGN(m, len)

/******************************************************************************
* M_LEADINGSPACE - Get ammount of space before start of data in cluster
*
* RETURNS: Number of bytes
******************************************************************************/

#define M_LEADINGSPACE(m)               ( (m)->m_data - (m)->m_extBuf )

/******************************************************************************
* M_TRAILINGSPACE - Get ammount of space after end of data in cluster
*
* RETURNS: Number of bytes
******************************************************************************/

#define M_TRAILINGSPACE(m)              ( (m)->m_extBuf + (m)->m_extSize -    \
                                          (m)->m_data - (m)->m_len )

/******************************************************************************
* M_PREPEND - Prepend space of size len to mbuf
*
* RETURNS: N/A
******************************************************************************/

#define M_PREPEND(m, plen, how)                                               \
{                                                                             \
  if ( M_LEADINGSPACE(m) >= (plen) ) {                                        \
    (m)->m_data -= (plen);                                                    \
    (m)->m_len += (plen);                                                     \
  }                                                                           \
  else                                                                        \
    (m) = m_prepend((m), (plen), (how) );                                     \
  if ( ((m) != NULL) && ((m)->m_flags & M_PKTHDR) )                           \
    (m)->m_pkthdr.len += (plen);                                              \
}

/******************************************************************************
* Free - Free mbuf memory
*
* RETURNS: N/A
******************************************************************************/

#define Free(buf)                       mb_free(buf)

/******************************************************************************
* m_free - Free mbuf cluster pair
*
* RETURNS: M_BLK_ID
******************************************************************************/

#define m_free(m)                       netMblkClFree(m)

/******************************************************************************
* m_freem - Free mbuf cluster chain
*
* RETURNS: N/A
******************************************************************************/

#define m_freem(m)                      netMblkClChainFree(m)

/******************************************************************************
* m_copy - Copy mbufs without wait
*
* RETURNS: M_BLK_ID
******************************************************************************/

#define m_copy(m, o, l)                 m_copym((m), (o), (l), M_DONTWAIT)

/******************************************************************************
* m_copym - Copy mbufs
*
* RETURNS: M_BLK_ID
******************************************************************************/

#define m_copym(m, o, l, w)                                                   \
  netMblkChainDup(netSysDpoolId, (m), (o), (l), (w));

/* Globlas */
IMPORT NET_POOL_ID netSysDpoolId;
IMPORT NET_POOL_ID netSysPoolId;

/* Functions */
IMPORT STATUS mbufLibInit(void);
IMPORT void* mb_alloc(size_t size, unsigned char type, int canWait);
IMPORT void mb_free(void *buf);
IMPORT struct mbuf* m_get(int canWait, unsigned char type,
                          int bufsize, BOOL bestFit);
IMPORT struct mbuf* m_gethdr(int canWait, unsigned char type,
                             int bufsize, BOOL bestFit);
IMPORT struct mbuf* m_prepend(struct mbuf *m, int len, int how);
IMPORT void m_cat(struct mbuf *m, struct mbuf *n);
IMPORT void m_adj(struct mbuf *mp, int req_len);
IMPORT struct mbuf* m_pullup(struct mbuf *n, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _mbuf_h */

