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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/iv.h>
#include <arch/sysArchLib.h>
#include <arch/intArchLib.h>
#include <arch/vmxArchLib.h>
#include <arch/taskArchLib.h>
#include <vmx/logLib.h>
#include <vmx/memPartLib.h>
#include <vmx/kernLib.h>
#include <vmx/tickLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/vmxLib.h>
#include <vmx/semLib.h>
#include <vmx/msgQLib.h>
#include <io/iosLib.h>
#include <io/pathLib.h>
#include <io/echoDrv.h>
#include "configAll.h"
#include "config.h"

#define INPUT_TEST
#define INT_STACK_SIZE          (8192 * 2048)
#define DELAY_TIME	        (18 * 1)
#define MAX_MESSAGES            10

/* Globals */
char intStack[INT_STACK_SIZE];
char smallString[] = "Hello World!\n";

char bigString[] = "\n"
		   "******************************************************\n"
  		   "Suddenly I awoke. I must have falled asleep under the \n"
                   "oak, where I used to sit and dream as a child.\n"
		   "I remebered my strange dream and knew the love and \n"
		   "strength in my heart to create something from my \n"
		   "life. The world was waiting, I was reborn...\n"
  		   "******************************************************"
		   "\n";
SEM_ID sem;
SEM_ID evtSem;
MSG_Q_ID msgQId;
int numMsg = 0;
char buf[1024];
int consoleFd;
int echoFd;
char consoleName[20];

char* itoa2(int a)
{
  static char str[10];

  itoa(a, str, 10);

  return str;
}

#ifdef TASK_POST
int runMe(ARG arg0)
{
  semTake(sem, WAIT_FOREVER);
  puts("----------------------------------------------------------------\n");
  puts(taskName(0));
  puts(" with args: ");
  puts(itoa2((int)arg0));
  puts("\n");
  puts("----------------------------------------------------------------\n");
  semGive(sem);

  return (int)arg0;
}
#endif

#ifdef RESTART_TASK
volatile int num = 0;
int restartMe(ARG arg0)
{
  volatile int *pInt = (volatile int *) arg0;

  semTake(sem, WAIT_FOREVER);
  puts(taskName(0));
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
  puts(taskName(0));
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

#ifdef TASK_POST
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
#else
int printBigString(void)
{
  while (1) {
    semTake(sem, WAIT_FOREVER);
    puts(bigString);
    semGive(sem);
    taskDelay(DELAY_TIME * 5);
  }

  return 1;
}
#endif

int printSysTime(ARG arg0, ARG arg1)
{
  int i;
#ifdef RESTART_TASK
  int taskId = (int) arg0;
  volatile int *pInt = (volatile int *) arg1;
#endif

  for (i = 0;;i++) {
    semTake(sem, WAIT_FOREVER);
    puts("System time is: ");
    puts(itoa2(tickGet()/18));
    puts("...");
    if (i == 3)
    {
      i=0;
#ifdef RESTART_TASK
      pInt[0]++;
      puts("I will try to restart: ");
      puts(taskName(taskId));
      puts(" for the ");
      puts(itoa2(*pInt));
      puts(" time");
      taskRestart(taskId);
#endif
      semGive(evtSem);
    }
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

int evtHandler(void)
{
  for (;;)
  {
    semTake(evtSem, WAIT_FOREVER);
    puts("?????????????????????????????????????????????????????????????????\n");
    puts("Event triggered\n");
    puts("?????????????????????????????????????????????????????????????????\n");
  }

  return 0;
}

int sendMessage(void)
{
  if ((numMsg + 1) >= MAX_MESSAGES) {
    semTake(sem, WAIT_FOREVER);
    puts("Message queue full.\n");
    semGive(sem);

    return 1;
  }

  numMsg++;
  semTake(sem, WAIT_FOREVER);
  puts("Sending message to message queue...");
  semGive(sem);

  msgQSend(msgQId, bigString, strlen(bigString), 600, MSG_PRI_NORMAL);

  semTake(sem, WAIT_FOREVER);
  puts("Message sent.\n");

  puts(itoa2(numMsg));
  puts(" message(s) sent to message queue.\n");

  semGive(sem);

  return 0;
}

int receiveMessage(void)
{
  char buf[512];

  semTake(sem, WAIT_FOREVER);
  puts("Receiving message from message queue...\n");
  semGive(sem);

  msgQReceive(msgQId, buf, sizeof(bigString), WAIT_FOREVER);
  numMsg--;

  semTake(sem, WAIT_FOREVER);
  puts("Received message: ");
  semGive(sem);

  buf[strlen(bigString)]='\0';
  puts(buf);

  puts(itoa2(numMsg));
  puts(" message(s) left in message queue.\n");

  return 0;
}

int messageSender(void)
{
  int rnd;

  for (;;) {
    sendMessage();
    rnd = rand() % 10;
    taskDelay(rnd * DELAY_TIME);
  }
}

int messageReceiver(void)
{
  int rnd;

  for (;;) {
    receiveMessage();
    rnd = rand() % 10;
    taskDelay(rnd * DELAY_TIME);
  }
}

#ifdef INPUT_TEST
int inputTask(void)
{
  size_t bread;

  for (;;) {
    puts("-> ");
    memset(buf, 0, 1024);
    bread = read(STDIN_FILENO, buf, 1024);
    puts("Read ");
    puts(itoa2(bread));
    puts(" byte(s): ");
    write(STDOUT_FILENO, buf, bread);
    puts("\n");
  }

  return 0;
}

void print_fds(void)
{
  int i;
  int fd = 0;
  static char *fns[3] = { "/pcConsole/0", "/pcConsole/1", "/pcConsole/2" };

  puts("stdin: ");
  puts(itoa2(ioGlobalStdGet(STDIN_FILENO)));
  puts("\n");

  puts("stdout: ");
  puts(itoa2(ioGlobalStdGet(STDOUT_FILENO)));
  puts("\n");

  puts("stderr: ");
  puts(itoa2(ioGlobalStdGet(STDERR_FILENO)));
  puts("\n");

  for (i = 0; i < 3; i++) {
    fd = open(fns[i], O_RDWR, 0);
    puts("Found fd: ");
    puts(itoa2(fd));
    puts("\n");
    close(fd);
  }

  puts("console drv: ");
  puts(itoa2(pcConDrvNumber()));
  puts("\n");

  puts("echo drv: ");
  puts(itoa2(echoDrvNumber()));
  puts("\n");
}

void echoWriteInt(void)
{
  write(echoFd, smallString, strlen(smallString));
}

int echoWrite(void)
{
  int bwrote;

  for (;;) {
    taskDelay(3 * DELAY_TIME);
    bwrote = write(echoFd, smallString, strlen(smallString));
    if (bwrote <= 0)
    {
      puts("Unable to write to echo device\n");
      break;
    }
    {
      puts("Wrote ");
      puts(itoa2(bwrote));
      puts(" byte(s)\n");
    }
  }
}

int echoRead(void)
{
  int bread;

  for (;;) {
    memset(buf, 0, 1024);
    puts("Waiting for data...\n");
    bread = read(echoFd, buf, 1024);
    puts("Read ");
    puts(itoa2(bread));
    puts(" byte(s): ");
    write(STDOUT_FILENO, buf, bread);
    puts("\n");
  }

  return 0;
}
#endif

int test_realloc(void)
{
  int slen = strlen(smallString);
  int blen = strlen(bigString);
  char *pData = malloc(slen + 1);
  if (pData == NULL)
  {
    puts("Error allocating small string.\n");
    return 1;
  }
  strcpy(pData, smallString);
  puts(pData);

  pData = realloc(pData, slen + blen + 10);
  if (pData == NULL)
  {
    puts("Error re-allocating plus big string.\n");
    return 1;
  }
  strcpy(&pData[slen], bigString);
  puts(pData);
}

int initTasks(void)
{
#ifdef RESTART_TASK
  int restartTaskId;
#endif

  intStackSet(&intStack[INT_STACK_SIZE/2]);
  intStackEnable(TRUE);

  puts("Welcome to Real VMX...\n");
  puts("This system is released under GNU public license.\n\n");

  sem = semCreate(SEM_TYPE_BINARY, SEM_Q_FIFO);
  if (sem == NULL)
  {
    puts("Unable to create semaphore\n");
    for (;;);
  }

  evtSem = semCreate(SEM_TYPE_BINARY, SEM_Q_FIFO);
  if (evtSem == NULL)
  {
    puts("Unable to create event semaphore\n");
    for (;;);
  }

  msgQId = msgQCreate(MAX_MESSAGES + 1, strlen(bigString) + 1, MSG_Q_PRIORITY);
  if (msgQId == NULL)
  {
    puts("Unable to create message queue\n");
    for (;;);
  }

  echoFd = open("/echo", O_RDWR, 0);
  if (echoFd == ERROR)
  {
    puts("Unable to open echo device\n");
    for (;;);
  }
  ioctl(echoFd, FIOSETOPTIONS, OPT_TERMINAL);

  //test_realloc();

#ifndef INPUT_TEST
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
	     (ARG) restartTaskId,
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
	     (ARG) 40,
	     (ARG) 41,
	     (ARG) 42,
	     (ARG) 43,
	     (ARG) 44,
	     (ARG) 45,
	     (ARG) 46,
	     (ARG) 47,
	     (ARG) 48,
	     (ARG) 49);

  taskSpawn("evtHandler", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) evtHandler,
	     (ARG) 50,
	     (ARG) 51,
	     (ARG) 52,
	     (ARG) 53,
	     (ARG) 54,
	     (ARG) 55,
	     (ARG) 56,
	     (ARG) 57,
	     (ARG) 58,
	     (ARG) 59);

  taskSpawn("messageReceiver", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) messageReceiver,
	     (ARG) 60,
	     (ARG) 61,
	     (ARG) 62,
	     (ARG) 63,
	     (ARG) 64,
	     (ARG) 65,
	     (ARG) 66,
	     (ARG) 67,
	     (ARG) 68,
	     (ARG) 69);

  taskSpawn("messageSender", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) messageSender,
	     (ARG) 60,
	     (ARG) 61,
	     (ARG) 62,
	     (ARG) 63,
	     (ARG) 64,
	     (ARG) 65,
	     (ARG) 66,
	     (ARG) 67,
	     (ARG) 68,
	     (ARG) 69);

#else

#if 0
  taskSpawn("readTask", 2, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) echoRead,
	     (ARG) 60,
	     (ARG) 61,
	     (ARG) 62,
	     (ARG) 63,
	     (ARG) 64,
	     (ARG) 65,
	     (ARG) 66,
	     (ARG) 67,
	     (ARG) 68,
	     (ARG) 69);

  taskSpawn("writeTask", 3, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) echoWrite,
	     (ARG) 60,
	     (ARG) 61,
	     (ARG) 62,
	     (ARG) 63,
	     (ARG) 64,
	     (ARG) 65,
	     (ARG) 66,
	     (ARG) 67,
	     (ARG) 68,
	     (ARG) 69);
#endif
  taskSpawn("inputTask", 3, 0,
	     DEFAULT_STACK_SIZE, (FUNCPTR) inputTask,
	     (ARG) 60,
	     (ARG) 61,
	     (ARG) 62,
	     (ARG) 63,
	     (ARG) 64,
	     (ARG) 65,
	     (ARG) 66,
	     (ARG) 67,
	     (ARG) 68,
	     (ARG) 69);
#endif

  return 0;
}

void kernelInit(char *pMemPoolStart, unsigned memPoolSize)
{
  char ttyName[20];
  int i, len;

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
  semBLibInit();
  semMLibInit();
  semCLibInit();

  msgQLibInit();
  /* For some reason in need this in order to include ffsLib */
  ffsLsb(0);

#ifdef DEBUG
  puts("Initializing task library:\n");
#endif

  tickLibInit();
  taskLibInit();

  iosLibInit(20, 50, "/null");
  pathLibInit();
  pcConDrvInit();

  /* For all virtual consoles */
  for (i = 0; i < N_VIRTUAL_CONSOLES; i++) {

    /* Create name for device */
    strcpy(ttyName, "/pcConsole/");
    len = strlen(ttyName);
    ttyName[len] = i + '0';
    ttyName[len + 1] = EOS;

    /* Create device */
    pcConDevCreate(ttyName, i, 512, 512);

    /* If pc console number */
    if (i == PC_CONSOLE) {

      /* Copy to global console name */
      strcpy(consoleName, ttyName);

      /* Open file and set console options */
      consoleFd = open(consoleName, O_RDWR, 0);
      ioctl(consoleFd, FIOSETOPTIONS, OPT_TERMINAL);

    } /* End if pc console number */

  } /* End for all virtual consoles */

  /* Set standard file descriptors */
  ioGlobalStdSet(STDIN_FILENO, consoleFd);
  ioGlobalStdSet(STDOUT_FILENO, consoleFd);
  ioGlobalStdSet(STDERR_FILENO, consoleFd);

  echoDrvInit();
  echoDevCreate("/echo", 1024, 1024);

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

