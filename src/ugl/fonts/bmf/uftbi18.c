/*
 * $XConsortium: timBI18.bdf,v 1.14 95/01/26 18:06:27 gildea Exp $
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

/* uftbi18.c - Times_Bold_Italic_18 font */

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
      4,        /* width */
      1,        /* height */
      1,        /* ascent */
    0x00, 

    /* 0x0021 ('!') */
      0,        /* page */
     33,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      7,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x0e, 0x1c, 0x30, 0xe1, 0xc3, 0x0e, 0x18, 0x20, 0x00, 0x03, 0x06, 0x00, 

    /* 0x0022 ('"') */
      0,        /* page */
     34,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      9,        /* width */
      5,        /* height */
     13,        /* ascent */
    0x33, 0x19, 0x99, 0x8c, 0xc4, 0x40, 

    /* 0x0023 ('#') */
      0,        /* page */
     35,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1b, 0x0d, 0x86, 0xcf, 0xf7, 0xf9, 0xb0, 0xd8, 0x6c, 0xff, 0x7f, 0x9b, 0x0d, 0x86, 0xc0, 

    /* 0x0024 ('$') */
      0,        /* page */
     36,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
      9,        /* width */
     16,        /* height */
     14,        /* ascent */
    0x02, 0x0f, 0x09, 0x4c, 0xa6, 0x83, 0xc0, 0xf0, 0x3c, 0x1f, 0x0b, 0x84, 0xd4, 0x6a, 0x63, 0xe1, 0x00, 0x80, 

    /* 0x0025 ('%') */
      0,        /* page */
     37,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3c, 0x30, 0xef, 0xe3, 0x91, 0x86, 0x26, 0x0c, 0xcc, 0x1b, 0x30, 0x1c, 0x6e, 0x01, 0xba, 0x02, 0xe4, 0x0d, 0x88, 0x33, 0x30, 0x66, 0xc1, 0x87, 0x00, 

    /* 0x0026 ('&') */
      0,        /* page */
     38,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0xc0, 0x0e, 0xc0, 0x1c, 0x80, 0x3b, 0x00, 0x3c, 0x00, 0xe0, 0x07, 0xcf, 0x1d, 0xcc, 0x73, 0xb0, 0xe3, 0xc1, 0xc7, 0x03, 0xdf, 0x63, 0xe7, 0x80, 

    /* 0x0027 (''') */
      0,        /* page */
     39,        /* index */
      0,        /* size (MSB) */
      8,        /* size (LSB) */
      5,        /* width */
      5,        /* height */
     13,        /* ascent */
    0x39, 0xc6, 0x22, 0x00, 

    /* 0x0028 ('(') */
      0,        /* page */
     40,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      6,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x0c, 0x63, 0x0c, 0x61, 0x8c, 0x30, 0xc3, 0x0c, 0x30, 0xc1, 0x86, 0x0c, 

    /* 0x0029 (')') */
      0,        /* page */
     41,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      7,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x30, 0x30, 0x60, 0x60, 0xc1, 0x83, 0x06, 0x0c, 0x18, 0x60, 0xc3, 0x06, 0x18, 0x60, 

    /* 0x002a ('*') */
      0,        /* page */
     42,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
     13,        /* ascent */
    0x0c, 0x03, 0x06, 0xd8, 0xfc, 0x0c, 0x0f, 0xc6, 0xd8, 0x30, 

    /* 0x002b ('+') */
      0,        /* page */
     43,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      9,        /* ascent */
    0x0c, 0x03, 0x00, 0xc1, 0xfe, 0x7f, 0x83, 0x00, 0xc0, 0x30, 

    /* 0x002c (',') */
      0,        /* page */
     44,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      5,        /* width */
      4,        /* height */
      2,        /* ascent */
    0x63, 0x09, 0x80, 

    /* 0x002d ('-') */
      0,        /* page */
     45,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
     11,        /* width */
      2,        /* height */
      6,        /* ascent */
    0x7f, 0xcf, 0xf8, 

    /* 0x002e ('.') */
      0,        /* page */
     46,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      5,        /* width */
      2,        /* height */
      2,        /* ascent */
    0x63, 0x00, 

    /* 0x002f ('/') */
      0,        /* page */
     47,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      6,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x0c, 0x31, 0x86, 0x18, 0xc3, 0x18, 0x61, 0x8c, 0x30, 0xc0, 

    /* 0x0030 ('0') */
      0,        /* page */
     48,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1e, 0x1d, 0x9c, 0x6e, 0x3e, 0x3f, 0x1f, 0x8f, 0x8e, 0xc7, 0x63, 0x33, 0x9d, 0x87, 0x80, 

    /* 0x0031 ('1') */
      0,        /* page */
     49,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x0e, 0x1f, 0x03, 0x81, 0xc1, 0xc0, 0xe0, 0x70, 0x38, 0x1c, 0x1c, 0x0e, 0x07, 0x0f, 0xe0, 

    /* 0x0032 ('2') */
      0,        /* page */
     50,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1f, 0x1f, 0xd1, 0xe0, 0x70, 0x38, 0x38, 0x18, 0x18, 0x18, 0x18, 0x5f, 0xff, 0xef, 0xe0, 

    /* 0x0033 ('3') */
      0,        /* page */
     51,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1f, 0x1b, 0xc0, 0xe0, 0x70, 0x70, 0xe0, 0x7c, 0x0f, 0x03, 0x81, 0xd8, 0xfc, 0xe7, 0xc0, 

    /* 0x0034 ('4') */
      0,        /* page */
     52,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0x03, 0x83, 0xc3, 0xe1, 0x61, 0xb1, 0x99, 0x9c, 0xff, 0x7f, 0x83, 0x03, 0x81, 0xc0, 

    /* 0x0035 ('5') */
      0,        /* page */
     53,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1f, 0x1f, 0x8f, 0x84, 0x07, 0x83, 0xf0, 0x78, 0x0e, 0x07, 0x03, 0xb3, 0x9b, 0x87, 0x80, 

    /* 0x0036 ('6') */
      0,        /* page */
     54,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0x07, 0x07, 0x07, 0x07, 0x03, 0x73, 0xfd, 0xc6, 0xe3, 0x63, 0xb1, 0xcd, 0xc3, 0xc0, 

    /* 0x0037 ('7') */
      0,        /* page */
     55,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0x9f, 0xd8, 0xe8, 0x60, 0x60, 0x60, 0x30, 0x30, 0x30, 0x18, 0x18, 0x18, 0x0c, 0x00, 

    /* 0x0038 ('8') */
      0,        /* page */
     56,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1e, 0x39, 0xdc, 0x6e, 0x33, 0xb0, 0xf0, 0xf8, 0xee, 0xc7, 0x61, 0xf0, 0xfd, 0xe3, 0xc0, 

    /* 0x0039 ('9') */
      0,        /* page */
     57,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1e, 0x1d, 0xdc, 0x6e, 0x37, 0x1b, 0x9c, 0xce, 0x3e, 0x07, 0x07, 0x07, 0x07, 0x0e, 0x00, 

    /* 0x003a (':') */
      0,        /* page */
     58,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      6,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x18, 0x60, 0x00, 0x00, 0x06, 0x18, 

    /* 0x003b (';') */
      0,        /* page */
     59,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      7,        /* width */
     10,        /* height */
      8,        /* ascent */
    0x18, 0x30, 0x00, 0x00, 0x00, 0x18, 0x30, 0x21, 0x80, 

    /* 0x003c ('<') */
      0,        /* page */
     60,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
     12,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x00, 0xe0, 0x38, 0x0e, 0x03, 0x80, 0x38, 0x00, 0xe0, 0x03, 0x80, 0x0e, 

    /* 0x003d ('=') */
      0,        /* page */
     61,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
     11,        /* width */
      5,        /* height */
      7,        /* ascent */
    0x7f, 0xcf, 0xf8, 0x00, 0x3f, 0xe7, 0xfc, 

    /* 0x003e ('>') */
      0,        /* page */
     62,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
     12,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x70, 0x01, 0xc0, 0x07, 0x00, 0x1c, 0x01, 0xc0, 0x70, 0x1c, 0x07, 0x00, 

    /* 0x003f ('?') */
      0,        /* page */
     63,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x0f, 0x0c, 0xc6, 0x60, 0x70, 0x78, 0x78, 0x30, 0x30, 0x18, 0x08, 0x00, 0x06, 0x03, 0x00, 

    /* 0x0040 ('@') */
      0,        /* page */
     64,        /* index */
      0,        /* size (MSB) */
     36,        /* size (LSB) */
     17,        /* width */
     15,        /* height */
     12,        /* ascent */
    0x01, 0xf0, 0x03, 0xce, 0x03, 0x81, 0x83, 0x80, 0x43, 0x8e, 0xb1, 0x8d, 0xc9, 0xcc, 0xe4, 0xc4, 0x62, 0x66, 0x33, 0x33, 0x39, 0x1d, 0xfd, 0x86, 0x73, 0x83, 0x80, 0x00, 0xf0, 0xc0, 0x1f, 0x80, 

    /* 0x0041 ('A') */
      0,        /* page */
     65,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x00, 0xc0, 0x07, 0x00, 0x1c, 0x00, 0xf8, 0x02, 0xe0, 0x19, 0x80, 0xc7, 0x03, 0x1c, 0x1f, 0xf0, 0x61, 0xc3, 0x03, 0x9c, 0x0e, 0xf8, 0x7c, 

    /* 0x0042 ('B') */
      0,        /* page */
     66,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0xe0, 0xe7, 0x87, 0x1c, 0x38, 0xe1, 0x86, 0x1f, 0xe0, 0xe3, 0x87, 0x0e, 0x30, 0x73, 0x83, 0x9c, 0x3c, 0xe3, 0xcf, 0xf8, 0x00, 

    /* 0x0043 ('C') */
      0,        /* page */
     67,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0xe8, 0x79, 0xc7, 0x06, 0x70, 0x33, 0x80, 0x18, 0x01, 0xc0, 0x0e, 0x00, 0x70, 0x03, 0xc0, 0x0e, 0x0c, 0x7d, 0xc0, 0xf8, 0x00, 

    /* 0x0044 ('D') */
      0,        /* page */
     68,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0x80, 0x77, 0x81, 0xc7, 0x07, 0x1c, 0x38, 0x38, 0xe0, 0xe3, 0x83, 0x8e, 0x0e, 0x70, 0x71, 0xc3, 0xc7, 0x0e, 0x1c, 0xf0, 0xff, 0x00, 

    /* 0x0045 ('E') */
      0,        /* page */
     69,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     12,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0xf1, 0xc7, 0x1c, 0x21, 0xc2, 0x38, 0x83, 0x98, 0x3f, 0x83, 0x98, 0x31, 0x07, 0x02, 0x70, 0x67, 0x0e, 0xff, 0xc0, 

    /* 0x0046 ('F') */
      0,        /* page */
     70,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     12,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0xf1, 0xc7, 0x1c, 0x21, 0xc2, 0x18, 0x83, 0x98, 0x3f, 0x83, 0x98, 0x31, 0x07, 0x00, 0x70, 0x07, 0x00, 0xf8, 0x00, 

    /* 0x0047 ('G') */
      0,        /* page */
     71,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0xe8, 0x3c, 0xe1, 0xc1, 0x8e, 0x06, 0x38, 0x00, 0xc0, 0x07, 0x00, 0x1c, 0x3e, 0x70, 0x71, 0xe1, 0xc3, 0x86, 0x0f, 0xb8, 0x0f, 0xe0, 

    /* 0x0048 ('H') */
      0,        /* page */
     72,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x3e, 0x38, 0x38, 0x70, 0x70, 0xe0, 0xe3, 0x83, 0x87, 0x07, 0x0f, 0xfe, 0x1c, 0x1c, 0x38, 0x38, 0xe0, 0xe1, 0xc1, 0xc3, 0x83, 0x8f, 0x8f, 0x80, 

    /* 0x0049 ('I') */
      0,        /* page */
     73,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      7,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x38, 0x70, 0xe3, 0x87, 0x0e, 0x1c, 0x70, 0xe1, 0xc3, 0x8f, 0x80, 

    /* 0x004a ('J') */
      0,        /* page */
     74,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     10,        /* width */
     14,        /* height */
     13,        /* ascent */
    0x07, 0xc0, 0xe0, 0x38, 0x0c, 0x07, 0x01, 0xc0, 0x70, 0x18, 0x0e, 0x03, 0x80, 0xe3, 0xb0, 0xfc, 0x1c, 0x00, 

    /* 0x004b ('K') */
      0,        /* page */
     75,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x7c, 0x70, 0xc1, 0xc6, 0x07, 0x30, 0x39, 0x80, 0xfc, 0x03, 0xf0, 0x0e, 0xe0, 0x3b, 0x81, 0xc7, 0x07, 0x1c, 0x1c, 0x38, 0xf9, 0xf0, 

    /* 0x004c ('L') */
      0,        /* page */
     76,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x03, 0x80, 0x70, 0x0e, 0x03, 0x80, 0x70, 0x0e, 0x01, 0xc0, 0x30, 0x0e, 0x05, 0xc1, 0xb8, 0x7f, 0xfc, 

    /* 0x004d ('M') */
      0,        /* page */
     77,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     18,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x07, 0xc7, 0x81, 0xe1, 0xe0, 0xf0, 0x58, 0x4c, 0x37, 0x36, 0x0d, 0xc9, 0x82, 0x76, 0x60, 0x9d, 0x30, 0x63, 0xcc, 0x18, 0xe3, 0x06, 0x38, 0xc1, 0x8c, 0x70, 0xf2, 0x3e, 0x00, 

    /* 0x004e ('N') */
      0,        /* page */
     78,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     16,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3c, 0x1f, 0x1c, 0x0e, 0x1e, 0x0c, 0x1e, 0x0c, 0x1b, 0x18, 0x33, 0x98, 0x33, 0x98, 0x31, 0xd8, 0x31, 0xd0, 0x60, 0xf0, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x60, 

    /* 0x004f ('O') */
      0,        /* page */
     79,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0xe0, 0x3d, 0xc1, 0xc3, 0x8e, 0x0e, 0x38, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x0e, 0x70, 0x71, 0xc1, 0xc7, 0x0e, 0x0e, 0xf0, 0x1f, 0x00, 

    /* 0x0050 ('P') */
      0,        /* page */
     80,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     12,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0xe1, 0xcf, 0x1c, 0x71, 0xc7, 0x38, 0x73, 0x8e, 0x3f, 0xc3, 0x80, 0x30, 0x07, 0x00, 0x70, 0x07, 0x00, 0xf8, 0x00, 

    /* 0x0051 ('Q') */
      0,        /* page */
     81,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x03, 0xe0, 0x3d, 0xc1, 0xc3, 0x8e, 0x0e, 0x38, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x0e, 0x70, 0x71, 0xc1, 0xc7, 0x0e, 0x0e, 0xf0, 0x1f, 0x00, 0x30, 0x43, 0xfe, 0x1b, 0xf0, 

    /* 0x0052 ('R') */
      0,        /* page */
     82,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     12,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0xe1, 0xcf, 0x1c, 0x71, 0x87, 0x38, 0x73, 0x8e, 0x3f, 0xc3, 0xb8, 0x39, 0xc7, 0x1c, 0x70, 0xe7, 0x0e, 0xf8, 0xf0, 

    /* 0x0053 ('S') */
      0,        /* page */
     83,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x0f, 0x43, 0xb8, 0xe3, 0x1c, 0x23, 0xc0, 0x3c, 0x03, 0xc0, 0x7c, 0x07, 0x98, 0x73, 0x0e, 0x73, 0x8f, 0xe0, 

    /* 0x0054 ('T') */
      0,        /* page */
     84,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0xff, 0xf9, 0xce, 0x38, 0x87, 0x01, 0xc0, 0x38, 0x07, 0x00, 0xe0, 0x18, 0x07, 0x00, 0xe0, 0x1c, 0x07, 0xc0, 

    /* 0x0055 ('U') */
      0,        /* page */
     85,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x7e, 0x7c, 0xe0, 0xe3, 0x83, 0x1e, 0x0c, 0x70, 0x61, 0xc1, 0x87, 0x06, 0x3c, 0x18, 0xe0, 0xc3, 0x83, 0x0e, 0x0c, 0x3c, 0x60, 0x7f, 0x00, 

    /* 0x0056 ('V') */
      0,        /* page */
     86,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x7c, 0xf9, 0xc3, 0x8e, 0x18, 0x71, 0x83, 0x8c, 0x1c, 0xc0, 0xec, 0x07, 0x60, 0x3e, 0x01, 0xe0, 0x0f, 0x00, 0x70, 0x03, 0x00, 0x00, 

    /* 0x0057 ('W') */
      0,        /* page */
     87,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     17,        /* width */
     13,        /* height */
     13,        /* ascent */
    0xf3, 0xe7, 0xb8, 0xe1, 0x9c, 0x71, 0x8e, 0x38, 0x87, 0x1c, 0xc3, 0x9e, 0x41, 0xcf, 0x60, 0xeb, 0xa0, 0x3c, 0xf0, 0x1c, 0x70, 0x0e, 0x38, 0x06, 0x18, 0x03, 0x0c, 0x00, 

    /* 0x0058 ('X') */
      0,        /* page */
     88,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0x3e, 0x1c, 0x30, 0x38, 0xc0, 0x3b, 0x00, 0x7c, 0x00, 0x70, 0x00, 0xe0, 0x03, 0xe0, 0x0d, 0xc0, 0x33, 0x80, 0xc3, 0x83, 0x07, 0x0f, 0x1f, 0x80, 

    /* 0x0059 ('Y') */
      0,        /* page */
     89,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     12,        /* width */
     13,        /* height */
     13,        /* ascent */
    0xf8, 0xf7, 0x06, 0x70, 0xc3, 0x98, 0x3b, 0x01, 0xe0, 0x1c, 0x01, 0xc0, 0x1c, 0x03, 0x80, 0x38, 0x03, 0x80, 0xfe, 0x00, 

    /* 0x005a ('Z') */
      0,        /* page */
     90,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x1f, 0xf9, 0xc7, 0x88, 0x38, 0x03, 0x80, 0x38, 0x03, 0x80, 0x3c, 0x01, 0xc0, 0x1c, 0x01, 0xc1, 0x1c, 0x19, 0xe1, 0xcf, 0xfc, 0x00, 

    /* 0x005b ('[') */
      0,        /* page */
     91,        /* index */
      0,        /* size (MSB) */
     20,        /* size (LSB) */
      8,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x0f, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x30, 0x30, 0x30, 0x30, 0x60, 0x60, 0x60, 0x60, 0xc0, 0xf0, 

    /* 0x005c ('\') */
      0,        /* page */
     92,        /* index */
      0,        /* size (MSB) */
     17,        /* size (LSB) */
      8,        /* width */
     13,        /* height */
     13,        /* ascent */
    0xc0, 0xc0, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 

    /* 0x005d (']') */
      0,        /* page */
     93,        /* index */
      0,        /* size (MSB) */
     20,        /* size (LSB) */
      8,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x0f, 0x03, 0x06, 0x06, 0x06, 0x06, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x30, 0xf0, 

    /* 0x005e ('^') */
      0,        /* page */
     94,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
     11,        /* width */
      8,        /* height */
     13,        /* ascent */
    0x06, 0x01, 0xc0, 0x38, 0x0d, 0x83, 0x30, 0x66, 0x18, 0x63, 0x0c, 

    /* 0x005f ('_') */
      0,        /* page */
     95,        /* index */
      0,        /* size (MSB) */
      8,        /* size (LSB) */
      9,        /* width */
      3,        /* height */
      0,        /* ascent */
    0x00, 0x00, 0x3f, 0xe0, 

    /* 0x0060 ('`') */
      0,        /* page */
     96,        /* index */
      0,        /* size (MSB) */
      8,        /* size (LSB) */
      5,        /* width */
      5,        /* height */
     13,        /* ascent */
    0x11, 0x18, 0xe7, 0x00, 

    /* 0x0061 ('a') */
      0,        /* page */
     97,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x9e, 0xe7, 0x3b, 0x8c, 0xe7, 0x39, 0xce, 0xf4, 0xee, 

    /* 0x0062 ('b') */
      0,        /* page */
     98,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x78, 0x1c, 0x0e, 0x06, 0x03, 0x03, 0xb9, 0xee, 0xe7, 0x73, 0xf1, 0xf9, 0xdc, 0xc7, 0x80, 

    /* 0x0063 ('c') */
      0,        /* page */
     99,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1f, 0x73, 0x63, 0xe0, 0xe0, 0xe0, 0xf6, 0x3c, 

    /* 0x0064 ('d') */
      0,        /* page */
    100,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
     10,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0xc0, 0x70, 0x1c, 0x06, 0x03, 0x86, 0xe7, 0x79, 0x8c, 0xe3, 0x38, 0xce, 0x73, 0xbd, 0x7b, 0x80, 

    /* 0x0065 ('e') */
      0,        /* page */
    101,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1e, 0x3b, 0x99, 0xdd, 0xcf, 0x87, 0x1b, 0xd8, 0xf8, 

    /* 0x0066 ('f') */
      0,        /* page */
    102,        /* index */
      0,        /* size (MSB) */
     28,        /* size (LSB) */
     11,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x03, 0xc0, 0xec, 0x19, 0x87, 0x00, 0xe0, 0x7f, 0x03, 0x80, 0x70, 0x0c, 0x03, 0x80, 0x70, 0x0e, 0x01, 0xc0, 0x30, 0x36, 0x06, 0xc0, 0x70, 0x00, 

    /* 0x0067 ('g') */
      0,        /* page */
    103,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x1f, 0x4e, 0xe7, 0x39, 0x8e, 0x77, 0x0f, 0x87, 0x00, 0xf8, 0xef, 0xb0, 0xec, 0x39, 0xfc, 

    /* 0x0068 ('h') */
      0,        /* page */
    104,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3c, 0x03, 0x80, 0x70, 0x0c, 0x03, 0x80, 0x77, 0x0f, 0xf1, 0xce, 0x31, 0xce, 0x31, 0xce, 0x39, 0xd7, 0x1c, 

    /* 0x0069 ('i') */
      0,        /* page */
    105,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      5,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x39, 0xc0, 0x00, 0x79, 0xce, 0x63, 0x39, 0xd7, 0x00, 

    /* 0x006a ('j') */
      0,        /* page */
    106,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
      8,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x07, 0x07, 0x00, 0x00, 0x00, 0x1e, 0x0e, 0x0e, 0x0c, 0x0c, 0x1c, 0x1c, 0x18, 0x18, 0xd8, 0xf0, 0x60, 

    /* 0x006b ('k') */
      0,        /* page */
    107,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
     10,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3c, 0x07, 0x01, 0xc0, 0x60, 0x38, 0x0e, 0xf3, 0x38, 0xd8, 0x3c, 0x1d, 0x07, 0x63, 0x9e, 0xe7, 0x00, 

    /* 0x006c ('l') */
      0,        /* page */
    108,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      5,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x79, 0xce, 0x67, 0x39, 0xce, 0x67, 0x39, 0xd7, 0x00, 

    /* 0x006d ('m') */
      0,        /* page */
    109,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     14,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xf7, 0x71, 0xee, 0xe7, 0x33, 0x9c, 0xce, 0x73, 0x33, 0x99, 0xce, 0x67, 0x79, 0x9e, 

    /* 0x006e ('n') */
      0,        /* page */
    110,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xf7, 0x1e, 0xe7, 0x39, 0xce, 0x63, 0x39, 0xce, 0x77, 0x8e, 

    /* 0x006f ('o') */
      0,        /* page */
    111,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1f, 0x1c, 0xe6, 0x3b, 0x8e, 0xe3, 0xb8, 0xce, 0x71, 0xf0, 

    /* 0x0070 ('p') */
      0,        /* page */
    112,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
     11,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x3b, 0xc3, 0xdc, 0x73, 0x8e, 0x73, 0x8e, 0x73, 0x8e, 0x61, 0xb8, 0x30, 0x0e, 0x01, 0xc0, 0x7c, 0x00, 

    /* 0x0071 ('q') */
      0,        /* page */
    113,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x1d, 0xbb, 0xd8, 0xfc, 0x6e, 0x77, 0x3b, 0xfc, 0x6c, 0x06, 0x07, 0x03, 0x83, 0xe0, 

    /* 0x0072 ('r') */
      0,        /* page */
    114,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xee, 0xfd, 0xc3, 0x86, 0x1c, 0x38, 0x70, 

    /* 0x0073 ('s') */
      0,        /* page */
    115,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x3e, 0x77, 0x72, 0x38, 0x1c, 0x4e, 0xee, 0x7c, 

    /* 0x0074 ('t') */
      0,        /* page */
    116,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      6,        /* width */
     10,        /* height */
     10,        /* ascent */
    0x31, 0xcf, 0xdc, 0x71, 0x8e, 0x38, 0xe9, 0xc0, 

    /* 0x0075 ('u') */
      0,        /* page */
    117,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xf3, 0x9c, 0xe7, 0x31, 0x9c, 0xe7, 0x3b, 0xcf, 0xb5, 0xce, 

    /* 0x0076 ('v') */
      0,        /* page */
    118,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      8,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xf7, 0x73, 0x73, 0x72, 0x36, 0x3c, 0x38, 0x30, 

    /* 0x0077 ('w') */
      0,        /* page */
    119,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
     12,        /* width */
      8,        /* height */
      8,        /* ascent */
    0xe2, 0x77, 0x33, 0x73, 0x37, 0x7a, 0x37, 0xe3, 0xdc, 0x39, 0x81, 0x08, 

    /* 0x0078 ('x') */
      0,        /* page */
    120,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      9,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x7b, 0x9d, 0x87, 0x83, 0x83, 0xc3, 0x73, 0x3b, 0xce, 

    /* 0x0079 ('y') */
      0,        /* page */
    121,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x7b, 0x9c, 0xce, 0x67, 0x21, 0xb0, 0xf0, 0x78, 0x18, 0x0c, 0x04, 0x34, 0x1c, 0x00, 

    /* 0x007a ('z') */
      0,        /* page */
    122,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      7,        /* width */
      9,        /* height */
      8,        /* ascent */
    0x3e, 0xfd, 0x30, 0xc3, 0x0c, 0x39, 0xfb, 0x1c, 

    /* 0x007b ('{') */
      0,        /* page */
    123,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      7,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x0e, 0x30, 0xc1, 0x83, 0x06, 0x18, 0x60, 0x60, 0xc1, 0x86, 0x0c, 0x18, 0x30, 0x38, 

    /* 0x007c ('|') */
      0,        /* page */
    124,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      6,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 

    /* 0x007d ('}') */
      0,        /* page */
    125,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      7,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x38, 0x18, 0x30, 0x60, 0xc3, 0x06, 0x0c, 0x0c, 0x30, 0xc1, 0x83, 0x06, 0x18, 0x60, 

    /* 0x007e ('~') */
      0,        /* page */
    126,        /* index */
      0,        /* size (MSB) */
      8,        /* size (LSB) */
     10,        /* width */
      3,        /* height */
      7,        /* ascent */
    0x38, 0x9f, 0xe4, 0x70, 

    /* 0x00a0 */
      0,        /* page */
    160,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      4,        /* width */
      1,        /* height */
      1,        /* ascent */
    0x00, 

    /* 0x00a1 */
      0,        /* page */
    161,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      6,        /* width */
     13,        /* height */
      9,        /* ascent */
    0x0c, 0x30, 0x00, 0x08, 0x63, 0x8c, 0x71, 0xc6, 0x38, 0xe0, 

    /* 0x00a2 */
      0,        /* page */
    162,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
     10,        /* ascent */
    0x01, 0x01, 0x07, 0xce, 0xe6, 0x47, 0x43, 0xa1, 0xe0, 0xf6, 0x3e, 0x08, 0x08, 0x00, 

    /* 0x00a3 */
      0,        /* page */
    163,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
     10,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x03, 0x81, 0xb0, 0xcc, 0x30, 0x1c, 0x06, 0x07, 0xf0, 0x60, 0x18, 0x0e, 0x07, 0xc7, 0x7e, 0xe7, 0x00, 

    /* 0x00a4 */
      0,        /* page */
    164,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
      9,        /* width */
      9,        /* height */
     11,        /* ascent */
    0x9c, 0xff, 0xd8, 0xd8, 0x3c, 0x1e, 0x0d, 0x8d, 0xff, 0x9c, 0x80, 

    /* 0x00a5 */
      0,        /* page */
    165,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0xf9, 0xee, 0x19, 0xc6, 0x1c, 0x83, 0xb0, 0x3c, 0x1f, 0xc0, 0xe0, 0x7f, 0x03, 0x00, 0xe0, 0x1c, 0x07, 0xe0, 

    /* 0x00a6 */
      0,        /* page */
    166,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
      6,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x30, 0xc3, 0x0c, 0x30, 0x00, 0x00, 0x30, 0xc3, 0x0c, 0x30, 

    /* 0x00a7 */
      0,        /* page */
    167,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
      9,        /* width */
     16,        /* height */
     13,        /* ascent */
    0x0f, 0x0d, 0xc4, 0xc3, 0x01, 0xc1, 0x71, 0x9c, 0xc6, 0x63, 0x39, 0x8e, 0x83, 0x80, 0xc6, 0x23, 0xb0, 0xf0, 

    /* 0x00a8 */
      0,        /* page */
    168,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      7,        /* width */
      2,        /* height */
     11,        /* ascent */
    0x66, 0xcc, 

    /* 0x00a9 */
      0,        /* page */
    169,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x07, 0xc0, 0x38, 0xe0, 0xc0, 0x61, 0x3f, 0x46, 0xe6, 0xc9, 0x84, 0x93, 0x01, 0x26, 0x02, 0x6e, 0x6c, 0x4f, 0x90, 0xc0, 0x60, 0xe3, 0x80, 0x7c, 0x00, 

    /* 0x00aa */
      0,        /* page */
    170,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      7,        /* height */
     12,        /* ascent */
    0x1e, 0x6d, 0xb3, 0x77, 0x40, 0x3e, 0x00, 

    /* 0x00ab */
      0,        /* page */
    171,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x08, 0x86, 0x63, 0x31, 0x98, 0xcc, 0x19, 0x83, 0x30, 0x44, 

    /* 0x00ac */
      0,        /* page */
    172,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
     11,        /* width */
      6,        /* height */
      7,        /* ascent */
    0x7f, 0xcf, 0xf8, 0x03, 0x00, 0x60, 0x0c, 0x01, 0x80, 

    /* 0x00ad */
      0,        /* page */
    173,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      6,        /* width */
      2,        /* height */
      5,        /* ascent */
    0xfb, 0xe0, 

    /* 0x00ae */
      0,        /* page */
    174,        /* index */
      0,        /* size (MSB) */
     29,        /* size (LSB) */
     15,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x07, 0xc0, 0x38, 0xe0, 0xc0, 0x61, 0x7e, 0x46, 0x66, 0xc8, 0xcc, 0x91, 0xf1, 0x23, 0x62, 0x66, 0x6c, 0x5e, 0xf0, 0xc0, 0x60, 0xe3, 0x80, 0x7c, 0x00, 

    /* 0x00af */
      0,        /* page */
    175,        /* index */
      0,        /* size (MSB) */
      5,        /* size (LSB) */
      6,        /* width */
      1,        /* height */
     12,        /* ascent */
    0x7c, 

    /* 0x00b0 */
      0,        /* page */
    176,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      7,        /* width */
      5,        /* height */
     13,        /* ascent */
    0x3c, 0xcd, 0x9b, 0x33, 0xc0, 

    /* 0x00b1 */
      0,        /* page */
    177,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x0c, 0x03, 0x00, 0xc1, 0xfe, 0x7f, 0x83, 0x00, 0xc0, 0x30, 0x00, 0x1f, 0xe7, 0xf8, 

    /* 0x00b2 */
      0,        /* page */
    178,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      6,        /* width */
      8,        /* height */
     13,        /* ascent */
    0x39, 0x30, 0xc6, 0x10, 0x86, 0x7e, 

    /* 0x00b3 */
      0,        /* page */
    179,        /* index */
      0,        /* size (MSB) */
     10,        /* size (LSB) */
      6,        /* width */
      8,        /* height */
     13,        /* ascent */
    0x39, 0x31, 0x8c, 0x18, 0x6b, 0x38, 

    /* 0x00b4 */
      0,        /* page */
    180,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      6,        /* width */
      3,        /* height */
     13,        /* ascent */
    0x1c, 0xe6, 0x00, 

    /* 0x00b5 */
      0,        /* page */
    181,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     12,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x3c, 0xe1, 0xce, 0x1c, 0xc1, 0x9c, 0x39, 0xc3, 0xbc, 0x3e, 0xd7, 0xce, 0x60, 0x04, 0x00, 0xe0, 0x06, 0x00, 

    /* 0x00b6 */
      0,        /* page */
    182,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     10,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x07, 0xc7, 0xa3, 0xe9, 0xf4, 0x7d, 0x1f, 0x47, 0xd0, 0xe8, 0x0a, 0x02, 0x81, 0x40, 0x50, 0x14, 0x05, 0x02, 0x80, 0xa0, 0x28, 0x00, 

    /* 0x00b7 */
      0,        /* page */
    183,        /* index */
      0,        /* size (MSB) */
      6,        /* size (LSB) */
      5,        /* width */
      2,        /* height */
      6,        /* ascent */
    0x73, 0x80, 

    /* 0x00b8 */
      0,        /* page */
    184,        /* index */
      0,        /* size (MSB) */
      7,        /* size (LSB) */
      6,        /* width */
      4,        /* height */
      0,        /* ascent */
    0x30, 0xe9, 0x9c, 

    /* 0x00b9 */
      0,        /* page */
    185,        /* index */
      0,        /* size (MSB) */
      9,        /* size (LSB) */
      5,        /* width */
      8,        /* height */
     13,        /* ascent */
    0x13, 0x8c, 0x66, 0x31, 0x9e, 

    /* 0x00ba */
      0,        /* page */
    186,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      7,        /* width */
      7,        /* height */
     12,        /* ascent */
    0x1c, 0x6d, 0x9b, 0x63, 0x80, 0x3e, 0x00, 

    /* 0x00bb */
      0,        /* page */
    187,        /* index */
      0,        /* size (MSB) */
     15,        /* size (LSB) */
     11,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x22, 0x06, 0x60, 0x66, 0x06, 0x61, 0x98, 0x66, 0x19, 0x82, 0x20, 

    /* 0x00bc */
      0,        /* page */
    188,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x10, 0x63, 0x86, 0x0c, 0x30, 0x63, 0x06, 0x18, 0x31, 0x99, 0x89, 0xde, 0xdc, 0x0c, 0xa0, 0x6b, 0x06, 0xfc, 0x31, 0x83, 0x0c, 0x00, 

    /* 0x00bd */
      0,        /* page */
    189,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x10, 0x63, 0x86, 0x0c, 0x30, 0x63, 0x06, 0x18, 0x31, 0xb9, 0x8a, 0x7e, 0xc3, 0x0c, 0x30, 0x61, 0x06, 0x10, 0x31, 0x93, 0x1f, 0x00, 

    /* 0x00be */
      0,        /* page */
    190,        /* index */
      0,        /* size (MSB) */
     26,        /* size (LSB) */
     13,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x38, 0x62, 0x66, 0x06, 0x30, 0x63, 0x01, 0x98, 0x0d, 0x9a, 0xc9, 0xdc, 0xdc, 0x0c, 0xa0, 0x6b, 0x06, 0xfc, 0x31, 0x83, 0x0c, 0x00, 

    /* 0x00bf */
      0,        /* page */
    191,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      9,        /* width */
     13,        /* height */
      9,        /* ascent */
    0x06, 0x03, 0x00, 0x00, 0x80, 0xc0, 0x60, 0x60, 0xf0, 0xf0, 0x70, 0x33, 0x19, 0x87, 0x80, 

    /* 0x00c0 */
      0,        /* page */
    192,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x07, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x1c, 0x00, 0xf8, 0x02, 0xe0, 0x19, 0x80, 0xc7, 0x03, 0x1c, 0x1f, 0xf0, 0x61, 0xc3, 0x03, 0x9c, 0x0e, 0xf8, 0x7c, 

    /* 0x00c1 */
      0,        /* page */
    193,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0x38, 0x01, 0xc0, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x1c, 0x00, 0xf8, 0x02, 0xe0, 0x19, 0x80, 0xc7, 0x03, 0x1c, 0x1f, 0xf0, 0x61, 0xc3, 0x03, 0x9c, 0x0e, 0xf8, 0x7c, 

    /* 0x00c2 */
      0,        /* page */
    194,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0xc0, 0x07, 0x80, 0x21, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x1c, 0x00, 0xf8, 0x02, 0xe0, 0x19, 0x80, 0xc7, 0x03, 0x1c, 0x1f, 0xf0, 0x61, 0xc3, 0x03, 0x9c, 0x0e, 0xf8, 0x7c, 

    /* 0x00c3 */
      0,        /* page */
    195,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x01, 0xd0, 0x0b, 0x80, 0x00, 0x00, 0x30, 0x01, 0xc0, 0x07, 0x00, 0x3e, 0x00, 0xb8, 0x06, 0x60, 0x31, 0xc0, 0xc7, 0x07, 0xfc, 0x18, 0x70, 0xc0, 0xe7, 0x03, 0xbe, 0x1f, 

    /* 0x00c4 */
      0,        /* page */
    196,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x03, 0x30, 0x0c, 0xc0, 0x00, 0x00, 0x30, 0x01, 0xc0, 0x07, 0x00, 0x3e, 0x00, 0xb8, 0x06, 0x60, 0x31, 0xc0, 0xc7, 0x07, 0xfc, 0x18, 0x70, 0xc0, 0xe7, 0x03, 0xbe, 0x1f, 

    /* 0x00c5 */
      0,        /* page */
    197,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0xc0, 0x04, 0x80, 0x12, 0x00, 0x30, 0x00, 0xc0, 0x07, 0x00, 0x1c, 0x00, 0xf8, 0x02, 0xe0, 0x19, 0x80, 0xc7, 0x03, 0x1c, 0x1f, 0xf0, 0x61, 0xc3, 0x03, 0x9c, 0x0e, 0xf8, 0x7c, 

    /* 0x00c6 */
      0,        /* page */
    198,        /* index */
      0,        /* size (MSB) */
     35,        /* size (LSB) */
     19,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x01, 0xff, 0xe0, 0x1f, 0x1c, 0x06, 0xe1, 0x01, 0x9c, 0x20, 0x33, 0x10, 0x0c, 0xe6, 0x03, 0x1f, 0xc0, 0x63, 0x98, 0x1f, 0xe2, 0x03, 0x1c, 0x08, 0xc3, 0x83, 0x38, 0x70, 0xef, 0x9f, 0xf8, 

    /* 0x00c7 */
      0,        /* page */
    199,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     13,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x03, 0xe8, 0x79, 0xc7, 0x06, 0x70, 0x33, 0x80, 0x18, 0x01, 0xc0, 0x0e, 0x00, 0x70, 0x03, 0xc0, 0x0e, 0x0c, 0x7d, 0xc0, 0xf8, 0x01, 0x80, 0x0e, 0x01, 0x30, 0x07, 0x00, 

    /* 0x00c8 */
      0,        /* page */
    200,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     12,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x1c, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x3f, 0xf1, 0xc7, 0x1c, 0x21, 0xc2, 0x38, 0x83, 0x98, 0x3f, 0x83, 0x98, 0x31, 0x07, 0x02, 0x70, 0x67, 0x0e, 0xff, 0xc0, 

    /* 0x00c9 */
      0,        /* page */
    201,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     12,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0xe0, 0x1c, 0x02, 0x00, 0x00, 0x3f, 0xf1, 0xc7, 0x1c, 0x21, 0xc2, 0x38, 0x83, 0x98, 0x3f, 0x83, 0x98, 0x31, 0x07, 0x02, 0x70, 0x67, 0x0e, 0xff, 0xc0, 

    /* 0x00ca */
      0,        /* page */
    202,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     12,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x01, 0x80, 0x3c, 0x04, 0x20, 0x00, 0x3f, 0xf1, 0xc7, 0x1c, 0x21, 0xc2, 0x38, 0x83, 0x98, 0x3f, 0x83, 0x98, 0x31, 0x07, 0x02, 0x70, 0x67, 0x0e, 0xff, 0xc0, 

    /* 0x00cb */
      0,        /* page */
    203,        /* index */
      0,        /* size (MSB) */
     28,        /* size (LSB) */
     12,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x06, 0x60, 0x66, 0x00, 0x03, 0xff, 0x1c, 0x71, 0xc2, 0x1c, 0x23, 0x88, 0x39, 0x83, 0xf8, 0x39, 0x83, 0x10, 0x70, 0x27, 0x06, 0x70, 0xef, 0xfc, 

    /* 0x00cc */
      0,        /* page */
    204,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
      7,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x70, 0x70, 0x10, 0x03, 0xe3, 0x87, 0x0e, 0x38, 0x70, 0xe1, 0xc7, 0x0e, 0x1c, 0x38, 0xf8, 

    /* 0x00cd */
      0,        /* page */
    205,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
      8,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x07, 0x0e, 0x10, 0x00, 0x3e, 0x1c, 0x1c, 0x1c, 0x38, 0x38, 0x38, 0x38, 0x70, 0x70, 0x70, 0x70, 0xf8, 

    /* 0x00ce */
      0,        /* page */
    206,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
      8,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x0c, 0x1e, 0x21, 0x00, 0x3e, 0x1c, 0x1c, 0x1c, 0x38, 0x38, 0x38, 0x38, 0x70, 0x70, 0x70, 0x70, 0xf8, 

    /* 0x00cf */
      0,        /* page */
    207,        /* index */
      0,        /* size (MSB) */
     20,        /* size (LSB) */
      8,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x33, 0x33, 0x00, 0x3e, 0x1c, 0x1c, 0x1c, 0x38, 0x38, 0x38, 0x38, 0x70, 0x70, 0x70, 0x70, 0xf8, 

    /* 0x00d0 */
      0,        /* page */
    208,        /* index */
      0,        /* size (MSB) */
     27,        /* size (LSB) */
     14,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3f, 0x80, 0x77, 0x81, 0xc7, 0x07, 0x1c, 0x38, 0x3b, 0xfc, 0xef, 0xf3, 0x8e, 0x0e, 0x70, 0x71, 0xc3, 0xc7, 0x0e, 0x1c, 0xf0, 0xff, 0x00, 

    /* 0x00d1 */
      0,        /* page */
    209,        /* index */
      0,        /* size (MSB) */
     36,        /* size (LSB) */
     16,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x01, 0xd0, 0x02, 0xe0, 0x00, 0x00, 0x3c, 0x1f, 0x1c, 0x0e, 0x1e, 0x0c, 0x1e, 0x0c, 0x1b, 0x18, 0x33, 0x98, 0x33, 0x98, 0x31, 0xd8, 0x31, 0xd0, 0x60, 0xf0, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x60, 

    /* 0x00d2 */
      0,        /* page */
    210,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x07, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x00, 0x03, 0xe0, 0x3d, 0xc1, 0xc3, 0x8e, 0x0e, 0x38, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x0e, 0x70, 0x71, 0xc1, 0xc7, 0x0e, 0x0e, 0xf0, 0x1f, 0x00, 

    /* 0x00d3 */
      0,        /* page */
    211,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0x38, 0x01, 0xc0, 0x08, 0x00, 0x00, 0x03, 0xe0, 0x3d, 0xc1, 0xc3, 0x8e, 0x0e, 0x38, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x0e, 0x70, 0x71, 0xc1, 0xc7, 0x0e, 0x0e, 0xf0, 0x1f, 0x00, 

    /* 0x00d4 */
      0,        /* page */
    212,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0xc0, 0x07, 0x80, 0x21, 0x00, 0x00, 0x03, 0xe0, 0x3d, 0xc1, 0xc3, 0x8e, 0x0e, 0x38, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x0e, 0x70, 0x71, 0xc1, 0xc7, 0x0e, 0x0e, 0xf0, 0x1f, 0x00, 

    /* 0x00d5 */
      0,        /* page */
    213,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x01, 0xd0, 0x0b, 0x80, 0x00, 0x00, 0xf8, 0x0f, 0x70, 0x70, 0xe3, 0x83, 0x8e, 0x0e, 0x70, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x1c, 0x70, 0x71, 0xc3, 0x83, 0xbc, 0x07, 0xc0, 

    /* 0x00d6 */
      0,        /* page */
    214,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x03, 0x30, 0x0c, 0xc0, 0x00, 0x00, 0xf8, 0x0f, 0x70, 0x70, 0xe3, 0x83, 0x8e, 0x0e, 0x70, 0x39, 0xc0, 0xe7, 0x03, 0x9c, 0x1c, 0x70, 0x71, 0xc3, 0x83, 0xbc, 0x07, 0xc0, 

    /* 0x00d7 */
      0,        /* page */
    215,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      9,        /* ascent */
    0xc1, 0x98, 0xc3, 0x60, 0x70, 0x1c, 0x0d, 0x86, 0x33, 0x06, 

    /* 0x00d8 */
      0,        /* page */
    216,        /* index */
      0,        /* size (MSB) */
     31,        /* size (LSB) */
     14,        /* width */
     15,        /* height */
     14,        /* ascent */
    0x00, 0x0c, 0x0f, 0xe0, 0xf7, 0x07, 0x1e, 0x38, 0xf8, 0xe2, 0xe7, 0x1b, 0x9c, 0xce, 0x76, 0x39, 0xd1, 0xc7, 0xc7, 0x1e, 0x38, 0x3b, 0xc0, 0xfc, 0x06, 0x00, 0x00, 

    /* 0x00d9 */
      0,        /* page */
    217,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x0e, 0x00, 0x1c, 0x00, 0x08, 0x00, 0x00, 0x7e, 0x7c, 0xe0, 0xe3, 0x83, 0x1e, 0x0c, 0x70, 0x61, 0xc1, 0x87, 0x06, 0x3c, 0x18, 0xe0, 0xc3, 0x83, 0x0e, 0x0c, 0x3c, 0x60, 0x7f, 0x00, 

    /* 0x00da */
      0,        /* page */
    218,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x00, 0x70, 0x03, 0x80, 0x10, 0x00, 0x00, 0x7e, 0x7c, 0xe0, 0xe3, 0x83, 0x1e, 0x0c, 0x70, 0x61, 0xc1, 0x87, 0x06, 0x3c, 0x18, 0xe0, 0xc3, 0x83, 0x0e, 0x0c, 0x3c, 0x60, 0x7f, 0x00, 

    /* 0x00db */
      0,        /* page */
    219,        /* index */
      0,        /* size (MSB) */
     34,        /* size (LSB) */
     14,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x01, 0x80, 0x0f, 0x00, 0x42, 0x00, 0x00, 0x7e, 0x7c, 0xe0, 0xe3, 0x83, 0x1e, 0x0c, 0x70, 0x61, 0xc1, 0x87, 0x06, 0x3c, 0x18, 0xe0, 0xc3, 0x83, 0x0e, 0x0c, 0x3c, 0x60, 0x7f, 0x00, 

    /* 0x00dc */
      0,        /* page */
    220,        /* index */
      0,        /* size (MSB) */
     32,        /* size (LSB) */
     14,        /* width */
     16,        /* height */
     16,        /* ascent */
    0x06, 0x60, 0x19, 0x80, 0x00, 0x1f, 0x9f, 0x38, 0x38, 0xe0, 0xc7, 0x83, 0x1c, 0x18, 0x70, 0x61, 0xc1, 0x8f, 0x06, 0x38, 0x30, 0xe0, 0xc3, 0x83, 0x0f, 0x18, 0x1f, 0xc0, 

    /* 0x00dd */
      0,        /* page */
    221,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     12,        /* width */
     17,        /* height */
     17,        /* ascent */
    0x01, 0xc0, 0x38, 0x04, 0x00, 0x00, 0xf8, 0xf7, 0x06, 0x70, 0xc3, 0x98, 0x3b, 0x01, 0xe0, 0x1c, 0x01, 0xc0, 0x1c, 0x03, 0x80, 0x38, 0x03, 0x80, 0xfe, 0x00, 

    /* 0x00de */
      0,        /* page */
    222,        /* index */
      0,        /* size (MSB) */
     22,        /* size (LSB) */
     11,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x3e, 0x03, 0x80, 0x70, 0x0f, 0xe3, 0x9e, 0x71, 0xce, 0x39, 0xc7, 0x31, 0xcf, 0xf1, 0xc0, 0x38, 0x0f, 0x80, 

    /* 0x00df */
      0,        /* page */
    223,        /* index */
      0,        /* size (MSB) */
     30,        /* size (LSB) */
     12,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x01, 0xe0, 0x37, 0x06, 0x70, 0x67, 0x06, 0xe0, 0xe8, 0x0e, 0xc0, 0xce, 0x0c, 0xe1, 0xce, 0x1c, 0xe1, 0x9c, 0x1b, 0x81, 0x80, 0xd8, 0x0d, 0x80, 0x70, 0x00, 

    /* 0x00e0 */
      0,        /* page */
    224,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x38, 0x07, 0x00, 0x20, 0x00, 0x1e, 0x9e, 0xe7, 0x3b, 0x8c, 0xe7, 0x39, 0xce, 0xf4, 0xee, 

    /* 0x00e1 */
      0,        /* page */
    225,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x03, 0x81, 0xc0, 0x80, 0x00, 0x1e, 0x9e, 0xe7, 0x3b, 0x8c, 0xe7, 0x39, 0xce, 0xf4, 0xee, 

    /* 0x00e2 */
      0,        /* page */
    226,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x06, 0x03, 0xc1, 0x08, 0x00, 0x1e, 0x9e, 0xe7, 0x3b, 0x8c, 0xe7, 0x39, 0xce, 0xf4, 0xee, 

    /* 0x00e3 */
      0,        /* page */
    227,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x0e, 0x85, 0xc0, 0x00, 0x7a, 0x7b, 0x9c, 0xee, 0x33, 0x9c, 0xe7, 0x3b, 0xd3, 0xb8, 

    /* 0x00e4 */
      0,        /* page */
    228,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x19, 0x86, 0x60, 0x00, 0x7a, 0x7b, 0x9c, 0xee, 0x33, 0x9c, 0xe7, 0x3b, 0xd3, 0xb8, 

    /* 0x00e5 */
      0,        /* page */
    229,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
     10,        /* width */
     13,        /* height */
     13,        /* ascent */
    0x06, 0x02, 0x40, 0x90, 0x18, 0x00, 0x07, 0xa7, 0xb9, 0xce, 0xe3, 0x39, 0xce, 0x73, 0xbd, 0x3b, 0x80, 

    /* 0x00e6 */
      0,        /* page */
    230,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     14,        /* width */
      8,        /* height */
      8,        /* ascent */
    0x1d, 0x71, 0xee, 0xe7, 0x33, 0xb8, 0xdc, 0xe7, 0xc3, 0x9c, 0x6e, 0xfb, 0x0e, 0xf8, 

    /* 0x00e7 */
      0,        /* page */
    231,        /* index */
      0,        /* size (MSB) */
     16,        /* size (LSB) */
      8,        /* width */
     12,        /* height */
      8,        /* ascent */
    0x1f, 0x7b, 0x60, 0xe0, 0xe0, 0xe0, 0xf6, 0x3c, 0x30, 0x18, 0x98, 0xf0, 

    /* 0x00e8 */
      0,        /* page */
    232,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x38, 0x0e, 0x00, 0x80, 0x01, 0xe3, 0xb9, 0x9d, 0xdc, 0xf8, 0x71, 0xbd, 0x8f, 0x80, 

    /* 0x00e9 */
      0,        /* page */
    233,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x07, 0x07, 0x04, 0x00, 0x01, 0xe3, 0xb9, 0x9d, 0xdc, 0xf8, 0x71, 0xbd, 0x8f, 0x80, 

    /* 0x00ea */
      0,        /* page */
    234,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
      9,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x0c, 0x0f, 0x08, 0x40, 0x01, 0xe3, 0xb9, 0x9d, 0xdc, 0xf8, 0x71, 0xbd, 0x8f, 0x80, 

    /* 0x00eb */
      0,        /* page */
    235,        /* index */
      0,        /* size (MSB) */
     17,        /* size (LSB) */
      9,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x33, 0x19, 0x80, 0x03, 0xc7, 0x73, 0x3b, 0xb9, 0xf0, 0xe3, 0x7b, 0x1f, 0x00, 

    /* 0x00ec */
      0,        /* page */
    236,        /* index */
      0,        /* size (MSB) */
     12,        /* size (LSB) */
      5,        /* width */
     12,        /* height */
     12,        /* ascent */
    0xe3, 0x82, 0x0f, 0x39, 0xcc, 0x67, 0x3c, 0xc0, 

    /* 0x00ed */
      0,        /* page */
    237,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      6,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x1c, 0xe4, 0x00, 0xf1, 0xc7, 0x18, 0x63, 0x8f, 0x18, 

    /* 0x00ee */
      0,        /* page */
    238,        /* index */
      0,        /* size (MSB) */
     13,        /* size (LSB) */
      6,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x31, 0xe8, 0x40, 0xf1, 0xc7, 0x18, 0x63, 0x8f, 0x18, 

    /* 0x00ef */
      0,        /* page */
    239,        /* index */
      0,        /* size (MSB) */
     11,        /* size (LSB) */
      5,        /* width */
     11,        /* height */
     11,        /* ascent */
    0xde, 0xc1, 0xe7, 0x39, 0x8c, 0xe7, 0x98, 

    /* 0x00f0 */
      0,        /* page */
    240,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x30, 0x06, 0xc1, 0xc0, 0x98, 0x1f, 0x1c, 0xe6, 0x3b, 0x8e, 0xe3, 0xb8, 0xce, 0x71, 0xf0, 

    /* 0x00f1 */
      0,        /* page */
    241,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x1d, 0x0b, 0x80, 0x03, 0xdc, 0x7b, 0x9c, 0xe7, 0x39, 0x8c, 0xe7, 0x39, 0xde, 0x78, 

    /* 0x00f2 */
      0,        /* page */
    242,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x38, 0x07, 0x00, 0x20, 0x00, 0x1f, 0x1c, 0xe6, 0x3b, 0x8e, 0xe3, 0xb8, 0xce, 0x71, 0xf0, 

    /* 0x00f3 */
      0,        /* page */
    243,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x03, 0x81, 0xc0, 0x80, 0x00, 0x1f, 0x1c, 0xe6, 0x3b, 0x8e, 0xe3, 0xb8, 0xce, 0x71, 0xf0, 

    /* 0x00f4 */
      0,        /* page */
    244,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x0c, 0x07, 0x82, 0x10, 0x00, 0x1f, 0x1c, 0xe6, 0x3b, 0x8e, 0xe3, 0xb8, 0xce, 0x71, 0xf0, 

    /* 0x00f5 */
      0,        /* page */
    245,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x1d, 0x0b, 0x80, 0x00, 0x7c, 0x73, 0x98, 0xee, 0x3b, 0x8e, 0xe3, 0x39, 0xc7, 0xc0, 

    /* 0x00f6 */
      0,        /* page */
    246,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x33, 0x0c, 0xc0, 0x00, 0x7c, 0x73, 0x98, 0xee, 0x3b, 0x8e, 0xe3, 0x39, 0xc7, 0xc0, 

    /* 0x00f7 */
      0,        /* page */
    247,        /* index */
      0,        /* size (MSB) */
     14,        /* size (LSB) */
     10,        /* width */
      8,        /* height */
      9,        /* ascent */
    0x0c, 0x03, 0x00, 0x01, 0xfe, 0x7f, 0x80, 0x00, 0xc0, 0x30, 

    /* 0x00f8 */
      0,        /* page */
    248,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     10,        /* ascent */
    0x01, 0x80, 0xc1, 0xf1, 0xde, 0x6d, 0xba, 0x6c, 0xbb, 0x4c, 0xf7, 0x1f, 0x06, 0x03, 0x00, 

    /* 0x00f9 */
      0,        /* page */
    249,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x70, 0x0e, 0x00, 0x40, 0x00, 0xf3, 0x9c, 0xe7, 0x31, 0x9c, 0xe7, 0x3b, 0xcf, 0xb5, 0xce, 

    /* 0x00fa */
      0,        /* page */
    250,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x03, 0x81, 0xc0, 0x80, 0x00, 0xf3, 0x9c, 0xe7, 0x31, 0x9c, 0xe7, 0x3b, 0xcf, 0xb5, 0xce, 

    /* 0x00fb */
      0,        /* page */
    251,        /* index */
      0,        /* size (MSB) */
     19,        /* size (LSB) */
     10,        /* width */
     12,        /* height */
     12,        /* ascent */
    0x0c, 0x07, 0x82, 0x10, 0x00, 0xf3, 0x9c, 0xe7, 0x31, 0x9c, 0xe7, 0x3b, 0xcf, 0xb5, 0xce, 

    /* 0x00fc */
      0,        /* page */
    252,        /* index */
      0,        /* size (MSB) */
     18,        /* size (LSB) */
     10,        /* width */
     11,        /* height */
     11,        /* ascent */
    0x33, 0x0c, 0xc0, 0x03, 0xce, 0x73, 0x9c, 0xc6, 0x73, 0x9c, 0xef, 0x3e, 0xd7, 0x38, 

    /* 0x00fd */
      0,        /* page */
    253,        /* index */
      0,        /* size (MSB) */
     24,        /* size (LSB) */
     10,        /* width */
     16,        /* height */
     12,        /* ascent */
    0x01, 0xc0, 0xe0, 0x40, 0x00, 0x7b, 0x8e, 0x63, 0x98, 0xe4, 0x1b, 0x07, 0x81, 0xe0, 0x30, 0x0c, 0x32, 0x0d, 0x81, 0xc0, 

    /* 0x00fe */
      0,        /* page */
    254,        /* index */
      0,        /* size (MSB) */
     28,        /* size (LSB) */
     11,        /* width */
     17,        /* height */
     13,        /* ascent */
    0x1e, 0x01, 0xc0, 0x38, 0x06, 0x00, 0xc0, 0x3b, 0x87, 0xb8, 0xe7, 0x1c, 0xe7, 0x1c, 0xe7, 0x1c, 0xc3, 0x70, 0x60, 0x1c, 0x03, 0x80, 0xf8, 0x00, 

    /* 0x00ff */
      0,        /* page */
    255,        /* index */
      0,        /* size (MSB) */
     21,        /* size (LSB) */
      9,        /* width */
     15,        /* height */
     11,        /* ascent */
    0x33, 0x19, 0x80, 0x0f, 0x73, 0x99, 0xcc, 0xe4, 0x36, 0x1e, 0x0f, 0x03, 0x01, 0x8c, 0x86, 0xc1, 0xc0, 

    /* End of page */
    0, 0, 0, 0
};

UGL_LOCAL const UGL_UINT8 * const pageArray[] = {
    page0Data,
    UGL_NULL
};

const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_18 = {
    {
        {18, 18},                       /* pixelSize */
        {UGL_FONT_BOLD, UGL_FONT_BOLD}, /* weight */
        UGL_FONT_ITALIC,                /* italic */
        UGL_FONT_PROPORTIONAL,          /* spacing */
        UGL_FONT_ISO_8859_1,            /* charSet */
        "Times Bold Italic",            /* faceName */
        "Times"                         /* familyName */
    },

      2,                /* leading */
     17,                /* maxAscent */
      4,                /* maxDescent */
     18,                /* maxAdvance */
    pageArray           /* pageData */
};

