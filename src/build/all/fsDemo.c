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
#include <sys/stat.h>
#include <vmx.h>
#include <os/iosLib.h>
#include <os/iosShow.h>
#include <os/pathLib.h>
#include <os/pipeDrv.h>
#include <os/symLib.h>
#include <os/erfLib.h>
#include <fs/xbd.h>
#include <fs/xbdRamDisk.h>
#include <fs/fsMonitor.h>
#include <fs/fsEventUtilLib.h>
#include <fs/fsMonitor.h>
#include <fs/rawfsLib.h>
#include <fs/rt11fsLib.h>

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

int waitForDev(
    char *name,
    BOOL partFlag,
    BOOL openDevice
    )
{
    char partName[10];
    FS_PATH_WAIT_STRUCT pathWait;
    int fd;

    strcpy(partName, name);
    if (partFlag == TRUE)
    {
        strcat(partName, ":0");
    }

    if (fsPathAddedEventSetup(&pathWait, partName) != OK)
    {
        fprintf(stderr, "ERROR: Unable to setup path wait for%s\n", partName);
        return (ERROR);
    }

    printf("Waiting for path: %s...", partName);

    if (fsWaitForPath(&pathWait) != OK)
    {
        fprintf(stderr, "ERROR: Path wait failed for: %s\n", partName);
        return (ERROR);
    }

    printf("Path %s got available.\n", partName);

    if (openDevice == FALSE)
    {
        return (OK);
    }

    fd = open(partName, O_RDWR, 0777);
    if (fd == ERROR)
    {
        fprintf(stderr, "ERROR: Unable to open file: %s\n", partName);
        return (ERROR);
    }

    return (fd);
}

int ramDevCreate(
    char *name,
    BOOL partFlag
    )
{
    device_t device;
    devname_t devname;

    device = xbdRamDiskDevCreate(512, 80 * 512, partFlag, name);
    if (device == ERROR)
    {
        fprintf(stderr, "ERROR: Failed to create device %s.\n", name);
        return (ERROR);
    }

    if (xbdDevName(device, devname) != OK)
    {
        fprintf(stderr, "ERROR: Failed to get device namne.\n");
        return (ERROR);
    }

    printf("Created device with name: %s\n", devname);

    return (OK);
}

int partition(
    char *name,
    int size1,
    int size2,
    int size3
    )
{
    if (!size1)
    {
        size1 = 50;
    }

    if (!size2)
    {
        size2 = 25;
    }

    if (!size3)
    {
        size3 = 25;
    }

    return xbdCreatePartition (name, 3, size1, size2, size3);
}

int geometry(
    int fd
    )
{
    XBD_GEOMETRY xbdGeometry;

    if (ioctl (fd, XBD_GEOMETRY_GET, &xbdGeometry) != OK)
    {
        fprintf(stderr, "ERROR: Unable to get geometry.\n");
        return (ERROR);
    }

    printf("Drive geometry:\n");
    printf("sectorsPerTrack:    %d\n", xbdGeometry.sectorsPerTrack);
    printf("numHeads:           %d\n", xbdGeometry.numHeads);
    printf("numCylinders:       %d\n", xbdGeometry.numCylinders);

    return (OK);
}

int filestat(
    int fd
    )
{
    struct stat st;

    if (fstat (fd, &st) != OK)
    {
        fprintf(stderr, "ERROR: Unable to get file stats.\n");
        return (ERROR);
    }

    printf("File status:\n");
    printf("Total size:      %d\n", st.st_size);
    printf("Block size:      %d\n", st.st_blksize);
    printf("Num blocks:      %d\n", st.st_blocks);

    return (OK);
}

STATUS format(
    char *path,
    int maxEntries
    )
{
    if (maxEntries < 0)
    {
        maxEntries = 0;
    }

    return rt11fsVolFormat (path, maxEntries);
}

int eject(
    char *name
    )
{
    int fd;

    fd = open(name, O_RDWR, 0777);
    if (fd == ERROR)
    {
        fprintf(stderr, "ERROR: Unable to open file: %s\n", name);
        return (ERROR);
    }

    if (ioctl(fd, XBD_HARD_EJECT, 0) != OK)
    {
        fprintf(stderr, "ERROR: Unable to eject device\n");
        return (ERROR);
    }

    close(fd);

    return (OK);
}

int insert(
    int dev_id,
    BOOL partFlag
    )
{
    device_t device = (device_t) dev_id;

    if (erfEventRaise (xbdEventCategory,
                       (partFlag == TRUE) ? xbdEventPrimaryInsert :
                                            xbdEventSecondaryInsert,
                       ERF_ASYNC_PROCESS, (void *) device, NULL) != OK) {
        return (ERROR);
    }

    return (OK);
}

int fsTest1(
    char *name,
    int   num,
    int   size
    )
{
    int    i;
    size_t pos, count;
    int    bread;
    int    fd;
    char   c;
    char  *buf;

    if (num <= 0)
    {
        num = 1;
    }

    if (size <= 0)
    {
        size = 1024;
    }

    buf = malloc(size);
    if (buf == NULL)
    {
        fprintf(stderr, "Unable to allocate memory\n");
        return 1;
    }

    fd = open(name, O_RDWR, 0777);
    if (fd == ERROR)
    {
        free(buf);
        fprintf(stderr, "Unable to open file %s\n", name);
        return 1;
    }

    for (i = 0, pos = 0; i < num; i++)
    {
        c = 'a' + i;
        printf("Write: %c at: %d\n", c, (int) pos);
        memset(buf, c, size);
        lseek(fd, pos, SEEK_SET);
        pos += write(fd, buf, size);
        lseek(fd, 0, SEEK_SET);
        for (count = 0, bread = 0; count < pos; count += bread)
        {
            memset(buf, 0, size);
            bread = read(fd, buf, size);
            write(STDOUT_FILENO, buf, bread);
            getchar();
        }
    }

    free(buf);
    close(fd);

    return 0;
}

void fsDemoInit(
    void
    )
{
    static SYMBOL symTableDemo[] =
    {
        {NULL, "_waitForDev", waitForDev, 0, N_TEXT | N_EXT},
        {NULL, "_ramDevCreate", ramDevCreate, 0, N_TEXT | N_EXT},
        {NULL, "_partition", partition, 0, N_TEXT | N_EXT},
        {NULL, "_geometry", geometry, 0, N_TEXT | N_EXT},
        {NULL, "_filestat", filestat, 0, N_TEXT | N_EXT},
        {NULL, "_format", format, 0, N_TEXT | N_EXT},
        {NULL, "_eject", eject, 0, N_TEXT | N_EXT},
        {NULL, "_insert", insert, 0, N_TEXT | N_EXT},
        {NULL, "_fsTest1", fsTest1, 0, N_TEXT | N_EXT}
    };

    int i;

    for (i = 0; i < NELEMENTS(symTableDemo); i++)
    {
        symTableAdd(sysSymTable, &symTableDemo[i]);
    }
}

