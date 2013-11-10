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

/* Defines */
#define MEM_POOL_START_ADRS             0x00400000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vmx.h>
#include <private/stdioP.h>
#include <arch/iv.h>
#include <arch/sysArchLib.h>
#include <arch/intArchLib.h>
#include <arch/excArchLib.h>
#include <arch/excArchShow.h>
#include <arch/vmxArchLib.h>
#include <arch/taskArchLib.h>
#include <util/hashLib.h>
#include <vmx/kernelLib.h>
#include <vmx/tickLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/vmxLib.h>
#include <vmx/semLib.h>
#include <vmx/msgQLib.h>
#include <vmx/wdLib.h>
#include <vmx/wdShow.h>
#include <os/memPartLib.h>
#include <os/memLib.h>
#include <os/memShow.h>
#include <os/taskHookLib.h>
#include <os/selectLib.h>
#include <os/symLib.h>
#include <os/symShow.h>
#include <os/iosLib.h>
#include <os/iosShow.h>
#include <os/pathLib.h>
#include <os/logLib.h>
#include <os/excLib.h>
#include <os/pipeDrv.h>
#include <tools/shellLib.h>
#include <usr/usrLib.h>
#include "configAll.h"
#include "config.h"

/* Imports */
IMPORT standTableSize;
IMPORT SYMBOL standTable[];

IMPORT void standTableInit(void);

IMPORT void sysHwInit0(void);
IMPORT void sysHwInit(void);

IMPORT STATUS sysClockConnect(FUNCPTR func, int arg);
IMPORT STATUS sysClockRateSet(int tickePerSecond);
IMPORT void   sysClockEnable(void);

#define INPUT_TEST
#define DELAY_TIME              (1 * sysClockRateGet())
#define WDOG_TIME               (2 * sysClockRateGet())
#define MAX_MESSAGES            10

LOCAL void usrRoot(
    char *pMemPoolStart,
    unsigned memPoolSize
    );

/* Globals */
SYMTAB_ID sysSymTable;

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
WDOG_ID wdogId;
int numMsg = 0;
char buf[1024];
int consoleFd;
char consoleName[20];

struct
{
    unsigned th_create;
    unsigned th_delete;
    unsigned th_switch;
    unsigned th_swapin;
    unsigned th_swapout;
} taskHookCnt;

/******************************************************************************
 * usrClock - User system clock routine
 *
 * RETURNS: N/A
 */

void usrClock(
    void
    )
{
    vmxTickAnnounce();
}

int task_create(
    void
    )
{
    return ++taskHookCnt.th_create;
}

int task_switch(
    void
    )
{
    return ++taskHookCnt.th_switch;
}

int task_swapin(
    void
    )
{
    return ++taskHookCnt.th_swapin;
}

int task_swapout(
    void
    )
{
    return ++taskHookCnt.th_swapout;
}

int task_delete(
    void
    )
{
    return ++taskHookCnt.th_delete;
}

int wdFunc(
    int arg
    )
{
    printf("Watchdog triggered with argument: %d\n", arg);

    return 0;
}

#ifdef TASK_POST
int runMe(ARG arg0)
{
  semTake(sem, WAIT_FOREVER);
  printf("----------------------------------------------------------------\n");
  printf("%s with args: %d\n", taskName(0), (int) arg0);
  printf("----------------------------------------------------------------\n");
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
  printf("%s restarted %d time(s)\n", taskName(0), *pInt);
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
  printf(
    "%s called with arguments: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    taskName(0),
    (int) arg0,
    (int) arg1,
    (int) arg2,
    (int) arg3,
    (int) arg4,
    (int) arg5,
    (int) arg6,
    (int) arg7,
    (int) arg8,
    (int) arg9
    );
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
    printf(bigString);
    semGive(sem);
    taskDelay(DELAY_TIME * 5);
    semTake(sem, WAIT_FOREVER);
    printf("---------------------------------------------------------------\n");
    printf("Return from last %s: %d\n", pTcb->name, pTcb->exitCode);
    printf("---------------------------------------------------------------\n");
    semGive(sem);
  }

  return 1;
}
#else
int printBigString(void)
{
  while (1) {
    semTake(sem, WAIT_FOREVER);
    printf(bigString);
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
    printf("System time is: %d...\n", tickGet() / 18);
    printf("CRE: %d, SWI: %d, SWI: %d, SWO: %d, DEL: %d\n",
      taskHookCnt.th_create,
      taskHookCnt.th_switch,
      taskHookCnt.th_swapin,
      taskHookCnt.th_swapout,
      taskHookCnt.th_delete);
    if (i == 3)
    {
      i=0;
#ifdef RESTART_TASK
      pInt[0]++;
      printf(
        "I will try to restart: %d for the %d time(s)\n",
        taskName(taskId),
        *pInt
        );
      taskRestart(taskId);
#endif
      semGive(evtSem);
    }
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
      printf("=");
      for(j = 0; j < 0x00ffffff; j++);
    }
    semGive(sem);
    taskDelay(DELAY_TIME * 10);
  }

  return 3;
}

int evtHandler(void)
{
  static int cnt;
  for (;;)
  {
    semTake(evtSem, WAIT_FOREVER);
    ++cnt;
    printf("???????????????????????????????????????????????????????????????\n");
    printf("Event triggered %d time(s), swap-in: %d, swap-out: %d\n",
           cnt, taskHookCnt.th_swapin, taskHookCnt.th_swapout);
    printf("???????????????????????????????????????????????????????????????\n");
    wdStart(wdogId, WDOG_TIME, (FUNCPTR) wdFunc, (ARG) cnt);
  }

  return 0;
}

int sendMessage(void)
{
  if ((numMsg + 1) >= MAX_MESSAGES) {
    semTake(sem, WAIT_FOREVER);
    printf("Message queue full.\n");
    semGive(sem);

    return 1;
  }

  numMsg++;
  semTake(sem, WAIT_FOREVER);
  printf("Sending message to message queue...");
  semGive(sem);

  msgQSend(msgQId, bigString, strlen(bigString), 600, MSG_PRI_NORMAL);

  semTake(sem, WAIT_FOREVER);
  printf("Message sent.\n");

  printf("%d message(s) sent to message queue.\n", numMsg);

  semGive(sem);

  return 0;
}

int receiveMessage(void)
{
  char buf[512];

  semTake(sem, WAIT_FOREVER);
  printf("Receiving message from message queue...\n");
  semGive(sem);

  msgQReceive(msgQId, buf, sizeof(bigString), WAIT_FOREVER);
  numMsg--;

  semTake(sem, WAIT_FOREVER);
  buf[strlen(bigString)]='\0';
  printf("Received message: %s", buf);
  semGive(sem);

  printf("%d message(s) left in message queue.\n", numMsg);

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
  int fd, num_msgs;
  size_t bread;

  for (;;) {
    printf("-> ");
    memset(buf, 0, 1024);
    bread = read(STDIN_FILENO, buf, 1024);
    printf("Read %d byte(s): ", bread);
    //write(STDOUT_FILENO, buf, bread);
    printf("\n");

    fd = open("/dev/pipe", O_RDWR, 0);
    if (fd == ERROR)
    {
      printf("Unable to open pipe\n");
      continue;
    }

    if (write(fd, buf, bread) == ERROR)
    {
      printf("Unable to write to pipe\n");
      close(fd);
      continue;
    }

    if (ioctl(fd, FIONMSGS, &num_msgs) == OK)
    {
      printf("%d message(s) on pipe\n", num_msgs);
    }

    close(fd);
  }

  return 0;
}

int readTask(void)
{
  size_t bread;
  int fd, result, i, num_msgs;
  fd_set readset;
  struct timeval tv;

  fd = open("/dev/pipe", O_RDWR, 0);
  if (fd == ERROR)
  {
    printf("Unable to open pipe\n");
    return 1;
  }

  while(1)
  {
    taskDelay(10 * DELAY_TIME);
    FD_ZERO(&readset);
    FD_SET(fd, &readset);

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    result = select(fd + 1, &readset, NULL, NULL, &tv);
    if (result > 0)
    {
      num_msgs = 0;
      if (ioctl(fd, FIONMSGS, &num_msgs) == ERROR)
      {
        printf("Unable to get number of messages from pipe\n", num_msgs);
        continue;
      }

      printf("%d message(s) on pipe:\n", num_msgs);
      for (i = 0; i < num_msgs; i++)
      {
        memset(buf, 0, 1024);
        bread = read(fd, buf, 1024);
        if (bread == ERROR)
        {
          printf("Unable to read from pipe\n");
          continue;
        }

        write(STDOUT_FILENO, buf, bread);
      }
    }
    else if (result == 0)
    {
      printf("No data available yet\n");
    }
    else
    {
      printf("Select error\n");
      break;
    }
  }

  close(fd);

  return result;
}

void print_fds(void)
{
  int i;
  int fd = 0;
  static char *fns[3] = { "/pcConsole/0", "/pcConsole/1", "/pcConsole/2" };

  printf(
    "stdin: %d, stdout: %d, stderr: %d\n",
    ioGlobalStdGet(STDIN_FILENO),
    ioGlobalStdGet(STDOUT_FILENO),
    ioGlobalStdGet(STDERR_FILENO)
    );

  for (i = 0; i < 3; i++) {
    fd = open(fns[i], O_RDWR, 0);
    printf("Found fd: %d\n", fd);
    close(fd);
  }

  printf("console drv: %d\n", pcConDrvNumber());
}
#endif

int test_realloc(void)
{
  int slen = strlen(smallString);
  int blen = strlen(bigString);
  char *pData = malloc(slen + 1);
  if (pData == NULL)
  {
    printf("Error allocating small string.\n");
    return 1;
  }
  strcpy(pData, smallString);
  printf(pData);

  pData = realloc(pData, slen + blen + 10);
  if (pData == NULL)
  {
    printf("Error re-allocating plus big string.\n");
    return 1;
  }
  strcpy(&pData[slen], bigString);
  printf(pData);
}

int initTasks(void)
{
  int evtTaskId;
#ifdef RESTART_TASK
  int restartTaskId;
#endif

  printf("\nWelcome to Real VMX...\n");
  printf("This system is released under GNU public license.\n\n");

  if (pipeDevCreate("/dev/pipe", 10, 1024) != OK)
  {
    printf("Unable to initialize pipe device\n");
    for (;;);
  }

  memset(&taskHookCnt, 0, sizeof(taskHookCnt));
  taskCreateHookAdd((FUNCPTR) task_create);
  taskSwitchHookAdd((FUNCPTR) task_switch);
  taskSwapHookAdd((FUNCPTR) task_swapin);
  taskSwapHookAdd((FUNCPTR) task_swapout);
  taskDeleteHookAdd((FUNCPTR) task_delete);

  sem = semCreate(SEM_TYPE_BINARY, SEM_Q_FIFO);
  if (sem == NULL)
  {
    printf("Unable to create semaphore\n");
    for (;;);
  }

  evtSem = semCreate(SEM_TYPE_BINARY, SEM_Q_FIFO);
  if (evtSem == NULL)
  {
    printf("Unable to create event semaphore\n");
    for (;;);
  }

  msgQId = msgQCreate(MAX_MESSAGES + 1, strlen(bigString) + 1, MSG_Q_PRIORITY);
  if (msgQId == NULL)
  {
    printf("Unable to create message queue\n");
    for (;;);
  }

  wdogId = wdCreate();
  if (wdogId == NULL)
  {
    printf("Unable to create watchdog timer\n");
    for (;;);
  }

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

  evtTaskId = taskSpawn("evtHandler", 2, 0,
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
  taskSuspend(evtTaskId);
  taskSwapHookAttach((FUNCPTR) task_swapin, evtTaskId, TRUE, FALSE);
  taskSwapHookAttach((FUNCPTR) task_swapout, evtTaskId, FALSE, TRUE);
  taskResume(evtTaskId);

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

  taskSpawn("readTask", 4, 0,
             DEFAULT_STACK_SIZE, (FUNCPTR) readTask,
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

void usrInit(
    void
    )
{
    sysHwInit0();

    /* Set exception base vector */
    intVecBaseSet((FUNCPTR *) VEC_BASE_ADRS);

    excVecInit();

    /* Initialize hardware */
    sysHwInit();

    /* Initialize kernel libraries */
    usrKernelInit();

    kernelInit(
        (FUNCPTR) usrRoot,
        ROOT_STACK_SIZE,
        (char *) MEM_POOL_START_ADRS,
        (char *) MEM_POOL_START_ADRS + 0x00400000,
        ISR_STACK_SIZE,
        INT_LOCK_LEVEL
        );
}

LOCAL void usrRoot(
    char *pMemPoolStart,
    unsigned memPoolSize
    )
{
  char ttyName[20];
  int i, len;

  intStackEnable(TRUE);

  memPartLibInit(pMemPoolStart, memPoolSize);

  wdLibInit();
  wdShowInit();

  /* Install and start system clock interrupt */
  sysClockConnect((FUNCPTR) usrClock, 0);
  sysClockRateSet(SYS_CLOCK_RATE);
  sysClockEnable();

  pathLibInit();
  iosLibInit(NUM_DRIVERS, NUM_FILES, "/null");
  iosShowInit();

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

  hashLibInit();
  symLibInit();
  symShowInit();

  stdioLibInit();

  excLibInit();
  excShowInit();

  selectLibInit();

  logLibInit(STDERR_FILENO, MAX_LOG_MSGS);

#ifdef INCLUDE_LOG_STARTUP
  logMsg("logging started to %s [%d], queue size %d\n",
         (ARG) consoleName, (ARG) consoleFd, (ARG) MAX_LOG_MSGS,
         (ARG) 4, (ARG) 5, (ARG) 6);
  taskDelay(2);
#endif /* INCLUDE_LOG_STARTUP */

  memLibInit();
  memShowInit();

  pipeDrvInit();

#ifdef INCLUDE_LOG_STARTUP
  logMsg("before symTableCreate()\n",
         (ARG) 1, (ARG) 2, (ARG) 3, (ARG) 4, (ARG) 5, (ARG) 6);
  taskDelay(2);
#endif /* INCLUDE_LOG_STARTUP */

  sysSymTable = symTableCreate(SYM_TABLE_HASH_SIZE_LOG2, TRUE, memSysPartId);
  standTableInit();

#ifdef INCLUDE_LOG_STARTUP
  logMsg("Adding %d symbols for standalone.\n",
         (ARG) standTableSize,
         (ARG) 2, (ARG) 3, (ARG) 4, (ARG) 5, (ARG) 6);
#endif /* INCLUDE_LOG_STARTUP */

  for (i = 0; i < standTableSize; i++)
  {
    symTableAdd(sysSymTable, &standTable[i]);
  }

#ifdef INCLUDE_LOG_STARTUP
  logMsg("sysSymTable complete.\n",
         (ARG) 1, (ARG) 2, (ARG) 3, (ARG) 4, (ARG) 5, (ARG) 6);
  taskDelay(2);
#endif /* INCLUDE_LOG_STARTUP */

  usrLibInit();

  printLogo();

  shellLibInit(SHELL_STACK_SIZE, (ARG) TRUE);
}

