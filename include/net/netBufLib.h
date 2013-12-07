/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2009 - 2010 Surplus Users Ham Society
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

/* netBufLib.h - Network buffer library header */

#ifndef _netBufLib_h
#define _netBufLib_h

#include <vmx.h>
#include <os/ffsLib.h>

#ifndef NUM_NET_MBLKS
#define NUM_NET_MBLKS           400 /* Blocks */
#endif /* NUM_NET_MBLKS */

#define MAX_MBLK_TYPES          256 /* Blocks */

/* Data clusters */

/* 64 byte clusters */
#ifndef NUM_64
#define NUM_64                  100 /* Blocks */
#endif /* NUM_64 */

/* 128 byte clusters */
#ifndef NUM_128
#define NUM_128                 100 /* Blocks */
#endif /* NUM_128 */

/* 256 byte clusters */
#ifndef NUM_256
#define NUM_256                 40 /* Blocks */
#endif /* NUM_256 */

/* 512 byte clusters */
#ifndef NUM_512
#define NUM_512                 40 /* Blocks */
#endif /* NUM_512 */

/* 1024 byte clusters */
#ifndef NUM_1024
#define NUM_1024                25 /* Blocks */
#endif /* NUM_1024 */

/* 2048 byte clusters */
#ifndef NUM_2048
#define NUM_2048                25 /* Blocks */
#endif /* NUM_1024 */

/* 4096 byte clusters */
#ifndef NUM_4096
#define NUM_4096                0 /* Blocks */
#endif /* NUM_4096 */

/* 8192 byte clusters */
#ifndef NUM_8192
#define NUM_8192                0 /* Blocks */
#endif /* NUM_8192 */

/* 16384 byte clusters */
#ifndef NUM_16384
#define NUM_16384               0 /* Blocks */
#endif /* NUM_16384 */

/* 32768 byte clusters */
#ifndef NUM_32768
#define NUM_32768               0 /* Blocks */
#endif /* NUM_32768 */

/* 65536 byte clusters */
#ifndef NUM_65536
#define NUM_65536               0 /* Blocks */
#endif /* NUM_65536 */

/* Total number of cluster blocks */
#ifndef NUM_CL_BLKS
#define NUM_CL_BLKS             ( NUM_64 + NUM_128 + NUM_256 + NUM_512 +       \
                                  NUM_1024 + NUM_2048 + NUM_4096 + NUM_8192 +  \
                                  NUM_16384 + NUM_32768 + NUM_65536 )
#endif /* NUM_CL_BLKS */

/* System data clusters */

/* 64 byte clusters */
#ifndef NUM_SYS_64
#define NUM_SYS_64              64 /* Blocks */
#endif /* NUM_SYS_64 */

/* 128 byte clusters */
#ifndef NUM_SYS_128
#define NUM_SYS_128             64 /* Blocks */
#endif /* NUM_128 */

/* 256 byte clusters */
#ifndef NUM_SYS_256
#define NUM_SYS_256             64 /* Blocks */
#endif /* NUM_256 */

/* 512 byte clusters */
#ifndef NUM_SYS_512
#define NUM_SYS_512             64 /* Blocks */
#endif /* NUM_512 */

#ifndef NUM_SYS_CL_BLKS
#define NUM_SYS_CL_BLKS         ( NUM_64 + NUM_128 + NUM_256 + NUM_512 )
#endif /* NUM_SYS_CL_BLKS */

#ifndef NUM_SYS_MBLKS
#define NUM_SYS_MBLKS           ( 2 * NUM_SYS_CL_BLKS )
#endif /* NUM_SYS_MBLKS */

/* Data clusters minimal configuration */

#define NUM_NET_MBLKS_MIN       200
#define NUM_64_MIN              50
#define NUM_128_MIN             50
#define NUM_256_MIN             10
#define NUM_512_MIN             10
#define NUM_1024_MIN            20
#define NUM_CL_BLKS_MIN         ( NUM_64_MIN + NUM_128_MIN + NUM_256_MIN +     \
                                  NUM_512_MIN + NUM_1024_MIN )

#define NUM_SYS_64_MIN          20
#define NUM_SYS_128_MIN         15
#define NUM_SYS_256_MIN         15
#define NUM_SYS_512_MIN         10
#define NUM_SYS_CL_BLKS_MIN     ( NUM_SYS_64_MIN + NUM_SYS_128_MIN +           \
                                  NUM_SYS_256_MIN + NUM_SYS_512_MIN )
#define NUM_SYS_MBLKS_MIN       NUM_SYS_CL_BLKS_MIN

/* Cluster constants */

#define CL_SIZE_64              64
#define CL_SIZE_128             128
#define CL_SIZE_256             256
#define CL_SIZE_512             512
#define CL_SIZE_1024            1024
#define CL_SIZE_2048            2048
#define CL_SIZE_4096            4096
#define CL_SIZE_8192            8192
#define CL_SIZE_16384           16386
#define CL_SIZE_32768           32768
#define CL_SIZE_65536           65536

#define CL_LOG2_64              6
#define CL_LOG2_128             7
#define CL_LOG2_256             8
#define CL_LOG2_512             9
#define CL_LOG2_1024            10
#define CL_LOG2_2048            11
#define CL_LOG2_4096            12
#define CL_LOG2_8192            13
#define CL_LOG2_16384           14
#define CL_LOG2_32768           15
#define CL_LOG2_65536           16

#define CL_LOG2_MIN             CL_LOG2_64
#define CL_LOG2_MAX             CL_LOG2_65536
#define CL_SIZE_MIN             ( 1 << CL_LOG2_MIN )
#define CL_SIZE_MAX             ( 1 << CL_LOG2_MAX )
#define CL_INDX_MIN             0
#define CL_INDX_MAX             ( CL_LOG2_MAX - CL_LOG2_MIN )
#define CL_TABLE_SIZE           ( CL_INDX_MAX + 1 )

#define CL_LOG2_TO_CL_INDEX(x)  ( (x) - CL_LOG2_MIN )
#define CL_LOG2_TO_CL_SIZE(x)   ( 1 << (x) )
#define SIZE_TO_LOG2(sz)        ( ffsMsb( (sz) ) - 1 )
#define CL_SIZE_TO_CL_INDEX(sz) ( ffsMsb( (sz) ) - (1 + CL_LOG2_MIN) )

/* Types */
#define MT_FREE                 0                       /* In free list */
#define MT_DATA                 1                       /* Dynamic alloc */
#define MT_HEADER               2                       /* Packet header */
#define MT_SOCKET               3                       /* Socket struct */
#define MT_PCB                  4                       /* Protocol control */
#define MT_RTABLE               5                       /* Routing table */
#define MT_HTABLE               6                       /* Host table */
#define MT_ATABLE               7                       /* Address resolv tbl */
#define MT_SONAME               8                       /* Socket name */
#define MT_ZOMBIE               9                       /* Zombie proc */
#define MT_SOOPTS               10                      /* Socke options */
#define MT_FTABLE               11                      /* Frament table */
#define MT_RIGHTS               12                      /* Access rights */
#define MT_IFADDR               13                      /* Interface address */
#define MT_CONTROL              14                      /* Control block */
#define MT_OOBDATA              15                      /* Exp data */
#define MT_IPMOPTS              16                      /* Multicast options */
#define MT_IPMADDR              17                      /* Multicast address */
#define MT_IFMADDR              18                      /* Link muticalst adr */
#define MT_MRTABLE              19                      /* Muticast routing */

/* Block types */

#define NUM_MBLK_TYPES          20

#define M_EXT                   0x01                    /* Have cluster */
#define M_PKTHDR                0x02                    /* Record start */
#define M_EOR                   0x04                    /* Record end */
#define M_FORWARD               0x08                    /* Forward packet */

#define M_BCAST                 0x10                    /* Broadcast */
#define M_MCAST                 0x20                    /* Multicast */
#define M_PROXY                 0x40                    /* Proxy */
#define M_SECURE_PKT            0x80                    /* Security checked */

#define M_DONTWAIT              1
#define M_WAIT                  0

#define M_COPYALL               1000000000

#define M_EOB                   M_EOR

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

/******************************************************************************
* M_HASCL - Check if block is associated with cluster
*
* RETURNS: TRUE or FALSE
******************************************************************************/

#define M_HASCL(pMblk)          ( (pMblk)->mBlkHdr.mFlags & M_EXT )

/******************************************************************************
* M_BLK_SZ - Get size of mBlk structure
*
* RETURNS: Structure size in bytes
******************************************************************************/

#define M_BLK_SZ                sizeof (struct mBlk)
#define MSIZE                   M_BLK_SZ

/******************************************************************************
* CL_BLK_SZ - Get size of clBlk structure
*
* RETURNS: Structure size in bytes
******************************************************************************/

#define CL_BLK_SZ               sizeof (struct clBlk)

/******************************************************************************
* CL_BUF_TO_CL_POOL - Convert CL_BUF to CL_POOL
*
* RETURNS: CL_POOL_ID
******************************************************************************/

#define CL_BUF_TO_CL_POOL(pClBuf) (*(CL_POOL_ID *) ((int)pClBuf - sizeof(int)))

/******************************************************************************
* MBLK_TO_NET_POOL - Convert M_BLK to NET_POOL
*
* RETURNS: NET_POOL_ID
******************************************************************************/

#define MBLK_TO_NET_POOL(pMblk) (*(NET_POOL_ID *) ((int)pMblk - sizeof(int)))

/******************************************************************************
* CL_BLK_TO_M_LINK - Convert cluster to to mlink
*
* RETURNS: M_LINK_ID
******************************************************************************/

#define CL_BLK_TO_M_LINK(pClBlk)                                              \
        (M_LINK_ID) ((char *) (pClBlk) - OFFSET(M_LINK, clBlk))

/******************************************************************************
* POOL_INIT - Call poolInitFunc from POOL_FUNC table
*
* RETURNS: OK or ERROR
******************************************************************************/

#define POOL_INIT(poolId, pMclBlkConfig, pClDescTable,                        \
                 clDescTableNumEntries)                                       \
        ( *(((NET_POOL_ID) (poolId))->pFuncTable->poolInitFunc) )             \
        ( (poolId), (pMclBlkConfig), (pClDescTable),                          \
          (clDescTableNumEntries) )

/******************************************************************************
* M_BLK_FREE - Call mBlkFreeFunc from POOL_FUNC table
*
* RETURNS: N/A
******************************************************************************/

#define M_BLK_FREE(poolId, mBlkId)                                            \
        ( *(((NET_POOL_ID) (poolId))->pFuncTable->mBlkFreeFunc) )             \
        ( (poolId), (mBlkId) )

/******************************************************************************
* CL_BLK_FREE - Call clBlkFreeFunc from POOL_FUNC table
*
* RETURNS: N/A
******************************************************************************/

#define CL_BLK_FREE(poolId, clBlkId)                                          \
        ( *(((NET_POOL_ID) (poolId))->pFuncTable->clBlkFreeFunc) )            \
        ( (clBlkId) )

/******************************************************************************
* CL_FREE - Call clFreeFunc from POOL_FUNC table
*
* RETURNS: N/A
******************************************************************************/

#define CL_FREE(poolId, pClBuf)                                               \
        ( *(((NET_POOL_ID) (poolId))->pFuncTable->clFreeFunc) )               \
        ( (poolId), (pClBuf) )

/******************************************************************************
* M_BLK_CL_FREE - Call mBlkClFreeFunc from POOL_FUNC table
*
* RETURNS: M_BLK_ID
******************************************************************************/

#define M_BLK_CL_FREE(poolId, mBlkId)                                         \
        (M_BLK_ID) ( *(((NET_POOL_ID) (poolId))->pFuncTable->mBlkClFreeFunc) )\
        ( (poolId), (mBlkId) )

/******************************************************************************
* M_BLK_GET - Call mBlkGetFunc from POOL_FUNC table
*
* RETURNS: M_BLK_ID
******************************************************************************/

#define M_BLK_GET(poolId, canWait, type)                                      \
        (M_BLK_ID) ( *(((NET_POOL_ID) (poolId))->pFuncTable->mBlkGetFunc) )   \
        ( (poolId), (canWait), (type) )

/******************************************************************************
* CL_BLK_GET - Call clBlkGetFunc from POOL_FUNC table
*
* RETURNS: CL_BLK_ID
******************************************************************************/

#define CL_BLK_GET(poolId, canWait)                                           \
        (CL_BLK_ID) ( *(((NET_POOL_ID) (poolId))->pFuncTable->clBlkGetFunc) ) \
        ( (poolId), (canWait) )

/******************************************************************************
* CLUSTER_GET - Call clusterGetFunc from POOL_FUNC table
*
* RETURNS: char*
******************************************************************************/

#define CLUSTER_GET(poolId, clPoolId)                                         \
        (char *) ( *(((NET_POOL_ID) (poolId))->pFuncTable->clusterGetFunc) )  \
        ( (poolId), (clPoolId) )

/******************************************************************************
* M_CL_GET - Call mBlkClGetFunc from POOL_FUNC table
*
* RETURNS: OK or ERROR
******************************************************************************/

#define M_CL_GET(poolId, mBlkId, size, canWait, bestFit)                      \
        ( *(((NET_POOL_ID) (poolId))->pFuncTable->mBlkClGetFunc) )            \
        ( (poolId), (mBlkId), (size), (canWait), (bestFit) )

/******************************************************************************
* CL_POOL_ID_GET - Call clPoolIdGetFunc from POOL_FUNC table
*
* RETURNS: CL_POOL_ID
******************************************************************************/

#define CL_POOL_ID_GET(poolId, size, bestFit)                                 \
        (CL_POOL_ID) (*(((NET_POOL_ID)(poolId))->pFuncTable->clPoolIdGetFunc))\
        ( (poolId), (size), (bestFit) )

/* Structs */
struct ifnet;

/* Cluster configuration structure */

typedef struct {
  int                   mBlkNum;                /* Number of B_BLK blocks */
  int                   clBlkNum;               /* Number of cluster blocks */
  char                  *memArea;               /* Memory area */
  int                   memSize;                /* Memory size */
} M_CL_CONFIG;

/* Cluster descriptor */

typedef struct clDesc {
  int                   clSize;                 /* Cluster size */
  int                   clNum;                  /* Number of clusters */
  char                  *memArea;               /* Memory area */
  int                   memSize;
} CL_DESC;

/* Cluster head */

typedef struct clBuff {
  struct clBuff         *pClNext;               /* Pointer to next in list */
} CL_BUF;
typedef CL_BUF          *CL_BUF_ID;             /* Id is as pointer */

/* Cluster pool descriptor */

typedef struct clPool {
  int                   clSize;                 /* Cluster size */
  int                   clLg2;                  /* Cluster size log2 */
  int                   clNum;                  /* Number of clusters */
  int                   clNumFree;              /* Number of free clusters */
  int                   clUsage;                /* Number of times used */
  CL_BUF_ID             pClHead;                /* Pointer to cluster head */
  struct netPool        *pNetPool;              /* Pointer to net pool */
} CL_POOL;
typedef CL_POOL         *CL_POOL_ID;            /* Id is a pointer */

/* Header at the beginnig of each block */

typedef struct mHdr {
  struct mBlk           *mNext;                 /* Next buffer in chain */
  struct mBlk           *mNextPkt;              /* Next chain in record */
  char                  *mData;                 /* Data pointer */
  int                   mLen;                   /* Data size */
  unsigned char         mType;                  /* Data type */
  unsigned short        mFlags;                 /* Flags */
  unsigned short        reserved;               /* reserved/pad */
} M_BLK_HDR;

/* Record header */

typedef struct pktHdr {
  struct ifnet          *rcvif;                 /* Receiver interface */
  int                   len;                    /* Packetlength */
} M_PKT_HDR;

/* Cluster block */

typedef union clBlkNext {
  struct clBlk          *pClBlkNext;            /* Pointer to next block */
  char                  *pClBuf;                /* Pointer to next buffer */
} CL_BLK_LIST;

typedef struct clBlk {
  CL_BLK_LIST           clNode;                 /* Se above */
  unsigned int          clSize;                 /* Cluser size */
  int                   clRefCount;             /* Referece count */
  FUNCPTR               clFreeFunc;             /* Pointer to free function */
  int                   clFreeArg1;             /* Free function argument 1 */
  int                   clFreeArg2;             /* Free function argument 2 */
  int                   clFreeArg3;             /* Free function argument 3 */
  struct netPool        *pNetPool;              /* Pointer to net pool */
} CL_BLK;

/* M_BLK block structure */

typedef struct mBlk {
  M_BLK_HDR             mBlkHdr;                /* Header, se above */
  M_PKT_HDR             mBlkPktHdr;             /* Packe header, se above */
  CL_BLK                *pClBlk;                /* Pointer to cluster block */
} M_BLK;

/* M_BLK statistics */

typedef struct mbstat {
  unsigned long         mNum;                   /* Blocks obtained from pool */
  unsigned long         mDrops;                 /* Number of failures */
  unsigned long         mWait;                  /* Number of waits for space */
  unsigned long         mDrain;                 /* Number of drained */
  unsigned long         mTypes[256];            /* Types for mBlks */
} M_STAT;

typedef struct _M_LINK {
  M_BLK                 mBlk;
  CL_BLK                clBlk;
} M_LINK;

/* Net pool functions */

/* Workaround - Must know type of *_ID in functions */
typedef M_BLK           *M_BLK_ID;              /* Id is a pointer */
typedef CL_BLK          *CL_BLK_ID;             /* Id is a pointer */
typedef M_LINK          *M_LINK_ID;             /* Id is a pointer */
typedef struct poolFunc POOL_FUNC;
typedef struct netPool  NET_POOL;
typedef NET_POOL        *NET_POOL_ID;           /* Id is a pointer */

struct poolFunc {
  FUNCPTR       poolInitFunc;
  FUNCPTR       mBlkFreeFunc;
  FUNCPTR       clBlkFreeFunc;
  FUNCPTR       clFreeFunc;
  FUNCPTR       mBlkClFreeFunc;
  FUNCPTR       mBlkGetFunc;
  FUNCPTR       clBlkGetFunc;
  FUNCPTR       clusterGetFunc;
  FUNCPTR       mBlkClGetFunc;
  FUNCPTR       clPoolIdGetFunc;
};

/* Net pool structure */

struct netPool {
  M_BLK_ID              pmBlkHead;              /* Head M_BLK */
  CL_BLK_ID             pClBlkHead;             /* Head cluster blocks */
  int                   mBlkCount;              /* Number of blocks */
  int                   mBlkFree;               /* Number of free blocks */
  int                   clMask;                 /* Cluster mask */
  int                   clLg2Min;               /* Cluster min size log2 */
  int                   clSizeMin;              /* Cluster min size */
  int                   clLg2Max;               /* Cluster max size log2 */
  int                   clSizeMax;              /* Cluster max size */
  CL_POOL               *clTable[CL_TABLE_SIZE];/* Pool table */
  M_STAT                *pPoolStat;             /* Pointer to pool stat */
  POOL_FUNC             *pFuncTable;            /* Pointer to fuction table */
};

IMPORT void netBufLibInit(void);
IMPORT STATUS netPoolInit(NET_POOL_ID poolId, M_CL_CONFIG *pMclBlkConfig,
                          CL_DESC *pClDescTable, int clDescTableNumEntries,
                          POOL_FUNC *pFuncTable);
IMPORT M_BLK_ID netMblkGet(NET_POOL_ID poolId, int canWait, unsigned char type);
IMPORT void netMblkFree(NET_POOL_ID poolId, M_BLK_ID mBlkId);
IMPORT CL_BLK_ID netClBlkGet(NET_POOL_ID poolId, int canWait);
IMPORT void netClBlkFree(NET_POOL_ID poolId, CL_BLK_ID clBlkId);
IMPORT char* netClusterGet(NET_POOL_ID poolId, CL_POOL_ID clPoolId);
IMPORT void netClFree(NET_POOL_ID poolId, char *pClBuf);
IMPORT STATUS netMblkClGet(NET_POOL_ID poolId, M_BLK_ID mBlkId, int size,
                         int canWait, BOOL bestFit);
IMPORT M_BLK_ID netMblkClFree(M_BLK_ID mBlkId);
IMPORT void netMblkClChainFree(M_BLK_ID mBlkId);
IMPORT CL_POOL_ID netClPoolIdGet(NET_POOL_ID poolId, int size, BOOL bestFit);
IMPORT M_BLK_ID netTupleGet(NET_POOL_ID poolId, int size, int canWait,
                            unsigned char type, BOOL bestFit);
IMPORT STATUS netPoolDelete(NET_POOL_ID poolId);
IMPORT M_BLK_ID netMblkChainDup(NET_POOL_ID poolId, M_BLK_ID blkId, int offset,
                                int len, int canWait);
IMPORT void netBufCollectFuncSet(FUNCPTR func);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _netBufLib_h */

