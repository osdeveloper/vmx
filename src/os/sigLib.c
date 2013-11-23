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

/* sigLib.c - Signal handeling library */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/taskArchLib.h>
#include <arch/excArchLib.h>
#include <arch/sigArchLib.h>
#include <util/qLib.h>
#include <util/qFifoLib.h>
#include <vmx/vmxLib.h>
#include <vmx/taskLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/taskHookLib.h>
#include <os/ffsLib.h>
#include <os/errnoLib.h>
#include <os/logLib.h>
#include <os/excLib.h>
#include <os/sigLib.h>

/* Macros */
#define issig(m)                        (1 <= (m) && (m) <= _NSIGS)
#define structbase(s, m, p)             ((s *) ((char *)(p) - OFFSET(s, m)))

/* Locals */
LOCAL struct sigpend *pSigQueueFreeHead;
LOCAL BOOL            sigLibInstalled   = FALSE;
LOCAL BOOL            sigqueueInstalled = FALSE;

LOCAL void sigDeleteHook(
    TCB_ID tcbId
    );

LOCAL struct sigtcb* sigTcbGet(
    void
    );

LOCAL int sigPendGet(
    struct sigtcb  *pSigTcb,
    const sigset_t *sigset,
    struct siginfo *pSigInfo
    );

LOCAL void sigWrapper(
    struct sigcontext *pSigContext
    );

LOCAL BOOL sigPendRun(
    struct sigtcb *pSigTcb
    );

LOCAL void sigExcSend(
    int      signo,
    int      code,
    REG_SET *pRegSet
    );

LOCAL void sigExcKill(
    int      type,
    int      code,
    REG_SET *pRegSet
    );

LOCAL int sigVmxRestart(
    TCB_ID tcbId
    );

LOCAL void sigVmxPendKill(
    TCB_ID          tcbId,
    struct sigpend *pSigPend
    );

LOCAL void sigVmxKill(
    TCB_ID tcbId,
    int    signo
    );

/******************************************************************************
 * sigLibInit - Initialize signal library
 *
 * RETURNS: OK or ERROR
 */

STATUS sigLibInit(
    void
    )
{
    STATUS status;

    if (sigLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Add task delete hook for signals */
        if (taskDeleteHookAdd((FUNCPTR) sigDeleteHook) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Setup exception kill hook */
            excSigKillHookSet(sigExcKill);

            /* Mark as installed */
            sigLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * sigqueueInit - Initialize signal queue
 *
 * RETURNS: OK or ERROR
 */

STATUS sigqueueInit(
    int nQueues
    )
{
    STATUS          status;
    struct sigpend *pSigPend;

    if (sigqueueInstalled)
    {
        status = OK;
    }
    else
    {
        /* Check minimum number if queues */
        if (nQueues < 1)
        {
            status = ERROR;
        }
        else
        {
            /* Allocate storage for pending queues */
            pSigPend = (struct sigpend *) malloc(
                                              nQueues * sizeof(struct sigpend)
                                              );
            if (pSigPend == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Initialize pending queues */
                while (nQueues--)
                {
                    /* Set buffer */
                    *(struct sigpend **) pSigPend = pSigQueueFreeHead;
                    pSigQueueFreeHead = pSigPend;

                    /* Advance to next node */
                    pSigPend++;
                }

                /* Mark as installed */
                sigqueueInstalled = TRUE;
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * sigDeleteHook - Task delete hook for signal library
 *
 * RETURNS: N/A
 */

LOCAL void sigDeleteHook(
    TCB_ID tcbId
    )
{
    struct sigtcb  *pSigTcb;
    struct sigpend *pSigPend;
    struct sigq    *pSigQueue;
    int             i;

    /* Get signal info from tcb */
    pSigTcb = tcbId->pSignalInfo;
    if (pSigTcb != NULL)
    {
        /* Enter kernel mode */
        kernelState = TRUE;

        /* For all signal types */
        for (i = 0; i <= _NSIGS; i++)
        {
            /* Get next node from signal queue */
            pSigQueue = pSigTcb->sigt_qhead[i].sigq_next;

            /* While not head node */
            while (pSigQueue != &pSigTcb->sigt_qhead[i])
            {
                /* Get next pending signal from task queue */
                pSigPend = structbase(struct sigpend, sigp_q, pSigQueue);
                pSigQueue = pSigQueue->sigq_next;

                /* Set prev and next nodes to zero */
                pSigPend->sigp_q.sigq_prev = NULL;
                pSigPend->sigp_q.sigq_next = NULL;

                /* Check if signal is on queue */
                if (pSigPend->sigp_info.si_code == SI_QUEUE)
                {
                    /* Release buffer */
                    *(struct sigpend **) pSigPend = pSigQueueFreeHead;
                    pSigQueueFreeHead = pSigPend;
                }
            }
        }

        /* Exit kernel */
        vmxExit();
    }
}

/******************************************************************************
 * sigTcbGet - Get signal tcb pointer for current task
 *
 * RETURNS: Signal tcb pointer
 */

LOCAL struct sigtcb* sigTcbGet(
    void
    )
{
    TCB_ID         tcbId;
    struct sigtcb *pSigTcb;
    int            i;

    /* Not callable from interrupt */
    if (INT_RESTRICT() != OK)
    {
        pSigTcb = NULL;
    }
    else
    {
        /* Get current task id */
        tcbId = taskIdCurrent;

        /* Check if signal info for the current task already exists */
        if (tcbId->pSignalInfo != NULL)
        {
            pSigTcb = tcbId->pSignalInfo;
        }
        else
        {
            /* Allocate signal info for task */
            pSigTcb = (struct sigtcb *) taskStackAllot(
                                            (int) taskIdCurrent,
                                            sizeof(struct sigtcb)
                                            );
            if (pSigTcb == NULL)
            {
                errnoSet(ENOMEM);
            }
            else
            {
                /* Initialize signal info */
                tcbId->pSignalInfo = pSigTcb;
                memset(pSigTcb, 0, sizeof(struct sigtcb));
                for (i = 0; i <= _NSIGS; i++)
                {
                    pSigTcb->sigt_qhead[i].sigq_next = &pSigTcb->sigt_qhead[i];
                    pSigTcb->sigt_qhead[i].sigq_prev = &pSigTcb->sigt_qhead[i];
                }
            }
        }
    }

    return pSigTcb;
}

/******************************************************************************
 * sigPendGet - Get a pending signal
 *
 * RETURNS: Signal number or OK
 */

LOCAL int sigPendGet(
    struct sigtcb  *pSigTcb,
    const sigset_t *sigset,
    struct siginfo *pSigInfo
    )
{
    long            signo;
    long            sigmsk;
    struct sigpend *pSigPend;

    /* Get signal mask sigset and tcb pending signals */
    sigmsk = *sigset & pSigTcb->sigt_pending;

    /* If zero, just return */
    if (!sigmsk)
    {
        signo = OK;
    }
    else
    {
        /* Extract lowest bits */
        sigmsk &= -sigmsk;
        signo = ffsMsb(sigmsk);

        /* If kill signal */
        if (sigmsk & pSigTcb->sigt_kilsigs)
        {
            pSigTcb->sigt_kilsigs &= ~sigmsk;

            pSigInfo->si_signo           = signo;
            pSigInfo->si_code            = SI_KILL;
            pSigInfo->si_value.sival_int = 0;
        }
        else
        {
            pSigPend = structbase(
                           struct sigpend,
                           sigp_q,
                           pSigTcb->sigt_qhead[signo].sigq_next
                           );

            pSigPend->sigp_q.sigq_prev->sigq_next = pSigPend->sigp_q.sigq_next;
            pSigPend->sigp_q.sigq_next->sigq_prev = pSigPend->sigp_q.sigq_prev;
            pSigPend->sigp_q.sigq_prev            = NULL;
            pSigPend->sigp_q.sigq_next            = NULL;

            *pSigInfo = pSigPend->sigp_info;

            pSigPend->sigp_overruns           = pSigPend->sigp_active_overruns;
            pSigPend->sigp_active_overruns    = 0;

            /* If on queue, remove it */
            if (pSigPend->sigp_info.si_code == SI_QUEUE)
            {
                *(struct sigpend **) pSigPend = pSigQueueFreeHead;
                pSigQueueFreeHead = pSigPend;
            }
        }

        if (pSigTcb->sigt_qhead[signo].sigq_next ==
            &pSigTcb->sigt_qhead[signo])
        {
            pSigTcb->sigt_pending &= ~sigmsk;
        }
    }

    return signo;
}

/******************************************************************************
 * sigreturn - Return from signal handler
 *
 * RETURNS: N/A
 */

void sigreturn(
    struct sigcontext *scp
    )
{
    sigprocmask(SIG_SETMASK, &scp->sc_mask, 0);
    _sigCtxLoad(&scp->sc_regs);
}

/******************************************************************************
 * sigWrapper - Goto signal hanlder from kernel
 *
 * RETURNS: N/A
 */

LOCAL void sigWrapper(
    struct sigcontext *pSigContext
    )
{
    int                signo;
    struct sigaction  *pSigAction;
    void             (*handler)();

    /* Setup local variables */
    signo      = pSigContext->sc_info.si_signo;
    pSigAction = &(taskIdCurrent->pSignalInfo)->sigt_vec[signo];
    handler    = pSigAction->sa_handler;

    /* If reset handler option set */
    if (pSigAction->sa_flags & SA_RESETHAND)
    {
        signal(signo, SIG_DFL);
    }

    /* Run if not default type of signal handler */
    if ((handler != SIG_DFL) && (handler != SIG_IGN) && (handler != SIG_ERR))
    {
        /* Check if extra info should be sent */
        if (pSigAction->sa_flags & SA_SIGINFO)
        {
            (*handler)(
                signo,
                &pSigContext->sc_info,
                pSigContext
                );
        }
        else
        {
            (*handler)(
                signo,
                pSigContext->sc_info.si_value.sival_int,
                pSigContext
                );
        }
    }

    /* Return from signal handler */
    sigreturn(pSigContext);
}

/******************************************************************************
 * sigPendRun - Get a pending signal and run it (Asume we are in kernel mode)
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL sigPendRun(
    struct sigtcb *pSigTcb
    )
{
    BOOL              ret;
    sigset_t          sigSet;
    struct sigcontext sigContext;
    int               signo;

    /* Get Negation of blocked signals */
    sigSet = ~pSigTcb->sigt_blocked;

    /* Get pending signal */
    signo = sigPendGet(pSigTcb, &sigSet, &sigContext.sc_info);

    /* If pending signal */
    if (signo > 0)
    {
        sigContext.sc_onstack = 0;
        sigContext.sc_restart = 0;
        sigContext.sc_mask    = pSigTcb->sigt_blocked;
        pSigTcb->sigt_blocked != (pSigTcb->sigt_vec[signo].sa_mask |
                                  sigmask(signo));

        /* Exit kernel */
        vmxExit();

        /* Save regs */
        if (_sigCtxSave(&sigContext.sc_regs) == 0)
        {
            /* Set return value to 1 if sucessfull */
            _sigCtxRetValueSet(&sigContext.sc_regs, 1);

            /* Run signal handler */
            sigWrapper(&sigContext);
        }

        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

/******************************************************************************
 * sigprocmask - Examine, change signal mask
 *
 * RETURNS: OK or ERROR
 */

int sigprocmask(
    int             what,
    const sigset_t *set,
    sigset_t       *oldset
    )
{
    struct sigtcb *pSigTcb;
    int            status = OK;
    BOOL           done   = FALSE;

    /* Get signal info for current task */
    pSigTcb = sigTcbGet();
    if (pSigTcb == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Enter kernel */
        kernelState = TRUE;

        /* If oldset not null, store old signal mask */
        if (oldset != NULL)
        {
            *oldset = pSigTcb->sigt_blocked;
        }

        /* If set is non null */
        if (set != NULL)
        {
            /* Select method */
            switch (what)
            {
                /* Block signal, exit kernel and return ok */
                case SIG_BLOCK:
                    pSigTcb->sigt_blocked |= *set;
                    vmxExit();
                    done = TRUE;
                    break;

                /* Unblock signal and exit switch */
                case SIG_UNBLOCK:
                    pSigTcb->sigt_blocked &= ~*set;
                    break;

                /* Setmask and exit switch */
                case SIG_SETMASK:
                    pSigTcb->sigt_blocked = *set;
                    break;

                /* Unknown method, exit kernel and return error */
                default:
                    vmxExit();
                    errnoSet(EINVAL);
                    status = ERROR;
                    done = TRUE;
                    break;
            }
        }

        if (done != TRUE)
        {
            /* Check for pending signal and try to run it */
            if (sigPendRun(pSigTcb) == FALSE)
            {
                vmxExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * sigTimeoutRecalc - Calculate new timeout when functions restarts
 *
 * RETURNS: Timeout value
 */

int sigTimeoutRecalc(
    int timeout
    )
{
    return timeout;
}


/******************************************************************************
 * sigExcSend - Signal task exception
 *
 * RETURNS: N/A
 */

LOCAL void sigExcSend(
    int      signo,
    int      code,
    REG_SET *pRegSet
    )
{
    struct sigpend pendSignal;
    REG_SET *pStoreRegSet;

    /* Store current reg set */
    pStoreRegSet = taskIdCurrent->pExcRegSet;
    taskIdCurrent->pExcRegSet = NULL;

    /* Initialize pending signal */
    sigPendInit(&pendSignal);

    /* Initialize structure */
    pendSignal.sigp_info.si_signo           = signo;
    pendSignal.sigp_info.si_code            = SI_SYNC;
    pendSignal.sigp_info.si_value.sival_int = code;
    pendSignal.sigp_pregs                   = pStoreRegSet;

    /* Kill and destroy */
    sigPendKill(taskIdCurrent, &pendSignal);
    sigPendDestroy(&pendSignal);

    /* Update exception reg set in current task */
    taskIdCurrent->pExcRegSet = pStoreRegSet;
}

/******************************************************************************
 * sigExcKill - Signal exception
 *
 * RETURNS: N/A
 */

LOCAL void sigExcKill(
    int      type,
    int      code,
    REG_SET *pRegSet
    )
{
    struct sigfaulttable *pFaultTable;
    BOOL                  done = FALSE;

    /* For all in fault table */
    for (pFaultTable = &_sigfaulttable[0];
         !((pFaultTable->sigf_fault == 0) && (pFaultTable->sigf_signo == 0));
         pFaultTable++)
    {
        /* Signal found */
        if (pFaultTable->sigf_fault == type)
        {
            sigExcSend(pFaultTable->sigf_signo, code, pRegSet);
            done = TRUE;
            break;
        }

    }

    /* Not in fault table */
    if (done != TRUE)
    {
        if (pFaultTable->sigf_signo != 0)
        {
            sigExcSend(pFaultTable->sigf_signo, code, pRegSet);
        }
    }
}

/******************************************************************************
 * signal - Specify handler associated with signal
 *
 * RETURNS: Old signal handler
 */

sighandler_t signal(
    int          signo,
    sighandler_t handler
    )
{
    sighandler_t     ret;
    struct sigaction ins;
    struct sigaction outs;

    ins.sa_handler = handler;
    ins.sa_flags   = 0;
    sigemptyset(&ins.sa_mask);

    /* Return old signal handler */
    if (sigaction(signo, &ins, &outs) != OK)
    {
        ret = SIG_ERR;
    }
    else
    {
        ret = outs.sa_handler;
    }

    return ret;
}

/******************************************************************************
 * sigaction - Specify action associated with a signal
 *
 * RETURNS: OK or ERROR
 */

int sigaction(
    int               signo,
    struct sigaction *act,
    struct sigaction *oact
    )
{
    struct sigtcb    *pSigTcb;
    struct sigaction *pSigAction;
    struct sigpend   *pSigPend;
    struct sigq      *pSigQueue;
    int               status = OK;

    /* Check signo */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        /* Get signal tcb for current task */
        pSigTcb = sigTcbGet();
        if (pSigTcb == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Get current action */
            pSigAction = &pSigTcb->sigt_vec[signo];

            /* Store old if argument contains valid pointer */
            if (oact != NULL)
            {
                *oact = *pSigAction;
            }

            /* Set action if act is non null */
            if (act != NULL)
            {
                /* Enter kernel */
                kernelState = TRUE;

                /* Store action in local variable */
                *pSigAction = *act;

                /* If action is ignore */
                if (pSigAction->sa_handler == SIG_IGN)
                {
                    /* Delete signal from pendign and kill sets */
                    sigdelset(&pSigTcb->sigt_pending, signo);
                    sigdelset(&pSigTcb->sigt_kilsigs, signo);

                    /* Destroy queued signals */
                    pSigQueue = pSigTcb->sigt_qhead[signo].sigq_next;
                    while (pSigQueue != &pSigTcb->sigt_qhead[signo])
                    {
                        pSigPend = structbase(
                                       struct sigpend,
                                       sigp_q,
                                       pSigQueue
                                       );
                        pSigQueue = pSigQueue->sigq_next;
                        pSigPend->sigp_q.sigq_prev = NULL;
                        pSigPend->sigp_q.sigq_next = NULL;

                        /* Put free on queue */
                        if (pSigPend->sigp_info.si_code == SI_QUEUE)
                        {
                            *(struct sigpend **) pSigPend = pSigQueueFreeHead;
                            pSigQueueFreeHead = pSigPend;
                        }
                    }
                }

                /* Exit kernel */
                vmxExit();
            }
        }
    }

    return status;
}

/******************************************************************************
 * sigemptyset - Initialize empty signal set
 *
 * RETURNS: OK
 */

int sigemptyset(
    sigset_t *set
    )
{
    *set = 0;

    return OK;
}

/******************************************************************************
 * sigfillset - Initialize full signal set
 *
 * RETURNS: OK
 */

int sigfillset(
    sigset_t *set
    )
{
    *set = 0xffffffff;

    return OK;
}

/******************************************************************************
 * sigaddset - Add signal from signal set
 *
 * RETURNS: OK or ERROR
 */

int sigaddset(
    sigset_t *set,
    int       signo
    )
{
    int status;

    /* Check signo */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        *set |= sigmask(signo);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sigdelset - Delete signal from signal set
 *
 * RETURNS: OK or ERROR
 */

int sigdelset(
    sigset_t *set,
    int       signo
    )
{
    int status;

    /* Check signo */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        *set &= ~sigmask(signo);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sigismember - Check if signal is in set
 *
 * RETURNS: TRUE, FALSE or ERROR
 */

int sigismember(
    sigset_t *set,
    int       signo
    )
{
    int ret;

    /* Check signo */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        ret =  ERROR;
    }
    else
    {
        ret =  ((*set & sigmask(signo)) != 0) ? TRUE : FALSE;
    }

    return ret;
}

/******************************************************************************
 * sigpending - Get pending signals from set blocked from delivery
 *
 * RETURNS: OK or ERROR
 */

int sigpending(
    sigset_t *set
    )
{
    int            status;
    struct sigtcb *pSigTcb;

    /* Get signal tcb */
    pSigTcb = sigTcbGet();
    if (pSigTcb == NULL)
    {
        status = ERROR;
    }
    else
    {
        *set = pSigTcb->sigt_pending;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sigpsuspend - Suspend task until it signals delivery, with signal set arg
 *
 * RETURNS: ERROR
 */

int sigsuspend(
    sigset_t *set
    )
{
    Q_HEAD         qHead;
    sigset_t       oset;
    struct sigtcb *pSigTcb;

    /* Get signal tcb */
    pSigTcb = sigTcbGet();
    if (pSigTcb == NULL)
    {
        return ERROR;
    }

    /* Enter kenrnel */
    kernelState = TRUE;

    /* Save old signal set */
    oset = pSigTcb->sigt_blocked;
    pSigTcb->sigt_blocked = *set;

    /* If pending signals */
    if (sigPendRun(pSigTcb) == TRUE)
    {
        sigprocmask(SIG_SETMASK, &oset, NULL);
        errnoSet(EINTR);
        return ERROR;
    }

    /* Sleep until signal arrives */
    qInit(&qHead, qFifoClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (vmxPendQPut(&qHead, WAIT_FOREVER) != OK)
    {
        vmxExit();
        return ERROR;
    }

    /* Exit kernel */
    vmxExit();

    /* Restore old signal mask */
    sigprocmask(SIG_SETMASK, &oset, NULL);

    errnoSet(EINTR);
    return ERROR;
}

/******************************************************************************
 * pause - Suspend task until it signals delivery, without signal set arg
 *
 * RETURNS: ERROR
 */

int pause(
    void
    )
{
    Q_HEAD qHead;

    /* Initialize fifo queue */
    qInit(&qHead, qFifoClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    /* Enter kenrnel */
    kernelState = TRUE;

    /* Sleep until signal arrives */
    if (vmxPendQPut(&qHead, WAIT_FOREVER) != OK)
    {
        vmxExit();
        return ERROR;
    }

    /* Exit kernel */
    vmxExit();

    errnoSet(EINTR);
    return ERROR;
}

/******************************************************************************
 * kill - Send signal to task
 *
 * RETURNS: OK or ERROR
 */

int kill(
    int taskId,
    int signo
    )
{
    int    status;
    TCB_ID tcbId = (TCB_ID) taskId;

    /* Check signo */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        /* If in kernel or in interrupt and task is current */
        if ((kernelState == TRUE) ||
            ((INT_CONTEXT() == TRUE) && (taskIdCurrent == tcbId)))
        {
            /* Check task id */
            if (TASK_ID_VERIFY(tcbId) != OK)
            {
                errnoSet(EINVAL);
                status = ERROR;
            }
            else
            {
                /* Send kill exception */
                excJobAdd(
                    (VOIDFUNCPTR) kill,
                    tcbId,
                    (ARG) signo,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0
                    );
                status = OK;
            }
        }
        else
        {
            /* Enter kernel */
            kernelState = TRUE;

            /* Check task id */
            if (TASK_ID_VERIFY(tcbId) != OK)
            {
                vmxExit();
                errnoSet(EINVAL);
                status = ERROR;
            }
            else
            {
                /* Send kill signal */
                sigVmxKill(tcbId, signo);

                /* Exit kernel */
                vmxExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * raise - Send signal current task
 *
 * RETURNS: OK or ERROR
 */

int raise(
    int signo
    )
{
    return kill((int) taskIdCurrent, signo);
}

/******************************************************************************
 * sigqueue - Send a queue signal
 *
 * RETURNS: OK or ERROR
 */

int sigqueue(
    int                taskId,
    int                signo,
    const union sigval value
    )
{
    int            status;
    struct sigpend sigPend;
    TCB_ID         tcbId = (TCB_ID) taskId;

    /* If invalid signal number */
    if (!issig(signo))
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        /* If in kernel or in interrupt and task is current */
        if ((kernelState == TRUE) ||
            ((INT_CONTEXT() == TRUE) && (taskIdCurrent == tcbId)))
        {
            /* If invalid task id */
            if (TASK_ID_VERIFY(tcbId) != OK)
            {
                errnoSet(EINVAL);
                status = ERROR;
            }
            else if (pSigQueueFreeHead == NULL)
            {
                errnoSet(EAGAIN);
                status = ERROR;
            }
            else
            {
                /* Call again in exception task */
                excJobAdd(
                    (VOIDFUNCPTR) sigqueue,
                    (ARG) taskId,
                    (ARG) signo,
                    (ARG) value.sival_int,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0
                    );
                status = OK;
            }
        }
        else
        {
            /* Enter kernel */
            kernelState = TRUE;

            /* If invalid task id */
            if (TASK_ID_VERIFY(tcbId) != OK)
            {
                vmxExit();
                errnoSet(EINVAL);
                status = ERROR;
            }
            else if (pSigQueueFreeHead == NULL)
            {
                vmxExit();
                errnoSet(EAGAIN);
                status = ERROR;
            }
            else
            {
                /* Initialize pendig signal */
                sigPendInit(&sigPend);
                sigPend.sigp_info.si_signo = signo;
                sigPend.sigp_info.si_code  = SI_QUEUE;
                sigPend.sigp_info.si_value = value;

                /* Send signal tru general kill function */
                sigVmxPendKill((TCB_ID) taskId, &sigPend);

                /* Exit kernel */
                vmxExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * sigPendInit - Initialize queue signal
 *
 * RETURNS: N/A
 */

void sigPendInit(
    struct sigpend *pSigPend
    )
{
    pSigPend->sigp_q.sigq_prev = NULL;
    pSigPend->sigp_q.sigq_next = NULL;
    pSigPend->sigp_overruns = 0;
    pSigPend->sigp_pregs = NULL;
}

/******************************************************************************
 * sigPendDestroy - Destroy queue signal
 *
 * RETURNS: OK or ERROR
 */

STATUS sigPendDestroy(
    struct sigpend *pSigPend
    )
{
    STATUS status;

    /* Not callable inside interrupts */
    if (INT_RESTRICT() != OK)
    {
        status = ERROR;
    }
    else
    {
        pSigPend->sigp_overruns = 0;

        /* Enter kernel */
        kernelState = TRUE;

        /* If on queue */
        if (pSigPend->sigp_q.sigq_next != NULL)
        {
            if (pSigPend->sigp_q.sigq_next == pSigPend->sigp_q.sigq_prev)
            {
                pSigPend->sigp_tcb->sigt_pending &=
                    (pSigPend->sigp_tcb->sigt_kilsigs |
                     ~sigmask(pSigPend->sigp_info.si_signo));
            }

            pSigPend->sigp_q.sigq_next->sigq_prev = pSigPend->sigp_q.sigq_prev;
            pSigPend->sigp_q.sigq_prev->sigq_next = pSigPend->sigp_q.sigq_next;
            pSigPend->sigp_q.sigq_prev            = NULL;
            pSigPend->sigp_q.sigq_next            = NULL;
        }

        /* Exit kernel */
        vmxExit();
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sigPendKill - Send a queued kill signal
 *
 * RETURNS: OK or ERROR
 */

STATUS sigPendKill(
    TCB_ID          tcbId,
    struct sigpend *pSigPend
    )
{
    STATUS status;

    /* Verify task id */
    if (TASK_ID_VERIFY(tcbId) != OK)
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        /* If in kernel or in interrupt and tcbId in the running task */
        if ((kernelState == TRUE) ||
            ((INT_CONTEXT() == TRUE) && (taskIdCurrent == tcbId)))
        {
            /* Add job to exception task */
            excJobAdd(
                (VOIDFUNCPTR) sigPendKill,
                (ARG) tcbId,
                (ARG) pSigPend,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );
            status = OK;
        }
        else
        {
            /* Enter kernel */
            kernelState = TRUE;

            /* Signal task */
            sigVmxPendKill(tcbId, pSigPend);

            /* Exit kernel */
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * sigVmxRestart - Test state of task for signal sending
 *
 * RETURNS: Delay value
 */

LOCAL int sigVmxRestart(
    TCB_ID tcbId
    )
{
    int delayValue = WAIT_FOREVER;

    /* If task is pending of delayed */
    if ((tcbId->status & (TASK_PEND | TASK_DELAY)) != 0)
    {
        /* If task is delayed */
        if ((tcbId->status & TASK_DELAY) != 0)
        {
            /* Set return value to restart */
            taskRetValueSet(tcbId, SIG_RESTART);

            /* Remove from delay queue */
            tcbId->status &= ~TASK_DELAY;
            Q_REMOVE(&tickQHead, &tcbId->tickNode);

            /* Get delay value */
            delayValue = Q_KEY(&tickQHead, &tcbId->tickNode, 1);
        }

        /* If task is pending */
        if ((tcbId->status & TASK_PEND) != 0)
        {
            /* Unpend task */
            tcbId->status &= ~TASK_PEND;
            if (Q_REMOVE(tcbId->pPendQ, tcbId) != OK)
            {
                taskRetValueSet(tcbId, ERROR);
            }
        }

        /* If task is ready now */
        if (tcbId->status == TASK_READY)
        {
            Q_PUT(&readyQHead, tcbId, tcbId->priority);
        }
    }

    return delayValue;
}

/******************************************************************************
 * sigVmxPendKill - Send kill kernel signal
 *
 * RETURNS: N/A
 */

LOCAL void sigVmxPendKill(
    TCB_ID          tcbId,
    struct sigpend *pSigPend
    )
{
    int signo;
    struct sigtcb     *pSigTcb;
    struct sigq       *qHead;
    struct sigpend    *pTmpPend;
    struct sigcontext  sigContext;
    struct sigcontext *pSigContext;
    long               sigmsk;
    int                args[2];
    sighandler_t       handler;

    /* Get locals */
    signo = pSigPend->sigp_info.si_signo;
    pSigTcb = tcbId->pSignalInfo;

    /* Ignore tasks with zero signal info */
    if (pSigTcb == NULL)
    {
        return;
    }

    /* If task is waiting for signal, wake it up */
    if ((pSigTcb->sigt_wait != NULL) &&
        (Q_FIRST(&pSigTcb->sigt_wait->sigw_wait) != NULL))
    {
        pSigTcb->sigt_wait->sigw_info = pSigPend->sigp_info;

        /* Get task from pending queue */
        vmxPendQGet(&pSigTcb->sigt_wait->sigw_wait);

        /* Make wait end and return */
        pSigTcb->sigt_wait = NULL;
        return;
    }

    /* Get signal handler function and mask */
    handler = pSigTcb->sigt_vec[signo].sa_handler;
    sigmsk = sigmask(signo);

    /* Ignore */
    if (handler == SIG_IGN)
    {
        return;
    }

    /* Default */
    if (handler == SIG_DFL)
    {
        return;
    }

    /* Get signal mask */
    sigmsk = sigmask(signo);

    /* If signal is among blocked */
    if (pSigTcb->sigt_blocked & sigmsk)
    {
        /* If it is a kill signal */
        if (pSigPend->sigp_info.si_code == SI_KILL)
        {
            pSigTcb->sigt_kilsigs |= sigmsk;
            pSigTcb->sigt_pending |= sigmsk;
        }
        else if (pSigPend->sigp_q.sigq_next != NULL)
        {
            pSigPend->sigp_active_overruns++;
        }
        else
        {
            qHead = &pSigTcb->sigt_qhead[signo];

            /* Allocate buffer */
            if (pSigPend->sigp_info.si_code == SI_QUEUE)
            {
                pTmpPend = pSigQueueFreeHead;
                pSigQueueFreeHead = *(struct sigpend **) pSigQueueFreeHead;
                *pTmpPend = *pSigPend;
                pSigPend = pTmpPend;
            }

            pSigPend->sigp_q.sigq_prev            = qHead->sigq_prev;
            pSigPend->sigp_q.sigq_next            = qHead;
            pSigPend->sigp_q.sigq_prev->sigq_next = &pSigPend->sigp_q;

            qHead->sigq_prev = &pSigPend->sigp_q;

            pSigTcb->sigt_pending |= sigmsk;

            pSigPend->sigp_tcb = pSigTcb;
        }

        return;
    }

    /* Forced develivery of signal */

    /* If current task */
    if (tcbId == taskIdCurrent)
    {
        sigContext.sc_onstack = 0;
        sigContext.sc_restart = 0;
        sigContext.sc_mask    = pSigTcb->sigt_blocked;

        pSigTcb->sigt_blocked |= (pSigTcb->sigt_vec[signo].sa_mask | sigmsk);

        sigContext.sc_info  = pSigPend->sigp_info;
        sigContext.sc_pregs = pSigPend->sigp_pregs;

        /* Exit kernel */
        vmxExit();

        if (_sigCtxSave(&sigContext.sc_regs) == 0)
        {
            _sigCtxRetValueSet(&sigContext.sc_regs, 1);
            sigWrapper(&sigContext);
        }

        /* Enter kernel */
        kernelState = TRUE;
    }
    else
    {
#if (_STACK_DIR == _STACK_GROWS_DOWN)
        /* Get stack storage for signal context */
        pSigContext = (struct sigcontext *) MEM_ROUND_DOWN(
            ((struct sigcontext *) _sigCtxStackEnd(&tcbId->regs)) - 1);
#else /* _STACK_GROWS_UP */
        /* Get stack storage for signal context */
        pSigContext = (struct sigcontext *) MEM_ROUND_UP(
                                                _sigCtxStackEnd(&tcbId->regs)
                                                );
#endif /* _STACK_DIR */

        /* Arguments count to store on stack */
        args[0] = 1;
        args[1] = (int) pSigContext;

        /* Wake up task */
        pSigTcb->sigt_wait = NULL;

        pSigContext->sc_onstack = 0;
        pSigContext->sc_restart = sigVmxRestart(tcbId);
        pSigContext->sc_mask    = pSigTcb->sigt_blocked;

        pSigTcb->sigt_blocked |= (pSigTcb->sigt_vec[signo].sa_mask | sigmsk);

        pSigContext->sc_info  = pSigPend->sigp_info;
        pSigContext->sc_regs  = tcbId->regs;
        pSigContext->sc_pregs = pSigPend->sigp_pregs;

#if (_STACK_DIR == _STACK_GROWS_DOWN)
        /* Setup task stack with signal context */
        _sigCtxSetup(
            &tcbId->regs,
            (void *) STACK_ROUND_DOWN(pSigContext),
            sigWrapper,
            args
            );
#else /* _STACK_DOWNS_UP */
        /* Setup task stack with signal context */
        _sigCtxSetup(
            &tcbId->regs,
            (void *) STACK_ROUND_UP(pSigContext + 1),
            sigWrapper,
            args
            );
#endif /* _STACK_DIR */
    }
}

/******************************************************************************
 * sigVmxKill - Send signal
 *
 * RETURNS: N/A
 */

LOCAL void sigVmxKill(
    TCB_ID tcbId,
    int    signo
    )
{
    struct sigpend sigPend;

    sigPend.sigp_info.si_signo           = signo;
    sigPend.sigp_info.si_code            = SI_KILL;
    sigPend.sigp_info.si_value.sival_int = 0;

    sigVmxPendKill(tcbId, &sigPend);
}

