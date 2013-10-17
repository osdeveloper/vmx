/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* usrConfig.c - usrInit() user code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/iv.h>
#include <arch/sysArchLib.h>
#include <arch/intArchLib.h>
#include <arch/taskArchLib.h>
#include <vmx/logLib.h>
#include <vmx/memPartLib.h>
#include <vmx/kernLib.h>
#include <vmx/tickLib.h>
#include <vmx/taskLib.h>
#include <vmx/vmxLib.h>

#define RESTART_TASK
#define DELAY_TIME	(18 * 1)

/* Globals */
SEM_ID sem;
char bigString[] = "\n"
		   "******************************************************\n"
  		   "Suddenly I awoke. I must have falled asleep under the \n"
                   "oak, where I used to sit and dream as a child.\n"
		   "I remebered my strange dream and knew the love and \n"
		   "strength in my heart to create something from my \n"
		   "life. The world was waiting, I was reborn...\n"
  		   "******************************************************"
		   "\n";

char* itoa2(int a)
{
  static char str[10];

  itoa(a, str, 10);

  return str;
}

int runMe(ARG arg0)
{
  TCB_ID pTcb;

  pTcb = taskTcb(taskIdSelf());

  semTake(sem, WAIT_FOREVER);
  puts("----------------------------------------------------------------\n");
  puts(pTcb->name);
  puts(" with args: ");
  puts(itoa2((int)arg0));
  puts("\n");
  puts("----------------------------------------------------------------\n");
  semGive(sem);

  return (int)arg0;
}

#ifdef RESTART_TASK
volatile int num = 0;
int restartMe(ARG arg0)
{
  TCB_ID pTcb;
  volatile int *pInt = (volatile int *) arg0;

  pTcb = taskTcb(taskIdSelf());

  semTake(sem, WAIT_FOREVER);
  puts(pTcb->name);
  puts(" restarted ");
  puts(itoa2(*pInt));
  puts(" times.");
  puts("\n");
  semGive(sem);

  for (;;);

  return *pInt;
}
#endif

int init(ARG arg0,
	 ARG arg1,
	 ARG arg2,
	 ARG arg3,
	 ARG arg4,
	 ARG arg5,
	 ARG arg6,
	 ARG arg7,
	 ARG arg8,
	 ARG arg9)
{
  TCB_ID pTcb;

  pTcb = taskTcb(taskIdSelf());

  semBInit(sem, SEM_Q_PRIORITY, SEM_FULL);
  puts(pTcb->name);
  puts(" called with arguments: ");
  puts(itoa2((int)arg0)); puts(", ");
  puts(itoa2((int)arg1)); puts(", ");
  puts(itoa2((int)arg2)); puts(", ");
  puts(itoa2((int)arg3)); puts(", ");
  puts(itoa2((int)arg4)); puts(", ");
  puts(itoa2((int)arg5)); puts(", ");
  puts(itoa2((int)arg6)); puts(", ");
  puts(itoa2((int)arg7)); puts(", ");
  puts(itoa2((int)arg8)); puts(", ");
  puts(itoa2((int)arg9)); puts("\n");
  return 0;
}

int printBigString(void)
{
  int i;
  int taskId;
  TCB_ID pTcb;
  for (i = 0;;i++) {
    taskId = taskSpawn("runMe", 1, TASK_OPTIONS_DEALLOC_STACK,
	       	     DEFAULT_STACK_SIZE, (FUNCPTR) runMe,
	       	     (ARG) i,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0,
	       	     (ARG) 0);
    pTcb = (TCB_ID) taskId;
    semTake(sem, WAIT_FOREVER);
    puts(bigString);
    semGive(sem);
    taskDelay(DELAY_TIME * 5);
    semTake(sem, WAIT_FOREVER);
    puts("----------------------------------------------------------------\n");
    puts("Return from last ");
    puts(pTcb->name);
    puts(": ");
    puts(itoa2(pTcb->exitCode));
    puts("\n");
    puts("----------------------------------------------------------------\n");
    semGive(sem);
  }

  return 1;
}

int printSysTime(ARG arg0, ARG arg1)
{
  int i;
#ifdef RESTART_TASK
  TCB_ID pTcb = (TCB_ID) arg0;
  volatile int *pInt = (volatile int *) arg1;
#endif

  for (i = 0;;i++) {
    semTake(sem, WAIT_FOREVER);
    puts("System time is: ");
    puts(itoa2(tickGet()/18));
    puts("...");
#ifdef RESTART_TASK
    if (i == 3)
    {
      i=0;
      pInt[0]++;
      puts("I will try to restart: ");
      puts(pTcb->name);
      puts(" for the ");
      puts(itoa2(*pInt));
      puts(" time");
      taskRestart((int) pTcb);
    }
#endif
    puts("\n");
    semGive(sem);
    taskDelay(DELAY_TIME);
  }

  return 2;
}

int slowFill(void)
{
  int i, j;
  for(;;) {
    semTake(sem, WAIT_FOREVER);
    for (i = 0; i < 80; i++) {
      puts("=");
      for(j = 0; j < 0x00ffffff; j++);
    }
    semGive(sem);
    taskDelay(DELAY_TIME * 10);
  }

  return 3;
}

int initTasks(void)
{
#ifdef RESTART_TASK
  int restartTaskId;
#endif

  puts("Welcome to Real VMX...\n");
  puts("This system is released under GNU public license.\n\n");

  taskSpawn("init", 1, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) init,
	     (ARG) 10,
	     (ARG) 11,
	     (ARG) 12,
	     (ARG) 13,
	     (ARG) 14,
	     (ARG) 15,
	     (ARG) 16,
	     (ARG) 17,
	     (ARG) 18,
	     (ARG) 19);

#ifdef RESTART_TASK
  restartTaskId =
  taskSpawn("restartMe", 100, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) restartMe,
	     (ARG) &num,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0,
	     (ARG) 0);
#endif

  taskSpawn("printBigString", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) printBigString,
	     (ARG) 20,
	     (ARG) 21,
	     (ARG) 22,
	     (ARG) 23,
	     (ARG) 24,
	     (ARG) 25,
	     (ARG) 26,
	     (ARG) 27,
	     (ARG) 28,
	     (ARG) 29);

  taskSpawn("printSysTime", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) printSysTime,
#ifdef RESTART_TASK
	     (ARG) taskTcb(restartTaskId),
	     (ARG) &num,
#else
	     (ARG) NULL,
	     (ARG) 31,
#endif
	     (ARG) 32,
	     (ARG) 33,
	     (ARG) 34,
	     (ARG) 35,
	     (ARG) 36,
	     (ARG) 37,
	     (ARG) 38,
	     (ARG) 39);

  taskSpawn("slowFill", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) slowFill,
	     (ARG) 30,
	     (ARG) 31,
	     (ARG) 32,
	     (ARG) 33,
	     (ARG) 34,
	     (ARG) 35,
	     (ARG) 36,
	     (ARG) 37,
	     (ARG) 38,
	     (ARG) 39);

  return 0;
}

void kernelInit(char *pMemPoolStart, unsigned memPoolSize)
{
#ifdef DEBUG
  puts("Initializing kernel logger library:\n");
#endif

  setLogFlags(LOG_TASK_LIB|LOG_VMX_LIB|LOG_SEM_LIB|LOG_KERN_HOOK_LIB);
  setLogLevel(LOG_LEVEL_ERROR|LOG_LEVEL_WARNING/*|LOG_LEVEL_INFO*/);

#ifdef DEBUG
  puts("Initializing class library:\n");
#endif

  classLibInit();

#ifdef DEBUG
  puts("Initializing memory partition library:\n");
#endif

  memPartLibInit(pMemPoolStart, memPoolSize);

#ifdef DEBUG
  puts("Initializing semaphores:\n");
#endif

  semLibInit();

#ifdef DEBUG
  puts("Initializing task library:\n");
#endif

  tickLibInit();
  taskLibInit();

#ifdef DEBUG
  puts("Initializing kernel:\n");
#endif

  kernInit((FUNCPTR) initTasks);
}

void usrInit(void)
{
  int i, j;
  char *ptr, *lastPtr;
  PART_ID memUsrPartitionId;

  sysHwInit();

  kernelInit((char *) 0x00400000, 0x00400000);
  kernelTimeSlice(1);

#ifdef DEBUG
  puts("Multitasking not enabled:\n");
#endif

  for(;;);
}

