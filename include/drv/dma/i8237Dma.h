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

/* i8237Dma.h - Intel 8237 DMA controller header */

#ifndef _i8237Dma_h
#define _i8237Dma_h

/* DMA1 I/O */
#define DMA1_BASE               0x00

#define DMA1_STATUS             0x08
#define DMA1_CMD                0x08
#define DMA1_MASK_SINGLE        0x0a
#define DMA1_MODE               0x0b
#define DMA1_CLEAR_FF           0x0c
#define DMA1_CLEAR_ALL          0x0d
#define DMA1_CLEAR_MASK         0x0e
#define DMA1_MASK_ALL           0x0f

/* DMA2 I/O */
#define DMA2_BASE               0x00

#define DMA2_STATUS             0xd0
#define DMA2_CMD                0xd0
#define DMA2_MASK_SINGLE        0xd4
#define DMA2_MODE               0xd6
#define DMA2_CLEAR_FF           0xd8
#define DMA2_CLEAR_ALL          0xda
#define DMA2_CLEAR_MASK         0xdc
#define DMA2_MASK_ALL           0xde

/* Mode register bit */
#define DMA_MODE_DEMAND         0x00
#define DMA_MODE_SINGLE         0x40
#define DMA_MODE_BLOCK          0x80
#define DMA_MODE_CASCADE        0xc0
#define DMA_MODE_DECREMENT      0x20
#define DMA_MODE_INCREMENT      0x00
#define DMA_MODE_AUTO_ENABLE    0x10
#define DMA_MODE_AUTO_DISABLE   0x00
#define DMA_MODE_WRITE          0x08
#define DMA_MODE_READ           0x04

/* Mask resiger bit */
#define DMA_MASK_SET            0x04
#define DMA_MASK_RESET          0x00

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

#define DMA1_ADDR(c)            (DMA1_BASE + (c << 1))
#define DMA1_COUNT(c)           (DMA1_BASE + (c << 1) + 1)

#define DMA2_ADDR(c)            (DMA2_BASE + ((c - 4) << 2))
#define DMA2_COUNT(c)           (DMA2_BASE + ((c - 4) << 2) + 2)

/* Functions */

/******************************************************************************
 * dmaSetup - Setup dma controller
 *
 * RETURNS: OK or ERROR
 */

STATUS dmaSetup(
    int           dir,
    void         *buf,
    unsigned int  nBytes,
    unsigned int  c
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _i8237Dma_h */

