/********************************************************************/
/*                                                                  */
/*  drw_emc.c     Graphic access using browser capabilities.        */
/*  Copyright (C) 2020, 2021  Thomas Mertes                         */
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
/*  File: seed7/src/drw_emc.c                                       */
/*  Changes: 2020, 2021  Thomas Mertes                              */
/*  Content: Graphic access using browser capabilities.             */
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
#include "striutl.h"
#include "rtl_err.h"
#include "emscripten.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"
#include "kbd_drv.h"


typedef struct {
    uintType usage_count;
    /* Up to here the structure is identical to struct winStruct */
    int window;
    boolType is_pixmap;
    boolType is_subwindow;
    int width;
    int height;
  } emc_winRecord, *emc_winType;

typedef const emc_winRecord *const_emc_winType;

#define to_window(win)       (((const_emc_winType) win)->window)
#define is_pixmap(win)       (((const_emc_winType) win)->is_pixmap)
#define is_subwindow(win)    (((const_emc_winType) win)->is_subwindow)
#define to_width(win)        (((const_emc_winType) win)->width)
#define to_height(win)       (((const_emc_winType) win)->height)

#define to_var_window(win)       (((emc_winType) win)->window)
#define is_var_pixmap(win)       (((emc_winType) win)->is_pixmap)
#define is_var_subwindow(win)    (((emc_winType) win)->is_subwindow)
#define to_var_width(win)        (((emc_winType) win)->width)
#define to_var_height(win)       (((emc_winType) win)->height)



charType gkbGetc (void)

  { /* gkbGetc */
    logFunction(printf("gkbGetc\n"););
    return (charType) EOF;
  } /* gkbGetc */



boolType gkbKeyPressed (void)

  { /* gkbKeyPressed */
    logFunction(printf("gkbKeyPressed\n"););
    return FALSE;
  } /* gkbKeyPressed */



boolType gkbButtonPressed (charType button)

  { /* gkbButtonPressed */
    logFunction(printf("gkbButtonPressed(%04x)\n", button););
    return FALSE;
  } /* gkbButtonPressed */



charType gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



void gkbSelectInput (winType aWindow, charType aKey, boolType active)

  { /* gkbSelectInput */
    if (aKey != K_RESIZE && aKey != K_CLOSE) {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* gkbSelectInput */



winType gkbWindow (void)

  { /* gkbWindow */
    return NULL;
  } /* gkbWindow */



intType gkbButtonXpos (void)

  { /* gkbButtonXpos */
    return 0;
  } /* gkbButtonXpos */



intType gkbButtonYpos (void)

  { /* gkbButtonYpos */
    return 0;
  } /* gkbButtonYpos */



intType drwPointerXpos (const_winType actual_window)

  { /* drwPointerXpos */
    return 0;
  } /* drwPointerXpos */



intType drwPointerYpos (const_winType actual_window)

  { /* drwPointerYpos */
    return 0;
  } /* drwPointerYpos */



void drwArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  { /* drwArc */
    logFunction(printf("drwArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
  } /* drwArc */



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  { /* drwPArc */
    logFunction(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
  } /* drwPArc */



void drwPFArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle,
    intType width, intType col)

  { /* drwPFArc */
    logFunction(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, width););
  } /* drwPFArc */



void drwFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2)

  { /* drwFArcChord */
  } /* drwFArcChord */



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2, intType col)

  { /* drwPFArcChord */
  } /* drwPFArcChord */



void drwFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2)

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2, intType col)

  { /* drwPFArcPieSlice */
  } /* drwPFArcPieSlice */



void drwArc2 (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType radius)

  { /* drwArc2 */
  } /* drwArc2 */



/**
 *  Determine the border widths of a window in pixels.
 *  These are the widths of the window decorations in the succession
 *  top, right, bottom, left.
 *  @return an array with border widths (top, right, bottom, left).
 */
rtlArrayType drwBorder (const_winType actual_window)

  {
    int twoBorders;
    rtlArrayType border;

  /* drwBorder */
    logFunction(printf("drwBorder(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      border = NULL;
    } else {
      twoBorders = EM_ASM_INT({
        if (typeof window !== 'undefined' && typeof mapIdToWindow[$0] !== 'undefined') {
          let windowObject = mapIdToWindow[$0];
          let rightBottomLeftBorder = windowObject.outerWidth - windowObject.innerWidth;
          let topBorder = windowObject.outerHeight - windowObject.innerHeight - rightBottomLeftBorder;
          if (rightBottomLeftBorder <= 32767 && topBorder <= 65535) {
            return rightBottomLeftBorder << 16 | topBorder;
          } else {
            return -1;
          }
        } else {
          console.log('windowId not found: ' + $0);
          return -1;
        }
      }, to_window(actual_window));
      if (unlikely(twoBorders == -1)) {
        raise_error(FILE_ERROR);
        border = NULL;
      } else if (unlikely(!ALLOC_RTL_ARRAY(border, 4))) {
        raise_error(MEMORY_ERROR);
      } else {
        border->min_position = 1;
        border->max_position = 4;
        border->arr[0].value.intValue = (intType) (twoBorders & 0xffff);
        twoBorders >>= 16;
        border->arr[1].value.intValue = (intType) twoBorders;
        border->arr[2].value.intValue = (intType) twoBorders;
        border->arr[3].value.intValue = (intType) twoBorders;
      } /* if */
    } /* if */
    logFunction(printf("drwBorder(" FMT_U_MEM ") -->"
                       " %s[" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D "]\n",
                       (memSizeType) actual_window,
                       border != NULL ? "" : "NULL ",
                       border != NULL ? border->arr[0].value.intValue : 0,
                       border != NULL ? border->arr[1].value.intValue : 0,
                       border != NULL ? border->arr[2].value.intValue : 0,
                       border != NULL ? border->arr[3].value.intValue : 0););
    return border;
  } /* drwBorder */



void drwCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwCircle */
    logFunction(printf("drwCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
  } /* drwCircle */



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPCircle */
    logFunction(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.strokeStyle = '#' + ('000000' + $4.toString(16)).slice(-6);
        context.beginPath();
        context.arc($1, $2, $3, 0, 2 * Math.PI);
        context.stroke();
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), castToInt(x), castToInt(y),
        castToInt(radius), (int) col);
  } /* drwPCircle */



void drwClear (winType actual_window, intType col)

  { /* drwClear */
    logFunction(printf("drwClear(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.fillStyle = '#' + ('000000' + $1.toString(16)).slice(-6);
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), (int) col);
  } /* drwClear */



void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y)

  { /* drwCopyArea */
    logFunction(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) src_window, (memSizeType) dest_window,
                       src_x, src_y, width, height, dest_x, dest_y););
  } /* drwCopyArea */



void drwFCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwFCircle */
    logFunction(printf("drwFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
  } /* drwFCircle */



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPFCircle */
    logFunction(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.fillStyle = '#' + ('000000' + $4.toString(16)).slice(-6);
        context.beginPath();
        context.arc($1, $2, $3, 0, 2 * Math.PI);
        context.fill();
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), castToInt(x), castToInt(y),
        castToInt(radius), (int) col);
  } /* drwPFCircle */



void drwFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwFEllipse */
    logFunction(printf("drwFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
  } /* drwFEllipse */



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPFEllipse */
    logFunction(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
  } /* drwPFEllipse */



void drwFlush (void)

  { /* drwFlush */
  } /* drwFlush */



winType drwEmpty (void)

  {
    emc_winType emptyWindow;

  /* drwEmpty */
    logFunction(printf("drwEmpty()\n"););
    if (unlikely(!ALLOC_RECORD2(emptyWindow, emc_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(emptyWindow, 0, sizeof(emc_winRecord));
      emptyWindow->usage_count = 0;  /* Do not use reference counting (will not be freed). */
      emptyWindow->window = 0;
      emptyWindow->is_pixmap = TRUE;
      emptyWindow->is_subwindow = FALSE;
      emptyWindow->width = 0;
      emptyWindow->height = 0;
    } /* if */
    logFunction(printf("drwEmpty --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) emptyWindow,
                       emptyWindow != NULL ? emptyWindow->usage_count : (uintType) 0););
    return (winType) emptyWindow;
  } /* drwEmpty */



void drwFree (winType old_window)

  { /* drwFree */
    logFunction(printf("drwFree(" FMT_U_MEM ") (usage=" FMT_U ")\n",
                       (memSizeType) old_window,
                       old_window != NULL ? old_window->usage_count : (uintType) 0););
    FREE_RECORD2(old_window, emc_winRecord, count.win, count.win_bytes);
  } /* drwFree */



/**
 *  Create a new pixmap with the given 'width' and 'height'.
 *  A rectangle with the upper left corner at (left, upper) and the given
 *  'width' and 'height' is copied from 'source_window' to the new pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative.
 *  @return the new pixmap.
 */
winType drwGet (const_winType source_window, intType left, intType upper,
    intType width, intType height)

  {
    emc_winType pixmap;

  /* drwGet */
    logFunction(printf("drwGet(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) source_window, left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(!ALLOC_RECORD2(pixmap, emc_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(pixmap, 0, sizeof(emc_winRecord));
      pixmap->usage_count = 1;

      pixmap->window = EM_ASM_INT({
        if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
          let sourceContext = mapIdToContext[$0];
          let left = $1;
          let upper = $2;
          let width = $3;
          let height = $4;
          let canvas = document.createElement('canvas');
          canvas.width = width;
          canvas.height = height;
          let context = canvas.getContext('2d');
          let imageData = sourceContext.getImageData($1, $2, $3, $4);
          context.putImageData(imageData, 0, 0);
          currentWindowId++;
          mapIdToCanvas[currentWindowId] = canvas;
          mapIdToContext[currentWindowId] = context;
          return currentWindowId;
        } else {
          console.log('windowId not found: ' + $0);
        }
      }, to_window(source_window), castToInt(left), castToInt(upper),
          castToInt(width), castToInt(height));

      pixmap->is_pixmap = TRUE;
      pixmap->is_subwindow = FALSE;
      pixmap->width = (int) width;
      pixmap->height = (int) height;
    } /* if */
    logFunction(printf("drwGet --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwGet */



winType drwCapture (intType left, intType upper,
    intType width, intType height)

  { /* drwCapture */
    return NULL;
  } /* drwCapture */



bstriType drwGetImage (const_winType source_window)

  {
    bstriType result;

  /* drwGetImage */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
      raise_error(RANGE_ERROR);
    } else {
      result->size = 0;
    } /* if */
    return result;
  } /* drwGetImage */



intType drwGetPixel (const_winType source_window, intType x, intType y)

  {
    intType col;

  /* drwGetPixel */
    col = (intType) EM_ASM_INT({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        let canvasColor = context.getImageData(x, y, 1, 1).data; // rgba e [0,255]
        let r = canvasColor[0];
        let g = canvasColor[1];
        let b = canvasColor[2];
        return canvasColor[0] << 16 | canvasColor[1] << 8 | canvasColor[2]
      } else {
        console.log('windowId not found: ' + $0);
        return 0;
      }
    }, to_window(source_window), castToInt(x), castToInt(y));
    return col;
  } /* drwGetPixel */



intType drwHeight (const_winType actual_window)

  {
    int height;

  /* drwHeight */
    logFunction(printf("drwHeight(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_pixmap(actual_window)) {
      height = to_height(actual_window);
    } else {
      height = EM_ASM_INT({
        if (typeof window !== 'undefined' && typeof mapIdToCanvas[$0] !== 'undefined') {
          return mapIdToCanvas[$0].height;
        } else {
          console.log('windowId not found: ' + $0);
          return -1;
        }
      }, to_window(actual_window));
      if (unlikely(height == -1)) {
        raise_error(FILE_ERROR);
        height = 0;
      } /* if */
    } /* if */
    logFunction(printf("drwHeight(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, height););
    return (intType) height;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height)

  { /* drwImage */
    return NULL;
  } /* drwImage */



void drwLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2)

  { /* drwLine */
  } /* drwLine */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  { /* drwPLine */
    logFunction(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.strokeStyle = '#' + ('000000' + $5.toString(16)).slice(-6);
        context.beginPath();
        context.moveTo($1, $2);
        context.lineTo($3, $4);
        context.stroke();
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), castToInt(x1), castToInt(y1),
        castToInt(x2), castToInt(y2), (int) col);
  } /* drwPLine */



winType drwNewPixmap (intType width, intType height)

  {
    emc_winType pixmap;

  /* drwNewPixmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(!inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else {
      if (unlikely(!ALLOC_RECORD2(pixmap, emc_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(emc_winRecord));
        pixmap->usage_count = 1;

        pixmap->window = EM_ASM_INT({
          if (typeof window !== 'undefined') {
            let width = $0;
            let height = $1;
            let canvas = document.createElement('canvas');
            canvas.width = width;
            canvas.height = height;
            let context = canvas.getContext('2d');
            currentWindowId++;
            mapIdToCanvas[currentWindowId] = canvas;
            mapIdToContext[currentWindowId] = context;
            return currentWindowId;
          } else {
            return 0;
          }
        }, (int) width, (int) height);

        pixmap->is_pixmap = TRUE;
        pixmap->is_subwindow = FALSE;
        pixmap->width = (int) width;
        pixmap->height = (int) height;
      } /* if */
    } /* if */
    logFunction(printf("drwNewPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwNewPixmap */



winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType window_name)

  {
    char *win_name;
    errInfoType err_info = OKAY_NO_ERROR;
    emc_winType result = NULL;

  /* drwOpen */
    logFunction(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", \"%s\")\n",
                        xPos, yPos, width, height, striAsUnquotedCStri(window_name)););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else {
      win_name = stri_to_cstri8(window_name, &err_info);
      if (unlikely(win_name == NULL)) {
        logError(printf("drwOpen: stri_to_cstri8(\"%s\") failed:\n"
                        "err_info=%d\n",
                        striAsUnquotedCStri(window_name), err_info););
        raise_error(err_info);
      } else {
        if (ALLOC_RECORD2(result, emc_winRecord, count.win, count.win_bytes)) {
          memset(result, 0, sizeof(emc_winRecord));
          result->usage_count = 1;

          result->window = EM_ASM_INT({
            if (typeof window !== 'undefined') {
              let left = $0;
              let top = $1;
              let width = $2;
              let height = $3;
              let windowName = Module.UTF8ToString($4);
              let windowFeatures = 'toolbar=no,menubar=no,left=' + left + ',top=' + top +
                                   ',width=' + width + ',height=' + height;
              let windowObject = window.open("", windowName, windowFeatures);
              if (windowObject == null) {
                return 0;
              } else {
                const title = windowObject.document.createElement('title');
                const titleText = windowObject.document.createTextNode(windowName);
                title.appendChild(titleText);
                windowObject.document.head.appendChild(title);
                windowObject.document.body.style.margin = 0;
                let canvas = windowObject.document.createElement('canvas');
                let context = canvas.getContext('2d');
                canvas.width  = windowObject.innerWidth;
                canvas.height = windowObject.innerHeight;
                context.fillStyle = '#000000';
                context.fillRect(0, 0, width, height);
                windowObject.document.body.appendChild(canvas);
                currentWindowId++;
                mapIdToWindow[currentWindowId] = windowObject;
                mapIdToCanvas[currentWindowId] = canvas;
                mapIdToContext[currentWindowId] = context;
                return currentWindowId;
              }
            } else {
              return 0;
            }
          }, (int) xPos, (int) yPos, (int) width, (int) height, win_name);
          result->is_pixmap = FALSE;
          result->is_subwindow = FALSE;
          result->width = (int) width;
          result->height = (int) height;
        } /* if */
        free_cstri8(win_name, window_name);
      } /* if */
    } /* if */
    logFunction(printf("drwOpen --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpen */



/**
 *  Create a sub window inside of 'parent_window'.
 *  The new sub window has no window decorations and is not managed by
 *  the window manager. If the empty window is used as 'parent_window'
 *  an unmanaged top level window without window decorations is generated.
 *  The coordinates 'xPos' and 'yPos' are measured relative to the top
 *  left corner of the 'parent_window' drawing area (inside of the window
 *  decorations). If the empty window is used as 'parent_window' the
 *  coordinates 'xPos' and 'yPos' are measured relative to the top left
 *  corner of the screen.
 *  @param parent-window Parent window (can be the empty window).
 *  @param xPos X-position of the left corner of the new window.
 *  @param yPos Y-position of the left corner of the new window.
 *  @param width Width of the new window.
 *  @param height Height of the new window.
 *  @return the new generated window.
 */
winType drwOpenSubWindow (const_winType parent_window, intType xPos, intType yPos,
    intType width, intType height)

  {
    emc_winType result = NULL;

  /* drwOpenSubWindow */
    logFunction(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else {
      if (ALLOC_RECORD2(result, emc_winRecord, count.win, count.win_bytes)) {
        memset(result, 0, sizeof(emc_winRecord));
        result->usage_count = 1;

        result->window = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToWindow[$0] !== 'undefined') {
            let windowObject = mapIdToWindow[$0];
            let left = $1;
            let top = $2;
            let width = $3;
            let height = $4;
            let canvas = windowObject.document.createElement('canvas');
            canvas.width = width;
            canvas.height = height;
            canvas.style.left = left;
            canvas.style.top = top;
            canvas.style.position = 'absolute';
            let context = canvas.getContext('2d');
            windowObject.document.body.appendChild(canvas);
            currentWindowId++;
            mapIdToCanvas[currentWindowId] = canvas;
            mapIdToContext[currentWindowId] = context;
            return currentWindowId;
          } else {
            return 0;
          }
        }, to_window(parent_window), (int) xPos, (int) yPos, (int) width, (int) height);

        result->is_pixmap = FALSE;
        result->is_subwindow = TRUE;
        result->width = (int) width;
        result->height = (int) height;
      } /* if */
    } /* if */
    logFunction(printf("drwOpenSubWindow --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpenSubWindow */



void drwSetCloseAction (winType actual_window, intType closeAction)

  { /* drwSetCloseAction */
  } /* drwSetCloseAction */



void drwSetCursorVisible (winType aWindow, boolType visible)

  { /* drwSetCursorVisible */
  } /* drwSetCursorVisible */



void drwPoint (const_winType actual_window, intType x, intType y)

  { /* drwPoint */
  } /* drwPoint */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  { /* drwPPoint */
    logFunction(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.fillStyle = '#' + ('000000' + $3.toString(16)).slice(-6);
        context.fillRect($1, $2, 1, 1);
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), castToInt(x), castToInt(y), (int) col);
  } /* drwPPoint */



rtlArrayType drwConvPointList (const const_bstriType pointList)

  {
    memSizeType numCoords;
    int *coords;
    memSizeType pos;
    rtlArrayType xyArray;

  /* drwConvPointList */
    numCoords = pointList->size / sizeof(int);
    if (unlikely(!ALLOC_RTL_ARRAY(xyArray, numCoords))) {
      raise_error(MEMORY_ERROR);
    } else {
      xyArray->min_position = 1;
      xyArray->max_position = (intType) numCoords;
      coords = (int *) pointList->mem;
      for (pos = 0; pos < numCoords; pos ++) {
        xyArray->arr[pos].value.intValue = (intType) coords[pos];
      } /* for */
    } /* if */
    return xyArray;
  } /* drwConvPointList */



bstriType drwGenPointList (const const_rtlArrayType xyArray)

  {
    memSizeType num_elements;
    memSizeType len;
    int *coords;
    memSizeType pos;
    bstriType result;

  /* drwGenPointList */
    logFunction(printf("drwGenPointList(" FMT_D " .. " FMT_D ")\n",
                       xyArray->min_position, xyArray->max_position););
    num_elements = arraySize(xyArray);
    if (unlikely(num_elements & 1)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      len = num_elements >> 1;
      if (unlikely(len > MAX_BSTRI_LEN / (sizeof(int) << 1) || len > MAX_MEM_INDEX)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, len * (sizeof(int) << 1)))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len * (sizeof(int) << 1);
          if (len > 0) {
            coords = (int *) result->mem;
            for (pos = 0; pos < len << 1; pos++) {
              coords[pos] = castToLong(xyArray->arr[pos].value.intValue);
            } /* for */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGenPointList */



intType drwLngPointList (bstriType point_list)

  { /* drwLngPointList */
    return (intType) ((point_list->size / sizeof(int)) >> 1);
  } /* drwLngPointList */



void drwPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    int *coords;
    memSizeType numCoords;

  /* drwPolyLine */
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      raise_error(RANGE_ERROR);
    } else {
      coords = (int *) point_list->mem;
      numCoords = point_list->size / sizeof(int);
      if (numCoords >= 4) {
        EM_ASM({
          if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
            let context = mapIdToContext[$0];
            context.lineWidth = 1;
            context.strokeStyle = '#' + ('000000' + $5.toString(16)).slice(-6);
            context.beginPath();
            context.moveTo($1 + HEAP32[$4 >> 2], $2 + HEAP32[($4 >> 2) + 1]);
            for (let i = 2; i < $3; i += 2) {
              context.lineTo($1 + HEAP32[($4 >> 2) + i], $2 + HEAP32[($4 >> 2) + i + 1]);
            }
            context.stroke();
          } else {
            console.log('windowId not found: ' + $0);
          }
        }, to_window(actual_window), (int) x, (int) y, numCoords, coords, (int) col);
      } /* if */
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    int *coords;
    memSizeType numCoords;

  /* drwFPolyLine */
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      raise_error(RANGE_ERROR);
    } else {
      coords = (int *) point_list->mem;
      numCoords = point_list->size / sizeof(int);
      if (numCoords >= 4) {
        EM_ASM({
          if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
            let context = mapIdToContext[$0];
            context.lineWidth = 1;
            context.strokeStyle = '#' + ('000000' + $5.toString(16)).slice(-6);
            context.beginPath();
            context.moveTo($1 + HEAP32[$4 >> 2], $2 + HEAP32[($4 >> 2) + 1]);
            for (let i = 2; i < $3; i += 2) {
              context.lineTo($1 + HEAP32[($4 >> 2) + i], $2 + HEAP32[($4 >> 2) + i + 1]);
            }
            context.closePath();
            context.fill();
          } else {
            console.log('windowId not found: ' + $0);
          }
        }, to_window(actual_window), (int) x, (int) y, numCoords, coords, (int) col);
      } /* if */
    } /* if */
  } /* drwFPolyLine */



void drwPut (const_winType destWindow, intType xDest, intType yDest,
    const_winType pixmap)

  { /* drwPut */
    logFunction(printf("drwPut(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ")\n",
                       (memSizeType) destWindow, xDest, yDest, (memSizeType) pixmap););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined' &&
                                           typeof mapIdToCanvas[$1] !== 'undefined') {
        mapIdToContext[$0].drawImage(mapIdToCanvas[$1], $2, $3);
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(destWindow), to_window(pixmap), castToInt(xDest), castToInt(yDest));
  } /* drwPut */



void drwPutScaled (const_winType destWindow, intType xDest, intType yDest,
    intType width, intType height, const_winType pixmap)

  { /* drwPutScaled */
    logFunction(printf("drwPutScaled(" FMT_U_MEM  ", " FMT_D ", " FMT_D ", "
                       FMT_D ", " FMT_D ", " FMT_U_MEM")\n",
                       (memSizeType) destWindow, xDest, yDest,
                       width, height, (memSizeType) pixmap););
    logFunction(printf("drwPutScaled -->\n"););
  } /* drwPutScaled */



void drwRect (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwRect */
  } /* drwRect */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPRect */
    logFunction(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, width, height, col););
    EM_ASM({
      if (typeof window !== 'undefined' && typeof mapIdToContext[$0] !== 'undefined') {
        let context = mapIdToContext[$0];
        context.fillStyle = '#' + ('000000' + $5.toString(16)).slice(-6);
        context.fillRect($1, $2, $3, $4);
      } else {
        console.log('windowId not found: ' + $0);
      }
    }, to_window(actual_window), castToInt(x), castToInt(y),
        castToInt(width), castToInt(height), (int) col);
  } /* drwPRect */



intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  {
    intType col;

  /* drwRgbColor */
    col = (intType) ((((((uintType) redLight)   >> 8) & 255) << 16) |
                     (((((uintType) greenLight) >> 8) & 255) <<  8) |
                      ((((uintType) blueLight)  >> 8) & 255));
    logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> " F_X(08) "\n",
                       redLight, greenLight, blueLight, col););
    return col;
  } /* drwRgbColor */



void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight)

  { /* drwPixelToRgb */
    *redLight   = (intType) (( ((uintType) col) >> 16       ) << 8);
    *greenLight = (intType) (((((uintType) col) >>  8) & 255) << 8);
    *blueLight  = (intType) (( ((uintType) col)        & 255) << 8);
  } /* drwPixelToRgb */



void drwBackground (intType col)

  { /* drwBackground */
  } /* drwBackground */



void drwColor (intType col)

  { /* drwColor */
  } /* drwColor */



intType drwScreenHeight (void)

  {
    int height;

  /* drwScreenHeight */
    height = EM_ASM_INT({
      if (typeof window !== 'undefined') {
        return window.screen.height;
      } else {
        return -1;
      }
    });
    if (unlikely(height == -1)) {
      raise_error(FILE_ERROR);
      height = 0;
    } /* if */
    return (intType) height;
  } /* drwScreenHeight */



intType drwScreenWidth (void)

  {
    int width;

  /* drwScreenWidth */
    width = EM_ASM_INT({
      if (typeof window !== 'undefined') {
        return window.screen.width;
      } else {
        return -1;
      }
    });
    if (unlikely(width == -1)) {
      raise_error(FILE_ERROR);
      width = 0;
    } /* if */
    return (intType) width;
  } /* drwScreenWidth */



void drwSetContent (const_winType actual_window, const_winType pixmap)

  { /* drwSetContent */
  } /* drwSetContent */



/**
 *  Move the top left corner of a window to the coordinates x/y.
 *  If window decorations are present the top left corner of the
 *  window decorations will be at the position x/y. For a sub window
 *  the position is relative to the top left corner of the parent window
 *  drawing area (inside of the window decorations). For top level windows
 *  the position is relative to the top left corner of the screen.
 */
void drwSetPos (const_winType actual_window, intType xPos, intType yPos)

  {
    int okay;

  /* drwSetPos */
    logFunction(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, xPos, yPos););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos))) {
      raise_error(RANGE_ERROR);
    } else if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
    } else {
      if (is_subwindow(actual_window)) {
        okay = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToCanvas[$0] !== 'undefined') {
            let canvas = mapIdToCanvas[$0];
            canvas.style.left = $1;
            canvas.style.top = $2;
            return 1;
          } else {
            console.log('windowId not found: ' + $0);
            return 0;
          }
        }, to_window(actual_window), (int) xPos, (int) yPos);
      } else {
        okay = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToWindow[$0] !== 'undefined') {
            let windowObject = mapIdToWindow[$0];
            windowObject.screenX = $1;
            windowObject.screenY = $2;
            return 1;
          } else {
            console.log('windowId not found: ' + $0);
            return 0;
          }
        }, to_window(actual_window), (int) xPos, (int) yPos);
      } /* if */
      if (unlikely(!okay)) {
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* drwSetPos */



void drwSetTransparentColor (winType pixmap, intType col)

  { /* drwSetTransparentColor */
  } /* drwSetTransparentColor */



void drwSetWindowName (winType aWindow, const const_striType windowName)

  { /* drwSetWindowName */
  } /* drwSetWindowName */



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  { /* drwText */
  } /* drwText */



void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
  } /* drwToBottom */



void drwToTop (const_winType actual_window)

  { /* drwToTop */
  } /* drwToTop */



intType drwWidth (const_winType actual_window)

  {
    int width;

  /* drwWidth */
    logFunction(printf("drwWidth(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_pixmap(actual_window)) {
      width = to_width(actual_window);
    } else {
      width = EM_ASM_INT({
        if (typeof window !== 'undefined' && typeof mapIdToCanvas[$0] !== 'undefined') {
          return mapIdToCanvas[$0].width;
        } else {
          console.log('windowId not found: ' + $0);
          return -1;
        }
      }, to_window(actual_window));
      if (unlikely(width == -1)) {
        raise_error(FILE_ERROR);
        width = 0;
      } /* if */
    } /* if */
    logFunction(printf("drwWidth(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, width););
    return (intType) width;
  } /* drwWidth */



/**
 *  Determine the X position of the top left corner of a window in pixels.
 *  If window decorations are present this uses the top left corner of
 *  the window decorations. For a sub window the X position is relative
 *  to the top left corner of the parent window drawing area (inside of
 *  the window decorations). For top level windows the X position is
 *  relative to the top left corner of the screen.
 *  @exception RANGE_ERROR If 'actual_window' is a pixmap.
 */
intType drwXPos (const_winType actual_window)

  {
    int xPos;

  /* drwXPos */
    logFunction(printf("drwXPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      xPos = 0;
    } else {
      if (is_subwindow(actual_window)) {
        xPos = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToCanvas[$0] !== 'undefined') {
            return mapIdToCanvas[$0].style.left;
          } else {
            console.log('windowId not found: ' + $0);
            return -2147483648;
          }
        }, to_window(actual_window));
      } else {
        xPos = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToWindow[$0] !== 'undefined') {
            return mapIdToWindow[$0].screenX;
          } else {
            console.log('windowId not found: ' + $0);
            return -2147483648;
          }
        }, to_window(actual_window));
      } /* if */
      if (unlikely(xPos == -2147483648)) {
        raise_error(FILE_ERROR);
        xPos = 0;
      } /* if */
    } /* if */
    logFunction(printf("drwXPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, xPos););
    return (intType) xPos;
  } /* drwXPos */



/**
 *  Determine the Y position of the top left corner of a window in pixels.
 *  If window decorations are present this uses the top left corner of
 *  the window decorations. For a sub window the Y position is relative
 *  to the top left corner of the parent window drawing area (inside of
 *  the window decorations). For top level windows the Y position is
 *  relative to the top left corner of the screen.
 *  @exception RANGE_ERROR If 'actual_window' is a pixmap.
 */
intType drwYPos (const_winType actual_window)

  {
    int yPos;

  /* drwYPos */
    logFunction(printf("drwYPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      yPos = 0;
    } else {
      if (is_subwindow(actual_window)) {
        yPos = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToCanvas[$0] !== 'undefined') {
            return mapIdToCanvas[$0].style.top;
          } else {
            console.log('windowId not found: ' + $0);
            return -2147483648;
          }
        }, to_window(actual_window));
      } else {
        yPos = EM_ASM_INT({
          if (typeof window !== 'undefined' && typeof mapIdToWindow[$0] !== 'undefined') {
            return mapIdToWindow[$0].screenY;
          } else {
            console.log('windowId not found: ' + $0);
            return -2147483648;
          }
        }, to_window(actual_window));
      } /* if */
      if (unlikely(yPos == -2147483648)) {
        raise_error(FILE_ERROR);
        yPos = 0;
      } /* if */
    } /* if */
    logFunction(printf("drwYPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, yPos););
    return (intType) yPos;
  } /* drwYPos */
