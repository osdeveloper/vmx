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

/* fsDemo.o - Filesystem demo */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <a.out.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vmx.h>
#include <os/iosLib.h>
#include <os/iosShow.h>
#include <os/pathLib.h>
#include <os/pipeDrv.h>
#include <os/symLib.h>
#include <fs/xbd.h>
#include <fs/xbdRamDisk.h>
#include <fs/fsMonitor.h>
#include <fs/fsEventUtilLib.h>
#include <fs/fsMonitor.h>
#include <fs/rawfsLib.h>

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

int rawDevCreate(char *name, BOOL partFlag, BOOL openDevice)
{
  char partName[10];
  char ioName[10];
  device_t device;
  devname_t devname;
  FS_PATH_WAIT_STRUCT pathWait;
  int fd;

  strcpy(partName, name);
  strcat(partName, ":0");

  strcpy(ioName, name);
  strcat(ioName, "$rawfs");

  //fsmNameMap(name, ioName);
  //fsmNameMap(partName, ioName);

  device = xbdRamDiskDevCreate(512, 80 * 512, partFlag, name);
  if (device == ERROR) {

    fprintf(stderr, "ERROR: Failed to create device.\n");
    return (ERROR);

  }

  if (xbdDevName(device, devname) != OK) {

    fprintf(stderr, "ERROR: Failed to get device namn.\n");
    return (ERROR);

  }
  printf("Created device with name: %s\n", devname);

  if (!openDevice)
    return (device);

  if (fsPathAddedEventSetup(&pathWait, ioName) != OK) {

    fprintf(stderr, "ERROR: Unable to setup path wait for%s\n", ioName);
    return (ERROR);

  }

  printf("Waiting for path: %s...", ioName);

  if (fsWaitForPath(&pathWait) != OK) {

    fprintf(stderr, "ERROR: Path wait failed for: %s\n", ioName);
    return (ERROR);

  }

  printf("Path got available.\n");

  fd = open(ioName, O_RDWR, 0777);
  if (fd == ERROR) {

    fprintf(stderr, "ERROR: Unable to open file: %s\n", ioName);
    return (ERROR);

  }

  return (fd);
}

void fsDemoInit(void)
{
    static SYMBOL symTableDemo[] =
    {
        {NULL, "_rawDevCreate", rawDevCreate, 0, N_TEXT | N_EXT}
    };

    int i;

    for (i = 0; i < NELEMENTS(symTableDemo); i++)
    {
        symTableAdd(sysSymTable, &symTableDemo[i]);
    }
}

