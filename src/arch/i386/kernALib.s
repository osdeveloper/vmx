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

/* kernALib.s - System dependent assembler parts of kernel */

#define _ASMLANGUAGE
#include <arch/asm.h>
#include <arch/regs.h>
#include <arch/taskArchLib.h>
#include <arch/esf.h>

        .data
        /* Imports */
        .globl  GDATA(taskIdCurrent)
        .globl  GDATA(kernReadyQ)
        .globl  GDATA(sysCsSuper)
        .globl  GDATA(errno)
        .globl  GDATA(kernelState)
        .globl  GDATA(intCnt)
        .globl  GDATA(taskExit)
        .globl  GDATA(kernQEmpty)
        .globl  GDATA(kernQDoWork)

        /* Internals */
        .globl  GTEXT(kernIntEnt)
        .globl  GTEXT(kernIntExit)
        .globl  GTEXT(kernExit)
        .globl  GTEXT(kernTaskLoadContext)
        .globl  GTEXT(kernTaskEntry)

FUNC_LABEL(intNest)
        .long   0x00000000

        .text
        .balign 16

/******************************************************************************
 * kernIntEnt - Called when an interrupt occurs
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernIntEnt)
        cli                                     /* Disable interrupts */
        pushl   (%esp)                          /* Store return address */
        pushl   %eax                            /* Save EAX */

        /* Store errno where return address was on stack */
        movl    FUNC(errno),%eax
        movl    %eax,8(%esp)

        /* Increase interrupt counter */
        incl    FUNC(intNest)
        incl    FUNC(intCnt)

intEnt0:
        /* Restore flags */
        pushl   ESF0_EFLAGS+12(%esp)
        popfl
        popl    %eax                            /* Restore EAX */

        /* Return */
        ret

/******************************************************************************
 * kernIntExit - Routine only brached to when interrupt ends
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernIntExit)

        /* Restore errno from stack */
        popl    FUNC(errno)
        pushl   %eax                            /* Save EAX */

        /* Decrease interrupt counter */
        cli                                     /* Disable interrupts */
        decl    FUNC(intNest)
        decl    FUNC(intCnt)

        /* If CS on stack is sysCsInt we are in an interrupt */
        movl    ESF0_CS+4(%esp), %eax
        cmpw    FUNC(sysCsInt),%ax
        je      intRet

intExit0:
        /* Check if in kernel mode */
        cmpl    $0,FUNC(kernelState)
        jne     intRet

        /* If not in kernel mode, rescheduling might be needed */
        movl    FUNC(taskIdCurrent),%eax
        cmpl    FUNC(kernReadyQ),%eax
        je      intRet

        /* Check if next task is ready */
        cmpl    $0,TASK_TCB_LOCK_COUNT(%eax)    /* Is task preemption on */
        je      FUNC(kernIntContextSave)
        cmpl    $0,TASK_TCB_STATUS(%eax)
        jne     FUNC(kernIntContextSave)

intRet:
        popl    %eax                            /* Restore EAX */

        /* Return from interrupt */
        iret

/******************************************************************************
 * kernIntContextSave - Save task context (helper)
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernIntContextSave)
        /* Save task context */
        movl    $1,FUNC(kernelState)            /* Move to kernel mode */
        movl    FUNC(taskIdCurrent),%eax
        popl    TASK_TCB_EAX(%eax)              /* EAX to TCB */
        popl    TASK_TCB_PC(%eax)               /* PC to TCB */
        leal    4(%esp),%esp                    /* Ignore CS */
        popl    TASK_TCB_EFLAGS(%eax)           /* EFLAGS to TCB */
        sti                                     /* Re-enable interrupts */

        /* Now save all registers */
        movl    %edx,TASK_TCB_EDX(%eax)         /* EDX to TCB */
        movl    %ecx,TASK_TCB_ECX(%eax)         /* ECX to TCB */
        movl    %ebx,TASK_TCB_EBX(%eax)         /* EBX to TCB */
        movl    %esi,TASK_TCB_ESI(%eax)         /* ESI to TCB */
        movl    %edi,TASK_TCB_EDI(%eax)         /* EDI to TCB */
        movl    %ebp,TASK_TCB_EBP(%eax)         /* EBP to TCB */
        movl    %esp,TASK_TCB_ESP(%eax)         /* ESP to TCB */

        /* Save errno */
        movl    FUNC(errno),%edx
        movl    %edx,TASK_TCB_ERRNO(%eax)
        movl    %eax,%edx                       /* Active task in EDX */

        /* Jump to rescheduler function here */
        jmp     FUNC(kernTaskReschedule)

/******************************************************************************
 * kernCheckTaskReady - Check if taskIdCurrent is ready to run (helper)
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernCheckTaskReady)
        cmpl    $0,TASK_TCB_STATUS(%edx)        /* Check status flag */
        jne     FUNC(kernSaveTaskContext)

        /* FALL TRU TO CHECK KERNEL QUEUE */

/******************************************************************************
 * kernQCheck - Check if kernel work need to be done (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernQCheck)

        cli                                     /* Lock interrupts */
        cmpl    $0,FUNC(kernQEmpty)             /* Check for kernel work */
        je      FUNC(kernQWorkPreSave)          /* Work to to */

        movl    $0,FUNC(kernelState)            /* Release mutex */
        sti                                     /* Unlock interrupts */
        xorl    %eax,%eax                       /* Return 0 */

        /* Return */
        ret

/******************************************************************************
 * kernQWorkPreSave - Exit kernel mode (helper)
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernQWorkPreSave)

        sti                                     /* Unlock interrputs */
        call    FUNC(kernQDoWork)               /* Do kernel work */
        jmp     FUNC(kernCheckTaskSwitch)       /* Check task switch */

/******************************************************************************
 * kernExit - Exit kernel mode
 *
 * RETURNS: OK or ERROR
 */

        .balign 16,0x90

FUNC_LABEL(kernExit)

        cmpl    $0,FUNC(intCnt)                 /* intCnt=0 from task */
        jne     FUNC(kernIntExit)               /* If not task go away */

        /* FALL TRU TO CHECK TASK SWITCH */

/******************************************************************************
 * kernCheckTaskSwitch - Check if another task should run (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernCheckTaskSwitch)

        movl    FUNC(taskIdCurrent),%edx
        cmpl    FUNC(kernReadyQ),%edx           /* Compare task to queue */
        je      FUNC(kernQCheck)                /* If the same leave */

        /* If preemption is allowed, check if task is ready */
        cmpl    $0,TASK_TCB_LOCK_COUNT(%edx)
        jne     FUNC(kernCheckTaskReady)

        /* FALL TRU TO SAVE CONTEXT */

/******************************************************************************
 * kernSaveTaskContext - Save task context (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernSaveTaskContext)

        movl    (%esp),%eax                     /* Save return address to PC */
        movl    %eax,TASK_TCB_PC(%edx)
        pushfl                                  /* Save flags */
        popl    TASK_TCB_EFLAGS(%edx)
        bts     $9,TASK_TCB_EFLAGS(%edx)        /* set IF to INT enable */

        /* Save regs */
        movl    %ebx,TASK_TCB_EBX(%edx)
        movl    %esi,TASK_TCB_ESI(%edx)
        movl    %edi,TASK_TCB_EDI(%edx)
        movl    %ebp,TASK_TCB_EBP(%edx)
        movl    %esp,TASK_TCB_ESP(%edx)
        movl    $0,TASK_TCB_EAX(%edx)           /* Clear eax and return */
        addl    $4,TASK_TCB_ESP(%edx)           /* Fix ret addr */

        pushl   FUNC(errno)                     /* Save error code */
        popl    TASK_TCB_ERRNO(%edx)

        /* FALL TRU TO TASK RESCHEDULE */

/******************************************************************************
 * kernTaskReschedule - Reschedule running task (helper)
 *
 * RETURNS: N/A
 *****************************************************************************/

        .balign 16,0x90

FUNC_LABEL(kernTaskReschedule)

        /* Load active task */
        movl    FUNC(kernReadyQ),%eax

        /* FALL TRU TO SWITCH TASKS */

/******************************************************************************
 * kernSwitchTasks - Do task switch (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernSwitchTasks)

        /* Store new task as current */
        movl    %eax,FUNC(taskIdCurrent)

        /* FALL TRU TO DISPATCH */

/******************************************************************************
 * kernDispatch - Store task context for task switch (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernDispatch)

        /* Switch to task stored in EAX */
        movl    TASK_TCB_ERRNO(%eax),%ecx               /* Restore errno */
        movl    %ecx,FUNC(errno)
        movl    TASK_TCB_ESP(%eax),%esp                 /* Restore stack */
        pushl   TASK_TCB_EFLAGS(%eax)                   /* Flags on stack */
        pushl   FUNC(sysCsSuper)                        /* Segment on stack */
        pushl   TASK_TCB_PC(%eax)                       /* PC on stack */

        /* Restore registers */
        movl    TASK_TCB_EDX(%eax),%edx                 /* TCB to EDX */
        movl    TASK_TCB_ECX(%eax),%ecx                 /* TCB to ECX */
        movl    TASK_TCB_EBX(%eax),%ebx                 /* TCB to EBX */
        movl    TASK_TCB_ESI(%eax),%esi                 /* TCB to ESI */
        movl    TASK_TCB_EDI(%eax),%edi                 /* TCB to EDI */
        movl    TASK_TCB_EBP(%eax),%ebp                 /* TCB to EBP */
        movl    TASK_TCB_EAX(%eax),%eax                 /* TCB to EAX */

        /* Now exit kernel mode */
        cli                                             /* Disable interrupts */
        cmpl    $0,FUNC(kernQEmpty)                     /* Check for work */
        je      FUNC(kernQWorkUnlock)

        movl    $0,FUNC(kernelState)                    /* Exit kern mode */

        /* Return from interrupt */
        iret

/******************************************************************************
 * kernQWorkUnlock - Unlock interrputs and do fall tru to do work (helper)
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernQWorkUnlock)

        sti                                             /* Unlock interrupts */

        /* FALL TRU TO KERNEL WORK */

/******************************************************************************
 * kernQWork - Do kernel work (helper)
 *
 * RETURNS: N/A
 */

FUNC_LABEL(kernQWork)

        call    FUNC(kernQDoWork)                       /* Do work */

        /* Check task queue if resheduling is needed */
        movl    FUNC(taskIdCurrent),%edx
        movl    FUNC(kernReadyQ),%eax
        cmpl    %edx,%eax                               /* Compare tasks */
        je      FUNC(kernDispatch)                      /* the same? */
        jmp     FUNC(kernSwitchTasks)                   /* else switch */

/******************************************************************************
 * kernTaskLoadContext - Load task context
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernTaskLoadContext)

        /* Load current tcb */
        movl    FUNC(kernReadyQ),%eax
        movl    %eax,FUNC(taskIdCurrent)
        movl    TASK_TCB_ERRNO(%eax),%ecx
        movl    %ecx,FUNC(errno)
        movl    TASK_TCB_ESP(%eax),%esp

        /* Setup stack */
        pushl   TASK_TCB_EFLAGS(%eax)
        pushl   FUNC(sysCsSuper)
        pushl   TASK_TCB_PC(%eax)

        /* Restore regs */
        movl    TASK_TCB_EDX(%eax),%edx
        movl    TASK_TCB_ECX(%eax),%ecx
        movl    TASK_TCB_EBX(%eax),%ebx
        movl    TASK_TCB_ESI(%eax),%esi
        movl    TASK_TCB_EDI(%eax),%edi
        movl    TASK_TCB_EBP(%eax),%ebp
        movl    TASK_TCB_EAX(%eax),%eax

        iret

/******************************************************************************
 * kernTaskEntry - Task entry point
 *
 * RETURNS: N/A
 */

        .balign 16,0x90

FUNC_LABEL(kernTaskEntry)
        xorl    %ebp,%ebp

        /* Load current tcb */
        movl    FUNC(taskIdCurrent),%eax

        /* Get entry point in tcb */
        movl    TASK_TCB_ENTRY(%eax),%eax

        /* Call entry point */
        call    *%eax

        /* Pop argument to task */
        addl    $40,%esp

        /* Pass result to exit func */
        pushl   %eax
        call    FUNC(taskExit)

