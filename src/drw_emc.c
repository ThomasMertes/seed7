/********************************************************************/
/*                                                                  */
/*  drw_emc.c     Graphic access using the browser.                 */
/*  Copyright (C) 2020 - 2023  Thomas Mertes                        */
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
/*  Changes: 2020 - 2023  Thomas Mertes                             */
/*  Content: Graphic access using the browser.                      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "float.h"
#include "limits.h"
#include "emscripten.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "int_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "flt_rtl.h"
#include "tim_drv.h"
#include "kbd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"


#define PI 3.141592653589793238462643383279502884197

typedef struct {
    uintType usage_count;
    /* Up to here the structure is identical to struct winStruct */
    int window;
    boolType is_pixmap;
    boolType is_subwindow;
    boolType is_substitute;
    boolType is_tab;
    const_winType parentWindow;
    int ignoreFirstResize;
    intType creationTimestamp;
    int width;
    int height;
    intType clear_col;
    boolType resizeReturnsKey;
    int close_action;
  } emc_winRecord, *emc_winType;

typedef const emc_winRecord *const_emc_winType;

#define to_window(win)            (((const_emc_winType) (win))->window)
#define is_pixmap(win)            (((const_emc_winType) (win))->is_pixmap)
#define is_subwindow(win)         (((const_emc_winType) (win))->is_subwindow)
#define is_substitute(win)        (((const_emc_winType) (win))->is_substitute)
#define is_tab(win)               (((const_emc_winType) (win))->is_tab)
#define to_parentWindow(win)      (((const_emc_winType) (win))->parentWindow)
#define to_ignoreFirstResize(win) (((const_emc_winType) (win))->ignoreFirstResize)
#define to_creationTimestamp(win) (((const_emc_winType) (win))->creationTimestamp)
#define to_width(win)             (((const_emc_winType) (win))->width)
#define to_height(win)            (((const_emc_winType) (win))->height)
#define to_clear_col(win)         (((const_emc_winType) (win))->clear_col)
#define to_resizeReturnsKey(win)  (((const_emc_winType) (win))->resizeReturnsKey)
#define to_close_action(win)      (((const_emc_winType) (win))->close_action)

#define to_var_window(win)            (((emc_winType) (win))->window)
#define is_var_pixmap(win)            (((emc_winType) (win))->is_pixmap)
#define is_var_subwindow(win)         (((emc_winType) (win))->is_subwindow)
#define is_var_substitute(win)        (((emc_winType) (win))->is_substitute)
#define is_var_tab(win)               (((emc_winType) (win))->is_tab)
#define to_var_parentWindow(win)      (((emc_winType) (win))->parentWindow)
#define to_var_ignoreFirstResize(win) (((emc_winType) (win))->ignoreFirstResize)
#define to_var_creationTimestamp(win) (((emc_winType) (win))->creationTimestamp)
#define to_var_width(win)             (((emc_winType) (win))->width)
#define to_var_height(win)            (((emc_winType) (win))->height)
#define to_var_clear_col(win)         (((emc_winType) (win))->clear_col)
#define to_var_resizeReturnsKey(win)  (((emc_winType) (win))->resizeReturnsKey)
#define to_var_close_action(win)      (((emc_winType) (win))->close_action)

int maxWindowId = 0;
static winType emptyWindow;
static boolType firstWindowOpen = TRUE;
static boolType openSubstitute = FALSE;


winType find_window (int windowId);
void enter_window (winType curr_window, int windowId);
void remove_window (int windowId);
void setClosePopupState (int windowId, int state);
void setupEventCallbacksForWindow (int windowId);
void gkbInitKeyboard (void);
void gkbCloseKeyboard (void);
void synchronizeTimAwaitWithGraphicKeyboard (void);
extern intType pointerX;
extern intType pointerY;



boolType isSubWindow (winType aWindow)

  {
    return is_subwindow(aWindow);
  }



boolType isTab (winType aWindow)

  {
    return is_tab(aWindow);
  }



int getCloseAction (winType aWindow)

  {
    int closeAction;

  /* getCloseAction */
    closeAction = aWindow != NULL ?
        to_close_action(aWindow) :
        CLOSE_BUTTON_RAISES_EXCEPTION;
    logFunction(printf("getCloseAction(" FMT_U_MEM " (window=%d)) --> %d\n",
                       (memSizeType) aWindow,
                       aWindow != NULL ? to_window(aWindow) : 0,
                       closeAction););
    return closeAction;
  } /* getCloseAction */



boolType windowExists (int windowId)

  {
    int exists;

  /* windowExists */
    logFunction(printf("windowExists(%d)\n", windowId););
    exists = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
        let windowObject = mapIdToWindow[$0];
        if (windowObject.closed) {
          return 0;
        } else {
          return 1;
        }
      } else {
        return 0;
      }
    }, windowId);
    logFunction(printf("windowExists(%d) --> %d\n",
                       windowId, exists););
    return (boolType) exists;
  } /* windowExists */



boolType ignoreResize (winType aWindow, int width, int height)

  {
    intType currentTimestamp;
    boolType doIgnoreResize;

  /* ignoreResize */
    logFunction(printf("ignoreResize(" FMT_U_MEM ", %d, %d)\n",
                        (memSizeType) aWindow, width, height););
    switch (to_ignoreFirstResize(aWindow)) {
      case 1:
        to_var_ignoreFirstResize(aWindow) = 0;
        currentTimestamp = timMicroSec() / 1000000;
        doIgnoreResize = to_creationTimestamp(aWindow) != 0 &&
                         currentTimestamp >= to_creationTimestamp(aWindow) &&
                         currentTimestamp <= to_creationTimestamp(aWindow) + 1;
        break;
      case 2:
        currentTimestamp = timMicroSec() / 1000000;
        if (to_creationTimestamp(aWindow) != 0 &&
            currentTimestamp >= to_creationTimestamp(aWindow) &&
            currentTimestamp <= to_creationTimestamp(aWindow) + 1) {
          if (to_width(aWindow) == width && to_height(aWindow) == height) {
            to_var_ignoreFirstResize(aWindow) = 0;
          } else {
            to_var_ignoreFirstResize(aWindow) = 1;
          } /* if */
          doIgnoreResize = TRUE;
        } else {
          to_var_ignoreFirstResize(aWindow) = 0;
          doIgnoreResize = FALSE;
        } /* if */
        break;
      default:
        doIgnoreResize = FALSE;
        break;
    } /* switch */
    logFunction(printf("ignoreResize --> %d\n", doIgnoreResize););
    return doIgnoreResize;
  } /* ignoreResize */



void setResizeReturnsKey (winType resizeWindow, boolType active)

  { /* setResizeReturnsKey */
    to_var_resizeReturnsKey(resizeWindow) = active;
  } /* setResizeReturnsKey */



boolType resize (winType resizeWindow, int width, int height)

  {
    int successInfo;
    boolType returnKeyResize = FALSE;

  /* resize */
    logFunction(printf("resize(" FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) resizeWindow, width, height););
    if (to_width(resizeWindow) != width || to_height(resizeWindow) != height) {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined" &&
                                             typeof mapIdToContext[$0] !== "undefined") {
          let canvas = mapIdToCanvas[$0];
          if ($1 > canvas.width || $2 > canvas.height) {
            let context = mapIdToContext[$0];
            let imageData = context.getImageData(0, 0, canvas.width, canvas.height);
            if ($1 > canvas.width) {
              canvas.width = $1;
            }
            if ($2 > canvas.height) {
              canvas.height = $2;
            }
            context.fillStyle = "#" + ("000000" + $3.toString(16)).slice(-6);
            context.fillRect(0, 0, context.canvas.width, context.canvas.height);
            context.putImageData(imageData, 0, 0);
          }
          return 0;
        } else {
          return 1;
        }
      }, to_window(resizeWindow), width, height,
          (int) (to_clear_col(resizeWindow) & 0xffffff));
      if (likely(successInfo == 0)) {
        to_var_width(resizeWindow) = width;
        to_var_height(resizeWindow) = height;
        returnKeyResize = to_resizeReturnsKey(resizeWindow);
      } /* if */
    } /* if */
    logFunction(printf("resize --> %d\n", returnKeyResize););
    return returnKeyResize;
  } /* resize */



winType generateEmptyWindow (void)

  {
    emc_winType newWindow;

  /* generateEmptyWindow */
    logFunction(printf("generateEmptyWindow()\n"););
    if (unlikely(!ALLOC_RECORD2(newWindow, emc_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(newWindow, 0, sizeof(emc_winRecord));
      newWindow->usage_count = 0;  /* Do not use reference counting (will not be freed). */
      newWindow->window = 0;
      newWindow->is_pixmap = TRUE;
      newWindow->is_subwindow = FALSE;
      newWindow->is_substitute = FALSE;
      newWindow->is_tab = FALSE;
      newWindow->parentWindow = NULL;
      newWindow->ignoreFirstResize = 0;
      newWindow->creationTimestamp = 0;
      newWindow->width = 0;
      newWindow->height = 0;
    } /* if */
    logFunction(printf("generateEmptyWindow --> " FMT_U_MEM " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->window : 0,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
  } /* generateEmptyWindow */



void drawShut (void)

  {
    int windowId;
    winType window;

  /* drawShut */
    logFunction(printf("drawShut\n"););
    for (windowId = 1; windowId <= maxWindowId; windowId++) {
      window = find_window(windowId);
      if (window != NULL) {
        drwFree(window);
      } /* if */
    } /* for */
    maxWindowId = 0;
    logFunction(printf("drawShut -->\n"););
  } /* drawShut */



void drawClose (void)

  { /* drawClose */
    if (emptyWindow != NULL) {
      FREE_RECORD2(emptyWindow, emc_winRecord, count.win, count.win_bytes);
      emptyWindow = NULL;
    } /* if */
    gkbCloseKeyboard();
  } /* drawClose */



void drawInit (void)

  { /* drawInit */
    logFunction(printf("drawInit()\n"););
    EM_ASM({
      mapWindowToId = new Map();
      mapCanvasToId = new Map();
    });
    emptyWindow = generateEmptyWindow();
    gkbInitKeyboard();
    os_atexit(drawShut);
    logFunction(printf("drawInit -->\n"););
  } /* drawInit */



intType getWindowLeftPos (const const_winType aWindow)

  {
    int xPos;

  /* getWindowLeftPos */
    logFunction(printf("getWindowLeftPos(" FMT_U_MEM ")\n",
                       (memSizeType) aWindow););
    if (is_pixmap(aWindow)) {
      logError(printf("getWindowLeftPos(" FMT_U_MEM "): Window is pixmap\n",
                      (memSizeType) aWindow););
      raise_error(RANGE_ERROR);
      xPos = 0;
    } else if (is_tab(aWindow)) {
      xPos = EM_ASM_INT({
        if (typeof document.scrollingElement != "undefined") {
          return -document.scrollingElement.scrollLeft;
        } else {
          return 0;
        }
      });
    } else if (is_subwindow(aWindow)) {
      xPos = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
          let left = mapIdToCanvas[$0].style.left;
          if (left.endsWith("px")) {
            return left.substring(0, left.length - 2);
          } else {
            return left;
          }
        } else {
          return -2147483648;
        }
      }, to_window(aWindow));
      if (unlikely(xPos == -2147483648)) {
        logError(printf("getWindowLeftPos(" FMT_U_MEM "): windowId not found: %d\n",
                        (memSizeType) aWindow, to_window(aWindow)););
        raise_error(GRAPHIC_ERROR);
        xPos = 0;
      } else  {
        xPos += getWindowLeftPos(to_parentWindow(aWindow));
      } /* if */
    } else {
      xPos = 0;
    } /* if */
    logFunction(printf("getWindowLeftPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) aWindow, xPos););
    return (intType) xPos;
  } /* getWindowLeftPos */



intType getWindowTopPos (const const_winType aWindow)

  {
    int yPos;

  /* getWindowTopPos */
    logFunction(printf("getWindowTopPos(" FMT_U_MEM ")\n",
                       (memSizeType) aWindow););
    if (is_pixmap(aWindow)) {
      logError(printf("getWindowTopPos(" FMT_U_MEM "): Window is pixmap\n",
                      (memSizeType) aWindow););
      raise_error(RANGE_ERROR);
      yPos = 0;
    } else if (is_tab(aWindow)) {
      yPos = EM_ASM_INT({
        if (typeof document.scrollingElement != "undefined") {
          return -document.scrollingElement.scrollTop;
        } else {
          return 0;
        }
      });
    } else if (is_subwindow(aWindow)) {
      yPos = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
          let top = mapIdToCanvas[$0].style.top;
          if (top.endsWith("px")) {
            return top.substring(0, top.length - 2);
          } else {
            return top;
          }
        } else {
          return -2147483648;
        }
      }, to_window(aWindow));
      if (unlikely(yPos == -2147483648)) {
        logError(printf("getWindowTopPos(" FMT_U_MEM "): windowId not found: %d\n",
                        (memSizeType) aWindow, to_window(aWindow)););
        raise_error(GRAPHIC_ERROR);
        yPos = 0;
      } else  {
        yPos += getWindowTopPos(to_parentWindow(aWindow));
      } /* if */
    } else {
      yPos = 0;
    } /* if */
    logFunction(printf("getWindowTopPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) aWindow, yPos););
    return (intType) yPos;
  } /* getWindowTopPos */



intType drwPointerXpos (const_winType actual_window)

  {
    intType xPos;

  /* drwPointerXpos */
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    xPos = pointerX - getWindowLeftPos(actual_window);
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, xPos););
    return xPos;
  } /* drwPointerXpos */



intType drwPointerYpos (const_winType actual_window)

  {
    intType yPos;

  /* drwPointerYpos */
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    yPos = pointerY - getWindowTopPos(actual_window);
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, yPos););
    return yPos;
  } /* drwPointerYpos */



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    double startAng;
    double endAng;
    int successInfo;

  /* drwPArc */
    logFunction(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (sweepAngle < 0.0) {
      startAngle += sweepAngle;
      sweepAngle = -sweepAngle;
    } /* if */
    startAng = (2 * PI) - startAngle - sweepAngle;
    endAng = (2 * PI) - startAngle;
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX ||
                 os_isnan(startAngle) || os_isnan(sweepAngle) ||
                 startAng == POSITIVE_INFINITY ||
                 startAng == NEGATIVE_INFINITY ||
                 endAng == POSITIVE_INFINITY ||
                 endAng == NEGATIVE_INFINITY)) {
      logError(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", %.4f, %.4f, " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.lineWidth=1;
          context.strokeStyle = "#" + ("000000" + $6.toString(16)).slice(-6);
          context.beginPath();
          context.arc($1, $2, $3, $4, $5);
          context.stroke();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y), (int) (radius),
          startAng, endAng, (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                        ", %.4f, %.4f, " F_X(08) "): windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius,
                        startAngle, sweepAngle, col, to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPArc */



void drwPFArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle,
    intType width, intType col)

  {
    double startAng;
    double endAng;
    int successInfo;

  /* drwPFArc */
    logFunction(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, width, col););
    if (sweepAngle < 0.0) {
      startAngle += sweepAngle;
      sweepAngle = -sweepAngle;
    } /* if */
    startAng = (2 * PI) - startAngle - sweepAngle;
    endAng = (2 * PI) - startAngle;
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX ||
                 width < 1 || width > radius ||
                 os_isnan(startAngle) || os_isnan(sweepAngle) ||
                 startAng == POSITIVE_INFINITY ||
                 startAng == NEGATIVE_INFINITY ||
                 endAng == POSITIVE_INFINITY ||
                 endAng == NEGATIVE_INFINITY)) {
      logError(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", %.4f, %.4f, " FMT_D ", " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, width, col););
      raise_error(RANGE_ERROR);
    } else {
      if ((width & 1) != 0) {
        radius -= width / 2;
      } else {
        radius -= width / 2 - 1;
      } /* if */
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.lineWidth=$6;
          context.strokeStyle = "#" + ("000000" + $8.toString(16)).slice(-6);
          context.beginPath();
          if ($7) {
            context.arc($1, $2, $3 - 0.5, $4, $5);
          } else {
            context.arc($1, $2, $3, $4, $5);
          }
          context.stroke();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y), (int) (radius),
          startAng, endAng, (int) (width), (width & 1) != 0,
          (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                        ", %.4f, %.4f, " FMT_D ", " F_X(08) "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius,
                        startAngle, sweepAngle, width, col,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPFArc */



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    double startAng;
    double endAng;
    int successInfo;

  /* drwPFArcChord */
    logFunction(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (sweepAngle < 0.0) {
      startAngle += sweepAngle;
      sweepAngle = -sweepAngle;
    } /* if */
    startAng = (2 * PI) - startAngle - sweepAngle;
    endAng = (2 * PI) - startAngle;
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX ||
                 os_isnan(startAngle) || os_isnan(sweepAngle) ||
                 startAng == POSITIVE_INFINITY ||
                 startAng == NEGATIVE_INFINITY ||
                 endAng == POSITIVE_INFINITY ||
                 endAng == NEGATIVE_INFINITY)) {
      logError(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", %.4f, %.4f, " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.fillStyle = "#" + ("000000" + $6.toString(16)).slice(-6);
          context.beginPath();
          context.arc($1, $2, $3, $4, $5);
          context.lineTo($1 + Math.cos($4) * $3, $2 + Math.sin($4) * $3);
          context.fill();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y), (int) (radius),
          startAng, endAng, (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                        ", " FMT_D ", %.4f, %.4f, " F_X(08) "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius,
                        startAngle, sweepAngle, col,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPFArcChord */



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    double startAng;
    double endAng;
    int successInfo;

  /* drwPFArcPieSlice */
    logFunction(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (sweepAngle < 0.0) {
      startAngle += sweepAngle;
      sweepAngle = -sweepAngle;
    } /* if */
    startAng = (2 * PI) - startAngle - sweepAngle;
    endAng = (2 * PI) - startAngle;
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX ||
                 os_isnan(startAngle) || os_isnan(sweepAngle) ||
                 startAng == POSITIVE_INFINITY ||
                 startAng == NEGATIVE_INFINITY ||
                 endAng == POSITIVE_INFINITY ||
                 endAng == NEGATIVE_INFINITY)) {
      logError(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", %.4f, %.4f, " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.fillStyle = "#" + ("000000" + $6.toString(16)).slice(-6);
          context.beginPath();
          context.moveTo($1, $2);
          context.arc($1, $2, $3, $4, $5);
          context.lineTo($1, $2);
          context.fill();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y), (int) (radius),
          startAng, endAng, (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                        ", " FMT_D ", %.4f, %.4f, " F_X(08) "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius,
                        startAngle, sweepAngle, col,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPFArcPieSlice */



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
      logError(printf("drwBorder(" FMT_U_MEM "): Window is pixmap\n",
                      (memSizeType) actual_window););
      raise_error(RANGE_ERROR);
      border = NULL;
    } else {
      twoBorders = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
          let windowObject = mapIdToWindow[$0];
          let rightBottomLeftBorder = windowObject.outerWidth - windowObject.innerWidth;
          let topBorder = windowObject.outerHeight - windowObject.innerHeight - rightBottomLeftBorder;
          if (rightBottomLeftBorder <= 32767 && topBorder <= 65535) {
            return rightBottomLeftBorder << 16 | topBorder;
          } else {
            return -1;
          }
        } else {
          return -1;
        }
      }, to_window(actual_window));
      if (unlikely(twoBorders == -1)) {
        logError(printf("drwBorder(" FMT_U_MEM "): "
                        "Border too large or windowId not found: %d\n",
                        (memSizeType) actual_window,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
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



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  {
    int successInfo;

  /* drwPCircle */
    logFunction(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX)) {
      logError(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius, col););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.lineWidth=1;
          context.strokeStyle = "#" + ("000000" + $4.toString(16)).slice(-6);
          context.beginPath();
          context.arc($1, $2, $3, 0, 2 * Math.PI);
          context.stroke();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y),
          (int) (radius), (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                        ", " FMT_D ", " F_X(08) "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius, col,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPCircle */



void drwClear (winType actual_window, intType col)

  {
    int successInfo;

  /* drwClear */
    logFunction(printf("drwClear(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, col););
    to_var_clear_col(actual_window) = col;
    successInfo = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        context.fillStyle = "#" + ("000000" + $1.toString(16)).slice(-6);
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);
        return 0;
      } else {
        return 1;
      }
    }, to_window(actual_window), (int) (col & 0xffffff));
    if (unlikely(successInfo != 0)) {
      logError(printf("drwClear(" FMT_U_MEM ", " F_X(08) "): "
                      "windowId not found: %d\n",
                      (memSizeType) actual_window, col,
                      to_window(actual_window)););
      raise_error(GRAPHIC_ERROR);
    } /* if */
  } /* drwClear */



void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y)

  {
    int successInfo;

  /* drwCopyArea */
    logFunction(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                       ", " FMT_D ")\n",
                       (memSizeType) src_window, (memSizeType) dest_window,
                       src_x, src_y, width, height, dest_x, dest_y););
    if (unlikely(!inIntRange(src_x) || !inIntRange(src_y) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX ||
                 !inIntRange(dest_x) || !inIntRange(dest_y))) {
      logError(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                      FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                      ", " FMT_D "): Raises RANGE_ERROR\n",
                      (memSizeType) src_window, (memSizeType) dest_window,
                      src_x, src_y, width, height, dest_x, dest_y););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined" &&
            typeof mapIdToContext[$1] !== "undefined") {
          let sourceContext = mapIdToContext[$0];
          let destContext = mapIdToContext[$1];
          let imageData = sourceContext.getImageData($2, $3, $4, $5);
          destContext.putImageData(imageData, $6, $7);
          return 0;
        } else {
          return 1;
        }
      }, to_window(src_window), to_window(dest_window), (int) (src_x), (int) (src_y),
        (int) (width), (int) (height), (int) (dest_x), (int) (dest_y));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                        FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                        ", " FMT_D "): windowId not found: %d or %d\n",
                        (memSizeType) src_window, (memSizeType) dest_window,
                        src_x, src_y, width, height, dest_x, dest_y,
                        to_window(src_window), to_window(dest_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwCopyArea */



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  {
    int successInfo;

  /* drwPFCircle */
    logFunction(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 radius <= 0 || radius > INT_MAX)) {
      logError(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius, col););
      raise_error(RANGE_ERROR);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let context = mapIdToContext[$0];
          context.fillStyle = "#" + ("000000" + $4.toString(16)).slice(-6);
          context.beginPath();
          context.arc($1, $2, $3, 0, 2 * Math.PI);
          context.fill();
          return 0;
        } else {
          return 1;
        }
      }, to_window(actual_window), (int) (x), (int) (y),
          (int) (radius), (int) (col & 0xffffff));
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                        ", " FMT_D ", " F_X(08) "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, x, y, radius, col,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPFCircle */



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  {
    int successInfo;

  /* drwPFEllipse */
    logFunction(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       width, height, col););
    successInfo = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        context.fillStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
        context.beginPath();
        context.ellipse($1, $2, $3 / 2, $4 / 2, 0, 0, 2 * Math.PI);
        context.fill();
        return 0;
      } else {
        return 1;
      }
    }, to_window(actual_window), castToInt(x + width / 2), castToInt(y + height / 2),
        castToInt(width), castToInt(height), (int) (col & 0xffffff));
    if (unlikely(successInfo != 0)) {
      logError(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " FMT_D ", " F_X(08) "): "
                      "windowId not found: %d\n",
                      (memSizeType) actual_window, x, y, width, height, col,
                      to_window(actual_window)););
      raise_error(GRAPHIC_ERROR);
    } /* if */
  } /* drwPFEllipse */



void drwFlush (void)

  { /* drwFlush */
  } /* drwFlush */



winType drwEmpty (void)

  { /* drwEmpty */
    logFunction(printf("drwEmpty --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) emptyWindow,
                       emptyWindow != NULL ? emptyWindow->usage_count : (uintType) 0););
    return emptyWindow;
  } /* drwEmpty */



static void closeWindow (int windowId)

  { /* closeWindow */
    logFunction(printf("closeWindow(%d)\n", windowId););
    EM_ASM({
      if (typeof window !== "undefined") {
        if (typeof mapIdToCanvas[$0] !== "undefined") {
          let canvas = mapIdToCanvas[$0];
          mapCanvasToId.delete(canvas);
          mapIdToCanvas[$0] = undefined;
          mapIdToContext[$0] = undefined;
          let parent = canvas.parentNode;
          parent.removeChild(canvas);
        }
        if (typeof mapIdToWindow[$0] !== "undefined") {
          let windowObject = mapIdToWindow[$0];
          mapWindowToId.delete(windowObject);
          mapIdToWindow[$0] = undefined;
          if (deregisterWindowFunction !== null) {
            deregisterWindowFunction(windowObject);
          }
          windowObject.close();
        }
      }
    }, windowId);
    logFunction(printf("closeWindow(%d) -->\n", windowId););
  } /* closeWindow */



void drwFree (winType old_window)

  { /* drwFree */
    logFunction(printf("drwFree(" FMT_U_MEM ") (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) old_window,
                       old_window != NULL ? to_window(old_window) : 0,
                       old_window != NULL ? old_window->usage_count : (uintType) 0););
    if (is_pixmap(old_window)) {
      EM_ASM({
        mapIdToCanvas[$0] = undefined;
        mapIdToContext[$0] = undefined;
      }, to_window(old_window));
    } else {
      closeWindow(to_window(old_window));
      remove_window(to_window(old_window));
      setClosePopupState(to_window(old_window), 0);
    } /* if */
    FREE_RECORD2(old_window, emc_winRecord, count.win, count.win_bytes);
    logFunction(printf("drwFree(" FMT_U_MEM ") -->\n",
                       (memSizeType) old_window););
  } /* drwFree */



winType drwCapture (intType left, intType upper,
    intType width, intType height)

  { /* drwCapture */
    return NULL;
  } /* drwCapture */



intType drwGetPixel (const_winType sourceWindow, intType x, intType y)

  {
    int col;

  /* drwGetPixel */
    logFunction(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, x, y););
    col = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        let canvasColor = context.getImageData(x, y, 1, 1).data; // rgba e [0,255]
        let r = canvasColor[0];
        let g = canvasColor[1];
        let b = canvasColor[2];
        return canvasColor[0] << 16 | canvasColor[1] << 8 | canvasColor[2]
      } else {
        return -1;
      }
    }, to_window(sourceWindow), castToInt(x), castToInt(y));
    if (unlikely(col == -1)) {
      logFunction(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                         "windowId not found: %d\n",
                         (memSizeType) sourceWindow, x, y,
                         to_window(sourceWindow)););
      raise_error(GRAPHIC_ERROR);
      col = 0;
    } /* if */
    logFunction(printf("drwGetPixel --> " F_X(08) "\n", (intType) col););
    return (intType) col;
  } /* drwGetPixel */



bstriType drwGetPixelData (const_winType sourceWindow)

  {
    bstriType result;

  /* drwGetPixelData */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
      raise_error(RANGE_ERROR);
    } else {
      result->size = 0;
    } /* if */
    return result;
  } /* drwGetPixelData */



/**
 *  Create a new pixmap with the given 'width' and 'height' from 'sourceWindow'.
 *  A rectangle with the upper left corner at (left, upper) and the given
 *  'width' and 'height' is copied from 'sourceWindow' to the new pixmap.
 *  @return the created pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative or zero.
 */
winType drwGetPixmap (const_winType sourceWindow, intType left, intType upper,
    intType width, intType height)

  {
    int windowId;
    emc_winType pixmap = NULL;

  /* drwGetPixmap */
    logFunction(printf("drwGetPixmap(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, left, upper,
                       width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwGetPixmap(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " FMT_D "): Raises RANGE_ERROR\n",
                      (memSizeType) sourceWindow, left, upper,
                      width, height););
      raise_error(RANGE_ERROR);
    } else {

      windowId = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
          let sourceContext = mapIdToContext[$0];
          let canvas = document.createElement("canvas");
          canvas.width = $3;
          canvas.height = $4;
          let context = canvas.getContext("2d");
          context.putImageData(sourceContext.getImageData($1, $2, $3, $4), 0, 0);
          currentWindowId++;
          mapIdToCanvas[currentWindowId] = canvas;
          mapIdToContext[currentWindowId] = context;
          return currentWindowId;
        } else {
          return 0;
        }
      }, to_window(sourceWindow), (int) (left), (int) (upper),
          (int) (width), (int) (height));

      if (unlikely(windowId == 0)) {
        logError(printf("drwGetPixmap(" FMT_U_MEM ", " FMT_D ", " FMT_D
                        ", " FMT_D ", " FMT_D "): Failed to get pixmap.\n",
                        (memSizeType) sourceWindow, left, upper,
                        width, height););
        raise_error(GRAPHIC_ERROR);
      } else if (unlikely(!ALLOC_RECORD2(pixmap, emc_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(emc_winRecord));
        pixmap->usage_count = 1;
        pixmap->window = windowId;
        pixmap->is_pixmap = TRUE;
        pixmap->is_subwindow = FALSE;
        pixmap->is_substitute = FALSE;
        pixmap->is_tab = FALSE;
        pixmap->parentWindow = NULL;
        pixmap->ignoreFirstResize = 0;
        pixmap->creationTimestamp = 0;
        pixmap->width = (int) width;
        pixmap->height = (int) height;
        maxWindowId = pixmap->window;
      } /* if */
    } /* if */
    logFunction(printf("drwGetPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwGetPixmap */



intType drwHeight (const_winType actual_window)

  {
    int height;

  /* drwHeight */
    logFunction(printf("drwHeight(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    height = to_height(actual_window);
    logFunction(printf("drwHeight(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, height););
    return (intType) height;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height,
    boolType hasAlphaChannel)

  {
    int windowId;
    emc_winType pixmap = NULL;

  /* drwImage */
    logFunction(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       width, height, hasAlphaChannel););
    if (unlikely(width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX ||
                 (unsigned int) height >
                     MAX_MEMSIZETYPE / (unsigned int) width / 4)) {
      logError(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ", %d): "
                      "Raises RANGE_ERROR\n",
                      width, height, hasAlphaChannel););
      raise_error(RANGE_ERROR);
    } else {

      windowId = EM_ASM_INT({
        if (typeof window !== "undefined") {
          let width = $1;
          let height = $2;
          let canvas = document.createElement("canvas");
          canvas.width = width;
          canvas.height = height;
          let context = canvas.getContext("2d");
          let imageData = context.createImageData(width, height);
          let data = imageData.data;
          let len = width * height * 4;
          // copy img byte-per-byte into our ImageData
          if ($3) {
            for (let i = 0; i < len; i += 4) {
              data[i] = HEAPU8[$0 + i + 2];
              data[i + 1] = HEAPU8[$0 + i + 1];
              data[i + 2] = HEAPU8[$0 + i];
              data[i + 3] = HEAPU8[$0 + i + 3];
            }
          } else {
            for (let i = 0; i < len; i += 4) {
              data[i] = HEAPU8[$0 + i + 2];
              data[i + 1] = HEAPU8[$0 + i + 1];
              data[i + 2] = HEAPU8[$0 + i];
              data[i + 3] = 0xff;
            }
          }
          context.putImageData(imageData, 0, 0);
          currentWindowId++;
          mapIdToCanvas[currentWindowId] = canvas;
          mapIdToContext[currentWindowId] = context;
          return currentWindowId;
        } else {
          return 0;
        }
      }, (int *) image_data, (int) width, (int) height, (int) hasAlphaChannel);

      if (unlikely(windowId == 0)) {
        logError(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ", %d): "
                        "Failed to create pixmap.\n",
                        width, height, hasAlphaChannel););
        raise_error(GRAPHIC_ERROR);
      } else if (unlikely(!ALLOC_RECORD2(pixmap, emc_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(emc_winRecord));
        pixmap->usage_count = 1;
        pixmap->window = windowId;
        pixmap->is_pixmap = TRUE;
        pixmap->is_subwindow = FALSE;
        pixmap->is_substitute = FALSE;
        pixmap->is_tab = FALSE;
        pixmap->parentWindow = NULL;
        pixmap->ignoreFirstResize = 0;
        pixmap->creationTimestamp = 0;
        pixmap->width = (int) width;
        pixmap->height = (int) height;
        maxWindowId = pixmap->window;
      } /* if */
    } /* if */
    logFunction(printf("drwImage --> " FMT_U_MEM " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->window : 0,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwImage */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  {
    int successInfo;

  /* drwPLine */
    logFunction(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x1, y1,
                       x2, y2, col););
    successInfo = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        context.lineWidth=1;
        context.strokeStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
        context.beginPath();
        context.moveTo($1, $2);
        context.lineTo($3, $4);
        context.stroke();
        return 0;
      } else {
        return 1;
      }
    }, to_window(actual_window), castToInt(x1), castToInt(y1),
        castToInt(x2), castToInt(y2), (int) (col & 0xffffff));
    if (unlikely(successInfo != 0)) {
      logError(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) "): "
                      "windowId not found: %d\n",
                      (memSizeType) actual_window, x1, y1, x2, y2, col,
                      to_window(actual_window)););
      raise_error(GRAPHIC_ERROR);
    } /* if */
  } /* drwPLine */



/**
 *  Create a new pixmap with the given 'width' and 'height'.
 *  @return the created pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative or zero.
 */
winType drwNewPixmap (intType width, intType height)

  {
    int windowId;
    emc_winType pixmap = NULL;

  /* drwNewPixmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n",
                       width, height););
    if (unlikely(width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwNewPixmap(" FMT_D ", " FMT_D "): "
                      "Raises RANGE_ERROR\n",
                      width, height););
      raise_error(RANGE_ERROR);
    } else {

      windowId = EM_ASM_INT({
        if (typeof window !== "undefined") {
          let width = $0;
          let height = $1;
          let canvas = document.createElement("canvas");
          canvas.width = width;
          canvas.height = height;
          let context = canvas.getContext("2d");
          currentWindowId++;
          mapIdToCanvas[currentWindowId] = canvas;
          mapIdToContext[currentWindowId] = context;
          return currentWindowId;
        } else {
          return 0;
        }
      }, (int) width, (int) height);

      if (unlikely(windowId == 0)) {
        logError(printf("drwNewPixmap(" FMT_D ", " FMT_D "): "
                        "Failed to create new pixmap.\n",
                        width, height););
        raise_error(GRAPHIC_ERROR);
      } else if (unlikely(!ALLOC_RECORD2(pixmap, emc_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(emc_winRecord));
        pixmap->usage_count = 1;
        pixmap->window = windowId;
        pixmap->is_pixmap = TRUE;
        pixmap->is_subwindow = FALSE;
        pixmap->is_substitute = FALSE;
        pixmap->is_tab = FALSE;
        pixmap->parentWindow = NULL;
        pixmap->ignoreFirstResize = 0;
        pixmap->creationTimestamp = 0;
        pixmap->width = (int) width;
        pixmap->height = (int) height;
        maxWindowId = pixmap->window;
      } /* if */
    } /* if */
    logFunction(printf("drwNewPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwNewPixmap */



static void moveCanvas (int sourceWindowId, int destWindowId)

  { /* moveCanvas */
    logFunction(printf("moveCanvas(%d, %d)\n",
                       sourceWindowId, destWindowId););
    EM_ASM({
      let sourceWindow = mapIdToWindow[$0];
      let destWindow = mapIdToWindow[$1];
      let children = sourceWindow.document.body.children;
      while (children.length > 0) {
        destWindow.document.body.appendChild(children[0]);
      }
    }, sourceWindowId, destWindowId);
    logFunction(printf("moveCanvas(%d, %d) -->\n",
                       sourceWindowId, destWindowId););
  } /* moveCanvas */



static void removeWindowMapping (int windowId)

  { /* removeWindowMapping */
    logFunction(printf("removeWindowMapping(%d)\n", windowId););
    EM_ASM({
      if (typeof window !== "undefined") {
        mapIdToCanvas[$0] = undefined;
        mapIdToContext[$0] = undefined;
        mapIdToWindow[$0] = undefined;
      }
    }, windowId);
    logFunction(printf("removeWindowMapping(%d) -->\n", windowId););
  } /* removeWindowMapping */



int copyWindow (int windowId)

  {
    int windowIdAndFlags;
    emc_winType aWindow;

  /* copyWindow */
    logFunction(printf("copyWindow(%d)\n", windowId););
    aWindow = (emc_winType) find_window(windowId);
    if (unlikely(aWindow == NULL)) {
      logError(printf("copyWindow(%d): Cannot find old window.\n",
                      windowId););
    } else {
      windowIdAndFlags = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined" &&
                                             typeof mapIdToCanvas[$0] !== "undefined") {
          let sourceWindow = mapIdToWindow[$0];
          let sourceCanvas = mapIdToCanvas[$0];
          let sourceContext = mapIdToContext[$0];
          let rightBottomLeftBorder = sourceWindow.outerWidth - sourceWindow.innerWidth;
          let topBorder = sourceWindow.outerHeight - sourceWindow.innerHeight - rightBottomLeftBorder;
          let left = sourceWindow.screenX;
          let top = sourceWindow.screenY;
          let width = $1;
          let height = $2;
          let sourceWindowTitle = sourceWindow.document.title;
          // The source window will be closed and nothing can stop this.
          if (deregisterWindowFunction !== null) {
            deregisterWindowFunction(sourceWindow);
          }
          // The new window name must differ from the original window name.
          let windowName = sourceWindow.name;
          if (windowName.endsWith("++")) {
            windowName = windowName.substring(0, windowName.length - 2);
          } else {
            windowName = windowName + "+";
          }
          let windowFeatures = "popup=true,left=" + left + ",top=" + top +
                               ",width=" + width + ",height=" + height;
          let windowObject = window.open("", windowName, windowFeatures);
          if (windowObject === null) {
            return 0;
          } else {
            const title = windowObject.document.createElement("title");
            const titleText = windowObject.document.createTextNode(sourceWindowTitle);
            title.appendChild(titleText);
            windowObject.document.head.appendChild(title);
            windowObject.document.body.style.margin = 0;
            windowObject.document.body.style.overflowX = "hidden";
            windowObject.document.body.style.overflowY = "hidden";
            currentWindowId++;
            mapIdToWindow[currentWindowId] = windowObject;
            mapWindowToId.set(windowObject, currentWindowId);
            let ignoreFirstResize = 0;
            if (windowObject.innerWidth === 0 || windowObject.innerHeight === 0) {
              ignoreFirstResize = 1;
            } else {
              windowObject.resizeTo(width + (windowObject.outerWidth - windowObject.innerWidth),
                                    height + (windowObject.outerHeight - windowObject.innerHeight));
              if (windowObject.screenLeft === 0 && windowObject.screenTop === 0) {
                ignoreFirstResize = 2;
              }
              windowObject.moveTo(left, top);
            }
            mapIdToCanvas[currentWindowId] = sourceCanvas;
            mapCanvasToId.set(sourceCanvas, currentWindowId);
            mapIdToContext[currentWindowId] = sourceContext;
            if (typeof windowObject.opener.registerWindow !== "undefined") {
              windowObject.opener.registerWindow(windowObject);
            }
            return (currentWindowId << 2) | ignoreFirstResize;
          }
        } else {
          return 0;
        }
      }, windowId, aWindow->width, aWindow->height);
      if (unlikely(windowIdAndFlags == 0)) {
        logError(printf("copyWindow(%d): Failed to open window.\n",
                        windowId););
        aWindow = NULL;
      } else {
        aWindow->window = windowIdAndFlags >> 2;
        aWindow->ignoreFirstResize = windowIdAndFlags & 3;
        aWindow->creationTimestamp = timMicroSec() / 1000000;
        maxWindowId = aWindow->window;
        moveCanvas(windowId, aWindow->window);
        setupEventCallbacksForWindow(aWindow->window);
        remove_window(windowId);
        setClosePopupState(windowId, 0);
        enter_window((winType) aWindow, aWindow->window);
        removeWindowMapping(windowId);
      } /* if */
    } /* if */
    logFunction(printf("copyWindow --> " FMT_U_MEM " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) aWindow,
                       aWindow != NULL ? aWindow->window : 0,
                       aWindow != NULL ? aWindow->usage_count : (uintType) 0););
    return aWindow != NULL ? aWindow->window : 0;
  } /* copyWindow */



static winType openSubstituteWindow (intType xPos, intType yPos,
    intType width, intType height)

  {
    int windowId;
    winType window;
    winType parentWindow = NULL;
    intType parentWinXPos;
    intType parentWinYPos;
    emc_winType newWindow;

  /* openSubstituteWindow */
    logFunction(printf("openSubstituteWindow(" FMT_D ", " FMT_D ", "
                       FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    for (windowId = 1; windowId <= maxWindowId && parentWindow == NULL; windowId++) {
      window = find_window(windowId);
      if (window != NULL) {
        parentWinXPos = drwXPos(window);
        parentWinYPos = drwYPos(window);
        if (parentWinXPos <= xPos && parentWinYPos <= yPos &&
            parentWinXPos + to_width(window) >= xPos + width &&
            parentWinYPos + to_height(window) >= yPos + height) {
          parentWindow = window;
        } /* if */
      } /* if */
    } /* for */
    if (unlikely(parentWindow == NULL)) {
      logError(printf("openSubstituteWindow(" FMT_D ", " FMT_D ", "
                      FMT_D ", " FMT_D "): Cannot find parent window.\n",
                      xPos, yPos, width, height););
      raise_error(GRAPHIC_ERROR);
      newWindow = NULL;
    } else {
      newWindow = (emc_winType) drwOpenSubWindow(parentWindow,
          xPos - parentWinXPos, yPos - parentWinYPos, width, height);
      if (newWindow != NULL) {
        newWindow->is_substitute = TRUE;
      } /* if */
    } /* if */
    logFunction(printf("openSubstituteWindow --> " FMT_U_MEM
                       " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->window : 0,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
  } /* openSubstituteWindow */



static char *getNameFromTitle (const char *winTitle, memSizeType *winNameSize)

  {
    const char *startPos;
    memSizeType nameSize;
    char *winName;

  /* getNameFromTitle */
    logFunction(printf("getNameFromTitle(\"%s\", *)\n", winTitle););
    while (*winTitle == '_') {
      winTitle++;
    } /* while */
    startPos = winTitle;
    while (*winTitle != ' ' && *winTitle != '\0') {
      winTitle++;
    } /* while */
    nameSize = (memSizeType) (winTitle - startPos) + 16;
    *winNameSize = nameSize;
    if (unlikely(!ALLOC_CSTRI(winName, nameSize))) {
      logError(printf("getNameFromTitle(\"%s\"): malloc(" FMT_U_MEM ") failed\n",
                      winTitle, nameSize););
    } else {
      memcpy(winName, startPos, nameSize - 16);
      sprintf(&winName[nameSize - 16], FMT_X64, uintRand());
    } /* if */
    logFunction(printf("getNameFromTitle(\"%s\", " FMT_U_MEM ") --> \"%s\"\n",
                        winTitle, *winNameSize,
                        winName == NULL ? "\\ *NULL* \\" : winName););
    return winName;
  } /* getNameFromTitle */



static int openDocumentTabAsWindow (intType width, intType height)

  {
    int windowIdAndFlags;

  /* openDocumentTabAsWindow */
    logFunction(printf("openDocumentTabAsWindow(" FMT_D ", " FMT_D ")\n",
                       width, height););
    windowIdAndFlags = EM_ASM_INT({
      let width = $0;
      let height = $1;
      currentWindowId++;
      mapIdToWindow[currentWindowId] = document;
      mapWindowToId.set(null, currentWindowId);
      let canvas = document.createElement("canvas");
      canvas.style.position = "absolute";
      canvas.style.left = "0px";
      canvas.style.top = "0px";
      let ignoreFirstResize = 0;
      canvas.width  = width;
      canvas.height = height;
      let context = canvas.getContext("2d");
      context.fillStyle = "#000000";
      context.fillRect(0, 0, width, height);
      document.getElementsByTagName("body")[0].innerHTML = "";
      document.body.appendChild(canvas);
      mapIdToCanvas[currentWindowId] = canvas;
      mapCanvasToId.set(canvas, currentWindowId);
      mapIdToContext[currentWindowId] = context;
      let script = document.createElement("script");
      script.setAttribute("type", "text/javascript");
      script.text = "function reloadPage() {\n" +
                    "    setTimeout(function() {\n" +
                    "        location.reload();\n" +
                    "    }, 250);\n" +
                    "}\n";
      document.body.appendChild(script);
      if (reloadPageFunction === null) {
          reloadPageFunction = reloadPage;
      }
      return (currentWindowId << 3) | ignoreFirstResize | 4;
    }, (int) width, (int) height);
    logFunction(printf("openDocumentTabAsWindow(" FMT_D ", " FMT_D ") --> %d\n",
                       width, height, windowIdAndFlags););
    return windowIdAndFlags;
  } /* openDocumentTabAsWindow */



static void moveSubWindowsToDocumentTab (winType sourceWindow,
    int destWindowId)

  { /* moveSubWindowsToDocumentTab */
    logFunction(printf("moveSubWindowsToDocumentTab(" FMT_U_MEM
                       " (windowId=%d), %d)\n",
                       (memSizeType) sourceWindow,
                       to_window(sourceWindow), destWindowId););
    EM_ASM({
      let sourceWindow = mapIdToWindow[$0];
      let sourceCanvas = mapIdToCanvas[$0];
      let destWindow = mapIdToWindow[$1];
      let destCanvas = mapIdToCanvas[$1];
      let addAfterMainCanvas = 0;
      let children = sourceWindow.document.body.children;
      for (let i = children.length; i > 0; i--) {
        let canvas = children[addAfterMainCanvas];
        if (canvas === sourceCanvas) {
          addAfterMainCanvas = 1;
        } else {
          if (mapCanvasToId.has(canvas)) {
            if (addAfterMainCanvas) {
              destWindow.body.appendChild(canvas);
            } else {
              destWindow.body.insertBefore(canvas, destCanvas);
            }
          }
        }
      }
    }, to_window(sourceWindow), destWindowId);
    logFunction(printf("moveSubWindowsToDocumentTab(" FMT_U_MEM
                       " (windowId=%d), %d) -->\n",
                       (memSizeType) sourceWindow,
                       to_window(sourceWindow), destWindowId););
  } /* moveSubWindowsToDocumentTab */



void moveWindowToDocumentTab (winType currentWindow)

  {
    int windowIdAndFlags;
    int windowId;

  /* moveWindowToDocumentTab */
    logFunction(printf("moveWindowToDocumentTab(" FMT_U_MEM " (windowId=%d))\n",
                       (memSizeType) currentWindow,
                       to_window(currentWindow)););
    windowIdAndFlags = openDocumentTabAsWindow(
        (intType) to_width(currentWindow),
        (intType) to_height(currentWindow));
    if (unlikely(windowIdAndFlags == 0)) {
      raise_error(GRAPHIC_ERROR);
    } else {
      EM_ASM({
        let sourceWindow = mapIdToWindow[$0];
        let sourceCanvas = mapIdToCanvas[$0];
        let destContext = mapIdToContext[$1];
        destContext.drawImage(sourceCanvas, 0, 0);
      }, to_window(currentWindow), windowIdAndFlags >> 3);
      windowId = windowIdAndFlags >> 3;
      moveSubWindowsToDocumentTab(currentWindow, windowId);
      is_var_tab(currentWindow) = TRUE;
      openSubstitute = TRUE;
      maxWindowId = windowId;
      setupEventCallbacksForWindow(windowId);
      remove_window(to_window(currentWindow));
      setClosePopupState(to_window(currentWindow), 0);
      enter_window(currentWindow, windowId);
      closeWindow(to_window(currentWindow));
      to_var_window(currentWindow) = windowId;
    } /* if */
    logFunction(printf("moveWindowToDocumentTab(" FMT_U_MEM " (windowId=%d)) -->\n",
                       (memSizeType) currentWindow,
                       to_window(currentWindow)););
  } /* moveWindowToDocumentTab */



boolType windowIsInNewTab (winType currentWindow)

  {
    boolType isInNewTab;

  /* windowIsInNewTab */
    logFunction(printf("windowIsInNewTab(" FMT_U_MEM " (windowId=%d))\n",
                       (memSizeType) currentWindow,
                       to_window(currentWindow)););
    isInNewTab = EM_ASM_INT({
      let windowObject = mapIdToWindow[$0];
      if ((windowObject.visualViewport !== null &&
           windowObject.visualViewport.scale !== 1) ||
          windowObject.toolbar.visible ||
          windowObject.menubar.visible ||
          windowObject.statusbar.visible) {
        return 1;
      } else {
        return 0;
      }
    }, to_window(currentWindow));
    logFunction(printf("windowIsInNewTab(" FMT_U_MEM " (windowId=%d)) --> %d\n",
                       (memSizeType) currentWindow,
                       to_window(currentWindow), isInNewTab););
    return isInNewTab;
  } /* windowIsInNewTab */



/**
 *  Open a graphic window with the given parameters.
 *  Depending on the settings a browser may open a popup or a new tab.
 *  A browser might be configured to block all popups. There are also
 *  settings which allow popups only for specific sites. Even if all
 *  popups are blocked some browsers allow opening one popup but not a
 *  second one. There is a problem if a browser opens a tab instead of
 *  a popup: The script runs in the old tab and the new tab gets the
 *  focus. Without focus the script is slowed down and therefore time
 *  related functionality will not work. The function drwOpen() tries
 *  to discover if a tab has been opened instead of a popup. It uses
 *  heuristics for this purpose. If a tab has been opened the new opened
 *  tab is closed and the content of the current tab is used as window.
 *  To do that the current tab document is overwritten with a canvas.
 *  When the program exits this tab is reloaded and this way it gets the
 *  original content. When opening a second window the creation of a
 *  popup might be blocked by the browser. In this case a canvas is put
 *  on top of the first window. This is only possible if the second
 *  window is within the bounds of the first window. This canvas will
 *  hold the contents of the second window. When the second window is
 *  closed the canvas is removed.
 */
winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType windowName)

  {
    char *winTitle8 = NULL;
    char *winName8 = NULL;
    memSizeType winNameSize;
    int windowIdAndFlags;
    errInfoType err_info = OKAY_NO_ERROR;
    emc_winType result = NULL;

  /* drwOpen */
    logFunction(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                       ", \"%s\")\n", xPos, yPos, width, height,
                       striAsUnquotedCStri(windowName)););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                      ", \"%s\"): Illegal window dimensions\n",
                      xPos, yPos, width, height,
                      striAsUnquotedCStri(windowName)););
      err_info = RANGE_ERROR;
    } else {
      winTitle8 = stri_to_cstri8(windowName, &err_info);
      if (unlikely(winTitle8 == NULL)) {
        logError(printf("drwOpen: stri_to_cstri8(\"%s\") failed:\n"
                        "err_info=%d\n",
                        striAsUnquotedCStri(windowName), err_info););
      } else {
        winName8 = getNameFromTitle(winTitle8, &winNameSize);
        if (unlikely(winName8 == NULL)) {
          logError(printf("drwOpen: getNameFromTitle(\"%s\", *): failed\n",
                          winTitle8););
          err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } /* if */
    if (likely(err_info == OKAY_NO_ERROR)) {
      if (openSubstitute) {
        windowIdAndFlags = 0;
      } else {

        windowIdAndFlags = EM_ASM_INT({
          if (typeof window !== "undefined") {
            let left = $0;
            let top = $1;
            let width = $2;
            let height = $3;
            let windowName = Module.UTF8ToString($4);
            let windowTitle = Module.UTF8ToString($5);
            let firstWindowOpen = $6;
            let leftTopZero = 0;
            if (left === 0 && top === 0) {
              left = 1;
              top = 1;
              leftTopZero = 1;
            }
            let windowFeatures = "popup=true,left=" + left + ",top=" + top +
                                 ",width=" + width + ",height=" + height;
            let windowObject = window.open("", windowName, windowFeatures);
            if (windowObject !== null && firstWindowOpen) {
              if ((windowObject.visualViewport !== null &&
                   windowObject.visualViewport.scale !== 1) ||
                  windowObject.toolbar.visible ||
                  windowObject.menubar.visible ||
                  windowObject.statusbar.visible ||
                  (windowObject.screenX === 0 &&
                   windowObject.screenY === 0)) {
                windowObject.close();
                return 4;
              }
            }
            if (windowObject === null) {
              return 0;
            } else {
              if (leftTopZero) {
                windowObject.screenX = 0;
                windowObject.screenY = 0;
              }
              const title = windowObject.document.createElement("title");
              const titleText = windowObject.document.createTextNode(windowTitle);
              title.appendChild(titleText);
              windowObject.document.head.appendChild(title);
              windowObject.document.body.style.margin = 0;
              windowObject.document.body.style.overflowX = "hidden";
              windowObject.document.body.style.overflowY = "hidden";
              currentWindowId++;
              mapIdToWindow[currentWindowId] = windowObject;
              mapWindowToId.set(windowObject, currentWindowId);
              let canvas = windowObject.document.createElement("canvas");
              canvas.style.position = "absolute";
              canvas.style.left = "0px";
              canvas.style.top = "0px";
              let ignoreFirstResize = 0;
              if (windowObject.innerWidth === 0 || windowObject.innerHeight === 0) {
                canvas.width  = width ;
                canvas.height = height;
                ignoreFirstResize = 1;
              } else {
                windowObject.resizeTo(width + (windowObject.outerWidth - windowObject.innerWidth),
                                      height + (windowObject.outerHeight - windowObject.innerHeight));
                if (windowObject.screenLeft === 0 && windowObject.screenTop === 0) {
                  ignoreFirstResize = 2;
                }
                windowObject.moveTo(left, top);
                canvas.width  = windowObject.innerWidth;
                canvas.height = windowObject.innerHeight;
              }
              let context = canvas.getContext("2d");
              context.fillStyle = "#000000";
              context.fillRect(0, 0, width, height);
              windowObject.document.body.appendChild(canvas);
              mapIdToCanvas[currentWindowId] = canvas;
              mapCanvasToId.set(canvas, currentWindowId);
              mapIdToContext[currentWindowId] = context;
              if (reloadPageFunction === null) {
                if (typeof windowObject.opener.reloadPage !== "undefined") {
                  reloadPageFunction = windowObject.opener.reloadPage;
                }
              }
              if (deregisterWindowFunction === null) {
                if (typeof windowObject.opener.deregisterWindow !== "undefined") {
                  deregisterWindowFunction = windowObject.opener.deregisterWindow;
                }
              }
              if (typeof windowObject.opener.registerWindow !== "undefined") {
                windowObject.opener.registerWindow(windowObject);
              }
              return (currentWindowId << 3) | ignoreFirstResize;
            }
          } else {
            return 0;
          }
        }, (int) xPos, (int) yPos, (int) width, (int) height,
              winName8, winTitle8, firstWindowOpen);

      } /* if */
      if (windowIdAndFlags == 4) {
        windowIdAndFlags = openDocumentTabAsWindow(width, height);
      } /* if */
      if (unlikely(windowIdAndFlags == 0)) {
        /* This might be triggered by the error: */
        /* Opening multiple popups was blocked due to lack of user activation. */
        result = (emc_winType) openSubstituteWindow(xPos, yPos, width, height);
      } else if (unlikely(!ALLOC_RECORD2(result, emc_winRecord, count.win,
                                         count.win_bytes))) {
        err_info = MEMORY_ERROR;
      } else {
        firstWindowOpen = FALSE;
        memset(result, 0, sizeof(emc_winRecord));
        result->usage_count = 1;
        result->window = windowIdAndFlags >> 3;
        result->is_pixmap = FALSE;
        result->is_subwindow = FALSE;
        result->is_substitute = FALSE;
        result->is_tab = (windowIdAndFlags & 4) != 0;
        result->parentWindow = NULL;
        result->ignoreFirstResize = windowIdAndFlags & 3;
        result->creationTimestamp = timMicroSec() / 1000000;
        result->width = (int) width;
        result->height = (int) height;
        openSubstitute = result->is_tab;
        maxWindowId = result->window;
        setupEventCallbacksForWindow(result->window);
        enter_window((winType) result, result->window);
        synchronizeTimAwaitWithGraphicKeyboard();
      } /* if */
    } /* if */
    if (winTitle8 != NULL) {
      free_cstri8(winTitle8, windowName);
    } /* if */
    if (winName8 != NULL) {
      UNALLOC_CSTRI(winName8, winNameSize);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("drwOpen --> " FMT_U_MEM " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->window : 0,
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
    int windowId;
    emc_winType result = NULL;

  /* drwOpenSubWindow */
    logFunction(printf("drwOpenSubWindow(" FMT_U_MEM " (window=%d), "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) parent_window,
                       parent_window != NULL ? to_window(parent_window) : 0,
                       xPos, yPos, width, height););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwOpenSubWindow(" FMT_U_MEM " (window=%d), "
                      FMT_D ", " FMT_D ", " FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      (memSizeType) parent_window,
                      parent_window != NULL ? to_window(parent_window) : 0,
                      xPos, yPos, width, height););
      raise_error(RANGE_ERROR);
    } else {

      windowId = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
          let windowObject = mapIdToWindow[$0];
          let left = $1;
          let top = $2;
          let width = $3;
          let height = $4;
          let canvas = null;
          if (typeof windowObject.document !== "undefined") {
            canvas = windowObject.document.createElement("canvas");
          } else {
            canvas = windowObject.createElement("canvas");
          }
          canvas.width = width;
          canvas.height = height;
          canvas.style.position = "absolute";
          canvas.style.left = left + "px";
          canvas.style.top = top + "px";
          let context = canvas.getContext("2d");
          if (typeof windowObject.document !== "undefined") {
            windowObject.document.body.appendChild(canvas);
          } else {
            windowObject.body.appendChild(canvas);
          }
          currentWindowId++;
          mapIdToCanvas[currentWindowId] = canvas;
          mapCanvasToId.set(canvas, currentWindowId);
          mapIdToContext[currentWindowId] = context;
          return currentWindowId;
        } else {
          return 0;
        }
      }, to_window(parent_window), (int) xPos, (int) yPos, (int) width, (int) height);

      if (unlikely(windowId == 0)) {
        logError(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                        "): Failed to open window.\n",
                        xPos, yPos, width, height););
        raise_error(GRAPHIC_ERROR);
      } else if (ALLOC_RECORD2(result, emc_winRecord, count.win, count.win_bytes)) {
        memset(result, 0, sizeof(emc_winRecord));
        result->usage_count = 1;
        result->window = windowId;
        result->is_pixmap = FALSE;
        result->is_subwindow = TRUE;
        result->is_substitute = FALSE;
        result->is_tab = FALSE;
        result->parentWindow = parent_window;
        result->ignoreFirstResize = 0;
        result->creationTimestamp = 0;
        result->width = (int) width;
        result->height = (int) height;
        maxWindowId = result->window;
        enter_window((winType) result, result->window);
        synchronizeTimAwaitWithGraphicKeyboard();
      } /* if */
    } /* if */
    logFunction(printf("drwOpenSubWindow --> " FMT_U_MEM " (window=%d, usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->window : 0,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpenSubWindow */



void drwSetCloseAction (winType actual_window, intType closeAction)

  { /* drwSetCloseAction */
    logFunction(printf("drwSetCloseAction(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) actual_window, closeAction););
    if (closeAction < 0 || closeAction > 2) {
      logError(printf("drwSetCloseAction(" FMT_U_MEM ", " FMT_D "): "
                      "Close action not in allowed range\n",
                      (memSizeType) actual_window, closeAction););
      raise_error(RANGE_ERROR);
    } else {
      to_var_close_action(actual_window) = (int) closeAction;
    } /* if */
  } /* drwSetCloseAction */



void drwSetCursorVisible (winType aWindow, boolType visible)

  { /* drwSetCursorVisible */
  } /* drwSetCursorVisible */



void drwSetPointerPos (const_winType aWindow, intType xPos, intType yPos)

  { /* drwSetPointerPos */
  } /* drwSetPointerPos */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  {
    int successInfo;

  /* drwPPoint */
    logFunction(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, col););
    successInfo = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        context.fillStyle = "#" + ("000000" + $3.toString(16)).slice(-6);
        context.fillRect($1, $2, 1, 1);
        return 0;
      } else {
        return 1;
      }
    }, to_window(actual_window), castToInt(x), castToInt(y), (int) (col & 0xffffff));
    if (unlikely(successInfo != 0)) {
      logError(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " F_X(08) "): "
                      "windowId not found: %d\n",
                      (memSizeType) actual_window, x, y, col,
                      to_window(actual_window)););
      raise_error(GRAPHIC_ERROR);
    } /* if */
  } /* drwPPoint */



rtlArrayType drwConvPointList (const const_bstriType pointList)

  {
    memSizeType numCoords;
    int *coords;
    memSizeType pos;
    rtlArrayType xyArray;

  /* drwConvPointList */
    logFunction(printf("drwConvPointList(\"%s\")\n",
                       bstriAsUnquotedCStri(pointList)););
    numCoords = pointList->size / sizeof(int);
    if (unlikely(!ALLOC_RTL_ARRAY(xyArray, numCoords))) {
      raise_error(MEMORY_ERROR);
    } else {
      xyArray->min_position = 1;
      xyArray->max_position = (intType) numCoords;
      coords = (int *) pointList->mem;
      for (pos = 0; pos < numCoords; pos++) {
        xyArray->arr[pos].value.intValue = (intType) coords[pos];
      } /* for */
    } /* if */
    logFunction(printf("drwConvPointList --> arr (size=" FMT_U_MEM ")\n",
                       arraySize(xyArray)););
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
      logError(printf("drwGenPointList(" FMT_D " .. " FMT_D "): "
                      "Number of elements odd\n",
                      xyArray->min_position, xyArray->max_position););
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
    int successInfo;

  /* drwPolyLine */
    logFunction(printf("drwPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       (memSizeType) point_list, col););
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      logError(printf("drwPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_U_MEM ", " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y,
                      (memSizeType) point_list, col););
      raise_error(RANGE_ERROR);
    } else {
      coords = (int *) point_list->mem;
      numCoords = point_list->size / sizeof(int);
      if (numCoords >= 4) {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
            let context = mapIdToContext[$0];
            context.lineWidth = 1;
            context.strokeStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
            context.beginPath();
            context.moveTo($1 + HEAP32[$4 >> 2], $2 + HEAP32[($4 >> 2) + 1]);
            for (let i = 2; i < $3; i += 2) {
              context.lineTo($1 + HEAP32[($4 >> 2) + i], $2 + HEAP32[($4 >> 2) + i + 1]);
            }
            context.stroke();
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) x, (int) y, numCoords, (int) coords,
            (int) (col & 0xffffff));
        if (unlikely(successInfo != 0)) {
          logError(printf("drwPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                          ", " FMT_U_MEM ", " F_X(08) "): "
                          "windowId not found: %d\n",
                          (memSizeType) actual_window, x, y,
                          (memSizeType) point_list, col,
                          to_window(actual_window)););
          raise_error(GRAPHIC_ERROR);
        } /* if */
      } /* if */
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    int *coords;
    memSizeType numCoords;
    int successInfo;

  /* drwFPolyLine */
    logFunction(printf("drwFPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       (memSizeType) point_list, col););
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      logError(printf("drwFPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_U_MEM ", " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y,
                      (memSizeType) point_list, col););
      raise_error(RANGE_ERROR);
    } else {
      coords = (int *) point_list->mem;
      numCoords = point_list->size / sizeof(int);
      if (numCoords >= 4) {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
            let context = mapIdToContext[$0];
            context.fillStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
            context.beginPath();
            context.moveTo($1 + HEAP32[$4 >> 2], $2 + HEAP32[($4 >> 2) + 1]);
            for (let i = 2; i < $3; i += 2) {
              context.lineTo($1 + HEAP32[($4 >> 2) + i], $2 + HEAP32[($4 >> 2) + i + 1]);
            }
            context.closePath();
            context.fill();
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) x, (int) y, numCoords, (int) coords,
            (int) (col & 0xffffff));
        if (unlikely(successInfo != 0)) {
          logError(printf("drwFPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                          ", " FMT_U_MEM ", " F_X(08) "): "
                          "windowId not found: %d\n",
                          (memSizeType) actual_window, x, y,
                          (memSizeType) point_list, col,
                          to_window(actual_window)););
          raise_error(GRAPHIC_ERROR);
        } /* if */
      } /* if */
    } /* if */
  } /* drwFPolyLine */



void drwPut (const_winType destWindow, intType xDest, intType yDest,
    const_winType pixmap)

  {
    int successInfo;

  /* drwPut */
    logFunction(printf("drwPut(" FMT_U_MEM " (window=%d), "
                       FMT_D ", " FMT_D ", " FMT_U_MEM " (window=%d))\n",
                       (memSizeType) destWindow,
                       destWindow != NULL ? to_window(destWindow) : 0,
                       xDest, yDest, (memSizeType) pixmap,
                       pixmap != NULL ? to_window(pixmap) : 0););
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest))) {
      logError(printf("drwPut(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_U_MEM "): Raises RANGE_ERROR\n",
                      (memSizeType) destWindow, xDest, yDest,
                      (memSizeType) pixmap););
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL && to_window(pixmap) != 0) {
      /* A pixmap value of NULL or a pixmap with a window of 0 */
      /* is used to describe an empty pixmap. In this case     */
      /* nothing should be done.                               */
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined") {
          if (typeof mapIdToContext[$0] === "undefined") {
            return 2;
          } else if (typeof mapIdToCanvas[$1] === "undefined") {
            return 3;
          } else {
            mapIdToContext[$0].drawImage(mapIdToCanvas[$1], $2, $3);
            return 0;
          }
        } else {
          return 1;
        }
      }, to_window(destWindow), to_window(pixmap), (int) xDest, (int) yDest);
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPut(" FMT_U_MEM " (window=%d), "
                        FMT_D ", " FMT_D ", " FMT_U_MEM " (window=%d)): "
                        "windowId not found: %d\n",
                        (memSizeType) destWindow,
                        destWindow != NULL ? to_window(destWindow) : 0,
                        xDest, yDest, (memSizeType) pixmap,
                        pixmap != NULL ? to_window(pixmap) : 0,
                        successInfo == 3 ?
                        (pixmap != NULL ? to_window(pixmap) : 0) :
                        (destWindow != NULL ? to_window(destWindow) : 0)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPut */



void drwPutScaled (const_winType destWindow, intType xDest, intType yDest,
    intType width, intType height, const_winType pixmap)

  {
    int successInfo;

  /* drwPutScaled */
    logFunction(printf("drwPutScaled(" FMT_U_MEM " (window=%d), "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", "
                       FMT_U_MEM " (window=%d))\n",
                       (memSizeType) destWindow,
                       destWindow != NULL ? to_window(destWindow) : 0,
                       xDest, yDest, width, height,
                       (memSizeType) pixmap,
                       pixmap != NULL ? to_window(pixmap) : 0););
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest) ||
                 width < 0 || width > INT_MAX ||
                 height < 0 || height > INT_MAX)) {
      logError(printf("drwPutScaled(" FMT_U_MEM  ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " FMT_D ", " FMT_U_MEM"): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) destWindow, xDest, yDest,
                      width, height, (memSizeType) pixmap););
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL && to_window(pixmap) != 0) {
      /* A pixmap value of NULL or a pixmap with a window of 0 */
      /* is used to describe an empty pixmap. In this case     */
      /* nothing should be done.                               */
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined") {
          if (typeof mapIdToContext[$0] === "undefined") {
            return 2;
          } else if (typeof mapIdToCanvas[$1] === "undefined") {
            return 3;
          } else {
            mapIdToContext[$0].drawImage(mapIdToCanvas[$1], $2, $3, $4, $5);
            return 0;
          }
        } else {
          return 1;
        }
      }, to_window(destWindow), to_window(pixmap), (int) xDest, (int) yDest,
          (int) width, (int) height);
      if (unlikely(successInfo != 0)) {
        logError(printf("drwPutScaled(" FMT_U_MEM " (window=%d), "
                        FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", "
                        FMT_U_MEM " (window=%d)): "
                        "windowId not found: %d\n",
                        (memSizeType) destWindow,
                        destWindow != NULL ? to_window(destWindow) : 0,
                        xDest, yDest, width, height,
                        (memSizeType) pixmap,
                        pixmap != NULL ? to_window(pixmap) : 0,
                        successInfo == 3 ?
                        (pixmap != NULL ? to_window(pixmap) : 0) :
                        (destWindow != NULL ? to_window(destWindow) : 0)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwPutScaled */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  {
    int successInfo;

  /* drwPRect */
    logFunction(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       width, height, col););
    successInfo = EM_ASM_INT({
      if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
        let context = mapIdToContext[$0];
        context.fillStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
        context.fillRect($1, $2, $3, $4);
        return 0;
      } else {
        return 1;
      }
    }, to_window(actual_window), castToInt(x), castToInt(y),
        castToInt(width), castToInt(height), (int) (col & 0xffffff));
    if (unlikely(successInfo != 0)) {
      logError(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) "): "
                      "windowId not found: %d\n",
                      (memSizeType) actual_window, x, y, width, height, col,
                      to_window(actual_window)););
      raise_error(GRAPHIC_ERROR);
    } /* if */
  } /* drwPRect */



intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  {
    intType col;

  /* drwRgbColor */
    col = (intType) ((((((uintType) redLight)   >> 8) & 255) << 16) |
                     (((((uintType) greenLight) >> 8) & 255) <<  8) |
                      ((((uintType) blueLight)  >> 8) & 255)        | 0xff000000);
    logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> " F_X(08) "\n",
                       redLight, greenLight, blueLight, col););
    return col;
  } /* drwRgbColor */



void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight)

  { /* drwPixelToRgb */
    *redLight   = (intType) (( ((uintType) col) >> 16       ) << 8);
    *greenLight = (intType) (((((uintType) col) >>  8) & 255) << 8);
    *blueLight  = (intType) (( ((uintType) col)        & 255) << 8);
    logFunction(printf("drwPixelToRgb(" F_X(08) ", " FMT_D ", " FMT_D ", " FMT_D ") -->\n",
                       col, *redLight, *greenLight, *blueLight););
  } /* drwPixelToRgb */



intType drwScreenHeight (void)

  {
    int height;

  /* drwScreenHeight */
    height = EM_ASM_INT({
      if (typeof window !== "undefined") {
        return window.screen.height;
      } else {
        return -1;
      }
    });
    if (unlikely(height == -1)) {
      raise_error(GRAPHIC_ERROR);
      height = 0;
    } /* if */
    return (intType) height;
  } /* drwScreenHeight */



intType drwScreenWidth (void)

  {
    int width;

  /* drwScreenWidth */
    width = EM_ASM_INT({
      if (typeof window !== "undefined") {
        return window.screen.width;
      } else {
        return -1;
      }
    });
    if (unlikely(width == -1)) {
      raise_error(GRAPHIC_ERROR);
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
    int successInfo;

  /* drwSetPos */
    logFunction(printf("drwSetPos(" FMT_U_MEM " (%d), " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, to_window(actual_window),
                       xPos, yPos););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos))) {
      logError(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, xPos, yPos););
      raise_error(RANGE_ERROR);
    } else if (is_pixmap(actual_window)) {
      logError(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "Window is pixmap\n",
                      (memSizeType) actual_window, xPos, yPos););
      raise_error(RANGE_ERROR);
    } else {
      if (is_subwindow(actual_window)) {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
            let canvas = mapIdToCanvas[$0];
            canvas.style.left = $1 + "px";
            canvas.style.top = $2 + "px";
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) xPos, (int) yPos);
      } else {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
            let windowObject = mapIdToWindow[$0];
            windowObject.screenX = $1;
            windowObject.screenY = $2;
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) xPos, (int) yPos);
      } /* if */
      if (unlikely(successInfo != 0)) {
        logError(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, xPos, yPos,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwSetPos */



void drwSetSize (winType actual_window, intType width, intType height)

  {
    int successInfo;

  /* drwSetSize */
    logFunction(printf("drwSetSize(" FMT_U_MEM " (%d), " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, to_window(actual_window),
                       width, height););
    if (unlikely(width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwSetSize(" FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      width, height););
      raise_error(RANGE_ERROR);
    } else if (to_width(actual_window) != (int) width ||
               to_height(actual_window) != (int) height) {
      if (is_pixmap(actual_window) || is_subwindow(actual_window)) {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
            let canvas = mapIdToCanvas[$0];
            let context = mapIdToContext[$0];
            let imageData = context.getImageData(0, 0, canvas.width, canvas.height);
            canvas.width = $1;
            canvas.height = $2;
            context.fillStyle = "#" + ("000000" + $3.toString(16)).slice(-6);
            context.fillRect(0, 0, context.canvas.width, context.canvas.height);
            context.putImageData(imageData, 0, 0);
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) width, (int) height,
              (int) (to_clear_col(actual_window) & 0xffffff));
      } else {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
            let windowObject = mapIdToWindow[$0];
            let canvas = mapIdToCanvas[$0];
            let context = mapIdToContext[$0];
            let imageData = context.getImageData(0, 0, canvas.width, canvas.height);
            windowObject.innerWidth = $1;
            windowObject.innerHeight = $2;
            canvas.width = $1;
            canvas.height = $2;
            context.fillStyle = "#" + ("000000" + $3.toString(16)).slice(-6);
            context.fillRect(0, 0, context.canvas.width, context.canvas.height);
            context.putImageData(imageData, 0, 0);
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) width, (int) height,
              (int) (to_clear_col(actual_window) & 0xffffff));
      } /* if */
      if (unlikely(successInfo != 0)) {
        logError(printf("drwSetSize(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                        "windowId not found: %d\n",
                        (memSizeType) actual_window, width, height,
                        to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
      } else {
        to_var_width(actual_window) = (int) width;
        to_var_height(actual_window) = (int) height;
      } /* if */
    } /* if */
  } /* drwSetSize */



void drwSetTransparentColor (winType pixmap, intType col)

  { /* drwSetTransparentColor */
  } /* drwSetTransparentColor */



void drwSetWindowName (winType aWindow, const const_striType windowName)

  {
    char *winName8;
    errInfoType err_info = OKAY_NO_ERROR;
    int successInfo;

  /* drwSetWindowName */
    logFunction(printf("drwSetWindowName(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) aWindow,
                       striAsUnquotedCStri(windowName)););
    winName8 = stri_to_cstri8(windowName, &err_info);
    if (unlikely(winName8 == NULL)) {
      logError(printf("drwSetWindowName: stri_to_cstri8(\"%s\") failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(windowName), err_info););
      raise_error(err_info);
    } else {
      successInfo = EM_ASM_INT({
        if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
          let windowObject = mapIdToWindow[$0];
          let windowName = Module.UTF8ToString($1);
          windowObject.document.title = windowName;
          return 0;
        } else {
          return 1;
        }
      }, to_window(aWindow), winName8);
      free_cstri8(winName8, windowName);
      if (unlikely(successInfo != 0)) {
        logError(printf("drwSetWindowName(" FMT_U_MEM ", \"%s\"): "
                        "windowId not found: %d\n",
                        (memSizeType) aWindow,
                        striAsUnquotedCStri(windowName),
                        to_window(aWindow)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
  } /* drwSetWindowName */



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  {
    cstriType stri8;
    errInfoType err_info = OKAY_NO_ERROR;
    int successInfo;

  /* drwText */
    logFunction(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", \"%s\", " F_X(08) ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       striAsUnquotedCStri(stri), col, bkcol););
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      logError(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", \"%s\", " F_X(08) ", " F_X(08) "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y,
                      striAsUnquotedCStri(stri), col, bkcol););
      raise_error(RANGE_ERROR);
    } else {
      stri8 = stri_to_cstri8(stri, &err_info);
      if (unlikely(stri8 == NULL)) {
        logError(printf("drwText: stri_to_cstri8(\"%s\") failed:\n"
                        "err_info=%d\n",
                        striAsUnquotedCStri(stri), err_info););
        raise_error(err_info);
      } else {
        successInfo = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToContext[$0] !== "undefined") {
            let context = mapIdToContext[$0];
            let text = Module.UTF8ToString($3);
            context.fillStyle = "#" + ("000000" + $5.toString(16)).slice(-6);
            context.font = "10px Courier New";
            let width = context.measureText(text).width;
            context.fillRect($1, $2 - 11, width, 13);
            context.fillStyle = "#" + ("000000" + $4.toString(16)).slice(-6);
            context.fillText(text, $1, $2);
            return 0;
          } else {
            return 1;
          }
        }, to_window(actual_window), (int) x, (int) y, stri8,
            (int) (col & 0xffffff), (int) (bkcol & 0xffffff));
        free_cstri8(stri8, stri);
        if (unlikely(successInfo != 0)) {
          logError(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D
                          ", \"%s\", " F_X(08) ", " F_X(08) "): "
                          "windowId not found: %d\n",
                          (memSizeType) actual_window, x, y,
                          striAsUnquotedCStri(stri), col, bkcol,
                          to_window(actual_window)););
          raise_error(GRAPHIC_ERROR);
        } /* if */
      } /* if */
    } /* if */
  } /* drwText */



void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
    logFunction(printf("drwToBottom(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_subwindow(actual_window)) {
      EM_ASM({
        let canvas = mapIdToCanvas[$0];
        let parent = canvas.parentNode;
        parent.removeChild(canvas);
        parent.insertBefore(canvas, parent.firstChild.nextSibling);
      }, to_window(actual_window));
    } /* if */
    logFunction(printf("drwToBottom(" FMT_U_MEM ") -->\n",
                       (memSizeType) actual_window););
  } /* drwToBottom */



void drwToTop (const_winType actual_window)

  { /* drwToTop */
    logFunction(printf("drwToTop(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (is_subwindow(actual_window)) {
      EM_ASM({
        let canvas = mapIdToCanvas[$0];
        let parent = canvas.parentNode;
        parent.removeChild(canvas);
        parent.appendChild(canvas);
      }, to_window(actual_window));
    } else {
      EM_ASM({
        if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
          let windowObject = mapIdToWindow[$0];
          if (typeof windowObject.focus !== "undefined") {
            windowObject.focus();
          } else {
            let canvas = mapIdToCanvas[$0];
            if (typeof canvas.focus !== "undefined") {
              canvas.focus();
            }
          }
        }
      }, to_window(actual_window));
    } /* if */
    logFunction(printf("drwToTop(" FMT_U_MEM ") -->\n",
                       (memSizeType) actual_window););
  } /* drwToTop */



intType drwWidth (const_winType actual_window)

  {
    int width;

  /* drwWidth */
    logFunction(printf("drwWidth(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    width = to_width(actual_window);
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
      logError(printf("drwXPos(" FMT_U_MEM "): Window is pixmap\n",
                      (memSizeType) actual_window););
      raise_error(RANGE_ERROR);
      xPos = 0;
    } else {
      if (is_subwindow(actual_window)) {
        xPos = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
            let left = mapIdToCanvas[$0].style.left;
            if (left.endsWith("px")) {
              return left.substring(0, left.length - 2);
            } else {
              return left;
            }
          } else {
            return -2147483648;
          }
        }, to_window(actual_window));
        if (is_substitute(actual_window)) {
          xPos += drwXPos(to_parentWindow(actual_window));
        } /* if */
      } else {
        xPos = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
            return mapIdToWindow[$0].screenX;
          } else {
            return -2147483648;
          }
        }, to_window(actual_window));
      } /* if */
      if (unlikely(xPos == -2147483648)) {
        logError(printf("drwXPos(" FMT_U_MEM "): windowId not found: %d\n",
                        (memSizeType) actual_window, to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
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
      logError(printf("drwYPos(" FMT_U_MEM "): Window is pixmap\n",
                      (memSizeType) actual_window););
      raise_error(RANGE_ERROR);
      yPos = 0;
    } else {
      if (is_subwindow(actual_window)) {
        yPos = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToCanvas[$0] !== "undefined") {
            let top = mapIdToCanvas[$0].style.top;
            if (top.endsWith("px")) {
              return top.substring(0, top.length - 2);
            } else {
              return top;
            }
          } else {
            return -2147483648;
          }
        }, to_window(actual_window));
        if (is_substitute(actual_window)) {
          yPos += drwYPos(to_parentWindow(actual_window));
        } /* if */
      } else {
        yPos = EM_ASM_INT({
          if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
            return mapIdToWindow[$0].screenY;
          } else {
            return -2147483648;
          }
        }, to_window(actual_window));
      } /* if */
      if (unlikely(yPos == -2147483648)) {
        logError(printf("drwYPos(" FMT_U_MEM "): windowId not found: %d\n",
                        (memSizeType) actual_window, to_window(actual_window)););
        raise_error(GRAPHIC_ERROR);
        yPos = 0;
      } /* if */
    } /* if */
    logFunction(printf("drwYPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, yPos););
    return (intType) yPos;
  } /* drwYPos */
