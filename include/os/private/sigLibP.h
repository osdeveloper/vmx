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

/* sigLibP.h - Private signals */

#ifndef _sigLibP_h
#define _sigLibP_h

#include <vmx.h>
#include <arch/regs.h>
#include <util/qLib.h>
#include <vmx/taskLib.h>
#include <signal.h>

#define SIG_RESTART             1

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct sigq
{
    struct sigq           *sigq_next;             /* Next node in queue */
    struct sigq           *sigq_prev;             /* Previous node in queue */
};

struct sigstack
{
    int                   ss_onstack;             /* On stack */
    void                  *ss_sp;                 /* Stack pointer */
};

struct sigwait
{
    sigset_t              sigw_set;               /* Signal set */
    struct siginfo        sigw_info;              /* Info */
    Q_HEAD                sigw_wait;              /* Wait queue */
};

struct sigtcb
{
    struct sigaction       sigt_vec[_NSIGS + 1];
    struct sigq            sigt_qhead[_NSIGS + 1];
    struct sigstack        sigt_stack;
    struct sigwait        *sigt_wait;
    sigset_t               sigt_kilsigs;
    sigset_t               sigt_blocked;
    sigset_t               sigt_pending;
    sigset_t               sigt_ignored;
};

struct sigpend
{
    struct sigq            sigp_q;
    struct siginfo         sigp_info;
    long                   sigp_overruns;
    long                   sigp_active_overruns;
    REG_SET               *sigp_pregs;
    struct sigtcb         *sigp_tcb;
};

struct sigfaulttable
{
    int                   sigf_fault;
    int                   sigf_signo;
};

struct sigcontext
{
    int                   sc_onstack;
    int                   sc_restart;
    sigset_t              sc_mask;
    struct siginfo        sc_info;
    REG_SET               sc_regs;
    REG_SET               *sc_pregs;
};

/* Functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _sigLibP_h */

