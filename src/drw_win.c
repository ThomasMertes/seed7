/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilitys.     */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Graphic access using the windows capabilitys.          */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "setjmp.h"

#include "version.h"
#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"

#define TRACE_ENTITYS

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
  unsigned int width;
  unsigned int height;
  struct win_winstruct *next;
} *win_wintype;

static win_wintype window_list = NULL;
static win_wintype pixmap_list = NULL;
static win_wintype bitmap_list = NULL;

#define to_hwnd(win)        (((win_wintype) win)->hWnd)
#define to_hBitmap(win)     (((win_wintype) win)->hBitmap)
#define to_hdc(win)         (((win_wintype) win)->hdc)
#define to_backup_hdc(win)  (((win_wintype) win)->backup_hdc)
#define to_width(win)       (((win_wintype) win)->width)
#define to_height(win)      (((win_wintype) win)->height)

#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE 674
#endif



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



LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    PAINTSTRUCT ps;
    win_wintype paint_window;
    HDC hdc;
    WINDOWPOS *windowpos;
    POINT point;
    RECT rect;
    MSG msg;
    int peek_num;
    LRESULT result;

  /* WndProc */
    /* printf("WndProc message=%d, %lu, %d, %u\n", message, hWnd, wParam, lParam); */
    switch (message) {
      case WM_PAINT:
        paint_window = find_window(hWnd);
        printf("WM_PAINT %lu %lu\n", hWnd, paint_window);
        if (paint_window != NULL && paint_window->backup_hdc != 0) {
          printf("hdc=%lu\n", paint_window->hdc);
          if (GetUpdateRect(paint_window->hdc, &rect, FALSE) != 0) {
            printf("GetUpdateRect left=%ld, top=%ld, right=%ld, bottom=%ld\n",
                rect.left, rect.top, rect.right, rect.bottom);
            BitBlt(to_hdc(paint_window), 0, 0, to_width(paint_window), to_height(paint_window),
                to_backup_hdc(paint_window), 0, 0, SRCCOPY);
          } else {
            printf("GetUpdateRect no update region\n");
            BitBlt(to_hdc(paint_window), 0, 0, to_width(paint_window), to_height(paint_window),
                to_backup_hdc(paint_window), 0, 0, SRCCOPY);
          } /* if */
        } else {
          printf("paint_window=%lu, backup_hdc=%lu\n",
              paint_window, paint_window->backup_hdc);
        } /* if */
        break;
#ifdef OUT_OF_ORDER
      case WM_SYSKEYUP:
        return(0);
        break;
#endif
      default:
        result = DefWindowProc(hWnd, message, wParam, lParam);
        /* printf("WndProc [Peek=%d] ==> %d\n", PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE), result); */
        return(result);
        break;
    } /* switch */
    result = DefWindowProc(hWnd, message, wParam, lParam);
    /* printf("WndProc [Peek=%d] ==> %d\n", PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE), result); */
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
    result = K_NONE;
      bRet = GetMessage(&msg, NULL, 0, 0);
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
                default:          result = K_NONE;       break;
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
                default:          result = K_NONE;       break;
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
                default:          result = K_NONE;       break;
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
                default:          result = K_NONE;       break;
              } /* switch */
            } /* if */
            if (result == K_NONE) {
	      /* printf("TranslateMessage(%d) %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              TranslateMessage(&msg);
              /* printf("translated message=%d %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		/* printf("PeekMessage(%d) %lu, %d %X\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
                if (msg.message != WM_CHAR) {
                  result = K_UNDEF;
                } /* if */
              } else {
		/* printf("PeekMessage ==> empty\n"); */
                result = K_UNDEF;
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
            switch (msg.wParam) {
                case 'A':    result = K_ALT_A;   break;
                case 'B':    result = K_ALT_B;   break;
                case 'C':    result = K_ALT_C;   break;
                case 'D':    result = K_ALT_D;   break;
                case 'E':    result = K_ALT_E;   break;
                case 'F':    result = K_ALT_F;   break;
                case 'G':    result = K_ALT_G;   break;
                case 'H':    result = K_ALT_H;   break;
                case 'I':    result = K_ALT_I;   break;
                case 'J':    result = K_ALT_J;   break;
                case 'K':    result = K_ALT_K;   break;
                case 'L':    result = K_ALT_L;   break;
                case 'M':    result = K_ALT_M;   break;
                case 'N':    result = K_ALT_N;   break;
                case 'O':    result = K_ALT_O;   break;
                case 'P':    result = K_ALT_P;   break;
                case 'Q':    result = K_ALT_Q;   break;
                case 'R':    result = K_ALT_R;   break;
                case 'S':    result = K_ALT_S;   break;
                case 'T':    result = K_ALT_T;   break;
                case 'U':    result = K_ALT_U;   break;
                case 'V':    result = K_ALT_V;   break;
                case 'W':    result = K_ALT_W;   break;
                case 'X':    result = K_ALT_X;   break;
                case 'Y':    result = K_ALT_Y;   break;
                case 'Z':    result = K_ALT_Z;   break;
                case '0':    result = K_ALT_0;   break;
                case '1':    result = K_ALT_1;   break;
                case '2':    result = K_ALT_2;   break;
                case '3':    result = K_ALT_3;   break;
                case '4':    result = K_ALT_4;   break;
                case '5':    result = K_ALT_5;   break;
                case '6':    result = K_ALT_6;   break;
                case '7':    result = K_ALT_7;   break;
                case '8':    result = K_ALT_8;   break;
                case '9':    result = K_ALT_9;   break;
                case VK_F1:  result = K_ALT_F1;  break;
                case VK_F2:  result = K_ALT_F2;  break;
                case VK_F3:  result = K_ALT_F3;  break;
                case VK_F4:  result = K_ALT_F4;  break;
                case VK_F5:  result = K_ALT_F5;  break;
                case VK_F6:  result = K_ALT_F6;  break;
                case VK_F7:  result = K_ALT_F7;  break;
                case VK_F8:  result = K_ALT_F8;  break;
                case VK_F9:  result = K_ALT_F9;  break;
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
            } /* switch */
          } else if (msg.message == WM_CHAR) {
            /* printf("WM_CHAR %lu, %d, %u\n", msg.hwnd, msg.wParam, msg.lParam); */
            result = msg.wParam;
          } else {
            /* printf("message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            DispatchMessage(&msg);
          } /* if */
        } /* if */
        if (result == K_NONE) {
          bRet = GetMessage(&msg, NULL, 0, 0);
        } /* if */
      } /* while */
    /* printf("getc() ==> %d\n", result); */
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
          printf("gkbKeyPressed WM_NCLBUTTONDOWN\n");
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            printf("x GetMessage(&msg, NULL, 0, 0)=0\n");
          } else if (bRet == -1) {
            printf("x GetMessage(&msg, NULL, 0, 0)=-1\n");
          } else {
            printf("x message=%d %lu, %d, %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            printf("x translated message=%d %lu, %d %u\n", msg.message, msg.hwnd, msg.wParam, msg.lParam);
            DispatchMessage(&msg);
            printf("x after DispatchMessage\n");
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

  { /* drwArc */
    AngleArc(to_hdc(actual_window), x, y, (unsigned) radius, ang1, ang2);
  } /* drwArc */



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

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
  } /* drwFlush */



#ifdef ANSI_C

void drwFree (wintype pixmap)
#else

void drwFree (pixmap)
wintype pixmap;
#endif

  { /* drwFree */
    DeleteObject(to_hBitmap(pixmap));
    DeleteDC(to_hdc(pixmap));
    free((win_wintype) pixmap);
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
    if (result != NULL) {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      result->hdc = CreateCompatibleDC(to_hdc(actual_window));
      result->hBitmap = CreateCompatibleBitmap(to_hdc(actual_window), width, height);
      SelectObject(result->hdc, result->hBitmap);
      result->width = width;
      result->height = height;
      BitBlt(result->hdc, 0, 0, width, height,
          to_backup_hdc(actual_window), left, upper, SRCCOPY);
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
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
      printf("drwPLine pen with color %ul is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    MoveToEx(to_hdc(actual_window), x1, y1, NULL);
    LineTo(to_hdc(actual_window), x2, y2);
    SelectObject(to_hdc(actual_window), old_pen);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
      LineTo(to_backup_hdc(actual_window), x2, y2);
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
    if (result != NULL) {
      memset(result, 0, sizeof(struct win_winstruct));
      result->usage_count = 1;
      result->hdc = CreateCompatibleDC(to_hdc(actual_window));
      result->hBitmap = CreateCompatibleBitmap(to_hdc(actual_window), width, height);
      SelectObject(result->hdc, result->hBitmap);
      result->width = width;
      result->height = height;
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
    win_wintype result;

  /* drwOpen */
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
            result->width = width;
            result->height = height;
            result->backup_hdc = CreateCompatibleDC(result->hdc);
            result->backup = CreateCompatibleBitmap(result->hdc, width, height);
            SelectObject(result->backup_hdc, result->backup);
            ShowWindow(result->hWnd, SW_SHOWDEFAULT);
            UpdateWindow(result->hWnd);
          } /* if */
        } /* if */
        free_cstri(win_name, window_name);
      } /* if */
    } /* if */
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

  {
    HPEN old_pen;
    HPEN current_pen;

  /* drwPPoint */
    /* SetDCPenColor(to_hdc(actual_window), (COLORREF) col); */
    current_pen = CreatePen(PS_SOLID, 1, (COLORREF) col);
    if (current_pen == NULL) {
      printf("drwPPoint pen with color %ul is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    MoveToEx(to_hdc(actual_window), x, y, NULL);
    LineTo(to_hdc(actual_window), x + 1, y + 1);
    SelectObject(to_hdc(actual_window), old_pen);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      MoveToEx(to_backup_hdc(actual_window), x, y, NULL);
      LineTo(to_backup_hdc(actual_window), x + 1, y + 1);
      SelectObject(to_backup_hdc(actual_window), old_pen);
    } /* if */
    DeleteObject(current_pen);
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
      printf("drwPRect pen with color %ul is NULL\n", col);
    } /* if */
    if (current_brush == NULL) {
      printf("drwPRect brush with color %ul is NULL\n", col);
    } /* if */
    old_pen = SelectObject(to_hdc(actual_window), current_pen);
    old_brush = SelectObject(to_hdc(actual_window), current_brush);
    if (length_x == 1 || length_y == 1) {
      MoveToEx(to_hdc(actual_window), x1, y1, NULL);
      LineTo(to_hdc(actual_window), x1 + length_x, y1 + length_y);
    } else {
      Rectangle(to_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
    } /* if */
    SelectObject(to_hdc(actual_window), old_pen);
    SelectObject(to_hdc(actual_window), old_brush);
    if (to_backup_hdc(actual_window) != 0) {
      old_pen = SelectObject(to_backup_hdc(actual_window), current_pen);
      old_brush = SelectObject(to_backup_hdc(actual_window), current_brush);
      if (length_x == 1 || length_y == 1) {
        MoveToEx(to_backup_hdc(actual_window), x1, y1, NULL);
        LineTo(to_backup_hdc(actual_window), x1 + length_x, y1 + length_y);
      } else {
        Rectangle(to_backup_hdc(actual_window), x1, y1, x1 + length_x, y1 + length_y);
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

void drwText (wintype actual_window, inttype x, inttype y, stritype stri)
#else

void drwText (actual_window, x, y, stri)
wintype actual_window;
inttype x, y;
stritype stri;
#endif

  { /* drwText */
#ifdef WIDE_CHAR_STRINGS
    {
      cstritype cstri;

      cstri = cp_to_cstri(stri);
      if (cstri != NULL) {
        TextOut(to_hdc(actual_window), x, y, cstri, strlen(cstri));
        if (to_backup_hdc(actual_window) != 0) {
          TextOut(to_backup_hdc(actual_window), x, y, cstri, strlen(cstri));
        } /* if */
        free_cstri(cstri, stri);
      } /* if */
    }
#else
    TextOut(to_hdc(actual_window), x, y, stri->mem, stri->size);
    if (to_backup_hdc(actual_window) != 0) {
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
