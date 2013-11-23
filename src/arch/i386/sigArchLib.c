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

/* sigArchLib.c - Signal processing library */

#include <string.h>
#include <vmx.h>
#include <os/private/sigLibP.h>
#include <arch/regs.h>
#include <arch/vmxArchLib.h>

/* Exception signals */
struct sigfaulttable _sigfaulttable[] =
{
    {  0, SIGFPE },
    {  1, SIGEMT },
    {  2, SIGILL },
    {  3, SIGEMT },
    {  4, SIGILL },
    {  5, SIGILL },
    {  6, SIGILL },
    {  7, SIGFPE },
    {  8, SIGILL },
    {  9, SIGFPE },
    { 10, SIGILL },
    { 11, SIGBUS },
    { 12, SIGBUS },
    { 13, SIGILL },
    { 14, SIGBUS },
    { 15, SIGILL },
    { 16, SIGFPE },
    { 17, SIGBUS },
    {  0,      0 }
};

/****************************************************************************
 * _sigCtxRetValueSet - Set return value of context
 *
 * RETURNS: N/A
 */

void _sigCtxRetValueSet(
    REG_SET *pRegSet,
    int      val
    )
{
    pRegSet->eax = val;
}

/*****************************************************************************
 * _sigCtxStackEnd - Get end of stack for context
 *
 * RETURNS: Pointer to stack end
 */

void* _sigCtxStackEnd(
    const REG_SET *pRegSet
    )
{
    return (void *) (pRegSet->esp - 12);
}

/*****************************************************************************
 * _sigCtxSetup - Setup context
 *
 * RETURNS: N/A
 */

void _sigCtxSetup(
    REG_SET *pRegSet,
    void    *pStackBase,
    void   (*taskEntry)(),
    int     *pArgs
    )
{
    int i;
    union
    {
        void   *pv;
        int    *pi;
        void (**ppfv)();
        int     i;
    } pu;

    /* Clear regset */
    memset(pRegSet, 0, sizeof(*pRegSet));

    /* Setup stack */
    pu.pv = (void *) ((int) pStackBase & ~3);
    for (i = pArgs[0]; i > 0; --i)
    {
        *--pu.pi = pArgs[i];
    }

    /* Setup entry */
    *--pu.ppfv      = vmxTaskEntry;
    pRegSet->esp    = pu.i;
    pRegSet->pc     = (INSTR *) taskEntry;
    pRegSet->eflags = EFLAGS_BRANDNEW;
}

