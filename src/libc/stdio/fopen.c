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

/* fopen.c - Open stream */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <vmx.h>

/******************************************************************************
 * fopen - Open stream
 *
 * RETURNS: File steam or NULL
 */

FILE* fopen(
    const char *file,
    const char *mode
    )
{
    FILE *fp;
    int fd, flags, oflags;

    /* Get flags for given mode */
    flags = __sflags(mode, &oflags);
    if (flags == 0)
    {
        fp = NULL;
    }
    else
    {
        /* Create stream object */
        fp = stdioFpCreate();

        /* Open file */
        fd = open(file, oflags, DEFFILEMODE);
        if (fd < 0)
        {
            fp->_flags = 0;
            stdioFpDestroy(fp);
            fp = NULL;
        }
        else
        {
            /* Setup FILE struct */
            fp->_file = fd;
            fp->_flags = flags;

            /* If append, goto end of file */
            if (oflags & O_APPEND)
            {
                __sseek(fp, (fpos_t) 0, SEEK_END);
            }
        }
    }

    return fp;
}

