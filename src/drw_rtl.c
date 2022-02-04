/********************************************************************/
/*                                                                  */
/*  drw_rtl.c     Platform idependent drawing functions.            */
/*  Copyright (C) 1989 - 2013, 2015 - 2018  Thomas Mertes           */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/drw_rtl.c                                       */
/*  Changes: 2007, 2013, 2015 - 2018  Thomas Mertes                 */
/*  Content: Platform idependent drawing functions.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"
#include "drw_drv.h"

#undef EXTERN
#define EXTERN
#include "drw_rtl.h"


#define USE_DUFFS_UNROLLING 1



#if USE_DUFFS_UNROLLING
static void memcpy_to_pixel (register int32Type *const dest,
    register const intType *const src, memSizeType len)

  {
    register memSizeType pos;

  /* memcpy_to_pixel */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: dest[pos -  1] = (int32Type) src[pos -  1];
          case 31: dest[pos -  2] = (int32Type) src[pos -  2];
          case 30: dest[pos -  3] = (int32Type) src[pos -  3];
          case 29: dest[pos -  4] = (int32Type) src[pos -  4];
          case 28: dest[pos -  5] = (int32Type) src[pos -  5];
          case 27: dest[pos -  6] = (int32Type) src[pos -  6];
          case 26: dest[pos -  7] = (int32Type) src[pos -  7];
          case 25: dest[pos -  8] = (int32Type) src[pos -  8];
          case 24: dest[pos -  9] = (int32Type) src[pos -  9];
          case 23: dest[pos - 10] = (int32Type) src[pos - 10];
          case 22: dest[pos - 11] = (int32Type) src[pos - 11];
          case 21: dest[pos - 12] = (int32Type) src[pos - 12];
          case 20: dest[pos - 13] = (int32Type) src[pos - 13];
          case 19: dest[pos - 14] = (int32Type) src[pos - 14];
          case 18: dest[pos - 15] = (int32Type) src[pos - 15];
          case 17: dest[pos - 16] = (int32Type) src[pos - 16];
          case 16: dest[pos - 17] = (int32Type) src[pos - 17];
          case 15: dest[pos - 18] = (int32Type) src[pos - 18];
          case 14: dest[pos - 19] = (int32Type) src[pos - 19];
          case 13: dest[pos - 20] = (int32Type) src[pos - 20];
          case 12: dest[pos - 21] = (int32Type) src[pos - 21];
          case 11: dest[pos - 22] = (int32Type) src[pos - 22];
          case 10: dest[pos - 23] = (int32Type) src[pos - 23];
          case  9: dest[pos - 24] = (int32Type) src[pos - 24];
          case  8: dest[pos - 25] = (int32Type) src[pos - 25];
          case  7: dest[pos - 26] = (int32Type) src[pos - 26];
          case  6: dest[pos - 27] = (int32Type) src[pos - 27];
          case  5: dest[pos - 28] = (int32Type) src[pos - 28];
          case  4: dest[pos - 29] = (int32Type) src[pos - 29];
          case  3: dest[pos - 30] = (int32Type) src[pos - 30];
          case  2: dest[pos - 31] = (int32Type) src[pos - 31];
          case  1: dest[pos - 32] = (int32Type) src[pos - 32];
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
  } /* memcpy_to_pixel */



static void memcpy_from_pixel (register intType *const dest,
    register const int32Type *const src, memSizeType len)

  {
    register memSizeType pos;

  /* memcpy_from_pixel */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: dest[pos -  1] = (intType) src[pos -  1];
          case 31: dest[pos -  2] = (intType) src[pos -  2];
          case 30: dest[pos -  3] = (intType) src[pos -  3];
          case 29: dest[pos -  4] = (intType) src[pos -  4];
          case 28: dest[pos -  5] = (intType) src[pos -  5];
          case 27: dest[pos -  6] = (intType) src[pos -  6];
          case 26: dest[pos -  7] = (intType) src[pos -  7];
          case 25: dest[pos -  8] = (intType) src[pos -  8];
          case 24: dest[pos -  9] = (intType) src[pos -  9];
          case 23: dest[pos - 10] = (intType) src[pos - 10];
          case 22: dest[pos - 11] = (intType) src[pos - 11];
          case 21: dest[pos - 12] = (intType) src[pos - 12];
          case 20: dest[pos - 13] = (intType) src[pos - 13];
          case 19: dest[pos - 14] = (intType) src[pos - 14];
          case 18: dest[pos - 15] = (intType) src[pos - 15];
          case 17: dest[pos - 16] = (intType) src[pos - 16];
          case 16: dest[pos - 17] = (intType) src[pos - 17];
          case 15: dest[pos - 18] = (intType) src[pos - 18];
          case 14: dest[pos - 19] = (intType) src[pos - 19];
          case 13: dest[pos - 20] = (intType) src[pos - 20];
          case 12: dest[pos - 21] = (intType) src[pos - 21];
          case 11: dest[pos - 22] = (intType) src[pos - 22];
          case 10: dest[pos - 23] = (intType) src[pos - 23];
          case  9: dest[pos - 24] = (intType) src[pos - 24];
          case  8: dest[pos - 25] = (intType) src[pos - 25];
          case  7: dest[pos - 26] = (intType) src[pos - 26];
          case  6: dest[pos - 27] = (intType) src[pos - 27];
          case  5: dest[pos - 28] = (intType) src[pos - 28];
          case  4: dest[pos - 29] = (intType) src[pos - 29];
          case  3: dest[pos - 30] = (intType) src[pos - 30];
          case  2: dest[pos - 31] = (intType) src[pos - 31];
          case  1: dest[pos - 32] = (intType) src[pos - 32];
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
  } /* memcpy_from_pixel */

#else



static void memcpy_to_pixel (register int32Type *dest,
    register const intType *src, memSizeType len)

  { /* memcpy_to_pixel */
    for (; len > 0; len--, src++, dest++) {
      *dest = (int32Type) *src;
    } /* for */
  } /* memcpy_to_pixel */



static void memcpy_from_pixel (register intType *const dest,
    register const int32Type *const src, memSizeType len)

  { /* memcpy_from_pixel */
    for (; len > 0; len--, src++, dest++) {
      *dest = (intType) *src;
    } /* for */
  } /* memcpy_from_pixel */

#endif



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 */
void drwCpy (winType *const dest, const winType source)

  { /* drwCpy */
    logFunction(printf("drwCpy(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    if (*dest != NULL && (*dest)->usage_count != 0) {
      (*dest)->usage_count--;
      if ((*dest)->usage_count == 0) {
        drwFree(*dest);
      } /* if */
    } /* if */
    *dest = source;
    logFunction(printf("drwCpy(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* drwCpy */



/**
 *  Reinterpret the generic parameters as winType and call drwCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(winType).
 */
void drwCpyGeneric (genericType *const dest, const genericType source)

  { /* drwCpyGeneric */
    drwCpy(&((rtlObjectType *) dest)->value.winValue,
           ((const_rtlObjectType *) &source)->value.winValue);
  } /* drwCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
winType drwCreate (const winType source)

  { /* drwCreate */
    logFunction(printf("drwCreate(" FMT_U_MEM ") (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    logFunction(printf("drwCreate --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* drwCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(winType).
 */
genericType drwCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* drwCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.winValue =
        drwCreate(((const_rtlObjectType *) &from_value)->value.winValue);
    return result.value.genericValue;
  } /* drwCreateGeneric */



/**
 *  Maintain a usage count and free an unused window 'old_win'.
 *  After a window is freed 'old_win' refers to not existing memory.
 *  The memory where 'old_win' is stored can be freed after drwDestr.
 */
void drwDestr (const winType old_win)

  { /* drwDestr */
    logFunction(printf("drwDestr(" FMT_U_MEM ") (usage=" FMT_U ")\n",
                       (memSizeType) old_win,
                       old_win != NULL ? old_win->usage_count : (uintType) 0););
    if (old_win != NULL && old_win->usage_count != 0) {
      old_win->usage_count--;
      if (old_win->usage_count == 0) {
        drwFree(old_win);
      } /* if */
    } /* if */
  } /* drwDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(winType).
 */
void drwDestrGeneric (const genericType old_value)

  { /* drwDestrGeneric */
    drwDestr(((const_rtlObjectType *) &old_value)->value.winValue);
  } /* drwDestrGeneric */



intType drwGetImagePixel (const_bstriType image, intType width,
    intType height, intType x, intType y)

  {
    memSizeType idx;
    int32Type *image_data;
    intType pixel;

  /* drwGetImagePixel */
    logFunction(printf("drwGetImagePixel(" FMT_U_MEM ", " FMT_D ", "
                       FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) image, width, height, x, y););
    if (unlikely(width  < 0 || x < 0 || x >= width ||
                 height < 0 || y < 0 || y >= height ||
                 (uintType) height >
                 MAX_MEMSIZETYPE / sizeof(int32Type) / (uintType) width)) {
      raise_error(RANGE_ERROR);
      pixel = 0;
    } else {
      idx = (memSizeType) y * (memSizeType) width + (memSizeType) x;
      if (unlikely(idx >= image->size / sizeof(int32Type))) {
        raise_error(RANGE_ERROR);
        pixel = 0;
      } else {
        image_data = (int32Type *) image->mem;
        pixel = image_data[idx];
      } /* if */
    } /* if */
    return pixel;
  } /* drwGetImagePixel */



rtlArrayType drwGetPixelArray (const_winType sourceWindow)

  {
    memSizeType height;
    memSizeType width;
    bstriType imageData;
    memSizeType yPos;
    rtlArrayType imageLine;
    int32Type *pixelArray;
    rtlArrayType imageArray;

  /* drwGetPixelArray */
    height = (memSizeType) drwHeight(sourceWindow);
    width = (memSizeType) drwWidth(sourceWindow);
    imageData = drwGetImage(sourceWindow);
    if (unlikely(!ALLOC_RTL_ARRAY(imageArray, height))) {
      raise_error(MEMORY_ERROR);
    } else {
      imageArray->min_position = 1;
      imageArray->max_position = (intType) height;
      pixelArray = (int32Type *) imageData->mem;
      for (yPos = 0; yPos < height; yPos++) {
	if (likely(ALLOC_RTL_ARRAY(imageLine, width))) {
          imageLine->min_position = 1;
          imageLine->max_position = (intType) width;
          imageArray->arr[yPos].value.arrayValue = imageLine;
          memcpy_from_pixel((intType *) imageLine->arr, pixelArray, width);
          pixelArray += width;
        } else {
          while (yPos >= 1) {
            yPos--;
            FREE_RTL_ARRAY(imageArray->arr[yPos].value.arrayValue, width);
          } /* while */
          FREE_RTL_ARRAY(imageArray, height);
          raise_error(MEMORY_ERROR);
          imageArray = NULL;
          yPos = height; /* leave for-loop */
        } /* if */
      } /* for */
    } /* if */
    return imageArray;
  } /* drwGetPixelArray */



winType drwGetPixmapFromPixels (const const_rtlArrayType image)

  {
    const_rtlObjectType *curr_line;
    rtlArrayType arr_line;
    int32Type *pixel_elem;
    memSizeType height;
    memSizeType width;
    memSizeType line;
    int32Type *image_data;
    winType result;

  /* drwGetPixmapFromPixels */
    height = arraySize(image);
    /* printf("drwGetPixmapFromPixels: height=" FMT_U_MEM "\n", height); */
    if (height == 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      curr_line = &image->arr[0];
      arr_line = curr_line->value.arrayValue;
      width = arraySize(arr_line);
      /* printf("drwGetPixmapFromPixels: width=" FMT_U_MEM "\n", width); */
      if (width == 0) {
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        if (height > MAX_MEMSIZETYPE / sizeof(int32Type) / (memSizeType) width ||
            (image_data = (int32Type *) malloc((memSizeType) height * (memSizeType) width *
                                               sizeof(int32Type))) == NULL) {
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          pixel_elem = image_data;
          for (line = height; line > 0; line--, curr_line++) {
            arr_line = curr_line->value.arrayValue;
            memcpy_to_pixel(pixel_elem, (intType *) arr_line->arr, width);
            pixel_elem += width;
          } /* for */
          result = drwImage(image_data, width, height);
          free(image_data);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGetPixmapFromPixels */
