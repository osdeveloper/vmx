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

/* usrFsLib.c - User filesystem tools */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <os/ioLib.h>
#include <os/pathLib.h>
#include <usr/usrFsLib.h>

/* Locals */
LOCAL void usrPathCat(
    const char *dirname,
    const char *filename,
    char *result
    );

LOCAL BOOL nameHasWildcard(
    const char *path
    );

LOCAL BOOL nameIsDir(
    const char *path
    );

LOCAL BOOL dirListPattern(
    char *pat,
    char *path
    );

LOCAL STATUS dirListEntry(
    FILE *fp,
    char *filename,
    struct stat *filestat,
    char *prefix,
    BOOL doLong
    );

LOCAL STATUS copyStreams(
    int inFd,
    int outFd
    );

/******************************************************************************
 * ioHelp - Print io help
 *
 * RETURNS: N/A
 */

void ioHelp(
    void
    )
{
    static char *helpMessages[] =
    {
        "cd        \"path\"             Set current working path",
        "pwd                            Print working path",
        "ls        [\"wpat\"[,long]]    List contents of directory",
        "ll        [\"wpat\"]           List contents of directory - long format",
        "lsr       [\"wpat\"[,long]]    Recursive list of directory contents",
        "llr       [\"wpat\"]           Recursive detailed list of directory",
        "copy      [\"in\"][,\"out\"]   Copy in file to out file (0 = stdin/out)",
        ""
    };

    int i;

    for (i = 0; i < NELEMENTS(helpMessages); i++)
    {
        printf(helpMessages[i]);
        printf("\n");
    }

    printf("\n");
}

/******************************************************************************
 * cd - Change working directory
 *
 * RETURNS: OK or ERROR
 */

STATUS cd(
    const char *path
    )
{
    STATUS status;

    if (ioDefPathCat((char *) path) != OK)
    {
        fprintf(stderr, "cd: error = %#x\n", errnoGet());
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * pwd - Printf working directory
 *
 * RETURNS: N/A
 */

void pwd(
    void
    )
{
    char path[PATH_MAX];

    ioDefPathGet(path);
    printf("%s\n", path);
}

/******************************************************************************
 * dirList - List directory
 *
 * RETURNS: OK or ERROR
 */

STATUS dirList(
    FILE *fp,
    char *dirname,
    BOOL  doLong,
    BOOL  doTree
    )
{
    struct dirent *pDirEnt;
    struct stat    fileStat;
    char           filename[PATH_MAX];
    char           dirName[PATH_MAX];
    DIR           *pDir   = NULL;
    char          *pPat   = NULL;
    STATUS         status = OK;

    if (dirname == NULL)
    {
        strcpy(dirName, ".");
    }
    else
    {
        strcpy(dirName, dirname);
    }

    /* Open directory */
    pDir = opendir(dirName);
    if (pDir == NULL)
    {
        pPat = strrchr(dirName, '/');
        if ((pPat != NULL) && (pPat != dirName) &&
            (nameHasWildcard(pPat) == TRUE))
        {
            *pPat++ = EOS;
            pDir = opendir(dirName);
        }
        else if (nameHasWildcard(dirName) == TRUE)
        {
            pPat = dirName;
            strcpy(dirName, ".");
            pDir = opendir(dirName);
        }
    }

    if (pDir == NULL)
    {
        goto dirListError;
    }

    if (strcmp(dirName, ".") == 0)
    {
        strcpy(dirName, "");
    }
    else if (doLong == TRUE)
    {
        fprintf(fp, "\nListing Directory %s:\n", dirName);
    }

    do
    {
        errnoSet(OK);
        pDirEnt = readdir(pDir);

        if (pDirEnt != NULL)
        {
            if ((pPat != NULL) &&
                (dirListPattern(pPat, pDirEnt->d_name) == FALSE))
            {
                continue;
            }

            if (doLong == TRUE)
            {
                usrPathCat(dirName, pDirEnt->d_name, filename);
                if (stat(filename, &fileStat) != OK)
                {
                    memset(&fileStat, 0, sizeof(struct stat));
                }
            }

            if (dirListEntry(
                    fp,
                    pDirEnt->d_name,
                    &fileStat,
                    dirName,
                    doLong
                    ) == ERROR)
            {
                status = ERROR;
            }
        }
        else
        {
            if (errnoGet() != OK)
            {
                fprintf(
                    stderr,
                    "error reading dir %s, errno: %x\n",
                    dirName,
                    errnoGet()
                    );
                status = ERROR;
            }
        }
    } while (pDirEnt != NULL);

    status |= closedir(pDir);

    if (doTree == TRUE)
    {
        /* Process subdirectories */
        if (dirName[0] == EOS)
        {
            pDir = opendir(".");
        }
        else
        {
            pDir = opendir(dirName);
        }

        if (pDir == NULL)
        {
            goto dirListError;
        }

        do
        {
            errnoSet(OK);

            pDirEnt = readdir(pDir);
            if (pDirEnt != NULL)
            {
                if ((pPat != NULL) &&
                    (dirListPattern(pPat, pDirEnt->d_name) == FALSE))
                {
                    continue;
                }

                usrPathCat(dirName, pDirEnt->d_name, filename);

                if (stat(filename, &fileStat) != OK)
                {
                    memset(&fileStat, 0, sizeof(struct stat));
                }

                /* Ignore dot and dot-dot directories */
                if (S_ISDIR(fileStat.st_mode) &&
                    strcmp(pDirEnt->d_name, ".") &&
                    strcmp(pDirEnt->d_name, ".."))
                {
                    status = dirList(fp, filename, doLong, doTree);
                }
            }
            else
            {
                if (errnoGet() != OK)
                {
                    fprintf(
                        fp,
                        "error reading dir %s, errno: %x\n",
                        dirName,
                        errnoGet()
                        );
                    status = ERROR;
                }
            }
        } while (pDirEnt != NULL);

        /* Close directory */
        status |= closedir(pDir);
    }

    return status;

dirListError:

    fprintf(fp, "Can't open \"%s\".\n", dirName);

    return ERROR;
}

/******************************************************************************
 * ls - List directory
 *
 * RETURNS: OK or ERROR
 */

STATUS ls(
    char *dirname,
    BOOL  doLong
    )
{
    return dirList(stdout, dirname, doLong, FALSE);
}

/******************************************************************************
 * ll - List directory (long version)
 *
 * RETURNS: OK or ERROR
 */

STATUS ll(
    char *dirname
    )
{
    return dirList(stdout, dirname, TRUE, FALSE);
}

/******************************************************************************
 * lsr - List directory and subdirectories
 *
 * RETURNS: OK or ERROR
 */

STATUS lsr(
    char *dirname
    )
{
    return dirList(stdout, dirname, FALSE, TRUE);
}

/******************************************************************************
 * llr - List directory and subdirectories (long version)
 *
 * RETURNS: OK or ERROR
 */

STATUS llr(
    char *dirname
    )
{
    return dirList(stdout, dirname, TRUE, TRUE);
}

/******************************************************************************
 * copy - Copy a file to another file
 *
 * RETURNS: OK or ERROR
 */

STATUS copy(
    const char *inFilename,
    const char *outFilename
    )
{
    int    inFd;
    int    outFd;
    STATUS status;

    /* Get input file descriptor */
    if (inFilename != NULL)
    {
        inFd = open(inFilename, O_RDONLY, 0);
    }
    else
    {
        inFd = STDIN_FILENO;
    }

    if (inFd == ERROR)
    {
        fprintf(
            stderr,
            "Can't open input file \"%s\" errno = %#x\n",
            inFilename,
            errnoGet()
            );
        return ERROR;
    }

    /* If output to directory */
    if (nameIsDir(outFilename))
    {
        close(inFd);
        return ERROR;
    }

    /* Get output file descriptor */
    if (outFilename != NULL)
    {
        outFd = creat(outFilename, O_WRONLY);
    }
    else
    {
        outFd = STDOUT_FILENO;
    }

    if (inFd == ERROR)
    {
        fprintf(
            stderr,
            "Can't write to file \"%s\" errno = %#x\n",
            outFilename,
            errnoGet()
            );

        if (inFilename != NULL)
        {
            close(inFd);
        }

        return ERROR;
    }

    /* Copy streams */
    status = copyStreams(inFd, outFd);

    if (inFilename != NULL)
    {
        close(inFd);
    }

    if (outFilename != NULL)
    {
        if (close(outFd) == ERROR)
        {
            status = ERROR;
        }
    }

    return status;
}

/******************************************************************************
 * usrPathCat - Concatenate directory to path
 *
 * RETURNS: N/A
 */

LOCAL void usrPathCat(
    const char *dirname,
    const char *filename,
    char       *result
    )
{
    *result = EOS;

    if ((dirname != NULL) && (dirname[0] != EOS) &&
        (strcmp(dirname, ".") != 0))
    {
        strcpy(result, dirname);
        strcat(result, "/");
    }

    strcat(result, filename);
}

/******************************************************************************
 * nameHasWildcard - Determine if name name contains wildcard
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL nameHasWildcard(
    const char *path
    )
{
    BOOL ret;

    if ((strchr(path, '*') != NULL) || (strchr(path, '?') != NULL))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

/******************************************************************************
 * nameIsDir - Determine if a name is a directory
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL nameIsDir(
    const char *path
    )
{
    BOOL        ret;
    struct stat st;

    if ((path == NULL) || (path[0] == EOS))
    {
        ret = FALSE;
    }
    else
    {
        if (stat((char *) path, &st) == ERROR)
        {
            ret = FALSE;
        }
        else if (S_ISDIR(st.st_mode))
        {
            ret = TRUE;
        }
    }

    return ret;
}

/******************************************************************************
 * dirListPattern - Match file pattern with filename
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL dirListPattern(
    char *pat,
    char *path
    )
{
    const char matchOne  = '?';
    const char matchMany = '*';
    BOOL       ret       = TRUE;
    char      *pPat      = pat;
    char      *pNext;

    for (pNext = path; *pNext != EOS; pNext++)
    {
        if (*pPat == matchOne)
        {
            pPat++;
        }
        else if (*pPat == matchMany)
        {
            if (pNext[0] == '.')
            {
                ret = FALSE;
                break;
            }

            if (__toupper(pPat[0]) == __toupper(pNext[1]))
            {
                pPat++;
            }
            else if (__toupper(pPat[1]) == __toupper(pNext[0]))
            {
                pPat += 2;
            }
            else
            {
                continue;
            }
        }
        else if (__toupper(*pPat) != __toupper(*pNext))
        {
            ret =  FALSE;
        }
        else
        {
            pPat++;
        }
    }

    if (ret == TRUE)
    {
        if (!((pPat[0] == EOS) || ((pPat[0] == matchMany) && (pPat[1] == EOS))))
        {
            ret = FALSE;
        }
    }

    return ret;
}

/******************************************************************************
 * dirListEntry - List file or directory
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS dirListEntry(
    FILE        *fp,
    char        *filename,
    struct stat *filestat,
    char        *prefix,
    BOOL         doLong
    )
{
    STATUS status;
    char   type;
    char   modbits[10];
    char   suffix = ' ';
    char   monthName[3];
    time_t now;
    struct tm nowDate;
    struct tm fileDate;

    if (doLong == TRUE)
    {
        if (S_ISDIR(filestat->st_mode))
        {
            type   = 'd';
            suffix = '/';
        }
        else if (S_ISREG(filestat->st_mode))
        {
            type = '-';
        }
        else
        {
            type = '?';
        }

        strcpy(modbits, "---------");
        modbits[0] = (filestat->st_mode & S_IRUSR) ? 'r' : '-';
        modbits[1] = (filestat->st_mode & S_IWUSR) ? 'w' : '-';
        modbits[2] = (filestat->st_mode & S_IXUSR) ? 'x' : '-';
        modbits[3] = (filestat->st_mode & S_IRGRP) ? 'r' : '-';
        modbits[4] = (filestat->st_mode & S_IWGRP) ? 'w' : '-';
        modbits[5] = (filestat->st_mode & S_IXGRP) ? 'x' : '-';
        modbits[6] = (filestat->st_mode & S_IROTH) ? 'r' : '-';
        modbits[7] = (filestat->st_mode & S_IWOTH) ? 'w' : '-';
        modbits[8] = (filestat->st_mode & S_IXOTH) ? 'x' : '-';
        modbits[9] = EOS;

        /* Print mode and permissions */
        fprintf(fp, "%c%9s", type, modbits);

        /* Print link, usr and group fields */
        fprintf(
            fp,
            " %2d %-7d %-7d",
            filestat->st_nlink,
            filestat->st_uid,
            filestat->st_gid
            );

        /* Print size */
        fprintf(fp, " %9lu ", filestat->st_size);

        /* Get current time */
        now = time(&now);
        localtime_r(&now, &nowDate);

        /* Get file timestamp */
        localtime_r(&filestat->st_mtime, &fileDate);
        strftime(monthName, 3, "%b", &fileDate);

        /* If file changed this year */
        if (fileDate.tm_year == nowDate.tm_year)
        {
            fprintf(
                fp,
                "%s %2d %02d:%02d ",
                monthName,
                fileDate.tm_mday,
                fileDate.tm_hour,
                fileDate.tm_min
                );
        }
        else
        {
            fprintf(
                fp,
                "%s %2d %04d ",
                monthName,
                fileDate.tm_mday,
                fileDate.tm_year + 1900
                );
        }
    }
    else
    {
        if ((strcmp(prefix, ".") == 0) || (prefix[0] == EOS))
        {
            /* Don not print "." prefix */
        }
        else if ((prefix != NULL) && (prefix[strlen(prefix) - 1] != '/'))
        {
            fprintf(fp, "%s/", prefix);
        }
        else if (prefix != NULL)
        {
            fprintf(fp, prefix);
        }
    }

    if (fprintf(fp, "%s%c\n", filename, suffix) == ERROR)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * copyStreams - Copy between two streams
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS copyStreams(
    int inFd,
    int outFd
    )
{
    char  *buf;
    int    nBytes;
    size_t bufSize;
    int    totBytes = 0;
    STATUS status   = OK;

    /* Get buffer size */
    if (ioctl(inFd, FIONREAD, (int) bufSize) == ERROR)
    {
        bufSize = 1024;
    }

    /* Get buffer */
    bufSize = min(0x10000, bufSize);
    buf = (char *) malloc(bufSize);
    if (buf == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* While bytes left */
        while ((nBytes = read(inFd, buf, bufSize)) > 0)
        {
            /* If write fails */
            if (write(outFd, buf, nBytes) != nBytes)
            {
                fprintf(
                    stderr,
                    "copy: error writing file. errno %#x\n",
                    errnoGet()
                    );
                free(buf);
                status = ERROR;
                break;
            }

            /* Advance */
            totBytes += nBytes;
        }

        if (status != ERROR)
        {
            free(buf);

            /* If read error */
            if (nBytes < 0)
            {
                fprintf(
                    stderr,
                    "copy: error reading file after copying %d bytes.\n",
                    totBytes
                    );
                status = ERROR;
            }
            else
            {
                printf("Copy OK: %u bytes copied\n", totBytes);
            }
        }
    }

    return status;
}

