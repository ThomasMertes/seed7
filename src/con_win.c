/********************************************************************/
/*                                                                  */
/*  con_win.c     Driver for windows console access.                */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/con_win.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Driver for windows console access.                     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "wchar.h"

#include "common.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "fil_rtl.h"
#include "ut8_rtl.h"
#include "rtl_err.h"
#include "con_drv.h"
#include "kbd_drv.h"


#define TRACE_EVENTS 0
#if TRACE_EVENTS
#define traceEvent(traceStatements) traceStatements
#else
#define traceEvent(traceStatements)
#endif
#define traceEventX(traceStatements) traceStatements

#define SCRHEIGHT 25
#define SCRWIDTH 80
#define WRITE_STRI_BLOCK_SIZE 256

static boolType keybd_initialized = FALSE;
static DWORD savedKeybdMode;
static HANDLE hKeyboard = INVALID_HANDLE_VALUE;

static char currentattribute;
static boolType console_initialized = FALSE;

static const charType map_1252_to_unicode[] = {
/* 128 */ 0x20AC,    '?', 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
/* 136 */ 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152,    '?', 0x017D,    '?',
/* 144 */    '?', 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
/* 152 */ 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153,    '?', 0x017E, 0x0178};



void kbdShut (void)

  { /* kbdShut */
    if (keybd_initialized) {
      if (hKeyboard != INVALID_HANDLE_VALUE) {
        SetConsoleMode(hKeyboard, savedKeybdMode);
      } /* if */
    } /* if */
  } /* kbdShut */



static void kbd_init (void)

  { /* kbd_init */
    logFunction(printf("kbd_init\n"););
    hKeyboard = GetStdHandle(STD_INPUT_HANDLE);
    if (hKeyboard != INVALID_HANDLE_VALUE) {
      if (unlikely(GetConsoleMode(hKeyboard, &savedKeybdMode) == 0)) {
        logError(printf("kbd_init: GetConsoleMode(hKeyboard, *) failed:\n"
                        "Error=" FMT_U32 "\n", (uint32Type) GetLastError());
                 fflush(stdout););
      } else {
#ifdef OUT_OF_ORDER
        if (savedKeybdMode & ENABLE_ECHO_INPUT) { printf("ECHO_INPUT\n"); }
        if (savedKeybdMode & ENABLE_EXTENDED_FLAGS) { printf("EXTENDED_FLAGS\n"); }
        if (savedKeybdMode & ENABLE_INSERT_MODE) { printf("INSERT_MODE\n"); }
        if (savedKeybdMode & ENABLE_LINE_INPUT) { printf("LINE_INPUT\n"); }
        if (savedKeybdMode & ENABLE_MOUSE_INPUT) { printf("MOUSE_INPUT\n"); }
        if (savedKeybdMode & ENABLE_PROCESSED_INPUT) { printf("PROCESSED_INPUT\n"); }
        if (savedKeybdMode & ENABLE_QUICK_EDIT_MODE) { printf("QUICK_EDIT_MODE\n"); }
        if (savedKeybdMode & ENABLE_WINDOW_INPUT) { printf("WINDOW_INPUT\n"); }
        /* if (savedKeybdMode & ENABLE_VIRTUAL_TERMINAL_INPUT) { printf("VIRTUAL_TERMINAL_INPUT\n"); } */
#endif
        /* ENABLE_LINE_INPUT enables CTRL-S processing. */
        /* ENABLE_PROCESSED_INPUT enables CTRL-C processing. */
        SetConsoleMode(hKeyboard, savedKeybdMode &
                       (DWORD) ~(ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT));
        keybd_initialized = TRUE;
        os_atexit(kbdShut);
      } /* if */
    } /* if */
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



boolType kbdInputReady (void)

  {
    INPUT_RECORD event;
    DWORD count = 0;
    boolType ignoreEvent;
    boolType result;

  /* kbdInputReady */
    logFunction(printf("kbdInputReady\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    do {
      ignoreEvent = FALSE;
      result = PeekConsoleInputW(hKeyboard, &event, 1, &count) != 0 &&
               count != 0;
      if (result) {
        if (event.EventType == KEY_EVENT) {
          if (event.Event.KeyEvent.bKeyDown) {
            traceEvent(printf("kbdInputReady: KEY_EVENT KeyDown"
                              ", ControlKeyState: " FMT_X32
                              ", VirtualKeyCode: " FMT_D16
                              ", UnicodeChar: " FMT_D16 "\n",
                              event.Event.KeyEvent.dwControlKeyState,
                              event.Event.KeyEvent.wVirtualKeyCode,
                              event.Event.KeyEvent.uChar.UnicodeChar););
            switch (event.Event.KeyEvent.wVirtualKeyCode){
              case VK_SHIFT:
              case VK_CONTROL:
              case VK_MENU:
              case VK_CAPITAL:
              case VK_NUMLOCK:
              case VK_SCROLL:
                ignoreEvent = TRUE;
                break;
            } /* switch */
          } else {
            traceEvent(printf("kbdInputReady: KEY_EVENT KeyUp"
                              ", ControlKeyState: " FMT_X32
                              ", VirtualKeyCode: " FMT_D16
                              ", UnicodeChar: " FMT_D16 "\n",
                              event.Event.KeyEvent.dwControlKeyState,
                              event.Event.KeyEvent.wVirtualKeyCode,
                              event.Event.KeyEvent.uChar.UnicodeChar););
            ignoreEvent = TRUE;
          } /* if */
        } else if (event.EventType == FOCUS_EVENT ||
                   event.EventType == MENU_EVENT ||
                   event.EventType == MOUSE_EVENT ||
                   event.EventType == WINDOW_BUFFER_SIZE_EVENT) {
          /* Ignore focus and menu events.                  */
          /* They are used internally by windows.           */
          /* Ignore mouse movement and button press events. */
          traceEvent(printf("kbdInputReady: Ignore event - EventType: " FMT_U16 "\n",
                            event.EventType););
          ignoreEvent = TRUE;
        } else {
          /* Unknown event */
          traceEvent(printf("kbdInputReady: EventType: " FMT_U16 "\n",
                            event.EventType););
        } /* if */
        if (ignoreEvent) {
          /* Skip the event to be ignored. */
          ignoreEvent = ReadConsoleInputW(hKeyboard, &event, 1, &count) != 0;
          /* If reading the event, that already has been */
          /* peeked, fails the loop is terminated. */
        } /* if */
      } /* if */
    } while (ignoreEvent);
    logFunction(printf("kbdInputReady --> %d\n", result););
    return result;
  } /* kbdInputReady */



charType kbdGetc (void)

  {
    INPUT_RECORD event;
    DWORD count;
    boolType altNumpadUsed = FALSE;
    charType highSurrogate = 0;
    charType result = K_NONE;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    while (result == K_NONE &&
        ReadConsoleInputW(hKeyboard, &event, 1, &count) != 0) {
      if (event.EventType == KEY_EVENT) {
        if (event.Event.KeyEvent.bKeyDown) {
          traceEvent(printf("kbdGetc: KEY_EVENT KeyDown"
                            ", ControlKeyState: " FMT_X32
                            ", VirtualKeyCode: " FMT_D16
                            ", UnicodeChar: " FMT_D16 "\n",
                            event.Event.KeyEvent.dwControlKeyState,
                            event.Event.KeyEvent.wVirtualKeyCode,
                            event.Event.KeyEvent.uChar.UnicodeChar););
          if (event.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) {
            switch (event.Event.KeyEvent.wVirtualKeyCode){
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
              case VK_F11:      result = K_SFT_F11;    break;
              case VK_F12:      result = K_SFT_F12;    break;
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
          } else if (event.Event.KeyEvent.dwControlKeyState &
                     (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
            switch (event.Event.KeyEvent.wVirtualKeyCode){
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
              case VK_F11:      result = K_ALT_F11;    break;
              case VK_F12:      result = K_ALT_F12;    break;
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
              case VK_NUMPAD0:
              case VK_NUMPAD1:
              case VK_NUMPAD2:
              case VK_NUMPAD3:
              case VK_NUMPAD4:
              case VK_NUMPAD5:
              case VK_NUMPAD6:
              case VK_NUMPAD7:
              case VK_NUMPAD8:
              case VK_NUMPAD9:
                if (event.Event.KeyEvent.dwControlKeyState & NUMLOCK_ON) {
                  result = K_UNDEF;
                } else {
                  altNumpadUsed = TRUE;
                } /* if */
                break;
              default:          result = K_UNDEF;      break;
            } /* switch */
          } else if (event.Event.KeyEvent.dwControlKeyState &
                     (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
            switch (event.Event.KeyEvent.wVirtualKeyCode){
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
              case VK_F11:      result = K_CTL_F11;    break;
              case VK_F12:      result = K_CTL_F12;    break;
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
            switch (event.Event.KeyEvent.wVirtualKeyCode){
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
            if (event.Event.KeyEvent.uChar.UnicodeChar != 0) {
              if (event.Event.KeyEvent.dwControlKeyState &
                  (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
                switch (event.Event.KeyEvent.uChar.UnicodeChar) {
                  case 'A': case 'a': result = K_ALT_A; break;
                  case 'B': case 'b': result = K_ALT_B; break;
                  case 'C': case 'c': result = K_ALT_C; break;
                  case 'D': case 'd': result = K_ALT_D; break;
                  case 'E': case 'e': result = K_ALT_E; break;
                  case 'F': case 'f': result = K_ALT_F; break;
                  case 'G': case 'g': result = K_ALT_G; break;
                  case 'H': case 'h': result = K_ALT_H; break;
                  case 'I': case 'i': result = K_ALT_I; break;
                  case 'J': case 'j': result = K_ALT_J; break;
                  case 'K': case 'k': result = K_ALT_K; break;
                  case 'L': case 'l': result = K_ALT_L; break;
                  case 'M': case 'm': result = K_ALT_M; break;
                  case 'N': case 'n': result = K_ALT_N; break;
                  case 'O': case 'o': result = K_ALT_O; break;
                  case 'P': case 'p': result = K_ALT_P; break;
                  case 'Q': case 'q': result = K_ALT_Q; break;
                  case 'R': case 'r': result = K_ALT_R; break;
                  case 'S': case 's': result = K_ALT_S; break;
                  case 'T': case 't': result = K_ALT_T; break;
                  case 'U': case 'u': result = K_ALT_U; break;
                  case 'V': case 'v': result = K_ALT_V; break;
                  case 'W': case 'w': result = K_ALT_W; break;
                  case 'X': case 'x': result = K_ALT_X; break;
                  case 'Y': case 'y': result = K_ALT_Y; break;
                  case 'Z': case 'z': result = K_ALT_Z; break;
                  case '0':           result = K_ALT_0; break;
                  case '1':           result = K_ALT_1; break;
                  case '2':           result = K_ALT_2; break;
                  case '3':           result = K_ALT_3; break;
                  case '4':           result = K_ALT_4; break;
                  case '5':           result = K_ALT_5; break;
                  case '6':           result = K_ALT_6; break;
                  case '7':           result = K_ALT_7; break;
                  case '8':           result = K_ALT_8; break;
                  case '9':           result = K_ALT_9; break;
                } /* switch */
              } /* if */
              if (result == K_UNDEF) {
                result = event.Event.KeyEvent.uChar.UnicodeChar;
                if (result >= 128 && result <= 159) {
                  result = map_1252_to_unicode[result - 128];
                } /* if */
              } /* if */
            } else {
              /*
              printf("VK: %lu\n",
                     (unsigned long) event.Event.KeyEvent.wVirtualKeyCode);
              printf("Char: %lu\n",
                     (unsigned long) event.Event.KeyEvent.uChar.UnicodeChar);
              printf("CKey: %lx\n",
                     (unsigned long) event.Event.KeyEvent.dwControlKeyState);
              */
            } /* if */
          } /* if */
        } else {
          traceEvent(printf("kbdGetc: KEY_EVENT KeyUp"
                            ", ControlKeyState: " FMT_X32
                            ", VirtualKeyCode: " FMT_D16
                            ", UnicodeChar: " FMT_D16 "\n",
                            event.Event.KeyEvent.dwControlKeyState,
                            event.Event.KeyEvent.wVirtualKeyCode,
                            event.Event.KeyEvent.uChar.UnicodeChar););
          if (event.Event.KeyEvent.wVirtualKeyCode == VK_MENU &&
              altNumpadUsed) {
            result = event.Event.KeyEvent.uChar.UnicodeChar;
            if (result >= 0xd800 && result <= 0xdfff) {
              if (result <= 0xdbff) {
                if (highSurrogate == 0) {
                  highSurrogate = result;
                  result = K_NONE;
                } else {
                  result = K_UNDEF;
                } /* if */
              } else {
                if (highSurrogate != 0) {
                  result = 0x10000 + (((highSurrogate - 0xd800) << 10) | (result - 0xdc00));
                } else {
                  result = K_UNDEF;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } else if (event.EventType == FOCUS_EVENT ||
                 event.EventType == MENU_EVENT ||
                 event.EventType == MOUSE_EVENT ||
                 event.EventType == WINDOW_BUFFER_SIZE_EVENT) {
        /* Ignore focus and menu events.                  */
        /* They are used internally by windows.           */
        /* Ignore mouse movement and button press events. */
        traceEvent(printf("kbdGetc: Ignore event - EventType: " FMT_U16 "\n",
                          event.EventType););
      } else {
        /* Unknown event */
        traceEvent(printf("kbdGetc: EventType: " FMT_U16 "\n",
                          event.EventType););
        result = K_UNDEF;
      } /* if */
    } /* while */
    logFunction(printf("kbdGetc --> %d\n", result););
    return result;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    logFunction(printf("kbdRawGetc\n"););
    return kbdGetc();
  } /* kbdRawGetc */



static void con_setcolor (intType foreground, intType background)

  { /* con_setcolor */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* con_setcolor */



static void con_standardcolour (void)

  { /* con_standardcolour */
    con_setcolor(lightgray, black);
  } /* con_standardcolour */



static void con_normalcolour (void)

  { /* con_normalcolour */
    con_setcolor(lightgray, black);
  } /* con_normalcolour */



intType textheight (void)

  { /* textheight */
    return 1;
  } /* textheight */



intType textwidth (striType stri,
    intType startcol, intType stopcol)

  { /* textwidth */
    return stopcol + 1 - startcol;
  } /* textwidth */



void textcolumns (striType stri, intType striwidth,
    intType *cols, intType *rest)

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



int conHeight (void)

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conHeight */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return con_info.dwSize.Y;
      } else {
        logMessage(printf("conHeight: GetConsoleScreenBufferInfo(" FMT_U_MEM ", *) "
                          "--> Error " FMT_U32 "\n",
                          (memSizeType) hConsole, (uint32Type) GetLastError()););
        return SCRHEIGHT;
      } /* if */
    } else {
      logMessage(printf("conHeight: GetStdHandle(STD_OUTPUT_HANDLE) "
                        "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                        (memSizeType) hConsole, (uint32Type) GetLastError()););
      return SCRHEIGHT;
    } /* if */
  } /* conHeight */



int conWidth (void)

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conWidth */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return con_info.dwSize.X;
      } else {
        logMessage(printf("conWidth: GetConsoleScreenBufferInfo(" FMT_U_MEM ", *) "
                          "--> Error " FMT_U32 "\n",
                          (memSizeType) hConsole, (uint32Type) GetLastError()););
        return SCRWIDTH;
      } /* if */
    } else {
      logMessage(printf("conWidth: GetStdHandle(STD_OUTPUT_HANDLE) "
                        "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                        (memSizeType) hConsole, (uint32Type) GetLastError()););
      return SCRWIDTH;
    } /* if */
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

  {
    HANDLE hConsole;
    CONSOLE_CURSOR_INFO info;

  /* conCursor */
    if (console_initialized) {
      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
        if (likely(GetConsoleCursorInfo(hConsole, &info) != 0)) {
          info.bVisible = on;
          if (unlikely(SetConsoleCursorInfo(hConsole, &info) == 0)) {
            logMessage(printf("conCursor: SetConsoleCursorInfo(" FMT_U_MEM ", "
                              "(visible=%d)) --> Error " FMT_U32 "\n",
                              (memSizeType) hConsole, on, (uint32Type) GetLastError()););
          } /* if */
        } else {
          logMessage(printf("conCursor: GetConsoleCursorInfo(" FMT_U_MEM ", *) "
                            "--> Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } else {
        logMessage(printf("conCursor: GetStdHandle(STD_OUTPUT_HANDLE) "
                          "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                          (memSizeType) hConsole, (uint32Type) GetLastError()););
      } /* if */
    } /* if */
  } /* conCursor */



/**
 *  Moves the system cursor to the given place of the console.
 *  If no system cursor exists this function can be replaced by
 *  a dummy function.
 */
void conSetCursor (intType line, intType column)

  {
    HANDLE hConsole;
    COORD position;

  /* conSetCursor */
    if (unlikely(line <= 0 || column <= 0)) {
      raise_error(RANGE_ERROR);
    } else if (line <= INT16TYPE_MAX && column <= INT16TYPE_MAX) {
      if (console_initialized) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          position.X = (int16Type) (column - 1);
          position.Y = (int16Type) (line - 1);
          if (SetConsoleCursorPosition(hConsole, position) == 0) {
            logMessage(printf("conSetCursor: SetConsoleCursorPosition(" FMT_U_MEM ", "
                              "(" FMT_D ", " FMT_D ")) --> Error " FMT_U32 "\n",
                              (memSizeType) hConsole, column - 1, line - 1,
                              (uint32Type) GetLastError()););
          } /* if */
        } else {
          logMessage(printf("conSetCursor: GetStdHandle(STD_OUTPUT_HANDLE) "
                            "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } /* if */
    } /* if */
  } /* conSetCursor */



intType conColumn (void)

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conColumn */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return (intType) con_info.dwCursorPosition.X + 1;
      } else {
        logMessage(printf("conColumn: GetConsoleScreenBufferInfo(" FMT_U_MEM ", *) "
                          "--> Error " FMT_U32 "\n",
                          (memSizeType) hConsole, (uint32Type) GetLastError()););
        return 1;
      } /* if */
    } else {
      logMessage(printf("conColumn: GetStdHandle(STD_OUTPUT_HANDLE) "
                        "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                        (memSizeType) hConsole, (uint32Type) GetLastError()););
      return 1;
    } /* if */
  } /* conColumn */



intType conLine (void)

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conLine */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return (intType) con_info.dwCursorPosition.Y + 1;
      } else {
        logMessage(printf("conLine: GetConsoleScreenBufferInfo(" FMT_U_MEM ", *) "
                          "--> Error " FMT_U32 "\n",
                          (memSizeType) hConsole, (uint32Type) GetLastError()););
        return 1;
      } /* if */
    } else {
      logMessage(printf("conLine: GetStdHandle(STD_OUTPUT_HANDLE) "
                        "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                        (memSizeType) hConsole, (uint32Type) GetLastError()););
      return 1;
    } /* if */
  } /* conLine */



static void doWriteConsole (HANDLE hConsole, const const_striType stri)

  {
    utf16charType wstri_buffer[WRITE_STRI_BLOCK_SIZE * SURROGATE_PAIR_FACTOR];
    utf16striType wstri;
    utf16striType wstri_part;
    memSizeType wstri_size;
    errInfoType err_info = OKAY_NO_ERROR;
    DWORD numchars;

  /* doWriteConsole */
    logFunction(fprintf(stderr, "doWriteConsole(%lx, ...)",
                        (unsigned long) hConsole););
    if (stri->size <= WRITE_STRI_BLOCK_SIZE) {
      wstri_size = stri_to_utf16(wstri_buffer, stri->mem, stri->size, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        WriteConsoleW(hConsole, wstri_buffer, (DWORD) wstri_size, &numchars, NULL);
      } /* if */
    } else {
      /* ALLOC_UTF16 adds space for a NULL termination, which is not needed here. */
      if (unlikely(stri->size >
                   (MAX_UTF16_LEN + NULL_TERMINATION_LEN) / SURROGATE_PAIR_FACTOR ||
                   !ALLOC_UTF16(wstri, stri->size * SURROGATE_PAIR_FACTOR -
                   NULL_TERMINATION_LEN))) {
        raise_error(MEMORY_ERROR);
      } else {
        wstri_size = stri_to_utf16(wstri, stri->mem, stri->size, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          wstri_part = wstri;
          /* Writing may fail for lengths above 26000 to 32000 */
          while (wstri_size > 25000) {
            WriteConsoleW(hConsole, wstri_part, 25000, &numchars, NULL);
            wstri_part = &wstri_part[25000];
            wstri_size -= 25000;
          } /* while */
          WriteConsoleW(hConsole, wstri_part, (DWORD) wstri_size, &numchars, NULL);
        } /* if */
        UNALLOC_UTF16(wstri, stri->size * SURROGATE_PAIR_FACTOR - NULL_TERMINATION_LEN);
      } /* if */
    } /* if */
  } /* doWriteConsole */



/**
 *  Write a string to the current position of the console.
 *  Unicode characters are written with the encoding of the
 *  operating system. The cursor position is changed, if
 *  one of the characters '\n', '\r' and '\b' is written.
 *  If the standard output file of the operating system has
 *  been redirected UTF-8 encoded characters are written to
 *  the redirected file.
 */
void conWrite (const const_striType stri)

  {
    HANDLE hConsole;
    DWORD mode;

  /* conWrite */
    logFunction(printf("conWrite(\"%s\")\n", striAsUnquotedCStri(stri)););
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE &&
        GetFileType(hConsole) == FILE_TYPE_CHAR &&
        GetConsoleMode(hConsole, &mode) != 0) {
      /* hConsole refers to a real console */
      doWriteConsole(hConsole, stri);
    } else {
      /* The output has been redirected */
      ut8Write(&stdoutFileRecord, stri);
    } /* if */
    logFunction(printf("conWrite -->\n"););
  } /* conWrite */



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  {
    HANDLE hConsole;
    COORD position;
    DWORD numchars;

  /* conClear */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (stoplin > INT16TYPE_MAX) {
        stoplin = INT16TYPE_MAX;
      } /* if */
      if (stopcol > INT16TYPE_MAX) {
        stopcol = INT16TYPE_MAX;
      } /* if */
      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
        position.X = (int16Type) (startcol - 1);
        position.Y = (int16Type) (startlin - 1);
        while (position.Y < (int16Type) stoplin) {
          FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
              (unsigned int) (stopcol - startcol + 1), position, &numchars);
          position.Y++;
        } /* while */
      } /* if */
    } /* if */
  } /* conClear */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by count lines. The upper count lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conUpScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin + count - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          logMessage(printf("conUpScroll: GetStdHandle(STD_OUTPUT_HANDLE) "
                            "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } /* if */
    } /* if */
  } /* conUpScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by count lines. The lower count lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conDownScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - count - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin + count - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          logMessage(printf("conDownScroll: GetStdHandle(STD_OUTPUT_HANDLE) "
                            "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } /* if */
    } /* if */
  } /* conDownScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by count columns. The left count columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conLeftScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stopcol - startcol + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol + count - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          logMessage(printf("conLeftScroll: GetStdHandle(STD_OUTPUT_HANDLE) "
                            "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } /* if */
    } /* if */
  } /* conLeftScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by count columns. The right count columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conRightScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stopcol - startcol + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - count - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol + count - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          logMessage(printf("conRightScroll: GetStdHandle(STD_OUTPUT_HANDLE) "
                            "--> " FMT_U_MEM " / Error " FMT_U32 "\n",
                            (memSizeType) hConsole, (uint32Type) GetLastError()););
        } /* if */
      } /* if */
    } /* if */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    logFunction(printf("conShut\n"););
    if (console_initialized) {
      con_standardcolour();
      conCursor(TRUE);
      conClear(1, 1, conHeight(), conWidth());
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
    logFunction(printf("conShut -->\n"););
  } /* conShut */



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  { /* conOpen */
    logFunction(printf("conOpen\n"););
    con_normalcolour();
    conClear(1, 1, conHeight(), conWidth());
    console_initialized = TRUE;
    conCursor(FALSE);
    conSetCursor(1, 1);
    os_atexit(conShut);
    logFunction(printf("conOpen -->\n"););
    return 1;
  } /* conOpen */
