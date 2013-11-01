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

/* fread.c - Read from stream */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>

/******************************************************************************
 * fread - Read from stream
 *
 * RETURNS: Number of objects read
 */

size_t fread(
    void *buf,
    size_t size,
    size_t count,
    FILE *fp
    )
{
    size_t len, total;
    char *p;
    int r;
    size_t ret;

    /* Check aruments */
    if ((fp == NULL) || (buf == NULL))
    {
        ret = 0;
    }
    else
    {
        /* Check object */
        if (OBJ_VERIFY(fp, fpClassId) != OK)
        {
            ret = 0;
        }
        else
        {
            /* Calculate length */
            len = count * size;
            if (len == 0)
            {
                ret = 0;
            }
            else
            {
                /* Reset read space left */
                if (fp->_r < 0)
                {
                    fp->_r = 0;
                }

                /* Setup locals for root */
                total = len;
                p = (char *) buf;
                ret = count;

                /* While length is not reached */
                while (len > (r = fp->_r))
                {
                    /* Copy */
                    memcpy(p, fp->_p, (size_t) r);

                    /* Advance */
                    fp->_p += r;
                    p += r;
                    len -= r;

                    /* Fill buffer */
                    if (__srefill(fp))
                    {
                        p = NULL;
                        ret = (total - len) / size;
                        break;
                    }
                }

                if (p != NULL)
                {
                    /* Copy */
                    memcpy(p, fp->_p, len);

                    /* Advance */
                    fp->_r -= len;
                    fp->_p += len;
                }
            }
        }
    }

    return ret;
}

