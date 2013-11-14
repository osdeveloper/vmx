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

/* time.c - Time functions */

#include <stdlib.h>
#include <time.h>
#include <vmx.h>

/******************************************************************************
 * time - Get current calendar time
 *
 * RETURNS: Current time
 */

time_t time(
    time_t *timer
    )
{
    time_t          ret;
    struct timespec tp;

    if (clock_gettime(CLOCK_REALTIME, &tp) != OK)
    {
        ret = ERROR;
    }
    else
    {
        if (*timer != NULL)
        {
            *timer = (time_t) tp.tv_sec;
        }

        ret = (time_t) tp.tv_sec;
    }
}

