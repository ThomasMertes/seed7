/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilities.    */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 2005 - 2007, 2013  Thomas Mertes                       */
/*  Content: Graphic access using the windows capabilities.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
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


#define PI 3.141592653589793238462643383279502884197

#define windowClass "MyWindowClass"

static inttype init_called = 0;


typedef struct win_winstruct {
    uinttype usage_count;
    /* Up to here the structure is identical to struct winstruct */
    HWND hWnd;
    HDC hdc;
    HBITMAP backup;
    HDC backup_hdc;
    HBITMAP hBitmap;
    HBITMAP oldBitmap;
    booltype hasTransparentPixel;
    UINT transparentPixel;
    booltype is_pixmap;
    unsigned int width;
    unsigned int height;
    unsigned int brutto_width_delta;
    unsigned int brutto_height_delta;
    inttype clear_col;
  } win_winrecord, *win_wintype;

#ifdef DO_HEAP_STATISTIC
size_t sizeof_winrecord = sizeof(win_winrecord);
#endif

#define to_hwnd(win)                 (((win_wintype) win)->hWnd)
#define to_hdc(win)                  (((win_wintype) win)->hdc)
#define to_backup_hdc(win)           (((win_wintype) win)->backup_hdc)
#define to_backup(win)               (((win_wintype) win)->backup)
#define to_hBitmap(win)              (((win_wintype) win)->hBitmap)
#define to_oldBitmap(win)            (((win_wintype) win)->oldBitmap)
#define is_pixmap(win)               (((win_wintype) win)->is_pixmap)
#define to_hasTransparentPixel(win)  (((win_wintype) win)->hasTransparentPixel)
#define to_transparentPixel(win)     (((win_wintype) win)->transparentPixel)
#define to_width(win)                (((win_wintype) win)->width)
#define to_height(win)               (((win_wintype) win)->height)
#define to_brutto_width_delta(win)   (((win_wintype) win)->brutto_width_delta)
#define to_brutto_height_delta(win)  (((win_wintype) win)->brutto_height_delta)
#define to_clear_col(win)            (((win_wintype) win)->clear_col)

#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE 674
#endif

static HWND (WINAPI *pGetConsoleWindow)(void) = NULL;



wintype find_window (HWND sys_window);
void enter_window (wintype curr_window, HWND sys_window);
void remove_window (HWND sys_window);



static void drawRectangle (win_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drawRectangle */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(actual_window->hdc, current_pen);
      old_brush = (HBRUSH) SelectObject(actual_window->hdc, current_brush);
      if (x1 == x2) {
        if (y1 == y2) {
          SetPixel(actual_window->hdc, castToInt(x1), castToInt(y1), (COLORREF) col);
        } else {
          MoveToEx(actual_window->hdc, castToInt(x1), castToInt(y1), NULL);
          LineTo(actual_window->hdc, castToInt(x1), castToInt(y2 + 1));
        } /* if */
      } else {
        if (y1 == y2) {
          MoveToEx(actual_window->hdc, castToInt(x1), castToInt(y1), NULL);
          LineTo(actual_window->hdc, castToInt(x2 + 1), castToInt(y1));
        } else {
          Rectangle(actual_window->hdc, castToInt(x1), castToInt(y1), castToInt(x2 + 1), castToInt(y2 + 1));
        } /* if */
      } /* if */
      SelectObject(actual_window->hdc, old_pen);
      SelectObject(actual_window->hdc, old_brush);
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drawRectangle */



LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    win_wintype paint_window;
    PAINTSTRUCT paintStruct;
    RECT rect;
    RECT rect2;
    /* MSG msg; */
    LRESULT result;

  /* WndProc */
    /* printf("WndProc message=%d, %lu, %d, %u\n", message, hWnd, wParam, lParam); */
    switch (message) {
      case WM_PAINT:
        /* printf("WM_PAINT %lu %lu\n", hWnd, paint_window); */
        paint_window = (win_wintype) find_window(hWnd);
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
          printf("paint_window=%lu, backup_hdc=%lu\n",
              (long unsigned int) paint_window,
              (long unsigned int) paint_window->backup_hdc);
        } /* if */
        result = 0;
        break;
      case WM_ERASEBKGND:
        /* printf("WM_ERASEBKGND %lu\n", hWnd); */
        paint_window = (win_wintype) find_window(hWnd);
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
              rect2.left = paint_window->width;
            } else {
              rect2.left = rect.left;
            } /* if */
            if (rect.bottom >= 0 && (unsigned int) rect.bottom >= paint_window->height) {
              rect2.bottom = paint_window->height - 1;
            } else {
              rect2.bottom = rect.bottom;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect2.left, rect.top, rect.right, rect2.bottom); */
            drawRectangle(paint_window, rect2.left, rect.top, rect.right, rect2.bottom,
                to_clear_col(paint_window));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
          if (rect.bottom >= 0 && (unsigned int) rect.bottom >= paint_window->height) {
            if (rect.top < 0 || (unsigned int) rect.top < paint_window->height) {
              rect2.top = paint_window->height;
            } else {
              rect2.top = rect.top;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect2.top, rect.right, rect.bottom); */
            drawRectangle(paint_window, rect.left, rect2.top, rect.right, rect.bottom,
                to_clear_col(paint_window));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
        } else {
          printf("paint_window=%lu, backup_hdc=%lu\n",
              (long unsigned int) paint_window,
              (long unsigned int) paint_window->backup_hdc);
        } /* if */
        result = 1;
        break;
      default:
        result = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    } /* switch */
    /* printf("WndProc ==> %d\n", result); */
    return result;
  } /* WndProc */



static void dra_init (void)

  {
    WNDCLASSEX wcex = {0};
    HMODULE hntdll = 0;

  /* dra_init */
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = /* CS_HREDRAW | CS_VREDRAW | */ CS_OWNDC;
    wcex.lpfnWndProc   = (WNDPROC)WndProc;
    wcex.hInstance     = NULL;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = windowClass;
    wcex.hIconSm       = NULL;
    RegisterClassEx(&wcex);
    hntdll = LoadLibraryA("kernel32.dll");
    if (hntdll != 0) {
      pGetConsoleWindow = (void *) GetProcAddress(hntdll, "GetConsoleWindow");
    } /* if */
    init_called = 1;
  } /* dra_init */



inttype drwPointerXpos (const_wintype actual_window)

  {
    POINT point;
    inttype result;

  /* drwPointerXpos */
    /* printf("begin drwPointerXpos\n"); */
    if (GetCursorPos(&point) == 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      if (ScreenToClient(to_hwnd(actual_window), &point) == 0) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = point.x;
      } /* if */
    } /* if */
    /* printf("end drwPointerXpos --> %ld\n", result); */
    return result;
  } /* drwPointerXpos */



inttype drwPointerYpos (const_wintype actual_window)

  {
    POINT point;
    inttype result;

  /* drwPointerYpos */
    /* printf("begin drwPointerYpos\n"); */
    if (GetCursorPos(&point) == 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      if (ScreenToClient(to_hwnd(actual_window), &point) == 0) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = point.y;
      } /* if */
    } /* if */
    /* printf("end drwPointerYpos --> %ld\n", result); */
    return result;
  } /* drwPointerYpos */



void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  {
    float startAng, sweepAng;

  /* drwArc */
    startAng = (startAngle * (360.0 / (2 * PI)));
    sweepAng = (sweepAngle * (360.0 / (2 * PI)));
    AngleArc(to_hdc(actual_window), castToInt(x), castToInt(y),
             (unsigned) radius, startAng, sweepAng);
  } /* drwArc */



void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  {
    float startAng, sweepAng;
    HPEN old_pen;
    HPEN current_pen;

  /* drwPArc */
    startAng = (startAngle * (360.0 / (2 * PI)));
    sweepAng = (sweepAngle * (360.0 / (2 * PI)));
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
      raise_error(MEMORY_ERROR);
    } else if (!inIntRange(x) || !inIntRange(x)) {
      raise_error(RANGE_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      /* MoveToEx(to_hdc(actual_window), x + radius, y, NULL); */
      AngleArc(to_hdc(actual_window), (int) x, (int) y, (unsigned) radius, startAng, sweepAng);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        /* MoveToEx(to_backup_hdc(actual_window), x + radius, y, NULL); */
        AngleArc(to_backup_hdc(actual_window), (int) x, (int) y, (unsigned) radius, startAng, sweepAng);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPArc */



void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  { /* drwFArcChord */
  } /* drwFArcChord */



void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  { /* drwPFArcChord */
  } /* drwPFArcChord */



void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  {
    float startAng, sweepAng;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFArcPieSlice */
    if (sweepAngle != 0.0) {
      startAng = (startAngle * (360.0 / (2 * PI)));
      sweepAng = (sweepAngle * (360.0 / (2 * PI)));
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      current_brush = CreateSolidBrush((COLORREF) col);
      if (current_pen == NULL || current_brush == NULL) {
        raise_error(MEMORY_ERROR);
      } else if (!inIntRange(x) || !inIntRange(x) || !inIntRange(radius) || radius < 0) {
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



void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)

  { /* drwArc2 */
  } /* drwArc2 */



void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwCircle */
    AngleArc(to_hdc(actual_window), castToInt(x), castToInt(y), (unsigned) radius, 0.0, 360.0);
  } /* drwCircle */



void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;

  /* drwPCircle */
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
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



void drwClear (wintype actual_window, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwClear */
#ifdef TRACE_WIN
    printf("drwClear(%lu, %lx)\n", actual_window, col);
#endif
    to_clear_col(actual_window) = col;
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      /* The main window is cleared with the real window size. */
      Rectangle(to_hdc(actual_window), 0, 0,
          drwWidth(actual_window), drwHeight(actual_window));
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Rectangle(to_backup_hdc(actual_window), 0, 0,
            (int) to_width(actual_window), (int) to_height(actual_window));
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwClear */



void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y)

  { /* drwCopyArea */
#ifdef TRACE_WIN
    printf("drwCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
        src_window, dest_window, src_x, src_y, width, height, dest_x, dest_y);
#endif
    if (!inIntRange(src_x) || !inIntRange(src_y) ||
        !inIntRange(width) || !inIntRange(height) ||
        !inIntRange(dest_x) || !inIntRange(dest_y) ||
        width < 1 || height < 1) {
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



void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwFCircle */
  } /* drwFCircle */



void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFCircle */
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL) {
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



void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwFEllipse */
  } /* drwFEllipse */



void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPFEllipse */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL) {
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



void drwFlush (void)

  { /* drwFlush */
  } /* drwFlush */



void drwFree (wintype old_window)

  { /* drwFree */
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
    FREE_RECORD(old_window, win_winrecord, count.win);
  } /* drwFree */



wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height)

  {
    win_wintype result;

  /* drwGet */
    if (!inIntRange(left) || !inIntRange(upper) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (!ALLOC_RECORD(result, win_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      result->hdc = CreateCompatibleDC(to_hdc(actual_window));
      result->hBitmap = CreateCompatibleBitmap(to_hdc(actual_window), (int) width, (int) height);
      if (result->hBitmap == NULL) {
        free(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        result->oldBitmap = (HBITMAP) SelectObject(result->hdc, result->hBitmap);
        result->hasTransparentPixel = FALSE;
        result->transparentPixel = 0;
        result->is_pixmap = TRUE;
        result->width = (unsigned int) width;
        result->height = (unsigned int) height;
        if (to_backup_hdc(actual_window) != 0) {
          BitBlt(result->hdc, 0, 0, (int) width, (int) height,
              to_backup_hdc(actual_window), (int) left, (int) upper, SRCCOPY);
        } else {
          BitBlt(result->hdc, 0, 0, (int) width, (int) height,
              to_hdc(actual_window), (int) left, (int) upper, SRCCOPY);
        } /* if */
      } /* if */
    } /* if */
    return (wintype) result;
  } /* drwGet */



inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y)

  { /* drwGetPixel */
    return (inttype) GetPixel(to_hdc(actual_window), castToInt(x), castToInt(y));
  } /* drwGetPixel */



inttype drwHeight (const_wintype actual_window)

  {
    RECT rect;
    inttype result;

  /* drwHeight */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      result = (inttype) to_height(actual_window);
    } else {
      result = (inttype) ((unsigned int) (rect.bottom - rect.top) -
                          to_brutto_height_delta(actual_window));
    } /* if */
    return result;
  } /* drwHeight */



wintype drwImage (int32type *image_data, memsizetype width, memsizetype height)

  {
    inttype xPos;
    inttype yPos;
    wintype result;

  /* drwImage */
    if (width < 1 || width > INTTYPE_MAX ||
        height < 1 || height > INTTYPE_MAX) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (init_called == 0) {
        dra_init();
      } /* if */
      result = drwNewPixmap((inttype) width, (inttype) height);
      if (result != NULL) {
        for (yPos = 0; yPos < height; yPos++) {
          for (xPos = 0; xPos < width; xPos++) {
            drwPPoint(result, xPos, yPos, image_data[yPos * width + xPos]);
          } /* for */
        } /* for */
      } /* if */
    } /* if */
    return result;
  } /* drwImage */



void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)

  { /* drwLine */
    MoveToEx(to_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
    LineTo(to_hdc(actual_window), castToInt(x2), castToInt(y2));
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), castToInt(x1), castToInt(y1), NULL);
      LineTo(to_backup_hdc(actual_window), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwLine */



void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;

  /* drwPLine */
#ifdef TRACE_WIN
    printf("drwPLine(%lu, %ld, %ld, %ld, %ld, %lx)\n", actual_window, x1, y1, x2, y2, col);
#endif
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
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



wintype drwNewPixmap (inttype width, inttype height)

  {
    HDC screenDC;
    win_wintype result;

  /* drwNewPixmap */
    if (!inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (init_called == 0) {
        dra_init();
      } /* if */
      if (!ALLOC_RECORD(result, win_winrecord, count.win)) {
        raise_error(MEMORY_ERROR);
      } else {
        memset(result, 0, sizeof(struct win_winstruct));
        result->usage_count = 1;
        screenDC = GetDC(NULL);
        result->hdc = CreateCompatibleDC(screenDC);
        result->hBitmap = CreateCompatibleBitmap(screenDC, (int) width, (int) height);
        ReleaseDC(NULL, screenDC);
        result->oldBitmap = (HBITMAP) SelectObject(result->hdc, result->hBitmap);
        result->hasTransparentPixel = FALSE;
        result->transparentPixel = 0;
        result->is_pixmap = TRUE;
        result->width = (unsigned int) width;
        result->height = (unsigned int) height;
      } /* if */
    } /* if */
    return (wintype) result;
  } /* drwNewPixmap */



wintype drwNewBitmap (const_wintype actual_window, inttype width, inttype height)

  { /* drwNewBitmap */
    return 0;
  } /* drwNewBitmap */



static booltype private_console (void)

  {
    CONSOLE_SCREEN_BUFFER_INFO conBufInfo;

  /* private_console */
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conBufInfo)) {
      return FALSE;
    } else {
      /* When the cursor position is (0,0) the program has a private console */
      return conBufInfo.dwCursorPosition.X == 0 && conBufInfo.dwCursorPosition.Y == 0;
    } /* if */
  } /* private_console */



wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name)

  {
    char *win_name;
    HFONT std_font;
    win_wintype result;

  /* drwOpen */
#ifdef TRACE_WIN
    printf("BEGIN drwOpen(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
    result = NULL;
    if (!inIntRange(xPos) || !inIntRange(yPos) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (init_called == 0) {
        dra_init();
      } /* if */
      if (init_called != 0) {
        win_name = cp_to_cstri8(window_name);
        if (win_name == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          if (private_console()) {
            /* printf("private_session\n"); */
            if (pGetConsoleWindow != NULL) {
              ShowWindow(pGetConsoleWindow(), SW_HIDE);
            } else {
              FreeConsole();
            } /* if */
          } /* if */
          if (ALLOC_RECORD(result, win_winrecord, count.win)) {
            memset(result, 0, sizeof(struct win_winstruct));
            result->usage_count = 1;
#ifdef OUT_OF_ORDER
            printf("SM_CXBORDER=%d\n",    GetSystemMetrics(SM_CXBORDER));
            printf("SM_CYBORDER=%d\n",    GetSystemMetrics(SM_CYBORDER));
            printf("SM_CXSIZE=%d\n",      GetSystemMetrics(SM_CXSIZE));
            printf("SM_CYSIZE=%d\n",      GetSystemMetrics(SM_CYSIZE));
            printf("SM_CXSIZEFRAME=%d\n", GetSystemMetrics(SM_CXSIZEFRAME));
            printf("SM_CYSIZEFRAME=%d\n", GetSystemMetrics(SM_CYSIZEFRAME));
            printf("SM_CXEDGE=%d\n",      GetSystemMetrics(SM_CXEDGE));
            printf("SM_CYEDGE=%d\n",      GetSystemMetrics(SM_CYEDGE));
            printf("width=%d\n",          width + 2 * GetSystemMetrics(SM_CXSIZEFRAME));
            printf("height=%d\n",         height + 2 * GetSystemMetrics(SM_CYSIZEFRAME) +
                GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER));
#endif
            result->brutto_width_delta = 2 * GetSystemMetrics(SM_CXSIZEFRAME);
            result->brutto_height_delta = 2 * GetSystemMetrics(SM_CYSIZEFRAME) +
                GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER);
            result->hWnd = CreateWindow(windowClass, win_name,
                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                width + result->brutto_width_delta,
                height + result->brutto_height_delta,
                (HWND) NULL, (HMENU) NULL, NULL, NULL);
            enter_window((wintype) result, result->hWnd);
            /* printf("hWnd=%lu\n", result->hWnd); */
            if (result->hWnd != NULL) {
              result->hdc = GetDC(result->hWnd);
              /* printf("hdc=%lu\n", result->hdc); */
              result->hasTransparentPixel = FALSE;
              result->transparentPixel = 0;
              result->is_pixmap = FALSE;
              result->width = (unsigned int) width;
              result->height = (unsigned int) height;
              result->clear_col = (inttype) RGB(0, 0, 0); /* black */
              result->backup_hdc = CreateCompatibleDC(result->hdc);
              result->backup = CreateCompatibleBitmap(result->hdc, (int) width, (int) height);
              SelectObject(result->backup_hdc, result->backup);
              std_font = CreateFont(16, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                  DEFAULT_QUALITY, FIXED_PITCH | FF_SWISS, NULL);
              SelectObject(result->hdc, std_font);
              SelectObject(result->backup_hdc, std_font);
              ShowWindow(result->hWnd, SW_SHOWDEFAULT);
              UpdateWindow(result->hWnd);
            } /* if */
          } /* if */
          free_cstri(win_name, window_name);
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_WIN
    printf("END drwOpen ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpen */



wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)

  {
    HFONT std_font;
    win_wintype result;

  /* drwOpenSubWindow */
#ifdef TRACE_WIN
    printf("BEGIN drwOpenSubWindow(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
    result = NULL;
    if (!inIntRange(xPos) || !inIntRange(yPos) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (init_called == 0) {
        dra_init();
      } /* if */
      if (init_called != 0) {
        if (ALLOC_RECORD(result, win_winrecord, count.win)) {
          memset(result, 0, sizeof(struct win_winstruct));
          result->usage_count = 1;
#ifdef OUT_OF_ORDER
          printf("SM_CXBORDER=%d\n",    GetSystemMetrics(SM_CXBORDER));
          printf("SM_CYBORDER=%d\n",    GetSystemMetrics(SM_CYBORDER));
          printf("SM_CXSIZE=%d\n",      GetSystemMetrics(SM_CXSIZE));
          printf("SM_CYSIZE=%d\n",      GetSystemMetrics(SM_CYSIZE));
          printf("SM_CXSIZEFRAME=%d\n", GetSystemMetrics(SM_CXSIZEFRAME));
          printf("SM_CYSIZEFRAME=%d\n", GetSystemMetrics(SM_CYSIZEFRAME));
          printf("SM_CXEDGE=%d\n",      GetSystemMetrics(SM_CXEDGE));
          printf("SM_CYEDGE=%d\n",      GetSystemMetrics(SM_CYEDGE));
          printf("width=%d\n",          width + 2 * GetSystemMetrics(SM_CXSIZEFRAME));
          printf("height=%d\n",         height + 2 * GetSystemMetrics(SM_CYSIZEFRAME) +
              GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER));
          printf("WS_OVERLAPPEDWINDOW = %lx\n", WS_OVERLAPPEDWINDOW);
          printf("WS_BORDER           = %lx\n", WS_BORDER);
          printf("WS_THICKFRAME       = %lx\n", WS_THICKFRAME);
          printf("WS_DLGFRAME         = %lx\n", WS_DLGFRAME);
          printf("WS_CAPTION          = %lx\n", WS_CAPTION);
          printf("WS_CHILD            = %lx\n", WS_CHILD);
#endif

          result->brutto_width_delta = 0;
          result->brutto_height_delta = 0;
          result->hWnd = CreateWindow(windowClass, "",
              WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
              (int) xPos, (int) yPos, (int) width, (int) height,
              to_hwnd(parent_window), (HMENU) NULL, NULL, NULL);
#ifdef OUT_OF_ORDER
          result->hWnd = CreateWindowEx(0, windowClass, "",
              (WS_VISIBLE | WS_SYSMENU /* | WS_THICKFRAME */),
              (int) xPos, (int) yPos, (int) width, (int) height,
              to_hwnd(parent_window), NULL, NULL /* hInstance */, NULL);
#endif
          enter_window((wintype) result, result->hWnd);
          /* printf("hWnd=%lu\n", result->hWnd); */
          if (result->hWnd != NULL) {
            SetWindowLong(result->hWnd , GWL_STYLE, GetWindowLong(result->hWnd , GWL_STYLE) &~ WS_CAPTION);
            SetWindowPos(result->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            result->hdc = GetDC(result->hWnd);
            /* printf("hdc=%lu\n", result->hdc); */
            result->hasTransparentPixel = FALSE;
            result->transparentPixel = 0;
            result->is_pixmap = FALSE;
            result->width = (unsigned int) width;
            result->height = (unsigned int) height;
            result->clear_col = (inttype) RGB(0, 0, 0); /* black */
            result->backup_hdc = CreateCompatibleDC(result->hdc);
            result->backup = CreateCompatibleBitmap(result->hdc, (int) width, (int) height);
            SelectObject(result->backup_hdc, result->backup);
            std_font = CreateFont(16, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_SWISS, NULL);
            SelectObject(result->hdc, std_font);
            SelectObject(result->backup_hdc, std_font);
            ShowWindow(result->hWnd, SW_SHOW /*SW_SHOWDEFAULT*/);
            UpdateWindow(result->hWnd);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_WIN
    printf("END drwOpenSubWindow ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpenSubWindow */



void drwPoint (const_wintype actual_window, inttype x, inttype y)

  { /* drwPoint */
    MoveToEx(to_hdc(actual_window), castToInt(x), castToInt(y), NULL);
    LineTo(to_hdc(actual_window), castToInt(x + 1), castToInt(y + 1));
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), castToInt(x), castToInt(y), NULL);
      LineTo(to_backup_hdc(actual_window), castToInt(x + 1), castToInt(y + 1));
    } /* if */
  } /* drwPoint */



void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col)

  { /* drwPPoint */
#ifdef TRACE_WIN
    printf("drwPPoint(%lu, %ld, %ld, %lx)\n", actual_window, x, y, col);
#endif
    SetPixel(to_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
    if (to_backup_hdc(actual_window) != 0) {
      SetPixel(to_backup_hdc(actual_window), castToInt(x), castToInt(y), (COLORREF) col);
    } /* if */
  } /* drwPPoint */



void drwConvPointList (bstritype pointList, inttype *xy)

  {
    memsizetype len;
    POINT *points;
    memsizetype pos;

  /* drwConvPointList */
    len = pointList->size / sizeof(POINT);
    if (len > 0) {
      points = (POINT *) pointList->mem;
      for (pos = 0; pos < len; pos ++) {
        xy[pos << 1]       = points[pos].x;
        xy[(pos << 1) + 1] = points[pos].y;
      } /* for */
    } /* if */
  } /* drwConvPointList */



bstritype drwGenPointList (const const_rtlArraytype xyArray)

  {
    memsizetype num_elements;
    memsizetype len;
    POINT *points;
    memsizetype pos;
    bstritype result;

  /* drwGenPointList */
    /* printf("drwGenPointList(%ld .. %ld)\n", xyArray->min_position, xyArray->max_position); */
    num_elements = arraySize(xyArray);
    if (num_elements & 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      len = num_elements >> 1;
      if (len > MAX_BSTRI_LEN / sizeof(POINT) || len > MAX_MEM_INDEX) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(POINT))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len * sizeof(POINT);
          if (len > 0) {
            points = (POINT *) result->mem;
            for (pos = 0; pos < len; pos ++) {
              points[pos].x = castToLong(xyArray->arr[ pos << 1     ].value.intvalue);
              points[pos].y = castToLong(xyArray->arr[(pos << 1) + 1].value.intvalue);
            } /* for */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGenPointList */



inttype drwLngPointList (bstritype point_list)

  { /* drwLngPointList */
    return (inttype) (point_list->size / sizeof(POINT));
  } /* drwLngPointList */



void drwPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  {
    POINT *points;
    memsizetype npoints;
    memsizetype pos;
    HPEN old_pen;
    HPEN current_pen;

  /* drwPolyLine */
    points = (POINT *) point_list->mem;
    npoints = point_list->size / sizeof(POINT);
    if (npoints >= 2) {
      current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
      if (current_pen == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        MoveToEx(to_hdc(actual_window), x + points[0].x, y + points[0].y, NULL);
        for (pos = 1; pos < npoints; pos ++) {
          LineTo(to_hdc(actual_window), x + points[pos].x, y + points[pos].y);
        } /* for */
        SetPixel(to_hdc(actual_window), x + points[npoints - 1].x, y + points[npoints - 1].y, (COLORREF) col);
        SelectObject(to_hdc(actual_window), old_pen);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          MoveToEx(to_backup_hdc(actual_window), x + points[0].x, y + points[0].y, NULL);
          for (pos = 1; pos < npoints; pos ++) {
            LineTo(to_backup_hdc(actual_window), x + points[pos].x, y + points[pos].y);
          } /* for */
          SetPixel(to_backup_hdc(actual_window), x + points[npoints - 1].x, y + points[npoints - 1].y, (COLORREF) col);
          SelectObject(to_backup_hdc(actual_window), old_pen);
        } /* if */
        DeleteObject(current_pen);
      } /* if */
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  {
    POINT *points;
    memsizetype npoints;
    memsizetype pos;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwFPolyLine */
    points = (POINT *) point_list->mem;
    npoints = point_list->size / sizeof(POINT);
    for (pos = 0; pos < npoints; pos ++) {
      points[pos].x += castToInt(x);
      points[pos].y += castToInt(y);
    } /* for */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL || npoints > INT_MAX) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      Polygon(to_hdc(actual_window), points, (int) npoints);
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Polygon(to_backup_hdc(actual_window), points, (int) npoints);
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
    for (pos = 0; pos < npoints; pos ++) {
      points[pos].x -= castToInt(x);
      points[pos].y -= castToInt(y);
    } /* for */
  } /* drwFPolyLine */



void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x, inttype y)

  { /* drwPut */
    if (pixmap != NULL) {
#ifdef USE_TRANSPARENTBLT
      if (to_hasTransparentPixel(pixmap)) {
        TransparentBlt(to_hdc(actual_window), x, y, to_width(pixmap), to_height(pixmap),
            to_hdc(pixmap), 0, 0, to_width(pixmap), to_height(pixmap), to_transparentPixel(pixmap));
        if (to_backup_hdc(actual_window) != 0) {
          TransparentBlt(to_backup_hdc(actual_window), x, y, to_width(pixmap), to_height(pixmap),
              to_hdc(pixmap), 0, 0, to_width(pixmap), to_height(pixmap), to_transparentPixel(pixmap));
        } /* if */
      } else {
#endif
        BitBlt(to_hdc(actual_window), castToInt(x), castToInt(y),
            (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCCOPY);
        if (to_backup_hdc(actual_window) != 0) {
          BitBlt(to_backup_hdc(actual_window), castToInt(x), castToInt(y),
              (int) to_width(pixmap), (int) to_height(pixmap), to_hdc(pixmap), 0, 0, SRCCOPY);
        } /* if */
#ifdef USE_TRANSPARENTBLT
      } /* if */
#endif
    } /* if */
  } /* drwPut */



void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwRect */
    Rectangle(to_hdc(actual_window), castToInt(x), castToInt(y),
              castToInt(x + width), castToInt(y + height));
    if (to_backup_hdc(actual_window) != 0) {
      Rectangle(to_backup_hdc(actual_window), castToInt(x), castToInt(y),
                castToInt(x + width), castToInt(y + height));
    } /* if */
  } /* drwRect */



void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPRect */
#ifdef TRACE_WIN
    printf("drwPRect(%lu, %ld, %ld, %ld, %ld, %lx)\n", actual_window, x, y, width, height, col);
#endif
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
    if (current_pen == NULL || current_brush == NULL) {
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



inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)

  { /* drwRgbColor */
#ifdef TRACE_WIN
    printf("drwRgbColor(%lu, %ld, %ld)\n", red_val, green_val, blue_val);
#endif
    return (inttype) RGB(((uinttype) red_val) >> 8,
                         ((uinttype) green_val) >> 8,
                         ((uinttype) blue_val) >> 8);
  } /* drwRgbColor */



void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)

  { /* drwPixelToRgb */
    *red_val   = GetRValue(col) << 8;
    *green_val = GetGValue(col) << 8;
    *blue_val  = GetBValue(col) << 8;
  } /* drwPixelToRgb */



void drwBackground (inttype col)

  { /* drwBackground */
  } /* drwBackground */



void drwColor (inttype col)

  { /* drwColor */
      /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
  } /* drwColor */



void drwSetContent (const_wintype actual_window, const_wintype pixmap)

  { /* drwSetContent */
    /* printf("begin drwSetContent(%lu, %lu)\n",
        to_hwnd(actual_window), to_hwnd(pixmap)); */
    if (pixmap != NULL) {
      BitBlt(to_hdc(actual_window), 0, 0, to_width(pixmap), to_height(pixmap),
          to_hdc(pixmap), 0, 0, SRCCOPY);
      if (to_backup_hdc(actual_window) != 0) {
        BitBlt(to_backup_hdc(actual_window), 0, 0, to_width(pixmap), to_height(pixmap),
            to_hdc(pixmap), 0, 0, SRCCOPY);
      } /* if */
    } /* if */
    /* printf("end drwSetContent(%lu, %lu)\n",
        to_hwnd(actual_window), to_hwnd(pixmap)); */
  } /* drwSetContent */



void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_hwnd(actual_window), xPos, yPos); */
    SetWindowPos(to_hwnd(actual_window), 0, castToInt(xPos), castToInt(yPos), 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOZORDER | SWP_NOSIZE);
    gkbKeyPressed();
    /* printf("end drwSetPos(%lu, %ld, %ld)\n",
        to_hwnd(actual_window), xPos, yPos); */
  } /* drwSetPos */



void drwSetTransparentColor (wintype pixmap, inttype col)

  { /* drwSetTransparentColor */
    to_hasTransparentPixel(pixmap) = TRUE;
    to_transparentPixel(pixmap) = col;
  } /* drwSetTransparentColor */



void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol)

  {
    wchar_t *stri_buffer;
    wchar_t *wstri;
    strelemtype *strelem;
    memsizetype len;

  /* drwText */
    stri_buffer = (wchar_t *) malloc(sizeof(wchar_t) * stri->size);
    if (stri_buffer != NULL) {
      wstri = stri_buffer;
      strelem = stri->mem;
      len = stri->size;
      for (; len > 0; wstri++, strelem++, len--) {
        if (*strelem >= 65536) {
          raise_error(RANGE_ERROR);
          return;
        } /* if */
        *wstri = (wchar_t) *strelem;
      } /* for */

      SetTextColor(to_hdc(actual_window), (COLORREF) col);
      SetBkColor(to_hdc(actual_window), (COLORREF) bkcol);
      SetTextAlign(to_hdc(actual_window), TA_BASELINE | TA_LEFT);
      TextOutW(to_hdc(actual_window), castToInt(x), castToInt(y), stri_buffer, stri->size);
      if (to_backup_hdc(actual_window) != 0) {
        SetTextColor(to_backup_hdc(actual_window), (COLORREF) col);
        SetBkColor(to_backup_hdc(actual_window), (COLORREF) bkcol);
        SetTextAlign(to_backup_hdc(actual_window), TA_BASELINE | TA_LEFT);
        TextOutW(to_backup_hdc(actual_window), castToInt(x), castToInt(y), stri_buffer, stri->size);
      } /* if */
      free(stri_buffer);
    } /* if */
  } /* drwText */



void drwToBottom (const_wintype actual_window)

  { /* drwToBottom */
    /* printf("begin drwToBottom(%lu)\n", to_hwnd(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_BOTTOM, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToBottom(%lu)\n", to_hwnd(actual_window)); */
  } /* drwToBottom */



void drwToTop (const_wintype actual_window)

  { /* drwToTop */
    /* printf("begin drwToTop(%lu)\n", to_hwnd(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_TOP, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToTop(%lu)\n", to_hwnd(actual_window)); */
  } /* drwToTop */



inttype drwWidth (const_wintype actual_window)

  {
    RECT rect;
    inttype result;

  /* drwWidth */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      result = (inttype) to_width(actual_window);
    } else {
      result = (inttype) ((unsigned int) (rect.right - rect.left) -
                          to_brutto_width_delta(actual_window));
    } /* if */
    return result;
  } /* drwWidth */



inttype drwXPos (const_wintype actual_window)

  {
    RECT rect;
    POINT point;
    inttype result;

  /* drwXPos */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      point.x = rect.left;
      point.y = rect.top;
      ScreenToClient(GetParent(to_hwnd(actual_window)), &point);
      result = point.x;
    } /* if */
    return result;
  } /* drwXPos */



inttype drwYPos (const_wintype actual_window)

  {
    RECT rect;
    POINT point;
    inttype result;

  /* drwYPos */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      point.x = rect.left;
      point.y = rect.top;
      ScreenToClient(GetParent(to_hwnd(actual_window)), &point);
      result = point.y;
    } /* if */
    return result;
  } /* drwYPos */
