/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/drwlib.c                                        */
/*  Changes: 1992, 1993, 1994, 2013  Thomas Mertes                  */
/*  Content: All primitive actions to do graphic output.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "drw_rtl.h"
#include "gkb_rtl.h"
#include "drw_drv.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "drwlib.h"



static objectType toIntArray (rtlArrayType aRtlArray)

  {
    memSizeType arraySize;
    memSizeType pos;
    arrayType anArray;
    objectType result;

  /* toIntArray */
    if (aRtlArray == NULL) {
      /* Assume that an exception was already raised */
      result = NULL;
    } else {
      arraySize = arraySize(aRtlArray);
      if (unlikely(!ALLOC_ARRAY(anArray, arraySize))) {
        FREE_RTL_ARRAY(aRtlArray, arraySize);
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        anArray->min_position = aRtlArray->min_position;
        anArray->max_position = aRtlArray->max_position;
        for (pos = 0; pos < arraySize; pos++) {
          anArray->arr[pos].type_of = take_type(SYS_INT_TYPE);
          anArray->arr[pos].descriptor.property = NULL;
          anArray->arr[pos].value.intValue = aRtlArray->arr[pos].value.intValue;
          INIT_CATEGORY_OF_VAR(&anArray->arr[pos], INTOBJECT);
        } /* for */
        FREE_RTL_ARRAY(aRtlArray, arraySize);
        result = bld_array_temp(anArray);
      } /* if */
    } /* if */
    return result;
  } /* toIntArray */



objectType gkb_busy_getc (listType arguments)

  { /* gkb_busy_getc */
    if (gkbKeyPressed()) {
      return bld_char_temp(gkbGetc());
    } else {
      return bld_char_temp((charType) K_NONE);
    } /* if */
  } /* gkb_busy_getc */



objectType gkb_button_pressed (listType arguments)

  { /* gkb_button_pressed */
    isit_char(arg_2(arguments));
    if (gkbButtonPressed(take_char(arg_2(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* gkb_button_pressed */



objectType gkb_button_xpos (listType arguments)

  { /* gkb_button_xpos */
    return bld_int_temp(gkbButtonXpos());
  } /* gkb_button_xpos */



objectType gkb_button_ypos (listType arguments)

  { /* gkb_button_ypos */
    return bld_int_temp(gkbButtonYpos());
  } /* gkb_button_ypos */



objectType gkb_getc (listType arguments)

  { /* gkb_getc */
    return bld_char_temp(gkbGetc());
  } /* gkb_getc */



objectType gkb_gets (listType arguments)

  { /* gkb_gets */
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        gkbGets(take_int(arg_2(arguments))));
  } /* gkb_gets */



objectType gkb_keypressed (listType arguments)

  { /* gkb_keypressed */
    if (gkbKeyPressed()) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* gkb_keypressed */



objectType gkb_line_read (listType arguments)

  {
    objectType ch_variable;

  /* gkb_line_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        gkbLineRead(&ch_variable->value.charValue));
  } /* gkb_line_read */



objectType gkb_raw_getc (listType arguments)

  { /* gkb_raw_getc */
    return bld_char_temp(gkbRawGetc());
  } /* gkb_raw_getc */



objectType gkb_window (listType arguments)

  { /* gkb_window */
    return bld_win_temp(gkbWindow());
  } /* gkb_window */



objectType gkb_word_read (listType arguments)

  {
    objectType ch_variable;

  /* gkb_word_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        gkbWordRead(&ch_variable->value.charValue));
  } /* gkb_word_read */



objectType drw_arc (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;

  /* drw_arc */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    drwArc(actual_window, x, y, radius, ang1, ang2);
    return SYS_EMPTY_OBJECT;
  } /* drw_arc */



objectType drw_arc2 (listType arguments)

  {
    winType actual_window;
    intType x1, y1, x2, y2, radius;

  /* drw_arc2 */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    x2 = take_int(arg_4(arguments));
    y2 = take_int(arg_5(arguments));
    radius = take_int(arg_6(arguments));
    drwArc2(actual_window, x1, y1, x2, y2, radius);
    return SYS_EMPTY_OBJECT;
  } /* drw_arc2 */



objectType drw_background (listType arguments)

  { /* drw_background */
    isit_int(arg_1(arguments));
    drwBackground(take_int(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_background */



objectType drw_circle (listType arguments)

  {
    winType actual_window;
    intType x1, y1, radius;

  /* drw_circle */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    drwCircle(actual_window, x1, y1, radius);
    return SYS_EMPTY_OBJECT;
  } /* drw_circle */



objectType drw_clear (listType arguments)

  { /* drw_clear */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    drwClear(take_win(arg_1(arguments)), take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_clear */



objectType drw_cmp (listType arguments)

  {
    memSizeType ref1;
    memSizeType ref2;
    intType signumValue;

  /* drw_cmp */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    ref1 = (memSizeType) take_win(arg_1(arguments));
    ref2 = (memSizeType) take_win(arg_2(arguments));
    if (ref1 < ref2) {
      signumValue = -1;
    } else {
      signumValue = ref1 > ref2;
    } /* if */
    return bld_int_temp(signumValue);
  } /* drw_cmp */



objectType drw_color (listType arguments)

  { /* drw_color */
    isit_int(arg_1(arguments));
    drwColor(take_int(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_color */



objectType drw_convPointList (listType arguments)

  { /* drw_convPointList */
    isit_bstri(arg_1(arguments));
    return toIntArray(drwConvPointList(take_bstri(arg_1(arguments))));
  } /* drw_convPointList */



objectType drw_copyarea (listType arguments)

  { /* drw_copyarea */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    drwCopyArea(take_win(arg_1(arguments)), take_win(arg_2(arguments)),
                take_int(arg_3(arguments)), take_int(arg_4(arguments)),
                take_int(arg_5(arguments)), take_int(arg_6(arguments)),
                take_int(arg_7(arguments)), take_int(arg_8(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_copyarea */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType drw_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    winType win_source;
    winType old_window;

  /* drw_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_win(dest);
    isit_win(source);
    is_variable(dest);
    win_source = take_win(source);
    if (TEMP_OBJECT(source)) {
      source->value.winValue = NULL;
    } else {
      if (win_source != NULL && win_source->usage_count != 0) {
        win_source->usage_count++;
      } /* if */
    } /* if */
    old_window = take_win(dest);
    if (old_window != NULL && old_window->usage_count != 0) {
      old_window->usage_count--;
      if (old_window->usage_count == 0) {
        drwFree(old_window);
      } /* if */
    } /* if */
    dest->value.winValue = win_source;
    return SYS_EMPTY_OBJECT;
  } /* drw_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType drw_create (listType arguments)

  {
    objectType source;
    winType win_value;

  /* drw_create */
    source = arg_3(arguments);
    isit_win(source);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), WINOBJECT);
    win_value = take_win(source);
    arg_1(arguments)->value.winValue = win_value;
    if (TEMP_OBJECT(source)) {
      source->value.winValue = NULL;
    } else {
      if (win_value != NULL && win_value->usage_count != 0) {
        win_value->usage_count++;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* drw_create */



objectType drw_destr (listType arguments)

  {
    winType old_win;

  /* drw_destr */
    isit_win(arg_1(arguments));
    old_win = take_win(arg_1(arguments));
    if (old_win != NULL && old_win->usage_count != 0) {
      old_win->usage_count--;
      if (old_win->usage_count == 0) {
        drwFree(old_win);
      } /* if */
      arg_1(arguments)->value.winValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* drw_destr */



objectType drw_empty (listType arguments)

  { /* drw_empty */
    return bld_win_temp(drwEmpty());
  } /* drw_empty */



objectType drw_eq (listType arguments)

  { /* drw_eq */
    isit_win(arg_1(arguments));
    isit_win(arg_3(arguments));
    if (take_win(arg_1(arguments)) ==
        take_win(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* drw_eq */



objectType drw_farcchord (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;

  /* drw_farcchord */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    drwFArcChord(actual_window, x, y, radius, ang1, ang2);
    return SYS_EMPTY_OBJECT;
  } /* drw_farcchord */



objectType drw_farcpieslice (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;

  /* drw_farcpieslice */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    drwFArcPieSlice(actual_window, x, y, radius, ang1, ang2);
    return SYS_EMPTY_OBJECT;
  } /* drw_farcpieslice */



objectType drw_fcircle (listType arguments)

  {
    winType actual_window;
    intType x1, y1, radius;

  /* drw_fcircle */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    drwFCircle(actual_window, x1, y1, radius);
    return SYS_EMPTY_OBJECT;
  } /* drw_fcircle */



objectType drw_fellipse (listType arguments)

  {
    winType actual_window;
    intType x1, y1;
    intType width, height;

  /* drw_fellipse */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    drwFEllipse(actual_window, x1, y1, width, height);
    return SYS_EMPTY_OBJECT;
  } /* drw_fellipse */



objectType drw_flush (listType arguments)

  { /* drw_flush */
    drwFlush();
    return SYS_EMPTY_OBJECT;
  } /* drw_flush */



objectType drw_fpolyLine (listType arguments)

  {
    winType actual_window;
    intType x, y, col;
    bstriType point_list;

  /* drw_fpolyLine */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_bstri(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    point_list = take_bstri(arg_4(arguments));
    col = take_int(arg_5(arguments));
    drwFPolyLine(actual_window, x, y, point_list, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_fpolyLine */



objectType drw_genPointList (listType arguments)

  {
    arrayType points_array;
    memSizeType len;
    rtlArrayType xyArray;
    objectType curr_number;
    memSizeType pos;
    bstriType result;

  /* drw_genPointList */
    isit_array(arg_1(arguments));
    points_array = take_array(arg_1(arguments));
    len = arraySize(points_array);
    if (len == 0 || len & 1) {
      logError(printf("drw_genPointList(arr1 (size=" FMT_U_MEM "): "
                      "Size zero or odd.",
                      len););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (!ALLOC_RTL_ARRAY(xyArray, len)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        xyArray->min_position = 1;
        xyArray->max_position = (intType) (len);
        curr_number = &points_array->arr[0];
        for (pos = 0; pos < len; pos++) {
          isit_int(curr_number);
          xyArray->arr[pos].value.intValue = take_int(curr_number);
          curr_number++;
        } /* for */
        result = drwGenPointList(xyArray);
        FREE_RTL_ARRAY(xyArray, len);
      } /* if */
    } /* if */
    return bld_bstri_temp(result);
  } /* drw_genPointList */



objectType drw_get (listType arguments)

  {
    winType actual_window;
    intType x1;
    intType y1;
    intType width;
    intType height;

  /* drw_get */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    return bld_win_temp(
        drwGet(actual_window, x1, y1, width, height));
  } /* drw_get */



objectType drw_getImage (listType arguments)

  {
    winType actual_window;

  /* drw_getImage */
    isit_win(arg_1(arguments));
    actual_window = take_win(arg_1(arguments));
    return bld_bstri_temp(
        drwGetImage(actual_window));
  } /* drw_getImage */



objectType drw_getImagePixel (listType arguments)

  {
    bstriType image;
    intType width;
    intType height;
    intType x;
    intType y;

  /* drw_getImagePixel */
    isit_bstri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    image = take_bstri(arg_1(arguments));
    width = take_int(arg_2(arguments));
    height = take_int(arg_3(arguments));
    x = take_int(arg_4(arguments));
    y = take_int(arg_5(arguments));
    return bld_int_temp(
        drwGetImagePixel(image, width, height, x, y));
  } /* drw_getImagePixel */



objectType drw_getPixel (listType arguments)

  {
    winType actual_window;
    intType x1;
    intType y1;

  /* drw_getPixel */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    return bld_int_temp(
        drwGetPixel(actual_window, x1, y1));
  } /* drw_getPixel */



objectType drw_hashcode (listType arguments)

  { /* drw_hashcode */
    isit_win(arg_1(arguments));
    return bld_int_temp((intType)
        (((memSizeType) take_win(arg_1(arguments))) >> 6));
  } /* drw_hashcode */



objectType drw_height (listType arguments)

  { /* drw_height */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwHeight(take_win(arg_1(arguments))));
  } /* drw_height */



objectType drw_image (listType arguments)

  {
    arrayType arr_image;
    objectType curr_line;
    arrayType arr_line;
    objectType curr_column;
    int32Type *pixel_elem;
    memSizeType height;
    memSizeType width;
    memSizeType line;
    memSizeType column;
    int32Type *image_data;
    winType result;

  /* drw_image */
    isit_array(arg_1(arguments));
    arr_image = take_array(arg_1(arguments));
    height = arraySize(arr_image);
    if (height == 0) {
      logError(printf("drw_image(arr1 (height=" FMT_U_MEM "): "
                      "Height zero.", height););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      curr_line = &arr_image->arr[0];
      isit_array(curr_line);
      arr_line = take_array(curr_line);
      width = arraySize(arr_line);
      if (width == 0) {
        logError(printf("drw_image(arr1 (height=" FMT_U_MEM "): "
                        "Width zero.", height););
        return raise_exception(SYS_RNG_EXCEPTION);
      } else {
        curr_column = &arr_line->arr[0];
        isit_int(curr_column);
        if (height > MAX_MEMSIZETYPE / sizeof(int32Type) / (memSizeType) width ||
            (image_data = (int32Type *) malloc((memSizeType) height * (memSizeType) width *
                                               sizeof(int32Type))) == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          pixel_elem = image_data;
          for (line = height; line > 0; line--, curr_line++) {
            arr_line = take_array(curr_line);
            curr_column = &arr_line->arr[0];
            for (column = width; column > 0; column--, curr_column++) {
              *pixel_elem = (int32Type) take_int(curr_column);
              pixel_elem++;
            } /* for */
          } /* for */
          result = drwImage(image_data, width, height);
          free(image_data);
        } /* if */
      } /* if */
    } /* if */
    return bld_win_temp(result);
  } /* drw_image */



objectType drw_line (listType arguments)

  {
    winType actual_window;
    intType x1, y1, x2, y2;

  /* drw_line */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    x2 = take_int(arg_4(arguments));
    y2 = take_int(arg_5(arguments));
    drwLine(actual_window, x1, y1, x2, y2);
    return SYS_EMPTY_OBJECT;
  } /* drw_line */



objectType drw_ne (listType arguments)

  { /* drw_ne */
    isit_win(arg_1(arguments));
    isit_win(arg_3(arguments));
    if (take_win(arg_1(arguments)) !=
        take_win(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* drw_ne */



objectType drw_new_pixmap (listType arguments)

  {
    intType width;
    intType height;

  /* drw_new_pixmap */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    width = take_int(arg_1(arguments));
    height = take_int(arg_2(arguments));
    return bld_win_temp(
        drwNewPixmap(width, height));
  } /* drw_new_pixmap */



objectType drw_open (listType arguments)

  {
    intType xPos, yPos, width, height;
    striType window_name;

  /* drw_open */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_stri(arg_5(arguments));
    xPos = take_int(arg_1(arguments));
    yPos = take_int(arg_2(arguments));
    width = take_int(arg_3(arguments));
    height = take_int(arg_4(arguments));
    window_name = take_stri(arg_5(arguments));
    return bld_win_temp(
        drwOpen(xPos, yPos, width, height, window_name));
  } /* drw_open */



objectType drw_open_sub_window (listType arguments)

  {
    winType parent_window;
    intType xPos, yPos, width, height;

  /* drw_open_sub_window */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    parent_window = take_win(arg_1(arguments));
    xPos = take_int(arg_2(arguments));
    yPos = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    return bld_win_temp(
        drwOpenSubWindow(parent_window, xPos, yPos, width, height));
  } /* drw_open_sub_window */



objectType drw_parc (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;
    intType col;

  /* drw_parc */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    isit_int(arg_7(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    col = take_int(arg_7(arguments));
    drwPArc(actual_window, x, y, radius, ang1, ang2, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_parc */



objectType drw_pcircle (listType arguments)

  {
    winType actual_window;
    intType x1, y1, radius, col;

  /* drw_pcircle */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    col = take_int(arg_5(arguments));
    drwPCircle(actual_window, x1, y1, radius, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pcircle */



objectType drw_pfarcchord (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;
    intType col;

  /* drw_pfarcchord */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    isit_int(arg_7(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    col = take_int(arg_7(arguments));
    drwPFArcChord(actual_window, x, y, radius, ang1, ang2, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pfarcchord */



objectType drw_pfarcpieslice (listType arguments)

  {
    winType actual_window;
    intType x, y, radius;
    floatType ang1, ang2;
    intType col;

  /* drw_pfarcpieslice */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_float(arg_5(arguments));
    isit_float(arg_6(arguments));
    isit_int(arg_7(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    ang1 = take_float(arg_5(arguments));
    ang2 = take_float(arg_6(arguments));
    col = take_int(arg_7(arguments));
    drwPFArcPieSlice(actual_window, x, y, radius, ang1, ang2, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pfarcpieslice */



objectType drw_pfcircle (listType arguments)

  {
    winType actual_window;
    intType x1, y1, radius, col;

  /* drw_pfcircle */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    radius = take_int(arg_4(arguments));
    col = take_int(arg_5(arguments));
    drwPFCircle(actual_window, x1, y1, radius, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pfcircle */



objectType drw_pfellipse (listType arguments)

  {
    winType actual_window;
    intType x1, y1;
    intType width, height, col;

  /* drw_pfellipse */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    col = take_int(arg_6(arguments));
    drwPFEllipse(actual_window, x1, y1, width, height, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pfellipse */



objectType drw_pixelToRgb (listType arguments)

  {
    objectType red_variable;
    objectType green_variable;
    objectType blue_variable;

  /* drw_pixelToRgb */
    isit_int(arg_1(arguments));
    red_variable = arg_2(arguments);
    isit_int(red_variable);
    is_variable(red_variable);
    green_variable = arg_3(arguments);
    isit_int(green_variable);
    is_variable(green_variable);
    blue_variable = arg_4(arguments);
    isit_int(blue_variable);
    is_variable(blue_variable);
    drwPixelToRgb(take_int(arg_1(arguments)),
                  &red_variable->value.intValue,
                  &green_variable->value.intValue,
                  &blue_variable->value.intValue);
    return SYS_EMPTY_OBJECT;
  } /* drw_pixelToRgb */



objectType drw_pline (listType arguments)

  {
    winType actual_window;
    intType x1, y1, x2, y2, col;

  /* drw_pline */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    x2 = take_int(arg_4(arguments));
    y2 = take_int(arg_5(arguments));
    col = take_int(arg_6(arguments));
    drwPLine(actual_window, x1, y1, x2, y2, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_pline */



objectType drw_point (listType arguments)

  {
    winType actual_window;
    intType x, y;

  /* drw_point */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    drwPoint(actual_window, x, y);
    return SYS_EMPTY_OBJECT;
  } /* drw_point */



objectType drw_pointer_xpos (listType arguments)

  { /* drw_pointer_xpos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwPointerXpos(take_win(arg_1(arguments))));
  } /* drw_pointer_xpos */



objectType drw_pointer_ypos (listType arguments)

  { /* drw_pointer_ypos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwPointerYpos(take_win(arg_1(arguments))));
  } /* drw_pointer_ypos */



objectType drw_polyLine (listType arguments)

  {
    winType actual_window;
    intType x, y, col;
    bstriType point_list;

  /* drw_polyLine */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_bstri(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    point_list = take_bstri(arg_4(arguments));
    col = take_int(arg_5(arguments));
    drwPolyLine(actual_window, x, y, point_list, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_polyLine */



objectType drw_ppoint (listType arguments)

  {
    winType actual_window;
    intType x, y, col;

  /* drw_ppoint */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    col = take_int(arg_4(arguments));
    drwPPoint(actual_window, x, y, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_ppoint */



objectType drw_prect (listType arguments)

  {
    winType actual_window;
    intType x1, y1, width, height, col;

  /* drw_prect */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    col = take_int(arg_6(arguments));
    drwPRect(actual_window, x1, y1, width, height, col);
    return SYS_EMPTY_OBJECT;
  } /* drw_prect */



objectType drw_put (listType arguments)

  {
    winType actual_window;
    winType pixmap;

  /* drw_put */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    actual_window = take_win(arg_1(arguments));
    pixmap = take_win(arg_2(arguments));
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    /* This is handled in the driver function drwPut.             */
    drwPut(actual_window, pixmap,
        take_int(arg_3(arguments)),
        take_int(arg_4(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_put */



objectType drw_rect (listType arguments)

  {
    winType actual_window;
    intType x1, y1, width, height;

  /* drw_rect */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    width = take_int(arg_4(arguments));
    height = take_int(arg_5(arguments));
    drwRect(actual_window, x1, y1, width, height);
    return SYS_EMPTY_OBJECT;
  } /* drw_rect */



objectType drw_rgbcol (listType arguments)

  { /* drw_rgbcol */
    return bld_int_temp(
        drwRgbColor(take_int(arg_1(arguments)), take_int(arg_2(arguments)),
                    take_int(arg_3(arguments))));
  } /* drw_rgbcol */



objectType drw_rot (listType arguments)

  { /* drw_rot */
    return SYS_EMPTY_OBJECT;
  } /* drw_rot */



objectType drw_scale (listType arguments)

  { /* drw_scale */
    return SYS_EMPTY_OBJECT;
  } /* drw_scale */



objectType drw_setContent (listType arguments)

  { /* drw_setContent */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    drwSetContent(take_win(arg_1(arguments)), take_win(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setContent */



objectType drw_setPos (listType arguments)

  { /* drw_setPos */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    drwSetPos(take_win(arg_1(arguments)),
        take_int(arg_2(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setPos */



objectType drw_setTransparentColor (listType arguments)

  { /* drw_setTransparentColor */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    drwSetTransparentColor(take_win(arg_1(arguments)),
        take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setTransparentColor */



objectType drw_text (listType arguments)

  {
    winType actual_window;
    intType x, y;
    striType stri;
    intType col, bkcol;

  /* drw_text */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    actual_window = take_win(arg_1(arguments));
    x = take_int(arg_2(arguments));
    y = take_int(arg_3(arguments));
    stri = take_stri(arg_4(arguments));
    col = take_int(arg_5(arguments));
    bkcol = take_int(arg_6(arguments));
    drwText(actual_window, x, y, stri, col, bkcol);
    return SYS_EMPTY_OBJECT;
  } /* drw_text */



objectType drw_toBottom (listType arguments)

  { /* drw_toBottom */
    isit_win(arg_1(arguments));
    drwToBottom(take_win(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_toBottom */



objectType drw_toTop (listType arguments)

  { /* drw_toTop */
    isit_win(arg_1(arguments));
    drwToTop(take_win(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_toTop */



objectType drw_value (listType arguments)

  {
    objectType obj_arg;
    winType win_value;

  /* drw_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 CATEGORY_OF_OBJ(obj_arg) != WINOBJECT)) {
      logError(printf("drw_value(");
               trace1(obj_arg);
               printf("): Category is not WINOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      win_value = take_win(obj_arg);
      if (win_value != NULL && win_value->usage_count != 0) {
        win_value->usage_count++;
      } /* if */
      return bld_win_temp(win_value);
    } /* if */
  } /* drw_value */



objectType drw_width (listType arguments)

  { /* drw_width */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwWidth(take_win(arg_1(arguments))));
  } /* drw_width */



objectType drw_xpos (listType arguments)

  { /* drw_xpos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwXPos(take_win(arg_1(arguments))));
  } /* drw_xpos */



objectType drw_ypos (listType arguments)

  { /* drw_ypos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwYPos(take_win(arg_1(arguments))));
  } /* drw_ypos */
