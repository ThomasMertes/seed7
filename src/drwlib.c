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



objecttype gkb_busy_getc (listtype arguments)

  { /* gkb_busy_getc */
    if (gkbKeyPressed()) {
      return bld_char_temp(gkbGetc());
    } else {
      return bld_char_temp((chartype) K_NONE);
    } /* if */
  } /* gkb_busy_getc */



objecttype gkb_button_pressed (listtype arguments)

  { /* gkb_button_pressed */
    isit_char(arg_2(arguments));
    if (gkbButtonPressed(take_char(arg_2(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* gkb_button_pressed */



objecttype gkb_button_xpos (listtype arguments)

  { /* gkb_button_xpos */
    return bld_int_temp(gkbButtonXpos());
  } /* gkb_button_xpos */



objecttype gkb_button_ypos (listtype arguments)

  { /* gkb_button_ypos */
    return bld_int_temp(gkbButtonYpos());
  } /* gkb_button_ypos */



objecttype gkb_getc (listtype arguments)

  { /* gkb_getc */
    return bld_char_temp(gkbGetc());
  } /* gkb_getc */



objecttype gkb_gets (listtype arguments)

  { /* gkb_gets */
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        gkbGets(take_int(arg_2(arguments))));
  } /* gkb_gets */



objecttype gkb_keypressed (listtype arguments)

  { /* gkb_keypressed */
    if (gkbKeyPressed()) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* gkb_keypressed */



objecttype gkb_line_read (listtype arguments)

  {
    objecttype ch_variable;

  /* gkb_line_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        gkbLineRead(&ch_variable->value.charvalue));
  } /* gkb_line_read */



objecttype gkb_raw_getc (listtype arguments)

  { /* gkb_raw_getc */
    return bld_char_temp(gkbRawGetc());
  } /* gkb_raw_getc */



objecttype gkb_window (listtype arguments)

  { /* gkb_window */
    return bld_win_temp(gkbWindow());
  } /* gkb_window */



objecttype gkb_word_read (listtype arguments)

  {
    objecttype ch_variable;

  /* gkb_word_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        gkbWordRead(&ch_variable->value.charvalue));
  } /* gkb_word_read */



objecttype drw_arc (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;

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



objecttype drw_arc2 (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, x2, y2, radius;

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



objecttype drw_background (listtype arguments)

  { /* drw_background */
    isit_int(arg_1(arguments));
    drwBackground(take_int(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_background */



objecttype drw_circle (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, radius;

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



objecttype drw_clear (listtype arguments)

  { /* drw_clear */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    drwClear(take_win(arg_1(arguments)), take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_clear */



objecttype drw_cmp (listtype arguments)

  {
    memsizetype ref1;
    memsizetype ref2;
    inttype result;

  /* drw_cmp */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    ref1 = (memsizetype) take_win(arg_1(arguments));
    ref2 = (memsizetype) take_win(arg_2(arguments));
    if (ref1 < ref2) {
      result = -1;
    } else if (ref1 > ref2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* drw_cmp */



objecttype drw_color (listtype arguments)

  { /* drw_color */
    isit_int(arg_1(arguments));
    drwColor(take_int(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_color */



objecttype drw_copyarea (listtype arguments)

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



objecttype drw_cpy (listtype arguments)

  {
    objecttype win_to;
    objecttype win_from;
    wintype win_source;
    wintype old_window;

  /* drw_cpy */
    win_to = arg_1(arguments);
    win_from = arg_3(arguments);
    isit_win(win_to);
    isit_win(win_from);
    is_variable(win_to);
    win_source = take_win(win_from);
    if (TEMP_OBJECT(win_from)) {
      win_from->value.winvalue = NULL;
    } else {
      if (win_source != NULL) {
        win_source->usage_count++;
      } /* if */
    } /* if */
    old_window = take_win(win_to);
    if (old_window != NULL) {
      old_window->usage_count--;
      if (old_window->usage_count == 0) {
        drwFree(old_window);
      } /* if */
    } /* if */
    win_to->value.winvalue = win_source;
    return SYS_EMPTY_OBJECT;
  } /* drw_cpy */



objecttype drw_create (listtype arguments)

  {
    objecttype win_from;
    wintype win_value;

  /* drw_create */
    win_from = arg_3(arguments);
    isit_win(win_from);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), WINOBJECT);
    win_value = take_win(win_from);
    arg_1(arguments)->value.winvalue = win_value;
    if (TEMP_OBJECT(win_from)) {
      win_from->value.winvalue = NULL;
    } else {
      if (win_value != NULL) {
        win_value->usage_count++;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* drw_create */



objecttype drw_destr (listtype arguments)

  {
    wintype old_win;

  /* drw_destr */
    isit_win(arg_1(arguments));
    old_win = take_win(arg_1(arguments));
    if (old_win != NULL) {
      old_win->usage_count--;
      if (old_win->usage_count == 0) {
        drwFree(old_win);
      } /* if */
      arg_1(arguments)->value.winvalue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* drw_destr */



objecttype drw_empty (listtype arguments)

  { /* drw_empty */
    return bld_win_temp(NULL);
  } /* drw_empty */



objecttype drw_eq (listtype arguments)

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



objecttype drw_farcchord (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;

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



objecttype drw_farcpieslice (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;

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



objecttype drw_fcircle (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, radius;

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



objecttype drw_fellipse (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1;
    inttype width, height;

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



objecttype drw_flush (listtype arguments)

  { /* drw_flush */
    drwFlush();
    return SYS_EMPTY_OBJECT;
  } /* drw_flush */



objecttype drw_fpolyLine (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, col;
    bstritype point_list;

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



objecttype drw_genPointList (listtype arguments)

  {
    arraytype points_array;
    memsizetype len;
    rtlArraytype xyArray;
    objecttype curr_number;
    memsizetype pos;
    bstritype result;

  /* drw_genPointList */
    isit_array(arg_1(arguments));
    points_array = take_array(arg_1(arguments));
    len = arraySize(points_array);
    if (len == 0 || len & 1) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (!ALLOC_RTL_ARRAY(xyArray, len)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        xyArray->min_position = 1;
        xyArray->max_position = (inttype) (len);
        curr_number = &points_array->arr[0];
        for (pos = 0; pos < len; pos++) {
          isit_int(curr_number);
          xyArray->arr[pos].value.intvalue = take_int(curr_number);
          curr_number++;
        } /* for */
        result = drwGenPointList(xyArray);
        FREE_RTL_ARRAY(xyArray, len);
      } /* if */
    } /* if */
    return bld_bstri_temp(result);
  } /* drw_genPointList */



objecttype drw_get (listtype arguments)

  {
    wintype actual_window;
    inttype x1;
    inttype y1;
    inttype width;
    inttype height;

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



objecttype drw_getPixel (listtype arguments)

  {
    wintype actual_window;
    inttype x1;
    inttype y1;

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



objecttype drw_hashcode (listtype arguments)

  { /* drw_hashcode */
    isit_win(arg_1(arguments));
    return bld_int_temp((inttype)
        (((memsizetype) take_win(arg_1(arguments))) >> 6));
  } /* drw_hashcode */



objecttype drw_height (listtype arguments)

  { /* drw_height */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwHeight(take_win(arg_1(arguments))));
  } /* drw_height */



objecttype drw_image (listtype arguments)

  {
    arraytype arr_image;
    objecttype curr_line;
    arraytype arr_line;
    objecttype curr_column;
    int32type *pixel_elem;
    memsizetype height;
    memsizetype width;
    memsizetype line;
    memsizetype column;
    int32type *image_data;
    wintype result;

  /* drw_image */
    isit_array(arg_1(arguments));
    arr_image = take_array(arg_1(arguments));
    height = arraySize(arr_image);
    if (height == 0) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      curr_line = &arr_image->arr[0];
      isit_array(curr_line);
      arr_line = take_array(curr_line);
      width = arraySize(arr_line);
      if (width == 0) {
        return raise_exception(SYS_RNG_EXCEPTION);
      } else {
        curr_column = &arr_line->arr[0];
        isit_int(curr_column);
        if (height > MAX_MEMSIZETYPE / sizeof(int32type) / (memsizetype) width ||
            (image_data = (int32type *) malloc((memsizetype) height * (memsizetype) width *
                                               sizeof(int32type))) == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          pixel_elem = image_data;
          for (line = height; line > 0; line--, curr_line++) {
            arr_line = take_array(curr_line);
            curr_column = &arr_line->arr[0];
            for (column = width; column > 0; column--, curr_column++) {
              *pixel_elem = (int32type) take_int(curr_column);
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



objecttype drw_line (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, x2, y2;

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



objecttype drw_ne (listtype arguments)

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



objecttype drw_new_pixmap (listtype arguments)

  {
    inttype width;
    inttype height;

  /* drw_new_pixmap */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    width = take_int(arg_1(arguments));
    height = take_int(arg_2(arguments));
    return bld_win_temp(
        drwNewPixmap(width, height));
  } /* drw_new_pixmap */



objecttype drw_open (listtype arguments)

  {
    inttype xPos, yPos, width, height;
    stritype window_name;

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



objecttype drw_open_sub_window (listtype arguments)

  {
    wintype parent_window;
    inttype xPos, yPos, width, height;

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



objecttype drw_parc (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;
    inttype col;

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



objecttype drw_pcircle (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, radius, col;

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



objecttype drw_pfarcchord (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;
    inttype col;

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



objecttype drw_pfarcpieslice (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, radius;
    floattype ang1, ang2;
    inttype col;

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



objecttype drw_pfcircle (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, radius, col;

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



objecttype drw_pfellipse (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1;
    inttype width, height, col;

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



objecttype drw_pixelToRgb (listtype arguments)

  {
    objecttype red_variable;
    objecttype green_variable;
    objecttype blue_variable;

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
                  &red_variable->value.intvalue,
                  &green_variable->value.intvalue,
                  &blue_variable->value.intvalue);
    return SYS_EMPTY_OBJECT;
  } /* drw_pixelToRgb */



objecttype drw_pline (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, x2, y2, col;

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



objecttype drw_point (listtype arguments)

  {
    wintype actual_window;
    inttype x, y;

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



objecttype drw_pointer_xpos (listtype arguments)

  { /* drw_pointer_xpos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwPointerXpos(take_win(arg_1(arguments))));
  } /* drw_pointer_xpos */



objecttype drw_pointer_ypos (listtype arguments)

  { /* drw_pointer_ypos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwPointerYpos(take_win(arg_1(arguments))));
  } /* drw_pointer_ypos */



objecttype drw_polyLine (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, col;
    bstritype point_list;

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



objecttype drw_ppoint (listtype arguments)

  {
    wintype actual_window;
    inttype x, y, col;

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



objecttype drw_prect (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, width, height, col;

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



objecttype drw_put (listtype arguments)

  {
    wintype actual_window;
    wintype pixmap;

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



objecttype drw_rect (listtype arguments)

  {
    wintype actual_window;
    inttype x1, y1, width, height;

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



objecttype drw_rgbcol (listtype arguments)

  { /* drw_rgbcol */
    return bld_int_temp(
        drwRgbColor(take_int(arg_1(arguments)), take_int(arg_2(arguments)),
                    take_int(arg_3(arguments))));
  } /* drw_rgbcol */



objecttype drw_rot (listtype arguments)

  { /* drw_rot */
    return SYS_EMPTY_OBJECT;
  } /* drw_rot */



objecttype drw_scale (listtype arguments)

  { /* drw_scale */
    return SYS_EMPTY_OBJECT;
  } /* drw_scale */



objecttype drw_setContent (listtype arguments)

  { /* drw_setContent */
    isit_win(arg_1(arguments));
    isit_win(arg_2(arguments));
    drwSetContent(take_win(arg_1(arguments)), take_win(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setContent */



objecttype drw_setPos (listtype arguments)

  { /* drw_setPos */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    drwSetPos(take_win(arg_1(arguments)),
        take_int(arg_2(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setPos */



objecttype drw_setTransparentColor (listtype arguments)

  { /* drw_setTransparentColor */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    drwSetTransparentColor(take_win(arg_1(arguments)),
        take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_setTransparentColor */



objecttype drw_text (listtype arguments)

  {
    wintype actual_window;
    inttype x, y;
    stritype stri;
    inttype col, bkcol;

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



objecttype drw_toBottom (listtype arguments)

  { /* drw_toBottom */
    isit_win(arg_1(arguments));
    drwToBottom(take_win(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_toBottom */



objecttype drw_toTop (listtype arguments)

  { /* drw_toTop */
    isit_win(arg_1(arguments));
    drwToTop(take_win(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* drw_toTop */



objecttype drw_value (listtype arguments)

  {
    objecttype obj_arg;
    wintype win_value;

  /* drw_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != WINOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      win_value = take_win(obj_arg);
      if (win_value != NULL) {
        win_value->usage_count++;
      } /* if */
      return bld_win_temp(win_value);
    } /* if */
  } /* drw_value */



objecttype drw_width (listtype arguments)

  { /* drw_width */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwWidth(take_win(arg_1(arguments))));
  } /* drw_width */



objecttype drw_xpos (listtype arguments)

  { /* drw_xpos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwXPos(take_win(arg_1(arguments))));
  } /* drw_xpos */



objecttype drw_ypos (listtype arguments)

  { /* drw_ypos */
    isit_win(arg_1(arguments));
    return bld_int_temp(
        drwYPos(take_win(arg_1(arguments))));
  } /* drw_ypos */
