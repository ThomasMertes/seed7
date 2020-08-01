/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilitys.     */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/drw_win.c                                       */
/*  Changes: 2005 - 2007  Thomas Mertes                             */
/*  Content: Graphic access using the windows capabilitys.          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "kbd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"

#define TRACE_ENTITYS


#define PI 3.14159265358979323846264338327950284197

#define windowClass "MyWindowClass"

static inttype init_called = 0;

static inttype button_x = 0;
static inttype button_y = 0;


typedef struct win_winstruct {
  unsigned long usage_count;
  HWND hWnd;
  HDC hdc;
  HBITMAP backup;
  HDC backup_hdc;
  HBITMAP hBitmap;
  HBITMAP oldBitmap;
  booltype is_pixmap;
  unsigned int width;
  unsigned int height;
  struct win_winstruct *next;
} *win_wintype;

static win_wintype window_list = NULL;

#define to_hwnd(win)        (((win_wintype) win)->hWnd)
#define to_hdc(win)         (((win_wintype) win)->hdc)
#define to_backup_hdc(win)  (((win_wintype) win)->backup_hdc)
#define to_backup(win)      (((win_wintype) win)->backup)
#define to_hBitmap(win)     (((win_wintype) win)->hBitmap)
#define to_oldBitmap(win)   (((win_wintype) win)->oldBitmap)
#define is_pixmap(win)      (((win_wintype) win)->is_pixmap)
#define to_width(win)       (((win_wintype) win)->width)
#define to_height(win)      (((win_wintype) win)->height)

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
        return(window);
      } /* if */
      window = window->next;
    } /* while */
    return(NULL);
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
    if (current_pen == NULL) {
      printf("drwPRect pen with color %lx is NULL\n", col);
    } /* if */
    if (current_brush == NULL) {
      printf("drwPRect brush with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(actual_window->hdc, current_pen);
    old_brush = SelectObject(actual_window->hdc, current_brush);
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
          BitBlt(to_hdc(paint_window),
              paintStruct.rcPaint.left, paintStruct.rcPaint.top,
              paintStruct.rcPaint.right - paintStruct.rcPaint.left,
              paintStruct.rcPaint.bottom - paintStruct.rcPaint.top,
              to_backup_hdc(paint_window),
              paintStruct.rcPaint.left, paintStruct.rcPaint.top, SRCCOPY);
          EndPaint(paint_window->hWnd, &paintStruct);
        } else {
          printf("paint_window=%lu, backup_hdc=%lu\n",
              paint_window, paint_window->backup_hdc);
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
          } else {
            /* printf("GetUpdateRect no update region\n"); */
            GetClientRect(paint_window->hWnd, &rect);
            /* printf("GetClientRect left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect.top, rect.right, rect.bottom); */
          } /* if */
          /* printf("window width=%ld, height=%ld\n",
              paint_window->width, paint_window->height); */
          if (rect.right >= paint_window->width) {
            if (rect.left < paint_window->width) {
              rect2.left = paint_window->width;
            } else {
              rect2.left = rect.left;
            } /* if */
            if (rect.bottom >= paint_window->height) {
              rect2.bottom = paint_window->height - 1;
            } else {
              rect2.bottom = rect.bottom;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect2.left, rect.top, rect.right, rect2.bottom); */
            drawRectangle(paint_window, rect2.left, rect.top, rect.right, rect2.bottom,
                RGB(255, 255, 255));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
          if (rect.bottom >= paint_window->height) {
            if (rect.top < paint_window->height) {
              rect2.top = paint_window->height;
            } else {
              rect2.top = rect.top;
            } /* if */
            /* printf("drawRectangle left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect2.top, rect.right, rect.bottom); */
            drawRectangle(paint_window, rect.left, rect2.top, rect.right, rect.bottom,
                RGB(255, 255, 255));
                /* GetBkColor(paint_window->hWnd)); */
          } /* if */
        } else {
          printf("paint_window=%lu, backup_hdc=%lu\n",
              paint_window, paint_window->backup_hdc);
        } /* if */
        result = 1;
        break;
      default:
        result = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    } /* switch */
    /* printf("WndProc ==> %d\n", result); */
    return(result);
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
          result = K_MOUSE1;
        } else if (msg.message == WM_MBUTTONDOWN) {
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          result = K_MOUSE2;
        } else if (msg.message == WM_RBUTTONDOWN) {
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
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
    return(result);
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
    return(result);
  } /* gkbKeyPressed */



#ifdef ANSI_C

chartype gkbRawGetc (void)
#else

chartype gkbRawGetc ()
#endif

  { /* gkbRawGetc */
    return(gkbGetc());
  } /* gkbRawGetc */



#ifdef ANSI_C

inttype gkbXpos (void)
#else

inttype gkbXpos ()
#endif

  { /* gkbXpos */
    return(button_x);
  } /* gkbXpos */



#ifdef ANSI_C

inttype gkbYpos (void)
#else

inttype gkbYpos ()
#endif

  { /* gkbYpos */
    return(button_y);
  } /* gkbYpos */



#ifdef ANSI_C

void drwArc (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwArc (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  {
    float angle1, angle2;

  /* drwArc */
    angle1 = (ang1 * (360.0 / (2 * PI)));
    angle2 = (ang2 * (360.0 / (2 * PI)));
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, angle1, angle2);
  } /* drwArc */



#ifdef ANSI_C

void drwPArc (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPArc (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  {
    float angle1, angle2;
    HPEN old_pen;
    HPEN current_pen;

  /* drwPArc */
    angle1 = (ang1 * (360.0 / (2 * PI)));
    angle2 = (ang2 * (360.0 / (2 * PI)));
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
      printf("drwPFCircle pen with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    /* MoveToEx(to_hdc(actual_window), x + radius, y, NULL); */
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, angle1, angle2);
    SelectObject(to_hdc(actual_window), old_pen);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      /* MoveToEx(to_backup_hdc(actual_window), x + radius, y, NULL); */
      AngleArc(to_backup_hdc(actual_window), x, y, (unsigned) radius, angle1, angle2);
      SelectObject(to_backup_hdc(actual_window), old_pen);
    } /* if */
    DeleteObject(current_pen);
  } /* drwPArc */



#ifdef ANSI_C

void drwFArcChord (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcChord (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwFArcChord */
  } /* drwFArcChord */



#ifdef ANSI_C

void drwPFArcChord (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcChord (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPFArcChord */
  } /* drwPFArcChord */



#ifdef ANSI_C

void drwFArcPieSlice (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcPieSlice (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



#ifdef ANSI_C

void drwPFArcPieSlice (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcPieSlice (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPFArcPieSlice */
  } /* drwPFArcPieSlice */



#ifdef ANSI_C

void drwArc2 (wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)
#else

void drwArc2 (actual_window, x1, y1, x2, y2, radius)
wintype actual_window;
inttype x1, y1, x2, y2, radius;
#endif

  { /* drwArc2 */
  } /* drwArc2 */



#ifdef ANSI_C

void drwCircle (wintype actual_window,
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

void drwPCircle (wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPCircle (actual_window, x, y, col)
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
      printf("drwPFCircle pen with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    MoveToEx(to_hdc(actual_window), x + radius, y, NULL);
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, 0.0, 360.0);
    SelectObject(to_hdc(actual_window), old_pen);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      MoveToEx(to_backup_hdc(actual_window), x + radius, y, NULL);
      AngleArc(to_backup_hdc(actual_window), x, y, (unsigned) radius, 0.0, 360.0);
      SelectObject(to_backup_hdc(actual_window), old_pen);
    } /* if */
    DeleteObject(current_pen);
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
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    old_brush = SelectObject(to_hdc(actual_window), current_brush);
    Rectangle(to_hdc(actual_window), 0, 0,
        to_width(actual_window), to_height(actual_window));
    SelectObject(to_hdc(actual_window), old_pen);
    SelectObject(to_hdc(actual_window), old_brush);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      old_brush = SelectObject(to_backup_hdc(actual_window), current_brush);
      Rectangle(to_backup_hdc(actual_window), 0, 0,
          to_width(actual_window), to_height(actual_window));
      SelectObject(to_backup_hdc(actual_window), old_pen);
      SelectObject(to_backup_hdc(actual_window), old_brush);
    } /* if */
    DeleteObject(current_pen);
    DeleteObject(current_brush);
  } /* drwClear */



#ifdef ANSI_C

void drwCopyArea (wintype src_window, wintype dest_window,
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

void drwCpy (wintype *win_to, wintype win_from)
#else

void drwCpy (win_to, win_from)
wintype *win_to;
wintype win_from;
#endif

  { /* drwCpy */
#ifdef TRACE_WIN
    printf("drwCpy(%lu, %ld)\n", win_to, win_from);
#endif
    if (*win_to != NULL) {
      (*win_to)->usage_count--;
      if ((*win_to)->usage_count == 0) {
        drwFree(*win_to);
      } /* if */
    } /* if */
    *win_to = win_from;
    if (win_from != NULL) {
      win_from->usage_count++;
    } /* if */
  } /* drwCpy */



#ifdef ANSI_C

wintype drwCreate (wintype win_from)
#else

wintype drwCreate (win_from)
wintype win_from;
#endif

  { /* drwCreate */
    if (win_from != NULL) {
      win_from->usage_count++;
    } /* if */
    return(win_from);
  } /* drwCreate */



#ifdef ANSI_C

void drwDestr (wintype old_win)
#else

void drwDestr (old_win)
wintype old_win;
#endif

  { /* drwDestr */
    if (old_win != NULL) {
      old_win->usage_count--;
      if (old_win->usage_count == 0) {
        drwFree(old_win);
      } /* if */
    } /* if */
  } /* drwDestr */



#ifdef ANSI_C

void drwFCircle (wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwFCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwFCircle */
  } /* drwFCircle */



#ifdef ANSI_C

void drwPFCircle (wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPFCircle (actual_window, x, y, col)
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
    if (current_pen == NULL) {
      printf("drwPFCircle pen with color %lx is NULL\n", col);
    } /* if */
    if (current_brush == NULL) {
      printf("drwPFCircle brush with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    old_brush = SelectObject(to_hdc(actual_window), current_brush);
    Ellipse(to_hdc(actual_window), x - radius, y - radius,
        x + radius, y + radius);
    SelectObject(to_hdc(actual_window), old_pen);
    SelectObject(to_hdc(actual_window), old_brush);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      old_brush = SelectObject(to_backup_hdc(actual_window), current_brush);
      Ellipse(to_backup_hdc(actual_window), x - radius, y - radius,
          x + radius, y + radius);
      SelectObject(to_backup_hdc(actual_window), old_pen);
      SelectObject(to_backup_hdc(actual_window), old_brush);
    } /* if */
    DeleteObject(current_pen);
    DeleteObject(current_brush);
  } /* drwPFCircle */



#ifdef ANSI_C

void drwFEllipse (wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void drwFEllipse (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* drwFEllipse */
  } /* drwFEllipse */



#ifdef ANSI_C

void drwPFEllipse (wintype actual_window,
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
    if (current_pen == NULL) {
      printf("drwPFEllipse pen with color %lx is NULL\n", col);
    } /* if */
    if (current_brush == NULL) {
      printf("drwPFEllipse brush with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    old_brush = SelectObject(to_hdc(actual_window), current_brush);
    Ellipse(to_hdc(actual_window), x, y, x + width, y + height);
    SelectObject(to_hdc(actual_window), old_pen);
    SelectObject(to_hdc(actual_window), old_brush);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      old_brush = SelectObject(to_backup_hdc(actual_window), current_brush);
      Ellipse(to_backup_hdc(actual_window), x, y, x + width, y + height);
      SelectObject(to_backup_hdc(actual_window), old_pen);
      SelectObject(to_backup_hdc(actual_window), old_brush);
    } /* if */
    DeleteObject(current_pen);
    DeleteObject(current_brush);
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
    free((win_wintype) old_window);
  } /* drwFree */



#ifdef ANSI_C

wintype drwGet (wintype actual_window, inttype left, inttype upper,
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
    result = (win_wintype) malloc(sizeof(struct win_winstruct));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
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
        result->oldBitmap = SelectObject(result->hdc, result->hBitmap);
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
    return((wintype) result);
  } /* drwGet */



#ifdef ANSI_C

inttype drwHeight (wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  { /* drwHeight */
    return(to_height(actual_window));
  } /* drwHeight */



#ifdef ANSI_C

wintype drwImage (wintype actual_window, inttype *image_data,
    inttype width, inttype height)
#else

wintype drwImage (actual_window, image_data, width, height)
wintype actual_window;
inttype *image_data;
inttype width;
inttype height;
#endif

  { /* drwImage */
    return((wintype) NULL);
  } /* drwImage */



#ifdef ANSI_C

void drwLine (wintype actual_window,
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

void drwPLine (wintype actual_window,
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
      printf("drwPLine pen with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    MoveToEx(to_hdc(actual_window), x1, y1, NULL);
    LineTo(to_hdc(actual_window), x2, y2);
    SetPixel(to_hdc(actual_window), x2, y2, (COLORREF) col);
    SelectObject(to_hdc(actual_window), old_pen);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
      LineTo(to_backup_hdc(actual_window), x2, y2);
      SetPixel(to_backup_hdc(actual_window), x2, y2, (COLORREF) col);
      SelectObject(to_backup_hdc(actual_window), old_pen);
    } /* if */
    DeleteObject(current_pen);
  } /* drwPLine */



#ifdef ANSI_C

wintype drwNewPixmap (wintype actual_window, inttype width, inttype height)
#else

wintype drwNewPixmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  {
    win_wintype result;

  /* drwNewPixmap */
    result = (win_wintype) malloc(sizeof(struct win_winstruct));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      result->hdc = CreateCompatibleDC(to_hdc(actual_window));
      result->hBitmap = CreateCompatibleBitmap(to_hdc(actual_window), width, height);
      result->oldBitmap = SelectObject(result->hdc, result->hBitmap);
      result->is_pixmap = TRUE;
      result->width = width;
      result->height = height;
      result->next = NULL;
    } /* if */
    return((wintype) result);
  } /* drwNewPixmap */



#ifdef ANSI_C

wintype drwNewBitmap (wintype actual_window, inttype width, inttype height)
#else

wintype drwNewBitmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  { /* drwNewBitmap */
    return(0);
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
    inttype width, inttype height, stritype window_name)
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
    result = NULL;
    if (init_called == 0) {
      dra_init();
    } /* if */
    if (init_called != 0) {
      win_name = cp_to_cstri(window_name);
      if (win_name == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        result = (win_wintype) malloc(sizeof(struct win_winstruct));
        if (result != NULL) {
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
          result->hWnd = CreateWindow(windowClass, win_name,
              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
              width + 2 * GetSystemMetrics(SM_CXSIZEFRAME),
              height + 2 * GetSystemMetrics(SM_CYSIZEFRAME) +
              GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER),
              (HWND) NULL, (HMENU) NULL, NULL, NULL);
          /* printf("hWnd=%lu\n", result->hWnd); */
          if (result->hWnd != NULL) {
            result->hdc = GetDC(result->hWnd);
            /* printf("hdc=%lu\n", result->hdc); */
            result->is_pixmap = FALSE;
            result->width = width;
            result->height = height;
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
    return((wintype) result);
  } /* drwOpen */



#ifdef ANSI_C

void drwPoint (wintype actual_window, inttype x, inttype y)
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

void drwPPoint (wintype actual_window, inttype x, inttype y, inttype col)
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

void drwPRect (wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y, inttype col)
#else

void drwPRect (actual_window, x1, y1, length_x, length_y, col)
wintype actual_window;
inttype x1, y1, length_x, length_y;
inttype col;
#endif

  {
    HPEN old_pen;
    HPEN current_pen;
    HBRUSH old_brush;
    HBRUSH current_brush;

  /* drwPRect */
#ifdef TRACE_WIN
    printf("drwPRect(%lu, %ld, %ld, %ld, %ld, %lx)\n", actual_window, x1, y1, length_x, length_y, col);
#endif
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
#ifdef OUT_OF_ORDER
    if (length_x == 0 && length_y == 0) {
      printf("length_x == 0 && length_y == 0\n");
    } /* if */
    if (length_x == 1 && length_y == 1) {
      printf("length_x == 1 && length_y == 1\n");
    } /* if */
#endif
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    current_brush = CreateSolidBrush((COLORREF) col);
    if (current_pen == NULL) {
      printf("drwPRect pen with color %lx is NULL\n", col);
    } /* if */
    if (current_brush == NULL) {
      printf("drwPRect brush with color %lx is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    old_brush = SelectObject(to_hdc(actual_window), current_brush);
    if (length_x == 1) {
      if (length_y == 1) {
        SetPixel(to_hdc(actual_window), x1, y1, (COLORREF) col);
      } else {
        MoveToEx(to_hdc(actual_window), x1, y1, NULL);
        LineTo(to_hdc(actual_window), x1, y1 + length_y);
      } /* if */
    } else {
      if (length_y == 1) {
        MoveToEx(to_hdc(actual_window), x1, y1, NULL);
        LineTo(to_hdc(actual_window), x1 + length_x, y1);
      } else {
        Rectangle(to_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
      } /* if */
    } /* if */
    SelectObject(to_hdc(actual_window), old_pen);
    SelectObject(to_hdc(actual_window), old_brush);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      old_brush = SelectObject(to_backup_hdc(actual_window), current_brush);
      if (length_x == 1) {
        if (length_y == 1) {
          SetPixel(to_backup_hdc(actual_window), x1, y1, (COLORREF) col);
        } else {
          MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
          LineTo(to_backup_hdc(actual_window), x1, y1 + length_y);
        } /* if */
      } else {
        if (length_y == 1) {
          MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
          LineTo(to_backup_hdc(actual_window), x1 + length_x, y1);
        } else {
          Rectangle(to_backup_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
        } /* if */
      } /* if */
      SelectObject(to_backup_hdc(actual_window), old_pen);
      SelectObject(to_backup_hdc(actual_window), old_brush);
    } /* if */
    DeleteObject(current_pen);
    DeleteObject(current_brush);
  } /* drwPRect */



#ifdef ANSI_C

void drwPut (wintype actual_window, wintype pixmap,
    inttype x1, inttype y1)
#else

void drwPut (actual_window, pixmap, x1, y1)
wintype actual_window;
wintype pixmap;
inttype x1;
inttype y1;
#endif

  { /* drwPut */
    if (pixmap != NULL) {
      BitBlt(to_hdc(actual_window), x1, y1, to_width(pixmap), to_height(pixmap),
          to_hdc(pixmap), 0, 0, SRCCOPY);
      if (to_backup_hdc(actual_window) != 0) {
        BitBlt(to_backup_hdc(actual_window), x1, y1, to_width(pixmap), to_height(pixmap),
            to_hdc(pixmap), 0, 0, SRCCOPY);
      } /* if */
    } /* if */
  } /* drwPut */



#ifdef ANSI_C

void drwRect (wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y)
#else

void drwRect (actual_window, x1, y1, length_x, length_y)
wintype actual_window;
inttype x1, y1, length_x, length_y;
#endif

  { /* drwRect */
    Rectangle(to_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
    if (to_backup_hdc(actual_window) != 0) {
      Rectangle(to_backup_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
    } /* if */
  } /* drwRect */



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
    return(RGB(((uinttype) red_val) >> 8,
               ((uinttype) green_val) >> 8,
               ((uinttype) blue_val) >> 8));
  } /* drwRgbColor */



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

void drwText (wintype actual_window, inttype x, inttype y, stritype stri,
    inttype col, inttype bkcol)
#else

void drwText (actual_window, x, y, stri, col, bkcol)
wintype actual_window;
inttype x, y;
stritype stri;
inttype col;
inttype bkcol;
#endif

  { /* drwText */
#ifdef WIDE_CHAR_STRINGS
    {
      WORD *stri_buffer;
      WORD *wstri;
      strelemtype *strelem;
      memsizetype len;

      stri_buffer = (WORD *) malloc(sizeof(WORD) * stri->size);
      if (stri_buffer != NULL) {
        wstri = stri_buffer;
        strelem = stri->mem;
        len = stri->size;
        for (; len > 0; wstri++, strelem++, len--) {
          *wstri = *strelem;
        } /* while */

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
    }
#else
    SetTextColor(to_hdc(actual_window), (COLORREF) col);
    SetBkColor(to_hdc(actual_window), (COLORREF) bkcol);
    SetTextAlign(to_hdc(actual_window), TA_BASELINE | TA_LEFT);
    TextOut(to_hdc(actual_window), x, y, stri->mem, stri->size);
    if (to_backup_hdc(actual_window) != 0) {
      SetTextColor(to_backup_hdc(actual_window), (COLORREF) col);
      SetBkColor(to_backup_hdc(actual_window), (COLORREF) bkcol);
      SetTextAlign(to_backup_hdc(actual_window), TA_BASELINE | TA_LEFT);
      TextOut(to_backup_hdc(actual_window), x, y, stri->mem, stri->size);
    } /* if */
#endif
  } /* drwText */



#ifdef ANSI_C

inttype drwWidth (wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  { /* drwWidth */
    return(to_width(actual_window));
  } /* drwWidth */
