/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2010 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* if_type.h - Interface types */

#ifndef _if_type_h
#define _if_type_h

#define IFT_OTHER                       0x01            /* Unknown */
#define IFT_1822                        0x02            /* Arpanet imp */
#define IFT_HDH1822                     0x03            /* HDH arpanet imp */
#define IFT_X25DNN                      0x04            /* X25 to imp */
#define IFT_X25                         0x05            /* X25 */
#define IFT_ETHER                       0x06            /* Etnernet CSMACD */
#define IFT_ISO88023                    0x07            /* CMSA CD */
#define IFT_ISO88024                    0x08            /* Token bus */
#define IFT_ISO88025                    0x09            /* Token ring */
#define IFT_ISO88026                    0x0a            /* MAN */
#define IFT_STARLAN                     0x0b
#define IFT_P10                         0x0c            /* Proteon 10MBit rng */
#define IFT_P80                         0x0d            /* Proteon 10MBit rng */
#define IFT_HY                          0x0e            /* Hyperchannel */
#define IFT_FDDI                        0x0f
#define IFT_LAPB                        0x10
#define IFT_SDLC                        0x11
#define IFT_T1                          0x12
#define IFT_CEPT                        0x13            /* E1 Europeian T1 */
#define IFT_ISDNBASIC                   0x14
#define IFT_ISDNPRIMARY                 0x15
#define IFT_PTPSERIAL                   0x16            /* PTP Serial */
#define IFT_PPP                         0x17            /* RFC 1331 */
#define IFT_LOOP                        0x18            /* Loopback */

#define IFT_PMP                         0x9d            /* Pnt to multipnt */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _if_type_h */

