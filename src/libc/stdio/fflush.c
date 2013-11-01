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

/* fflush.c - Flush file */

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <vmx/errnoLib.h>

/******************************************************************************
 * fflush - Flush stdio buffers
 *
 * RETURNS: 0 or EOF
 */

int fflush(
    FILE *fp
    )
{
    int ret;

    /* Check fp */
    if (fp == NULL)
    {
        errnoSet(EBADF);
        ret = EOF;
    }
    else
    {
        /* Check class */
        if (OBJ_VERIFY(fp, fpClassId) != OK)
        {
            errnoSet(EBADF);
            ret = EOF;
        }
        else
        {
            /* Check flags */
            if ((fp->_flags & __SWR) == 0)
            {
                errnoSet(EBADF);
                ret = EOF;
            }
            else
            {
                /* Flush */
                ret = __sflush(fp);
            }
        }
    }

    return ret;
}

