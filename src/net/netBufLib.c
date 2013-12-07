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

/* netBufLib.c - Net buffer library */

#include <vmx.h>
#include <arch/intArchLib.h>
#include <net/netBufLib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Locals */
LOCAL STATUS _poolInit(NET_POOL_ID pNetPool,
		       M_CL_CONFIG *pMclBlkConfig,
		       CL_DESC *pClDescTable,
		       int clDescTableNumEntries);
LOCAL STATUS _memPoolInit(int num, int unitSize, int headerSize, char *memArea);
LOCAL void _mBlkFree(NET_POOL_ID pNetPool, M_BLK_ID pMblk);
LOCAL M_BLK_ID _mBlkCarve(NET_POOL_ID pNetPool, int num, char *pool);
LOCAL void _clBlkFree(CL_BLK_ID pClBlk);
LOCAL CL_BLK_ID _clBlkCarve(int num, char *pool);
LOCAL void _clFree(NET_POOL_ID pNetPool, char *pClBuf);
LOCAL CL_BUF_ID _clPoolCarve(CL_POOL_ID pClPool, int num, int size, char *pool);
LOCAL M_BLK_ID _mBlkClFree(NET_POOL_ID pNetPool, M_BLK_ID pMblk);
LOCAL M_BLK_ID _mBlkGet(NET_POOL_ID pNetPool, int canWait, unsigned char type);
LOCAL CL_BLK_ID _clBlkGet(NET_POOL_ID pNetPool, int canWait);
LOCAL char* _clusterGet(NET_POOL_ID pNetPool, CL_POOL_ID pClPool);
LOCAL CL_POOL_ID _clPoolIdGet(NET_POOL_ID pNetPool, int size, BOOL bestFit);
LOCAL STATUS _mClGet(NET_POOL_ID pNetPool, M_BLK_ID pMblk, int size,
	      int canWait, BOOL bestFit);

/* Default pool function table */
LOCAL POOL_FUNC defaultFuncTable = {
  (FUNCPTR) _poolInit,
  (FUNCPTR) _mBlkFree,
  (FUNCPTR) _clBlkFree,
  (FUNCPTR) _clFree,
  (FUNCPTR) _mBlkClFree,
  (FUNCPTR) _mBlkGet,
  (FUNCPTR) _clBlkGet,
  (FUNCPTR) _clusterGet,
  (FUNCPTR) _mClGet,
  (FUNCPTR) _clPoolIdGet
};

/* Globals */
FUNCPTR _pNetBufCollect = NULL;
POOL_FUNC *_pNetPoolFuncTable = &defaultFuncTable;

/******************************************************************************
* _poolInit - Initialize pool
*
* RETURNS: OK or ERROR
******************************************************************************/

LOCAL STATUS _poolInit(NET_POOL_ID pNetPool,
		       M_CL_CONFIG *pMclBlkConfig,
		       CL_DESC *pClDescTable,
		       int clDescTableNumEntries)
{
  int i, j, numFree;
  int sizeLog2;
  CL_DESC *pClDesc;
  CL_POOL *pClPool;
  char *memArea;

  /* Zero out clTable */
  memset(pNetPool->clTable, 0, sizeof(pNetPool->clTable));

  /* Allocate memory */
  pNetPool->pPoolStat = (M_STAT *) malloc(sizeof(M_STAT));
  if (pNetPool == NULL)
    return ERROR;

  /* Set Block head to NULL */
  pNetPool->pmBlkHead = NULL;

  /* If configuration given */
  if (pMclBlkConfig != NULL) {

    /* Check if resonable */
    if (pMclBlkConfig->memSize <
	( (pMclBlkConfig->mBlkNum  * (M_BLK_SZ + sizeof(long))) +
	  (pMclBlkConfig->clBlkNum *  CL_BLK_SZ)) ) {
      netPoolDelete(pNetPool);
      return ERROR;
    }

    /* Initialize pool */
    if (_memPoolInit(pMclBlkConfig->mBlkNum, M_BLK_SZ,
		     sizeof(void *), pMclBlkConfig->memArea) != OK) {
      netPoolDelete(pNetPool);
      return ERROR;
    }

    /* Carve up pool */
    pNetPool->pmBlkHead = _mBlkCarve(pNetPool,
				     pMclBlkConfig->mBlkNum,
				     pMclBlkConfig->memArea);

    /* Set number of free memory blocks */
    numFree = pMclBlkConfig->mBlkNum;
    pNetPool->mBlkCount = numFree;
    pNetPool->mBlkFree = numFree;
    pNetPool->pPoolStat->mTypes[MT_FREE] = numFree;
    pNetPool->pPoolStat->mNum = numFree;

    /* Get memory area */
    memArea = (char *) ( (int) pMclBlkConfig->memArea +
			 (numFree * (M_BLK_SZ + sizeof(long))) );

    /* If block number geater than zero */
    if (pMclBlkConfig->clBlkNum > 0) {

      /* Initialize memory pool */
      if (_memPoolInit(pMclBlkConfig->clBlkNum, CL_BLK_SZ, 0, memArea) != OK) {
        netPoolDelete(pNetPool);
	return ERROR;
      }

      /* Carve up pool */
      pNetPool->pClBlkHead = _clBlkCarve(pMclBlkConfig->clBlkNum, memArea);
      if (pNetPool->pClBlkHead == NULL) {
        netPoolDelete(pNetPool);
	return ERROR;
      }

    } /* End if block number greater that zero */

  } /* End if config given */

  /* Init clusters */
  pNetPool->clMask = 0;
  pNetPool->clLg2Max = 0;
  pNetPool->clLg2Min = 0;

  /* For all in decriptor table */
  for (i = 0, pClDesc = pClDescTable;
       (i < clDescTableNumEntries) && (pClDesc != NULL) && (pClDesc->clNum > 0);
       i++, pClDesc++) {

    /* Check range */
    if ( (pClDesc->clSize < CL_SIZE_MIN) || (pClDesc->clSize > CL_SIZE_MAX) ) {
      netPoolDelete(pNetPool);
      return ERROR;
    }

    /* Calculate size log2 */
    sizeLog2 = SIZE_TO_LOG2(pClDesc->clSize);

    /* Set mask */
    pNetPool->clMask |= CL_LOG2_TO_CL_SIZE(sizeLog2);

    /* Check if need to calculate log2 values */
    if ( (pNetPool->clLg2Min == 0) && (pNetPool->clLg2Max == 0) ) {
      pNetPool->clLg2Min = sizeLog2;
      pNetPool->clLg2Max = sizeLog2;
    }

    /* Get min and max values */
    pNetPool->clLg2Min = min(sizeLog2, pNetPool->clLg2Min);
    pNetPool->clLg2Max = max(sizeLog2, pNetPool->clLg2Max);
    pNetPool->clSizeMin = CL_LOG2_TO_CL_SIZE(pNetPool->clLg2Min);
    pNetPool->clSizeMax = CL_LOG2_TO_CL_SIZE(pNetPool->clLg2Max);

    /* Allocate memory */
    pClPool = (CL_POOL *) malloc(sizeof(CL_POOL));
    if (pClPool == NULL) {
      netPoolDelete(pNetPool);
      return ERROR;
    }

    /* Store in cluster pool table */
    pNetPool->clTable[CL_LOG2_TO_CL_INDEX(sizeLog2)] = pClPool;

    for (j = (sizeLog2 - 1);
	 ( ( !(pNetPool->clMask & CL_LOG2_TO_CL_SIZE(j)) ) &&
	   (CL_LOG2_TO_CL_INDEX(j) >= CL_INDX_MIN) );
	 j--)
      pNetPool->clTable[CL_LOG2_TO_CL_INDEX(j)] = pClPool;

    /* Initialize more fields in structure */
    pClPool->clSize = pClDesc->clSize;
    pClPool->clLg2 = sizeLog2;

    /* Initalize back pointer */
    pClPool->pNetPool = pNetPool;

    /* Check memory size */
    if (pClDesc->memSize < ( pClDesc->clNum *
			    (pClDesc->clSize + sizeof(int))) ) {
        netPoolDelete(pNetPool);
        return ERROR;
    }

    /* Initialize memory pool */
    if (_memPoolInit(pClDesc->clNum, pClDesc->clSize,
		     sizeof(void *), pClDesc->memArea) != OK) {
        netPoolDelete(pNetPool);
        return ERROR;
    }

    /* Carve up pool */
    pClPool->pClHead = _clPoolCarve(pClPool, pClDesc->clNum,
				    pClDesc->clSize, pClDesc->memArea);
    if (pClPool->pClHead == NULL) {
        netPoolDelete(pNetPool);
        return ERROR;
    }

    /* Initialize all as free */
    pClPool->clNum = pClDesc->clNum;
    pClPool->clNumFree = pClDesc->clNum;

  } /* End for all in descriptor table */

  return OK;
}

/******************************************************************************
* _memPoolInit - Initialize memory pool
*
* RETURNS: OK or ERROR
******************************************************************************/

LOCAL STATUS _memPoolInit(int num, int unitSize, int headerSize, char *memArea)
{
  /* Check unig size alignment */
  if ( ((int) unitSize & ~(sizeof(void *) - 1)) != unitSize )
    return ERROR;

  /* Check if memory has not been allocated */
  if (memArea == NULL)
{
    return ERROR;
}

  /* Check memory area alignment */
  if ( ((int) memArea & ~(sizeof(void *) - 1)) != (int) memArea )
    return ERROR;

  /* Adjust size for NET_POOL_ID */
  unitSize += headerSize;

  /* Zero out memory area */
  memset(memArea, 0, num * unitSize);

  return OK;
}

/******************************************************************************
* _mBlkFree - Free memory block
*
* RETURNS: N/A
******************************************************************************/

LOCAL void _mBlkFree(NET_POOL_ID pNetPool, M_BLK_ID pMblk)
{
  int level;

  /* Set next packet to NULL */
  pMblk->mBlkHdr.mNextPkt = NULL;

  /* Lock interrupts */
  INT_LOCK(level);

  /* Update pool status */
  pNetPool->pPoolStat->mTypes[pMblk->mBlkHdr.mType]--;
  pNetPool->pPoolStat->mTypes[MT_FREE]++;

  /* Mark as free */
  pMblk->mBlkHdr.mType = MT_FREE;
  pMblk->mBlkHdr.mNext = pNetPool->pmBlkHead;

  /* Add to free list */
  pNetPool->pmBlkHead = pMblk;

  INT_UNLOCK(level);
}

/******************************************************************************
* _mBlkCarve - Carve up memory block pool
*
* RETURNS: M_BLK_ID
******************************************************************************/

LOCAL M_BLK_ID _mBlkCarve(NET_POOL_ID pNetPool, int num, char *pool)
{
  M_BLK_ID blkId, *pBlkId;
  int i, size;

  /* Initialize local variables */
  blkId = NULL;
  pBlkId = &blkId;
  size = M_BLK_SZ + sizeof(void *);

  /* For all units */
  for (i = 0; i < num; i++) {

    /* Calculate pointer */
    *pBlkId = (M_BLK_ID) ( pool + sizeof(void *) );

    /* Set in pointer */
    *( (NET_POOL_ID *) (pool) ) = pNetPool;

    /* Set type as MT_FREE */
    (*pBlkId)->mBlkHdr.mType = MT_FREE;

    /* Get next element */
    pBlkId = &( (*pBlkId)->mBlkHdr.mNext );

    /* Advance pool pointer to next address */
    pool += size;

  } /* End for all units */

  return blkId;
}

/******************************************************************************
* _clBlkFree - Free cluster block
*
* RETURNS: N/A
******************************************************************************/

LOCAL void _clBlkFree(CL_BLK_ID pClBlk)
{
  CL_POOL_ID poolId;
  CL_BUF_ID bufId;
  NET_POOL_ID netPoolId;
  int level;

  /* Initialize local variables */
  netPoolId = pClBlk->pNetPool;
  bufId = (CL_BUF_ID) pClBlk->clNode.pClBuf;

  /* Lock interrupts */
  INT_LOCK(level);

  /* Check if buffer is valid */
  if (bufId == NULL) {
    pClBlk->clNode.pClBlkNext = netPoolId->pClBlkHead;
    netPoolId->pClBlkHead = pClBlk;
    INT_UNLOCK(level);
    return;
  }

  /* Check if reference count reaches zero */
  if ( --(pClBlk->clRefCount) == 0 ) {

    /* Call cluster free if it exists */
    if (pClBlk->clFreeFunc != NULL) {
      INT_UNLOCK(level);
      (*pClBlk->clFreeFunc) (pClBlk->clFreeArg1,
			     pClBlk->clFreeArg2,
			     pClBlk->clFreeArg3);
      INT_LOCK(level);
    }

    /* Otherwise retrun cluster to pool */
    else {
      /* Return cluster to pool */
      poolId = CL_BUF_TO_CL_POOL(bufId);
      bufId->pClNext = poolId->pClHead;
      poolId->pClHead = bufId;

      /* Update mask */
      poolId->pNetPool->clMask |= CL_LOG2_TO_CL_SIZE(poolId->clLg2);
      poolId->clNumFree++;
    }

    /* Return */
    pClBlk->clNode.pClBlkNext = netPoolId->pClBlkHead;
    netPoolId->pClBlkHead = pClBlk;
    INT_UNLOCK(level);
    return;

  } /* End if reference count reached zero */

  /* Unlock interrupts */
  INT_UNLOCK(level);
}

/******************************************************************************
* _clBlkCarve - Carve up cluster block
*
* RETURNS: CL_BLK_ID
******************************************************************************/

LOCAL CL_BLK_ID _clBlkCarve(int num, char *pool)
{
  CL_BLK_ID blkId, *pBlkId;
  int i;

  /* Initialize local variables */
  blkId = NULL;
  pBlkId = &blkId;

  /* For all units */
  for (i = 0; i < num; i++) {

    /* Set in pointer */
    *pBlkId = (CL_BLK_ID) (pool);

    /* Get next element */
    pBlkId = &( (*pBlkId)->clNode.pClBlkNext );

    /* Advance pool pointer to next address */
    pool += CL_BLK_SZ;

  } /* End for all units */

  return blkId;
}

/******************************************************************************
* _clFree - Free cluster
*
* RETURNS: N/A
******************************************************************************/

LOCAL void _clFree(NET_POOL_ID pNetPool, char *pClBuf)
{
  CL_POOL_ID poolId;
  int level;

  /* Initialize local variables */
  poolId = CL_BUF_TO_CL_POOL(pClBuf);

  /* Lock interrupts */
  INT_LOCK(level);

  ( (CL_BUF_ID) pClBuf )->pClNext = poolId->pClHead;
  poolId->pClHead = (CL_BUF_ID) pClBuf;

  /* Update mask */
  poolId->pNetPool->clMask |= CL_LOG2_TO_CL_SIZE(poolId->clLg2);
  poolId->clNumFree++;

  /* Unlock interrputs */
  INT_UNLOCK(level);
}

/******************************************************************************
* _clPoolCarve - Carve up cluster pool
*
* RETURNS: CL_BUF_ID
******************************************************************************/

LOCAL CL_BUF_ID _clPoolCarve(CL_POOL_ID pClPool, int num, int size, char *pool)
{
  CL_BUF_ID bufId, *pBufId;
  int i;

  /* Initialize local variables */
  bufId = NULL;
  pBufId = &bufId;

  /* Make space for pointer */
  size += sizeof(void *);

  /* For all units */
  for (i = 0; i < num; i++) {

    /* Calculate pointer */
    *pBufId = (CL_BUF_ID) ( pool + sizeof(void *) );

    /* Set in pointer */
    *( (CL_POOL_ID *) (pool) ) = pClPool;

    /* Get next element */
    pBufId = &( (*pBufId)->pClNext );

    /* Advance pool pointer to next address */
    pool += size;

  } /* End for all units */

  return bufId;
}

/******************************************************************************
* _mBlkClFree - Free memory block cluster pair
*
* RETURNS: M_BLK_ID
******************************************************************************/

LOCAL M_BLK_ID _mBlkClFree(NET_POOL_ID pNetPool, M_BLK_ID pMblk)
{
  M_BLK_ID blkIdNext;

  /*  Check if block already freed */
  if (pMblk->mBlkHdr.mType == MT_FREE)
    return NULL;

  /* Initialize local variables */
  blkIdNext = pMblk->mBlkHdr.mNext;

  if (M_HASCL(pMblk))
    netClBlkFree(pMblk->pClBlk->pNetPool, pMblk->pClBlk);

  /* Free memory block */
  _mBlkFree(pNetPool, pMblk);

  return blkIdNext;
}

/******************************************************************************
* _mBlkGet - Get a free memory block
*
* RETURNS: M_BLK_ID or NULL
******************************************************************************/

LOCAL M_BLK_ID _mBlkGet(NET_POOL_ID pNetPool, int canWait, unsigned char type)
{
  int level, retry;
  M_BLK_ID blkId;

  /* Initialize local variables */
  blkId = NULL;

  /* Retry loop */
  do {

    /* Clear retry flag */
    retry = 0;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Get block from head */
    blkId = pNetPool->pmBlkHead;

    /* Check if free block available */
    if (blkId != NULL) {

      /* Set head to next free block in pool */
      pNetPool->pmBlkHead = blkId->mBlkHdr.mNext;
      pNetPool->pPoolStat->mTypes[MT_FREE]--;
      pNetPool->pPoolStat->mTypes[type]++;

      /* Unlock interrupts */
      INT_UNLOCK(level);

      /* Check free flag */
      if (blkId->mBlkHdr.mType != MT_FREE)
        return NULL;

      /* Setup block struct */
      blkId->pClBlk = NULL;
      blkId->mBlkHdr.mType = type;
      blkId->mBlkHdr.mNext = NULL;
      blkId->mBlkHdr.mNextPkt = NULL;
      blkId->mBlkHdr.mFlags = 0;

    } /* End if free block available */

    /* Else no free block available */
    else {

      /* If we can wait for block to become free loop all over again */
      if (canWait == M_WAIT) {

        /* If net buffer collect function is set */
        if (_pNetBufCollect != NULL) {

           /* Unlock interrupts */
          INT_UNLOCK(level);

	  /* Call collect function */
          (*_pNetBufCollect) (pNetPool->pPoolStat);

	  /* Retry once */
	  canWait = M_DONTWAIT;
          retry = 1;

        } /* End if net buffer collect function is set */

      } /* End if we can wait for free block to become avialable */

      /* Else can't wait for block to become free */
      else {

	/* Increase packet drops */
        pNetPool->pPoolStat->mDrops++;

        /* Unlock interrupts */
	INT_UNLOCK(level);

        return NULL;

      } /* End else can't wait for block to become free */

    } /* End else no free block available */

  } while (retry);

  return blkId;
}

/******************************************************************************
* _clBlkGet - Get a free cluster block
*
* RETURNS: CL_BLK_ID or NULL
******************************************************************************/

LOCAL CL_BLK_ID _clBlkGet(NET_POOL_ID pNetPool, int canWait)
{
  int level, retry;
  CL_BLK_ID blkId;

  /* Initialize local variables */
  blkId = NULL;

  /* Retry loop */
  do {

    /* Clear retry flag */
    retry = 0;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Get block from head */
    blkId = pNetPool->pClBlkHead;

    /* Check if free block available */
    if (blkId != NULL) {

      /* Set head to next free block */
      pNetPool->pClBlkHead = blkId->clNode.pClBlkNext;

      /* Unlock interrupts */
      INT_UNLOCK(level);

      /* Setup block struct */
      blkId->clNode.pClBuf = NULL;
      blkId->clFreeFunc = NULL;
      blkId->clRefCount = 0;

      /* Set origin pool */
      blkId->pNetPool = pNetPool;

    } /* End if free block available */

    /* Else no free block available */
    else {

      /* If we can wait for block to become free loop all over again */
      if (canWait == M_WAIT) {

        /* If net buffer collect function is set */
        if (_pNetBufCollect != NULL) {

          /* Unlock interrupts */
          INT_UNLOCK(level);

	  /* Call collect function */
          (*_pNetBufCollect) (pNetPool->pPoolStat);

	  /* Retry once */
	  canWait = M_DONTWAIT;
          retry = 1;

        } /* End if net buffer collect function is set */

      } /* End if we can wait for free block to become avialable */

      /* Else can't wait for block to become free */
      else {

        /* Unlock interrupts */
	INT_UNLOCK(level);

        return NULL;

      } /* End else can't wait for block to become free */

    } /* End else no free block available */

  } while (retry);

  return blkId;
}

/******************************************************************************
* _clusterGet - Get new cluster from cluster pool
*
* RETURNS: Pointer to cluster or NULL
******************************************************************************/

LOCAL char* _clusterGet(NET_POOL_ID pNetPool, CL_POOL_ID pClPool)
{
  int level;
  CL_BUF_ID bufId;

  /* Lock interrupts */
  INT_LOCK(level);

  /* Check if any buffers in pool */
  if (pClPool->pClHead == NULL) {

    /* Unlock interrupts */
    INT_UNLOCK(level);
    return NULL;

  }

  /* Get head of pool */
  bufId = pClPool->pClHead;

  /* Get buffer from head of pool */
  pClPool->pClHead = bufId->pClNext;

  /* Update mask if none available */
  if (pClPool->pClHead == NULL)
    pNetPool->clMask &= ~( CL_LOG2_TO_CL_SIZE(pClPool->clLg2) );

  /* Set free counters */
  pClPool->clNumFree--;
  pClPool->clUsage++;

  /* Unlock interrupts */
  INT_UNLOCK(level);

  return ((char *) bufId);
}

/******************************************************************************
* _mClGet - Get memory block cluster pair from pool
*
* RETURNS: OK or ERROR
******************************************************************************/

LOCAL STATUS _mClGet(NET_POOL_ID pNetPool, M_BLK_ID pMblk, int size,
	      int canWait, BOOL bestFit)
{
  int level, retry, sizeLog2;
  CL_POOL_ID poolId;
  CL_BLK_ID blkId;
  CL_BUF_ID bufId;

  /* Initialize local variables */
  blkId = NULL;
  bufId = NULL;

  /* Check memory block */
  if (pMblk == NULL)
    return ERROR;

  /* Get cluster block from pool */
  blkId = _clBlkGet(pNetPool, canWait);
  if (blkId == NULL) {
    _clBlkFree(blkId);
    return ERROR;
  }

  /* Get pool id for requested size */
  poolId = _clPoolIdGet(pNetPool, size, bestFit);
  if (poolId == NULL) {
    _clBlkFree(blkId);
    return ERROR;
  }

  /* Retry loop */
  do {

    /* Clear retry flag */
    retry = 0;

    /* Lock interrupts */
    INT_LOCK(level);

    /* If pool head null */
    if (poolId->pClHead == NULL) {

      /* If best match */
      if (pNetPool->clMask >= CL_LOG2_TO_CL_SIZE(poolId->clLg2)) {

	/* Get size and size log2 */
        size = CL_LOG2_TO_CL_SIZE(poolId->clLg2);
        sizeLog2 = poolId->clLg2;

	/* Scan for pool with resonable size */
	while (size <= CL_SIZE_MAX) {

	  /* Resonable sized pool found */
	  if (pNetPool->clMask & size) {
	    poolId = pNetPool->clTable[CL_LOG2_TO_CL_INDEX(sizeLog2)];
	    break;
          }

	  /* Advance to next size to scan for */
	  size <<= 1;
	  sizeLog2++;

        } /* End scan for pool with resonable size */

      } /* End end best match */

      /* Else if close fit */
      else if (!bestFit && pNetPool->clMask) {

	/* Find buffer with cloest size */
        poolId = pNetPool->clTable[CL_SIZE_TO_CL_INDEX(pNetPool->clMask)];

      } /* End else if close fit */

      /* Else wait for buffer to become free */
      else if (canWait == M_WAIT) {

        /* Check if collect function is set */
	if (_pNetBufCollect) {

	  /* Unlock interrupts */
	  INT_UNLOCK(level);

	  /* Call collect function */
	  (*_pNetBufCollect)(pNetPool->pPoolStat);

	  /* Try once again */
	  canWait = M_DONTWAIT;
	  retry = 1;

        } /* End if collect function is set */

      } /* End else wait for buffer to become free */

    } /* End if pool head null */

  } while (retry);

  /* Get buffer from pool head */
  bufId = poolId->pClHead;

  /* If buffer exists */
  if (bufId != NULL) {

    /* Set free head to next buffer */
    poolId->pClHead = bufId->pClNext;

    /* If this is the last cluster set mask */
    if (poolId->pClHead == NULL)
      pNetPool->clMask &= ~( CL_LOG2_TO_CL_SIZE(poolId->clLg2) );

   /* Set free buffer counters */
   poolId->clNumFree--;
   poolId->clUsage++;

   /* Unlock interrupts */
   INT_UNLOCK(level);

  } /* End if buffer exists */

  /* Else buffer not exists */
  else {

    /* Increase number of drops */
    pNetPool->pPoolStat->mDrops++;

    /* Unlock interrupts */
    INT_UNLOCK(level);

    /* Free block */
    _clBlkFree(blkId);

    return ERROR;

  } /* End else buffer not exists */

  /* Update memory block struct */
  pMblk->mBlkHdr.mData = (char *) bufId;
  pMblk->mBlkHdr.mFlags |= M_EXT;

  /* Update buffer structure */
  blkId->clNode.pClBuf = (char *) bufId;
  blkId->clSize = poolId->clSize;
  blkId->clFreeFunc = NULL;
  blkId->clRefCount = 1;

  /* Store cluster block in memory block struct */
  pMblk->pClBlk = blkId;

  return OK;
}

/******************************************************************************
* _clPoolIdGet - Get pool id for given cluster size
*
* RETURNS: CL_POOL_ID or NULL
******************************************************************************/

LOCAL CL_POOL_ID _clPoolIdGet(NET_POOL_ID pNetPool, int size, BOOL bestFit)
{
  int sizeLog2;

  /* Buffer size greater than maximum */
  if (size > pNetPool->clSizeMax) {

    sizeLog2 = pNetPool->clLg2Max;

    /* Not a best fit */
    if (bestFit)
      return NULL;
  } /* End if size greater than max */

  /* Buffer size less than minimum */
  else if (size < pNetPool->clSizeMin) {

    sizeLog2 = pNetPool->clLg2Min;

  } /* End else if size less than min */

  /* Else buffer size within range */
  else {

    sizeLog2 = SIZE_TO_LOG2(size);

    /* Check if need one extra in size */
    if ( size > CL_LOG2_TO_CL_SIZE(sizeLog2) )
      sizeLog2++;

  } /* End else size within range */

  return ( pNetPool->clTable[CL_LOG2_TO_CL_INDEX(sizeLog2)] );
}

/******************************************************************************
* netBufLibInit - Initialize net buffers library
*
* RETURNS: N/A
******************************************************************************/

void netBufLibInit(void)
{
}

/******************************************************************************
* netPoolInit - Initialize netPool
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS netPoolInit(NET_POOL_ID poolId, M_CL_CONFIG *pMclBlkConfig,
		   CL_DESC *pClDescTable, int clDescTableNumEntries,
		   POOL_FUNC *pFuncTable)
{

  /* Check net pool argument */
  if (poolId == NULL)
    return ERROR;

  /* Set to custom pool function table in not null */
  if (pFuncTable != NULL)
    poolId->pFuncTable = pFuncTable;
  else
    poolId->pFuncTable = _pNetPoolFuncTable;

  /* Call poolInit from function table */
  return POOL_INIT(poolId, pMclBlkConfig, pClDescTable,
		  clDescTableNumEntries);
}

/******************************************************************************
* netMblkGet - Get memory block from net pool
*
* RETURNS: M_BLK_ID or NULL
******************************************************************************/

M_BLK_ID netMblkGet(NET_POOL_ID poolId, int canWait, unsigned char type)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->mBlkGetFunc == NULL)
    return NULL;

  return M_BLK_GET(poolId, canWait, type);
}

/******************************************************************************
* netMblkFree - Free memory block
*
* RETURNS: N/A
******************************************************************************/

void netMblkFree(NET_POOL_ID poolId, M_BLK_ID mBlkId)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->mBlkFreeFunc == NULL || mBlkId == NULL)
    return;

  M_BLK_FREE(poolId, mBlkId);
}

/******************************************************************************
* netClBlkGet - Get cluster block from net pool
*
* RETURNS: CL_BLK_ID or NULL
******************************************************************************/

CL_BLK_ID netClBlkGet(NET_POOL_ID poolId, int canWait)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->clBlkGetFunc == NULL)
    return NULL;

  return CL_BLK_GET(poolId, canWait);
}

/******************************************************************************
* netClBlkFree - Free cluster block
*
* RETURNS: N/A
******************************************************************************/

void netClBlkFree(NET_POOL_ID poolId, CL_BLK_ID clBlkId)
{
  /* Get net pool accosicated with cluster block */
  //poolId = clBlkId->pNetPool;

  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->clBlkFreeFunc == NULL || clBlkId == NULL)
    return;

  CL_BLK_FREE(poolId, clBlkId);
}

/******************************************************************************
* netClusterGet - Get cluster cluster
*
* RETURNS: char*
******************************************************************************/

char* netClusterGet(NET_POOL_ID poolId, CL_POOL_ID clPoolId)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->clusterGetFunc == NULL)
    return NULL;

  return CLUSTER_GET(poolId, clPoolId);
}

/******************************************************************************
* netClFree - Free cluster
*
* RETURNS: N/A
******************************************************************************/

void netClFree(NET_POOL_ID poolId, char *pClBuf)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->clFreeFunc == NULL || pClBuf == NULL)
    return;

  CL_FREE(poolId, pClBuf);
}

/******************************************************************************
* netMblkClGet - Get memory cluster block pair
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS netMblkClGet(NET_POOL_ID poolId, M_BLK_ID mBlkId, int size,
		  int canWait, BOOL bestFit)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->mBlkClGetFunc == NULL)
    return ERROR;

  return M_CL_GET(poolId, mBlkId, size, canWait, bestFit);
}

/******************************************************************************
* netMblkClFree - Free memory cluster block pair
*
* RETURNS: M_BLK_ID
******************************************************************************/

M_BLK_ID netMblkClFree(M_BLK_ID mBlkId)
{
  NET_POOL_ID poolId;

  /* Check memory block */
  if (mBlkId == NULL)
    return NULL;

  /* Get net pool */
  poolId = MBLK_TO_NET_POOL(mBlkId);

  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->mBlkClFreeFunc == NULL)
    return NULL;

  return M_BLK_CL_FREE(poolId, mBlkId);
}

/******************************************************************************
* netMblkClChainFree - Free memory cluster block chain
*
* RETURNS: N/A
******************************************************************************/

void netMblkClChainFree(M_BLK_ID mBlkId)
{
  NET_POOL_ID poolId;

  /* While valid memory block */
  while (mBlkId != NULL) {

    /* Get net pool */
    poolId = MBLK_TO_NET_POOL(mBlkId);

    /* Check if valid net pool */
    if (poolId == NULL || poolId->pFuncTable == NULL ||
        poolId->pFuncTable->mBlkClFreeFunc == NULL)
      return;

    /* Free memory cluster block pair and get next block */
    mBlkId = M_BLK_CL_FREE(poolId, mBlkId);

  } /* End while valid memory block */

}

/******************************************************************************
* netClPoolIdGet - Get cluster pool id from size
*
* RETURNS: CL_POOL_ID or NULL
******************************************************************************/

CL_POOL_ID netClPoolIdGet(NET_POOL_ID poolId, int size, BOOL bestFit)
{
  /* Check if valid net pool */
  if (poolId == NULL || poolId->pFuncTable == NULL ||
      poolId->pFuncTable->clPoolIdGetFunc == NULL)
    return NULL;

  return CL_POOL_ID_GET(poolId, size, bestFit);
}

/******************************************************************************
* netTupleGet - Get mBlk, clBlk and cluster tuple
*
* RETURNS: M_BLK_ID or NULL
******************************************************************************/

M_BLK_ID netTupleGet(NET_POOL_ID poolId, int size, int canWait,
		     unsigned char type, BOOL bestFit)
{
  M_BLK_ID blkId;

  /* Check pool and pool function table */
  if (poolId == NULL || poolId->pFuncTable == NULL)
    return NULL;

  /* Get a memory block */
  blkId = M_BLK_GET(poolId, canWait, type);

  /* If all went painlessly */
  if (blkId != NULL && blkId->pClBlk != NULL) {

    /* Check if size is not to big */
    if (bestFit && size > blkId->pClBlk->clSize) {
      netMblkFree(poolId, blkId);
      return NULL;
    }

    return blkId;

  } /* End if all went painlessly */

  /* Try to allocate a cluster and point memory block to it */
  if (M_CL_GET(poolId, blkId, size, canWait, bestFit) != OK) {
    if (blkId != NULL)
      netMblkFree(poolId, blkId);
    return NULL;

  }

  return blkId;
}

/******************************************************************************
* netPoolDelete - Delete netPool
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS netPoolDelete(NET_POOL_ID poolId)
{
  int i, j;
  CL_POOL_ID pClPool;

  /* Check if poolStat is valid */
  if (poolId->pPoolStat == NULL)
    return ERROR;

  free(poolId->pPoolStat);

  /* For all entries in cluster table */
  for (i = 0; i < CL_TABLE_SIZE; i++) {

    /* Get current cluster from table */
    if ( (pClPool = poolId->clTable[i]) != NULL) {

      /* Set all other entries in cluster table equal to this to NULL */
      for (j = i + 1; j < CL_TABLE_SIZE; j++)
        if (pClPool == poolId->clTable[j])
          poolId->clTable[j] = NULL;

      /* Free (this) */
      free(pClPool);

    } /* End if (this) in cluster table */

  } /* End for all entries in cluster table */

  /* Zero out struct */
  memset(poolId, 0, sizeof(NET_POOL));

  return OK;
}

/******************************************************************************
* netMblkChainDup - Duplicate chain
*
* RETURNS: M_BLK_ID
******************************************************************************/

M_BLK_ID netMblkChainDup(NET_POOL_ID poolId, M_BLK_ID blkId, int offset,
			 int len, int canWait)
{
  M_BLK_ID newBlkId, topBlkId;
  M_BLK_ID *pBlkId;
  BOOL copyhdr;
  int offs, level;

  /* Get offset */
  offs = offset;

  /* If invalid arguments */
  if ( (offs < 0) || (len < 0) )
    return NULL;

  /* If zero offset and block header */
  if ( (offs == 0) && (blkId->mBlkHdr.mFlags & M_PKTHDR) )
    copyhdr = TRUE;
  else
    copyhdr = FALSE;

  /* While offset gt. zero */
  while (offs > 0) {

    /* If null block */
    if (blkId == NULL)
      return NULL;

    /* If offset lt. length */
    if (offs < blkId->mBlkHdr.mLen)
      break;

    /* Advance */
    offs -= blkId->mBlkHdr.mLen;
    blkId = blkId->mBlkHdr.mNext;

  } /* End while offset gt. zero */

  /* Set double pointer to top block */
  pBlkId = &topBlkId;

  /* While length gt. zero */
  while (len > 0) {

    /* If null block */
    if (blkId == NULL) {

      /* If length copy all */
      if (len != M_COPYALL) {

        if (topBlkId != NULL)
          netMblkClChainFree(topBlkId);

        return NULL;

      } /* End if length copy all */

      /* Get out of loop */
      break;

    } /* End if null block */

    /* Get net block */
    newBlkId = M_BLK_GET(poolId, canWait, blkId->mBlkHdr.mType);

    /* Store new block at double pointer */
    *pBlkId = newBlkId;

    if (*pBlkId == NULL) {

      if (topBlkId != NULL)
        netMblkClChainFree(topBlkId);

      return NULL;

    }

    /* If copy header */
    if (copyhdr) {

      newBlkId->mBlkPktHdr = blkId->mBlkPktHdr;
      newBlkId->mBlkHdr.mFlags = blkId->mBlkHdr.mFlags;

      if (len == M_COPYALL)
        newBlkId->mBlkPktHdr.len -= offset;
      else
        newBlkId->mBlkPktHdr.len = len;

      copyhdr = FALSE;

    } /* End if copy header */

    /* Else don't copy header */
    else {

      newBlkId->mBlkHdr.mFlags = blkId->mBlkHdr.mFlags & ~M_PKTHDR;

    } /* End else don't copy header */

    newBlkId->mBlkHdr.mLen = min(len, blkId->mBlkHdr.mLen - offs);
    newBlkId->mBlkHdr.mData = blkId->mBlkHdr.mData + offs;
    newBlkId->pClBlk = blkId->pClBlk;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Increase referece count */
    ++(newBlkId->pClBlk->clRefCount);

    /* Unlock interrupts */
    INT_UNLOCK(level);

    /* If copy all */
    if (len != M_COPYALL)
      len -= newBlkId->mBlkHdr.mLen;

    /* Prepeare for next run */
    offs = 0;
    blkId = blkId->mBlkHdr.mNext;
    pBlkId = &newBlkId->mBlkHdr.mNext;

  } /* End while length gt. zero */

  return topBlkId;
}

/******************************************************************************
* netBufCollectFuncSet - Set net buffer collect function
*
* RETURNS: N/A
******************************************************************************/

void netBufCollectFuncSet(FUNCPTR func)
{
  _pNetBufCollect = func;
}

