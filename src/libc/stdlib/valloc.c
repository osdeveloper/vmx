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

/* valloc.c - Allocate page aligned memory block */

#include <stdlib.h>
#include <os/memPartLib.h>
#include <os/vmLib.h>

/******************************************************************************
 * valloc - Allocate memory that begins at page boundary
 *
 * RETURNS: Pointer to memory or NULL
 */

void* valloc(
    size_t size
    )
{
    int   pageSize;
    void *pData;

    /* Get page size */
    pageSize = vmPageSizeGet();
    if (pageSize == ERROR)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        pData = NULL;
    }
    else
    {
        pData = memalign(pageSize, size);
    }

    return pData;
}

