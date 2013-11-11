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

/* erfLib.c - event reporting framework */

/*
 * The event reporting framework will be used with the file system framework.
 * It may also be used without file systems for other purposes.
 */

/* includes */

#include <vmx.h>

#include <stdlib.h>
#include <string.h>

#include <vmx/msgQLib.h>
#include <vmx/semLib.h>
#include <vmx/taskLib.h>

#include <os/erfLib.h>

/* defines */

#define ERF_TASK_PRIORITY    50    /* currently an arbitrary priority level */
#define ERF_TASK_STACK_SIZE  8192  /* currently an arbitrary stack size */

/* structs */

struct erfType;

/* typedefs */

typedef struct erfType {
    struct erfType *  next;
    void    (*func)(int, int, void *, void*);
    void *  pUserData;
} ERF_TYPE;

typedef struct erfCategory {
    int          typesUsed;
    ERF_TYPE **  type;
} ERF_CATEGORY;

typedef struct erfMsg {
    int     category;
    int     type;
    void    (*pFreeFunc)(void *);
    void *  pEventData;
} ERF_MSG;

typedef struct erfHdl {
    int     category;
    int     type;
    void    (*func)(int, int, void *, void*);
    void *  pUserData;
} ERF_HDL;

/* forward declarations */

LOCAL STATUS erfTask (
    void
    );

LOCAL void   erfHandler (
    int    category,
    int    type,
    void * pEventData,
    void * pUserData
    );

/* locals */

LOCAL ERF_CATEGORY *  erfTable;
LOCAL int             erfCategories;    /* maximum # of user categories */
LOCAL int             erfTypes;         /* maximum # of user types */
LOCAL int             erfCatUsed;       /* # of user categories in use */
LOCAL int             erfTid;
LOCAL MSG_Q_ID        erfMsgQ;
LOCAL SEM_ID          erfMutex;

LOCAL int             erfEventCategory          = -1;
LOCAL int             erfEventHandlerRegister   = -1;
LOCAL int             erfEventHandlerUnregister = -1;

/* globals */
int                   erfTaskPriority  = ERF_TASK_PRIORITY;
int                   erfTaskStackSize = ERF_TASK_STACK_SIZE;

/***************************************************************************
 * erfLibInit - Initialize event reporting framework
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfLibInit (
    int  maxCategories,
    int  maxTypes
    )
{
    char *  data;
    int     size;
    int     i;

    if (erfTable != NULL) {
        return (OK);
    }

    size  = maxCategories * sizeof (ERF_CATEGORY);
    size += maxCategories * maxTypes * sizeof (ERF_TYPE **);

    data = (char *) malloc (size);
    if (data == NULL) {
        /* errno handled by malloc() */
        return (ERROR);
    }

    erfCategories = maxCategories;
    erfTypes = maxTypes;
    memset (data, 0, size);

    erfTable = (ERF_CATEGORY *) data;
    data += maxCategories * sizeof(ERF_CATEGORY);

    for (i = 0; i < maxCategories; i++) {
        erfTable[i].type = (ERF_TYPE **) data;
        data += (maxTypes * sizeof (ERF_TYPE **));
    }

    erfCatUsed = 0;

    erfMutex = semMCreate (SEM_Q_PRIORITY);
    if (erfMutex == NULL) {
        /* errno handled by semMCreate() */
        free (erfTable);
        erfTable = NULL;
        return (ERROR);
    }

    erfMsgQ = msgQCreate (32, sizeof (ERF_MSG), MSG_Q_PRIORITY);
    if (erfMsgQ == NULL) {
        /* errno handled by msgQCreate() */
        free (erfTable);
        semDelete (erfMutex);
        erfTable = NULL;
        return (ERROR);
    }

    if ((erfCategoryAllocate (&erfEventCategory) != OK) ||
        (erfTypeAllocate (erfEventCategory,
                          &erfEventHandlerRegister) != OK) ||
        (erfTypeAllocate (erfEventCategory,
                          &erfEventHandlerUnregister) != OK)) {
        /* errno set by ERF routines */
        free (erfTable);
        semDelete (erfMutex);
        msgQDelete (erfMsgQ);
        erfTable = NULL;
        return (ERROR);
    }

    if ((erfHandlerRegister (erfEventCategory, erfEventHandlerRegister,
                             erfHandler, NULL, 0) != OK) ||
        (erfHandlerRegister (erfEventCategory, erfEventHandlerUnregister,
                             erfHandler, NULL, 0) != OK)) {
        /* errno set by ERF routines */
        free (erfTable);
        semDelete (erfMutex);
        msgQDelete (erfMsgQ);
        erfTable = NULL;
        return (ERROR);
    }

    erfTid = taskSpawn ("tErfTask", erfTaskPriority, 0, erfTaskStackSize,
                         erfTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (erfTid == ERROR) {
        /* errno handled by taskSpawn() */
        free (erfTable);
        semDelete (erfMutex);
        msgQDelete (erfMsgQ);
        erfTable = NULL;
        return (ERROR);
    }

    return (OK);
}

/***************************************************************************
 * erfHandlerRegister - Register event handler
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfHandlerRegister (
    int     eventCategory,
    int     eventType,
    void    (*func)(int, int, void *, void*),
    void *  pUserData,
    int     flags
    )
{
    ERF_TYPE *     erfType;
    ERF_HDL  *     pErfHdl;
    STATUS         status;

    if ((func == NULL) || (eventCategory >= erfCatUsed) ||
        (eventType >= erfTable[eventCategory].typesUsed)) {
        errnoSet (S_erfLib_INVALID_PARAMETER);
        return (ERROR);
    }

    status = OK;

    if (!flags) {

        erfType = (ERF_TYPE *) malloc (sizeof (ERF_TYPE));
        if (erfType == NULL) {
            /* errno set by malloc() */
            return (ERROR);
        }

        erfType->func = func;
        erfType->pUserData = pUserData;

        semTake (erfMutex, WAIT_FOREVER);

        erfType->next = erfTable[eventCategory].type[eventType];
        erfTable[eventCategory].type[eventType] = erfType;

        semGive (erfMutex);

    } else {

        pErfHdl = (ERF_HDL *) malloc (sizeof (ERF_HDL));
        if (pErfHdl == NULL) {
            /* errno set by malloc() */
            return (ERROR);
        }

        pErfHdl->category  = eventCategory;
        pErfHdl->type      = eventType;
        pErfHdl->func      = func;
        pErfHdl->pUserData = pUserData;

        status = erfEventRaise (erfEventCategory, erfEventHandlerRegister,
                                ERF_ASYNC_PROCESS, pErfHdl, free);

    }

    return (status);
}

/***************************************************************************
 * erfHandlerUnregister - Unregister event handler
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfHandlerUnregister (
    int     eventCategory,
    int     eventType,
    void    (*func)(int, int, void *, void*),
    void *  pUserData
    )
{
    ERF_HDL  *     pErfHdl;
    STATUS         status;

    if ((eventCategory >= erfCatUsed) ||
        (eventType >= erfTable[eventCategory].typesUsed)) {
        errnoSet (S_erfLib_INVALID_PARAMETER);
        return (ERROR);
    }

    pErfHdl = (ERF_HDL *) malloc (sizeof (ERF_HDL));
    if (pErfHdl == NULL) {
        /* errno set by malloc() */
        return (ERROR);
    }

    /* Inititalize handler identifier */
    pErfHdl->category  = eventCategory;
    pErfHdl->type      = eventType;
    pErfHdl->func      = func;
    pErfHdl->pUserData = pUserData;

    status = erfEventRaise (erfEventCategory, erfEventHandlerUnregister,
                            ERF_ASYNC_PROCESS, pErfHdl, free);

    return (status);
}

/***************************************************************************
 * erfEventRaise - raise an event
 * 
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfEventRaise (
    int     eventCategory,
    int     eventType,
    int     processType,
    void *  pEventData,
    void    (*freeFunc)(void *)
    )
{
    ERF_TYPE *  erfType;
    ERF_MSG     erfMsg;
    STATUS      status;

    if ((eventCategory >= erfCatUsed) ||
        (eventType >= erfTable[eventCategory].typesUsed)) {
        errnoSet (S_erfLib_INVALID_PARAMETER);
        return (ERROR);
    }

    if (processType == ERF_ASYNC_PROCESS) {

        erfMsg.category   = eventCategory;
        erfMsg.type       = eventType;
        erfMsg.pFreeFunc  = freeFunc;
        erfMsg.pEventData = pEventData;

        status = msgQSend (erfMsgQ, &erfMsg, sizeof (ERF_MSG),
                           WAIT_NONE, MSG_PRI_NORMAL);
        return (status);
    }

    semTake (erfMutex, WAIT_FOREVER);

    erfType = erfTable[eventCategory].type[eventType];
    while (erfType != NULL) {
        erfType->func (eventCategory, eventType,
                       pEventData, erfType->pUserData);

        erfType = erfType->next;
    }

    semGive (erfMutex);

    if (freeFunc != NULL) {
        freeFunc (pEventData);
    }

    return (OK);
}

/***************************************************************************
 * erfCategoryAllocate - Allocate event category
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfCategoryAllocate (
    int *  pCategory
    )
{

    if (pCategory == NULL) {
        errnoSet (S_erfLib_INVALID_PARAMETER);
        return (ERROR);
    }

    semTake (erfMutex, WAIT_FOREVER);

    if (erfCatUsed == erfCategories) {
        semGive (erfMutex);
        errnoSet (S_erfLib_TOO_MANY_USER_CATS);
        return (ERROR);
    }

    /*
     * Presently, neither categories nor types can be deleted.  Thus, only the
     * number of allocated categories needs to be incremented.  Also, there is
     * no need to initialize [typesUsed] as that was done when the table was
     * initialized.
     */

    *pCategory = erfCatUsed;
    erfCatUsed++;

    semGive (erfMutex);

    return (OK);
}

/***************************************************************************
 * erfTypeAllocate - Allocate event type
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfTypeAllocate (
    int    category,
    int *  pType
    )
{

    if ((category >= erfCatUsed) || (pType == NULL)) {
        errnoSet (S_erfLib_INVALID_PARAMETER);
        return (ERROR);
    }

    semTake (erfMutex, WAIT_FOREVER);

    if (erfTable[category].typesUsed == erfTypes) {
        semGive (erfMutex);
        errnoSet (S_erfLib_TOO_MANY_USER_TYPES);
        return (ERROR);
    }

    /*
     * Presently, neither categories, nor types can be deleted.
     * The ERF_TYPE pointer is already NULL from table initialization.
     * Thus, only the number of allocated types needs to be incremented.
     */

    *pType = erfTable[category].typesUsed;
    erfTable[category].typesUsed++;

    semGive (erfMutex);

    return (OK);
}

/***************************************************************************
 * erfTask - Event handler task
 *
 * RETURNS: Not expected to return
 */

LOCAL STATUS erfTask (
    void
    )
{
    ERF_MSG     erfMsg;
    ERF_TYPE *  erfType;
    int  eventCategory;
    int  eventType;

    while (1) {

        if (msgQReceive (erfMsgQ, &erfMsg, sizeof (ERF_MSG),
                         WAIT_FOREVER) == sizeof(ERF_MSG)) {

            eventCategory = erfMsg.category;
            eventType     = erfMsg.type;

            semTake (erfMutex, WAIT_FOREVER);

            erfType = erfTable[eventCategory].type[eventType];
            while (erfType != NULL) {
                erfType->func (eventCategory, eventType,
                               erfMsg.pEventData, erfType->pUserData);

                erfType = erfType->next;
            }

            semGive (erfMutex);

            if (erfMsg.pFreeFunc != NULL) {
                erfMsg.pFreeFunc (erfMsg.pEventData);
            }
        }
    }

    /* XXX Do we want to restart this task? XXX */

    return (ERROR);
}

/***************************************************************************
 * erfHandler - Event handler function
 *
 * RETURNS: N/A
 */

LOCAL void erfHandler (
    int     category,      /* event category */
    int     type,          /* event type */
    void *  pEventData,    /* event data */
    void *  pUserData      /* not used */
    )
{
    ERF_HDL *   pErfHdl;
    ERF_TYPE *  erfType;
    ERF_TYPE *  prevErfType;

    if (category != erfEventCategory) {
        return;
    }

    /* Get handler identifier */
    pErfHdl = (ERF_HDL *) pEventData;

    if (type == erfEventHandlerRegister) {
        erfType = (ERF_TYPE *) malloc (sizeof (ERF_TYPE));
        if (erfType == NULL) {
            /* errno set by malloc() */
            return;
        }

        erfType->func = pErfHdl->func;
        erfType->pUserData = pErfHdl->pUserData;

        semTake (erfMutex, WAIT_FOREVER);

        erfType->next = erfTable[pErfHdl->category].type[pErfHdl->type];
        erfTable[pErfHdl->category].type[pErfHdl->type] = erfType;

        semGive (erfMutex);

    } else if (type == erfEventHandlerUnregister) {

        semTake (erfMutex, WAIT_FOREVER);

        erfType = erfTable[pErfHdl->category].type[pErfHdl->type];

        prevErfType = NULL;
        while (erfType != NULL) {
            /* Check if handler and user data match. */
            if ((erfType->func == pErfHdl->func) &&
                (erfType->pUserData == pErfHdl->pUserData)) {
                /* Update pointers */
                if (prevErfType != NULL) {
                    prevErfType->next = erfType->next;
                } else {
                    erfTable[pErfHdl->category].type[pErfHdl->type] =
                        erfType->next;
                }

                free (erfType);
                semGive (erfMutex);
                return;
            }

            prevErfType = erfType;
            erfType = erfType->next;
        }

        semGive (erfMutex);

        errnoSet (S_erfLib_HANDLER_NOT_FOUND);
    }
}

