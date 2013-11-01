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

/* assert.c - Assert function */

/* Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/*
 * Manuel Novoa III       Dec 2000
 *
 * Converted to use my new (un)signed long (long) to string routines, which
 * are smaller than the previous functions and don't require static buffers.
 */

#define NO_LOGLIB
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <vmx.h>
#ifndef NO_LOGLIB
#include <os/logLib.h>
#endif

char *__ltostr(char *buf, unsigned long uval, int base, int uppercase);

void __assert(char *assertion, char *filename,
              int linenumber, char *function)
{
        char buf[12];
        char str[255];

        strcpy(str, filename);
        strcat(str, ":");
        strcat(str, __ltostr(buf + sizeof(buf) - 1, linenumber, 10, 0));
        strcat(str, function ? ": " : "");
        strcat(str, function ? function : "");
        strcat(str, function ? "() " : "");
        strcat(str, ": Assertion \"");
        strcat(str, assertion);
        strcat(str, "\" failed.\n");
#ifndef NO_LOGLIB
        logMsg(str, (ARG) 1, (ARG) 2, (ARG) 3, (ARG) 4, (ARG) 5, (ARG) 6);
#else
        fprintf(stderr, str);
#endif
        exit(1); /* abort(); */
}

