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

/* latin1CharMap.c - 8-bit latin-1 character map */

unsigned char *charTable[] =
{
    /* PC */
    (unsigned char *)
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\040\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
    "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
    "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
    "\376\245\376\376\376\376\231\376\235\376\376\376\232\376\376\341"
    "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\241\213"
    "\376\244\225\242\223\376\224\366\233\227\243\226\201\376\376\230",

    /* VT100 */
    (unsigned char *)
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^ "
    "\004\261\007\007\007\007\370\361\040\007\331\277\332\300\305\007"
    "\007\304\007\007\303\264\301\302\263\007\007\007\007\007\234\000"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\040\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
    "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
    "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
    "\376\245\376\376\376\376\231\376\235\376\376\376\232\376\376\341"
    "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\241\213"
    "\376\244\225\242\223\376\224\366\233\227\243\226\201\376\376\230"
};

