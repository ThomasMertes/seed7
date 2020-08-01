/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do graphic output.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "option.h"
#include "drw_rtl.h"
#include "kbd_rtl.h"
#include "drw_drv.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "drwlib.h"



#ifdef ANSI_C

objecttype gkb_busy_getc (listtype arguments)
#else

objecttype gkb_busy_getc (arguments)
listtype arguments;
#endif

  { /* gkb_busy_getc */
    if (gkbKeyPressed()) {
      return(bld_char_temp(gkbGetc()));
    } else {
      return(bld_char_temp((chartype) K_NONE));
    } /* if */
  } /* gkb_busy_getc */



#ifdef ANSI_C

objecttype gkb_getc (listtype arguments)
#else

objecttype gkb_getc (arguments)
listtype arguments;
#endif

  { /* gkb_getc */
    return(bld_char_temp(gkbGetc()));
  } /* gkb_getc */



#ifdef ANSI_C

objecttype gkb_gets (listtype arguments)
#else

objecttype gkb_gets (arguments)
listtype arguments;
#endif

  { /* gkb_gets */
    isit_int(arg_2(arguments));
    return(bld_stri_temp(
        gkbGets(take_int(arg_2(arguments)))));
  } /* gkb_gets */



#ifdef ANSI_C

objecttype gkb_keypressed (listtype arguments)
#else

objecttype gkb_keypressed (arguments)
listtype arguments;
#endif

  { /* gkb_keypressed */
    if (gkbKeyPressed()) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* gkb_keypressed */



#ifdef ANSI_C

objecttype gkb_line_read (listtype arguments)
#else

objecttype gkb_line_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* gkb_line_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        gkbLineRead(&ch_variable->value.charvalue)));
  } /* gkb_line_read */



#ifdef ANSI_C

objecttype gkb_raw_getc (listtype arguments)
#else

objecttype gkb_raw_getc (arguments)
listtype arguments;
#endif

  { /* gkb_raw_getc */
    return(bld_char_temp(gkbRawGetc()));
  } /* gkb_raw_getc */



#ifdef ANSI_C

objecttype gkb_word_read (listtype arguments)
#else

objecttype gkb_word_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* gkb_word_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        gkbWordRead(&ch_variable->value.charvalue)));
  } /* gkb_word_read */



#ifdef ANSI_C

objecttype gkb_window (listtype arguments)
#else

objecttype gkb_window (arguments)
listtype arguments;
#endif

  { /* gkb_window */
    return(bld_win_temp(gkbWindow()));
  } /* gkb_window */



#ifdef ANSI_C

objecttype gkb_xpos (listtype arguments)
#else

objecttype gkb_xpos (arguments)
listtype arguments;
#endif

  { /* gkb_xpos */
    return(bld_int_temp(gkbXpos()));
  } /* gkb_xpos */



#ifdef ANSI_C

objecttype gkb_ypos (listtype arguments)
#else

objecttype gkb_ypos (arguments)
listtype arguments;
#endif

  { /* gkb_ypos */
    return(bld_int_temp(gkbYpos()));
  } /* gkb_ypos */



#ifdef ANSI_C

objecttype drw_arc (listtype arguments)
#else

objecttype drw_arc (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_arc */



#ifdef ANSI_C

objecttype drw_arc2 (listtype arguments)
#else

objecttype drw_arc2 (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_arc2 */



#ifdef ANSI_C

objecttype drw_background (listtype arguments)
#else

objecttype drw_background (arguments)
listtype arguments;
#endif

  { /* drw_background */
    isit_int(arg_1(arguments));
    drwBackground(take_int(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* drw_background */



#ifdef ANSI_C

objecttype drw_circle (listtype arguments)
#else

objecttype drw_circle (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_circle */



#ifdef ANSI_C

objecttype drw_clear (listtype arguments)
#else

objecttype drw_clear (arguments)
listtype arguments;
#endif

  { /* drw_clear */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    drwClear(take_win(arg_1(arguments)), take_int(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* drw_clear */



#ifdef ANSI_C

objecttype drw_color (listtype arguments)
#else

objecttype drw_color (arguments)
listtype arguments;
#endif

  { /* drw_color */
    isit_int(arg_1(arguments));
    drwColor(take_int(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* drw_color */



#ifdef ANSI_C

objecttype drw_copyarea (listtype arguments)
#else

objecttype drw_copyarea (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_copyarea */



#ifdef ANSI_C

objecttype drw_cpy (listtype arguments)
#else

objecttype drw_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype win_to;
    objecttype win_from;
    wintype win_value;

  /* drw_cpy */
    win_to = arg_1(arguments);
    win_from = arg_3(arguments);
    isit_win(win_to);
    isit_win(win_from);
    is_variable(win_to);
    win_value = take_win(win_to);
    if (win_value != NULL) {
      win_value->usage_count--;
      if (win_value->usage_count == 0) {
        drwFree(win_value);
      } /* if */
    } /* if */
    win_value = take_win(win_from);
    win_to->value.winvalue = win_value;
    if (TEMP_OBJECT(win_from)) {
      win_from->value.winvalue = NULL;
    } else {
      if (win_value != NULL) {
        win_value->usage_count++;
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* drw_cpy */



#ifdef ANSI_C

objecttype drw_create (listtype arguments)
#else

objecttype drw_create (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_create */



#ifdef ANSI_C

objecttype drw_destr (listtype arguments)
#else

objecttype drw_destr (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_destr */



#ifdef ANSI_C

objecttype drw_empty (listtype arguments)
#else

objecttype drw_empty (arguments)
listtype arguments;
#endif

  { /* drw_empty */
    return(bld_win_temp(NULL));
  } /* drw_empty */



#ifdef ANSI_C

objecttype drw_eq (listtype arguments)
#else

objecttype drw_eq (arguments)
listtype arguments;
#endif

  { /* drw_eq */
    isit_win(arg_1(arguments));
    isit_win(arg_3(arguments));
    if (take_win(arg_1(arguments)) ==
        take_win(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* drw_eq */



#ifdef ANSI_C

objecttype drw_farcchord (listtype arguments)
#else

objecttype drw_farcchord (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_farcchord */



#ifdef ANSI_C

objecttype drw_farcpieslice (listtype arguments)
#else

objecttype drw_farcpieslice (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_farcpieslice */



#ifdef ANSI_C

objecttype drw_fcircle (listtype arguments)
#else

objecttype drw_fcircle (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_fcircle */



#ifdef ANSI_C

objecttype drw_fellipse (listtype arguments)
#else

objecttype drw_fellipse (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_fellipse */



#ifdef ANSI_C

objecttype drw_flush (listtype arguments)
#else

objecttype drw_flush (arguments)
listtype arguments;
#endif

  { /* drw_flush */
    drwFlush();
    return(SYS_EMPTY_OBJECT);
  } /* drw_flush */



#ifdef ANSI_C

objecttype drw_get (listtype arguments)
#else

objecttype drw_get (arguments)
listtype arguments;
#endif

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
    return(bld_win_temp(
        drwGet(actual_window, x1, y1, width, height)));
  } /* drw_get */



#ifdef ANSI_C

objecttype drw_height (listtype arguments)
#else

objecttype drw_height (arguments)
listtype arguments;
#endif

  { /* drw_height */
    isit_win(arg_1(arguments));
    return(bld_int_temp(drwHeight(take_win(arg_1(arguments)))));
  } /* drw_height */



#ifdef ANSI_C

objecttype drw_image (listtype arguments)
#else

objecttype drw_image (arguments)
listtype arguments;
#endif

  {
    wintype actual_window;
    arraytype arr_image;
    objecttype curr_line;
    arraytype arr_line;
    objecttype curr_column;
    inttype *pixel_elem;
    inttype width;
    inttype height;
    inttype line;
    inttype column;
    inttype *image_data;
    wintype result;

  /* drw_image */
    isit_win(arg_1(arguments));
    isit_array(arg_2(arguments));
    actual_window = take_win(arg_1(arguments));
    arr_image = take_array(arg_2(arguments));
    height = arr_image->max_position - arr_image->min_position + 1;
    if (height > 0) {
      curr_line = &arr_image->arr[0];
      isit_array(curr_line);
      arr_line = take_array(curr_line);
      width = arr_line->max_position - arr_line->min_position + 1;
      if (width > 0) {
        curr_column = &arr_line->arr[0];
        isit_int(curr_column);
        image_data = malloc(height * width * sizeof(inttype));
        pixel_elem = image_data;
        for (line = height; line > 0; line--, curr_line++) {
          arr_line = take_array(curr_line);
          curr_column = &arr_line->arr[0];
          for (column = width; column > 0; column--, curr_column++) {
            *pixel_elem = take_int(curr_column);
            pixel_elem++;
          } /* for */
        } /* for */
        result = drwImage(actual_window, image_data, width, height);
        free(image_data);
      } else {
        result = NULL;
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return(bld_win_temp(result));
  } /* drw_image */



#ifdef ANSI_C

objecttype drw_line (listtype arguments)
#else

objecttype drw_line (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_line */



#ifdef ANSI_C

objecttype drw_ne (listtype arguments)
#else

objecttype drw_ne (arguments)
listtype arguments;
#endif

  { /* drw_ne */
    isit_win(arg_1(arguments));
    isit_win(arg_3(arguments));
    if (take_win(arg_1(arguments)) !=
        take_win(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* drw_ne */



#ifdef ANSI_C

objecttype drw_new_pixmap (listtype arguments)
#else

objecttype drw_new_pixmap (arguments)
listtype arguments;
#endif

  {
    wintype actual_window;
    inttype width;
    inttype height;

  /* drw_new_pixmap */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    actual_window = take_win(arg_1(arguments));
    width = take_int(arg_2(arguments));
    height = take_int(arg_3(arguments));
    return(bld_win_temp(drwNewPixmap(actual_window, width, height)));
  } /* drw_new_pixmap */



#ifdef ANSI_C

objecttype drw_open (listtype arguments)
#else

objecttype drw_open (arguments)
listtype arguments;
#endif

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
    return(bld_win_temp(drwOpen(xPos, yPos, width, height, window_name)));
  } /* drw_open */



#ifdef ANSI_C

objecttype drw_parc (listtype arguments)
#else

objecttype drw_parc (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_parc */



#ifdef ANSI_C

objecttype drw_pcircle (listtype arguments)
#else

objecttype drw_pcircle (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pcircle */



#ifdef ANSI_C

objecttype drw_pfarcchord (listtype arguments)
#else

objecttype drw_pfarcchord (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pfarcchord */



#ifdef ANSI_C

objecttype drw_pfarcpieslice (listtype arguments)
#else

objecttype drw_pfarcpieslice (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pfarcpieslice */



#ifdef ANSI_C

objecttype drw_pfcircle (listtype arguments)
#else

objecttype drw_pfcircle (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pfcircle */



#ifdef ANSI_C

objecttype drw_pfellipse (listtype arguments)
#else

objecttype drw_pfellipse (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pfellipse */



#ifdef ANSI_C

objecttype drw_pline (listtype arguments)
#else

objecttype drw_pline (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_pline */



#ifdef ANSI_C

objecttype drw_point (listtype arguments)
#else

objecttype drw_point (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_point */



#ifdef ANSI_C

objecttype drw_ppoint (listtype arguments)
#else

objecttype drw_ppoint (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_ppoint */



#ifdef ANSI_C

objecttype drw_prect (listtype arguments)
#else

objecttype drw_prect (arguments)
listtype arguments;
#endif

  {
    wintype actual_window;
    inttype x1, y1, length_x, length_y, col;

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
    length_x = take_int(arg_4(arguments));
    length_y = take_int(arg_5(arguments));
    col = take_int(arg_6(arguments));
    drwPRect(actual_window, x1, y1, length_x, length_y, col);
    return(SYS_EMPTY_OBJECT);
  } /* drw_prect */



#ifdef ANSI_C

objecttype drw_put (listtype arguments)
#else

objecttype drw_put (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_put */



#ifdef ANSI_C

objecttype drw_rect (listtype arguments)
#else

objecttype drw_rect (arguments)
listtype arguments;
#endif

  {
    wintype actual_window;
    inttype x1, y1, length_x, length_y;

  /* drw_rect */
    isit_win(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    actual_window = take_win(arg_1(arguments));
    x1 = take_int(arg_2(arguments));
    y1 = take_int(arg_3(arguments));
    length_x = take_int(arg_4(arguments));
    length_y = take_int(arg_5(arguments));
    drwRect(actual_window, x1, y1, length_x, length_y);
    return(SYS_EMPTY_OBJECT);
  } /* drw_rect */



#ifdef ANSI_C

objecttype drw_rgbcol (listtype arguments)
#else

objecttype drw_rgbcol (arguments)
listtype arguments;
#endif

  { /* drw_rgbcol */
    return(bld_int_temp(drwRgbColor(take_int(arg_1(arguments)),
        take_int(arg_2(arguments)), take_int(arg_3(arguments)))));
  } /* drw_rgbcol */



#ifdef ANSI_C

objecttype drw_rot (listtype arguments)
#else

objecttype drw_rot (arguments)
listtype arguments;
#endif

  { /* drw_rot */
    return(SYS_EMPTY_OBJECT);
  } /* drw_rot */



#ifdef ANSI_C

objecttype drw_scale (listtype arguments)
#else

objecttype drw_scale (arguments)
listtype arguments;
#endif

  { /* drw_scale */
    return(SYS_EMPTY_OBJECT);
  } /* drw_scale */



#ifdef ANSI_C

objecttype drw_text (listtype arguments)
#else

objecttype drw_text (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* drw_text */



#ifdef ANSI_C

objecttype drw_width (listtype arguments)
#else

objecttype drw_width (arguments)
listtype arguments;
#endif

  { /* drw_width */
    isit_win(arg_1(arguments));
    return(bld_int_temp(drwWidth(take_win(arg_1(arguments)))));
  } /* drw_width */
