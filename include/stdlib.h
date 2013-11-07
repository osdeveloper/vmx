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

/* stdlib.h - Standard library */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/types.h>
#include <vmx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Define NULL */
#if     !defined(NULL)
#if defined __GNUG__
#define NULL (__null__)
#else
#if !defined __cplusplus && 0
#define ((void *)0)
#else
#define NULL (0)
#endif
#endif
#endif

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

#define PATH_MAX               1024

#define RAND_MAX               32767

#define min(a,b)               ((a) < (b) ? (a) : (b))
#define max(a,b)               ((a) > (b) ? (a) : (b))

/******************************************************************************
 * itoa - integer to ascii
 *
 * RETURNS: ptr to <string>
 */

char* itoa(
    int value,
    char *string,
    int radix
    );

/******************************************************************************
 * itox - Integer to hex-string
 *
 * RETURNS: Pointer to ascii string
 */

char* itox(
    int val,
    char *str
    );

/******************************************************************************
 * strtol - Convert string to long integer
 *
 * RETURNS: Long integer number
 */

long strtol(
    const char *str,
    char **endptr,
    int base
    );

/******************************************************************************
 * abs - Get absolute value of integer
 *
 * RETURNS: Absoulte value of arument
 */

int abs(
    int i
    );

/******************************************************************************
 * div - Calculate quotient and reminder
 *
 * RETURNS: Result in a div_t structure
 */

div_t div(
    int numer,
    int denom
    );

/******************************************************************************
 * div_r - Calculate quotient and reminder
 *
 * RETURNS: N/A
 */

void div_r(
    int numer,
    int denom,
    div_t *result
    );

/******************************************************************************
 * ldiv - Calculate quotient and reminder
 *
 * RETURNS: Result in a ldiv_t structure
 */

ldiv_t ldiv(
    long numer,
    long denom
    );

/******************************************************************************
 * ldiv_r - Calculate quotient and reminder
 *
 * RETURNS: N/A
 */

void ldiv_r(
    long numer,
    long denom,
    ldiv_t *result
    );

/******************************************************************************
 * srand - Set random seed for random number generator
 *
 * RETURNS: NULL
 */

void* srand(
    unsigned int seed
    );

/******************************************************************************
 * rand - Generate random number
 *
 * RETURNS: Random number
 */

int rand(
    void
    );

/******************************************************************************
 * malloc - Allocate memory from heap
 *
 * RETURNS: Pointer to allocated memory or NULL
 */

void* malloc(
    size_t size
    );

/******************************************************************************
 * realloc - Reallocated memory from heap, keep existing data
 *
 * RETURNS: Pointer to allocated memory or NULL
 */

void* realloc(
    void *ptr,
    size_t size
    );

/******************************************************************************
 * free - Free allocate memory to heap
 *
 * RETURNS: N/A
 */

void free(
    void *ptr
    );

/******************************************************************************
 * exit - Exit from task
 *
 * RETURNS: N/A
 */

void exit(
    int status
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STDLIB_H */

