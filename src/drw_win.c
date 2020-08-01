/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilities.    */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  Changes: 2005 - 2007  Thomas Mertes                             */
/*  Content: Graphic access using the windows capabilities.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
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

#define TRACE_ENTITYS


#define PI 3.141592653589793238462643383279502884197

#define windowClass "MyWindowClass"

static inttype init_called = 0;

static inttype button_x = 0;
static inttype button_y = 0;
static HWND button_window = 0;


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
    struct win_winstruct *next;
  } win_winrecord, *win_wintype;

static win_wintype window_list = NULL;

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

chartype map_1252_to_unicode[] = {
/* 128 */ 0x20AC,    '?', 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
/* 136 */ 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152,    '?', 0x017D,    '?',
/* 144 */    '?', 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
/* 152 */ 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153,    '?', 0x017E, 0x0178};



#ifdef ANSI_C

static win_wintype find_window (HWND curr_window)
#else

static win_wintype find_window (curr_window)
HWND curr_window;
#endif

  {
    win_wintype window;

  /* find_window */
    window = window_list;
    while (window != NULL) {
      if (to_hwnd(window) == curr_window) {
        return window;
      } /* if */
      window = window->next;
    } /* while */
    return NULL;
  } /* find_window */



#ifdef ANSI_C

static void remove_window (win_wintype curr_window)
#else

static void remove_window (curr_window)
win_wintype curr_window;
#endif

  {
    win_wintype *win_addr;
    win_wintype window;

  /* remove_window */
    win_addr = &window_list;
    window = window_list;
    while (window != NULL) {
      if (window == curr_window) {
        *win_addr = window->next;
      } /* if */
      win_addr = &window->next;
      window = window->next;
    } /* while */
  } /* remove_window */



#ifdef ANSI_C

static void drawRectangle (win_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)
#else

static void drawRectangle (actual_window, x1, y1, x2, y2, col)
win_wintype actual_window;
inttype x1, y1, x2, y2;
inttype col;
#endif

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
          SetPixel(actual_window->hdc, x1, y1, (COLORREF) col);
        } else {
          MoveToEx(actual_window->hdc, x1, y1, NULL);
          LineTo(actual_window->hdc, x1, y2 + 1);
        } /* if */
      } else {
        if (y1 == y2) {
          MoveToEx(actual_window->hdc, x1, y1, NULL);
          LineTo(actual_window->hdc, x2 + 1, y1);
        } else {
          Rectangle(actual_window->hdc, x1, y1, x2 + 1, y2 + 1);
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
        paint_window = find_window(hWnd);
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
        paint_window = find_window(hWnd);
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



#ifdef ANSI_C

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  {
    BOOL bRet;
    MSG msg;
    chartype result;

  /* gkbGetc */
    /* printf("begin getc()\n"); */
    result = K_NONE;
    /* printf("before GetMessage\n"); */
    bRet = GetMessage(&msg, NULL, 0, 0);
    /* printf("after GetMessage\n"); */
    while (result == K_NONE && bRet != 0) {
      if (bRet == -1) {
        printf("GetMessage(&msg, NULL, 0, 0)=-1\n");
      } else {
        /* printf("gkbGetc message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
        if (msg.message == WM_KEYDOWN) {
          /* printf("WM_KEYDOWN %lu, %d, %d\n", msg.hwnd, msg.wParam, msg.lParam); */
          /* printf("GetKeyState(VK_SHIFT)=%hx\n",   GetKeyState(VK_SHIFT));
          printf("GetKeyState(VK_CONTROL)=%hx\n", GetKeyState(VK_CONTROL));
          printf("GetKeyState(VK_MENU)=%hx\n",   GetKeyState(VK_MENU)); */
          if (GetKeyState(VK_SHIFT) & 0xFF80) {
            switch (msg.wParam) {
              case VK_LBUTTON:  result = K_MOUSE1;     break;
              case VK_MBUTTON:  result = K_MOUSE2;     break;
              case VK_RBUTTON:  result = K_MOUSE3;     break;
              case VK_RETURN:   result = K_NL;         break;
              case VK_F1:       result = K_SFT_F1;     break;
              case VK_F2:       result = K_SFT_F2;     break;
              case VK_F3:       result = K_SFT_F3;     break;
              case VK_F4:       result = K_SFT_F4;     break;
              case VK_F5:       result = K_SFT_F5;     break;
              case VK_F6:       result = K_SFT_F6;     break;
              case VK_F7:       result = K_SFT_F7;     break;
              case VK_F8:       result = K_SFT_F8;     break;
              case VK_F9:       result = K_SFT_F9;     break;
              case VK_F10:      result = K_SFT_F10;    break;
              case VK_LEFT:     result = K_LEFT;       break;
              case VK_RIGHT:    result = K_RIGHT;      break;
              case VK_UP:       result = K_UP;         break;
              case VK_DOWN:     result = K_DOWN;       break;
              case VK_HOME:     result = K_HOME;       break;
              case VK_END:      result = K_END;        break;
              case VK_PRIOR:    result = K_PGUP;       break;
              case VK_NEXT:     result = K_PGDN;       break;
              case VK_INSERT:   result = K_INS;        break;
              case VK_DELETE:   result = K_DEL;        break;
              case VK_CLEAR:    result = K_PAD_CENTER; break;
              case VK_APPS:     result = K_UNDEF;      break;
              case VK_TAB:      result = K_BACKTAB;    break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;       break;
              default:          result = K_UNDEF;      break;
            } /* switch */
          } else if (GetKeyState(VK_MENU) & 0xFF80) {
            switch (msg.wParam) {
              case VK_LBUTTON:  result = K_MOUSE1;     break;
              case VK_MBUTTON:  result = K_MOUSE2;     break;
              case VK_RBUTTON:  result = K_MOUSE3;     break;
              case VK_RETURN:   result = K_NL;         break;
              case VK_F1:       result = K_ALT_F1;     break;
              case VK_F2:       result = K_ALT_F2;     break;
              case VK_F3:       result = K_ALT_F3;     break;
              case VK_F4:       result = K_ALT_F4;     break;
              case VK_F5:       result = K_ALT_F5;     break;
              case VK_F6:       result = K_ALT_F6;     break;
              case VK_F7:       result = K_ALT_F7;     break;
              case VK_F8:       result = K_ALT_F8;     break;
              case VK_F9:       result = K_ALT_F9;     break;
              case VK_F10:      result = K_ALT_F10;    break;
              case VK_LEFT:     result = K_LEFT;       break;
              case VK_RIGHT:    result = K_RIGHT;      break;
              case VK_UP:       result = K_UP;         break;
              case VK_DOWN:     result = K_DOWN;       break;
              case VK_HOME:     result = K_HOME;       break;
              case VK_END:      result = K_END;        break;
              case VK_PRIOR:    result = K_PGUP;       break;
              case VK_NEXT:     result = K_PGDN;       break;
              case VK_INSERT:   result = K_INS;        break;
              case VK_DELETE:   result = K_DEL;        break;
              case VK_CLEAR:    result = K_PAD_CENTER; break;
              case VK_APPS:     result = K_UNDEF;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;       break;
              default:          result = K_UNDEF;      break;
            } /* switch */
          } else if (GetKeyState(VK_CONTROL) & 0xFF80) {
            switch (msg.wParam) {
              case VK_LBUTTON:  result = K_MOUSE1;     break;
              case VK_MBUTTON:  result = K_MOUSE2;     break;
              case VK_RBUTTON:  result = K_MOUSE3;     break;
              case VK_RETURN:   result = K_NL;         break;
              case VK_F1:       result = K_CTL_F1;     break;
              case VK_F2:       result = K_CTL_F2;     break;
              case VK_F3:       result = K_CTL_F3;     break;
              case VK_F4:       result = K_CTL_F4;     break;
              case VK_F5:       result = K_CTL_F5;     break;
              case VK_F6:       result = K_CTL_F6;     break;
              case VK_F7:       result = K_CTL_F7;     break;
              case VK_F8:       result = K_CTL_F8;     break;
              case VK_F9:       result = K_CTL_F9;     break;
              case VK_F10:      result = K_CTL_F10;    break;
              case VK_LEFT:     result = K_CTL_LEFT;   break;
              case VK_RIGHT:    result = K_CTL_RIGHT;  break;
              case VK_UP:       result = K_CTL_UP;     break;
              case VK_DOWN:     result = K_CTL_DOWN;   break;
              case VK_HOME:     result = K_CTL_HOME;   break;
              case VK_END:      result = K_CTL_END;    break;
              case VK_PRIOR:    result = K_CTL_PGUP;   break;
              case VK_NEXT:     result = K_CTL_PGDN;   break;
              case VK_INSERT:   result = K_CTL_INS;    break;
              case VK_DELETE:   result = K_CTL_DEL;    break;
              case VK_CLEAR:    result = K_PAD_CENTER; break;
              case VK_APPS:     result = K_UNDEF;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;       break;
              default:          result = K_UNDEF;      break;
            } /* switch */
          } else {
            switch (msg.wParam) {
              case VK_LBUTTON:  result = K_MOUSE1;     break;
              case VK_MBUTTON:  result = K_MOUSE2;     break;
              case VK_RBUTTON:  result = K_MOUSE3;     break;
              case VK_RETURN:   result = K_NL;         break;
              case VK_F1:       result = K_F1;         break;
              case VK_F2:       result = K_F2;         break;
              case VK_F3:       result = K_F3;         break;
              case VK_F4:       result = K_F4;         break;
              case VK_F5:       result = K_F5;         break;
              case VK_F6:       result = K_F6;         break;
              case VK_F7:       result = K_F7;         break;
              case VK_F8:       result = K_F8;         break;
              case VK_F9:       result = K_F9;         break;
              case VK_F10:      result = K_F10;        break;
              case VK_LEFT:     result = K_LEFT;       break;
              case VK_RIGHT:    result = K_RIGHT;      break;
              case VK_UP:       result = K_UP;         break;
              case VK_DOWN:     result = K_DOWN;       break;
              case VK_HOME:     result = K_HOME;       break;
              case VK_END:      result = K_END;        break;
              case VK_PRIOR:    result = K_PGUP;       break;
              case VK_NEXT:     result = K_PGDN;       break;
              case VK_INSERT:   result = K_INS;        break;
              case VK_DELETE:   result = K_DEL;        break;
              case VK_CLEAR:    result = K_PAD_CENTER; break;
              case VK_APPS:     result = K_UNDEF;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;       break;
              default:          result = K_UNDEF;      break;
            } /* switch */
          } /* if */
          if (result == K_UNDEF) {
            /* printf("TranslateMessage(%d) %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("translated message=%d %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
              /* printf("PeekMessage(%d) %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              if (msg.message == WM_CHAR) {
                result = K_NONE;
              } /* if */
            } else {
              /* printf("PeekMessage ==> empty\n"); */
            } /* if */
          } /* if */
        } else if (msg.message == WM_LBUTTONDOWN) {
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          result = K_MOUSE1;
        } else if (msg.message == WM_MBUTTONDOWN) {
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          result = K_MOUSE2;
        } else if (msg.message == WM_RBUTTONDOWN) {
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          result = K_MOUSE3;
        } else if (msg.message == WM_SYSKEYDOWN) {
          /* printf("WM_SYSKEYDOWN %lu, %d, %u %d\n", msg.hwnd, msg.wParam, msg.lParam, msg.lParam & 0x20000000); */
          /* printf("GetKeyState(VK_SHIFT)=%hx\n",   GetKeyState(VK_SHIFT));
          printf("GetKeyState(VK_CONTROL)=%hx\n", GetKeyState(VK_CONTROL));
          printf("GetKeyState(VK_MENU)=%hx\n",   GetKeyState(VK_MENU)); */
          switch (msg.wParam) {
              case 'A':         result = K_ALT_A;   break;
              case 'B':         result = K_ALT_B;   break;
              case 'C':         result = K_ALT_C;   break;
              case 'D':         result = K_ALT_D;   break;
              case 'E':         result = K_ALT_E;   break;
              case 'F':         result = K_ALT_F;   break;
              case 'G':         result = K_ALT_G;   break;
              case 'H':         result = K_ALT_H;   break;
              case 'I':         result = K_ALT_I;   break;
              case 'J':         result = K_ALT_J;   break;
              case 'K':         result = K_ALT_K;   break;
              case 'L':         result = K_ALT_L;   break;
              case 'M':         result = K_ALT_M;   break;
              case 'N':         result = K_ALT_N;   break;
              case 'O':         result = K_ALT_O;   break;
              case 'P':         result = K_ALT_P;   break;
              case 'Q':         result = K_ALT_Q;   break;
              case 'R':         result = K_ALT_R;   break;
              case 'S':         result = K_ALT_S;   break;
              case 'T':         result = K_ALT_T;   break;
              case 'U':         result = K_ALT_U;   break;
              case 'V':         result = K_ALT_V;   break;
              case 'W':         result = K_ALT_W;   break;
              case 'X':         result = K_ALT_X;   break;
              case 'Y':         result = K_ALT_Y;   break;
              case 'Z':         result = K_ALT_Z;   break;
              case '0':         result = K_ALT_0;   break;
              case '1':         result = K_ALT_1;   break;
              case '2':         result = K_ALT_2;   break;
              case '3':         result = K_ALT_3;   break;
              case '4':         result = K_ALT_4;   break;
              case '5':         result = K_ALT_5;   break;
              case '6':         result = K_ALT_6;   break;
              case '7':         result = K_ALT_7;   break;
              case '8':         result = K_ALT_8;   break;
              case '9':         result = K_ALT_9;   break;
              case VK_NUMPAD0:  result = K_ALT_0;   break;
              case VK_NUMPAD1:  result = K_ALT_1;   break;
              case VK_NUMPAD2:  result = K_ALT_2;   break;
              case VK_NUMPAD3:  result = K_ALT_3;   break;
              case VK_NUMPAD4:  result = K_ALT_4;   break;
              case VK_NUMPAD5:  result = K_ALT_5;   break;
              case VK_NUMPAD6:  result = K_ALT_6;   break;
              case VK_NUMPAD7:  result = K_ALT_7;   break;
              case VK_NUMPAD8:  result = K_ALT_8;   break;
              case VK_NUMPAD9:  result = K_ALT_9;   break;
              case VK_F1:       result = K_ALT_F1;  break;
              case VK_F2:       result = K_ALT_F2;  break;
              case VK_F3:       result = K_ALT_F3;  break;
              case VK_F4:       result = K_ALT_F4;  break;
              case VK_F5:       result = K_ALT_F5;  break;
              case VK_F6:       result = K_ALT_F6;  break;
              case VK_F7:       result = K_ALT_F7;  break;
              case VK_F8:       result = K_ALT_F8;  break;
              case VK_F9:       result = K_ALT_F9;  break;
              case VK_F10:
                if (msg.lParam & 0x20000000) {
                  result = K_ALT_F10;
                } else if (GetKeyState(VK_SHIFT) & 0xFF80) {
                  result = K_SFT_F10;
                } else if (GetKeyState(VK_CONTROL) & 0xFF80) {
                  result = K_CTL_F10;
                } else {
                  result = K_F10;
                } /* if */
                break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;    break;
              default:
                /* printf("WM_SYSKEYDOWN %lu, %d %X\n", msg.hwnd, msg.wParam, msg.lParam); */
                result = K_UNDEF;
                break;
          } /* switch */
        } else if (msg.message == WM_NCLBUTTONDOWN) {
          /* printf("WM_NCLBUTTONDOWN %lu, %d, %u\n", msg.hwnd, msg.wParam, msg.lParam); */
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          if (msg.wParam == HTCLOSE && !IsWindow(msg.hwnd)) {
            /* printf("HTCLOSE\n"); */
            exit(1);
          } /* if */
        } else if (msg.message == WM_CHAR) {
          /* printf("WM_CHAR %lu, %d, %u\n", msg.hwnd, msg.wParam, msg.lParam); */
          result = msg.wParam;
          if (result >= 128 && result <= 159) {
            result = map_1252_to_unicode[result - 128];
          } /* if */
        } else {
          /* printf("message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
          TranslateMessage(&msg);
          /* printf("translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
          DispatchMessage(&msg);
        } /* if */
      } /* if */
      if (result == K_NONE) {
        /* printf("before GetMessage\n"); */
        bRet = GetMessage(&msg, NULL, 0, 0);
        /* printf("after GetMessage\n"); */
      } /* if */
    } /* while */
    /* printf("end getc() ==> %d\n", result); */
    return result;
  } /* gkbGetc */



#ifdef ANSI_C

booltype gkbKeyPressed (void)
#else

booltype gkbKeyPressed ()
#endif

  {
    BOOL msg_present;
    BOOL bRet;
    MSG msg;
    booltype result;

  /* gkbKeyPressed */
    result = FALSE;
    msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    while (msg_present) {
      /* printf("gkbKeyPressed message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
      switch (msg.message) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          if (msg.wParam == VK_SHIFT   || msg.wParam == VK_CONTROL ||
              msg.wParam == VK_MENU    || msg.wParam == VK_CAPITAL ||
              msg.wParam == VK_NUMLOCK || msg.wParam == VK_SCROLL) {
            bRet = GetMessage(&msg, NULL, 0, 0);
            if (bRet == 0) {
              printf("GetMessage(&msg, NULL, 0, 0)=0\n");
            } else if (bRet == -1) {
              printf("GetMessage(&msg, NULL, 0, 0)=-1\n");
            } else {
              /* printf("message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              TranslateMessage(&msg);
              /* printf("translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              DispatchMessage(&msg);
            } /* if */
            msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          } else {
            msg_present = 0;
            result = TRUE;
          } /* if */
          break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
          /* printf("gkbKeyPressed ==> TRUE for message %d\n", msg.message); */
          msg_present = 0;
          result = TRUE;
          break;
        case WM_NCLBUTTONDOWN:
          /* printf("gkbKeyPressed WM_NCLBUTTONDOWN\n"); */
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            printf("x GetMessage(&msg, NULL, 0, 0)=0\n");
          } else if (bRet == -1) {
            printf("x GetMessage(&msg, NULL, 0, 0)=-1\n");
          } else {
            /* printf("x message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("x translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            DispatchMessage(&msg);
            /* printf("x after DispatchMessage\n"); */
            if (msg.wParam == HTCLOSE && !IsWindow(msg.hwnd)) {
              /* printf("HTCLOSE\n"); */
              exit(1);
            } /* if */
          } /* if */
          msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          break;
        case WM_SYSKEYUP:
          /* printf("gkbKeyPressed WM_SYSKEYUP\n"); */
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            printf("y GetMessage(&msg, NULL, 0, 0)=0\n");
          } else if (bRet == -1) {
            printf("y GetMessage(&msg, NULL, 0, 0)=-1\n");
          } /* if */
          msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          break;
        default:
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            printf("GetMessage(&msg, NULL, 0, 0)=0\n");
          } else if (bRet == -1) {
            printf("GetMessage(&msg, NULL, 0, 0)=-1\n");
          } else {
            /* printf("message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            DispatchMessage(&msg);
            /* printf("after DispatchMessage\n"); */
          } /* if */
          msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          break;
      } /* switch */
    } /* while */
    /* printf("gkbKeyPressed() ==> %d\n", result); */
    return result;
  } /* gkbKeyPressed */



#ifdef ANSI_C

booltype gkbButtonPressed (chartype button)
#else

booltype gkbButtonPressed (button)
chartype button;
#endif

  {
    int vkey1;
    int vkey2 = 0;
    booltype okay = TRUE;
    booltype result;

  /* gkbButtonPressed */
    gkbKeyPressed();
    switch (button) {
      case K_CTL_A: case K_ALT_A: case 'A': case 'a': vkey1 = 'A'; break;
      case K_CTL_B: case K_ALT_B: case 'B': case 'b': vkey1 = 'B'; break;
      case K_CTL_C: case K_ALT_C: case 'C': case 'c': vkey1 = 'C'; break;
      case K_CTL_D: case K_ALT_D: case 'D': case 'd': vkey1 = 'D'; break;
      case K_CTL_E: case K_ALT_E: case 'E': case 'e': vkey1 = 'E'; break;
      case K_CTL_F: case K_ALT_F: case 'F': case 'f': vkey1 = 'F'; break;
      case K_CTL_G: case K_ALT_G: case 'G': case 'g': vkey1 = 'G'; break;
      /* K_CTL_H */ case K_ALT_H: case 'H': case 'h': vkey1 = 'H'; break;
      /* K_CTL_I */ case K_ALT_I: case 'I': case 'i': vkey1 = 'I'; break;
      /* K_CTL_J */ case K_ALT_J: case 'J': case 'j': vkey1 = 'J'; break;
      case K_CTL_K: case K_ALT_K: case 'K': case 'k': vkey1 = 'K'; break;
      case K_CTL_L: case K_ALT_L: case 'L': case 'l': vkey1 = 'L'; break;
      case K_CTL_M: case K_ALT_M: case 'M': case 'm': vkey1 = 'M'; break;
      case K_CTL_N: case K_ALT_N: case 'N': case 'n': vkey1 = 'N'; break;
      case K_CTL_O: case K_ALT_O: case 'O': case 'o': vkey1 = 'O'; break;
      case K_CTL_P: case K_ALT_P: case 'P': case 'p': vkey1 = 'P'; break;
      case K_CTL_Q: case K_ALT_Q: case 'Q': case 'q': vkey1 = 'Q'; break;
      case K_CTL_R: case K_ALT_R: case 'R': case 'r': vkey1 = 'R'; break;
      case K_CTL_S: case K_ALT_S: case 'S': case 's': vkey1 = 'S'; break;
      case K_CTL_T: case K_ALT_T: case 'T': case 't': vkey1 = 'T'; break;
      case K_CTL_U: case K_ALT_U: case 'U': case 'u': vkey1 = 'U'; break;
      case K_CTL_V: case K_ALT_V: case 'V': case 'v': vkey1 = 'V'; break;
      case K_CTL_W: case K_ALT_W: case 'W': case 'w': vkey1 = 'W'; break;
      case K_CTL_X: case K_ALT_X: case 'X': case 'x': vkey1 = 'X'; break;
      case K_CTL_Y: case K_ALT_Y: case 'Y': case 'y': vkey1 = 'Y'; break;
      case K_CTL_Z: case K_ALT_Z: case 'Z': case 'z': vkey1 = 'Z'; break;

      case K_ALT_0: case '0': vkey1 = '0'; vkey2 = VK_NUMPAD0; break;
      case K_ALT_1: case '1': vkey1 = '1'; vkey2 = VK_NUMPAD1; break;
      case K_ALT_2: case '2': vkey1 = '2'; vkey2 = VK_NUMPAD2; break;
      case K_ALT_3: case '3': vkey1 = '3'; vkey2 = VK_NUMPAD3; break;
      case K_ALT_4: case '4': vkey1 = '4'; vkey2 = VK_NUMPAD4; break;
      case K_ALT_5: case '5': vkey1 = '5'; vkey2 = VK_NUMPAD5; break;
      case K_ALT_6: case '6': vkey1 = '6'; vkey2 = VK_NUMPAD6; break;
      case K_ALT_7: case '7': vkey1 = '7'; vkey2 = VK_NUMPAD7; break;
      case K_ALT_8: case '8': vkey1 = '8'; vkey2 = VK_NUMPAD8; break;
      case K_ALT_9: case '9': vkey1 = '9'; vkey2 = VK_NUMPAD9; break;

      case K_F1:  case K_SFT_F1:  case K_CTL_F1:  case K_ALT_F1:  vkey1 = VK_F1;  break;
      case K_F2:  case K_SFT_F2:  case K_CTL_F2:  case K_ALT_F2:  vkey1 = VK_F2;  break;
      case K_F3:  case K_SFT_F3:  case K_CTL_F3:  case K_ALT_F3:  vkey1 = VK_F3;  break;
      case K_F4:  case K_SFT_F4:  case K_CTL_F4:  case K_ALT_F4:  vkey1 = VK_F4;  break;
      case K_F5:  case K_SFT_F5:  case K_CTL_F5:  case K_ALT_F5:  vkey1 = VK_F5;  break;
      case K_F6:  case K_SFT_F6:  case K_CTL_F6:  case K_ALT_F6:  vkey1 = VK_F6;  break;
      case K_F7:  case K_SFT_F7:  case K_CTL_F7:  case K_ALT_F7:  vkey1 = VK_F7;  break;
      case K_F8:  case K_SFT_F8:  case K_CTL_F8:  case K_ALT_F8:  vkey1 = VK_F8;  break;
      case K_F9:  case K_SFT_F9:  case K_CTL_F9:  case K_ALT_F9:  vkey1 = VK_F9;  break;
      case K_F10: case K_SFT_F10: case K_CTL_F10: case K_ALT_F10: vkey1 = VK_F10; break;

      case K_LEFT:  case K_CTL_LEFT:  vkey1 = VK_LEFT;   break;
      case K_RIGHT: case K_CTL_RIGHT: vkey1 = VK_RIGHT;  break;
      case K_UP:    case K_CTL_UP:    vkey1 = VK_UP;     break;
      case K_DOWN:  case K_CTL_DOWN:  vkey1 = VK_DOWN;   break;
      case K_HOME:  case K_CTL_HOME:  vkey1 = VK_HOME;   break;
      case K_END:   case K_CTL_END:   vkey1 = VK_END;    break;
      case K_PGUP:  case K_CTL_PGUP:  vkey1 = VK_PRIOR;  break;
      case K_PGDN:  case K_CTL_PGDN:  vkey1 = VK_NEXT;   break;
      case K_INS:   case K_CTL_INS:   vkey1 = VK_INSERT; break;
      case K_DEL:   case K_CTL_DEL:   vkey1 = VK_DELETE; break;

      case K_ESC:                 vkey1 = VK_ESCAPE;               break;
      case K_PAD_CENTER:          vkey1 = VK_CLEAR;                break;
      case K_BS:                  vkey1 = VK_BACK;    vkey2 = 'H'; break;
      case K_NL:                  vkey1 = VK_RETURN;  vkey2 = 'J'; break;
      case K_TAB: case K_BACKTAB: vkey1 = VK_TAB;     vkey2 = 'I'; break;

      case ' ': vkey1 = VK_SPACE;    break;
      case '*': vkey1 = VK_MULTIPLY; break;
      case '+': vkey1 = VK_ADD;      break;
      case '-': vkey1 = VK_SUBTRACT; break;
      case '/': vkey1 = VK_DIVIDE;   break;

      case K_MOUSE1: vkey1 = VK_LBUTTON; break;
      case K_MOUSE2: vkey1 = VK_MBUTTON; break;
      case K_MOUSE3: vkey1 = VK_RBUTTON; break;

      default: result = FALSE; okay = FALSE; break;
    } /* switch */

    if (okay) {
      result = (GetAsyncKeyState(vkey1) & 0x8000) != 0;
      if (!result && vkey2 != 0) {
        result = (GetAsyncKeyState(vkey2) & 0x8000) != 0;
      } /* if */
    } /* if */
    return result;
  } /* gkbButtonPressed */



#ifdef ANSI_C

chartype gkbRawGetc (void)
#else

chartype gkbRawGetc ()
#endif

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



#ifdef ANSI_C

wintype gkbWindow (void)
#else

wintype gkbWindow ()
#endif

  {
    wintype result;

  /* gkbWindow */
    result = (wintype) find_window(button_window);
    if (result != NULL) {
      result->usage_count++;
    } /* if */
    return result;
  } /* gkbWindow */



#ifdef ANSI_C

inttype gkbButtonXpos (void)
#else

inttype gkbButtonXpos ()
#endif

  { /* gkbButtonXpos */
    return button_x;
  } /* gkbButtonXpos */



#ifdef ANSI_C

inttype gkbButtonYpos (void)
#else

inttype gkbButtonYpos ()
#endif

  { /* gkbButtonYpos */
    return button_y;
  } /* gkbButtonYpos */



#ifdef ANSI_C

inttype drwPointerXpos (const_wintype actual_window)
#else

inttype drwPointerXpos (actual_window)
wintype actual_window;
#endif

  {
    POINT point;
    inttype result;

  /* drwPointerXpos */
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
    return result;
  } /* drwPointerXpos */



#ifdef ANSI_C

inttype drwPointerYpos (const_wintype actual_window)
#else

inttype drwPointerYpos (actual_window)
wintype actual_window;
#endif

  {
    POINT point;
    inttype result;

  /* drwPointerYpos */
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
    return result;
  } /* drwPointerYpos */



#ifdef ANSI_C

void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)
#else

void drwArc (actual_window, x, y, radius, startAngle, sweepAngle)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
#endif

  {
    float startAng, sweepAng;

  /* drwArc */
    startAng = (startAngle * (360.0 / (2 * PI)));
    sweepAng = (sweepAngle * (360.0 / (2 * PI)));
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, startAng, sweepAng);
  } /* drwArc */



#ifdef ANSI_C

void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)
#else

void drwPArc (actual_window, x, y, radius, startAngle, sweepAngle, col)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
inttype col;
#endif

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
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      /* MoveToEx(to_hdc(actual_window), x + radius, y, NULL); */
      AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, startAng, sweepAng);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        /* MoveToEx(to_backup_hdc(actual_window), x + radius, y, NULL); */
        AngleArc(to_backup_hdc(actual_window), x, y, (unsigned) radius, startAng, sweepAng);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPArc */



#ifdef ANSI_C

void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)
#else

void drwFArcChord (actual_window, x, y, radius, startAngle, sweepAngle)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
#endif

  { /* drwFArcChord */
  } /* drwFArcChord */



#ifdef ANSI_C

void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)
#else

void drwPFArcChord (actual_window, x, y, radius, startAngle, sweepAngle, col)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
inttype col;
#endif

  { /* drwPFArcChord */
  } /* drwPFArcChord */



#ifdef ANSI_C

void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)
#else

void drwFArcPieSlice (actual_window, x, y, radius, startAngle, sweepAngle)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
#endif

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



#ifdef ANSI_C

void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)
#else

void drwPFArcPieSlice (actual_window, x, y, radius, startAngle, sweepAngle, col)
wintype actual_window;
inttype x, y, radius;
floattype startAngle, sweepAngle;
inttype col;
#endif

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
      } else {
        old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
        BeginPath(to_hdc(actual_window));
        MoveToEx(to_hdc(actual_window), x, y, (LPPOINT) NULL);
        AngleArc(to_hdc(actual_window), x, y, radius, startAng, sweepAng);
        LineTo(to_hdc(actual_window), x, y);
        EndPath(to_hdc(actual_window));
        StrokeAndFillPath(to_hdc(actual_window));
        SelectObject(to_hdc(actual_window), old_pen);
        SelectObject(to_hdc(actual_window), old_brush);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
          BeginPath(to_backup_hdc(actual_window));
          MoveToEx(to_backup_hdc(actual_window), x, y, (LPPOINT) NULL);
          AngleArc(to_backup_hdc(actual_window), x, y, radius, startAng, sweepAng);
          LineTo(to_backup_hdc(actual_window), x, y);
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



#ifdef ANSI_C

void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)
#else

void drwArc2 (actual_window, x1, y1, x2, y2, radius)
wintype actual_window;
inttype x1, y1, x2, y2, radius;
#endif

  { /* drwArc2 */
  } /* drwArc2 */



#ifdef ANSI_C

void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwCircle */
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, 0.0, 360.0);
  } /* drwCircle */



#ifdef ANSI_C

void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPCircle (actual_window, x, y, radius, col)
wintype actual_window;
inttype x, y, radius;
inttype col;
#endif

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
      MoveToEx(to_hdc(actual_window), x + radius, y, NULL);
      AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, 0.0, 360.0);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        MoveToEx(to_backup_hdc(actual_window), x + radius, y, NULL);
        AngleArc(to_backup_hdc(actual_window), x, y, (unsigned) radius, 0.0, 360.0);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPCircle */



#ifdef ANSI_C

void drwClear (wintype actual_window, inttype col)
#else

void drwClear (actual_window, col)
wintype actual_window;
inttype col;
#endif

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
            to_width(actual_window), to_height(actual_window));
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwClear */



#ifdef ANSI_C

void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y)
#else

void drwCopyArea (src_window, dest_window, src_x, src_y, width, height,
    dest_x, dest_y)
wintype src_window;
wintype dest_window;
inttype src_x;
inttype src_y;
inttype width;
inttype height;
inttype dest_x;
inttype dest_y;
#endif

  { /* drwCopyArea */
#ifdef TRACE_WIN
    printf("drwCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
        src_window, dest_window, src_x, src_y, width, height, dest_x, dest_y);
#endif
    if (to_backup_hdc(src_window) != 0) {
      BitBlt(to_hdc(dest_window), dest_x, dest_y, width, height,
          to_backup_hdc(src_window), src_x, src_y, SRCCOPY);
      if (to_backup_hdc(dest_window) != 0) {
        BitBlt(to_backup_hdc(dest_window), dest_x, dest_y, width, height,
            to_backup_hdc(src_window), src_x, src_y, SRCCOPY);
      } /* if */
    } else {
      BitBlt(to_hdc(dest_window), dest_x, dest_y, width, height,
          to_hdc(src_window), src_x, src_y, SRCCOPY);
      if (to_backup_hdc(dest_window) != 0) {
        BitBlt(to_backup_hdc(dest_window), dest_x, dest_y, width, height,
            to_hdc(src_window), src_x, src_y, SRCCOPY);
      } /* if */
    } /* if */
  } /* drwCopyArea */



#ifdef ANSI_C

void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwFCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwFCircle */
  } /* drwFCircle */



#ifdef ANSI_C

void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPFCircle (actual_window, x, y, radius, col)
wintype actual_window;
inttype x, y, radius;
inttype col;
#endif

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
      Ellipse(to_hdc(actual_window), x - radius, y - radius,
          x + radius + 1, y + radius + 1);
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Ellipse(to_backup_hdc(actual_window), x - radius, y - radius,
            x + radius + 1, y + radius + 1);
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPFCircle */



#ifdef ANSI_C

void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void drwFEllipse (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* drwFEllipse */
  } /* drwFEllipse */



#ifdef ANSI_C

void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)
#else

void drwPFEllipse (actual_window, x, y, width, height, col)
wintype actual_window;
inttype x, y, width, height;
inttype col;
#endif

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
      Ellipse(to_hdc(actual_window), x, y, x + width, y + height);
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Ellipse(to_backup_hdc(actual_window), x, y, x + width, y + height);
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPFEllipse */



#ifdef ANSI_C

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
  } /* drwFlush */



#ifdef ANSI_C

void drwFree (wintype old_window)
#else

void drwFree (old_window)
wintype old_window;
#endif

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
      remove_window((win_wintype) old_window);
    } /* if */
    FREE_RECORD(old_window, win_winrecord, count.win);
  } /* drwFree */



#ifdef ANSI_C

wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height)
#else

wintype drwGet (actual_window, left, upper, width, height)
wintype actual_window;
inttype left;
inttype upper;
inttype width;
inttype height;
#endif

  {
    win_wintype result;

  /* drwGet */
    if (!ALLOC_RECORD(result, win_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      result->hdc = CreateCompatibleDC(to_hdc(actual_window));
      result->hBitmap = CreateCompatibleBitmap(to_hdc(actual_window), width, height);
      if (result->hBitmap == NULL) {
        free(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        result->oldBitmap = (HBITMAP) SelectObject(result->hdc, result->hBitmap);
        result->hasTransparentPixel = FALSE;
        result->transparentPixel = 0;
        result->is_pixmap = TRUE;
        result->width = width;
        result->height = height;
        result->next = NULL;
        if (to_backup_hdc(actual_window) != 0) {
          BitBlt(result->hdc, 0, 0, width, height,
              to_backup_hdc(actual_window), left, upper, SRCCOPY);
        } else {
          BitBlt(result->hdc, 0, 0, width, height,
              to_hdc(actual_window), left, upper, SRCCOPY);
        } /* if */
      } /* if */
    } /* if */
    return (wintype) result;
  } /* drwGet */



#ifdef ANSI_C

inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y)
#else

inttype drwGetPixel (actual_window, x, y)
wintype actual_window;
inttype x;
inttype y;
#endif

  { /* drwGetPixel */
    return (inttype) GetPixel(to_hdc(actual_window), x, y);
  } /* drwGetPixel */



#ifdef ANSI_C

inttype drwHeight (const_wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  {
    RECT rect;
    inttype result;

  /* drwHeight */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      result = to_height(actual_window);
    } else {
      result = rect.bottom - rect.top - to_brutto_height_delta(actual_window);
    } /* if */
    return result;
  } /* drwHeight */



#ifdef ANSI_C

wintype drwImage (inttype *image_data, inttype width, inttype height)
#else

wintype drwImage (image_data, width, height)
inttype *image_data;
inttype width;
inttype height;
#endif

  {
    inttype xPos;
    inttype yPos;
    wintype result;

  /* drwImage */
    result = drwNewPixmap(width, height);
    if (result != NULL) {
      for (yPos = 0; yPos < height; yPos++) {
        for (xPos = 0; xPos < width; xPos++) {
          drwPPoint(result, xPos, yPos, image_data[yPos * width + xPos]);
        } /* for */
      } /* for */
    } /* if */
    return result;
  } /* drwImage */



#ifdef ANSI_C

void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)
#else

void drwLine (actual_window, x1, y1, x2, y2)
wintype actual_window;
inttype x1, y1, x2, y2;
#endif

  { /* drwLine */
    MoveToEx(to_hdc(actual_window), x1, y1, NULL);
    LineTo(to_hdc(actual_window), x2, y2);
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
      LineTo(to_backup_hdc(actual_window), x2, y2);
    } /* if */
  } /* drwLine */



#ifdef ANSI_C

void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)
#else

void drwPLine (actual_window, x1, y1, x2, y2, col)
wintype actual_window;
inttype x1, y1, x2, y2;
inttype col;
#endif

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
      MoveToEx(to_hdc(actual_window), x1, y1, NULL);
      LineTo(to_hdc(actual_window), x2, y2);
      SetPixel(to_hdc(actual_window), x2, y2, (COLORREF) col);
      SelectObject(to_hdc(actual_window), old_pen);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
        LineTo(to_backup_hdc(actual_window), x2, y2);
        SetPixel(to_backup_hdc(actual_window), x2, y2, (COLORREF) col);
        SelectObject(to_backup_hdc(actual_window), old_pen);
      } /* if */
      DeleteObject(current_pen);
    } /* if */
  } /* drwPLine */



#ifdef ANSI_C

wintype drwNewPixmap (inttype width, inttype height)
#else

wintype drwNewPixmap (width, height)
inttype width;
inttype height;
#endif

  {
    HDC screenDC;
    win_wintype result;

  /* drwNewPixmap */
    if (!ALLOC_RECORD(result, win_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      screenDC = GetDC(NULL);
      result->hdc = CreateCompatibleDC(screenDC);
      result->hBitmap = CreateCompatibleBitmap(screenDC, width, height);
      ReleaseDC(NULL, screenDC);
      result->oldBitmap = (HBITMAP) SelectObject(result->hdc, result->hBitmap);
      result->hasTransparentPixel = FALSE;
      result->transparentPixel = 0;
      result->is_pixmap = TRUE;
      result->width = width;
      result->height = height;
      result->next = NULL;
    } /* if */
    return (wintype) result;
  } /* drwNewPixmap */



#ifdef ANSI_C

wintype drwNewBitmap (const_wintype actual_window, inttype width, inttype height)
#else

wintype drwNewBitmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  { /* drwNewBitmap */
    return 0;
  } /* drwNewBitmap */



#ifdef ANSI_C

static void dra_init (void)
#else

static void dra_init ()
#endif

  {
    WNDCLASSEX wcex = {0};

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
    init_called = 1;
  } /* dra_init */



#ifdef ANSI_C

wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name)
#else

wintype drwOpen (xPos, yPos, width, height, window_name)
inttype xPos;
inttype yPos;
inttype width;
inttype height;
stritype window_name;
#endif

  {
    char *win_name;
    HFONT std_font;
    win_wintype result;

  /* drwOpen */
#ifdef TRACE_WIN
    printf("BEGIN drwOpen(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
#ifdef DO_HEAP_STATISTIC
    count.size_winrecord = SIZ_REC(win_winrecord);
#endif
    result = NULL;
    if (init_called == 0) {
      dra_init();
    } /* if */
    if (init_called != 0) {
      win_name = cp_to_cstri8(window_name);
      if (win_name == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        if (ALLOC_RECORD(result, win_winrecord, count.win)) {
          memset(result, 0, sizeof(struct win_winstruct));
          result->usage_count = 1;
          result->next = window_list;
          window_list = result;
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
          /* printf("hWnd=%lu\n", result->hWnd); */
          if (result->hWnd != NULL) {
            result->hdc = GetDC(result->hWnd);
            /* printf("hdc=%lu\n", result->hdc); */
            result->hasTransparentPixel = FALSE;
            result->transparentPixel = 0;
            result->is_pixmap = FALSE;
            result->width = width;
            result->height = height;
            result->clear_col = (inttype) RGB(0, 0, 0); /* black */
            result->backup_hdc = CreateCompatibleDC(result->hdc);
            result->backup = CreateCompatibleBitmap(result->hdc, width, height);
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
#ifdef TRACE_WIN
    printf("END drwOpen ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpen */



#ifdef ANSI_C

wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)
#else

wintype drwOpenSubWindow (parent_window, xPos, yPos, width, height)
const_wintype parent_window;
inttype xPos;
inttype yPos;
inttype width;
inttype height;
#endif

  {
    HFONT std_font;
    win_wintype result;

  /* drwOpenSubWindow */
#ifdef TRACE_WIN
    printf("BEGIN drwOpenSubWindow(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
#ifdef DO_HEAP_STATISTIC
    count.size_winrecord = SIZ_REC(win_winrecord);
#endif
    result = NULL;
    if (init_called == 0) {
      dra_init();
    } /* if */
    if (init_called != 0) {
      if (ALLOC_RECORD(result, win_winrecord, count.win)) {
        memset(result, 0, sizeof(struct win_winstruct));
        result->usage_count = 1;
        result->next = window_list;
        window_list = result;
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
            xPos, yPos, width, height,
            to_hwnd(parent_window), (HMENU) NULL, NULL, NULL);
#ifdef OUT_OF_ORDER
        result->hWnd = CreateWindowEx(0, windowClass, "",
            (WS_VISIBLE | WS_SYSMENU /* | WS_THICKFRAME */),
            xPos, yPos, width, height,
            to_hwnd(parent_window), NULL, NULL /* hInstance */, NULL);
#endif

        /* printf("hWnd=%lu\n", result->hWnd); */
        if (result->hWnd != NULL) {
          SetWindowLong(result->hWnd , GWL_STYLE, GetWindowLong(result->hWnd , GWL_STYLE) &~ WS_CAPTION);
          SetWindowPos(result->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
          result->hdc = GetDC(result->hWnd);
          /* printf("hdc=%lu\n", result->hdc); */
          result->hasTransparentPixel = FALSE;
          result->transparentPixel = 0;
          result->is_pixmap = FALSE;
          result->width = width;
          result->height = height;
          result->clear_col = (inttype) RGB(0, 0, 0); /* black */
          result->backup_hdc = CreateCompatibleDC(result->hdc);
          result->backup = CreateCompatibleBitmap(result->hdc, width, height);
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
#ifdef TRACE_WIN
    printf("END drwOpenSubWindow ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpenSubWindow */



#ifdef ANSI_C

void drwPoint (const_wintype actual_window, inttype x, inttype y)
#else

void drwPoint (actual_window, x, y)
wintype actual_window;
inttype x, y;
#endif

  { /* drwPoint */
    MoveToEx(to_hdc(actual_window), x, y, NULL);
    LineTo(to_hdc(actual_window), x + 1, y + 1);
    if (to_backup_hdc(actual_window) != 0) {
      MoveToEx(to_backup_hdc(actual_window), x, y, NULL);
      LineTo(to_backup_hdc(actual_window), x + 1, y + 1);
    } /* if */
  } /* drwPoint */



#ifdef ANSI_C

void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col)
#else

void drwPPoint (actual_window, x, y, col)
wintype actual_window;
inttype x, y;
inttype col;
#endif

  { /* drwPPoint */
#ifdef TRACE_WIN
    printf("drwPPoint(%lu, %ld, %ld, %lx)\n", actual_window, x, y, col);
#endif
    SetPixel(to_hdc(actual_window), x, y, (COLORREF) col);
    if (to_backup_hdc(actual_window) != 0) {
      SetPixel(to_backup_hdc(actual_window), x, y, (COLORREF) col);
    } /* if */
  } /* drwPPoint */



#ifdef ANSI_C

void drwConvPointList (bstritype pointList, inttype *xy)
#else

void drwConvPointList (pointList, xy)
     bstritype pointList;
inttype *xy;
#endif

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



#ifdef ANSI_C

bstritype drwGenPointList (const const_rtlArraytype xyArray)
#else

bstritype drwGenPointList (xyArray)
rtlArraytype xyArray;
#endif

  {
    uinttype num_elements;
    memsizetype len;
    POINT *points;
    memsizetype pos;
    bstritype result;

  /* drwGenPointList */
    /* printf("drwGenPointList(%ld .. %ld)\n", xyArray->min_position, xyArray->max_position); */
    num_elements = (uinttype) (xyArray->max_position - xyArray->min_position) + 1;
    if (num_elements & 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (num_elements >> 1 > MAX_BSTRI_LEN / sizeof(POINT)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      len = (memsizetype) num_elements >> 1;
      if (!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(POINT))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = len * sizeof(POINT);
        if (len > 0) {
          points = (POINT *) result->mem;
          for (pos = 0; pos < len; pos ++) {
            points[pos].x = xyArray->arr[ pos << 1     ].value.intvalue;
            points[pos].y = xyArray->arr[(pos << 1) + 1].value.intvalue;
          } /* for */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGenPointList */



#ifdef ANSI_C

inttype drwLngPointList (bstritype point_list)
#else

inttype drwLngPointList (point_list)
bstritype point_list;
#endif

  { /* drwLngPointList */
    return point_list->size / sizeof(POINT);
  } /* drwLngPointList */



#ifdef ANSI_C

void drwPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col)
#else

void drwPolyLine (actual_window, x1, y1, point_list, col)
wintype actual_window;
inttype x1, y1;
const_bstritype point_list;
inttype col;
#endif

  {
    POINT *points;
    int npoints;
    int pos;
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
        MoveToEx(to_hdc(actual_window), x1 + points[0].x, y1 + points[0].y, NULL);
        for (pos = 1; pos < npoints; pos ++) {
          LineTo(to_hdc(actual_window), x1 + points[pos].x, y1 + points[pos].y);
        } /* for */
        SetPixel(to_hdc(actual_window), x1 + points[npoints - 1].x, y1 + points[npoints - 1].y, (COLORREF) col);
        SelectObject(to_hdc(actual_window), old_pen);
        if (to_backup_hdc(actual_window) != 0) {
          old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
          MoveToEx(to_backup_hdc(actual_window), x1 + points[0].x, y1 + points[0].y, NULL);
          for (pos = 1; pos < npoints; pos ++) {
            LineTo(to_backup_hdc(actual_window), x1 + points[pos].x, y1 + points[pos].y);
          } /* for */
          SetPixel(to_backup_hdc(actual_window), x1 + points[npoints - 1].x, y1 + points[npoints - 1].y, (COLORREF) col);
          SelectObject(to_backup_hdc(actual_window), old_pen);
        } /* if */
        DeleteObject(current_pen);
      } /* if */
    } /* if */
  } /* drwPolyLine */



#ifdef ANSI_C

void drwFPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col)
#else

void drwFPolyLine (actual_window, x1, y1, point_list, col)
wintype actual_window;
inttype x1, y1;
const_bstritype point_list;
inttype col;
#endif

  {
    POINT *points;
    int npoints;
    int pos;
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwFPolyLine */
    points = (POINT *) point_list->mem;
    npoints = point_list->size / sizeof(POINT);
    for (pos = 0; pos < npoints; pos ++) {
      points[pos].x += x1;
      points[pos].y += y1;
    } /* for */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL || current_brush == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      old_pen = (HPEN) SelectObject(to_hdc(actual_window), current_pen);
      old_brush = (HBRUSH) SelectObject(to_hdc(actual_window), current_brush);
      Polygon(to_hdc(actual_window), points, npoints);
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        Polygon(to_backup_hdc(actual_window), points, npoints);
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
    for (pos = 0; pos < npoints; pos ++) {
      points[pos].x -= x1;
      points[pos].y -= y1;
    } /* for */
  } /* drwFPolyLine */



#ifdef ANSI_C

void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x, inttype y)
#else

void drwPut (actual_window, pixmap, x, y)
wintype actual_window;
wintype pixmap;
inttype x;
inttype y;
#endif

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
        BitBlt(to_hdc(actual_window), x, y, to_width(pixmap), to_height(pixmap),
            to_hdc(pixmap), 0, 0, SRCCOPY);
        if (to_backup_hdc(actual_window) != 0) {
          BitBlt(to_backup_hdc(actual_window), x, y, to_width(pixmap), to_height(pixmap),
              to_hdc(pixmap), 0, 0, SRCCOPY);
        } /* if */
#ifdef USE_TRANSPARENTBLT
      } /* if */
#endif
    } /* if */
  } /* drwPut */



#ifdef ANSI_C

void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void drwRect (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* drwRect */
    Rectangle(to_hdc(actual_window), x, y, x + width, y + height);
    if (to_backup_hdc(actual_window) != 0) {
      Rectangle(to_backup_hdc(actual_window), x, y, x + width, y + height);
    } /* if */
  } /* drwRect */



#ifdef ANSI_C

void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)
#else

void drwPRect (actual_window, x, y, width, height, col)
wintype actual_window;
inttype x, y, width, height;
inttype col;
#endif

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
          SetPixel(to_hdc(actual_window), x, y, (COLORREF) col);
        } else {
          MoveToEx(to_hdc(actual_window), x, y, NULL);
          LineTo(to_hdc(actual_window), x, y + height);
        } /* if */
      } else {
        if (height == 1) {
          MoveToEx(to_hdc(actual_window), x, y, NULL);
          LineTo(to_hdc(actual_window), x + width, y);
        } else {
          Rectangle(to_hdc(actual_window), x, y, x + width, y + height);
        } /* if */
      } /* if */
      SelectObject(to_hdc(actual_window), old_pen);
      SelectObject(to_hdc(actual_window), old_brush);
      if (to_backup_hdc(actual_window) != 0) {
        old_pen = (HPEN) SelectObject(to_backup_hdc(actual_window), current_pen);
        old_brush = (HBRUSH) SelectObject(to_backup_hdc(actual_window), current_brush);
        if (width == 1) {
          if (height == 1) {
            SetPixel(to_backup_hdc(actual_window), x, y, (COLORREF) col);
          } else {
            MoveToEx(to_backup_hdc(actual_window), x, y, NULL);
            LineTo(to_backup_hdc(actual_window), x, y + height);
          } /* if */
        } else {
          if (height == 1) {
            MoveToEx(to_backup_hdc(actual_window), x, y, NULL);
            LineTo(to_backup_hdc(actual_window), x + width, y);
          } else {
            Rectangle(to_backup_hdc(actual_window), x, y, x + width, y + height);
          } /* if */
        } /* if */
        SelectObject(to_backup_hdc(actual_window), old_pen);
        SelectObject(to_backup_hdc(actual_window), old_brush);
      } /* if */
      DeleteObject(current_pen);
      DeleteObject(current_brush);
    } /* if */
  } /* drwPRect */



#ifdef ANSI_C

inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)
#else

inttype drwRgbColor (red_val, green_val, blue_val)
inttype red_val;
inttype green_val;
inttype blue_val;
#endif

  { /* drwRgbColor */
#ifdef TRACE_WIN
    printf("drwRgbColor(%lu, %ld, %ld)\n", red_val, green_val, blue_val);
#endif
    return (inttype) RGB(((uinttype) red_val) >> 8,
                         ((uinttype) green_val) >> 8,
                         ((uinttype) blue_val) >> 8);
  } /* drwRgbColor */



#ifdef ANSI_C

void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)
#else

void drwPixelToRgb (col, red_val, green_val, blue_val)
inttype col;
inttype *red_val;
inttype *green_val;
inttype *blue_val;
#endif

  { /* drwPixelToRgb */
    *red_val   = GetRValue(col) << 8;
    *green_val = GetGValue(col) << 8;
    *blue_val  = GetBValue(col) << 8;
  } /* drwPixelToRgb */



#ifdef ANSI_C

void drwBackground (inttype col)
#else

void drwBackground (col)
inttype col;
#endif

  { /* drwBackground */
  } /* drwBackground */



#ifdef ANSI_C

void drwColor (inttype col)
#else

void drwColor (col)
inttype col;
#endif

  { /* drwColor */
      /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
  } /* drwColor */



#ifdef ANSI_C

void drwSetContent (const_wintype actual_window, const_wintype pixmap)
#else

void drwSetContent (actual_window, pixmap)
wintype actual_window;
wintype pixmap;
#endif

  { /* drwSetContent */
    /* printf("begin drwSetContent(%lu, %lu)\n",
        to_window(actual_window), to_window(pixmap)); */
    if (pixmap != NULL) {
      BitBlt(to_hdc(actual_window), 0, 0, to_width(pixmap), to_height(pixmap),
          to_hdc(pixmap), 0, 0, SRCCOPY);
      if (to_backup_hdc(actual_window) != 0) {
        BitBlt(to_backup_hdc(actual_window), 0, 0, to_width(pixmap), to_height(pixmap),
            to_hdc(pixmap), 0, 0, SRCCOPY);
      } /* if */
    } /* if */
    /* printf("end drwSetContent(%lu, %lu)\n",
        to_window(actual_window), to_window(pixmap)); */
  } /* drwSetContent */



#ifdef ANSI_C

void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)
#else

void drwSetPos (actual_window, xPos, yPos)
wintype actual_window;
inttype xPos, yPos;
#endif

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_hwnd(actual_window), xPos, yPos); */
    SetWindowPos(to_hwnd(actual_window), 0, xPos, yPos, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOZORDER | SWP_NOSIZE);
    gkbKeyPressed();
    /* printf("end drwSetPos(%lu, %ld, %ld)\n",
        to_hwnd(actual_window), xPos, yPos); */
  } /* drwSetPos */



#ifdef ANSI_C

void drwSetTransparentColor (wintype pixmap, inttype col)
#else

void drwSetTransparentColor (pixmap, col)
wintype pixmap;
inttype col;
#endif

  { /* drwSetTransparentColor */
    to_hasTransparentPixel(pixmap) = TRUE;
    to_transparentPixel(pixmap) = col;
  } /* drwSetTransparentColor */



#ifdef ANSI_C

void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol)
#else

void drwText (actual_window, x, y, stri, col, bkcol)
wintype actual_window;
inttype x, y;
stritype stri;
inttype col;
inttype bkcol;
#endif

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
      TextOutW(to_hdc(actual_window), x, y, stri_buffer, stri->size);
      if (to_backup_hdc(actual_window) != 0) {
        SetTextColor(to_backup_hdc(actual_window), (COLORREF) col);
        SetBkColor(to_backup_hdc(actual_window), (COLORREF) bkcol);
        SetTextAlign(to_backup_hdc(actual_window), TA_BASELINE | TA_LEFT);
        TextOutW(to_backup_hdc(actual_window), x, y, stri_buffer, stri->size);
      } /* if */
      free(stri_buffer);
    } /* if */
  } /* drwText */



#ifdef ANSI_C

void drwToBottom (const_wintype actual_window)
#else

void drwToBottom (actual_window)
wintype actual_window;
#endif

  { /* drwToBottom */
    /* printf("begin drwToBottom(%lu)\n", to_window(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_BOTTOM, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToBottom(%lu)\n", to_window(actual_window)); */
  } /* drwToBottom */



#ifdef ANSI_C

void drwToTop (const_wintype actual_window)
#else

void drwToTop (actual_window)
wintype actual_window;
#endif

  { /* drwToTop */
    /* printf("begin drwToTop(%lu)\n", to_window(actual_window)); */
    SetWindowPos(to_hwnd(actual_window), HWND_TOP, 0, 0, 0, 0,
        /* SWP_NOSENDCHANGING | */ SWP_NOMOVE | SWP_NOSIZE);
    /* printf("end drwToTop(%lu)\n", to_window(actual_window)); */
  } /* drwToTop */



#ifdef ANSI_C

inttype drwWidth (const_wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  {
    RECT rect;
    inttype result;

  /* drwWidth */
    if (GetWindowRect(to_hwnd(actual_window), &rect) == 0) {
      result = to_width(actual_window);
    } else {
      result = rect.right - rect.left - to_brutto_width_delta(actual_window);
    } /* if */
    return result;
  } /* drwWidth */



#ifdef ANSI_C

inttype drwXPos (const_wintype actual_window)
#else

inttype drwXPos (actual_window)
wintype actual_window;
#endif

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



#ifdef ANSI_C

inttype drwYPos (const_wintype actual_window)
#else

inttype drwYPos (actual_window)
wintype actual_window;
#endif

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
