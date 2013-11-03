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

/* fdopen.c - Create stream assisiated with file descriptor */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vmx.h>
#include <os/iosLib.h>

/******************************************************************************
 * Create stream for file descritptor
 *
 * RETURNS: File stream or NULL
 */

FILE* fdopen(
    int fd,
    const char *mode
    )
{
    FILE *fp;
    int flags, oflags;

    /* Check file descriptor */
    if (iosFdValue(fd) == (ARG) 0)
    {
        fp = NULL;
    }
    else
    {
        /* Get flags */
        flags = __sflags(mode, &oflags);
        if (flags == 0)
        {
            fp = NULL;
        }
        else
        {
            /* Create stream object */
            fp = stdioFpCreate();
            if (fp != NULL)
            {
                /* Setup FILE struct */
                fp->_flags = flags;

                if (oflags & O_APPEND)
                {
                    fp->_flags |= __SAPP;
                }

                fp->_file = fd;
            }
        }
    }

    return fp;
}

