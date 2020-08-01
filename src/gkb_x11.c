/********************************************************************/
/*                                                                  */
/*  gkb_x11.c     Keyboard and mouse access with X11 capabilities.  */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
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
/*  Changes: 1994, 2000 - 2011  Thomas Mertes                       */
/*  Content: Keyboard and mouse access with X11 capabilities.       */
/*                                                                  */
/********************************************************************/


#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef OUT_OF_ORDER
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#endif
#include <X11/keysym.h>

#include "common.h"
#include "data_rtl.h"
#include "hsh_rtl.h"
#include "kbd_drv.h"

#undef FLAG_EVENTS
#undef TRACE_KBD

#define ALLOW_REPARENT_NOTIFY


extern Display *mydisplay;
extern Atom wm_delete_window;
static booltype eventPresent = FALSE;
static XEvent currentEvent;
static inttype button_x = 0;
static inttype button_y = 0;
static Window button_window = 0;
static rtlHashtype window_hash = NULL;


#ifdef ANSI_C

extern void redraw (wintype redraw_window, int xPos, int yPos, int width, int height);
extern void doFlush (void);
extern void flushBeforeRead (void);

#else

extern void redraw ();
extern void doFlush ();
extern void flushBeforeRead ();

#endif



#ifdef ANSI_C

wintype find_window (Window curr_window)
#else

wintype find_window (curr_window)
Window curr_window;
#endif

  {
    wintype window;

  /* find_window */
    if (window_hash == NULL) {
      window = NULL;
    } else {
      window = (wintype) (memsizetype)
          hshIdxWithDefault(window_hash,
                            (rtlGenerictype) (memsizetype) curr_window,
                            (rtlGenerictype) (memsizetype) NULL,
                            (inttype) ((memsizetype) curr_window) >> 6,
                            (comparetype) &uintCmpGeneric);
    } /* if */
    return window;
  } /* find_window */



#ifdef ANSI_C

void enter_window (wintype curr_window, Window xWin)
#else

void enter_window (curr_window, xWin)
wintype curr_window;
Window xWin;
#endif

  { /* enter_window */
    if (window_hash == NULL) {
      window_hash = hshEmpty();
    } /* if */
    (void) hshIdxEnterDefault(window_hash, (rtlGenerictype) (memsizetype) xWin,
                              (rtlGenerictype) (memsizetype) curr_window,
                              (inttype) ((memsizetype) xWin) >> 6,
                              (comparetype) &uintCmpGeneric,
                              (createfunctype) &intCreateGeneric,
                              (createfunctype) &intCreateGeneric);
  } /* enter_window */



#ifdef ANSI_C

void remove_window (wintype curr_window, Window xWin)
#else

void remove_window (curr_window, xWin)
wintype curr_window;
Window xWin;
#endif

  { /* remove_window */
    hshExcl(window_hash, (rtlGenerictype) (memsizetype) xWin,
            (inttype) ((memsizetype) xWin) >> 6, (comparetype) &uintCmpGeneric,
            (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
  } /* remove_window */



#ifdef ANSI_C

void handleExpose (XExposeEvent *xexpose)
#else

void handleExpose (xexpose)
XExposeEvent *xexpose;
#endif

  {
    wintype redraw_window;

  /* handleExpose */
#ifdef TRACE_KBD
    printf("begin handleExpose\n");
#endif
    /* printf("XExposeEvent x=%d, y=%d, width=%d, height=%d, count=%d\n",
        xexpose->x, xexpose->y, xexpose->width, xexpose->height, xexpose->count); */
    redraw_window = find_window(xexpose->window);
    redraw(redraw_window, xexpose->x, xexpose->y, xexpose->width, xexpose->height);
#ifdef TRACE_KBD
    printf("end handleExpose\n");
#endif
  } /* handleExpose */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void waitForReparent (void)
#else

void waitForReparent ()
#endif

  { /* waitForReparent */
    XNextEvent(mydisplay, &currentEvent);
    switch(currentEvent.type) {
      case ReparentNotify:
      case ConfigureNotify:
        break;
      default:
        printf("Other Event %d\n", currentEvent.type);
        break;
    } /* switch */
    XNextEvent(mydisplay, &currentEvent);
    switch(currentEvent.type) {
      case ReparentNotify:
      case ConfigureNotify:
        break;
      default:
        printf("Other Event %d\n", currentEvent.type);
        break;
    } /* switch */
  } /* waitForReparent */
#endif



#ifdef ANSI_C

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  {
    KeySym currentKey;
    int lookup_count;
    unsigned char buffer[21];
    chartype result;

  /* gkbGetc */
#ifdef TRACE_KBD
    printf("begin gkbGetc\n");
#endif
    flushBeforeRead();
    result = K_NONE;
    if (eventPresent) {
      eventPresent = FALSE;
    } else {
      XNextEvent(mydisplay, &currentEvent);
    } /* if */
    while (currentEvent.type == KeyRelease) {
      XNextEvent(mydisplay, &currentEvent);
    } /* while */
    switch(currentEvent.type) {
      case Expose:
#ifdef FLAG_EVENTS
        printf("Expose\n");
#endif
        handleExpose(&currentEvent.xexpose);
        result = gkbGetc();
        break;

#ifdef OUT_OF_ORDER
      case ConfigureNotify:
#ifdef FLAG_EVENTS
        printf("ConfigureNotify");
#endif
        configure(&currentEvent.xconfigure);
        result = gkbGetc();
        break;
      case MapNotify:
#endif

#ifdef TRACE_REPARENT_NOTIFY
      case ReparentNotify:
        printf("gkbGetc: Reparent\n");
        result = gkbGetc();
        break;

      case ConfigureNotify:
        printf("gkbGetc: Configure %lx %d, %d\n",
               currentEvent.xconfigure.window,
               currentEvent.xconfigure.width, currentEvent.xconfigure.height);
        result = gkbGetc();
        break;
#endif
#ifdef ALLOW_REPARENT_NOTIFY
      case ReparentNotify:
      case ConfigureNotify:
#endif
      case GraphicsExpose:
      case NoExpose:
#ifdef FLAG_EVENTS
        printf("NoExpose\n");
#endif
        result = gkbGetc();
        break;

      case MappingNotify:
#ifdef FLAG_EVENTS
        printf("MappingNotify\n");
#endif
        XRefreshKeyboardMapping(&currentEvent.xmapping);
        break;

#ifdef OUT_OF_ORDER
      case DestroyNotify:
#ifdef FLAG_EVENTS
        printf("DestroyNotify\n");
#endif
        exit(1);
        break;
#endif

      case ClientMessage:
#ifdef FLAG_EVENTS
        printf("ClientMessage\n");
#endif
        if ((Atom) currentEvent.xclient.data.l[0] == wm_delete_window) {
          /* printf("do exit\n"); */
          exit(1);
        } /* if */
        break;

      case ButtonPress:
#ifdef FLAG_EVENTS
        printf("ButtonPress (%d, %d, %u)\n",
            currentEvent.xbutton.x, currentEvent.xbutton.y, currentEvent.xbutton.button);
#endif
        button_x = currentEvent.xbutton.x;
        button_y = currentEvent.xbutton.y;
        button_window = currentEvent.xbutton.window;
        if (currentEvent.xbutton.button >= 1 && currentEvent.xbutton.button <= 5) {
          result = currentEvent.xbutton.button + K_MOUSE1 - 1;
        } else {
          result = K_UNDEF;
        } /* if */
        break;

      case KeyPress:
#ifdef FLAG_EVENTS
        printf("KeyPress\n");
        printf("xkey.state (%o)\n", currentEvent.xkey.state);
#endif
        lookup_count = XLookupString(&currentEvent.xkey, (cstritype) buffer,
                                     20, &currentKey, 0);
        buffer[lookup_count] = '\0';
        if (currentEvent.xkey.state & ShiftMask) {
          /* printf("ShiftMask\n"); */
          switch (currentKey) {
            case XK_Return:     result = K_NL;          break;
            case XK_BackSpace:  result = K_BS;          break;
            case XK_ISO_Left_Tab:
            case XK_Tab:        result = K_BACKTAB;     break;
            case XK_Linefeed:   result = K_NL;          break;
            case XK_Escape:     result = K_ESC;         break;
            case XK_F1:         result = K_SFT_F1;      break;
            case XK_F2:         result = K_SFT_F2;      break;
            case XK_F3:         result = K_SFT_F3;      break;
            case XK_F4:         result = K_SFT_F4;      break;
            case XK_F5:         result = K_SFT_F5;      break;
            case XK_F6:         result = K_SFT_F6;      break;
            case XK_F7:         result = K_SFT_F7;      break;
            case XK_F8:         result = K_SFT_F8;      break;
            case XK_F9:         result = K_SFT_F9;      break;
            case XK_F10:        result = K_SFT_F10;     break;
            case XK_F11:        result = K_SFT_F11;     break;
            case XK_F12:        result = K_SFT_F12;     break;
            case XK_Left:       result = K_LEFT;        break;
            case XK_Right:      result = K_RIGHT;       break;
            case XK_Up:         result = K_UP;          break;
            case XK_Down:       result = K_DOWN;        break;
            case XK_Home:       result = K_HOME;        break;
            case XK_End:        result = K_END;         break;
            case XK_Prior:      result = K_PGUP;        break;
            case XK_Next:       result = K_PGDN;        break;
            case XK_Insert:     result = K_INS;         break;
            case XK_Delete:     result = K_DEL;         break;
            case XK_KP_Enter:   result = K_NL;          break;
            case XK_Shift_L:
            case XK_Shift_R:
            case XK_Control_L:
            case XK_Control_R:
            case XK_Alt_L:
            case XK_Alt_R:
            case XK_Mode_switch:
            case XK_Caps_Lock:
            case XK_Num_Lock:
            case XK_Shift_Lock: result = gkbGetc(); break;
            default:
              if (lookup_count == 1) {
                result = buffer[0];
              } else {
                printf("1 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (currentEvent.xkey.state & ControlMask) {
          /* printf("ControlMask\n"); */
          switch (currentKey) {
            case XK_Return:     result = K_CTL_NL;      break;
            case XK_BackSpace:  result = K_UNDEF;       break;
            case XK_Tab:        result = K_UNDEF;       break;
            case XK_Linefeed:   result = K_CTL_NL;      break;
            case XK_Escape:     result = K_UNDEF;       break;
            case XK_F1:         result = K_CTL_F1;      break;
            case XK_F2:         result = K_CTL_F2;      break;
            case XK_F3:         result = K_CTL_F3;      break;
            case XK_F4:         result = K_CTL_F4;      break;
            case XK_F5:         result = K_CTL_F5;      break;
            case XK_F6:         result = K_CTL_F6;      break;
            case XK_F7:         result = K_CTL_F7;      break;
            case XK_F8:         result = K_CTL_F8;      break;
            case XK_F9:         result = K_CTL_F9;      break;
            case XK_F10:        result = K_CTL_F10;     break;
            case XK_F11:        result = K_CTL_F11;     break;
            case XK_F12:        result = K_CTL_F12;     break;
            case XK_Left:       result = K_CTL_LEFT;    break;
            case XK_Right:      result = K_CTL_RIGHT;   break;
            case XK_Up:         result = K_CTL_UP;      break;
            case XK_Down:       result = K_CTL_DOWN;    break;
            case XK_Home:       result = K_CTL_HOME;    break;
            case XK_End:        result = K_CTL_END;     break;
            case XK_Prior:      result = K_CTL_PGUP;    break;
            case XK_Next:       result = K_CTL_PGDN;    break;
            case XK_Insert:     result = K_CTL_INS;     break;
            case XK_Delete:     result = K_CTL_DEL;     break;
            case XK_KP_Left:    result = K_CTL_LEFT;    break;
            case XK_KP_Right:   result = K_CTL_RIGHT;   break;
            case XK_KP_Up:      result = K_CTL_UP;      break;
            case XK_KP_Down:    result = K_CTL_DOWN;    break;
            case XK_KP_Home:    result = K_CTL_HOME;    break;
            case XK_KP_End:     result = K_CTL_END;     break;
            case XK_KP_Prior:   result = K_CTL_PGUP;    break;
            case XK_KP_Next:    result = K_CTL_PGDN;    break;
            case XK_KP_Insert:  result = K_CTL_INS;     break;
            case XK_KP_Delete:  result = K_CTL_DEL;     break;
            case XK_KP_Begin:   result = K_UNDEF;       break;
            case XK_KP_4:       result = K_CTL_LEFT;    break;
            case XK_KP_6:       result = K_CTL_RIGHT;   break;
            case XK_KP_8:       result = K_CTL_UP;      break;
            case XK_KP_2:       result = K_CTL_DOWN;    break;
            case XK_KP_7:       result = K_CTL_HOME;    break;
            case XK_KP_1:       result = K_CTL_END;     break;
            case XK_KP_9:       result = K_CTL_PGUP;    break;
            case XK_KP_3:       result = K_CTL_PGDN;    break;
            case XK_KP_0:       result = K_CTL_INS;     break;
            case XK_KP_5:       result = K_UNDEF;       break;
            case XK_KP_Enter:   result = K_CTL_NL;      break;
            case XK_KP_Decimal: result = K_CTL_DEL;     break;
            case XK_0:          result = '0';           break;
            case XK_1:          result = '1';           break;
            case XK_2:          result = '2';           break;
            case XK_3:          result = '3';           break;
            case XK_4:          result = '4';           break;
            case XK_5:          result = '5';           break;
            case XK_6:          result = '6';           break;
            case XK_7:          result = '7';           break;
            case XK_8:          result = '8';           break;
            case XK_9:          result = '9';           break;
            case XK_Shift_L:
            case XK_Shift_R:
            case XK_Control_L:
            case XK_Control_R:
            case XK_Alt_L:
            case XK_Alt_R:
            case XK_Mode_switch:
            case XK_Caps_Lock:
            case XK_Num_Lock:
            case XK_Shift_Lock: result = gkbGetc(); break;
            default:
              if (lookup_count == 1) {
                result = buffer[0];
              } else {
                printf("2 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (currentEvent.xkey.state & Mod1Mask) { /* Left ALT modifier */
          /* printf("Mod1Mask\n"); */
          switch (currentKey) {
            case XK_Return:     result = K_UNDEF;       break;
            case XK_BackSpace:  result = K_UNDEF;       break;
            case XK_Tab:        result = K_UNDEF;       break;
            case XK_Linefeed:   result = K_UNDEF;       break;
            case XK_Escape:     result = K_UNDEF;       break;
            case XK_F1:         result = K_ALT_F1;      break;
            case XK_F2:         result = K_ALT_F2;      break;
            case XK_F3:         result = K_ALT_F3;      break;
            case XK_F4:         result = K_ALT_F4;      break;
            case XK_F5:         result = K_ALT_F5;      break;
            case XK_F6:         result = K_ALT_F6;      break;
            case XK_F7:         result = K_ALT_F7;      break;
            case XK_F8:         result = K_ALT_F8;      break;
            case XK_F9:         result = K_ALT_F9;      break;
            case XK_F10:        result = K_ALT_F10;     break;
            case XK_F11:        result = K_ALT_F11;     break;
            case XK_F12:        result = K_ALT_F12;     break;
            case XK_Left:       result = K_UNDEF;       break;
            case XK_Right:      result = K_UNDEF;       break;
            case XK_Up:         result = K_UNDEF;       break;
            case XK_Down:       result = K_UNDEF;       break;
            case XK_Home:       result = K_UNDEF;       break;
            case XK_End:        result = K_UNDEF;       break;
            case XK_Prior:      result = K_UNDEF;       break;
            case XK_Next:       result = K_UNDEF;       break;
            case XK_Insert:     result = K_UNDEF;       break;
            case XK_Delete:     result = K_UNDEF;       break;
            case XK_KP_Left:    result = K_UNDEF;       break;
            case XK_KP_Right:   result = K_UNDEF;       break;
            case XK_KP_Up:      result = K_UNDEF;       break;
            case XK_KP_Down:    result = K_UNDEF;       break;
            case XK_KP_Home:    result = K_UNDEF;       break;
            case XK_KP_End:     result = K_UNDEF;       break;
            case XK_KP_Prior:   result = K_UNDEF;       break;
            case XK_KP_Next:    result = K_UNDEF;       break;
            case XK_KP_Insert:  result = K_UNDEF;       break;
            case XK_KP_Delete:  result = K_UNDEF;       break;
            case XK_KP_Begin:   result = K_UNDEF;       break;
            case XK_KP_4:       result = K_UNDEF;       break;
            case XK_KP_6:       result = K_UNDEF;       break;
            case XK_KP_8:       result = K_UNDEF;       break;
            case XK_KP_2:       result = K_UNDEF;       break;
            case XK_KP_7:       result = K_UNDEF;       break;
            case XK_KP_1:       result = K_UNDEF;       break;
            case XK_KP_9:       result = K_UNDEF;       break;
            case XK_KP_3:       result = K_UNDEF;       break;
            case XK_KP_0:       result = K_UNDEF;       break;
            case XK_KP_5:       result = K_UNDEF;       break;
            case XK_KP_Enter:   result = K_UNDEF;       break;
            case XK_KP_Decimal: result = K_UNDEF;       break;
            case XK_Shift_L:
            case XK_Shift_R:
            case XK_Control_L:
            case XK_Control_R:
            case XK_Alt_L:
            case XK_Alt_R:
            case XK_Mode_switch:
            case XK_Caps_Lock:
            case XK_Num_Lock:
            case XK_Shift_Lock: result = gkbGetc(); break;
            default:
              if (lookup_count == 1) {
                switch (buffer[0]) {
                  case '0':     result = K_ALT_0;       break;
                  case '1':     result = K_ALT_1;       break;
                  case '2':     result = K_ALT_2;       break;
                  case '3':     result = K_ALT_3;       break;
                  case '4':     result = K_ALT_4;       break;
                  case '5':     result = K_ALT_5;       break;
                  case '6':     result = K_ALT_6;       break;
                  case '7':     result = K_ALT_7;       break;
                  case '8':     result = K_ALT_8;       break;
                  case '9':     result = K_ALT_9;       break;
                  case 'a':     result = K_ALT_A;       break;
                  case 'b':     result = K_ALT_B;       break;
                  case 'c':     result = K_ALT_C;       break;
                  case 'd':     result = K_ALT_D;       break;
                  case 'e':     result = K_ALT_E;       break;
                  case 'f':     result = K_ALT_F;       break;
                  case 'g':     result = K_ALT_G;       break;
                  case 'h':     result = K_ALT_H;       break;
                  case 'i':     result = K_ALT_I;       break;
                  case 'j':     result = K_ALT_J;       break;
                  case 'k':     result = K_ALT_K;       break;
                  case 'l':     result = K_ALT_L;       break;
                  case 'm':     result = K_ALT_M;       break;
                  case 'n':     result = K_ALT_N;       break;
                  case 'o':     result = K_ALT_O;       break;
                  case 'p':     result = K_ALT_P;       break;
                  case 'q':     result = K_ALT_Q;       break;
                  case 'r':     result = K_ALT_R;       break;
                  case 's':     result = K_ALT_S;       break;
                  case 't':     result = K_ALT_T;       break;
                  case 'u':     result = K_ALT_U;       break;
                  case 'v':     result = K_ALT_V;       break;
                  case 'w':     result = K_ALT_W;       break;
                  case 'x':     result = K_ALT_X;       break;
                  case 'y':     result = K_ALT_Y;       break;
                  case 'z':     result = K_ALT_Z;       break;
                  default:
                    printf("3 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                    break;
                } /* switch */
              } else {
                printf("4 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (currentEvent.xkey.state & Mod2Mask) { /* Num Lock modifier */
          /* printf("Mod2Mask\n"); */
          switch (currentKey) {
            case XK_Return:     result = K_NL;          break;
            case XK_BackSpace:  result = K_BS;          break;
            case XK_Tab:        result = K_TAB;         break;
            case XK_Linefeed:   result = K_NL;          break;
            case XK_Escape:     result = K_ESC;         break;
            case XK_F1:         result = K_F1;          break;
            case XK_F2:         result = K_F2;          break;
            case XK_F3:         result = K_F3;          break;
            case XK_F4:         result = K_F4;          break;
            case XK_F5:         result = K_F5;          break;
            case XK_F6:         result = K_F6;          break;
            case XK_F7:         result = K_F7;          break;
            case XK_F8:         result = K_F8;          break;
            case XK_F9:         result = K_F9;          break;
            case XK_F10:        result = K_F10;         break;
            case XK_F11:        result = K_F11;         break;
            case XK_F12:        result = K_F12;         break;
            case XK_Left:       result = K_LEFT;        break;
            case XK_Right:      result = K_RIGHT;       break;
            case XK_Up:         result = K_UP;          break;
            case XK_Down:       result = K_DOWN;        break;
            case XK_Home:       result = K_HOME;        break;
            case XK_End:        result = K_END;         break;
            case XK_Prior:      result = K_PGUP;        break;
            case XK_Next:       result = K_PGDN;        break;
            case XK_Insert:     result = K_INS;         break;
            case XK_Delete:     result = K_DEL;         break;
            case XK_KP_Enter:   result = K_NL;          break;
            case XK_EuroSign:   result = 0x20ac;        break;
            case XK_Shift_L:
            case XK_Shift_R:
            case XK_Control_L:
            case XK_Control_R:
            case XK_Alt_L:
            case XK_Alt_R:
            case XK_Mode_switch:
            case XK_Caps_Lock:
            case XK_Num_Lock:
            case XK_Shift_Lock:
            case XK_ISO_Level3_Shift:
              result = gkbGetc(); break;
            default:
              if (lookup_count == 1) {
                result = buffer[0];
              } else {
                printf("5 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else {
          switch (currentKey) {
            case XK_Return:     result = K_NL;          break;
            case XK_BackSpace:  result = K_BS;          break;
            case XK_Tab:        result = K_TAB;         break;
            case XK_Linefeed:   result = K_NL;          break;
            case XK_Escape:     result = K_ESC;         break;
            case XK_F1:         result = K_F1;          break;
            case XK_F2:         result = K_F2;          break;
            case XK_F3:         result = K_F3;          break;
            case XK_F4:         result = K_F4;          break;
            case XK_F5:         result = K_F5;          break;
            case XK_F6:         result = K_F6;          break;
            case XK_F7:         result = K_F7;          break;
            case XK_F8:         result = K_F8;          break;
            case XK_F9:         result = K_F9;          break;
            case XK_F10:        result = K_F10;         break;
            case XK_F11:        result = K_F11;         break;
            case XK_F12:        result = K_F12;         break;
            case XK_Left:       result = K_LEFT;        break;
            case XK_Right:      result = K_RIGHT;       break;
            case XK_Up:         result = K_UP;          break;
            case XK_Down:       result = K_DOWN;        break;
            case XK_Home:       result = K_HOME;        break;
            case XK_End:        result = K_END;         break;
            case XK_Prior:      result = K_PGUP;        break;
            case XK_Next:       result = K_PGDN;        break;
            case XK_Insert:     result = K_INS;         break;
            case XK_Delete:     result = K_DEL;         break;
            case XK_KP_Left:    result = K_LEFT;        break;
            case XK_KP_Right:   result = K_RIGHT;       break;
            case XK_KP_Up:      result = K_UP;          break;
            case XK_KP_Down:    result = K_DOWN;        break;
            case XK_KP_Home:    result = K_HOME;        break;
            case XK_KP_End:     result = K_END;         break;
            case XK_KP_Prior:   result = K_PGUP;        break;
            case XK_KP_Next:    result = K_PGDN;        break;
            case XK_KP_Insert:  result = K_INS;         break;
            case XK_KP_Delete:  result = K_DEL;         break;
            case XK_KP_Begin:   result = K_PAD_CENTER;  break;
            case XK_KP_4:       result = K_LEFT;        break;
            case XK_KP_6:       result = K_RIGHT;       break;
            case XK_KP_8:       result = K_UP;          break;
            case XK_KP_2:       result = K_DOWN;        break;
            case XK_KP_7:       result = K_HOME;        break;
            case XK_KP_1:       result = K_END;         break;
            case XK_KP_9:       result = K_PGUP;        break;
            case XK_KP_3:       result = K_PGDN;        break;
            case XK_KP_0:       result = K_INS;         break;
            case XK_KP_5:       result = K_UNDEF;       break;
            case XK_KP_Enter:   result = K_NL;          break;
            case XK_KP_Decimal: result = K_DEL;         break;
            case XK_EuroSign:   result = 0x20ac;        break;
            case XK_Shift_L:
            case XK_Shift_R:
            case XK_Control_L:
            case XK_Control_R:
            case XK_Alt_L:
            case XK_Alt_R:
            case XK_Mode_switch:
            case XK_Caps_Lock:
            case XK_Num_Lock:
            case XK_Shift_Lock:
            case XK_ISO_Level3_Shift:
              result = gkbGetc(); break;
            default:
              if (lookup_count == 1) {
                result = buffer[0];
              } else {
                printf("6 undef key: %ld %lx\n", (long) currentKey, (long) currentKey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } /* if */
        break;
      default:
#ifdef FLAG_EVENTS
        printf("Other Event %d\n", currentEvent.type);
#endif
        result = gkbGetc();
        break;
    } /* switch */
#ifdef TRACE_KBD
    printf("end gkbGetc key: \"%s\" %ld %lx %d\n",
        buffer, (long) currentKey, (long) currentKey, result);
#endif
    return result;
  } /* gkbGetc */



#ifdef ANSI_C

booltype processEvents (void)
#else

booltype processEvents ()
#endif

  {
    KeySym currentKey;
    int num_events;
    int lookup_count;
    char buffer[21];
    booltype result;

  /* processEvents */
#ifdef TRACE_KBD
    printf("begin processEvents\n");
#endif
    result = FALSE;
    if (!eventPresent) {
      num_events = XEventsQueued(mydisplay, QueuedAfterReading);
      while (num_events != 0) {
        XNextEvent(mydisplay, &currentEvent);
        switch(currentEvent.type) {
          case Expose:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            handleExpose(&currentEvent.xexpose);
            result = TRUE;
            break;
#ifdef OUT_OF_ORDER
          case ConfigureNotify:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            configure(&currentEvent.xconfigure);
            result = TRUE;
            break;
          case MapNotify:
#endif
#ifdef TRACE_REPARENT_NOTIFY
          case ReparentNotify:
            printf("processEvents: Reparent\n");
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
          case ConfigureNotify:
            printf("processEvents: Configure %lx %d, %d\n",
                   currentEvent.xconfigure.window,
                   currentEvent.xconfigure.width, currentEvent.xconfigure.height);
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
#endif
#ifdef ALLOW_REPARENT_NOTIFY
          case ReparentNotify:
          case ConfigureNotify:
#endif
          case GraphicsExpose:
          case NoExpose:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
          case KeyPress:
            lookup_count = XLookupString(&currentEvent.xkey, buffer,
                                         20, &currentKey, 0);
            buffer[lookup_count] = '\0';
            switch (currentKey) {
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
              case XK_ISO_Level3_Shift:
                if (num_events == 1) {
                  num_events = XEventsQueued(mydisplay, QueuedAfterReading);
                } else {
                  num_events--;
                } /* if */
                break;
              default:
                /* printf("currentKey=%d\n", currentKey); */
                num_events = 0;
                eventPresent = TRUE;
                break;
            } /* switch */
            break;
          default:
            /* printf("currentEvent.type=%d\n", currentEvent.type); */
            num_events = 0;
            eventPresent = TRUE;
            break;
        } /* switch */
      } /* while */
    } /* if */
#ifdef TRACE_KBD
    printf("end processEvents ==> %d\n", result);
#endif
    return result;
  } /* processEvents */



#ifdef ANSI_C

booltype gkbKeyPressed (void)
#else

booltype gkbKeyPressed ()
#endif

  { /* gkbKeyPressed */
#ifdef TRACE_KBD
    printf("begin gkbKeyPressed\n");
#endif
    flushBeforeRead();
    processEvents();
#ifdef TRACE_KBD
    printf("end gkbKeyPressed ==> %d\n", eventPresent);
#endif
    return eventPresent;
  } /* gkbKeyPressed */



#ifdef ANSI_C

static booltype mouseButtonPressed (unsigned int button_mask)
#else

static booltype mouseButtonPressed (button_mask)
unsigned int button_mask;
#endif

  {
    Window root;
    Window child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int keys_buttons;
    booltype result;

  /* mouseButtonPressed */
    /* printf("mouseButtonPressed(%x)\n", button_mask); */
    XQueryPointer(mydisplay, DefaultRootWindow(mydisplay), &root, &child,
                  &root_x, &root_y, &win_x, &win_y, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, win_x, win_y, keys_buttons); */
    result = (keys_buttons & button_mask) != 0;
    /* printf("mouseButtonPressed(%d) ==> %d\n", button_mask, result); */
    return result;
  } /* mouseButtonPressed */



#ifdef ANSI_C

static booltype keyboardButtonPressed (KeySym sym)
#else

static booltype keyboardButtonPressed (sym)
KeySym sym;
#endif

  {
    char key_vector[32];
    KeyCode code;
    unsigned int byteindex;
    unsigned int bitindex;
    booltype result;

  /* keyboardButtonPressed */
    code = XKeysymToKeycode(mydisplay, sym);
    if (code == NoSymbol) {
      /* printf("keyboardButtonPressed: XKeysymToKeycode returns NoSymbol\n"); */
      result = 0;
    } else {
      byteindex = code >> 3;
      bitindex = code & 0x7;
      XQueryKeymap(mydisplay, key_vector);
      /*
        printf("sym=%d, code=%d, byteindex=%d, bitindex=%d\n", sym, code, byteindex, bitindex);
        for (byteindex = 0; byteindex < 32; byteindex++) {
          printf("%02x", key_vector[byteindex]);
        }
        printf("\n");
      */
      result = 1 & (key_vector[byteindex] >> bitindex);
    } /* if */
    return result;
  } /* keyboardButtonPressed */



#ifdef ANSI_C

booltype gkbButtonPressed (chartype button)
#else

booltype gkbButtonPressed (button)
chartype button;
#endif

  {
    unsigned int button_mask = 0;
    KeySym sym1;
    KeySym sym2 = 0;
    KeySym sym3 = 0;
    booltype okay = TRUE;
    booltype result;

  /* gkbButtonPressed */
    switch (button) {
      case K_CTL_A: case K_ALT_A: case 'A': case 'a': sym1 = 'A'; break;
      case K_CTL_B: case K_ALT_B: case 'B': case 'b': sym1 = 'B'; break;
      case K_CTL_C: case K_ALT_C: case 'C': case 'c': sym1 = 'C'; break;
      case K_CTL_D: case K_ALT_D: case 'D': case 'd': sym1 = 'D'; break;
      case K_CTL_E: case K_ALT_E: case 'E': case 'e': sym1 = 'E'; break;
      case K_CTL_F: case K_ALT_F: case 'F': case 'f': sym1 = 'F'; break;
      case K_CTL_G: case K_ALT_G: case 'G': case 'g': sym1 = 'G'; break;
      /* K_CTL_H */ case K_ALT_H: case 'H': case 'h': sym1 = 'H'; break;
      /* K_CTL_I */ case K_ALT_I: case 'I': case 'i': sym1 = 'I'; break;
      /* K_CTL_J */ case K_ALT_J: case 'J': case 'j': sym1 = 'J'; break;
      case K_CTL_K: case K_ALT_K: case 'K': case 'k': sym1 = 'K'; break;
      case K_CTL_L: case K_ALT_L: case 'L': case 'l': sym1 = 'L'; break;
      case K_CTL_M: case K_ALT_M: case 'M': case 'm': sym1 = 'M'; break;
      case K_CTL_N: case K_ALT_N: case 'N': case 'n': sym1 = 'N'; break;
      case K_CTL_O: case K_ALT_O: case 'O': case 'o': sym1 = 'O'; break;
      case K_CTL_P: case K_ALT_P: case 'P': case 'p': sym1 = 'P'; break;
      case K_CTL_Q: case K_ALT_Q: case 'Q': case 'q': sym1 = 'Q'; break;
      case K_CTL_R: case K_ALT_R: case 'R': case 'r': sym1 = 'R'; break;
      case K_CTL_S: case K_ALT_S: case 'S': case 's': sym1 = 'S'; break;
      case K_CTL_T: case K_ALT_T: case 'T': case 't': sym1 = 'T'; break;
      case K_CTL_U: case K_ALT_U: case 'U': case 'u': sym1 = 'U'; break;
      case K_CTL_V: case K_ALT_V: case 'V': case 'v': sym1 = 'V'; break;
      case K_CTL_W: case K_ALT_W: case 'W': case 'w': sym1 = 'W'; break;
      case K_CTL_X: case K_ALT_X: case 'X': case 'x': sym1 = 'X'; break;
      case K_CTL_Y: case K_ALT_Y: case 'Y': case 'y': sym1 = 'Y'; break;
      case K_CTL_Z: case K_ALT_Z: case 'Z': case 'z': sym1 = 'Z'; break;

      case K_ALT_0: case '0': sym1 = '0'; sym2 = XK_KP_0; break;
      case K_ALT_1: case '1': sym1 = '1'; sym2 = XK_KP_1; break;
      case K_ALT_2: case '2': sym1 = '2'; sym2 = XK_KP_2; break;
      case K_ALT_3: case '3': sym1 = '3'; sym2 = XK_KP_3; break;
      case K_ALT_4: case '4': sym1 = '4'; sym2 = XK_KP_4; break;
      case K_ALT_5: case '5': sym1 = '5'; sym2 = XK_KP_5; break;
      case K_ALT_6: case '6': sym1 = '6'; sym2 = XK_KP_6; break;
      case K_ALT_7: case '7': sym1 = '7'; sym2 = XK_KP_7; break;
      case K_ALT_8: case '8': sym1 = '8'; sym2 = XK_KP_8; break;
      case K_ALT_9: case '9': sym1 = '9'; sym2 = XK_KP_9; break;

      case K_F1:  case K_SFT_F1:  case K_CTL_F1:  case K_ALT_F1:  sym1 = XK_F1;  break;
      case K_F2:  case K_SFT_F2:  case K_CTL_F2:  case K_ALT_F2:  sym1 = XK_F2;  break;
      case K_F3:  case K_SFT_F3:  case K_CTL_F3:  case K_ALT_F3:  sym1 = XK_F3;  break;
      case K_F4:  case K_SFT_F4:  case K_CTL_F4:  case K_ALT_F4:  sym1 = XK_F4;  break;
      case K_F5:  case K_SFT_F5:  case K_CTL_F5:  case K_ALT_F5:  sym1 = XK_F5;  break;
      case K_F6:  case K_SFT_F6:  case K_CTL_F6:  case K_ALT_F6:  sym1 = XK_F6;  break;
      case K_F7:  case K_SFT_F7:  case K_CTL_F7:  case K_ALT_F7:  sym1 = XK_F7;  break;
      case K_F8:  case K_SFT_F8:  case K_CTL_F8:  case K_ALT_F8:  sym1 = XK_F8;  break;
      case K_F9:  case K_SFT_F9:  case K_CTL_F9:  case K_ALT_F9:  sym1 = XK_F9;  break;
      case K_F10: case K_SFT_F10: case K_CTL_F10: case K_ALT_F10: sym1 = XK_F10; break;
      case K_F11: case K_SFT_F11: case K_CTL_F11: case K_ALT_F11: sym1 = XK_F11; break;
      case K_F12: case K_SFT_F12: case K_CTL_F12: case K_ALT_F12: sym1 = XK_F12; break;

      case K_LEFT:  case K_CTL_LEFT:  sym1 = XK_Left;   sym2 = XK_KP_Left;   break;
      case K_RIGHT: case K_CTL_RIGHT: sym1 = XK_Right;  sym2 = XK_KP_Right;  break;
      case K_UP:    case K_CTL_UP:    sym1 = XK_Up;     sym2 = XK_KP_Up;     break;
      case K_DOWN:  case K_CTL_DOWN:  sym1 = XK_Down;   sym2 = XK_KP_Down;   break;
      case K_HOME:  case K_CTL_HOME:  sym1 = XK_Home;   sym2 = XK_KP_Home;   break;
      case K_END:   case K_CTL_END:   sym1 = XK_End;    sym2 = XK_KP_End;    break;
      case K_PGUP:  case K_CTL_PGUP:  sym1 = XK_Prior;  sym2 = XK_KP_Prior;  break;
      case K_PGDN:  case K_CTL_PGDN:  sym1 = XK_Next;   sym2 = XK_KP_Next;   break;
      case K_INS:   case K_CTL_INS:   sym1 = XK_Insert; sym2 = XK_KP_Insert; break;
      case K_DEL:   case K_CTL_DEL:   sym1 = XK_Delete; sym2 = XK_KP_Delete; break;

      case K_ESC:                 sym1 = XK_Escape;                break;
      case K_PAD_CENTER:          sym1 = XK_KP_Begin;              break;
      case K_BS:                  sym1 = XK_BackSpace; sym2 = 'H'; break;
      case K_NL:                  sym1 = XK_Return;    sym2 = 'J'; sym3 = XK_KP_Enter; break;
      case K_TAB: case K_BACKTAB: sym1 = XK_Tab;       sym2 = 'I'; break;

      case '*': sym1 = '*'; sym2 = XK_KP_Multiply; break;
      case '+': sym1 = '+'; sym2 = XK_KP_Add;      break;
      case '-': sym1 = '-'; sym2 = XK_KP_Subtract; break;
      case '/': sym1 = '/'; sym2 = XK_KP_Divide;   break;

      case ' ':  case '!':  case '\"': case '#':  case '$':
      case '%':  case '&':  case '\'': case '(':  case ')':
      case ',':  case '.':  case ':':  case ';':  case '<':
      case '=':  case '>':  case '?':  case '@':  case '[':
      case '\\': case ']':  case '^':  case '_':  case '`':
      case '{':  case '|':  case '}':  case '~':
        sym1 = (KeySym) button; break;

      /*
      case K_SCRLUP:
      case K_SCRLDN:
      case K_INSLN:
      case K_DELLN:
      case K_ERASE:
      case K_CTL_NL:
      case K_NULLCMD:
      case K_REDRAW:
      case K_NEWWINDOW:
      case K_UNDEF:
      case K_NONE:

      case K_NULCHAR:
      */

      case K_MOUSE1: button_mask = Button1Mask; break;
      case K_MOUSE2: button_mask = Button2Mask; break;
      case K_MOUSE3: button_mask = Button3Mask; break;
      case K_MOUSE4: button_mask = Button4Mask; break;
      case K_MOUSE5: button_mask = Button5Mask; break;

      default: result = FALSE; okay = FALSE; break;
    } /* switch */

    if (okay) {
      if (button_mask != 0) {
        result = mouseButtonPressed(button_mask);
      } else {
        result = keyboardButtonPressed(sym1);
        if (!result && sym2 != 0) {
          result = keyboardButtonPressed(sym2);
          if (!result && sym3 != 0) {
            result = keyboardButtonPressed(sym3);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_KBD
    printf("gkbButtonPressed -> %d\n", result);
#endif
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

inttype gkbButtonXpos (void)
#else

inttype gkbButtonXpos ()
#endif

  { /* gkbButtonXpos */
#ifdef TRACE_KBD
    printf("gkbButtonXpos -> %d\n", button_x);
#endif
    return button_x;
  } /* gkbButtonXpos */



#ifdef ANSI_C

inttype gkbButtonYpos (void)
#else

inttype gkbButtonYpos ()
#endif

  { /* gkbButtonYpos */
#ifdef TRACE_KBD
    printf("gkbButtonYpos -> %d\n", button_y);
#endif
    return button_y;
  } /* gkbButtonYpos */



#ifdef ANSI_C

wintype gkbWindow (void)
#else

wintype gkbWindow ()
#endif

  {
    wintype result;

  /* gkbWindow */
    result = find_window(button_window);
    if (result != NULL) {
      result->usage_count++;
    } /* if */
#ifdef TRACE_KBD
    printf("gkbWindow -> %lu\n", result);
#endif
    return result;
  } /* gkbWindow */



#ifdef ANSI_C

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
/*  printf("drwFlush()\n"); */
    processEvents();
    do {
      doFlush();
    } while (processEvents());
  } /* drwFlush */
