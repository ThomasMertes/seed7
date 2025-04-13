/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilities.    */
/*  Copyright (C) 1989 - 2006, 2009 - 2017  Thomas Mertes           */
/*                2019 - 2024  Thomas Mertes                        */
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
/*  File: seed7/src/drw_win.c                                       */
/*  Changes: 2005 - 2007, 2013 - 2017, 2019 - 2021  Thomas Mertes   */
/*  Content: Graphic access using the windows capabilities.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "math.h"
#include "windows.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "kbd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"


#define TRACE_EVENTS 0
#if TRACE_EVENTS
#define traceEvent(traceStatements) traceStatements
#else
#define traceEvent(traceStatements)
#endif
#define traceEventX(traceStatements) traceStatements

#define PI 3.141592653589793238462643383279502884197

#define windowClassW L"MyWindowClass"

static boolType init_called = FALSE;

typedef struct {
    uintType usage_count;
    /* Up to here the structure is identical to struct winStruct */
    HWND hWnd;
    HDC hdc;
    HBITMAP backup;
    HDC backup_hdc;
    HBITMAP hBitmap;
    HBITMAP oldBitmap;
    boolType hasTransparentPixel;
    UINT transparentPixel;
    HBITMAP maskBitmap;
    boolType is_pixmap;
    unsigned int width;  /* Always <= INT_MAX: Cast to int is safe. */
    unsigned int height; /* Always <= INT_MAX: Cast to int is safe. */
    unsigned int backupWidth;  /* Always <= INT_MAX: Cast to int is safe. */
    unsigned int backupHeight; /* Always <= INT_MAX: Cast to int is safe. */
    boolType minimized;
    intType clear_col;
    int close_action;
    boolType resizeReturnsKey;
    boolType cursorVisible;
  } win_winRecord, *win_winType;

typedef const win_winRecord *const_win_winType;

#define to_hwnd(win)                 (((const_win_winType) (win))->hWnd)
#define to_hdc(win)                  (((const_win_winType) (win))->hdc)
#define to_backup_hdc(win)           (((const_win_winType) (win))->backup_hdc)
#define to_backup(win)               (((const_win_winType) (win))->backup)
#define to_hBitmap(win)              (((const_win_winType) (win))->hBitmap)
#define to_oldBitmap(win)            (((const_win_winType) (win))->oldBitmap)
#define is_pixmap(win)               (((const_win_winType) (win))->is_pixmap)
#define to_hasTransparentPixel(win)  (((const_win_winType) (win))->hasTransparentPixel)
#define to_transparentPixel(win)     (((const_win_winType) (win))->transparentPixel)
#define to_maskBitmap(win)           (((const_win_winType) (win))->maskBitmap)
#define to_width(win)                (((const_win_winType) (win))->width)
#define to_height(win)               (((const_win_winType) (win))->height)
#define to_backupWidth(win)          (((const_win_winType) (win))->backupWidth)
#define to_backupHeight(win)         (((const_win_winType) (win))->backupHeight)
#define to_minimized(win)            (((const_win_winType) (win))->minimized)
#define to_clear_col(win)            (((const_win_winType) (win))->clear_col)
#define to_close_action(win)         (((const_win_winType) (win))->close_action)
#define to_resizeReturnsKey(win)     (((const_win_winType) (win))->resizeReturnsKey)
#define to_cursorVisible(win)        (((const_win_winType) (win))->cursorVisible)

#define to_var_hasTransparentPixel(win)  (((win_winType) (win))->hasTransparentPixel)
#define to_var_transparentPixel(win)     (((win_winType) (win))->transparentPixel)
#define to_var_maskBitmap(win)           (((win_winType) (win))->maskBitmap)
#define to_var_width(win)                (((win_winType) (win))->width)
#define to_var_height(win)               (((win_winType) (win))->height)
#define to_var_clear_col(win)            (((win_winType) (win))->clear_col)
#define to_var_close_action(win)         (((win_winType) (win))->close_action)
#define to_var_resizeReturnsKey(win)     (((win_winType) (win))->resizeReturnsKey)
#define to_var_cursorVisible(win)        (((win_winType) (win))->cursorVisible)

#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE 674
#endif

#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE 0x08000000L
#endif

typedef HWND (WINAPI *pGetConsoleWindowType)(void);
static pGetConsoleWindowType pGetConsoleWindow = NULL;
static winType emptyWindow = NULL;


winType find_window (HWND sys_window);
void enter_window (winType curr_window, HWND sys_window);
void remove_window (HWND sys_window);
void gkbInitKeyboard (void);



int getCloseAction (winType actual_window)

  { /* getCloseAction */
    logFunction(printf("getCloseAction(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    return actual_window != NULL ? to_close_action(actual_window)
                                 : CLOSE_BUTTON_RAISES_EXCEPTION;
  } /* getCloseAction */



static void drawRectangle (HDC hdc, intType x1, intType y1,
    intType x2, intType y2, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drawRectangle */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(hdc, current_pen);
      old_brush = (HBRUSH) SelectObject(hdc, current_brush);
      if (x1 == x2) {
        if (y1 == y2) {
          SetPixel(hdc, castToInt(x1), castToInt(y1), (COLORREF) col);
        } else {
          MoveToEx(hdc, castToInt(x1), castToInt(y1), NULL);
          LineTo(hdc, castToInt(x1), castToInt(y2 + 1));
        } /* if */
      } else {
        if (y1 == y2) {
          MoveToEx(hdc, castToInt(x1), castToInt(y1), NULL);
          LineTo(hdc, castToInt(x2 + 1), castToInt(y1));
        } else {
          Rectangle(hdc, castToInt(x1), castToInt(y1), castToInt(x2 + 1), castToInt(y2 + 1));
        } /* if */
      } /* if */
      SelectObject(hdc, old_pen);
      SelectObject(hdc, old_brush);
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drawRectangle */



void setResizeReturnsKey (winType resizeWindow, boolType active)

  { /* setResizeReturnsKey */
    to_var_resizeReturnsKey(resizeWindow) = active;
  } /* setResizeReturnsKey */



boolType getResizeReturnsKey (winType resizeWindow)

  { /* getResizeReturnsKey */
    return to_resizeReturnsKey(resizeWindow);
  } /* getResizeReturnsKey */



static boolType resize (win_winType resizeWindow, unsigned int width,
    unsigned int height)

  {
    HDC newBackupHdc;
    HBITMAP newBackup;
    unsigned int newWidth;
    unsigned int newHeight;
    boolType returnKeyResize = FALSE;

  /* resize */
    logFunction(printf("resize(" FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) resizeWindow, width, height););
    if (resizeWindow != NULL && resizeWindow->backup_hdc != 0) {
      if (resizeWindow->width != width || resizeWindow->height != height) {
        if (resizeWindow->backupWidth < width ||
            resizeWindow->backupHeight < height) {
          newWidth = resizeWindow->backupWidth > width ? resizeWindow->backupWidth : width;
          newHeight = resizeWindow->backupHeight > height ? resizeWindow->backupHeight : height;
          newBackupHdc = CreateCompatibleDC(resizeWindow->hdc);
          newBackup = CreateCompatibleBitmap(resizeWindow->hdc, (int) newWidth, (int) newHeight);
          SelectObject(newBackupHdc, newBackup);
          drawRectangle(newBackupHdc, 0, 0, newWidth - 1, newHeight - 1,
                        resizeWindow->clear_col);
          BitBlt(newBackupHdc, 0, 0,
                 (int) resizeWindow->backupWidth, (int) resizeWindow->backupHeight,
                 resizeWindow->backup_hdc, 0, 0, SRCCOPY);
          DeleteObject(resizeWindow->backup);
          DeleteDC(resizeWindow->backup_hdc);
          resizeWindow->backup = newBackup;
          resizeWindow->backup_hdc = newBackupHdc;
          resizeWindow->backupWidth = newWidth;
          resizeWindow->backupHeight = newHeight;
        } /* if */
        resizeWindow->width = width;
        resizeWindow->height = height;
        if (resizeWindow->resizeReturnsKey) {
          returnKeyResize = TRUE;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("resize --> %d\n", returnKeyResize););
    return returnKeyResize;
  } /* resize */



LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    win_winType paint_window;
    PAINTSTRUCT paintStruct;
    RECT rect;
    RECT rect2;
    LRESULT result;

  /* WndProc */
    logFunction(printf("WndProc(" FMT_U_MEM ", %u, " FMT_U_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) hWnd, message, wParam, lParam););
    switch (message) {
      case WM_PAINT:
        traceEvent(printf("WndProc: WM_PAINT hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        paint_window = (win_winType) find_window(hWnd);
        if (paint_window != NULL && paint_window->backup_hdc != 0) {
          BeginPaint(paint_window->hWnd, &paintStruct);
          /* printf("BeginPaint left=%ld, top=%ld, right=%ld, bottom=%ld\n",
              paintStruct.rcPaint.left, paintStruct.rcPaint.top,
              paintStruct.rcPaint.right, paintStruct.rcPaint.bottom); */
          /* printf("WM_PAINT      %lu %d/%d %d/%d\n", hWnd,
              paintStruct.rcPaint.left, paintStruct.rcPaint.top,
              paintStruct.rcPaint.right - paintStruct.rcPaint.left,
              paintStruct.rcPaint.bottom - paintStruct.rcPaint.top); */
          BitBlt(to_hdc(paint_window),
              paintStruct.rcPaint.left, paintStruct.rcPaint.top,
              paintStruct.rcPaint.right - paintStruct.rcPaint.left,
              paintStruct.rcPaint.bottom - paintStruct.rcPaint.top,
              to_backup_hdc(paint_window),
              paintStruct.rcPaint.left, paintStruct.rcPaint.top, SRCCOPY);
          EndPaint(paint_window->hWnd, &paintStruct);
        } else {
          printf("paint_window=" FMT_U_MEM ", backup_hdc=" FMT_U_MEM "\n",
              (memSizeType) paint_window,
              (memSizeType) paint_window->backup_hdc);
        } /* if */
        result = 0;
        break;
      case WM_ERASEBKGND:
        traceEvent(printf("WndProc: WM_ERASEBKGND hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        paint_window = (win_winType) find_window(hWnd);
        if (paint_window != NULL && paint_window->backup_hdc != 0) {
          if (GetUpdateRect(paint_window->hWnd, &rect, FALSE) != 0) {
            /* printf("GetUpdateRect left=%ld, top=%ld, right=%ld, bottom=%ld\n",
               rect.left, rect.top, rect.right, rect.bottom); */
            /* printf("WM_ERASEBKGND %lu %d/%d %d/%d\n", hWnd,
                rect.left, rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top); */
          } else {
            /* printf("GetUpdateRect no update region\n"); */
            GetClientRect(paint_window->hWnd, &rect);
            /* printf("GetClientRect left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect.top, rect.right, rect.bottom); */
            /* printf("WM_ERASEBKGND %lu %d/%d %d/%d +\n", hWnd,
                rect.left, rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top); */
          } /* if */
          /* printf("window width=%ld, height=%ld\n",
              paint_window->width, paint_window->height); */
          if (rect.right >= 0 && (unsigned int) rect.right >= paint_window->width) {
            if (rect.left < 0 || (unsigned int) rect.left < paint_window->width) {
              rect2.left = (LONG) paint_window->width;
            } else {
              rect2.left = rect.left;
            } /* if */
            if (rect.bottom >= 0 && (unsigned int) rect.bottom >= paint_window->height) {
              rect2.bottom = (LONG) paint_window->height - 1;
            } else {
              rect2.bottom = rect.bottom;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect2.left, rect.top, rect.right, rect2.bottom); */
            drawRectangle(paint_window->hdc, rect2.left, rect.top, rect.right, rect2.bottom,
                to_clear_col(paint_window));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
          if (rect.bottom >= 0 && (unsigned int) rect.bottom >= paint_window->height) {
            if (rect.top < 0 || (unsigned int) rect.top < paint_window->height) {
              rect2.top = (LONG) paint_window->height;
            } else {
              rect2.top = rect.top;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect2.top, rect.right, rect.bottom); */
            drawRectangle(paint_window->hdc, rect.left, rect2.top, rect.right, rect.bottom,
                to_clear_col(paint_window));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
        } else {
          printf("paint_window=" FMT_U_MEM ", backup_hdc=" FMT_U_MEM "\n",
              (memSizeType) paint_window,
              (memSizeType) paint_window->backup_hdc);
        } /* if */
        result = 1;
        break;
      case WM_SIZE:
        traceEvent(printf("WndProc: WM_SIZE hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        if (wParam != SIZE_MINIMIZED) {
          paint_window = (win_winType) find_window(hWnd);
          if (resize(paint_window, (unsigned int) LOWORD(lParam),
                 (unsigned int) HIWORD(lParam))) {
            PostMessageW(hWnd, WM_USER + 1, wParam, lParam);
          } /* if */
        } /* if */
        result = 0;
        break;
      case WM_USER + 1:
        traceEvent(printf("WndProc: WM_USER + 1 hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        PostMessageW(hWnd, WM_USER, wParam, lParam);
        result = 1;
        break;
      case WM_SETCURSOR:
        traceEvent(printf("WndProc: WM_SETCURSOR hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        paint_window = (win_winType) find_window(hWnd);
        if (LOWORD(lParam) == HTCLIENT && !paint_window->cursorVisible) {
          SetCursor(NULL);
          result = 1;
        } else{
          result = DefWindowProcW(hWnd, message, wParam, lParam);
        } /* if */
        break;
      case WM_SYSCOMMAND:
        traceEvent(printf("WndProc: WM_SYSCOMMAND: hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          (memSizeType) hWnd, wParam, lParam););
        if ((wParam & 0xfff0) == SC_MAXIMIZE || (wParam & 0xfff0) == SC_RESTORE) {
          /* printf("SC_MAXIMIZE / SC_RESTORE\n"); */
          paint_window = (win_winType) find_window(hWnd);
          if (paint_window != NULL) {
            if (paint_window->minimized) {
              paint_window->minimized = FALSE;
            } /* if */
          } /* if */
        } else if ((wParam & 0xfff0) == SC_MINIMIZE) {
          /* printf("SC_MINIMIZE\n"); */
          paint_window = (win_winType) find_window(hWnd);
          if (paint_window != NULL) {
            paint_window->minimized = TRUE;
          } /* if */
        } /* if */
        result = DefWindowProcW(hWnd, message, wParam, lParam);
        break;
      default:
        traceEvent(printf("WndProc: message=%d, hwnd=" FMT_U_MEM
                          ", wParam=" FMT_U_MEM ", lParam=" FMT_X_MEM "\n",
                          message, (memSizeType) hWnd, wParam, lParam););
        result = DefWindowProcW(hWnd, message, wParam, lParam);
        break;
    } /* switch */
    logFunction(printf("WndProc --> " FMT_D_MEM "\n", result););
    return result;
  } /* WndProc */



static winType generateEmptyWindow (void)

  {
    HDC screenDC;
    win_winType newWindow;

  /* generateEmptyWindow */
    logFunction(printf("generateEmptyWindow()\n"););
    if (unlikely(!ALLOC_RECORD2(newWindow, win_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(newWindow, 0, sizeof(win_winRecord));
      newWindow->usage_count = 0;  /* Do not use reference counting (will not be freed). */
      screenDC = GetDC(NULL);
      newWindow->hdc = CreateCompatibleDC(screenDC);
      newWindow->hBitmap = CreateCompatibleBitmap(screenDC, 0, 0);
      ReleaseDC(NULL, screenDC);
      newWindow->oldBitmap = (HBITMAP) SelectObject(newWindow->hdc, newWindow->hBitmap);
      newWindow->hasTransparentPixel = FALSE;
      newWindow->transparentPixel = 0;
      newWindow->is_pixmap = TRUE;
      newWindow->width = 0;
      newWindow->height = 0;
    } /* if */
    logFunction(printf("generateEmptyWindow --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
  } /* generateEmptyWindow */



void drawClose (void)

  { /* drawClose */
    if (emptyWindow != NULL) {
      FREE_RECORD2(emptyWindow, win_winRecord, count.win, count.win_bytes);
      emptyWindow = NULL;
    } /* if */
  } /* drawClose */



void drawInit (void)

  {
    WNDCLASSEXW wcex = {0};
    HMODULE hntdll;

  /* drawInit */
    logFunction(printf("drawInit()\n"););
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = /* CS_HREDRAW | CS_VREDRAW | */ CS_OWNDC;
    wcex.lpfnWndProc   = (WNDPROC) WndProc;
    wcex.hInstance     = NULL;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = windowClassW;
    wcex.hIconSm       = NULL;
    RegisterClassExW(&wcex);
    hntdll = LoadLibraryA("kernel32.dll");
    if (hntdll != 0) {
      pGetConsoleWindow = (pGetConsoleWindowType) GetProcAddress(hntdll, "GetConsoleWindow");
    } /* if */
    emptyWindow = generateEmptyWindow();
    gkbInitKeyboard();
    init_called = TRUE;
  } /* drawInit */



/**
 *  Return the X position of the pointer relative to the specified window.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'actual_window' (inside of the window decorations).
 *  If 'actual_window' is the empty window the pointer X position is
 *  relative to the top left corner of the screen.
 */
intType drwPointerXpos (const_winType actual_window)

  {
    POINT point;
    intType xPos;

  /* drwPointerXpos */
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (unlikely(GetCursorPos(&point) == 0)) {
      raise_error(GRAPHIC_ERROR);
      xPos = 0;
    } else if (to_width(actual_window) == 0 && to_height(actual_window) == 0) {
      xPos = point.x;
    } else {
      if (unlikely(ScreenToClient(to_hwnd(actual_window), &point) == 0)) {
        raise_error(GRAPHIC_ERROR);
        xPos = 0;
      } else {
        xPos = point.x;
      } /* if */
    } /* if */
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, xPos););
    return xPos;
  } /* drwPointerXpos */



/**
 *  Return the Y position of the pointer relative to the specified window.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'actual_window' (inside of the window decorations).
 *  If 'actual_window' is the empty window the pointer Y position is
 *  relative to the top left corner of the screen.
 */
intType drwPointerYpos (const_winType actual_window)

  {
    POINT point;
    intType yPos;

  /* drwPointerYpos */
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (unlikely(GetCursorPos(&point) == 0)) {
      raise_error(GRAPHIC_ERROR);
      yPos = 0;
    } else if (to_width(actual_window) == 0 && to_height(actual_window) == 0) {
      yPos = point.y;
    } else {
      if (unlikely(ScreenToClient(to_hwnd(actual_window), &point) == 0)) {
        raise_error(GRAPHIC_ERROR);
        yPos = 0;
      } else {
        yPos = point.y;
      } /* if */
    } /* if */
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, yPos););
    return yPos;
  } /* drwPointerYpos */



void drwArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    FLOAT startAng, sweepAng;

  /* drwArc */
    startAng = (FLOAT) (startAngle * (360.0 / (2 * PI)));
    sweepAng = (FLOAT) (sweepAngle * (360.0 / (2 * PI)));
    AngleArc(to_hdc(actual_window), castToInt(x), castToInt(y),
             (unsigned) radius, startAng, sweepAng);
  } /* drwArc */



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    FLOAT startAng, sweepAng;
    HPEN old_pen;
    HPEN current_pen;

  /* drwPArc */
    logFunction(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (unlikely(!inIntRange(x) || !inIntRange(x) || !inIntRange(radius) || radius <= 0)) {
      raise_error(RANGE_ERROR);
    } else {
      startAng = (FLOAT) (startAngle * (360.0 / (2 * PI)));
      sweepAng = (FLOAT) (sweepAngle * (360.0 / (2 * PI)));
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      if (unlikely(current_pen == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        MoveToEx(to_hdc(actual_window),
                 (int) x + (int) (cos(startAngle) * (double) radius),
                 (int) y - (int) (sin(startAngle) * (double) radius),
                 (LPPOINT) NULL);
        AngleArc(to_hdc(actual_window), (int) x, (int) y, (unsigned) radius, startAng, sweepAng);
        SelectObject(to_hdc(actual_window), old_pen);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          MoveToEx(to_backup_hdc(actual_window),
                   (int) x + (int) (cos(startAngle) * (double) radius),
                   (int) y - (int) (sin(startAngle) * (double) radius),
                   (LPPOINT) NULL);
          AngleArc(to_backup_hdc(actual_window), (int) x, (int) y, (unsigned) radius, startAng, sweepAng);
          SelectObject(to_backup_hdc(actual_window), old_pen);
        } /* if */
        DeleteObject(current_pen);
      } /* if */
    } /* if */
  } /* drwPArc */



void drwPFArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle,
    intType width, intType col)

  {
    FLOAT startAng, sweepAng;
    double minRadius;
    double maxRadius;
    double angleDelta;
    double angle;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFArc */
    logFunction(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, width, col););
    if (width == 1) {
      drwPArc(actual_window, x, y, radius, startAngle, sweepAngle, col);
    } else if (unlikely(!inIntRange(x) || !inIntRange(y) || !inIntRange(radius) ||
                        radius <= 0 || width <= 0 || width > radius)) {
      raise_error(RANGE_ERROR);
    } else {
      startAng = (FLOAT) (startAngle * (360.0 / (2 * PI)));
      sweepAng = (FLOAT) (sweepAngle * (360.0 / (2 * PI)));
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      current_brush = CreateSolidBrush((COLORREF) col);
      if (unlikely(current_pen == NULL || current_brush == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
        minRadius = (double) (radius - width + 1);
        maxRadius = (double) radius;
        angleDelta = asin(2.0 / (double) radius);
        BeginPath(to_hdc(actual_window));
        MoveToEx(to_hdc(actual_window),
                 (int) x + (int) (cos(startAngle) * maxRadius),
                 (int) y - (int) (sin(startAngle) * maxRadius),
                 (LPPOINT) NULL);
        AngleArc(to_hdc(actual_window), (int) x, (int) y,
                 (DWORD) radius, startAng, sweepAng);
        for (angle = startAngle + sweepAngle; angle >= startAngle; angle -= angleDelta) {
          LineTo(to_hdc(actual_window),
                 (int) x + (int) (cos(angle) * minRadius),
                 (int) y - (int) (sin(angle) * minRadius));
        } /* if */
        LineTo(to_hdc(actual_window),
               (int) x + (int) (cos(startAngle) * minRadius),
               (int) y - (int) (sin(startAngle) * minRadius));
        EndPath(to_hdc(actual_window));
        StrokeAndFillPath(to_hdc(actual_window));
        SelectObject(to_hdc(actual_window), old_pen);
        SelectObject(to_hdc(actual_window), old_brush);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
          BeginPath(to_backup_hdc(actual_window));
          MoveToEx(to_backup_hdc(actual_window),
                   (int) x + (int) (cos(startAngle) * maxRadius),
                   (int) y - (int) (sin(startAngle) * maxRadius),
                   (LPPOINT) NULL);
          AngleArc(to_backup_hdc(actual_window), (int) x, (int) y,
                   (DWORD) radius, startAng, sweepAng);
          for (angle = startAngle + sweepAngle; angle >= startAngle; angle -= angleDelta) {
            LineTo(to_backup_hdc(actual_window),
                   (int) x + (int) (cos(angle) * minRadius),
                   (int) y - (int) (sin(angle) * minRadius));
          } /* if */
          LineTo(to_backup_hdc(actual_window),
                 (int) x + (int) (cos(startAngle) * minRadius),
                 (int) y - (int) (sin(startAngle) * minRadius));
          EndPath(to_backup_hdc(actual_window));
          StrokeAndFillPath(to_backup_hdc(actual_window));
          SelectObject(to_backup_hdc(actual_window), old_pen);
          SelectObject(to_backup_hdc(actual_window), old_brush);
        } /* if */
        DeleteObject(current_pen);
        DeleteObject(current_brush);
      } /* if */
    } /* if */
  } /* drwPFArc */



void drwFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  { /* drwFArcChord */
  } /* drwFArcChord */



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    FLOAT startAng, sweepAng;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFArcChord */
    logFunction(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (sweepAngle != 0.0) {
      startAng = (FLOAT) (startAngle * (360.0 / (2 * PI)));
      sweepAng = (FLOAT) (sweepAngle * (360.0 / (2 * PI)));
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      current_brush = CreateSolidBrush((COLORREF) col);
      if (unlikely(current_pen == NULL || current_brush == NULL)) {
        raise_error(MEMORY_ERROR);
      } else if (unlikely(!inIntRange(x) || !inIntRange(x) ||
                          !inIntRange(radius) || radius < 0)) {
        raise_error(RANGE_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
        BeginPath(to_hdc(actual_window));
        MoveToEx(to_hdc(actual_window),
                 (int) x + (int) (cos(startAngle) * (double) radius),
                 (int) y - (int) (sin(startAngle) * (double) radius),
                 (LPPOINT) NULL);
        AngleArc(to_hdc(actual_window), (int) x, (int) y, (DWORD) radius, startAng, sweepAng);
        EndPath(to_hdc(actual_window));
        StrokeAndFillPath(to_hdc(actual_window));
        SelectObject(to_hdc(actual_window), old_pen);
        SelectObject(to_hdc(actual_window), old_brush);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
          BeginPath(to_backup_hdc(actual_window));
          MoveToEx(to_backup_hdc(actual_window),
                   (int) x + (int) (cos(startAngle) * (double) radius),
                   (int) y - (int) (sin(startAngle) * (double) radius),
                   (LPPOINT) NULL);
          AngleArc(to_backup_hdc(actual_window), (int) x, (int) y, (DWORD) radius, startAng, sweepAng);
          EndPath(to_backup_hdc(actual_window));
          StrokeAndFillPath(to_backup_hdc(actual_window));
          SelectObject(to_backup_hdc(actual_window), old_pen);
          SelectObject(to_backup_hdc(actual_window), old_brush);
        } /* if */
        DeleteObject(current_pen);
        DeleteObject(current_brush);
      } /* if */
    } /* if */
  } /* drwPFArcChord */



void drwFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    FLOAT startAng, sweepAng;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFArcPieSlice */
    logFunction(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (sweepAngle != 0.0) {
      startAng = (FLOAT) (startAngle * (360.0 / (2 * PI)));
      sweepAng = (FLOAT) (sweepAngle * (360.0 / (2 * PI)));
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      current_brush = CreateSolidBrush((COLORREF) col);
      if (unlikely(current_pen == NULL || current_brush == NULL)) {
        raise_error(MEMORY_ERROR);
      } else if (unlikely(!inIntRange(x) || !inIntRange(x) ||
                          !inIntRange(radius) || radius < 0)) {
        raise_error(RANGE_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
        BeginPath(to_hdc(actual_window));
        MoveToEx(to_hdc(actual_window), (int) x, (int) y, (LPPOINT) NULL);
        AngleArc(to_hdc(actual_window), (int) x, (int) y, (DWORD) radius, startAng, sweepAng);
        LineTo(to_hdc(actual_window), (int) x, (int) y);
        EndPath(to_hdc(actual_window));
        StrokeAndFillPath(to_hdc(actual_window));
        SelectObject(to_hdc(actual_window), old_pen);
        SelectObject(to_hdc(actual_window), old_brush);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
          BeginPath(to_backup_hdc(actual_window));
          MoveToEx(to_backup_hdc(actual_window), (int) x, (int) y, (LPPOINT) NULL);
          AngleArc(to_backup_hdc(actual_window), (int) x, (int) y, (DWORD) radius, startAng, sweepAng);
          LineTo(to_backup_hdc(actual_window), (int) x, (int) y);
          EndPath(to_backup_hdc(actual_window));
          StrokeAndFillPath(to_backup_hdc(actual_window));
          SelectObject(to_backup_hdc(actual_window), old_pen);
          SelectObject(to_backup_hdc(actual_window), old_brush);
        } /* if */
        DeleteObject(current_pen);
        DeleteObject(current_brush);
      } /* if */
    } /* if */
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
    HWND hWnd;
    RECT windowRect;
    RECT clientRect;
    POINT clientTopLeft;
    POINT clientBottomRight;
    rtlArrayType border;

  /* drwBorder */
    logFunction(printf("drwBorder(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    hWnd = to_hwnd(actual_window);
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      border = NULL;
    } else if (unlikely(GetWindowRect(hWnd, &windowRect) == 0 ||
                        GetClientRect(hWnd, &clientRect) == 0)) {
      raise_error(GRAPHIC_ERROR);
      border = NULL;
    } else {
      clientTopLeft.x = clientRect.left;
      clientTopLeft.y = clientRect.top;
      clientBottomRight.x = clientRect.right;
      clientBottomRight.y = clientRect.bottom;
      if (unlikely(ClientToScreen(hWnd, &clientTopLeft) == 0 ||
                   ClientToScreen(hWnd, &clientBottomRight) == 0)) {
        raise_error(GRAPHIC_ERROR);
        border = NULL;
      } else {
        if (unlikely(!ALLOC_RTL_ARRAY(border, 4))) {
          raise_error(MEMORY_ERROR);
        } else {
          border->min_position = 1;
          border->max_position = 4;
          border->arr[0].value.intValue = (intType) (clientTopLeft.y - windowRect.top);
          border->arr[1].value.intValue = (intType) (windowRect.right - clientBottomRight.x);
          border->arr[2].value.intValue = (intType) (windowRect.bottom - clientBottomRight.y);
          border->arr[3].value.intValue = (intType) (clientTopLeft.x - windowRect.left);
        } /* if */
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
    AngleArc(to_hdc(actual_window), castToInt(x), castToInt(y), (unsigned) radius, 0.0, 360.0);
  } /* drwCircle */



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;

  /* drwPCircle */
    logFunction(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (unlikely(current_pen == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      MoveToEx(to_hdc(actual_window), castToInt(x + radius), castToInt(y), NULL);
      AngleArc(to_hdc(actual_window), castToInt(x), castToInt(y), (unsigned) radius, 0.0, 360.0);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        MoveToEx(to_backup_hdc(actual_window), castToInt(x + radius), castToInt(y), NULL);
        AngleArc(to_backup_hdc(actual_window), castToInt(x), castToInt(y), (unsigned) radius, 0.0, 360.0);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPCircle */



void drwClear (winType actual_window, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwClear */
    logFunction(printf("drwClear(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, col););
    to_var_clear_col(actual_window) = col;
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      /* The main window is cleared with the real window size. */
      Rectangle(to_hdc(actual_window), 0, 0,
          (int) drwWidth(actual_window), (int) drwHeight(actual_window));
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Rectangle(to_backup_hdc(actual_window), 0, 0,
            (int) to_backupWidth(actual_window), (int) to_backupHeight(actual_window));
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwClear */



/**
 *  Copy a rectangular area from 'src_window' to 'dest_window'.
 *  Coordinates are measured relative to the top left corner of the
 *  corresponding window drawing area (inside of the window decorations).
 *  @param src_window Source window.
 *  @param dest_window Destination window.
 *  @param src_x X-position of the top left corner of the source area.
 *  @param src_y Y-position of the top left corner of the source area.
 *  @param width Width of the rectangular area.
 *  @param height Height of the rectangular area.
 *  @param dest_x X-position of the top left corner of the destination area.
 *  @param dest_y Y-position of the top left corner of the destination area.
 */
void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y)

  { /* drwCopyArea */
    logFunction(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) src_window, (memSizeType) dest_window,
                       src_x, src_y, width, height, dest_x, dest_y););
    if (unlikely(!inIntRange(src_x) || !inIntRange(src_y) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 !inIntRange(dest_x) || !inIntRange(dest_y) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else if (to_backup_hdc(src_window) != 0) {
      BitBlt(to_hdc(dest_window), (int) dest_x, (int) dest_y, (int) width, (int) height,
          to_backup_hdc(src_window), (int) src_x, (int) src_y, SRCCOPY);
      if (to_backup_hdc(dest_window) != 0) {
        BitBlt(to_backup_hdc(dest_window), (int) dest_x, (int) dest_y, (int) width, (int) height,
            to_backup_hdc(src_window), (int) src_x, (int) src_y, SRCCOPY);
      } /* if */
    } else {
      BitBlt(to_hdc(dest_window), (int) dest_x, (int) dest_y, (int) width, (int) height,
          to_hdc(src_window), (int) src_x, (int) src_y, SRCCOPY);
      if (to_backup_hdc(dest_window) != 0) {
        BitBlt(to_backup_hdc(dest_window), (int) dest_x, (int) dest_y, (int) width, (int) height,
            to_hdc(src_window), (int) src_x, (int) src_y, SRCCOPY);
      } /* if */
    } /* if */
  } /* drwCopyArea */



void drwFCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwFCircle */
  } /* drwFCircle */



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFCircle */
    logFunction(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      Ellipse(to_hdc(actual_window), castToInt(x - radius), castToInt(y - radius),
          castToInt(x + radius + 1), castToInt(y + radius + 1));
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Ellipse(to_backup_hdc(actual_window), castToInt(x - radius), castToInt(y - radius),
            castToInt(x + radius + 1), castToInt(y + radius + 1));
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPFCircle */



void drwFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwFEllipse */
  } /* drwFEllipse */



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFEllipse */
    logFunction(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, width, height, col););
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      Ellipse(to_hdc(actual_window), castToInt(x), castToInt(y),
              castToInt(x + width), castToInt(y + height));
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Ellipse(to_backup_hdc(actual_window), castToInt(x), castToInt(y),
                castToInt(x + width), castToInt(y + height));
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPFEllipse */



winType drwEmpty (void)

  { /* drwEmpty */
    logFunction(printf("drwEmpty()\n"););
    if (!init_called) {
      drawInit();
    } /* if */
    logFunction(printf("drwEmpty --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) emptyWindow,
                       emptyWindow != NULL ? emptyWindow->usage_count : (uintType) 0););
    return emptyWindow;
  } /* drwEmpty */



void drwFree (winType old_window)

  { /* drwFree */
    logFunction(printf("drwFree(" FMT_U_MEM ") (usage=" FMT_U ")\n",
                       (memSizeType) old_window,
                       old_window != NULL ? old_window->usage_count : (uintType) 0););
    if (is_pixmap(old_window)) {
      SelectObject(to_hdc(old_window), to_oldBitmap(old_window));
      DeleteObject(to_hBitmap(old_window));
      DeleteDC(to_hdc(old_window));
    } else {
      DeleteObject(to_backup(old_window));
      DeleteDC(to_backup_hdc(old_window));
      DeleteDC(to_hdc(old_window));
      DestroyWindow(to_hwnd(old_window));
      remove_window(to_hwnd(old_window));
    } /* if */
    FREE_RECORD2(old_window, win_winRecord, count.win, count.win_bytes);
  } /* drwFree */



/**
 *  Capture a rectangular area from the screen.
 *  The function takes a screenshot of the rectangular area.
 *  The 'left' and 'upper' coordinates are measured relative to
 *  the top left corner of the screen.
 *  @param left X-position of the upper left corner of the capture area.
 *  @param upper Y-position of the upper left corner of the capture area.
 *  @param width Width of the capture area.
 *  @param height Height of the capture area.
 *  @return the content of the rectangular screen area as pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative.
 */
winType drwCapture (intType left, intType upper,
    intType width, intType height)

  {
    HDC screenDC;
    int horizRes;
    int vertRes;
    int desktopHorizRes;
    int desktopVertRes;
    win_winType pixmap;

  /* drwCapture */
    logFunction(printf("drwCapture(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(!ALLOC_RECORD2(pixmap, win_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(pixmap, 0, sizeof(win_winRecord));
      pixmap->usage_count = 1;
      screenDC = GetDC(NULL);
      pixmap->hdc = CreateCompatibleDC(screenDC);
      pixmap->hBitmap = CreateCompatibleBitmap(screenDC, (int) width, (int) height);
      if (unlikely(pixmap->hBitmap == NULL)) {
        free(pixmap);
        pixmap = NULL;
        ReleaseDC(NULL, screenDC);
        raise_error(MEMORY_ERROR);
      } else {
        pixmap->oldBitmap = (HBITMAP) SelectObject(pixmap->hdc, pixmap->hBitmap);
        pixmap->hasTransparentPixel = FALSE;
        pixmap->transparentPixel = 0;
        pixmap->is_pixmap = TRUE;
        pixmap->width = (unsigned int) width;
        pixmap->height = (unsigned int) height;
        horizRes = GetDeviceCaps(screenDC, HORZRES);
        vertRes = GetDeviceCaps(screenDC, VERTRES);
        desktopHorizRes = GetDeviceCaps(screenDC, DESKTOPHORZRES);
        desktopVertRes = GetDeviceCaps(screenDC, DESKTOPVERTRES);
        if (horizRes == desktopHorizRes && vertRes == desktopVertRes) {
          BitBlt(pixmap->hdc, 0, 0, (int) width, (int) height,
                 screenDC, (int) left, (int) upper, SRCCOPY);
        } else {
          SetStretchBltMode(pixmap->hdc, COLORONCOLOR);
          StretchBlt(pixmap->hdc, 0, 0, (int) width, (int) height, screenDC,
                     (int) (left * desktopHorizRes / horizRes),
                     (int) (upper * desktopVertRes / vertRes),
                     (int) (width * desktopHorizRes / horizRes),
                     (int) (height * desktopVertRes / vertRes), SRCCOPY);
        } /* if */
        ReleaseDC(NULL, screenDC);
      } /* if */
    } /* if */
    logFunction(printf("drwCapture --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwCapture */



intType drwGetPixel (const_winType sourceWindow, intType x, intType y)

  {
    intType pixel;

  /* drwGetPixel */
    logFunction(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, x, y););
    pixel = (intType) GetPixel(to_hdc(sourceWindow), castToInt(x), castToInt(y));
    logFunction(printf("drwGetPixel --> " F_X(08) "\n", pixel););
    return pixel;
  } /* drwGetPixel */



bstriType drwGetPixelData (const_winType sourceWindow)

  {
    unsigned int xPos;
    unsigned int yPos;
    memSizeType result_size;
    uint32Type *image_data;
    bstriType result;

  /* drwGetPixelData */
    logFunction(printf("drwGetPixelData(" FMT_U_MEM ")\n", (memSizeType) sourceWindow););
    result_size = to_width(sourceWindow) * to_height(sourceWindow) * sizeof(uint32Type);
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      image_data = (uint32Type *) result->mem;
      for (yPos = 0; yPos < to_height(sourceWindow); yPos++) {
        for (xPos = 0; xPos < to_width(sourceWindow); xPos++) {
          image_data[yPos * to_width(sourceWindow) + xPos] =
              (uint32Type) GetPixel(to_hdc(sourceWindow), (int) xPos, (int) yPos);
        } /* for */
      } /* for */
    } /* if */
    return result;
  } /* drwGetPixelData */



/**
 *  Create a new pixmap with the given 'width' and 'height' from 'sourceWindow'.
 *  A rectangle with the upper left corner at ('left', 'upper') and the given
 *  'width' and 'height' is copied from 'sourceWindow' to the new pixmap.
 *  The rectangle may extend to areas outside of 'sourceWindow'. The rectangle
 *  areas outside of 'sourceWindow' are colored with black.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative.
 *  @return the new pixmap.
 */
winType drwGetPixmap (const_winType sourceWindow, intType left, intType upper,
    intType width, intType height)

  {
    win_winType pixmap;

  /* drwGetPixmap */
    logFunction(printf("drwGetPixmap(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(!ALLOC_RECORD2(pixmap, win_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(pixmap, 0, sizeof(win_winRecord));
      pixmap->usage_count = 1;
      pixmap->hdc = CreateCompatibleDC(to_hdc(sourceWindow));
      pixmap->hBitmap = CreateCompatibleBitmap(to_hdc(sourceWindow), (int) width, (int) height);
      if (unlikely(pixmap->hBitmap == NULL)) {
        free(pixmap);
        pixmap = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        pixmap->oldBitmap = (HBITMAP) SelectObject(pixmap->hdc, pixmap->hBitmap);
        pixmap->hasTransparentPixel = FALSE;
        pixmap->transparentPixel = 0;
        pixmap->is_pixmap = TRUE;
        pixmap->width = (unsigned int) width;
        pixmap->height = (unsigned int) height;
        if (to_backup_hdc(sourceWindow) != 0) {
          BitBlt(pixmap->hdc, 0, 0, (int) width, (int) height,
              to_backup_hdc(sourceWindow), (int) left, (int) upper, SRCCOPY);
        } else {
          BitBlt(pixmap->hdc, 0, 0, (int) width, (int) height,
              to_hdc(sourceWindow), (int) left, (int) upper, SRCCOPY);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("drwGetPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwGetPixmap */



/**
 *  Determine the height of the window drawing area in pixels.
 *  This excludes window decorations at top and bottom. Add top and bottom
 *  border widths to get the height inclusive window decorations.
 */
intType drwHeight (const_winType actual_window)

  {
    RECT rect;
    intType height;

  /* drwHeight */
    logFunction(printf("drwHeight(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    if (is_pixmap(actual_window) ||
        GetClientRect(to_hwnd(actual_window), &rect) == 0) {
      height = to_height(actual_window);
    } else {
      height = (intType) ((unsigned int) (rect.bottom - rect.top));
    } /* if */
    logFunction(printf("drwHeight(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, height););
    return height;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height,
    boolType hasAlphaChannel)

  {
    memSizeType pos;
    int32Type color;
    HDC screenDC;
    win_winType pixmap;

  /* drwImage */
    logFunction(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       width, height, hasAlphaChannel););
    if (unlikely(width < 1 || width > INTTYPE_MAX ||
                 height < 1 || height > INTTYPE_MAX)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!ALLOC_RECORD2(pixmap, win_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(win_winRecord));
        pixmap->usage_count = 1;
        pos = height * width;
        do {
          pos--;
          color = image_data[pos];
          image_data[pos] = (GetRValue(color) << 16) | (GetGValue(color) << 8) | GetBValue(color);
        } while (pos != 0);
        screenDC = GetDC(NULL);
        pixmap->hdc = CreateCompatibleDC(screenDC);
        pixmap->hBitmap = CreateBitmap((int) width, (int) height, 1, 32, image_data);
        ReleaseDC(NULL, screenDC);
        pixmap->oldBitmap = (HBITMAP) SelectObject(pixmap->hdc, pixmap->hBitmap);
        pixmap->hasTransparentPixel = FALSE;
        pixmap->transparentPixel = 0;
        pixmap->is_pixmap = TRUE;
        pixmap->width = (unsigned int) width;
        pixmap->height = (unsigned int) height;
      } /* if */
    } /* if */
    logFunction(printf("drwImage --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwImage */



void drwLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2)

  { /* drwLine */
    MoveToEx(to_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
    LineTo(to_hdc(actual_window), castToInt(x2), castToInt(y2));
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
      LineTo(to_backup_hdc(actual_window), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwLine */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;

  /* drwPLine */
    logFunction(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2, col););
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (unlikely(current_pen == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      MoveToEx(to_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
      LineTo(to_hdc(actual_window), castToInt(x2), castToInt(y2));
      SetPixel(to_hdc(actual_window), castToInt(x2), castToInt(y2), (COLORREF) col);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        MoveToEx(to_backup_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
        LineTo(to_backup_hdc(actual_window), castToInt(x2), castToInt(y2));
        SetPixel(to_backup_hdc(actual_window), castToInt(x2), castToInt(y2), (COLORREF) col);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPLine */



winType drwNewPixmap (intType width, intType height)

  {
    HDC screenDC;
    win_winType pixmap;

  /* drwNewPixmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(!inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!ALLOC_RECORD2(pixmap, win_winRecord, count.win, count.win_bytes))) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(pixmap, 0, sizeof(win_winRecord));
        pixmap->usage_count = 1;
        screenDC = GetDC(NULL);
        pixmap->hdc = CreateCompatibleDC(screenDC);
        pixmap->hBitmap = CreateCompatibleBitmap(screenDC, (int) width, (int) height);
        ReleaseDC(NULL, screenDC);
        pixmap->oldBitmap = (HBITMAP) SelectObject(pixmap->hdc, pixmap->hBitmap);
        pixmap->hasTransparentPixel = FALSE;
        pixmap->transparentPixel = 0;
        pixmap->is_pixmap = TRUE;
        pixmap->width = (unsigned int) width;
        pixmap->height = (unsigned int) height;
      } /* if */
    } /* if */
    logFunction(printf("drwNewPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwNewPixmap */



winType drwNewBitmap (const_winType actual_window, intType width, intType height)

  {
    win_winType result;

  /* drwNewBitmap */
    logFunction(printf("drwNewBitmap(" FMT_D ", " FMT_D ")\n", width, height););
    result = NULL;
    logFunction(printf("drwNewBitmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwNewBitmap */



static boolType privateConsole (void)

  {
    HWND consoleWnd;
    DWORD dwProcessId;
    CONSOLE_SCREEN_BUFFER_INFO conBufInfo;
    boolType private;

  /* privateConsole */
    logFunction(printf("privateConsole\n"););
    if (pGetConsoleWindow != NULL) {
      consoleWnd = pGetConsoleWindow();
      GetWindowThreadProcessId(consoleWnd, &dwProcessId);
      private = GetCurrentProcessId() == dwProcessId;
    } else if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conBufInfo)) {
      private = FALSE;
    } else {
      /* If the cursor position is (0,0) the console is assumed to be private. */
      private = conBufInfo.dwCursorPosition.X == 0 && conBufInfo.dwCursorPosition.Y == 0;
    } /* if */
    logFunction(printf("privateConsole --> %d\n", private););
    return private;
  } /* privateConsole */



winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType windowName)

  {
    RECT windowRect;
    os_striType winName;
    HFONT std_font;
    errInfoType err_info = OKAY_NO_ERROR;
    win_winType newWindow = NULL;

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
      raise_error(RANGE_ERROR);
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (init_called) {
        windowRect.left = 0;
        windowRect.top = 0;
        windowRect.right = (long) width;
        windowRect.bottom = (long) height;
        if (unlikely(AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0) == 0 ||
                     windowRect.right < 0 ||
                     windowRect.right - INT_MAX > windowRect.left ||
                     windowRect.bottom < 0 ||
                     windowRect.bottom - INT_MAX > windowRect.top)) {
          /* Unable to compute window width or height that includes decorations. */
          raise_error(RANGE_ERROR);
        } else if (unlikely((winName = stri_to_os_stri(windowName, &err_info)) == NULL)) {
          raise_error(err_info);
        } else {
          if (privateConsole()) {
            /* printf("private_session\n"); */
            if (pGetConsoleWindow != NULL) {
              ShowWindow(pGetConsoleWindow(), SW_HIDE);
            } else {
              FreeConsole();
            } /* if */
          } /* if */
          if (ALLOC_RECORD2(newWindow, win_winRecord, count.win, count.win_bytes)) {
            memset(newWindow, 0, sizeof(win_winRecord));
            newWindow->usage_count = 1;
            newWindow->hWnd = CreateWindowW(windowClassW, winName,
                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                (int) xPos, (int) yPos,
                (int) (windowRect.right - windowRect.left),
                (int) (windowRect.bottom - windowRect.top),
                (HWND) NULL, (HMENU) NULL, NULL, NULL);
            enter_window((winType) newWindow, newWindow->hWnd);
            /* printf("hWnd=%lu\n", newWindow->hWnd); */
            if (newWindow->hWnd != NULL) {
              newWindow->hdc = GetDC(newWindow->hWnd);
              /* printf("hdc=%lu\n", newWindow->hdc); */
              newWindow->hasTransparentPixel = FALSE;
              newWindow->transparentPixel = 0;
              newWindow->is_pixmap = FALSE;
              newWindow->width = (unsigned int) width;
              newWindow->height = (unsigned int) height;
              newWindow->backupWidth = (unsigned int) width;
              newWindow->backupHeight = (unsigned int) height;
              newWindow->minimized = FALSE;
              newWindow->clear_col = (intType) RGB(0, 0, 0); /* black */
              newWindow->cursorVisible = TRUE;
              newWindow->backup_hdc = CreateCompatibleDC(newWindow->hdc);
              newWindow->backup = CreateCompatibleBitmap(newWindow->hdc, (int) width, (int) height);
              SelectObject(newWindow->backup_hdc, newWindow->backup);
              std_font = CreateFont(16, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                  DEFAULT_QUALITY, FIXED_PITCH | FF_SWISS, NULL);
              SelectObject(newWindow->hdc, std_font);
              SelectObject(newWindow->backup_hdc, std_font);
              ShowWindow(newWindow->hWnd, SW_SHOWDEFAULT);
              UpdateWindow(newWindow->hWnd);
            } /* if */
          } /* if */
          os_stri_free(winName);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("drwOpen --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
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
    HFONT std_font;
    win_winType newWindow = NULL;

  /* drwOpenSubWindow */
    logFunction(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      xPos, yPos, width, height););
      raise_error(RANGE_ERROR);
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (init_called) {
        if (ALLOC_RECORD2(newWindow, win_winRecord, count.win, count.win_bytes)) {
          memset(newWindow, 0, sizeof(win_winRecord));
          newWindow->usage_count = 1;
          if (to_width(parent_window) == 0 && to_height(parent_window) == 0) {
            newWindow->hWnd = CreateWindowExW(WS_EX_NOACTIVATE, windowClassW, L"",
                WS_POPUP,
                (int) xPos, (int) yPos, (int) width, (int) height,
                (HWND) NULL, (HMENU) NULL, NULL, NULL);
          } else {
            newWindow->hWnd = CreateWindowW(windowClassW, L"",
                WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                (int) xPos, (int) yPos, (int) width, (int) height,
                to_hwnd(parent_window), (HMENU) NULL, NULL, NULL);
          } /* if */
          enter_window((winType) newWindow, newWindow->hWnd);
          /* printf("hWnd=%lu\n", newWindow->hWnd); */
          if (newWindow->hWnd != NULL) {
            SetWindowLong(newWindow->hWnd , GWL_STYLE, GetWindowLong(newWindow->hWnd , GWL_STYLE) &~ WS_CAPTION);
            SetWindowPos(newWindow->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            newWindow->hdc = GetDC(newWindow->hWnd);
            /* printf("hdc=%lu\n", newWindow->hdc); */
            newWindow->hasTransparentPixel = FALSE;
            newWindow->transparentPixel = 0;
            newWindow->is_pixmap = FALSE;
            newWindow->width = (unsigned int) width;
            newWindow->height = (unsigned int) height;
            newWindow->backupWidth = (unsigned int) width;
            newWindow->backupHeight = (unsigned int) height;
            newWindow->minimized = FALSE;
            newWindow->clear_col = (intType) RGB(0, 0, 0); /* black */
            newWindow->cursorVisible = TRUE;
            newWindow->backup_hdc = CreateCompatibleDC(newWindow->hdc);
            newWindow->backup = CreateCompatibleBitmap(newWindow->hdc, (int) width, (int) height);
            SelectObject(newWindow->backup_hdc, newWindow->backup);
            std_font = CreateFont(16, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_SWISS, NULL);
            SelectObject(newWindow->hdc, std_font);
            SelectObject(newWindow->backup_hdc, std_font);
            ShowWindow(newWindow->hWnd, SW_SHOW /*SW_SHOWDEFAULT*/);
            UpdateWindow(newWindow->hWnd);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("drwOpenSubWindow --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
  } /* drwOpenSubWindow */



void drwSetCloseAction (winType actual_window, intType closeAction)

  { /* drwSetCloseAction */
    logFunction(printf("drwSetCloseAction(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) actual_window, closeAction););
    if (closeAction < 0 || closeAction > 2) {
      raise_error(RANGE_ERROR);
    } else {
      to_var_close_action(actual_window) = (int) closeAction;
    } /* if */
  } /* drwSetCloseAction */



/**
 *  Set the visibility of the mouse cursor in 'aWindow'.
 *  @param aWindow Window for which the mouse cursor visibility is set.
 *  @param visible TRUE, if the mouse cursor should be visible in 'aWindow', or
 *                 FALSE, if the mouse curser should be invisible in 'aWindow'.
 */
void drwSetCursorVisible (winType aWindow, boolType visible)

  {
    POINT point;

  /* drwSetCursorVisible */
    logFunction(printf("drwSetCursorVisible(" FMT_U_MEM ", %d)\n",
                       (memSizeType) aWindow, visible););
    to_var_cursorVisible(aWindow) = visible;
    if (GetCursorPos(&point) != 0) {
      SetCursorPos(point.x, point.y);
    } /* if */
  } /* drwSetCursorVisible */



/**
 *  Set the pointer x and y position relative to the specified 'aWindow'.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'aWindow' (inside of the window decorations).
 *  If 'aWindow' is the empty window the pointer x and y position
 *  is relative to the top left corner of the screen.
 */
void drwSetPointerPos (const_winType aWindow, intType xPos, intType yPos)

  {
    POINT point;

  /* drwSetPointerPos */
    logFunction(printf("drwSetPointerPos(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) aWindow, xPos, yPos););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos))) {
      raise_error(RANGE_ERROR);
    } else {
      if (to_width(aWindow) == 0 && to_height(aWindow) == 0) {
        SetCursorPos((int) xPos, (int) yPos);
      } else {
        point.x = (long) xPos;
        point.y = (long) yPos;
        if (unlikely(ClientToScreen(to_hwnd(aWindow), &point) == 0)) {
          raise_error(GRAPHIC_ERROR);
        } else {
          SetCursorPos(point.x, point.y);
        } /* if */
      } /* if */
    } /* if */
  } /* drwSetPointerPos */



void drwPoint (const_winType actual_window, intType x, intType y)

  { /* drwPoint */
    MoveToEx(to_hdc(actual_window), castToInt(x), castToInt(y), NULL);
    LineTo(to_hdc(actual_window), castToInt(x + 1), castToInt(y + 1));
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), castToInt(x), castToInt(y), NULL);
      LineTo(to_backup_hdc(actual_window), castToInt(x + 1), castToInt(y + 1));
    } /* if */
  } /* drwPoint */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  { /* drwPPoint */
    logFunction(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, col););
    SetPixel(to_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
    if (to_backup_hdc(actual_window) != 0) {
      SetPixel(to_backup_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
    } /* if */
  } /* drwPPoint */



rtlArrayType drwConvPointList (const const_bstriType pointList)

  {
    memSizeType len;
    POINT *points;
    memSizeType pos;
    rtlArrayType xyArray;

  /* drwConvPointList */
    logFunction(printf("drwConvPointList(\"%s\")\n",
                       bstriAsUnquotedCStri(pointList)););
    len = pointList->size / sizeof(POINT);
    if (unlikely(!ALLOC_RTL_ARRAY(xyArray, len << 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      xyArray->min_position = 1;
      xyArray->max_position = (intType) (len << 1);
      points = (POINT *) pointList->mem;
      for (pos = 0; pos < len; pos++) {
        xyArray->arr[ pos << 1     ].value.intValue = (intType) points[pos].x;
        xyArray->arr[(pos << 1) + 1].value.intValue = (intType) points[pos].y;
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
    POINT *points;
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
      if (unlikely(len > MAX_BSTRI_LEN / sizeof(POINT) || len > MAX_MEM_INDEX)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(POINT)))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len * sizeof(POINT);
          if (len > 0) {
            points = (POINT *) result->mem;
            for (pos = 0; pos < len; pos++) {
              points[pos].x = castToLong(xyArray->arr[ pos << 1     ].value.intValue);
              points[pos].y = castToLong(xyArray->arr[(pos << 1) + 1].value.intValue);
            } /* for */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGenPointList */



intType drwLngPointList (bstriType point_list)

  { /* drwLngPointList */
    return (intType) (point_list->size / sizeof(POINT));
  } /* drwLngPointList */



void drwPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    POINT *points;
    memSizeType numPoints;
    memSizeType pos;
    HPEN old_pen;
    HPEN current_pen;

  /* drwPolyLine */
    logFunction(printf("drwPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, (memSizeType) point_list, col););
    points = (POINT *) point_list->mem;
    numPoints = point_list->size / sizeof(POINT);
    if (numPoints >= 2) {
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      if (unlikely(current_pen == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        MoveToEx(to_hdc(actual_window), (int) x + points[0].x, (int) y + points[0].y, NULL);
        for (pos = 1; pos < numPoints; pos++) {
          LineTo(to_hdc(actual_window), (int) x + points[pos].x, (int) y + points[pos].y);
        } /* for */
        SetPixel(to_hdc(actual_window), (int) x + points[numPoints - 1].x, (int) y + points[numPoints - 1].y, (COLORREF) col);
        SelectObject(to_hdc(actual_window), old_pen);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          MoveToEx(to_backup_hdc(actual_window), (int) x + points[0].x, (int) y + points[0].y, NULL);
          for (pos = 1; pos < numPoints; pos++) {
            LineTo(to_backup_hdc(actual_window), (int) x + points[pos].x, (int) y + points[pos].y);
          } /* for */
          SetPixel(to_backup_hdc(actual_window), (int) x + points[numPoints - 1].x, (int) y + points[numPoints - 1].y, (COLORREF) col);
          SelectObject(to_backup_hdc(actual_window), old_pen);
        } /* if */
        DeleteObject(current_pen);
      } /* if */
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    POINT *points;
    memSizeType numPoints;
    memSizeType pos;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwFPolyLine */
    logFunction(printf("drwFPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, (memSizeType) point_list, col););
    points = (POINT *) point_list->mem;
    numPoints = point_list->size / sizeof(POINT);
    for (pos = 0; pos < numPoints; pos++) {
      points[pos].x += (int) x;
      points[pos].y += (int) y;
    } /* for */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL || numPoints > INT_MAX)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      Polygon(to_hdc(actual_window), points, (int) numPoints);
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Polygon(to_backup_hdc(actual_window), points, (int) numPoints);
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
    for (pos = 0; pos < numPoints; pos++) {
      points[pos].x -= (int) x;
      points[pos].y -= (int) y;
    } /* for */
  } /* drwFPolyLine */



void drwPut (const_winType destWindow, intType xDest, intType yDest,
    const_winType pixmap)

  {
    HDC hdcMem;

  /* drwPut */
    logFunction(printf("drwPut(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ")\n",
                       (memSizeType) destWindow, xDest, yDest, (memSizeType) pixmap););
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest))) {
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL) {
      if (to_hasTransparentPixel(pixmap)) {
        hdcMem = CreateCompatibleDC(0);
        SelectObject(hdcMem, to_maskBitmap(pixmap));
        BitBlt(to_hdc(destWindow), (int) xDest, (int) yDest,
               (int) to_width(pixmap), (int) to_height(pixmap), hdcMem, 0, 0, SRCAND);
        BitBlt(to_hdc(destWindow), (int) xDest, (int) yDest,
               (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCPAINT);
        if (to_backup_hdc(destWindow) != 0) {
          SelectObject(hdcMem, to_maskBitmap(pixmap));
          BitBlt(to_backup_hdc(destWindow), (int) xDest, (int) yDest,
                 (int) to_width(pixmap), (int) to_height(pixmap), hdcMem, 0, 0, SRCAND);
          BitBlt(to_backup_hdc(destWindow), (int) xDest, (int) yDest,
                 (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCPAINT);
        } /* if */
        DeleteDC(hdcMem);
      } else {
        BitBlt(to_hdc(destWindow), (int) xDest, (int) yDest,
               (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCCOPY);
        if (to_backup_hdc(destWindow) != 0) {
          BitBlt(to_backup_hdc(destWindow), (int) xDest, (int) yDest,
                 (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCCOPY);
        } /* if */
      } /* if */
    } /* if */
  } /* drwPut */



void drwPutScaled (const_winType destWindow, intType xDest, intType yDest,
    intType width, intType height, const_winType pixmap)

  { /* drwPutScaled */
    logFunction(printf("drwPutScaled(" FMT_U_MEM  ", " FMT_D ", " FMT_D ", "
                       FMT_D ", " FMT_D ", " FMT_U_MEM")\n",
                       (memSizeType) destWindow, xDest, yDest,
                       width, height, (memSizeType) pixmap););
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest) ||
                 !inIntRange(width) || width < 0 ||
                 !inIntRange(height) || height < 0)) {
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL) {
      SetStretchBltMode(to_hdc(destWindow), COLORONCOLOR);
      StretchBlt(to_hdc(destWindow), (int) xDest, (int) yDest,
                 (int) width, (int) height, to_hdc(pixmap), 0, 0,
                 (int) to_width(pixmap), (int) to_height(pixmap), SRCCOPY);
      if (to_backup_hdc(destWindow) != 0) {
        SetStretchBltMode(to_backup_hdc(destWindow), COLORONCOLOR);
        StretchBlt(to_backup_hdc(destWindow), (int) xDest, (int) yDest,
                   (int) width, (int) height, to_hdc(pixmap), 0, 0,
                   (int) to_width(pixmap), (int) to_height(pixmap), SRCCOPY);
      } /* if */
    } /* if */
  } /* drwPutScaled */



void drwRect (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwRect */
    logFunction(printf("drwRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
    Rectangle(to_hdc(actual_window), castToInt(x), castToInt(y),
              castToInt(x + width), castToInt(y + height));
    if (to_backup_hdc(actual_window) != 0) {
      Rectangle(to_backup_hdc(actual_window), castToInt(x), castToInt(y),
                castToInt(x + width), castToInt(y + height));
    } /* if */
  } /* drwRect */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPRect */
    logFunction(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, width, height, col););
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
#ifdef OUT_OF_ORDER
    if (width == 0 && height == 0) {
      printf("width == 0 && height == 0\n");
    } /* if */
    if (width == 1 && height == 1) {
      printf("width == 1 && height == 1\n");
    } /* if */
#endif
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (unlikely(current_pen == NULL || current_brush == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      if (width == 1) {
        if (height == 1) {
          SetPixel(to_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
        } else {
          MoveToEx(to_hdc(actual_window), castToInt(x), castToInt(y), NULL);
          LineTo(to_hdc(actual_window), castToInt(x), castToInt(y + height));
        } /* if */
      } else {
        if (height == 1) {
          MoveToEx(to_hdc(actual_window), castToInt(x), castToInt(y), NULL);
          LineTo(to_hdc(actual_window), castToInt(x + width), castToInt(y));
        } else {
          Rectangle(to_hdc(actual_window), castToInt(x), castToInt(y), castToInt(x + width), castToInt(y + height));
        } /* if */
      } /* if */
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        if (width == 1) {
          if (height == 1) {
            SetPixel(to_backup_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
          } else {
            MoveToEx(to_backup_hdc(actual_window), castToInt(x), castToInt(y), NULL);
            LineTo(to_backup_hdc(actual_window), castToInt(x), castToInt(y + height));
          } /* if */
        } else {
          if (height == 1) {
            MoveToEx(to_backup_hdc(actual_window), castToInt(x), castToInt(y), NULL);
            LineTo(to_backup_hdc(actual_window), castToInt(x + width), castToInt(y));
          } else {
            Rectangle(to_backup_hdc(actual_window), castToInt(x), castToInt(y), castToInt(x + width), castToInt(y + height));
          } /* if */
        } /* if */
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPRect */



intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  {
    intType col;

  /* drwRgbColor */
    col = (intType) RGB(((uintType) redLight) >> 8,
                        ((uintType) greenLight) >> 8,
                        ((uintType) blueLight) >> 8);
    logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> " F_X(08) "\n",
                       redLight, greenLight, blueLight, col););
    return col;
  } /* drwRgbColor */



void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight)

  { /* drwPixelToRgb */
    *redLight   = GetRValue(col) << 8;
    *greenLight = GetGValue(col) << 8;
    *blueLight  = GetBValue(col) << 8;
    logFunction(printf("drwPixelToRgb(" F_X(08) ", " FMT_D ", " FMT_D ", " FMT_D ") -->\n",
                       col, *redLight, *greenLight, *blueLight););
  } /* drwPixelToRgb */



void drwBackground (intType col)

  { /* drwBackground */
  } /* drwBackground */



void drwColor (intType col)

  { /* drwColor */
      /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
  } /* drwColor */



/**
 *  Determine the height of the screen in pixels.
 */
intType drwScreenHeight (void)

  {
    int height;

  /* drwScreenHeight */
    logFunction(printf("drwScreenHeight()\n"););
    height = GetSystemMetrics(SM_CYSCREEN);
    logFunction(printf("drwScreenHeight() --> %u\n", height););
    return (intType) height;
  } /* drwScreenHeight */



/**
 *  Determine the width of the screen in pixels.
 */
intType drwScreenWidth (void)

  {
    int width;

  /* drwScreenWidth */
    logFunction(printf("drwScreenWidth()\n"););
    width = GetSystemMetrics(SM_CXSCREEN);
    logFunction(printf("drwScreenWidth() --> %u\n", width););
    return (intType) width;
  } /* drwScreenWidth */



void drwSetContent (const_winType actual_window, const_winType pixmap)

  { /* drwSetContent */
    /* printf("begin drwSetContent(%lu, %lu)\n",
        to_hwnd(actual_window), to_hwnd(pixmap)); */
    if (pixmap != NULL) {
      BitBlt(to_hdc(actual_window), 0, 0,
             (int) to_width(pixmap), (int) to_height(pixmap),
             to_hdc(pixmap), 0, 0, SRCCOPY);
      if (to_backup_hdc(actual_window) != 0) {
        BitBlt(to_backup_hdc(actual_window), 0, 0,
               (int) to_width(pixmap), (int) to_height(pixmap),
               to_hdc(pixmap), 0, 0, SRCCOPY);
      } /* if */
    } /* if */
    /* printf("end drwSetContent(%lu, %lu)\n",
        to_hwnd(actual_window), to_hwnd(pixmap)); */
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

  { /* drwSetPos */
    logFunction(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, xPos, yPos););
    SetWindowPos(to_hwnd(actual_window), 0, castToInt(xPos), castToInt(yPos), 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOZORDER | SWP_NOSIZE);
    gkbInputReady();
  } /* drwSetPos */



void drwSetSize (winType actual_window, intType width, intType height)

  { /* drwSetSize */
    logFunction(printf("drwSetSize(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, width, height););
    if (unlikely(width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwSetSize(" FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      width, height););
      raise_error(RANGE_ERROR);
    } else {
      SetWindowPos(to_hwnd(actual_window), 0, 0, 0, (int) width, (int) height,
          /* SWP_NOSENDCHANGING | */ SWP_NOZORDER | SWP_NOMOVE);
      to_var_width(actual_window) = (unsigned int) width;
      to_var_height(actual_window) = (unsigned int) height;
    } /* if */
  } /* drwSetSize */



static HBITMAP createMaskBitmap (HDC pixmapHdc, int width, int height,
    COLORREF transparentColor)

  {
    HDC maskHdc;
    COLORREF oldBackgroundColor;
    HBITMAP maskBitmap;

  /* createMaskBitmap */
    /* Create monochrome (1 bit) maskBitmap. */
    maskBitmap = CreateBitmap(width, height, 1, 1, NULL);
    maskHdc = CreateCompatibleDC(0);
    SelectObject(maskHdc, maskBitmap);
    oldBackgroundColor = SetBkColor(pixmapHdc, transparentColor);
    /* Copy the bits from pixmapHdc to the B+W mask. */
    /* Everything with the background colour ends up */
    /* white and everything else ends up black.      */
    BitBlt(maskHdc, 0, 0, width, height, pixmapHdc, 0, 0, SRCCOPY);
    /* Take the new mask and use it to turn the      */
    /* transparent colour in the pixmapHdc image to  */
    /* black so the transparency effect works right. */
    BitBlt(pixmapHdc, 0, 0, width, height, maskHdc, 0, 0, SRCINVERT);
    DeleteDC(maskHdc);
    SetBkColor(pixmapHdc, oldBackgroundColor);
    return maskBitmap;
  } /* createMaskBitmap */



void drwSetTransparentColor (winType pixmap, intType col)

  { /* drwSetTransparentColor */
    logFunction(printf("drwSetTransparentColor(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) pixmap, col););
    to_var_hasTransparentPixel(pixmap) = TRUE;
    to_var_transparentPixel(pixmap) = (UINT) col;
    to_var_maskBitmap(pixmap) = createMaskBitmap(to_hdc(pixmap),
        (int) to_width(pixmap), (int) to_height(pixmap), (COLORREF) col);
  } /* drwSetTransparentColor */



void drwSetWindowName (winType aWindow, const const_striType windowName)

  {
    os_striType winName;
    BOOL funcRes;
    errInfoType err_info = OKAY_NO_ERROR;

  /* drwSetWindowName */
    logFunction(printf("drwSetWindowName(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) aWindow,
                       striAsUnquotedCStri(windowName)););
    winName = stri_to_os_stri(windowName, &err_info);
    if (unlikely(winName == NULL)) {
      raise_error(err_info);
    } else {
      funcRes = SetWindowTextW(to_hwnd(aWindow), (LPCWSTR) winName);
      os_stri_free(winName);
      if (unlikely(funcRes == 0)) {
        logError(printf("SetWindowTextW(" FMT_U_MEM ", \"%s\") failed\n",
                        (memSizeType) aWindow,
                        striAsUnquotedCStri(windowName)););
        raise_error(GRAPHIC_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("drwSetWindowName -->\n"););
  } /* drwSetWindowName */



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  {
    wchar_t *stri_buffer;
    wchar_t *wstri;
    const strElemType *strelem;
    memSizeType len;

  /* drwText */
    logFunction(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D ", \"%s\", "
                       F_X(08) ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       striAsUnquotedCStri(stri), col, bkcol););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 stri->size >= (unsigned int) INT_MAX)) {
      raise_error(RANGE_ERROR);
    } else if (unlikely(stri->size > MAX_UTF16_LEN ||
                        !ALLOC_UTF16(stri_buffer, stri->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      wstri = stri_buffer;
      strelem = stri->mem;
      len = stri->size;
      for (; len > 0; wstri++, strelem++, len--) {
        if (unlikely(*strelem >= 65536)) {
          UNALLOC_UTF16(stri_buffer, stri->size);
          raise_error(RANGE_ERROR);
          return;
        } /* if */
        *wstri = (wchar_t) *strelem;
      } /* for */

      SetTextColor(to_hdc(actual_window), (COLORREF) col);
      SetBkColor(to_hdc(actual_window), (COLORREF) bkcol);
      SetTextAlign(to_hdc(actual_window), TA_BASELINE | TA_LEFT);
      TextOutW(to_hdc(actual_window), (int) x, (int) y, stri_buffer, (int) stri->size);
      if (to_backup_hdc(actual_window) != 0) {
        SetTextColor(to_backup_hdc(actual_window), (COLORREF) col);
        SetBkColor(to_backup_hdc(actual_window), (COLORREF) bkcol);
        SetTextAlign(to_backup_hdc(actual_window), TA_BASELINE | TA_LEFT);
        TextOutW(to_backup_hdc(actual_window), (int) x, (int) y, stri_buffer, (int) stri->size);
      } /* if */
      UNALLOC_UTF16(stri_buffer, stri->size);
    } /* if */
  } /* drwText */



/**
 *  Lower a window to the bottom so that it does not obscure any other window.
 */
void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
    /* printf("begin drwToBottom(%lu)\n", to_hwnd(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_BOTTOM, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToBottom(%lu)\n", to_hwnd(actual_window)); */
  } /* drwToBottom */



/**
 *  Raise a window to the top so that no other window obscures it.
 */
void drwToTop (const_winType actual_window)

  { /* drwToTop */
    /* printf("begin drwToTop(%lu)\n", to_hwnd(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_TOP, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToTop(%lu)\n", to_hwnd(actual_window)); */
  } /* drwToTop */



/**
 *  Determine the width of the window drawing area in pixels.
 *  This excludes window declarations left and right. Add left and right
 *  border widths to get the width inclusive window decorations.
 */
intType drwWidth (const_winType actual_window)

  {
    RECT rect;
    intType width;

  /* drwWidth */
    logFunction(printf("drwWidth(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    if (is_pixmap(actual_window) ||
        GetClientRect(to_hwnd(actual_window), &rect) == 0) {
      width = to_width(actual_window);
    } else {
      width = (intType) ((unsigned int) (rect.right - rect.left));
    } /* if */
    logFunction(printf("drwWidth(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, width););
    return width;
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
    RECT rect;
    POINT point;
    intType xPos;

  /* drwXPos */
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      xPos = 0;
    } else if (unlikely(GetWindowRect(to_hwnd(actual_window), &rect) == 0)) {
      raise_error(GRAPHIC_ERROR);
      xPos = 0;
    } else {
      point.x = rect.left;
      point.y = rect.top;
      ScreenToClient(GetParent(to_hwnd(actual_window)), &point);
      xPos = point.x;
    } /* if */
    logFunction(printf("drwXPos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, xPos););
    return xPos;
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
    RECT rect;
    POINT point;
    intType yPos;

  /* drwYPos */
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      yPos = 0;
    } else if (unlikely(GetWindowRect(to_hwnd(actual_window), &rect) == 0)) {
      raise_error(GRAPHIC_ERROR);
      yPos = 0;
    } else {
      point.x = rect.left;
      point.y = rect.top;
      ScreenToClient(GetParent(to_hwnd(actual_window)), &point);
      yPos = point.y;
    } /* if */
    logFunction(printf("drwYPos(" FMT_U_MEM ") --> " FMT_D "\n",
                       (memSizeType) actual_window, yPos););
    return yPos;
  } /* drwYPos */
