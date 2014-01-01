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

/* udbmffnt.c - Bitmap font driver for Universal Graphics Library */

#include <ugl/ugl.h>
#include <ugl/driver/font/udbmffnt.h>

/* Defines */

#define UGL_BMF_FONT_ENGINE_VERSION     1
#define UGL_BMF_FONT_DRIVER_VERSION     1
#define UGL_BMF_RETRY_SLEEP_TIME        100
#define UGL_BMF_RETRY_TIMES             100
#define UGL_BMF_GLYPH_WIDTH_INDEX       2
#define UGL_BMF_GLYPH_HEIGHT_INDEX      3
#define UGL_BMF_GLYPH_ASCENT_INDEX      4
#define UGL_BMF_GLYPH_BMP_INDEX         5
#define UGL_BMF_LONG_TEXT               0xfff

/* Imports */

extern const UGL_BMF_FONT_DESC * uglBMFFontData[];
extern UGL_MEM_POOL_ID           uglBMFGlyphCachePoolId;
extern UGL_SIZE                  uglBMFGlyphCacheSize;

/* Locals */

UGL_LOCAL UGL_STATUS uglBMFFontDriverInfo (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_INFO_REQ        infoRequest,
    void *              pInfo
    );

UGL_LOCAL UGL_STATUS uglBMFFontDriverDestroy (
    UGL_FONT_DRIVER_ID  drvId
    );

UGL_LOCAL UGL_SEARCH_ID uglBMFFontFindFirst (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor
    );

UGL_LOCAL UGL_STATUS uglBMFFontFindNext (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor,
    UGL_SEARCH_ID       searchId
    );

UGL_LOCAL UGL_STATUS uglBMFFontFindClose (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_SEARCH_ID       searchId
    );

UGL_LOCAL UGL_FONT_ID uglBMFFontCreate (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DEF *      pFontDefinition
    );

UGL_LOCAL UGL_STATUS uglBMFFontDestroy (
    UGL_FONT_ID  fontId 
    );

UGL_LOCAL UGL_STATUS uglBMFFontInfo (
    UGL_FONT_ID   fontId,
    UGL_INFO_REQ  infoRequest,
    void *        pInfo
    );

UGL_LOCAL UGL_STATUS uglBMFFontMetricsGet (
    UGL_FONT_ID        fontId,
    UGL_FONT_METRICS * pFontMetrics
    );

UGL_LOCAL UGL_STATUS uglBMFTextSizeGet (
    UGL_FONT_ID        fontId,
    UGL_SIZE *         pWidth,
    UGL_SIZE *         pHeight,
    UGL_SIZE           length,
    const UGL_CHAR *   pText
    );

UGL_LOCAL UGL_STATUS uglBMFTextDraw (
    UGL_GC_ID          gc,
    UGL_POS            x,
    UGL_POS            y,
    UGL_SIZE           length,
    const UGL_CHAR *   pText
    );

UGL_LOCAL UGL_GLYPH_CACHE_ELEMENT * uglBMFGlyphCacheAlloc (
    UGL_FONT_DRIVER_ID  drvId,
    void **             ppPageElement
    );

UGL_LOCAL UGL_STATUS uglBMFGlyphCacheFree (
    UGL_FONT_DRIVER_ID  drvId,
    void **             ppPageElement
    );

/******************************************************************************
 *
 * uglBMFFontDriverCreate - Create bitmap font driver
 *
 * RETURNS: UGL_FONT_DRIVER_ID or UGL_NULL
 */

UGL_FONT_DRIVER_ID uglBMFFontDriverCreate (
    UGL_DEVICE_ID  devId
    ) {
    UGL_BMF_FONT_DRIVER * pBMFDrv;
    UGL_FONT_DRIVER *     pDrv;
    UGL_SIZE              numFonts;
    UGL_LOCK_ID           lockId;

    if (uglBMFFontData == NULL) {
        return (UGL_NULL);
    }

    /* Check number of fonts */
    for (numFonts = 0; uglBMFFontData[numFonts] != UGL_NULL; numFonts++);
    if (numFonts == 0) {
        return (UGL_NULL);
    }

    /* Allocate memory for driver */
    pBMFDrv =
        (UGL_BMF_FONT_DRIVER *) UGL_CALLOC (1, sizeof (UGL_BMF_FONT_DRIVER));
    if (pBMFDrv == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Create lock */
    lockId = uglOSLockCreate ();
    if (lockId == UGL_NULL) {
        UGL_FREE (pBMFDrv);
        return (UGL_NULL);
    }

    /* Setup struct */
    pDrv = (UGL_FONT_DRIVER *) pBMFDrv;
    pDrv->pDriver = devId;
    pDrv->fontDriverInfo    = uglBMFFontDriverInfo;
    pDrv->fontDriverDestroy = uglBMFFontDriverDestroy;
    pDrv->fontFindFirst     = uglBMFFontFindFirst;
    pDrv->fontFindNext      = uglBMFFontFindNext;
    pDrv->fontFindClose     = uglBMFFontFindClose;
    pDrv->fontCreate        = uglBMFFontCreate;
    pDrv->fontDestroy       = uglBMFFontDestroy;
    pDrv->fontInfo          = uglBMFFontInfo;
    pDrv->fontMetricsGet    = uglBMFFontMetricsGet;
    pDrv->textSizeGet       = uglBMFTextSizeGet;
    pDrv->textDraw          = uglBMFTextDraw;

    /* Setup driver specific part of struct */
    pBMFDrv->textOrigin = UGL_FONT_TEXT_BASELINE;
    pBMFDrv->lockId     = lockId;

    return (UGL_FONT_DRIVER_ID) pBMFDrv;
}

/******************************************************************************
 *
 * uglBMFFontDriverInfo - Get information about bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontDriverInfo (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_INFO_REQ        infoRequest,
    void *              pInfo
    ) {
    UGL_BMF_FONT_DRIVER * pDrv   = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_STATUS            status = UGL_STATUS_ERROR;

    switch (infoRequest) {
        case UGL_FONT_ENGINE_VERSION_GET:
            if (pInfo != UGL_NULL) {
                *(UGL_INT32 *) pInfo = UGL_BMF_FONT_ENGINE_VERSION;
                status = UGL_STATUS_OK;
            }
            break;

        case UGL_FONT_DRIVER_VERSION_GET:
            if (pInfo != UGL_NULL) {
                *(UGL_INT32 *) pInfo = UGL_BMF_FONT_DRIVER_VERSION;
                status = UGL_STATUS_OK;
            }
            break;

        case UGL_FONT_TEXT_ORIGIN:
            if (pInfo != UGL_NULL) {
                if (*(UGL_ORD *) pInfo == UGL_FONT_TEXT_UPPER_LEFT ||
                    *(UGL_ORD *) pInfo == UGL_FONT_TEXT_BASELINE) {
                    if (uglOSLock(pDrv->lockId) == UGL_STATUS_ERROR) {
                        return (status);
                    }
                    pDrv->textOrigin = *(UGL_ORD *) pInfo;
                    uglOSUnlock (pDrv->lockId);
                    status = UGL_STATUS_OK;
                }
            }
            break;

        default:
            if (pInfo != UGL_NULL) {
                *(UGL_BOOL *) pInfo = UGL_FALSE;
            }
            break;
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontDriverDestroy - Destroy bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontDriverDestroy (
    UGL_FONT_DRIVER_ID  drvId
    ) {
    UGL_BMF_FONT_DRIVER * pDrv   = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_STATUS            status = UGL_STATUS_OK;

    uglOSLockDestroy (pDrv->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontFindFirst - Find first font in font driver
 *
 * RETURNS: UGL_SEARCH_ID or UGL_NULL
 */

UGL_LOCAL UGL_SEARCH_ID uglBMFFontFindFirst (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor
    ) {
    UGL_SEARCH_ID  searchId = UGL_NULL;

    if (uglBMFFontData != UGL_NULL) {
        searchId = (UGL_SEARCH_ID) UGL_MALLOC (sizeof (UGL_SEARCH_ID));
        if (searchId != UGL_NULL) {
            *pFontDescriptor = *(UGL_FONT_DESC *) uglBMFFontData[0];
            *(UGL_UINT32 *) searchId = 1;
        }
    }

    return (searchId);
}

/******************************************************************************
 *
 * uglBMFFontFindNext - Find next font in font driver
 *
 * RETURNS: UGL_STATUS_OK, UGL_STATUS_FINISHED or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontFindNext (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor,
    UGL_SEARCH_ID       searchId
    ) {
    UGL_UINT32  index;
    UGL_STATUS  status = UGL_STATUS_ERROR;

    if (searchId != UGL_NULL) {
        index = *(UGL_UINT32 *) searchId;
        if (uglBMFFontData[index] != UGL_NULL) {
            *pFontDescriptor = *(UGL_FONT_DESC *) uglBMFFontData[index++];
            *(UGL_UINT32 *) searchId = index;
            status = UGL_STATUS_OK;
        }
        else {
            status = UGL_STATUS_FINISHED;
        }
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontFindClose - Terminate font search for font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontFindClose (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_SEARCH_ID       searchId
    ) {
    UGL_STATUS  status = UGL_STATUS_ERROR;

    if (searchId != UGL_NULL) {
        UGL_FREE (searchId);
        status = UGL_STATUS_OK;
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontCreate - Create font for font driver
 *
 * RETURNS: UGL_FONT_ID or UGL_NULL
 */

UGL_LOCAL UGL_FONT_ID uglBMFFontCreate (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DEF *      pFontDefinition
    ) {
    UGL_INT32             i;
    UGL_INT32             pageIndex;
    UGL_INT32             page;
    UGL_INT32             index;
    UGL_UINT32            size;
    UGL_BMF_FONT *        pBMFFont;
    const UGL_UINT8 *     pPageData;
    UGL_FONT *            pFont   = UGL_NULL;
    UGL_BMF_FONT_DRIVER * pBMFDrv = (UGL_BMF_FONT_DRIVER *) drvId;

    if (uglOSLock(pBMFDrv->lockId) == UGL_STATUS_ERROR) {
        return (UGL_NULL);
    }

    /* Get first created font */
    pBMFFont = pBMFDrv->pFirstFont;

    /* Check if matching font exists */
    if (pBMFFont != UGL_NULL) {
        for (pFont = (UGL_FONT *) pBMFFont;
             pFont != UGL_NULL;
             pBMFFont = pBMFFont->pNextFont, pFont = (UGL_FONT *) pBMFFont) {
            if (pFontDefinition->structSize == sizeof (UGL_FONT_DEF) &&
                pBMFFont->pBMFFontDesc->header.pixelSize.min ==
                pFontDefinition->pixelSize &&
                pBMFFont->pBMFFontDesc->header.weight.min ==
                pFontDefinition->weight &&
                pBMFFont->pBMFFontDesc->header.italic ==
                pFontDefinition->italic &&
                pBMFFont->pBMFFontDesc->header.charSet ==
                pFontDefinition->charSet &&
                (strncmp(pBMFFont->pBMFFontDesc->header.faceName,
                 pFontDefinition->faceName,
                 UGL_FONT_FACE_NAME_MAX_LENGTH - 1) == 0) &&
                (strncmp(pBMFFont->pBMFFontDesc->header.familyName,
                 pFontDefinition->familyName,
                 UGL_FONT_FAMILY_NAME_MAX_LENGTH - 1) == 0)) {

                /* Found match */
                pBMFFont->referenceCount++;
                uglOSUnlock (pBMFDrv->lockId);
                return (UGL_FONT_ID) pBMFFont;
            }
        }
    }

    /* Font needs to be created */
    pBMFFont = (UGL_BMF_FONT *) UGL_CALLOC (1, sizeof (UGL_BMF_FONT));
    if (pBMFFont != UGL_NULL) {
        pageIndex = 0;
        pFont = (UGL_FONT *) pBMFFont;

        /* Search font array for match */
        for (i = 0; uglBMFFontData[i] != UGL_NULL; i++) {
            if (pFontDefinition->structSize == sizeof (UGL_FONT_DEF) &&
                uglBMFFontData[i]->header.pixelSize.min ==
                pFontDefinition->pixelSize &&
                uglBMFFontData[i]->header.weight.min ==
                pFontDefinition->weight &&
                uglBMFFontData[i]->header.italic ==
                pFontDefinition->italic &&
                uglBMFFontData[i]->header.charSet ==
                pFontDefinition->charSet &&
                (strncmp(uglBMFFontData[i]->header.faceName,
                 pFontDefinition->faceName,
                 UGL_FONT_FACE_NAME_MAX_LENGTH - 1) == 0) &&
                (strncmp(uglBMFFontData[i]->header.familyName,
                 pFontDefinition->familyName,
                 UGL_FONT_FAMILY_NAME_MAX_LENGTH - 1) == 0)) {

                /* Found exact match */
                pFont->pFontDriver = drvId;
                pBMFFont->pBMFFontDesc   = uglBMFFontData[i];
                pBMFFont->referenceCount = 1;
                pBMFFont->pageTable[0]   = &pBMFFont->pageZero;

                while (uglBMFFontData[i]->pageData[pageIndex] != UGL_NULL) {
                    pPageData = uglBMFFontData[i]->pageData[pageIndex];
                    size = 0;

                    while (1) {

                        /* Get page and advance */
                        page = *pPageData;
                        pPageData++;

                        /* Get index and advance */
                        index = *pPageData;
                        pPageData++;

                        /* Get size */
                        size  = (*pPageData) << 8;
                        size += *(pPageData + 1);

                        /* Check if end of data */
                        if (size == 0) {
                            break;
                        }

                        /* Create font page if needed */
                        if (pBMFFont->pageTable[page] == UGL_NULL) {
                            pBMFFont->pageTable[page] = (UGL_FONT_PAGE *)
                                UGL_CALLOC (1, sizeof (UGL_FONT_PAGE));
                        }

                        /* Set page table pointer */
                        if (pBMFFont->pageTable[page] != UGL_NULL) {
                            (*pBMFFont->pageTable[page])[index] = (UGL_UINT8 *)
                                pPageData;
                        }

                        /* Advance to next glyph */
                        pPageData += (size + 1);
                    }

                    /* Advance to next page */
                    pageIndex++;
                }

                /* Setup rest of bmf font structure */
                pBMFFont->textOrigin = pBMFDrv->textOrigin;

                /* Add created font to font list */
                if (pBMFDrv->pFirstFont != UGL_NULL) {
                    pBMFFont->pPrevFont = pBMFDrv->pLastFont;
                    pBMFDrv->pLastFont->pNextFont = pBMFFont;
                    pBMFDrv->pLastFont = pBMFFont;
                }
                else {
                    pBMFDrv->pFirstFont = pBMFFont;
                    pBMFDrv->pLastFont  = pBMFFont;
                }

                /* Unlock and return */
                uglOSUnlock (pBMFDrv->lockId);
                return (UGL_FONT_ID) pBMFFont;
            }
        }
    }

    uglOSUnlock (pBMFDrv->lockId);
    return (UGL_NULL);
}

/******************************************************************************
 *
 * uglBMFFontDestroy - Destroy font for bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontDestroy (
    UGL_FONT_ID  fontId
    ) {
    UGL_INT32             i;
    UGL_INT32             j;
    UGL_BMF_FONT *        pBMFLoopFont;
    UGL_FONT_DRIVER *     pDrv     = fontId->pFontDriver;
    UGL_BMF_FONT_DRIVER * pBMFDrv  =
        (UGL_BMF_FONT_DRIVER *) fontId->pFontDriver;
    UGL_BMF_FONT *        pBMFFont = (UGL_BMF_FONT *) fontId;
    UGL_STATUS            status   = UGL_STATUS_OK;

    if (uglOSLock(pBMFDrv->lockId) == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    if (--pBMFFont->referenceCount == 0) {
        if (pBMFFont == pBMFDrv->pFirstFont) {
            pBMFDrv->pFirstFont = pBMFFont->pNextFont;
            if (pBMFDrv->pFirstFont != UGL_NULL) {
                pBMFDrv->pFirstFont->pPrevFont = UGL_NULL;
            }
        }
        else if (pBMFFont == pBMFDrv->pLastFont) {
            pBMFDrv->pLastFont = pBMFFont->pPrevFont;
            if (pBMFDrv->pLastFont != UGL_NULL) {
                pBMFDrv->pLastFont->pNextFont = UGL_NULL;
            }
        }
        else {
            for (pBMFLoopFont = pBMFDrv->pFirstFont;
                 pBMFLoopFont != UGL_NULL &&
                     pBMFLoopFont->pNextFont != (UGL_BMF_FONT *) fontId;
                 pBMFLoopFont = pBMFLoopFont->pNextFont);

            if (pBMFLoopFont == UGL_NULL) {
                status = UGL_STATUS_ERROR;
            }
            else {
                pBMFLoopFont->pNextFont = ((UGL_BMF_FONT *) fontId)->pNextFont;
                if (pBMFLoopFont->pNextFont != UGL_NULL) {
                    pBMFLoopFont->pNextFont->pPrevFont = pBMFLoopFont;
                }
                pBMFFont = pBMFLoopFont;
            }
        }

        if (status == UGL_STATUS_OK) {
            for (i = 0; i < UGL_BMF_FONT_PAGE_TABLE_SIZE; i++) {
                if (pBMFFont->pageTable[i] != UGL_NULL) {
                    for (j = 0; j < UGL_BMF_FONT_PAGE_SIZE; j++) {
                        if (i > 0 && pBMFFont->pageTable[i] != UGL_NULL) {
                            UGL_FREE (pBMFFont->pageTable[i]);
                        }
                    }
                }
            }
        }

        UGL_FREE (fontId);
    }

    uglOSUnlock (pBMFDrv->lockId);
    return (status);
}

/******************************************************************************
 *
 * uglBMFFontInfo - Get information about bitmap font
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontInfo (
    UGL_FONT_ID   fontId,
    UGL_INFO_REQ  infoRequest,
    void *        pInfo
    ) {
    UGL_BMF_FONT * pBMFFont = (UGL_BMF_FONT *) fontId;
    UGL_STATUS     status   = UGL_STATUS_ERROR;

    switch (infoRequest) {
        case UGL_FONT_TEXT_ORIGIN_SET:
            if (pInfo != UGL_NULL) {
                if (*(UGL_ORD *) pInfo == UGL_FONT_TEXT_UPPER_LEFT ||
                    *(UGL_ORD *) pInfo == UGL_FONT_TEXT_BASELINE) {
                    pBMFFont->textOrigin = *(UGL_ORD *) pInfo;
                    status = UGL_STATUS_OK;
                }
            }
            break;

        case UGL_FONT_TEXT_ORIGIN_GET:
            if (pInfo != UGL_NULL) {
                *(UGL_ORD *) pInfo = pBMFFont->textOrigin;
                status = UGL_STATUS_OK;
            }
            break;

        default:
            if (pInfo != UGL_NULL) {
                *(UGL_BOOL *) pInfo = UGL_FALSE;
            }
            break;
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontMetricsGet - Get metrics for bitmap font
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontMetricsGet (
    UGL_FONT_ID        fontId,
    UGL_FONT_METRICS * pFontMetrics
    ) {
    UGL_BMF_FONT * pBMFFont = (UGL_BMF_FONT *) fontId;

    pFontMetrics->pixelSize  = pBMFFont->pBMFFontDesc->header.pixelSize.min;
    pFontMetrics->weight     = pBMFFont->pBMFFontDesc->header.weight.min;
    pFontMetrics->italic     = pBMFFont->pBMFFontDesc->header.italic;
    pFontMetrics->maxAscent  = pBMFFont->pBMFFontDesc->maxAscent;
    pFontMetrics->maxDescent = pBMFFont->pBMFFontDesc->maxDescent;
    pFontMetrics->maxAdvance = pBMFFont->pBMFFontDesc->maxAdvance;
    pFontMetrics->leading    = pBMFFont->pBMFFontDesc->leading;
    pFontMetrics->spacing    = pBMFFont->pBMFFontDesc->header.spacing;
    pFontMetrics->fontType   = UGL_FONT_BITMAPPED;
    pFontMetrics->scalable   = UGL_FALSE;
    pFontMetrics->charSet    = pBMFFont->pBMFFontDesc->header.charSet;
    pFontMetrics->height     =
        pFontMetrics->maxAscent + pFontMetrics->maxDescent;
    strncpy (pFontMetrics->faceName,
             pBMFFont->pBMFFontDesc->header.faceName,
             UGL_FONT_FACE_NAME_MAX_LENGTH);
    pFontMetrics->faceName[UGL_FONT_FACE_NAME_MAX_LENGTH - 1] = '\0';
    strncpy (pFontMetrics->familyName,
             pBMFFont->pBMFFontDesc->header.familyName,
             UGL_FONT_FAMILY_NAME_MAX_LENGTH);
    pFontMetrics->familyName[UGL_FONT_FAMILY_NAME_MAX_LENGTH - 1] = '\0';

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglBMFTextSizeGet - Get text message size
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFTextSizeGet (
    UGL_FONT_ID        fontId,
    UGL_SIZE *         pWidth,
    UGL_SIZE *         pHeight,
    UGL_SIZE           length,
    const UGL_CHAR *   pText
    ) {
    UGL_INT32      i;
    UGL_SIZE       textWidth  = 0;
    UGL_BMF_FONT * pBMFFont   = (UGL_BMF_FONT *) fontId;
    void **        ppPageZero = pBMFFont->pageZero;
    UGL_UINT8 *    pGlyphData;

    if (length < 0) {
        length = UGL_BMF_LONG_TEXT;
    }

    /* Calculate text width */
    if (pWidth != UGL_NULL) {
        if (pText != UGL_NULL && pText[0] != '\0') {
            for (i = 0; *pText != '\0' && i < length; i++) {
                pGlyphData = (UGL_UINT8 *) ppPageZero[(UGL_UINT8) *pText];
                pText++;

                if (pGlyphData != UGL_NULL) {
                    textWidth +=
                        (UGL_SIZE) pGlyphData[UGL_BMF_GLYPH_WIDTH_INDEX];
                }
            }
        }

        *pWidth = textWidth;
    }

    if (pHeight != UGL_NULL) {
        *pHeight = pBMFFont->pBMFFontDesc->maxAscent +
                   pBMFFont->pBMFFontDesc->maxDescent;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglBMFTextDraw - Draw text using bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFTextDraw (
    UGL_GC_ID          gc,
    UGL_POS            x,
    UGL_POS            y,
    UGL_SIZE           length,
    const UGL_CHAR *   pText
    ) {
    UGL_INT32                 i;
    UGL_POINT                 point;
    void **                   ppPageElement;
    UGL_GLYPH_CACHE_ELEMENT * pCacheElement;
    UGL_UGI_DRIVER *          pDrv       = gc->pDriver;
    UGL_BMF_FONT *            pBMFFont   = (UGL_BMF_FONT *) gc->pFont;
    UGL_SIZE                  maxAscent  = pBMFFont->pBMFFontDesc->maxAscent;
    void **                   ppPageZero = pBMFFont->pageZero;
    UGL_FONT_DRIVER *         pFntDrv    =
        (UGL_FONT_DRIVER *)   pBMFFont->header.pFontDriver;
    UGL_STATUS                status     = UGL_STATUS_OK;

    if (pBMFFont == UGL_NULL || pText == UGL_NULL || pText[0] == '\0') {
        return (UGL_STATUS_ERROR);
    }

    if (length < 0) {
        length = UGL_BMF_LONG_TEXT;
    }

    for (i = 0; *pText != '\0' && i < length; i++) {
        ppPageElement = &ppPageZero[(UGL_UINT8) *pText];
        pCacheElement = (UGL_GLYPH_CACHE_ELEMENT *) *ppPageElement;

        if (pCacheElement == UGL_NULL) {
            continue;
        }

        if (pCacheElement->cacheFlag != UGL_BMF_GLYPH_IN_CACHE) {
            pCacheElement = uglBMFGlyphCacheAlloc (pFntDrv, ppPageElement);
            if (pCacheElement == UGL_NULL) {
                status = UGL_STATUS_ERROR;
                break;
            }
        }

        /* Draw glyph to screen */
        if (pBMFFont->textOrigin == UGL_FONT_TEXT_UPPER_LEFT) {
            point.x = x;
            point.y = y + maxAscent - (UGL_POS) pCacheElement->ascent;
        }
        else if (pBMFFont->textOrigin == UGL_FONT_TEXT_BASELINE) {
            point.x = x;
            point.y = y - (UGL_POS) pCacheElement->ascent;
        }
        else {
            status = UGL_STATUS_ERROR;
            break;
        }

        status = (*pDrv->monoBitmapBlt) (pDrv, pCacheElement->bitmapId,
                                         &pCacheElement->bitmapRect,
                                         UGL_DEFAULT_ID, &point);

        /* Advance */
        x += (UGL_POS) pCacheElement->width;
        pText++;
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFGlyphCacheAlloc - Allocate cache for glyphs
 *
 * RETURNS: Pointer to glypha cache element or UGL_NULL
 */

UGL_LOCAL UGL_GLYPH_CACHE_ELEMENT * uglBMFGlyphCacheAlloc (
    UGL_FONT_DRIVER_ID  drvId,
    void **             ppPageElement
    ) {
    UGL_INT32                 i;
    UGL_GLYPH_CACHE_ELEMENT * pCacheElement;
    UGL_MDIB                  mDib;
    UGL_BMF_FONT_DRIVER *     pBMFDrv    = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_UGI_DRIVER *          pDrv       = drvId->pDriver;
    UGL_UINT8 *               pGlyphData = (UGL_UINT8 *) *ppPageElement;

    if (uglBMFGlyphCacheSize >= 0 &&
        pBMFDrv->numCachedGlyphs >= uglBMFGlyphCacheSize) {
        if (uglBMFGlyphCacheFree (drvId,
                pBMFDrv->pLastCacheElement->ppPageElement) != UGL_STATUS_OK) {
            return (UGL_NULL);
        }
    }

    i = 0;
    pCacheElement = (UGL_GLYPH_CACHE_ELEMENT *)
        uglOSMemCalloc (uglBMFGlyphCachePoolId,
                        1, sizeof (UGL_GLYPH_CACHE_ELEMENT));
    while (pCacheElement == UGL_NULL) {
        if (uglBMFGlyphCacheFree (drvId,
                pBMFDrv->pLastCacheElement->ppPageElement) != UGL_STATUS_OK) {
            return (UGL_NULL);
        }

        pCacheElement = (UGL_GLYPH_CACHE_ELEMENT *)
            uglOSMemCalloc (uglBMFGlyphCachePoolId,
                            1, sizeof (UGL_GLYPH_CACHE_ELEMENT));

        if (pCacheElement == UGL_NULL) {
            uglOSTaskDelay (UGL_BMF_RETRY_SLEEP_TIME);
            if (++i == UGL_BMF_RETRY_TIMES) {
                return (UGL_NULL);
            }
        }
    }

    /* Setup struct */
    pCacheElement->pGlyphData        = pGlyphData;
    pCacheElement->ppPageElement     = ppPageElement;
    pCacheElement->cacheFlag         = UGL_BMF_GLYPH_IN_CACHE;
    pCacheElement->width             = pGlyphData[UGL_BMF_GLYPH_WIDTH_INDEX];
    pCacheElement->height            = pGlyphData[UGL_BMF_GLYPH_HEIGHT_INDEX];
    pCacheElement->ascent            = pGlyphData[UGL_BMF_GLYPH_ASCENT_INDEX];
    pCacheElement->bitmapRect.left   = 0;
    pCacheElement->bitmapRect.right  = (UGL_POS) pCacheElement->width - 1;
    pCacheElement->bitmapRect.top    = 0;
    pCacheElement->bitmapRect.bottom = (UGL_POS) pCacheElement->height - 1;

    /* Setup monochrome bitmap struct */
    mDib.width  = (UGL_SIZE) pCacheElement->width;
    mDib.height = (UGL_SIZE) pCacheElement->height;
    mDib.stride = mDib.width;
    mDib.pData  = &pGlyphData[UGL_BMF_GLYPH_BMP_INDEX];

    /* Create device bitmap */
    i = 0;
    if (pCacheElement->width > 0 && pCacheElement->height > 0) {
        pCacheElement->bitmapId = (*pDrv->monoBitmapCreate) (
            pDrv, &mDib, UGL_DIB_INIT_DATA, 0, uglBMFGlyphCachePoolId);

        while (pCacheElement->bitmapId == UGL_NULL) {
            if (uglBMFGlyphCacheFree (drvId,
                  pBMFDrv->pLastCacheElement->ppPageElement) != UGL_STATUS_OK) {
                uglOSMemFree (pCacheElement);
                return (UGL_NULL);
            }

            pCacheElement->bitmapId = (*pDrv->monoBitmapCreate) (
                pDrv, &mDib, UGL_DIB_INIT_DATA, 0, uglBMFGlyphCachePoolId);

            if (pCacheElement->bitmapId == UGL_NULL) {
                uglOSTaskDelay (UGL_BMF_RETRY_SLEEP_TIME);
                if (++i == UGL_BMF_RETRY_TIMES) {
                    uglOSMemFree (pCacheElement);
                    return (UGL_NULL);
                }
            }
        }
    }

    /* Add cache element to list */
    pCacheElement->pNext = pBMFDrv->pFirstCacheElement;
    if (pBMFDrv->pFirstCacheElement != UGL_NULL) {
        pBMFDrv->pFirstCacheElement->pPrev = pCacheElement;
    }
    pBMFDrv->pFirstCacheElement = pCacheElement;
    if (pBMFDrv->pLastCacheElement == UGL_NULL) {
        pBMFDrv->pLastCacheElement = pCacheElement;
    }

    *ppPageElement = pCacheElement;
    pBMFDrv->numCachedGlyphs++;

    return (pCacheElement);
}

/******************************************************************************
 *
 * uglBMFGlyphCacheFree - Free cache for glyphs
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFGlyphCacheFree (
    UGL_FONT_DRIVER_ID  drvId,
    void **             ppPageElement
    ) {
    UGL_BMF_FONT_DRIVER *     pBMFDrv       = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_GLYPH_CACHE_ELEMENT * pCacheElement =
        (UGL_GLYPH_CACHE_ELEMENT *) *ppPageElement;
    UGL_UGI_DRIVER *          pDrv          = drvId->pDriver;

    if (pCacheElement == UGL_NULL || pBMFDrv->numCachedGlyphs < 1) {
        return (UGL_STATUS_ERROR);
    }

    /* Remove element from list */
    if (pCacheElement->pPrev != UGL_NULL) {
        pCacheElement->pPrev->pNext = pCacheElement->pNext;
    }
    else {
        pBMFDrv->pFirstCacheElement = pCacheElement->pNext;
    }
    if (pCacheElement->pNext != UGL_NULL) {
        pCacheElement->pNext->pPrev = pCacheElement->pPrev;
    }
    else {
        pBMFDrv->pLastCacheElement = pCacheElement->pPrev;
    }

    /* Destroy font bitmap */
    if (pCacheElement->bitmapId != UGL_NULL) {
        (*pDrv->monoBitmapDestroy) (pDrv, pCacheElement->bitmapId);
    }

    *ppPageElement = pCacheElement->pGlyphData;
    pBMFDrv->numCachedGlyphs--;
    uglOSMemFree (pCacheElement);

    return (UGL_STATUS_OK);
}

