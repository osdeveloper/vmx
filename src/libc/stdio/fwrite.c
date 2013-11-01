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

/* fwrite.c - Write to stream */

#include <stdio.h>
#include <vmx.h>

/******************************************************************************
 * fwrite - Write data to stream
 *
 * RETURNS: Number of objects written
 */

size_t fwrite(
    const void *buf,
    size_t size,
    size_t count,
    FILE *fp
    )
{
    size_t i, len, ret;
    struct __suio uio;
    struct __siov iov;

    /* Check object */
    if (OBJ_VERIFY(fp, fpClassId) != OK)
    {
        ret = 0;
    }
    else
    {
        /* Setup locals */
        len = count * size;

        iov.iov_base   = (void *) buf;
        iov.iov_len    = len;

        uio.uio_resid  = len;
        uio.uio_iov    = &iov;
        uio.uio_iovcnt = 1;

        /* Write */
        if (__sfvwrite(fp, &uio) == 0)
        {
            ret = count;
        }
        else
        {
            ret = (len - uio.uio_resid) / size;
        }
    }

    return ret;
}

