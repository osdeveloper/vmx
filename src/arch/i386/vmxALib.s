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

/* vmxALib.s - System dependent assembler parts of kernel */

#define _ASMLANGUAGE
#include <vmx.h>
#include <arch/asm.h>
#include <arch/regs.h>
#include <arch/taskArchLib.h>
#include <arch/esf.h>

        .data
        /* Imports */
        .globl  GDATA(taskIdCurrent)
        .globl  GDATA(readyQHead)
        .globl  GDATA(workQIsEmpty)
        .globl  GDATA(taskSwitchHooks)
        .globl  GDATA(taskSwapHooks)
        .globl  GDATA(sysCsSuper)
        .globl  GDATA(errno)
        .globl  GDATA(kernelState)
        .globl  GDATA(kernelIsIdle)
        .globl  GDATA(intCnt)
        .globl  GTEXT(workQDoWork)
        .globl  GTEXT(taskExit)

        /* Internals */
        .globl  GTEXT(intEnt)
        .globl  GTEXT(intExit)
        .globl  GTEXT(vmxExit)
        .globl  GTEXT(vmxTaskReschedule)
        .globl  GTEXT(vmxTaskContextLoad)
        .globl  GTEXT(vmxTaskEntry)
        .globl  GTEXT(intStackSet)
        .globl  GTEXT(intStackEnable)

FUNC_LABEL(intNest)
        .long   0x00000000

FUNC_LABEL(kernelIsIdle)
        .long   0x00000000

FUNC_LABEL(excStackPtr)
        .long   0x00000000

FUNC_LABEL(excStackEnabled)
        .long   0x00000000

FUNC_LABEL(excStackCnt)
        .long   0x00000000

        .text
        .balign 16, 0x90

/******************************************************************************
 * intEnt - Called when an interrupt occurs
 *
 * Stack looks like ...
 *  ...
 *  EFLAGS   <- ESF + 0x08                  : ESP + 0x10
 *  CS       <- ESF + 0x04                  : ESP + 0x0C
 *  EIP      <- ESF + 0x00                  : ESP + 0x08
 *  EIP      <- excIntStub() return address : ESP + 0x04
 *  EIP      <- intEnt() return address     : ESP + 0x00
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(intEnt)
        cli                                     /* Disable interrupts */
        pushl   (%esp)                          /* Duplicate return address */
        pushl   %eax                            /* Save EAX */

        /*
         * Store errno on stack.  Over-write the original intEnt()
         * return address.  When we return from this routine, we will
         * use the duplicated return address.
         */

        movl    FUNC(errno), %eax
        movl    %eax, 8(%esp)

        /* Increase interrupt counter */
        incl    FUNC(intNest)
        incl    FUNC(intCnt)

        incl    FUNC(excStackCnt)

        /* Switch to the new stack if needed */

        cmpl    $FALSE, FUNC(excStackEnabled)
        je      intEntDone

        cmpl    $1, FUNC(excStackCnt)
        jne     intEntDone

        /*
         * EFLAGS  <- ESF + 0x08                    : ESP + 0x18
         * CS      <- ESF + 0x04                    : ESP + 0x14
         * EIP     <- ESF + 0x00                    : ESP + 0x10
         * EIP     <- excIntStub() return address   : ESP + 0x0C
         * errno   <-                               : ESP + 0x08
         * EIP     <- intEnt() return address       : ESP + 0x04
         * EAX                                      : ESP + 0x00
         */

        leal    0x10(%esp), %eax

        pushl   %edi                            /* Save registers used */
        pushl   %esi                            /* to copy the stack.  */
        pushl   %ecx

        std                                     /* Set direction flag. */

        movl    $10, %ecx
        leal    0x24(%esp), %esi
        movl    FUNC(excStackPtr), %edi
        stosl                                   /* Save old stack pointer. */
        rep     movsl                           /* Copy to new stack. */

        leal    4(%edi), %esp                   /* Adjustment factor. */

        popl    %ecx
        popl    %esi
        popl    %edi

intEntDone:

        popl    %eax                            /* Restore EAX */

        pushl   0x14(%esp)                      /* Restore interrupts  */
        popfl                                   /* and direction flag. */

        /* Return */
        ret

/******************************************************************************
 * intExit - Routine only brached to when interrupt ends
 *
 * Stack looks like ...
 *   ...
 *  (old ESP     <- possibly                 : ESP + 0x14)
 *   EFLAGS      <- ESF + 0x08               : ESP + 0x10
 *   CS          <- ESF + 0x04               : ESP + 0x0C
 *   EIP         <- ESF + 0x00               : ESP + 0x08
 *   EIP         <- excIntStub() ret addr    : ESP + 0x04
 *   errno       <-                          : ESP + 0x00
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(intExit)

        /* Restore errno from stack */
        popl    FUNC(errno)
        addl    $4, %esp                        /* Skip excIntStub ret addr */

        /* Decrease interrupt counter */
        cli                                     /* Disable interrupts */
        decl    FUNC(intNest)
        decl    FUNC(intCnt)

        cmpl    $FALSE, FUNC(excStackEnabled)
        je      intExitStackDone

        decl    FUNC(excStackCnt)
        jnz     intExitStackDone

        /*
         * <excStackCnt> is zero.  Switch back to the task stack.
         * Note that old ESP points to the base of the old ESF.
         */

        addl    $0xc, %esp
        popl    %esp

intExitStackDone:
        pushl   %eax                            /* Save EAX */

        /* If CS on stack is sysCsInt we are in an interrupt */
        movl    ESF0_CS+4(%esp), %eax
        cmpw    FUNC(sysCsInt), %ax
        je      intRet

        /* Check if in kernel mode */
        cmpl    $FALSE, FUNC(kernelState)
        jne     intRet

        cmpl    $FALSE, FUNC(kernelIsIdle)      /* Branch if interrupt went */
        jne     intExitFromIdle                 /* off during the idle loop */

        /* If not in kernel mode, rescheduling might be needed */
        movl    FUNC(taskIdCurrent), %eax
        cmpl    FUNC(readyQHead), %eax
        je      intRet

        /* Check if next task is ready */
        cmpl    $0, TASK_TCB_LOCK_COUNT(%eax)   /* Is task preemption on */
        je      FUNC(intContextSave)
        cmpl    $0, TASK_TCB_STATUS(%eax)
        jne     FUNC(intContextSave)

intRet:
        popl    %eax                            /* Restore EAX */

        /* Return from interrupt */
        iret

/******************************************************************************
 * intExitFromIdle - exit the interrupt (kernel was idle)
 *
 * Stack looks like ...
 *   ...
 *   EFLAGS      <- ESF + 0x08               : ESP + 0x0C
 *   CS          <- ESF + 0x04               : ESP + 0x08
 *   EIP         <- ESF + 0x00               : ESP + 0x04
 *   EAX         <-                          : ESP + 0x00
 *
 * Interrupts are locked.  No tasks were ready to run when we entered the
 * interrupt.  Now that we are leaving the interrupt, one or more tasks
 * might be ready.  Thus we will return from the interrupt to
 * vmxTaskReschedule().
 *
 * It is critical that the stack frame be reset.  If it is not, then our stack
 * usage will continue to grow and eventually overflow, which of course is
 * very bad.  Fortunately resetting it is easy--use the stack pointer stored in
 * the interrupted idling task.  (Currently, when the kernel is idle, it uses
 * the stack of the last running task.)  Also, since we will be iret'ing into
 * vmxTaskReschedule(), <kernelState> must be set.
 *
 * Do we need to restore our modified registers before we iret?  No.  Also,
 * before we iret, vmxTaskReschedule() expects EDX to be FUNC(taskIdCurrent).
 */ 

        .balign 16, 0x90

FUNC_LABEL(intExitFromIdle)
        movl    $TRUE, FUNC(kernelState)
        movl    FUNC(taskIdCurrent), %edx
        movl    TASK_TCB_ESP(%edx), %esp

        pushfl
        bts $9, (%esp)
        pushl   FUNC(sysCsSuper)
        pushl   $FUNC(vmxTaskReschedule)

        iret

/******************************************************************************
 * intContextSave - Save task context
 *
 * Stack looks like ...
 *   ...
 *   EFLAGS      <- ESF + 0x08     : ESP + 0x0C
 *   CS          <- ESF + 0x04     : ESP + 0x08
 *   EIP         <- ESF + 0x00     : ESP + 0x04
 *   EAX         <-                : ESP + 0x00
 *
 * Interrupts are locked.
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(intContextSave)
        /* Save task context */
        movl    $TRUE, FUNC(kernelState)        /* Move to kernel mode */
        movl    FUNC(taskIdCurrent), %eax
        popl    TASK_TCB_EAX(%eax)              /* EAX to TCB */
        popl    TASK_TCB_PC(%eax)               /* PC to TCB */
        leal    4(%esp), %esp                   /* Ignore CS */
        popl    TASK_TCB_EFLAGS(%eax)           /* EFLAGS to TCB */
        sti                                     /* Re-enable interrupts */

        /* All items have been popped off the stack. */

        /* Now save all registers */
        movl    %edx, TASK_TCB_EDX(%eax)        /* EDX to TCB */
        movl    %ecx, TASK_TCB_ECX(%eax)        /* ECX to TCB */
        movl    %ebx, TASK_TCB_EBX(%eax)        /* EBX to TCB */
        movl    %esi, TASK_TCB_ESI(%eax)        /* ESI to TCB */
        movl    %edi, TASK_TCB_EDI(%eax)        /* EDI to TCB */
        movl    %ebp, TASK_TCB_EBP(%eax)        /* EBP to TCB */
        movl    %esp, TASK_TCB_ESP(%eax)        /* ESP to TCB */

        /* Save errno */
        movl    FUNC(errno), %edx
        movl    %edx, TASK_TCB_ERRNO(%eax)
        movl    %eax, %edx                      /* Active task in EDX */

        /* Jump to vmxTaskReschedule() function here */

        pushfl                                  /* Re-create ESF to leave */
        pushl   FUNC(sysCsSuper)                /* interrupt via 'iret'.  */
        pushl   $FUNC(vmxTaskReschedule)

        iret

/******************************************************************************
 * checkTaskReady - Check if taskIdCurrent is ready to run
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(checkTaskReady)
        cmpl    $0, TASK_TCB_STATUS(%edx)       /* Check status flag */
        jne     FUNC(saveTaskContext)

        /* FALL THRU TO CHECK KERNEL QUEUE */

/******************************************************************************
 * workQCheck - Check if kernel work need to be done
 *
 * RETURNS: OK
 */

FUNC_LABEL(workQCheck)

        cli                                     /* Lock interrupts */
        cmpl    $FALSE, FUNC(workQIsEmpty)      /* Check for kernel work */
        je      FUNC(workQWorkPreSave)          /* Work to to */

        movl    $FALSE, FUNC(kernelState)       /* Release mutex */
        sti                                     /* Unlock interrupts */
        movl    $OK, %eax                       /* Return OK */

        /* Return */
        ret

/******************************************************************************
 * workQWorkPreSave -
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(workQWorkPreSave)

        sti                                     /* Unlock interrputs */
        call    FUNC(workQDoWork)               /* Do kernel work */
        jmp     FUNC(checkTaskSwitch)           /* Check task switch */


/******************************************************************************
 * exitFromIntDoWork -
 *
 * This code is only called from 'exitFromInt'.  If the kernel work 
 * queue was not empty, it empties it, and then falls through back into
 * 'exitFromInt'.
 *
 * Notable states:  <kernelState> is TRUE, EFLAGS is pushed
 */

        .balign 16, 0x90

FUNC_LABEL (exitFromIntDoWork)
        popfl                                   /* Restore interrupts. */
        call    FUNC(workQDoWork)               /* Do kernel work */

        /* FALL THRU TO 'exitFromInt' */

/******************************************************************************
 * exitFromInt -
 * 
 * This code is used to do the vmxExit() work when it is called from within
 * an interrupt.  It does not perform any re-scheduling, as that will be done
 * when the interrupt exits.  If there is any outstanding kernel work to do, it
 * must be done here.  This is safe to do as interrupts that are serviced while
 * <kernelState> is FALSE can make calls to vmxExit().  Interrupts that are
 * serviced while <kernelState> is TRUE can not--their work is added to the
 * kernel work queue.
 *
 * RETURNS: OK
 */

FUNC_LABEL(exitFromInt)
        pushfl                                 /* Save interrupts. */
        cli                                    /* Lock interrupts. */

        cmpl    $FALSE, FUNC(workQIsEmpty)     /* Check for kernel work. */
        je      FUNC(exitFromIntDoWork)     /* Work to do. */

        /* Kernel work queue is empty. */

        movl    $FALSE, FUNC(kernelState)       /* Leaving kernel state. */
        popfl                                   /* Restore interrupts.   */
        movl    $OK, %eax

        ret

/******************************************************************************
 * vmxExit - Exit kernel mode
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(vmxExit)

        /*
         * This check for taskIdCurrent == 0 is temporary.  The startup code
         * will need to be checked to make sure that we never call vmxExit()
         * before the root task has been initialized.  This prevents any
         * possible rescheduling including flipping to the idle loop.
         */

        cmpl    $0, FUNC(taskIdCurrent)
        je      FUNC(noTasksInSystem)

        cmpl    $0, FUNC(intCnt)                /* intCnt=0 from task */
        jne     FUNC(exitFromInt)               /* If not task go away */

        /* FALL THRU TO CHECK TASK SWITCH */

/******************************************************************************
 * checkTaskSwitch - Check if another task should run instead
 *
 * RETURNS: N/A
 */

FUNC_LABEL(checkTaskSwitch)

        movl    FUNC(taskIdCurrent), %edx
        cmpl    FUNC(readyQHead), %edx          /* Compare task to queue */
        je      FUNC(workQCheck)                /* If the same leave */

        /* If preemption is allowed, check if task is ready */
        cmpl    $0, TASK_TCB_LOCK_COUNT(%edx)
        jne     FUNC(checkTaskReady)

        /* FALL THRU TO SAVE CONTEXT */

/******************************************************************************
 * saveTaskContext - Save task context
 *
 * RETURNS: N/A
 */

FUNC_LABEL(saveTaskContext)

        movl    (%esp), %eax                    /* Save return address to PC */
        movl    %eax, TASK_TCB_PC(%edx)
        pushfl                                  /* Save flags */
        popl    TASK_TCB_EFLAGS(%edx)

        /* Save regs */
        movl    %ebx, TASK_TCB_EBX(%edx)
        movl    %esi, TASK_TCB_ESI(%edx)
        movl    %edi, TASK_TCB_EDI(%edx)
        movl    %ebp, TASK_TCB_EBP(%edx)
        movl    %esp, TASK_TCB_ESP(%edx)
        movl    $0, TASK_TCB_EAX(%edx)          /* Clear eax and return */
        addl    $4, TASK_TCB_ESP(%edx)          /* Fix ret addr */

        pushl   FUNC(errno)                     /* Save error code */
        popl    TASK_TCB_ERRNO(%edx)

#ifdef schedLib_PORTABLE
        call    (vmxTaskReschedule)
#else

        /* FALL THRU TO TASK RESCHEDULE */

/******************************************************************************
 * vmxTaskReschedule - Reschedule running task
 *
 * On entry ...
 *   EDX = FUNC(taskIdCurrent)
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(vmxTaskReschedule)
        movl    $FALSE, FUNC(kernelIsIdle)

        /* Load active task */
        movl    FUNC(readyQHead), %eax
        cmpl    $0, %eax
        je      FUNC(noTaskReady)

        /* FALL THRU TO SWITCH TASKS */

/******************************************************************************
 * switchTasks - Do task switch
 *
 * RETURNS: N/A
 */

FUNC_LABEL(switchTasks)
        
        /* Store new task as current */
        movl    %eax, FUNC(taskIdCurrent)

        /* Get swap masks */
        movw    TASK_TCB_SWAP_IN(%eax), %bx
        orw     TASK_TCB_SWAP_OUT(%edx), %bx

        /* Swap requested */
        jne     FUNC(taskHookSwap)

        /* Check if any global hooks */
        cmpl    $0, FUNC(taskSwitchHooks)

        /* Run hooks */
        jne     FUNC(taskHookSwitch)

        /* FALL TRU TO DISPATCH */

/******************************************************************************
 * dispatch - Store task context for task switch
 *
 * RETURNS: N/A
 */

FUNC_LABEL(dispatch)

        /* Switch to task stored in EAX */
        movl    TASK_TCB_ERRNO(%eax), %ecx      /* Restore errno */
        movl    %ecx, FUNC(errno)
        movl    TASK_TCB_ESP(%eax), %esp        /* Restore stack */
        pushl   TASK_TCB_EFLAGS(%eax)           /* Flags on stack */
        pushl   FUNC(sysCsSuper)                /* Segment on stack */
        pushl   TASK_TCB_PC(%eax)               /* PC on stack */

        /* Restore registers */
        movl    TASK_TCB_EDX(%eax), %edx        /* TCB to EDX */
        movl    TASK_TCB_ECX(%eax), %ecx        /* TCB to ECX */
        movl    TASK_TCB_EBX(%eax), %ebx        /* TCB to EBX */
        movl    TASK_TCB_ESI(%eax), %esi        /* TCB to ESI */
        movl    TASK_TCB_EDI(%eax), %edi        /* TCB to EDI */
        movl    TASK_TCB_EBP(%eax), %ebp        /* TCB to EBP */
        movl    TASK_TCB_EAX(%eax), %eax        /* TCB to EAX */

        /* Now exit kernel mode */
        cli                                     /* Disable interrupts */
        cmpl    $FALSE, FUNC(workQIsEmpty)      /* Check for work */
        je      FUNC(workQWorkUnlock)

        movl    $FALSE, FUNC(kernelState)       /* Exit kern mode */

        /* Return from interrupt */
        iret

/******************************************************************************
 * noTaskReady -
 *
 * Interrupts are unlocked.  <kernelState> is set.
 */

        .balign 16, 0x90

FUNC_LABEL(noTaskReady)
        cli
        cmpl    $FALSE, FUNC(workQIsEmpty)      /* Check for work */
        je      FUNC(workQWorkUnlock)

        movl    $FALSE, FUNC(kernelState)
        movl    $TRUE, FUNC(kernelIsIdle)

        sti                                     /* Re-enable interrupts */

idleLoop:
        /*
         * Wait for an interrupt.  An interrupt will cause us to jump to
         * FUNC(vmxTaskReschedule) on its exit.  As long as the 'hlt'
         * instruction is used, the following 'jmp' is pointless.
         */

/*      hlt */
        jmp     idleLoop

/******************************************************************************
 * noTasksInSystem -
 *
 * RETURNS: N/A
 */

FUNC_LABEL(noTasksInSystem)
        ret

/******************************************************************************
 * workQWorkUnlock - Unlock interrputs and do fall tru to do work
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(workQWorkUnlock)

        sti                                             /* Unlock interrupts */

        /* FALL THRU TO KERNEL WORK */

/******************************************************************************
 * workQWork - Do kernel work
 *
 * RETURNS: N/A
 */

FUNC_LABEL(workQWork)

        call    FUNC(workQDoWork)                       /* Do work */

        /* Check task queue if resheduling is needed */
        movl    FUNC(taskIdCurrent), %edx
        movl    FUNC(readyQHead), %eax
        cmpl    $0, %eax                                /* Any tasks ready? */
        je      FUNC(noTaskReady)                       /*    If not--jump. */
        cmpl    %edx, %eax                              /* Compare tasks */
        je      FUNC(dispatch)                          /* the same? */
        jmp     FUNC(switchTasks)                       /* else switch */

#endif /* schedLib_PORTABLE */

/******************************************************************************
 * vmxTaskContextLoad - Load task context
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(vmxTaskContextLoad)

        /* Load current tcb */
        movl    FUNC(readyQHead), %eax
        movl    %eax, FUNC(taskIdCurrent)
        movl    TASK_TCB_ERRNO(%eax), %ecx
        movl    %ecx, FUNC(errno)
        movl    TASK_TCB_ESP(%eax), %esp

        /* Setup stack */
        pushl   TASK_TCB_EFLAGS(%eax)
        pushl   FUNC(sysCsSuper)
        pushl   TASK_TCB_PC(%eax)

        /* Restore regs */
        movl    TASK_TCB_EDX(%eax), %edx
        movl    TASK_TCB_ECX(%eax), %ecx
        movl    TASK_TCB_EBX(%eax), %ebx
        movl    TASK_TCB_ESI(%eax), %esi
        movl    TASK_TCB_EDI(%eax), %edi
        movl    TASK_TCB_EBP(%eax), %ebp
        movl    TASK_TCB_EAX(%eax), %eax

        iret

/******************************************************************************
 * vmxTaskEntry - Task entry point
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(vmxTaskEntry)
        xorl    %ebp, %ebp

        /* Load current tcb */
        movl    FUNC(taskIdCurrent), %eax

        /* Get entry point in tcb */
        movl    TASK_TCB_ENTRY(%eax), %eax

        /* Call entry point */
        call    *%eax

        /* Pop argument to task */
        addl    $40, %esp

        /* Pass result to exit func */
        pushl   %eax
        call    FUNC(taskExit)

#ifndef schedLib_PORTABLE

/******************************************************************************
 * taskHookSwap - Run task swap hooks
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(taskHookSwap)

        /* Push args */
        pushl   %eax
        pushl   %edx

        leal    FUNC(taskSwapHooks), %edi       /* Load swap hooks */
        movl    $-4, %esi
        jmp     swapLoop1

        .balign 16, 0x90

swapLoop0:
        movl    (%esi, %edi, 1), %ecx           /* call r3 + 4 * n */
        call    *%ecx

swapLoop1:
        addl    $4, %esi                        /* Move to next hook */
        shlw    $1, %bx                         /* Shift mask pattern left */
        jc      swapLoop0                       /* Loop if carry flag set */

        jne     swapLoop1                       /* Else loop to next hook */

        /* Restore task pointer and move on */
        movl    FUNC(taskIdCurrent), %eax

        /* Now check if any global switch hooks */
        cmpl    $0, FUNC(taskSwitchHooks)
        je      FUNC(dispatch)

        jmp     FUNC(switchFromSwap)

/******************************************************************************
 * taskHookSwitch - Run task switch hooks
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(taskHookSwitch)

        /* Push args */
        pushl   %eax
        pushl   %edx

FUNC_LABEL(switchFromSwap)
        leal    FUNC(taskSwitchHooks), %edi     /* Get switch table */
        movl    (%edi), %esi

switchLoop:
        call    *%esi                           /* Call hook */
        addl    $4, %edi                        /* Go to next routine */
        movl    (%edi), %esi
        cmpl    $0, %esi                        /* Check if installed */
        jne     switchLoop

        movl    FUNC(taskIdCurrent), %eax       /* Restore task id */

        jmp     FUNC(dispatch)

#endif /* schedLib_PORTABLE */

/******************************************************************************
 * intStackSet - Set interrupt stack pointer
 *
 * RETURNS: N/A
 */

        .balign 16, 0x90

FUNC_LABEL(intStackSet)
        movl    SP_ARG1(%esp), %eax
        movl    %eax, FUNC(excStackPtr)
        ret

/******************************************************************************
 * intStackEnable - Enable/disable interrupt stack
 *
 * RETURNS: OK or ERROR
 */

        .balign 16, 0x90

FUNC_LABEL(intStackEnable)
        movl    %cs, %eax
        cmpl    FUNC(sysCsSuper), %eax
        jne     intStackEnableError

        movl    SP_ARG1(%esp), %edx
        movl    %edx, FUNC(excStackEnabled)
        movl    $OK, %eax
        ret

intStackEnableError:
        movl    $ERROR, %eax
        ret

