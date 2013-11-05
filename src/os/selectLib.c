/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
 *
 *   Real VMX is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Real VMX is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
 */

/* selectLib.c - Select Library */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/errnoLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskHookLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/ioLib.h>
#include <os/iosLib.h>
#include <os/private/iosLibP.h>
#include <os/excLib.h>
#include <os/selectLib.h>

IMPORT int sysClockRateGet(
    void
    );

/* Locals */
LOCAL BOOL selectLibInstalled = FALSE;
LOCAL int  selMutexOptions = SEM_Q_FIFO | SEM_DELETE_SAFE;

LOCAL STATUS selTaskCreateHook(
    TCB_ID tcbId
    );

LOCAL void selTaskDeleteHook(
    TCB_ID tcbId
    );

LOCAL STATUS selDoIoctls(
    fd_set *pFdSet,
    int width,
    int func,
    SEL_WAKEUP_NODE *pWakeupNode,
    BOOL stopOnError
    );

/******************************************************************************
 * selectLibInit - Initialize select library
 *
 * RETURNS: OK or ERROR
 */

STATUS selectLibInit(
    void
    )
{
    STATUS status;

    if (selectLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        if (taskCreateHookAdd((FUNCPTR) selTaskCreateHook) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Initialize select context for current task */
            if (taskIdCurrent->selectContextId == NULL)
            {
                if (selTaskCreateHook(taskIdCurrent) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    selectLibInstalled = TRUE;
                    status = OK;
                }
            }
            else
            {
                selectLibInstalled = TRUE;
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * selTaskDeleteHookAdd - Initialize select library delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS selTaskDeleteHookAdd(
    void
    )
{
    STATUS status;

    /* If failed to install hook */
    if (taskDeleteHookAdd((FUNCPTR) selTaskDeleteHook) != OK)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * select - Select on a set of file descriptors
 *
 * RETURNS: Number of file descritors, zero if timeout, otherwise ERROR
 */

int select(
    int width,
    fd_set *pReadFds,
    fd_set *pWriteFds,
    fd_set *pExceptFds,
    struct timeval *pTimeOut
    )
{
    int ret;
    SEL_WAKEUP_NODE wNode;
    int fd, msk, bw, errnoValue;
    int quitTime, clockRate;
    int numFound = 0;
    SEL_CONTEXT_ID selectContextId = taskIdCurrent->selectContextId;

    /* Check select context */
    if (selectContextId == NULL)
    {
        errnoSet(S_selectLib_NO_SELECT_CONTEXT);
        ret = ERROR;
    }
    else
    {
        /* Check width */
        if ((width < 0) || ((width > iosMaxFd) && (width > FD_SETSIZE)))
        {
            errnoSet(S_selectLib_WIDTH_OUT_OF_RANGE);
            ret = ERROR;
        }
        else
        {
            /* Adjust width */
            if (width > iosMaxFd)
            {
                width = iosMaxFd;
            }

            /* Get max width in bytes */
            bw = howmany(width, NFDBITS) * sizeof(fd_mask);

            /* Copy or erase original read filedescriptors */
            if (pReadFds != NULL)
            {
                memcpy(selectContextId->pOrigReadFds, pReadFds, bw);
            }
            else
            {
                memset(selectContextId->pOrigReadFds, 0, bw);
            }

            /* Copy or erase original write filedescriptors */
            if (pWriteFds != NULL)
            {
                memcpy(selectContextId->pOrigWriteFds, pWriteFds, bw);
            }
            else
            {
                memset(selectContextId->pOrigWriteFds, 0, bw);
            }

            /* If time info argument given */
            if (pTimeOut != NULL)
            {
                clockRate = sysClockRateGet();

                /* Convert */
                quitTime = (pTimeOut->tv_sec * clockRate) +
                    ((((pTimeOut->tv_usec * clockRate) / 100) / 100) / 100);
            }
            else
            {
                quitTime = 0;
            }

            /* Initialize select context */
            selectContextId->pReadFds  = pReadFds;
            selectContextId->pWriteFds = pWriteFds;

            semTake(&selectContextId->wakeupSync, WAIT_NONE);

            wNode.taskId = taskIdSelf();

            /* Clear read file descriptor set */
            if (pReadFds != NULL)
            {
                memset(pReadFds, 0, bw);
            }

            /* Clear write file descriptor set */
            if (pWriteFds != NULL)
            {
                memset(pWriteFds, 0, bw);
            }

            /* Clear except file descriptor set */
            if (pExceptFds != NULL)
            {
                memset(pExceptFds, 0, bw);
            }

            /* Reset status flag */
            ret = OK;

            /* If read file descriptors */
            if (pReadFds != NULL)
            {
                wNode.type = SELREAD;
                if (selDoIoctls(
                        selectContextId->pOrigReadFds,
                        width,
                        FIOSELECT,
                        &wNode,
                        TRUE
                        ) != OK)
                {
                    ret = ERROR;
                }
            }

            if (ret != ERROR)
            {
                /* If write file descriptors */
                if (pWriteFds != NULL)
                {
                    wNode.type = SELWRITE;
                    if (selDoIoctls(
                            selectContextId->pOrigWriteFds,
                            width,
                            FIOSELECT,
                            &wNode,
                            TRUE
                            ) != OK)
                    {
                        ret = ERROR;
                    }
                }
            }

            /* If error */
            if (ret != OK)
            {
                /* Get errno */
                errnoValue = errnoGet();

                /* Unselect read file descriptors */
                wNode.type = SELREAD;
                selDoIoctls(
                    selectContextId->pOrigReadFds,
                    width,
                    FIOUNSELECT,
                    &wNode,
                    FALSE
                    );

                /* Unselect write file descriptors */
                wNode.type = SELWRITE;
                selDoIoctls(
                    selectContextId->pOrigWriteFds,
                    width,
                    FIOUNSELECT,
                    &wNode,
                    FALSE
                    );

                if (errnoValue == S_ioLib_UNKNOWN_REQUEST)
                {
                    errnoSet(S_selectLib_NO_SELECT_SUPPORT_IN_DRIVER);
                }
            }
            else
            {
                /* Get requested timeout */
                if ((pTimeOut != NULL) && (quitTime == 0))
                {
                    quitTime = WAIT_NONE;
                }
                else if (pTimeOut == NULL)
                {
                    quitTime = WAIT_FOREVER;
                }

                /* Indicate task pending on selection */
                selectContextId->width          = width;
                selectContextId->pendedOnSelect = TRUE;

                semTake(&selectContextId->wakeupSync, quitTime);

                /* If read file descriptors */
                if (pReadFds != NULL)
                {
                    wNode.type = SELREAD;
                    if (selDoIoctls(
                            selectContextId->pOrigReadFds,
                            width,
                            FIOUNSELECT,
                            &wNode,
                            FALSE
                            ) != OK)
                    {
                        ret = ERROR;
                    }
                }

                /* If write file descriptors */
                if (pWriteFds != NULL)
                {
                    wNode.type = SELWRITE;
                    if (selDoIoctls(
                            selectContextId->pOrigWriteFds,
                            width,
                            FIOUNSELECT,
                            &wNode,
                            FALSE
                            ) != OK)
                    {
                        ret = ERROR;
                    }
                }

                /* Mark as not pended on select */
                selectContextId->pendedOnSelect = FALSE;
            }

            if (ret == OK)
            {
                /* If read file descriptors */
                if (pReadFds != NULL)
                {
                    /* For all file descriptors */
                    for (fd = 0; fd < width; fd++)
                    {
                        msk = pReadFds->fds_bits[((unsigned) fd) / NFDBITS];
                        if (msk == 0)
                        {
                            fd += NFDBITS - 1;
                        }
                        else if (msk & (1 << (((unsigned) fd) % NFDBITS)))
                        {
                            numFound++;
                        }
                    }
                }

                /* If write file descriptors */
                if (pWriteFds != NULL)
                {
                    /* For all file descriptors */
                    for (fd = 0; fd < width; fd++)
                    {
                        msk = pWriteFds->fds_bits[((unsigned) fd) / NFDBITS];
                        if (msk == 0)
                        {
                            fd += NFDBITS - 1;
                        }
                        else if (msk & (1 << (((unsigned) fd) % NFDBITS)))
                        {
                            numFound++;
                        }
                    }
                }

                ret = numFound;
            }
        }
    }

    return numFound;
}

/******************************************************************************
 * selWakeupListInit - Initialize wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selWakeupListInit(
    SEL_WAKEUP_LIST *pList
    )
{
    STATUS status;

    if (listInit(&pList->wakeupList) != OK)
    {
        status = ERROR;
    }
    else
    {
        if (semMInit(&pList->mutex, selMutexOptions) != OK)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * selWakeupListTerminate - Terminate wakeup list
 *
 * RETURNS: N/A
 */

void selWakeupListTerminate(
    SEL_WAKEUP_LIST *pList
    )
{
    semTerminate(&pList->mutex);
}

/******************************************************************************
 * selWakeupListLen - Get wakeup list size
 *
 * RETURNS: Number of nodes
 */

int selWakeupListLen(
    SEL_WAKEUP_LIST *pList
    )
{
    return listCount(&pList->wakeupList);
}

/******************************************************************************
 * selWakeupType - Get select type
 *
 * RETURNS: Select type
 */

SELECT_TYPE selWakeupType(
    SEL_WAKEUP_NODE *pNode
    )
{
    return pNode->type;
}

/******************************************************************************
 * selNodeAdd - Add to wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selNodeAdd(
    SEL_WAKEUP_LIST *pList,
    SEL_WAKEUP_NODE *pNode
    )
{
    STATUS status;
    BOOL dontFree;
    SEL_WAKEUP_NODE *pCpNode;

    semTake(&pList->mutex, WAIT_FOREVER);

    /* Check if zero */
    if (listCount(&pList->wakeupList) == 0)
    {
        pCpNode = &pList->first;
        dontFree = TRUE;
    }
    else
    {
        pCpNode = (SEL_WAKEUP_NODE *) malloc(sizeof(SEL_WAKEUP_NODE));
        dontFree = FALSE;
    }

    if (pCpNode == NULL)
    {
        semGive(&pList->mutex);
        status = ERROR;
    }
    else
    {
        /* Copy node */
        *pCpNode = *pNode;
        pCpNode->dontFree = dontFree;

        /* Add to list */
        listAdd(&pList->wakeupList, (LIST_NODE *) pCpNode);

        semGive(&pList->mutex);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * selNodeDelete - Delete from wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selNodeDelete(
    SEL_WAKEUP_LIST *pList,
    SEL_WAKEUP_NODE *pNode
    )
{
    SEL_WAKEUP_NODE *pI;
    STATUS status = ERROR;

    semTake(&pList->mutex, WAIT_FOREVER);

    for (pI = (SEL_WAKEUP_NODE *) LIST_HEAD(&pList->wakeupList);
         pI != NULL;
         pI = (SEL_WAKEUP_NODE *) LIST_NEXT((LIST_NODE *) pI))
    {
        /* Check for match */
        if ((pI->taskId == pNode->taskId) && (pI->type == pNode->type))
        {
            /* Delete node */
            listRemove(&pList->wakeupList, (LIST_NODE *) pI);

            if (pI->dontFree == FALSE)
            {
                free(pI);
            }

            semGive(&pList->mutex);
            status = OK;
            break;
        }
    }

    semGive(&pList->mutex);

    return status;
}

/******************************************************************************
 * selWakeup - Wakeup a task sleeping on file descriptor
 *
 * RETURNS: N/A
 */

void selWakeup(
    SEL_WAKEUP_NODE *pNode
    )
{
    TCB_ID tcbId;
    SEL_CONTEXT_ID selectContextId;

    /* Get current select context */
    tcbId = (TCB_ID) pNode->taskId;
    selectContextId = tcbId->selectContextId;

    /* Select type */
    switch(pNode->type)
    {
        case SELREAD:
            FD_SET(pNode->fd, selectContextId->pReadFds);
            break;

        case SELWRITE:
            FD_SET(pNode->fd, selectContextId->pWriteFds);
            break;
    }

    semGive(&selectContextId->wakeupSync);
}

/******************************************************************************
 * selWakeupAll - Wakeup all tasks sleeping in wakeup list of specified type
 *
 * RETURNS: N/A
 */

void selWakeupAll(
    SEL_WAKEUP_LIST *pList,
    SELECT_TYPE type
    )
{
    SEL_WAKEUP_NODE *pNode;

    /* Check if any */
    if (listCount(&pList->wakeupList) != 0)
    {
        /* If in interrupt */
        if (INT_CONTEXT() == TRUE)
        {
            excJobAdd(
                selWakeupAll,
                (ARG) pList,
                (ARG) type,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );
        }
        else
        {
            semTake(&pList->mutex, WAIT_FOREVER);

            /* Wakeup all of correct type */
            for (pNode = (SEL_WAKEUP_NODE *) LIST_HEAD(&pList->wakeupList);
                 pNode != NULL;
                 pNode = (SEL_WAKEUP_NODE *) LIST_NEXT((LIST_NODE *) pNode))
            {
                if (pNode->type == type)
                {
                    selWakeup(pNode);
                }
            }

            semGive(&pList->mutex);
        }
    }
}

/******************************************************************************
 * selTaskCreateHook - Run for every created task
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS selTaskCreateHook(
    TCB_ID tcbId
    )
{
    STATUS status;
    SEL_CONTEXT *pSelectContext;
    int fdSetBytes;

    /* Allocate memory */
    fdSetBytes = sizeof (fd_mask) * howmany(iosMaxFd, NFDBITS);
    pSelectContext = (SEL_CONTEXT *) malloc(
                                         sizeof(SEL_CONTEXT) + 2 * fdSetBytes
                                         );
    if (pSelectContext == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Initialize structure */
        pSelectContext->pOrigReadFds =
            (fd_set *) ((int) pSelectContext + sizeof(SEL_CONTEXT));
        pSelectContext->pOrigWriteFds = (fd_set *)
            (fd_set *) ((int) pSelectContext->pOrigReadFds + fdSetBytes);
        pSelectContext->pendedOnSelect = FALSE;

        /* Initialize sempahore */
        if (semBInit(
                &pSelectContext->wakeupSync,
                SEM_Q_PRIORITY,
                SEM_EMPTY
                ) != OK)
        {
            free(pSelectContext);
            status = ERROR;
        }
        else
        {
            /* Setup pointer in tcb */
            tcbId->selectContextId = pSelectContext;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * selTaskDeleteHook - Run for every deleted task
 *
 * RETURNS: OK or ERROR
 */

LOCAL void selTaskDeleteHook(
    TCB_ID tcbId
    )
{
    SEL_WAKEUP_NODE wNode;
    SEL_CONTEXT_ID selectContextId;

    /* Get select context from task to be deleted */
    selectContextId = tcbId->selectContextId;
    if (selectContextId != NULL)
    {
        /* If pending on select */
        if (selectContextId->pendedOnSelect == TRUE)
        {
            /* Store task id in wakeup node */
            wNode.taskId = (int) tcbId;

            /* Unselect all read file descriptors */
            wNode.type = SELREAD;
            selDoIoctls(
                selectContextId->pOrigReadFds,
                selectContextId->width,
                FIOUNSELECT,
                &wNode,
                FALSE
                );

            /* Unselect all write file descriptors */
            wNode.type = SELWRITE;
            selDoIoctls(
                selectContextId->pOrigWriteFds,
                selectContextId->width,
                FIOUNSELECT,
                &wNode,
                FALSE
                );
        }

        /* Terminate semaphore */
        semTerminate(&selectContextId->wakeupSync);

        /* Free memory */
        free(selectContextId);
    }
}

/******************************************************************************
 * selDoIoctls - Perform ioctl on all file descriptors in mask
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS selDoIoctls(
    fd_set *pFdSet,
    int width,
    int func,
    SEL_WAKEUP_NODE *pWakeupNode,
    BOOL stopOnError
    )
{
    int fd, msk;
    int ret = OK;

    /* For all file descritptors */
    for (fd = 0; fd < width; fd++)
    {
        /* Get long word from fd_set */
        msk = pFdSet->fds_bits[((unsigned) fd) / NFDBITS];
        if (msk == 0)
        {
            fd += NFDBITS - 1;
        }
        else if (msk & (1 << (((unsigned) fd) % NFDBITS)))
        {
            pWakeupNode->fd = fd;

            /* If ioctl performed on file descriptor */
            if (ioctl(fd, func, (int) pWakeupNode) != OK)
            {
                ret = ERROR;
                if (stopOnError == TRUE)
                {
                    break;
                }
            }
        }
    }

    return ret;
}

