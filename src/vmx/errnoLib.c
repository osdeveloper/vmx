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

/* errnoLib.c - Error types */

#include <stdlib.h>
#include <vmx.h>
#include <errno.h>
#include <vmx/taskLib.h>
#include <vmx/errnoLib.h>

int errno;

/******************************************************************************
 * errnoSet - Set error number
 *
 * RETURNS: OK
 */

STATUS errnoSet(
    int val
    )
{
    errno = val;

    return OK;
}

/******************************************************************************
 * errnoGet - Get error number
 *
 * RETURNS: Error code
 */

int errnoGet(
    void
    )
{
    return errno;
}

/******************************************************************************
 * errnoOfTaskGet - Get error number from task
 *
 * RETURNS: Error code or ERROR
 */

int errnoOfTaskGet(
    int taskId
    )
{
    int ret;
    TCB_ID tcbId;

    if ((taskId == 0) || (taskId == taskIdSelf()))
    {
        ret = errno;
    }
    else
    {
        tcbId = taskTcb(taskId);
        if (tcbId == NULL)
        {
            ret = ERROR;
        }
        else
        {
            ret = tcbId->errorStatus;
        }
    }

    return ret;
}

