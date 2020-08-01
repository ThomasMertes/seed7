/********************************************************************/
/*                                                                  */
/*  gkb_win.c     Keyboard and mouse access for windows.            */
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
/*  File: seed7/src/gkb_x11.c                                       */
/*  Changes: 2005 - 2007, 2013  Thomas Mertes                       */
/*  Content: Keyboard and mouse access for windows.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"

#include "common.h"
#include "data_rtl.h"
#include "hsh_rtl.h"
#include "kbd_drv.h"


#define TRACE_EVENTS 0
#if TRACE_EVENTS
#define traceEvent(traceStatements) traceStatements
#else
#define traceEvent(traceStatements)
#endif

static intType button_x = 0;
static intType button_y = 0;
static HWND button_window = 0;
static rtlHashType window_hash = NULL;

static const charType map_1252_to_unicode[] = {
/* 128 */ 0x20AC,    '?', 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
/* 136 */ 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152,    '?', 0x017D,    '?',
/* 144 */    '?', 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
/* 152 */ 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153,    '?', 0x017E, 0x0178};

#ifdef DMC_GKB_WIN_DEFINES
#define WM_MOUSEWHEEL                   0x020A
#endif

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#endif

#if defined(_WIN64)
typedef int64Type intPtrType;
#else
typedef int32Type intPtrType;
#endif



winType find_window (HWND sys_window)

  {
    winType window;

  /* find_window */
    if (window_hash == NULL) {
      window = NULL;
    } else {
      window = (winType) (memSizeType)
          hshIdxDefault0(window_hash,
                         (genericType) (memSizeType) sys_window,
                         (intType) ((memSizeType) sys_window) >> 6,
                         (compareType) &genericCmp);
    } /* if */
    logFunction(printf("find_window(" FMT_X_MEM ") --> " FMT_X_MEM "\n",
                       (memSizeType) sys_window, (memSizeType) window););
    return window;
  } /* find_window */



void enter_window (winType curr_window, HWND sys_window)

  { /* enter_window */
    /* printf("enter_window(%lx, %lx)\n", (unsigned long) curr_window, (unsigned long) sys_window); */
    if (window_hash == NULL) {
      window_hash = hshEmpty();
    } /* if */
    (void) hshIdxEnterDefault(window_hash,
                              (genericType) (memSizeType) sys_window,
                              (genericType) (memSizeType) curr_window,
                              (intType) ((memSizeType) sys_window) >> 6);
  } /* enter_window */



void remove_window (HWND sys_window)

  { /* remove_window */
    /* printf("remove_window(%lx)\n", (unsigned long) sys_window); */
    hshExcl(window_hash,
            (genericType) (memSizeType) sys_window,
            (intType) ((memSizeType) sys_window) >> 6,
            (compareType) &genericCmp,
            (destrFuncType) &genericDestr,
            (destrFuncType) &genericDestr);
  } /* remove_window */



charType gkbGetc (void)

  {
    BOOL bRet;
    MSG msg;
    charType result = K_NONE;

  /* gkbGetc */
    logFunction(printf("gkbGetc\n"););
    /* printf("before GetMessage\n"); */
    bRet = GetMessage(&msg, NULL, 0, 0);
    /* printf("after GetMessage\n"); */
    while (result == K_NONE && bRet != 0) {
      if (bRet == -1) {
        logError(printf("GetMessage(&msg, NULL, 0, 0)=-1\n"););
      } else {
        /* printf("gkbGetc message=%d %lu, %d, %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
        if (msg.message == WM_KEYDOWN) {
          traceEvent(printf("WM_KEYDOWN hwnd=%lu, msg.wParam=%d, lParam=%x, "
                            "SHIFT=%hx, CONTROL=%hx, MENU=%hx\n",
                            msg.hwnd, msg.wParam, msg.lParam, GetKeyState(VK_SHIFT),
                            GetKeyState(VK_CONTROL), GetKeyState(VK_MENU)););
          if (GetKeyState(VK_SHIFT) & 0x8000) {
            /* printf("VK_SHIFT\n"); */
            if (msg.lParam & 0x1000000) {
              /* The key is a cursor key but not from the numeric keypad. */
              switch (msg.wParam) {
                case VK_RETURN:   result = K_SFT_NL;     break;
                case VK_BACK:     result = K_SFT_BS;     break;
                case VK_TAB:      result = K_BACKTAB;    break;
                case VK_ESCAPE:   result = K_SFT_ESC;    break;
                case VK_LEFT:     result = K_LEFT;       break;
                case VK_RIGHT:    result = K_RIGHT;      break;
                case VK_UP:       result = K_UP;         break;
                case VK_DOWN:     result = K_DOWN;       break;
                case VK_HOME:     result = K_HOME;       break;
                case VK_END:      result = K_END;        break;
                case VK_PRIOR:    result = K_PGUP;       break;
                case VK_NEXT:     result = K_PGDN;       break;
                case VK_CLEAR:    result = K_PAD_CENTER; break;
                case VK_INSERT:   result = K_INS;        break;
                case VK_DELETE:   result = K_DEL;        break;
                case VK_APPS:     result = K_SFT_MENU;   break;
                case VK_PRINT:    result = K_SFT_PRINT;  break;
                case VK_PAUSE:    result = K_SFT_PAUSE;  break;
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
                case VK_RETURN:   result = K_SFT_NL;     break;
                case VK_BACK:     result = K_SFT_BS;     break;
                case VK_TAB:      result = K_BACKTAB;    break;
                case VK_ESCAPE:   result = K_SFT_ESC;    break;
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
                case VK_F11:      result = K_SFT_F11;    break;
                case VK_F12:      result = K_SFT_F12;    break;
                case VK_LEFT:     result = '4';          break;
                case VK_RIGHT:    result = '6';          break;
                case VK_UP:       result = '8';          break;
                case VK_DOWN:     result = '2';          break;
                case VK_HOME:     result = '7';          break;
                case VK_END:      result = '1';          break;
                case VK_PRIOR:    result = '9';          break;
                case VK_NEXT:     result = '3';          break;
                case VK_CLEAR:    result = '5';          break;
                case VK_INSERT:   result = '0';          break;
                case VK_DELETE:   result = K_DEL;        break;
                case VK_APPS:     result = K_SFT_MENU;   break;
                case VK_PRINT:    result = K_SFT_PRINT;  break;
                case VK_PAUSE:    result = K_SFT_PAUSE;  break;
                case VK_SHIFT:
                case VK_CONTROL:
                case VK_MENU:
                case VK_CAPITAL:
                case VK_NUMLOCK:
                case VK_SCROLL:   result = K_NONE;       break;
                default:          result = K_UNDEF;      break;
              } /* switch */
            } /* if */
          } else if (((GetKeyState(VK_CONTROL)  & 0x8000) != 0 &&
                      (GetKeyState(VK_RMENU)    & 0x8000) == 0) ||
                      (GetKeyState(VK_RCONTROL) & 0x8000) != 0) {
            /* This condition is complicated because Alt Gr is     */
            /* encoded as if left-control + right-alt are pressed. */
            /* This situation is filtered out with this condition. */
            /* printf("VK_CONTROL\n"); */
            switch (msg.wParam) {
              case VK_RETURN:   result = K_CTL_NL;         break;
              case VK_BACK:     result = K_CTL_BS;         break;
              case VK_TAB:      result = K_CTL_TAB;        break;
              case VK_ESCAPE:   result = K_CTL_ESC;        break;
              case VK_F1:       result = K_CTL_F1;         break;
              case VK_F2:       result = K_CTL_F2;         break;
              case VK_F3:       result = K_CTL_F3;         break;
              case VK_F4:       result = K_CTL_F4;         break;
              case VK_F5:       result = K_CTL_F5;         break;
              case VK_F6:       result = K_CTL_F6;         break;
              case VK_F7:       result = K_CTL_F7;         break;
              case VK_F8:       result = K_CTL_F8;         break;
              case VK_F9:       result = K_CTL_F9;         break;
              case VK_F10:      result = K_CTL_F10;        break;
              case VK_F11:      result = K_CTL_F11;        break;
              case VK_F12:      result = K_CTL_F12;        break;
              case VK_NUMPAD0:  result = K_CTL_INS;        break;
              case VK_NUMPAD1:  result = K_CTL_END;        break;
              case VK_NUMPAD2:  result = K_CTL_DOWN;       break;
              case VK_NUMPAD3:  result = K_CTL_PGDN;       break;
              case VK_NUMPAD4:  result = K_CTL_LEFT;       break;
              case VK_NUMPAD5:  result = K_CTL_PAD_CENTER; break;
              case VK_NUMPAD6:  result = K_CTL_RIGHT;      break;
              case VK_NUMPAD7:  result = K_CTL_HOME;       break;
              case VK_NUMPAD8:  result = K_CTL_UP;         break;
              case VK_NUMPAD9:  result = K_CTL_PGUP;       break;
              case VK_DECIMAL:  result = K_CTL_DEL;        break;
              case VK_LEFT:     result = K_CTL_LEFT;       break;
              case VK_RIGHT:    result = K_CTL_RIGHT;      break;
              case VK_UP:       result = K_CTL_UP;         break;
              case VK_DOWN:     result = K_CTL_DOWN;       break;
              case VK_HOME:     result = K_CTL_HOME;       break;
              case VK_END:      result = K_CTL_END;        break;
              case VK_PRIOR:    result = K_CTL_PGUP;       break;
              case VK_NEXT:     result = K_CTL_PGDN;       break;
              case VK_CLEAR:    result = K_CTL_PAD_CENTER; break;
              case VK_INSERT:   result = K_CTL_INS;        break;
              case VK_DELETE:   result = K_CTL_DEL;        break;
              case VK_APPS:     result = K_CTL_MENU;       break;
              case VK_PRINT:    result = K_CTL_PRINT;      break;
              case VK_PAUSE:    result = K_CTL_PAUSE;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;           break;
              default:          result = K_UNDEF;          break;
            } /* switch */
          } else if (GetKeyState(VK_MENU) & 0x8000) {
            /* This condition is true for the Alt key and the Alt Gr key. */
            /* printf("VK_MENU\n"); */
            switch (msg.wParam) {
              case VK_RETURN:   result = K_ALT_NL;         break;
              case VK_BACK:     result = K_ALT_BS;         break;
              case VK_TAB:      result = K_ALT_TAB;        break;
              case VK_ESCAPE:   result = K_ALT_ESC;        break;
              case VK_F1:       result = K_ALT_F1;         break;
              case VK_F2:       result = K_ALT_F2;         break;
              case VK_F3:       result = K_ALT_F3;         break;
              case VK_F4:       result = K_ALT_F4;         break;
              case VK_F5:       result = K_ALT_F5;         break;
              case VK_F6:       result = K_ALT_F6;         break;
              case VK_F7:       result = K_ALT_F7;         break;
              case VK_F8:       result = K_ALT_F8;         break;
              case VK_F9:       result = K_ALT_F9;         break;
              case VK_F10:      result = K_ALT_F10;        break;
              case VK_F11:      result = K_ALT_F11;        break;
              case VK_F12:      result = K_ALT_F12;        break;
              case VK_NUMPAD0:  result = K_ALT_INS;        break;
              case VK_NUMPAD1:  result = K_ALT_END;        break;
              case VK_NUMPAD2:  result = K_ALT_DOWN;       break;
              case VK_NUMPAD3:  result = K_ALT_PGDN;       break;
              case VK_NUMPAD4:  result = K_ALT_LEFT;       break;
              case VK_NUMPAD5:  result = K_ALT_PAD_CENTER; break;
              case VK_NUMPAD6:  result = K_ALT_RIGHT;      break;
              case VK_NUMPAD7:  result = K_ALT_HOME;       break;
              case VK_NUMPAD8:  result = K_ALT_UP;         break;
              case VK_NUMPAD9:  result = K_ALT_PGUP;       break;
              case VK_DECIMAL:  result = K_ALT_DEL;        break;
              case VK_LEFT:     result = K_ALT_LEFT;       break;
              case VK_RIGHT:    result = K_ALT_RIGHT;      break;
              case VK_UP:       result = K_ALT_UP;         break;
              case VK_DOWN:     result = K_ALT_DOWN;       break;
              case VK_HOME:     result = K_ALT_HOME;       break;
              case VK_END:      result = K_ALT_END;        break;
              case VK_PRIOR:    result = K_ALT_PGUP;       break;
              case VK_NEXT:     result = K_ALT_PGDN;       break;
              case VK_CLEAR:    result = K_ALT_PAD_CENTER; break;
              case VK_INSERT:   result = K_ALT_INS;        break;
              case VK_DELETE:   result = K_ALT_DEL;        break;
              case VK_APPS:     result = K_ALT_MENU;       break;
              case VK_PRINT:    result = K_ALT_PRINT;      break;
              case VK_PAUSE:    result = K_ALT_PAUSE;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;           break;
              default:          result = K_UNDEF;          break;
            } /* switch */
          } else {
            /* printf("no key state\n"); */
            switch (msg.wParam) {
              case VK_RETURN:   result = K_NL;         break;
              case VK_ESCAPE:   result = K_ESC;        break;
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
              case VK_F11:      result = K_F11;        break;
              case VK_F12:      result = K_F12;        break;
              case VK_LEFT:     result = K_LEFT;       break;
              case VK_RIGHT:    result = K_RIGHT;      break;
              case VK_UP:       result = K_UP;         break;
              case VK_DOWN:     result = K_DOWN;       break;
              case VK_HOME:     result = K_HOME;       break;
              case VK_END:      result = K_END;        break;
              case VK_PRIOR:    result = K_PGUP;       break;
              case VK_NEXT:     result = K_PGDN;       break;
              case VK_CLEAR:    result = K_PAD_CENTER; break;
              case VK_INSERT:   result = K_INS;        break;
              case VK_DELETE:   result = K_DEL;        break;
              case VK_APPS:     result = K_MENU;       break;
              case VK_PRINT:    result = K_PRINT;      break;
              case VK_PAUSE:    result = K_PAUSE;      break;
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
            /* printf("TranslateMessage(%d) %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("translated message=%d %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
              /* printf("PeekMessage(%d) %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              if (msg.message == WM_CHAR) {
                result = K_NONE;
              } /* if */
            } else {
              /* printf("PeekMessage --> empty\n"); */
            } /* if */
          } /* if */
        } else if (msg.message == WM_LBUTTONDOWN) {
          traceEvent(printf("WM_LBUTTONDOWN hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          if (GetKeyState(VK_MENU) & 0x8000) {
            result = K_ALT_MOUSE1;
          } else if (msg.wParam & 0x04) {
            result = K_SFT_MOUSE1;
          } else if (msg.wParam & 0x08) {
            result = K_CTL_MOUSE1;
          } else {
            result = K_MOUSE1;
          } /* if */
        } else if (msg.message == WM_MBUTTONDOWN) {
          traceEvent(printf("WM_MBUTTONDOWN hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          if (GetKeyState(VK_MENU) & 0x8000) {
            result = K_ALT_MOUSE2;
          } else if (msg.wParam & 0x04) {
            result = K_SFT_MOUSE2;
          } else if (msg.wParam & 0x08) {
            result = K_CTL_MOUSE2;
          } else {
            result = K_MOUSE2;
          } /* if */
        } else if (msg.message == WM_RBUTTONDOWN) {
          traceEvent(printf("WM_RBUTTONDOWN hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          if (GetKeyState(VK_MENU) & 0x8000) {
            result = K_ALT_MOUSE3;
          } else if (msg.wParam & 0x04) {
            result = K_SFT_MOUSE3;
          } else if (msg.wParam & 0x08) {
            result = K_CTL_MOUSE3;
          } else {
            result = K_MOUSE3;
          } /* if */
        } else if (msg.message == WM_MOUSEWHEEL) {
          traceEvent(printf("WM_MOUSEWHEEL hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          if ((intPtrType) msg.wParam > 0) {
            if (GetKeyState(VK_MENU) & 0x8000) {
              result = K_ALT_MOUSE4;
            } else if (msg.wParam & 0x04) {
              result = K_SFT_MOUSE4;
            } else if (msg.wParam & 0x08) {
              result = K_CTL_MOUSE4;
            } else {
              result = K_MOUSE4;
            } /* if */
          } else {
            if (GetKeyState(VK_MENU) & 0x8000) {
              result = K_ALT_MOUSE5;
            } else if (msg.wParam & 0x04) {
              result = K_SFT_MOUSE5;
            } else if (msg.wParam & 0x08) {
              result = K_CTL_MOUSE5;
            } else {
              result = K_MOUSE5;
            } /* if */
          } /* if */
        } else if (msg.message == WM_XBUTTONDOWN) {
          traceEvent(printf("WM_XBUTTONDOWN hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          button_x = LOWORD(msg.lParam);
          button_y = HIWORD(msg.lParam);
          button_window = msg.hwnd;
          if ((msg.wParam & 0xffff0) == 0x20040) {
            if (GetKeyState(VK_MENU) & 0x8000) {
              result = K_ALT_MOUSE_FWD;
            } else if (msg.wParam & 0x04) {
              result = K_SFT_MOUSE_FWD;
            } else if (msg.wParam & 0x08) {
              result = K_CTL_MOUSE_FWD;
            } else {
              result = K_MOUSE_FWD;
            } /* if */
          } else if ((msg.wParam & 0xffff0) == 0x10020) {
            if (GetKeyState(VK_MENU) & 0x8000) {
              result = K_ALT_MOUSE_BACK;
            } else if (msg.wParam & 0x04) {
              result = K_SFT_MOUSE_BACK;
            } else if (msg.wParam & 0x08) {
              result = K_CTL_MOUSE_BACK;
            } else {
              result = K_MOUSE_BACK;
            } /* if */
          } else {
            result = K_UNDEF;
          } /* if */
        } else if (msg.message == WM_SYSKEYDOWN) {
          traceEvent(printf("WM_SYSKEYDOWN hwnd=%lu, msg.wParam=%d, lParam=%x, "
                            "SHIFT=%hx, CONTROL=%hx, MENU=%hx\n",
                            msg.hwnd, msg.wParam, msg.lParam, GetKeyState(VK_SHIFT),
                            GetKeyState(VK_CONTROL), GetKeyState(VK_MENU)););
          if (msg.lParam & 0x1000000) {
            /* The key is a cursor key but not from the numeric keypad. */
            switch (msg.wParam) {
              case VK_RETURN:   result = K_ALT_NL;         break;
              case VK_BACK:     result = K_ALT_BS;         break;
              case VK_TAB:      result = K_ALT_TAB;        break;
              case VK_ESCAPE:   result = K_ALT_ESC;        break;
              case VK_LEFT:     result = K_ALT_LEFT;       break;
              case VK_RIGHT:    result = K_ALT_RIGHT;      break;
              case VK_UP:       result = K_ALT_UP;         break;
              case VK_DOWN:     result = K_ALT_DOWN;       break;
              case VK_HOME:     result = K_ALT_HOME;       break;
              case VK_END:      result = K_ALT_END;        break;
              case VK_PRIOR:    result = K_ALT_PGUP;       break;
              case VK_NEXT:     result = K_ALT_PGDN;       break;
              case VK_CLEAR:    result = K_ALT_PAD_CENTER; break;
              case VK_INSERT:   result = K_ALT_INS;        break;
              case VK_DELETE:   result = K_ALT_DEL;        break;
              case VK_APPS:     result = K_ALT_MENU;       break;
              case VK_PRINT:    result = K_ALT_PRINT;      break;
              case VK_PAUSE:    result = K_ALT_PAUSE;      break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;        break;
              default:
                /* printf("WM_SYSKEYDOWN %lu, %d %x\n", msg.hwnd, msg.wParam, msg.lParam); */
                result = K_UNDEF;
                break;
            } /* switch */
          } else {
            switch (msg.wParam) {
              case 'A':         result = K_ALT_A;          break;
              case 'B':         result = K_ALT_B;          break;
              case 'C':         result = K_ALT_C;          break;
              case 'D':         result = K_ALT_D;          break;
              case 'E':         result = K_ALT_E;          break;
              case 'F':         result = K_ALT_F;          break;
              case 'G':         result = K_ALT_G;          break;
              case 'H':         result = K_ALT_H;          break;
              case 'I':         result = K_ALT_I;          break;
              case 'J':         result = K_ALT_J;          break;
              case 'K':         result = K_ALT_K;          break;
              case 'L':         result = K_ALT_L;          break;
              case 'M':         result = K_ALT_M;          break;
              case 'N':         result = K_ALT_N;          break;
              case 'O':         result = K_ALT_O;          break;
              case 'P':         result = K_ALT_P;          break;
              case 'Q':         result = K_ALT_Q;          break;
              case 'R':         result = K_ALT_R;          break;
              case 'S':         result = K_ALT_S;          break;
              case 'T':         result = K_ALT_T;          break;
              case 'U':         result = K_ALT_U;          break;
              case 'V':         result = K_ALT_V;          break;
              case 'W':         result = K_ALT_W;          break;
              case 'X':         result = K_ALT_X;          break;
              case 'Y':         result = K_ALT_Y;          break;
              case 'Z':         result = K_ALT_Z;          break;
              case '0':         result = K_ALT_0;          break;
              case '1':         result = K_ALT_1;          break;
              case '2':         result = K_ALT_2;          break;
              case '3':         result = K_ALT_3;          break;
              case '4':         result = K_ALT_4;          break;
              case '5':         result = K_ALT_5;          break;
              case '6':         result = K_ALT_6;          break;
              case '7':         result = K_ALT_7;          break;
              case '8':         result = K_ALT_8;          break;
              case '9':         result = K_ALT_9;          break;
              case VK_RETURN:   result = K_ALT_NL;         break;
              case VK_BACK:     result = K_ALT_BS;         break;
              case VK_TAB:      result = K_ALT_TAB;        break;
              case VK_ESCAPE:   result = K_ALT_ESC;        break;
              case VK_NUMPAD0:  result = K_ALT_INS;        break;
              case VK_NUMPAD1:  result = K_ALT_END;        break;
              case VK_NUMPAD2:  result = K_ALT_DOWN;       break;
              case VK_NUMPAD3:  result = K_ALT_PGDN;       break;
              case VK_NUMPAD4:  result = K_ALT_LEFT;       break;
              case VK_NUMPAD5:  result = K_ALT_PAD_CENTER; break;
              case VK_NUMPAD6:  result = K_ALT_RIGHT;      break;
              case VK_NUMPAD7:  result = K_ALT_HOME;       break;
              case VK_NUMPAD8:  result = K_ALT_UP;         break;
              case VK_NUMPAD9:  result = K_ALT_PGUP;       break;
              case VK_DECIMAL:  result = K_ALT_DEL;        break;
              case VK_LEFT:     result = K_ALT_LEFT;       break;
              case VK_RIGHT:    result = K_ALT_RIGHT;      break;
              case VK_UP:       result = K_ALT_UP;         break;
              case VK_DOWN:     result = K_ALT_DOWN;       break;
              case VK_HOME:     result = K_ALT_HOME;       break;
              case VK_END:      result = K_ALT_END;        break;
              case VK_PRIOR:    result = K_ALT_PGUP;       break;
              case VK_NEXT:     result = K_ALT_PGDN;       break;
              case VK_CLEAR:    result = K_ALT_PAD_CENTER; break;
              case VK_INSERT:   result = K_ALT_INS;        break;
              case VK_DELETE:   result = K_ALT_DEL;        break;
              case VK_APPS:     result = K_ALT_MENU;       break;
              case VK_PRINT:    result = K_ALT_PRINT;      break;
              case VK_PAUSE:    result = K_ALT_PAUSE;      break;
              case VK_F1:       result = K_ALT_F1;         break;
              case VK_F2:       result = K_ALT_F2;         break;
              case VK_F3:       result = K_ALT_F3;         break;
              case VK_F4:       result = K_ALT_F4;         break;
              case VK_F5:       result = K_ALT_F5;         break;
              case VK_F6:       result = K_ALT_F6;         break;
              case VK_F7:       result = K_ALT_F7;         break;
              case VK_F8:       result = K_ALT_F8;         break;
              case VK_F9:       result = K_ALT_F9;         break;
              case VK_F10:
                if (msg.lParam & 0x20000000) {
                  result = K_ALT_F10;
                } else if (GetKeyState(VK_SHIFT) & 0x8000) {
                  result = K_SFT_F10;
                } else if (GetKeyState(VK_CONTROL) & 0x8000) {
                  result = K_CTL_F10;
                } else {
                  result = K_F10;
                } /* if */
                break;
              case VK_F11:      result = K_ALT_F11;        break;
              case VK_F12:      result = K_ALT_F12;        break;
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:   result = K_NONE;           break;
              default:
                /* printf("WM_SYSKEYDOWN %lu, %d %x\n", msg.hwnd, msg.wParam, msg.lParam); */
                result = K_UNDEF;
                break;
            } /* switch */
          } /* if */
        } else if (msg.message == WM_NCLBUTTONDOWN) {
          traceEvent(printf("WM_NCLBUTTONDOWN hwnd=%lu, msg.wParam=%d, lParam=%x, "
                            "SHIFT=%hx, CONTROL=%hx, MENU=%hx\n",
                            msg.hwnd, msg.wParam, msg.lParam, GetKeyState(VK_SHIFT),
                            GetKeyState(VK_CONTROL), GetKeyState(VK_MENU)););
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          if (msg.wParam == HTCLOSE && !IsWindow(msg.hwnd)) {
            /* printf("HTCLOSE\n"); */
            exit(1);
          } /* if */
        } else if (msg.message == WM_CHAR) {
          traceEvent(printf("WM_CHAR hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.hwnd, msg.wParam, msg.lParam););
          result = (charType) msg.wParam;
          if (result >= 128 && result <= 159) {
            result = map_1252_to_unicode[result - 128];
          } /* if */
        } else {
          traceEvent(printf("message=%d, hwnd=%lu, msg.wParam=%d, lParam=%x\n",
                            msg.message, msg.hwnd, msg.wParam, msg.lParam););
          /* E.g.: WM_NCMOUSELEAVE */
          TranslateMessage(&msg);
          /* printf("translated message=%d %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
          DispatchMessage(&msg);
        } /* if */
      } /* if */
      if (result == K_NONE) {
        /* printf("before GetMessage\n"); */
        bRet = GetMessage(&msg, NULL, 0, 0);
        /* printf("after GetMessage\n"); */
      } /* if */
    } /* while */
    logFunction(printf("gkbGetc --> %d\n", result););
    return result;
  } /* gkbGetc */



boolType gkbKeyPressed (void)

  {
    BOOL msg_present;
    BOOL bRet;
    MSG msg;
    boolType result;

  /* gkbKeyPressed */
    logFunction(printf("gkbKeyPressed\n"););
    result = FALSE;
    msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    while (msg_present) {
      /* printf("gkbKeyPressed message=%d %lu, %d, %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
      switch (msg.message) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          if (msg.wParam == VK_SHIFT   || msg.wParam == VK_CONTROL ||
              msg.wParam == VK_MENU    || msg.wParam == VK_CAPITAL ||
              msg.wParam == VK_NUMLOCK || msg.wParam == VK_SCROLL) {
            bRet = GetMessage(&msg, NULL, 0, 0);
            if (bRet == 0) {
              logError(printf("GetMessage(&msg, NULL, 0, 0)=0\n"););
            } else if (bRet == -1) {
              logError(printf("GetMessage(&msg, NULL, 0, 0)=-1\n"););
            } else {
              /* printf("message=%d %lu, %d, %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
              TranslateMessage(&msg);
              /* printf("translated message=%d %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
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
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
          /* printf("gkbKeyPressed --> TRUE for message %d\n", msg.message); */
          msg_present = 0;
          result = TRUE;
          break;
        case WM_NCLBUTTONDOWN:
          /* printf("gkbKeyPressed WM_NCLBUTTONDOWN\n"); */
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            logError(printf("x GetMessage(&msg, NULL, 0, 0)=0\n"););
          } else if (bRet == -1) {
            logError(printf("x GetMessage(&msg, NULL, 0, 0)=-1\n"););
          } else {
            /* printf("x message=%d %lu, %d, %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("x translated message=%d %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
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
            logError(printf("y GetMessage(&msg, NULL, 0, 0)=0\n"););
          } else if (bRet == -1) {
            logError(printf("y GetMessage(&msg, NULL, 0, 0)=-1\n"););
          } /* if */
          msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          break;
        default:
          bRet = GetMessage(&msg, NULL, 0, 0);
          if (bRet == 0) {
            logError(printf("GetMessage(&msg, NULL, 0, 0)=0\n"););
          } else if (bRet == -1) {
            logError(printf("GetMessage(&msg, NULL, 0, 0)=-1\n"););
          } else {
            /* printf("message=%d %lu, %d, %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            TranslateMessage(&msg);
            /* printf("translated message=%d %lu, %d %x\n", msg.message, msg.hwnd, msg.wParam, msg.lParam); */
            DispatchMessage(&msg);
            /* printf("after DispatchMessage\n"); */
          } /* if */
          msg_present = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
          break;
      } /* switch */
    } /* while */
    logFunction(printf("gkbKeyPressed --> %d\n", result););
    return result;
  } /* gkbKeyPressed */



boolType gkbButtonPressed (charType button)

  {
    int vkey1 = 0;
    int vkey2 = 0;
    int not_vkey = 0;
    int state_vkey = 0;
    boolType okay = TRUE;
    boolType result;

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
      case K_F11: case K_SFT_F11: case K_CTL_F11: case K_ALT_F11: vkey1 = VK_F11; break;
      case K_F12: case K_SFT_F12: case K_CTL_F12: case K_ALT_F12: vkey1 = VK_F12; break;

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
      case K_PAD_CENTER: case K_CTL_PAD_CENTER: vkey1 = VK_CLEAR;  break;

      case K_NL:
        vkey1 = VK_RETURN; vkey2 = 'J'; break;
      case K_SFT_NL:  case K_CTL_NL:  case K_ALT_NL:
        vkey1 = VK_RETURN; break;
      case K_BS:
        vkey1 = VK_BACK;   vkey2 = 'H'; break;
      case K_SFT_BS:  case K_CTL_BS:  case K_ALT_BS:
        vkey1 = VK_BACK;   break;
      case K_TAB:
        vkey1 = VK_TAB;     vkey2 = 'I'; break;
      case K_BACKTAB: case K_CTL_TAB: case K_ALT_TAB:
        vkey1 = VK_TAB;    break;
      case K_ESC: case K_SFT_ESC: case K_CTL_ESC: case K_ALT_ESC:
        vkey1 = VK_ESCAPE; break;

      case K_MENU:   case K_SFT_MENU:   case K_CTL_MENU:   case K_ALT_MENU:
        vkey1 = VK_APPS;   break;
      case K_PRINT:  case K_SFT_PRINT:  case K_CTL_PRINT:  case K_ALT_PRINT:
        vkey1 = VK_PRINT;  break;
      case K_PAUSE:  case K_SFT_PAUSE:  case K_CTL_PAUSE:  case K_ALT_PAUSE:
        vkey1 = VK_PAUSE;  break;

      case ' ': vkey1 = VK_SPACE;    break;
      case '*': vkey1 = VK_MULTIPLY; break;
      case '+': vkey1 = VK_ADD;      break;
      case '-': vkey1 = VK_SUBTRACT; break;
      case '/': vkey1 = VK_DIVIDE;   break;

      case K_MOUSE1: case K_SFT_MOUSE1: case K_CTL_MOUSE1: case K_ALT_MOUSE1: vkey1 = VK_LBUTTON; break;
      case K_MOUSE2: case K_SFT_MOUSE2: case K_CTL_MOUSE2: case K_ALT_MOUSE2: vkey1 = VK_MBUTTON; break;
      case K_MOUSE3: case K_SFT_MOUSE3: case K_CTL_MOUSE3: case K_ALT_MOUSE3: vkey1 = VK_RBUTTON; break;

      /*
      case K_MOUSE_FWD:      case K_SFT_MOUSE_FWD:
      case K_CTL_MOUSE_FWD:  case K_ALT_MOUSE_FWD:  vkey1 = VK_XBUTTON1; break;
      case K_MOUSE_BACK:     case K_SFT_MOUSE_BACK:
      case K_CTL_MOUSE_BACK: case K_ALT_MOUSE_BACK: vkey1 = VK_XBUTTON2; break;
      */

      case K_SHIFT:          vkey1 = VK_SHIFT;    break;
      case K_LEFT_SHIFT:     vkey1 = VK_LSHIFT;   break;
      case K_RIGHT_SHIFT:    vkey1 = VK_RSHIFT;   break;
      case K_CONTROL:        vkey1 = VK_CONTROL;  not_vkey = VK_RMENU; vkey2 = VK_RCONTROL; break;
      case K_LEFT_CONTROL:   vkey1 = VK_LCONTROL; not_vkey = VK_RMENU; break;
      case K_RIGHT_CONTROL:  vkey1 = VK_RCONTROL; break;
      case K_ALT:            vkey1 = VK_MENU;     break;
      case K_LEFT_ALT:       vkey1 = VK_LMENU;    break;
      case K_RIGHT_ALT:      vkey1 = VK_RMENU;    break;
      case K_SHIFT_LOCK:     vkey1      = VK_CAPITAL; break;
      case K_SHIFT_LOCK_ON:  state_vkey = VK_CAPITAL; break;
      case K_NUM_LOCK:       vkey1      = VK_NUMLOCK; break;
      case K_NUM_LOCK_ON:    state_vkey = VK_NUMLOCK; break;
      case K_SCROLL_LOCK:    vkey1      = VK_SCROLL;  break;
      case K_SCROLL_LOCK_ON: state_vkey = VK_SCROLL;  break;

      default: result = FALSE; okay = FALSE; break;
    } /* switch */

    if (okay) {
      if (vkey1 != 0) {
        result = (GetKeyState(vkey1) & 0x8000) != 0;
        if (result && not_vkey != 0) {
          result = (GetKeyState(not_vkey) & 0x8000) == 0;
        } /* if */
        if (!result && vkey2 != 0) {
          result = (GetKeyState(vkey2) & 0x8000) != 0;
        } /* if */
      } else {
        result = (GetKeyState(state_vkey) & 0x0001) != 0;
      } /* if */
    } /* if */
    return result;
  } /* gkbButtonPressed */



charType gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



winType gkbWindow (void)

  {
    winType result;

  /* gkbWindow */
    logFunction(printf("gkbWindow\n"););
    result = find_window(button_window);
    if (result != NULL) {
      result->usage_count++;
    } /* if */
    logFunction(printf("gkbWindow -> " FMT_U_MEM "\n", (memSizeType) result););
    return result;
  } /* gkbWindow */



intType gkbButtonXpos (void)

  { /* gkbButtonXpos */
    return button_x;
  } /* gkbButtonXpos */



intType gkbButtonYpos (void)

  { /* gkbButtonYpos */
    return button_y;
  } /* gkbButtonYpos */
