/*
 * $XConsortium: courO12.bdf,v 1.11 95/01/26 18:00:44 gildea Exp $
 * 
 * +
 *  Copyright 1984-1989, 1994 Adobe Systems Incorporated.
 *  Copyright 1988, 1994 Digital Equipment Corporation.
 * 
 *  Adobe is a trademark of Adobe Systems Incorporated which may be
 *  registered in certain jurisdictions.
 *  Permission to use these trademarks is hereby granted only in
 *  association with the images described in this file.
 * 
 *  Permission to use, copy, modify, distribute and sell this software
 *  and its documentation for any purpose and without fee is hereby
 *  granted, provided that the above copyright notices appear in all
 *  copies and that both those copyright notices and this permission
 *  notice appear in supporting documentation, and that the names of
 *  Adobe Systems and Digital Equipment Corporation not be used in
 *  advertising or publicity pertaining to distribution of the software
 *  without specific, written prior permission.  Adobe Systems and
 *  Digital Equipment Corporation make no representations about the
 *  suitability of this software for any purpose.  It is provided "as
 *  is" without express or implied warranty.
 * -
 */

/* ufcni12.c - Courier_Oblique_12 font */

/*
 * This file was converted to C from BDF using bdf2bmf.py
 * Source: blenderocv.googlecode.com/svn/
 */

#include "ugl.h"
#include "driver/font/udbmffnt.h"

UGL_LOCAL const UGL_UINT8 page0Data[] = {
    /* 0x0020 (' ') */
      0,        /* page */
     32,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      1,        /* ascent */
    0x00, 

    /* 0x0021 ('!') */
      0,        /* page */
     33,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x08, 0x10, 0x40, 0x81, 0x04, 0x00, 0x10, 

    /* 0x0022 ('"') */
      0,        /* page */
     34,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      8,        /* ascent */
    0x14, 0x28, 0xa0, 

    /* 0x0023 ('#') */
      0,        /* page */
     35,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x14, 0x29, 0xf9, 0x42, 0x9f, 0x94, 0x28, 

    /* 0x0024 ('$') */
      0,        /* page */
     36,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     10,        /* ascent */
    0x08, 0x10, 0xf2, 0x22, 0x42, 0x22, 0x44, 0x70, 0x40, 0x80, 

    /* 0x0025 ('%') */
      0,        /* page */
     37,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x30, 0xa1, 0x88, 0x63, 0x09, 0x85, 0x0c, 

    /* 0x0026 ('&') */
      0,        /* page */
     38,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x18, 0x48, 0x81, 0x05, 0x51, 0x26, 0x32, 

    /* 0x0027 (''') */
      0,        /* page */
     39,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      8,        /* ascent */
    0x08, 0x10, 0x40, 

    /* 0x0028 ('(') */
      0,        /* page */
     40,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x04, 0x10, 0x40, 0x82, 0x04, 0x10, 0x20, 0x40, 0x80, 0x80, 

    /* 0x0029 (')') */
      0,        /* page */
     41,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x10, 0x10, 0x20, 0x40, 0x82, 0x04, 0x10, 0x20, 0x82, 0x00, 

    /* 0x002a ('*') */
      0,        /* page */
     42,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x08, 0x10, 0xf0, 0x82, 0x80, 

    /* 0x002b ('+') */
      0,        /* page */
     43,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x10, 0x21, 0xe1, 0x02, 0x00, 

    /* 0x002c (',') */
      0,        /* page */
     44,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      2,        /* ascent */
    0x20, 0x41, 0x00, 

    /* 0x002d ('-') */
      0,        /* page */
     45,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      4,        /* ascent */
    0x78, 

    /* 0x002e ('.') */
      0,        /* page */
     46,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      1,        /* ascent */
    0x20, 

    /* 0x002f ('/') */
      0,        /* page */
     47,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      8,        /* ascent */
    0x02, 0x08, 0x20, 0x41, 0x04, 0x08, 0x20, 0x80, 

    /* 0x0030 ('0') */
      0,        /* page */
     48,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x18, 0x48, 0x92, 0x24, 0x48, 0x92, 0x24, 0x30, 

    /* 0x0031 ('1') */
      0,        /* page */
     49,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x70, 0x40, 0x81, 0x04, 0x08, 0x10, 0xf8, 

    /* 0x0032 ('2') */
      0,        /* page */
     50,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x38, 0x88, 0x10, 0x41, 0x04, 0x10, 0x44, 0xf8, 

    /* 0x0033 ('3') */
      0,        /* page */
     51,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x38, 0x88, 0x10, 0x23, 0x81, 0x02, 0x44, 0x70, 

    /* 0x0034 ('4') */
      0,        /* page */
     52,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x04, 0x18, 0x51, 0x24, 0x91, 0x3f, 0x08, 0x38, 

    /* 0x0035 ('5') */
      0,        /* page */
     53,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x3c, 0x41, 0x03, 0xc0, 0x40, 0x81, 0x44, 0x70, 

    /* 0x0036 ('6') */
      0,        /* page */
     54,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x06, 0x30, 0x81, 0x05, 0xcc, 0x50, 0xa2, 0x38, 

    /* 0x0037 ('7') */
      0,        /* page */
     55,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x3e, 0x44, 0x10, 0x20, 0x81, 0x04, 0x08, 0x20, 

    /* 0x0038 ('8') */
      0,        /* page */
     56,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x18, 0x49, 0x12, 0x23, 0x88, 0xa1, 0x44, 0x70, 

    /* 0x0039 ('9') */
      0,        /* page */
     57,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x38, 0x89, 0x12, 0x24, 0xc6, 0x82, 0x08, 0xe0, 

    /* 0x003a (':') */
      0,        /* page */
     58,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      5,        /* ascent */
    0x10, 0x00, 0x00, 0x02, 0x00, 

    /* 0x003b (';') */
      0,        /* page */
     59,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      5,        /* ascent */
    0x10, 0x00, 0x01, 0x02, 0x08, 0x00, 

    /* 0x003c ('<') */
      0,        /* page */
     60,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      7,        /* height */
      7,        /* ascent */
    0x08, 0x20, 0x82, 0x04, 0x04, 0x04, 0x00, 

    /* 0x003d ('=') */
      0,        /* page */
     61,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      5,        /* ascent */
    0x7c, 0x03, 0xe0, 

    /* 0x003e ('>') */
      0,        /* page */
     62,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      7,        /* height */
      7,        /* ascent */
    0x20, 0x20, 0x20, 0x41, 0x04, 0x10, 0x00, 

    /* 0x003f ('?') */
      0,        /* page */
     63,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1c, 0x44, 0x08, 0x61, 0x02, 0x00, 0x10, 

    /* 0x0040 ('@') */
      0,        /* page */
     64,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x38, 0x8a, 0x75, 0x2a, 0x93, 0x30, 0x3c, 

    /* 0x0041 ('A') */
      0,        /* page */
     65,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x0042 ('B') */
      0,        /* page */
     66,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x11, 0x22, 0x3c, 0x22, 0x42, 0x44, 0xf8, 

    /* 0x0043 ('C') */
      0,        /* page */
     67,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x45, 0x02, 0x08, 0x10, 0x23, 0x38, 

    /* 0x0044 ('D') */
      0,        /* page */
     68,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x11, 0x21, 0x21, 0x22, 0x42, 0x44, 0xf8, 

    /* 0x0045 ('E') */
      0,        /* page */
     69,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3f, 0x11, 0x24, 0x38, 0x28, 0x40, 0x44, 0xfc, 

    /* 0x0046 ('F') */
      0,        /* page */
     70,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3f, 0x11, 0x24, 0x38, 0x28, 0x40, 0x40, 0xf0, 

    /* 0x0047 ('G') */
      0,        /* page */
     71,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x45, 0x02, 0x08, 0xf0, 0xa3, 0x38, 

    /* 0x0048 ('H') */
      0,        /* page */
     72,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3b, 0x88, 0x88, 0x87, 0xc2, 0x22, 0x21, 0x11, 0xdc, 

    /* 0x0049 ('I') */
      0,        /* page */
     73,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x10, 0x40, 0x81, 0x04, 0x08, 0x7c, 

    /* 0x004a ('J') */
      0,        /* page */
     74,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x08, 0x20, 0x40, 0x92, 0x24, 0x30, 

    /* 0x004b ('K') */
      0,        /* page */
     75,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3b, 0x12, 0x2c, 0x30, 0x38, 0x48, 0x44, 0xe6, 

    /* 0x004c ('L') */
      0,        /* page */
     76,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x38, 0x20, 0x81, 0x02, 0x08, 0x51, 0x7e, 

    /* 0x004d ('M') */
      0,        /* page */
     77,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x31, 0x8d, 0x8d, 0x45, 0x42, 0xa2, 0x11, 0x11, 0xdc, 

    /* 0x004e ('N') */
      0,        /* page */
     78,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x33, 0x88, 0x8c, 0x85, 0x42, 0xa2, 0x61, 0x11, 0xc8, 

    /* 0x004f ('O') */
      0,        /* page */
     79,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1c, 0x45, 0x0a, 0x18, 0x50, 0xa2, 0x38, 

    /* 0x0050 ('P') */
      0,        /* page */
     80,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x11, 0x21, 0x22, 0x3c, 0x40, 0x40, 0xf0, 

    /* 0x0051 ('Q') */
      0,        /* page */
     81,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
      8,        /* ascent */
    0x1c, 0x45, 0x0a, 0x18, 0x50, 0xa2, 0x38, 0x24, 0xb0, 

    /* 0x0052 ('R') */
      0,        /* page */
     82,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x11, 0x21, 0x3e, 0x24, 0x44, 0x42, 0xe7, 

    /* 0x0053 ('S') */
      0,        /* page */
     83,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x45, 0x01, 0xc0, 0xc0, 0xa2, 0x78, 

    /* 0x0054 ('T') */
      0,        /* page */
     84,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x7f, 0x49, 0x8a, 0x10, 0x10, 0x20, 0x20, 0xf8, 

    /* 0x0055 ('U') */
      0,        /* page */
     85,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x77, 0x22, 0x44, 0x44, 0x44, 0x88, 0x88, 0x70, 

    /* 0x0056 ('V') */
      0,        /* page */
     86,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xe7, 0x42, 0x44, 0x48, 0x28, 0x30, 0x20, 0x20, 

    /* 0x0057 ('W') */
      0,        /* page */
     87,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xf3, 0xa0, 0x92, 0x89, 0x45, 0x42, 0xa1, 0xa0, 0x90, 

    /* 0x0058 ('X') */
      0,        /* page */
     88,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3b, 0x89, 0x05, 0x01, 0x01, 0x41, 0x21, 0x09, 0xcc, 

    /* 0x0059 ('Y') */
      0,        /* page */
     89,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xe7, 0x42, 0x44, 0x28, 0x30, 0x20, 0x20, 0xf0, 

    /* 0x005a ('Z') */
      0,        /* page */
     90,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x89, 0x20, 0x82, 0x08, 0xa1, 0x7c, 

    /* 0x005b ('[') */
      0,        /* page */
     91,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x0e, 0x10, 0x20, 0x81, 0x02, 0x08, 0x10, 0x40, 0x81, 0xc0, 

    /* 0x005c ('\') */
      0,        /* page */
     92,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      8,        /* ascent */
    0x20, 0x40, 0x40, 0x81, 0x01, 0x02, 0x04, 0x04, 

    /* 0x005d (']') */
      0,        /* page */
     93,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x1c, 0x08, 0x10, 0x40, 0x82, 0x04, 0x08, 0x20, 0x43, 0x80, 

    /* 0x005e ('^') */
      0,        /* page */
     94,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      8,        /* ascent */
    0x10, 0x51, 0x20, 

    /* 0x005f ('_') */
      0,        /* page */
     95,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      0,        /* ascent */
    0x00, 0x03, 0xf8, 

    /* 0x0060 ('`') */
      0,        /* page */
     96,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      8,        /* ascent */
    0x10, 0x20, 0x20, 

    /* 0x0061 ('a') */
      0,        /* page */
     97,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x78, 0x09, 0xf4, 0x48, 0x8e, 0x80, 

    /* 0x0062 ('b') */
      0,        /* page */
     98,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x18, 0x10, 0x41, 0xe2, 0x24, 0x50, 0xa2, 0xb8, 

    /* 0x0063 ('c') */
      0,        /* page */
     99,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x38, 0x8a, 0x04, 0x08, 0x8e, 0x00, 

    /* 0x0064 ('d') */
      0,        /* page */
    100,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      8,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x03, 0x01, 0x02, 0x34, 0x4c, 0x84, 0x88, 0x88, 0x74, 

    /* 0x0065 ('e') */
      0,        /* page */
    101,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x38, 0x8b, 0xf4, 0x08, 0x8e, 0x00, 

    /* 0x0066 ('f') */
      0,        /* page */
    102,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x0e, 0x20, 0x43, 0xe2, 0x04, 0x10, 0x20, 0xf0, 

    /* 0x0067 ('g') */
      0,        /* page */
    103,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x3a, 0x8a, 0x14, 0x28, 0x8f, 0x02, 0x08, 0xe0, 

    /* 0x0068 ('h') */
      0,        /* page */
    104,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x18, 0x10, 0x40, 0xe2, 0x24, 0x50, 0xa2, 0xe6, 

    /* 0x0069 ('i') */
      0,        /* page */
    105,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x00, 0x01, 0xc1, 0x02, 0x08, 0x10, 0xf8, 

    /* 0x006a ('j') */
      0,        /* page */
    106,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      7,        /* width */
     12,        /* height */
      9,        /* ascent */
    0x02, 0x00, 0x01, 0xe0, 0x40, 0x82, 0x04, 0x08, 0x20, 0x47, 0x00, 

    /* 0x006b ('k') */
      0,        /* page */
    107,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x30, 0x20, 0x41, 0x32, 0x86, 0x14, 0x24, 0xcc, 

    /* 0x006c ('l') */
      0,        /* page */
    108,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x38, 0x10, 0x20, 0x81, 0x02, 0x08, 0x10, 0xf8, 

    /* 0x006d ('m') */
      0,        /* page */
    109,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x54, 0x54, 0xaa, 0x55, 0x5a, 0xc0, 

    /* 0x006e ('n') */
      0,        /* page */
    110,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x5c, 0x44, 0x8a, 0x14, 0x5c, 0xc0, 

    /* 0x006f ('o') */
      0,        /* page */
    111,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x38, 0x8a, 0x14, 0x28, 0x8e, 0x00, 

    /* 0x0070 ('p') */
      0,        /* page */
    112,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      8,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x2e, 0x11, 0x11, 0x21, 0x22, 0x3c, 0x20, 0x40, 0xe0, 

    /* 0x0071 ('q') */
      0,        /* page */
    113,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x3a, 0x8a, 0x14, 0x48, 0x8f, 0x02, 0x08, 0x38, 

    /* 0x0072 ('r') */
      0,        /* page */
    114,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x5c, 0x48, 0x82, 0x04, 0x1e, 0x00, 

    /* 0x0073 ('s') */
      0,        /* page */
    115,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x38, 0x88, 0xc0, 0x48, 0x8e, 0x00, 

    /* 0x0074 ('t') */
      0,        /* page */
    116,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x10, 0x21, 0xf1, 0x02, 0x08, 0x11, 0x1c, 

    /* 0x0075 ('u') */
      0,        /* page */
    117,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0xcc, 0x89, 0x24, 0x48, 0x8e, 0x80, 

    /* 0x0076 ('v') */
      0,        /* page */
    118,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0xee, 0x89, 0x22, 0x82, 0x04, 0x00, 

    /* 0x0077 ('w') */
      0,        /* page */
    119,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0xee, 0x89, 0x53, 0x45, 0x0a, 0x00, 

    /* 0x0078 ('x') */
      0,        /* page */
    120,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      8,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x77, 0x14, 0x18, 0x28, 0x44, 0xc6, 

    /* 0x0079 ('y') */
      0,        /* page */
    121,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      8,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x77, 0x22, 0x24, 0x24, 0x18, 0x10, 0x10, 0x20, 0xf0, 

    /* 0x007a ('z') */
      0,        /* page */
    122,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x7c, 0x90, 0x41, 0x04, 0x9f, 0x00, 

    /* 0x007b ('{') */
      0,        /* page */
    123,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x0c, 0x20, 0x41, 0x02, 0x18, 0x08, 0x20, 0x40, 0x80, 0xc0, 

    /* 0x007c ('|') */
      0,        /* page */
    124,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      8,        /* ascent */
    0x08, 0x10, 0x40, 0x81, 0x04, 0x08, 0x20, 0x40, 

    /* 0x007d ('}') */
      0,        /* page */
    125,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
      9,        /* ascent */
    0x30, 0x10, 0x20, 0x41, 0x01, 0x84, 0x08, 0x20, 0x43, 0x00, 

    /* 0x007e ('~') */
      0,        /* page */
    126,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      7,        /* width */
      2,        /* height */
      4,        /* ascent */
    0x65, 0x30, 

    /* 0x00a0 */
      0,        /* page */
    160,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      1,        /* ascent */
    0x00, 

    /* 0x00a1 */
      0,        /* page */
    161,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x08, 0x00, 0x00, 0x82, 0x04, 0x08, 0x20, 0x40, 

    /* 0x00a2 */
      0,        /* page */
    162,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      8,        /* ascent */
    0x10, 0x20, 0xf2, 0x28, 0x11, 0x1c, 0x20, 0x40, 

    /* 0x00a3 */
      0,        /* page */
    163,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x18, 0x48, 0x83, 0x82, 0x04, 0x11, 0x7c, 

    /* 0x00a4 */
      0,        /* page */
    164,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      7,        /* width */
      6,        /* height */
      7,        /* ascent */
    0x44, 0x71, 0x22, 0x47, 0x50, 0x00, 

    /* 0x00a5 */
      0,        /* page */
    165,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xe6, 0x88, 0xa3, 0xe1, 0x0f, 0x88, 0x38, 

    /* 0x00a6 */
      0,        /* page */
    166,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
      8,        /* ascent */
    0x08, 0x10, 0x40, 0x80, 0x00, 0x08, 0x10, 0x40, 0x80, 

    /* 0x00a7 */
      0,        /* page */
    167,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
      9,        /* ascent */
    0x1e, 0x44, 0x87, 0x88, 0x88, 0x8f, 0x04, 0x89, 0xe0, 

    /* 0x00a8 */
      0,        /* page */
    168,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      8,        /* ascent */
    0x14, 

    /* 0x00a9 */
      0,        /* page */
    169,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1c, 0x62, 0x5d, 0xa5, 0xa1, 0x9a, 0x46, 0x38, 

    /* 0x00aa */
      0,        /* page */
    170,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x1c, 0x08, 0xd0, 0x07, 0x80, 

    /* 0x00ab */
      0,        /* page */
    171,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x24, 0x92, 0x44, 0x84, 0x80, 

    /* 0x00ac */
      0,        /* page */
    172,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      5,        /* ascent */
    0x7c, 0x10, 0x20, 

    /* 0x00ad */
      0,        /* page */
    173,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      4,        /* ascent */
    0x78, 

    /* 0x00ae */
      0,        /* page */
    174,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1c, 0x62, 0x59, 0x95, 0xb9, 0xaa, 0x46, 0x38, 

    /* 0x00af */
      0,        /* page */
    175,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      8,        /* ascent */
    0x3c, 

    /* 0x00b0 */
      0,        /* page */
    176,        /* index */
      0,        /* size (MSB) */
      8,        /* size (LSB) */
      7,        /* width */
      4,        /* height */
      8,        /* ascent */
    0x30, 0x91, 0x21, 0x80, 

    /* 0x00b1 */
      0,        /* page */
    177,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x10, 0xf0, 0x80, 0x0f, 0x00, 

    /* 0x00b2 */
      0,        /* page */
    178,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x38, 0x08, 0x61, 0x07, 0x80, 

    /* 0x00b3 */
      0,        /* page */
    179,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x38, 0x08, 0xe0, 0x47, 0x00, 

    /* 0x00b4 */
      0,        /* page */
    180,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      7,        /* width */
      2,        /* height */
      9,        /* ascent */
    0x04, 0x10, 

    /* 0x00b5 */
      0,        /* page */
    181,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x66, 0x44, 0x92, 0x24, 0x4f, 0x60, 0x40, 0x80, 

    /* 0x00b6 */
      0,        /* page */
    182,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
      9,        /* ascent */
    0x1f, 0x2a, 0x54, 0x54, 0x68, 0x28, 0x28, 0x50, 0x50, 0xd8, 

    /* 0x00b7 */
      0,        /* page */
    183,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      7,        /* width */
      1,        /* height */
      4,        /* ascent */
    0x20, 

    /* 0x00b8 */
      0,        /* page */
    184,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      7,        /* width */
      3,        /* height */
      0,        /* ascent */
    0x10, 0x10, 0xc0, 

    /* 0x00b9 */
      0,        /* page */
    185,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x18, 0x10, 0x40, 0x83, 0x80, 

    /* 0x00ba */
      0,        /* page */
    186,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      8,        /* ascent */
    0x1c, 0x48, 0xe0, 0x07, 0x80, 

    /* 0x00bb */
      0,        /* page */
    187,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x48, 0x48, 0x92, 0x49, 0x00, 

    /* 0x00bc */
      0,        /* page */
    188,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
      9,        /* ascent */
    0x60, 0x21, 0x42, 0x44, 0xe8, 0x12, 0x26, 0x4a, 0x9f, 0x04, 

    /* 0x00bd */
      0,        /* page */
    189,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
      9,        /* ascent */
    0x60, 0x21, 0x42, 0x44, 0xe8, 0x1e, 0x21, 0x4e, 0x90, 0x1e, 

    /* 0x00be */
      0,        /* page */
    190,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
      9,        /* ascent */
    0x70, 0x09, 0x72, 0x14, 0xe8, 0x12, 0x26, 0x4a, 0x9f, 0x04, 

    /* 0x00bf */
      0,        /* page */
    191,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      6,        /* ascent */
    0x08, 0x00, 0x40, 0x86, 0x10, 0x22, 0x38, 

    /* 0x00c0 */
      0,        /* page */
    192,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x04, 0x02, 0x00, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c1 */
      0,        /* page */
    193,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x01, 0x02, 0x00, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c2 */
      0,        /* page */
    194,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x05, 0x00, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c3 */
      0,        /* page */
    195,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x05, 0x0a, 0x00, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c4 */
      0,        /* page */
    196,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x05, 0x00, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c5 */
      0,        /* page */
    197,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x05, 0x02, 0x1e, 0x06, 0x0a, 0x12, 0x1e, 0x22, 0x42, 0xe7, 

    /* 0x00c6 */
      0,        /* page */
    198,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x0f, 0xc2, 0x91, 0x28, 0x5c, 0x25, 0x0e, 0x04, 0x93, 0xbc, 

    /* 0x00c7 */
      0,        /* page */
    199,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
      8,        /* ascent */
    0x1e, 0x23, 0x40, 0x40, 0x80, 0x80, 0x8c, 0x70, 0x20, 0x10, 0x60, 

    /* 0x00c8 */
      0,        /* page */
    200,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x08, 0x04, 0x00, 0x3f, 0x11, 0x24, 0x38, 0x28, 0x44, 0x44, 0xfc, 

    /* 0x00c9 */
      0,        /* page */
    201,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x04, 0x00, 0x3f, 0x11, 0x24, 0x38, 0x28, 0x44, 0x44, 0xfc, 

    /* 0x00ca */
      0,        /* page */
    202,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x04, 0x0a, 0x00, 0x3f, 0x11, 0x24, 0x38, 0x28, 0x44, 0x44, 0xfc, 

    /* 0x00cb */
      0,        /* page */
    203,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x0a, 0x00, 0x3f, 0x11, 0x24, 0x38, 0x28, 0x44, 0x44, 0xfc, 

    /* 0x00cc */
      0,        /* page */
    204,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x10, 0x10, 0x01, 0xf0, 0x82, 0x04, 0x08, 0x20, 0x43, 0xe0, 

    /* 0x00cd */
      0,        /* page */
    205,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x08, 0x01, 0xf0, 0x82, 0x04, 0x08, 0x20, 0x43, 0xe0, 

    /* 0x00ce */
      0,        /* page */
    206,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x04, 0x14, 0x01, 0xf0, 0x82, 0x04, 0x08, 0x20, 0x43, 0xe0, 

    /* 0x00cf */
      0,        /* page */
    207,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x0a, 0x00, 0xf8, 0x41, 0x02, 0x04, 0x10, 0x21, 0xf0, 

    /* 0x00d0 */
      0,        /* page */
    208,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x11, 0x21, 0x79, 0x22, 0x42, 0x44, 0xf8, 

    /* 0x00d1 */
      0,        /* page */
    209,        /* index */
      0,        /* size (MSB) */
     17,        /* size (LSB) */
      9,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x05, 0x05, 0x00, 0x06, 0x71, 0x11, 0x88, 0xa8, 0x54, 0x4c, 0x22, 0x39, 0x00, 

    /* 0x00d2 */
      0,        /* page */
    210,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x10, 0x10, 0x00, 0xe2, 0x28, 0x50, 0xc2, 0x85, 0x11, 0xc0, 

    /* 0x00d3 */
      0,        /* page */
    211,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x08, 0x00, 0xe2, 0x28, 0x50, 0xc2, 0x85, 0x11, 0xc0, 

    /* 0x00d4 */
      0,        /* page */
    212,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x04, 0x14, 0x00, 0xe2, 0x28, 0x50, 0xc2, 0x85, 0x11, 0xc0, 

    /* 0x00d5 */
      0,        /* page */
    213,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      7,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x0a, 0x28, 0x00, 0xe2, 0x28, 0x50, 0xc2, 0x85, 0x11, 0xc0, 

    /* 0x00d6 */
      0,        /* page */
    214,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x0a, 0x00, 0x71, 0x14, 0x28, 0x61, 0x42, 0x88, 0xe0, 

    /* 0x00d7 */
      0,        /* page */
    215,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x44, 0x50, 0xc2, 0x88, 0x80, 

    /* 0x00d8 */
      0,        /* page */
    216,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x0e, 0x89, 0x89, 0x45, 0x25, 0x23, 0x11, 0x11, 0x70, 

    /* 0x00d9 */
      0,        /* page */
    217,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x10, 0x08, 0x00, 0x77, 0x22, 0x44, 0x44, 0x44, 0x88, 0x88, 0x70, 

    /* 0x00da */
      0,        /* page */
    218,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x02, 0x04, 0x00, 0x77, 0x22, 0x44, 0x44, 0x44, 0x88, 0x88, 0x70, 

    /* 0x00db */
      0,        /* page */
    219,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x08, 0x14, 0x00, 0x77, 0x22, 0x44, 0x44, 0x44, 0x88, 0x88, 0x70, 

    /* 0x00dc */
      0,        /* page */
    220,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      8,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x14, 0x00, 0x77, 0x22, 0x44, 0x44, 0x44, 0x88, 0x88, 0x70, 

    /* 0x00dd */
      0,        /* page */
    221,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x04, 0x08, 0x00, 0xe7, 0x42, 0x44, 0x28, 0x30, 0x20, 0x20, 0xf0, 

    /* 0x00de */
      0,        /* page */
    222,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x38, 0x10, 0x3e, 0x21, 0x22, 0x7c, 0x40, 0xe0, 

    /* 0x00df */
      0,        /* page */
    223,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x0c, 0x24, 0x49, 0x12, 0xc4, 0x50, 0xaa, 0xc8, 

    /* 0x00e0 */
      0,        /* page */
    224,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x20, 0x20, 0x03, 0xc0, 0x4f, 0xa2, 0x44, 0x74, 

    /* 0x00e1 */
      0,        /* page */
    225,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x20, 0x03, 0xc0, 0x4f, 0xa2, 0x44, 0x74, 

    /* 0x00e2 */
      0,        /* page */
    226,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x10, 0x50, 0x03, 0xc0, 0x4f, 0xa2, 0x44, 0x74, 

    /* 0x00e3 */
      0,        /* page */
    227,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x14, 0x50, 0x03, 0xc0, 0x4f, 0xa2, 0x44, 0x74, 

    /* 0x00e4 */
      0,        /* page */
    228,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x28, 0x01, 0xe0, 0x27, 0xd1, 0x22, 0x3a, 

    /* 0x00e5 */
      0,        /* page */
    229,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x10, 0x50, 0x40, 0x07, 0x80, 0x9f, 0x44, 0x88, 0xe8, 

    /* 0x00e6 */
      0,        /* page */
    230,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      8,        /* width */
      6,        /* height */
      6,        /* ascent */
    0x76, 0x09, 0x7f, 0x90, 0x92, 0x6c, 

    /* 0x00e7 */
      0,        /* page */
    231,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      6,        /* ascent */
    0x38, 0x8a, 0x04, 0x08, 0x8e, 0x08, 0x08, 0x60, 

    /* 0x00e8 */
      0,        /* page */
    232,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x20, 0x20, 0x01, 0xc4, 0x5f, 0xa0, 0x44, 0x70, 

    /* 0x00e9 */
      0,        /* page */
    233,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x20, 0x01, 0xc4, 0x5f, 0xa0, 0x44, 0x70, 

    /* 0x00ea */
      0,        /* page */
    234,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x10, 0x50, 0x01, 0xc4, 0x5f, 0xa0, 0x44, 0x70, 

    /* 0x00eb */
      0,        /* page */
    235,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x28, 0x00, 0xe2, 0x2f, 0xd0, 0x22, 0x38, 

    /* 0x00ec */
      0,        /* page */
    236,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x20, 0x20, 0x01, 0xc1, 0x02, 0x08, 0x10, 0xf8, 

    /* 0x00ed */
      0,        /* page */
    237,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x20, 0x01, 0xc1, 0x02, 0x08, 0x10, 0xf8, 

    /* 0x00ee */
      0,        /* page */
    238,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x10, 0x50, 0x01, 0xc1, 0x02, 0x08, 0x10, 0xf8, 

    /* 0x00ef */
      0,        /* page */
    239,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x28, 0x00, 0xe0, 0x81, 0x04, 0x08, 0x7c, 

    /* 0x00f0 */
      0,        /* page */
    240,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x60, 0x28, 0xe2, 0x43, 0xc8, 0xa1, 0x42, 0x88, 0xe0, 

    /* 0x00f1 */
      0,        /* page */
    241,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x0a, 0x28, 0x02, 0xe2, 0x24, 0x50, 0xa2, 0xe6, 

    /* 0x00f2 */
      0,        /* page */
    242,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x20, 0x20, 0x01, 0xc4, 0x50, 0xa1, 0x44, 0x70, 

    /* 0x00f3 */
      0,        /* page */
    243,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x20, 0x01, 0xc4, 0x50, 0xa1, 0x44, 0x70, 

    /* 0x00f4 */
      0,        /* page */
    244,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x10, 0x50, 0x01, 0xc4, 0x50, 0xa1, 0x44, 0x70, 

    /* 0x00f5 */
      0,        /* page */
    245,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x14, 0x50, 0x01, 0xc4, 0x50, 0xa1, 0x44, 0x70, 

    /* 0x00f6 */
      0,        /* page */
    246,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x28, 0x00, 0xe2, 0x28, 0x50, 0xa2, 0x38, 

    /* 0x00f7 */
      0,        /* page */
    247,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
      6,        /* ascent */
    0x10, 0x01, 0xe0, 0x02, 0x00, 

    /* 0x00f8 */
      0,        /* page */
    248,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      7,        /* ascent */
    0x04, 0x71, 0x34, 0xaa, 0x59, 0x1c, 0x40, 

    /* 0x00f9 */
      0,        /* page */
    249,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x20, 0x20, 0x06, 0x64, 0x49, 0x22, 0x44, 0x74, 

    /* 0x00fa */
      0,        /* page */
    250,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x08, 0x20, 0x06, 0x64, 0x49, 0x22, 0x44, 0x74, 

    /* 0x00fb */
      0,        /* page */
    251,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      9,        /* ascent */
    0x10, 0x50, 0x06, 0x64, 0x49, 0x22, 0x44, 0x74, 

    /* 0x00fc */
      0,        /* page */
    252,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x28, 0x03, 0x32, 0x24, 0x91, 0x22, 0x3a, 

    /* 0x00fd */
      0,        /* page */
    253,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      8,        /* width */
     12,        /* height */
      9,        /* ascent */
    0x04, 0x08, 0x00, 0x77, 0x22, 0x24, 0x24, 0x18, 0x10, 0x10, 0x20, 0xf0, 

    /* 0x00fe */
      0,        /* page */
    254,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      8,        /* width */
     12,        /* height */
      9,        /* ascent */
    0x18, 0x08, 0x08, 0x1e, 0x11, 0x11, 0x21, 0x22, 0x3c, 0x40, 0x40, 0xe0, 

    /* 0x00ff */
      0,        /* page */
    255,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      8,        /* width */
     11,        /* height */
      8,        /* ascent */
    0x14, 0x00, 0x77, 0x22, 0x24, 0x24, 0x18, 0x10, 0x10, 0x20, 0xf0, 

    /* End of page */
    0, 0, 0, 0
};

UGL_LOCAL const UGL_UINT8 * const pageArray[] = {
    page0Data,
    UGL_NULL
};

const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_12 = {
    {
        {12, 12},                               /* pixelSize */
        {UGL_FONT_BOLD_OFF, UGL_FONT_BOLD_OFF}, /* weight */
        UGL_FONT_ITALIC,                        /* italic */
        UGL_FONT_MONO_SPACED,                   /* spacing */
        UGL_FONT_ISO_8859_1,                    /* charSet */
        "Courier Oblique",                      /* faceName */
        "Courier"                               /* familyName */
    },

      1,                /* leading */
     11,                /* maxAscent */
      3,                /* maxDescent */
      7,                /* maxAdvance */
    pageArray           /* pageData */
};

