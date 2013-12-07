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
#include <a.out.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vmx.h>
#include <private/stdioP.h>
#include <private/timeP.h>
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
#include <os/taskVarLib.h>
#include <os/selectLib.h>
#include <os/private/timerLibP.h>
#include <os/unixLib.h>
#include <os/symLib.h>
#include <os/symShow.h>
#include <os/iosLib.h>
#include <os/iosShow.h>
#include <os/pathLib.h>
#include <os/logLib.h>
#include <os/sigLib.h>
#include <os/excLib.h>
#include <os/envLib.h>
#include <os/envShow.h>
#include <os/erfLib.h>
#include <os/cacheLib.h>
#include <os/pipeDrv.h>
#include <os/vmLib.h>
#include <os/vmShow.h>
#include <ostool/shellLib.h>
#include <ostool/dbgLib.h>
#include <ostool/moduleLib.h>
#include <ostool/moduleShow.h>
#include <ostool/loadLib.h>
#include <ostool/loadElfLib.h>
#include <fs/xbd.h>
#include <fs/xbdRamDisk.h>
#include <fs/fsMonitor.h>
#include <fs/fsEventUtilLib.h>
#include <fs/fsMonitor.h>
#include <fs/rawfsLib.h>
#include <fs/rt11fsLib.h>
#include <fs/ext2fsLib.h>
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

IMPORT void fsDemoInit();

/* Globals */
SYMTAB_ID sysSymTable;

LOCAL void usrRoot(
    char *pMemPoolStart,
    unsigned memPoolSize
    );

void usrClock(
    void
    )
{
    vmxTickAnnounce();
}

void usrInit(
    void
    )
{
    sysHwInit0();

#ifdef INCLUDE_CACHE_SUPPORT
    cacheLibInit(USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#endif /* INCLUDE_CACHE_SUPPORT */

    /* Set exception base vector */
    intVecBaseSet((FUNCPTR *) VEC_BASE_ADRS);

    excVecInit();

    /* Initialize hardware */
    sysHwInit();

    /* Initialize kernel libraries */
    usrKernelInit();

#ifdef INCLUDE_CACHE_SUPPORT
#ifdef USER_I_CACHE_ENABLE
    cacheEnable(INSTRUCTION_CACHE);
#endif /* USER_I_CACHE_ENABLE */

#ifdef USER_D_CACHE_ENABLE
    cacheEnable(DATA_CACHE);
#endif /* USER_D_CACHE_ENABLE */
#endif /* INCLUDE_CACHE_SUPPORT */

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
  static char ttyName[20];
  static char consoleName[20];

  int i;
  int len;
  int consoleFd;

  intStackEnable(TRUE);

  memPartLibInit(pMemPoolStart, memPoolSize);

  memLibInit();
  memShowInit();

  wdLibInit();
  wdShowInit();

#ifdef INCLUDE_MMU
  usrMmuInit();
  vmShowInit();
#endif /* INCLUDE_MMU */

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
  timeLibInit();
  clockLibInit();

  excLibInit();
  excShowInit();

  taskVarLibInit();
  selectLibInit();
  unixLibInit();

  logLibInit(STDERR_FILENO, MAX_LOG_MSGS);

  sigLibInit();

#ifdef INCLUDE_LOG_STARTUP
  logMsg("logging started to %s [%d], queue size %d\n",
         (ARG) consoleName, (ARG) consoleFd, (ARG) MAX_LOG_MSGS,
         (ARG) 4, (ARG) 5, (ARG) 6);
  taskDelay(2);
#endif /* INCLUDE_LOG_STARTUP */

  pipeDrvInit();

  sigqueueInit(NUM_SIGNAL_QUEUES);

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

  moduleLibInit();
  moduleShowInit();
  loadLibInit();
  loadElfLibInit();

  erfLibInit(MAX_EVENT_CATEGORIES, MAX_EVENT_TYPES);

  envLibInit(TRUE);
  envShowInit();

  xbdLibInit(MAX_XBD_DEVICES);
  xbdPartitionLibInit();
  fsEventUtilInit();
  fsMonitorInit();
  vfsInit();
  rawfsLibInit(MAX_FS_BUFFERS, MAX_FS_FILES, 0, 0);
  usrRawfsInit(MAX_FS_BUFFERS, MAX_FS_FILES, 0, 0);
  rt11fsLibInit(MAX_FS_BUFFERS, MAX_FS_FILES, MAX_FS_DIR_ENTRIES, 0);
  usrRt11fsInit(MAX_FS_BUFFERS, MAX_FS_FILES, MAX_FS_DIR_ENTRIES, 0);
  usrExt2fsInit(MAX_FS_BUFFERS, MAX_FS_FILES, 0, 0);

#ifdef INCLUDE_NET
  usrNetInit("");
#endif /* INCLUDE_NET */

  usrLibInit();

  printLogo();

#ifdef INCLUDE_USR_TEST
  usrTestInit();
#endif /* INCLUDE_USR_TEST */

#ifdef INCLUDE_FS_DEMO
  fsDemoInit();
#endif /* INCLUDE_FS_DEMO */

#ifdef INCLUDE_NET_DEMO
  netDemoInit();
#endif /* INCLUDE_NET_DEMO */

#ifdef INCLUDE_UGL_DEMO
  uglDemoInit();
#endif /* INCLUDE_UGL_DEMO */

  shellLibInit(SHELL_STACK_SIZE, (ARG) TRUE);
  dbgLibInit();
}

