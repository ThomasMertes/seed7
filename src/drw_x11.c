/********************************************************************/
/*                                                                  */
/*  drw_x11.c     Graphic access using the X11 capabilitys.         */
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
/*  File: seed7/src/drw_x11.c                                       */
/*  Changes: 1994, 2000 - 2007  Thomas Mertes                       */
/*  Content: Graphic access using the X11 capabilitys.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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
#include "heaputl.h"
#include "striutl.h"
#include "kbd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"

#undef TRACE_X11

#undef FLAG_EVENTS
#undef TRACE_KBD

#ifndef C_PLUS_PLUS
#define c_class class
#endif


#define PI 3.14159265358979323846264338327950284197

Display *mydisplay = NULL;
Window mywindow;
unsigned long myforeground, mybackground;
GC mygc;
XEvent myevent;
KeySym mykey;
XSizeHints myhint;
XWMHints mywmhint;
int myscreen;
static inttype button_x = 0;
static inttype button_y = 0;
static Window button_window = 0;

typedef struct x11_winstruct {
  uinttype usage_count;
  Window window;
  Pixmap backup;
  Pixmap clip_mask;
  booltype is_pixmap;
  unsigned int width;
  unsigned int height;
  struct x11_winstruct *next;
} x11_winrecord, *x11_wintype;

typedef const struct x11_winstruct *const_x11_wintype;

static x11_wintype window_list = NULL;

#define to_window(win)    (((const_x11_wintype) win)->window)
#define to_backup(win)    (((const_x11_wintype) win)->backup)
#define to_clip_mask(win) (((const_x11_wintype) win)->clip_mask)
#define is_pixmap(win)    (((const_x11_wintype) win)->is_pixmap)
#define to_width(win)     (((const_x11_wintype) win)->width)
#define to_height(win)    (((const_x11_wintype) win)->height)

#define to_var_window(win)    (((x11_wintype) win)->window)
#define to_var_backup(win)    (((x11_wintype) win)->backup)
#define to_var_clip_mask(win) (((x11_wintype) win)->clip_mask)
#define is_var_pixmap(win)    (((x11_wintype) win)->is_pixmap)
#define to_var_width(win)     (((x11_wintype) win)->width)
#define to_var_height(win)    (((x11_wintype) win)->height)

Visual *default_visual;

int lshift_red;
int rshift_red;
int lshift_green;
int rshift_green;
int lshift_blue;
int rshift_blue;

typedef struct color_entry {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned long pixel;
  struct color_entry *red_less;
  struct color_entry *red_greater;
  struct color_entry *green_less;
  struct color_entry *green_greater;
  struct color_entry *blue_less;
  struct color_entry *blue_greater;
} *color_entry_type;

static const int highest_bit[16] = {
  0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4
};



#ifdef ANSI_C

static x11_wintype find_window (Window curr_window)
#else

static x11_wintype find_window (curr_window)
Window curr_window;
#endif

  {
    x11_wintype window;

  /* find_window */
    window = window_list;
    while (window != NULL) {
      if (to_window(window) == curr_window) {
        return(window);
      } /* if */
      window = window->next;
    } /* while */
    return(NULL);
  } /* find_window */



#ifdef ANSI_C

static void remove_window (x11_wintype curr_window)
#else

static void remove_window (curr_window)
x11_wintype curr_window;
#endif

  {
    x11_wintype *win_addr;
    x11_wintype window;

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

void redraw (XExposeEvent *xexpose)
#else

void redraw (xexpose)
XExposeEvent *xexpose;
#endif

  {
    x11_wintype expose_window;

  /* redraw */
#ifdef TRACE_X11
    printf("begin redraw\n");
#endif
    if (xexpose->count == 0) {
      expose_window = find_window(xexpose->window);
      if (expose_window != NULL && expose_window->backup != 0) {
        /* printf("XExposeEvent x=%d, y=%d, width=%d, height=%d\n",
            xexpose->x, xexpose->y, xexpose->width, xexpose->height); */
        XCopyArea(mydisplay, expose_window->backup,
            expose_window->window, mygc, 0, 0,
            to_width(expose_window), to_height(expose_window), 0, 0);
        /* XFlush(mydisplay);
            XSync(mydisplay, 0); */
      } /* if */
    } /* if */
#ifdef TRACE_X11
    printf("end redraw\n");
#endif
  } /* redraw */



#ifdef ANSI_C

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  {
    int lookup_count;
    unsigned char buffer[21];
    chartype result;

  /* gkbGetc */
#ifdef TRACE_KBD
    printf("begin gkbGetc\n");
#endif
    result = K_NONE;
    do {
      XNextEvent(mydisplay, &myevent);
    } while (myevent.type == KeyRelease);
    switch(myevent.type) {
      case Expose:
#ifdef FLAG_EVENTS
        printf("Expose\n");
#endif
        redraw(&myevent.xexpose);
        result = gkbGetc();
        break;

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
        XRefreshKeyboardMapping(&myevent.xmapping);
        break;

      case ButtonPress:
#ifdef FLAG_EVENTS
        printf("ButtonPress (%d, %d, %u)\n",
            myevent.xbutton.x, myevent.xbutton.y, myevent.xbutton.button);
#endif
        button_x = myevent.xbutton.x;
        button_y = myevent.xbutton.y;
        button_window = myevent.xbutton.window;
        if (myevent.xbutton.button >= 1 && myevent.xbutton.button <= 5) {
          result = myevent.xbutton.button + K_MOUSE1 - 1;
        } else {
          result = K_UNDEF;
        } /* if */
        break;

      case KeyPress:
#ifdef FLAG_EVENTS
        printf("KeyPress\n");
        printf("xkey.state (%o)\n", myevent.xkey.state);
#endif
        lookup_count = XLookupString(&myevent.xkey, (cstritype) buffer, 20, &mykey, 0);
        buffer[lookup_count] = '\0';
        if (myevent.xkey.state & ShiftMask) {
          /* printf("ShiftMask\n"); */
          switch (mykey) {
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
                printf("1 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (myevent.xkey.state & ControlMask) {
          /* printf("ControlMask\n"); */
          switch (mykey) {
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
                printf("2 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (myevent.xkey.state & Mod1Mask) { /* Left ALT modifier */
          /* printf("Mod1Mask\n"); */
          switch (mykey) {
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
                    printf("3 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                    break;
                } /* switch */
              } else {
                printf("4 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (myevent.xkey.state & Mod2Mask) { /* Num Lock modifier */
          /* printf("Mod2Mask\n"); */
          switch (mykey) {
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
            case XK_EuroSign:   result = mykey;         break;
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
                printf("5 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else {
          switch (mykey) {
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
            case XK_EuroSign:   result = mykey;         break;
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
                printf("6 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } /* if */
        break;
      default:
        printf("Other Event %d\n", myevent.type);
        break;
    } /* switch */
#ifdef TRACE_KBD
    printf("end gkbGetc key: \"%s\" %ld %lx %d\n",
        buffer, (long) mykey, (long) mykey, result);
#endif
    return(result);
  } /* gkbGetc */



#ifdef ANSI_C

booltype gkbKeyPressed (void)
#else

booltype gkbKeyPressed ()
#endif

  {
    int num_events;
    int lookup_count;
    char buffer[21];
    booltype result;

  /* gkbKeyPressed */
#ifdef TRACE_KBD
    printf("begin gkbKeyPressed\n");
#endif
    result = FALSE;
    num_events = XEventsQueued(mydisplay, QueuedAfterReading);
    while (num_events != 0) {
      XPeekEvent(mydisplay, &myevent);
      switch(myevent.type) {
        case Expose:
          XNextEvent(mydisplay, &myevent);
          num_events = XEventsQueued(mydisplay, QueuedAfterReading);
          redraw(&myevent.xexpose);
          break;
        case GraphicsExpose:
        case NoExpose:
          XNextEvent(mydisplay, &myevent);
          num_events = XEventsQueued(mydisplay, QueuedAfterReading);
          break;
        case KeyPress:
          lookup_count = XLookupString(&myevent.xkey, buffer, 20, &mykey, 0);
          buffer[lookup_count] = '\0';
          switch (mykey) {
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
              XNextEvent(mydisplay, &myevent);
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
              break;
            default:
              num_events = 0;
              result = TRUE;
              break;
          } /* switch */
          break;
        default:
          num_events = 0;
          result = TRUE;
          break;
      } /* switch */
    } /* while */
    if (result && XEventsQueued(mydisplay, QueuedAfterReading) == 0) {
      printf("problem");
    } /* if */
#ifdef TRACE_KBD
    printf("end gkbKeyPressed ==> %d\n", result);
#endif
    return(result);
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
    return(result);
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
    return(result);
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
    return(result);
  } /* gkbButtonPressed */



#ifdef ANSI_C

chartype gkbRawGetc (void)
#else

chartype gkbRawGetc ()
#endif

  { /* gkbRawGetc */
    return(gkbGetc());
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
    return(result);
  } /* gkbWindow */



#ifdef ANSI_C

inttype gkbButtonXpos (void)
#else

inttype gkbButtonXpos ()
#endif

  { /* gkbButtonXpos */
    return(button_x);
  } /* gkbButtonXpos */



#ifdef ANSI_C

inttype gkbButtonYpos (void)
#else

inttype gkbButtonYpos ()
#endif

  { /* gkbButtonYpos */
    return(button_y);
  } /* gkbButtonYpos */



#ifdef ANSI_C

int get_highest_bit (unsigned long number)
#else

int get_highest_bit (number)
unsigned long number;
#endif

  {
    int result = 0;

  /* get_highest_bit */
    if (number & 0xffff0000) {
        number >>= 16;
        result += 16;
    } /* if */
    if (number & 0xff00) {
        number >>= 8;
        result += 8;
    } /* if */
    if (number & 0xf0) {
        number >>= 4;
        result += 4;
    } /* if */
    return(result + highest_bit[number]);
  } /* get_highest_bit */



#ifdef ANSI_C

inttype drwPointerXpos (const_wintype actual_window)
#else

inttype drwPointerXpos (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    Window child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int keys_buttons;

  /* drwPointerXpos */
    XQueryPointer(mydisplay, to_window(actual_window), &root, &child,
                  &root_x, &root_y, &win_x, &win_y, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, win_x, win_y, keys_buttons); */
    /* printf("drwPointerXpos ==> %ld\n", win_x); */
    return(win_x);
  } /* drwPointerXpos */



#ifdef ANSI_C

inttype drwPointerYpos (const_wintype actual_window)
#else

inttype drwPointerYpos (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    Window child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int keys_buttons;

  /* drwPointerYpos */
    XQueryPointer(mydisplay, to_window(actual_window), &root, &child,
                  &root_x, &root_y, &win_x, &win_y, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, win_x, win_y, keys_buttons); */
    /* printf("drwPointerYpos ==> %ld\n", win_y); */
    return(win_y);
  } /* drwPointerYpos */



#ifdef ANSI_C

void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwArc (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  {
    int angle1, angle2;

  /* drwArc */
#ifdef TRACE_X11
    printf("arc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, ang1, ang2);
#endif
    angle1 = (int) (ang1 * (23040.0 / (2 * PI)));
    angle2 = (int) (ang2 * (23040.0 / (2 * PI)));
/*  printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2); */
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          angle1, angle2);
    } /* if */
  } /* drwArc */



#ifdef ANSI_C

void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPArc (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  {
    int angle1, angle2;

  /* drwPArc */
#ifdef TRACE_X11
    printf("arc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, ang1, ang2);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    angle1 = (int) (ang1 * (23040.0 / (2 * PI)));
    angle2 = (int) (ang2 * (23040.0 / (2 * PI)));
/*  printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2); */
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          angle1, angle2);
    } /* if */
  } /* drwPArc */



#ifdef ANSI_C

void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcChord (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  {
    int angle1, angle2;

  /* drwFArcChord */
#ifdef TRACE_X11
    printf("arcchord(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, ang1, ang2);
#endif
    angle1 = (int) (ang1 * (23040.0 / (2 * PI)));
    angle2 = (int) (ang2 * (23040.0 / (2 * PI)));
/*  printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2); */
    XSetArcMode(mydisplay, mygc, ArcChord);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          angle1, angle2);
    } /* if */
  } /* drwFArcChord */



#ifdef ANSI_C

void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcChord (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  {
    int angle1, angle2;

  /* drwPFArcChord */
#ifdef TRACE_X11
    printf("arcchord(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, ang1, ang2);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    angle1 = (int) (ang1 * (23040.0 / (2 * PI)));
    angle2 = (int) (ang2 * (23040.0 / (2 * PI)));
/*  printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2); */
    XSetArcMode(mydisplay, mygc, ArcChord);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        angle1, angle2);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          angle1, angle2);
    } /* if */
  } /* drwPFArcChord */



#ifdef ANSI_C

void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcPieSlice (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwFArcPieSlice */
/*  printf("arc(%d, %d, %d, %.4f, %.4f)\n", x, y, radius, ang1, ang2);
    printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        (int) (ang1 * (23040.0 / (2 * PI))), (int) (ang2 * (23040.0 / (2 * PI)))); */
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        (int) (ang1 * (23040.0 / (2 * PI))), (int) (ang2 * (23040.0 / (2 * PI))));
    /* XDrawLine(mydisplay, to_window(actual_window), mygc, x, y, x, y); */
  } /* drwFArcPieSlice */



#ifdef ANSI_C

void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcPieSlice (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPFArcPieSlice */
/*  printf("arc(%d, %d, %d, %.4f, %.4f)\n", x, y, radius, ang1, ang2);
    printf("XDrawArc(*, %d, %d, %d, %d, %d, %d)\n",
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        (int) (ang1 * (23040.0 / (2 * PI))), (int) (ang2 * (23040.0 / (2 * PI)))); */
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        (int) (ang1 * (23040.0 / (2 * PI))), (int) (ang2 * (23040.0 / (2 * PI))));
    /* XDrawLine(mydisplay, to_window(actual_window), mygc, x, y, x, y); */
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
/*  printf("arc2(%d, %d, %d, %d)\n", x1, y1, radius); */
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x1 - radius, y1 - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XDrawLine(mydisplay, to_window(actual_window), mygc, x1, y1, x2, y2);
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
#ifdef TRACE_X11
    printf("circle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
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

  { /* drwPCircle */
#ifdef TRACE_X11
    printf("fcircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwPCircle */



#ifdef ANSI_C

void drwClear (const_wintype actual_window, inttype col)
#else

void drwClear (actual_window, col)
wintype actual_window;
inttype col;
#endif

  { /* drwClear */
#ifdef TRACE_X11
    printf("drwClear(%lu, %08lx)\n", actual_window, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillRectangle(mydisplay, to_window(actual_window), mygc, 0, 0,
        to_width(actual_window), to_height(actual_window));
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, 0, 0,
          to_width(actual_window), to_height(actual_window));
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
#ifdef TRACE_X11
    printf("XCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
        src_window, dest_window, src_x, src_y, width, height, dest_x, dest_y);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else if (to_backup(src_window) != 0) {
      XCopyArea(mydisplay, to_backup(src_window), to_window(dest_window),
          mygc, src_x, src_y, (unsigned int) width, (unsigned int) height,
          dest_x, dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_backup(src_window), to_backup(dest_window),
            mygc, src_x, src_y, (unsigned int) width, (unsigned int) height,
            dest_x, dest_y);
      } /* if */
    } else {
      XCopyArea(mydisplay, to_window(src_window), to_window(dest_window),
          mygc, src_x, src_y, (unsigned int) width, (unsigned int) height,
          dest_x, dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_window(src_window), to_backup(dest_window),
            mygc, src_x, src_y, (unsigned int) width, (unsigned int) height,
            dest_x, dest_y);
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
#ifdef TRACE_X11
    printf("fcircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
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

  { /* drwPFCircle */
#ifdef TRACE_X11
    printf("fcircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
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
#ifdef TRACE_X11
    printf("fellipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XFillArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
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

  { /* drwPFEllipse */
#ifdef TRACE_X11
    printf("fellipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned) col);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
  } /* drwPFEllipse */



#ifdef ANSI_C

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
/*  printf("Xflush(%d, %d, %d, %d)\n", x, y, radius); */
    gkbKeyPressed();
#ifdef OUT_OF_ORDER
    if (XEventsQueued(mydisplay, QueuedAfterReading) > 0) {
      printf("drwFlush(%d)\n", XEventsQueued(mydisplay, QueuedAfterReading));
      while (XEventsQueued(mydisplay, QueuedAfterReading) > 0) {
        XNextEvent(mydisplay, &myevent);
        switch(myevent.type) {
          case Expose:
            printf("Expose\n");
            break;
          case GraphicsExpose:
            printf("GraphicsExpose\n");
            break;
          case NoExpose:
            printf("NoExpose\n");
            break;
          case MappingNotify:
            printf("MappingNotify\n");
            break;
          case ButtonPress:
            printf("ButtonPress (%d, %d, %u)\n",
                myevent.xbutton.x, myevent.xbutton.y, myevent.xbutton.button);
            break;
          case KeyPress:
            printf("KeyPress\n");
            printf("xkey.state (%o)\n", myevent.xkey.state);
            break;
          default:
            printf("Other Event %d\n", myevent.type);
            break;
        } /* switch */
      } /* while */
    } /* if */
#endif
    XFlush(mydisplay);
    XSync(mydisplay, 0);
  } /* drwFlush */



#ifdef ANSI_C

void drwFree (wintype old_window)
#else

void drwFree (old_window)
wintype old_window;
#endif

  { /* drwFree */
#ifdef TRACE_X11
    printf("drwFree(%lu)\n", old_window);
#endif
    if (is_pixmap(old_window)) {
      XFreePixmap(mydisplay, to_window(old_window));
    } else {
      XDestroyWindow(mydisplay, to_window(old_window));
      remove_window((x11_wintype) old_window);
    } /* if */
    FREE_RECORD(old_window, x11_winrecord, count.win);
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
    x11_wintype result;

  /* drwGet */
#ifdef TRACE_X11
    printf("drwGet(%lu, %ld, %ld, %ld, %ld)\n", actual_window, left, upper, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (!ALLOC_RECORD(result, x11_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), (unsigned int) width, (unsigned int) height,
          DefaultDepth(mydisplay, myscreen));
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
      result->next = NULL;
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_backup(actual_window),
            result->window, mygc, left, upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } else {
        XCopyArea(mydisplay, to_window(actual_window),
            result->window, mygc, left, upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } /* if */
      /* printf("XCopyArea(%ld, %ld, %ld, %ld)\n", left, upper, width, height); */
    } /* if */
    return((wintype) result);
  } /* drwGet */



#ifdef ANSI_C

inttype drwHeight (const_wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    Status status;

  /* drwHeight */
    status = XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth);
    return((inttype) height);
  } /* drwHeight */



#ifdef ANSI_C

wintype drwImage (const_wintype actual_window, inttype *image_data,
    inttype width, inttype height)
#else

wintype drwImage (actual_window, image_data, width, height)
wintype actual_window;
inttype *image_data;
inttype width;
inttype height;
#endif

  {
    XImage *image;
    x11_wintype result;

  /* drwImage */
#ifdef TRACE_X11
    printf("image(%ld, %ld)\n", width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      image = XCreateImage(mydisplay, default_visual,
          DefaultDepth(mydisplay, myscreen), ZPixmap, 0, (char *) image_data,
          (unsigned int) width, (unsigned int) height, 32, 0);
      if (image == NULL) {
        result = NULL;
      } else {
        if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
          memset(result, 0, sizeof(struct x11_winstruct));
          result->usage_count = 1;
          result->window = XCreatePixmap(mydisplay,
              to_window(actual_window), (unsigned int) width, (unsigned int) height,
              DefaultDepth(mydisplay, myscreen));
          result->backup = 0;
          result->clip_mask = 0;
          result->is_pixmap = TRUE;
          result->width = (unsigned int) width;
          result->height = (unsigned int) height;
          result->next = NULL;
          XPutImage(mydisplay, result->window, mygc, image, 0, 0, 0, 0,
              (unsigned int) width, (unsigned int) height);
        } /* if */
        XFree(image);
      } /* if */
    } /* if */
    return((wintype) result);
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
#ifdef TRACE_X11
    printf("line(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x1, y1, x2, y2);
#endif
    XDrawLine(mydisplay, to_window(actual_window), mygc, x1, y1, x2, y2);
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc, x1, y1, x2, y2);
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

  { /* drwPLine */
#ifdef TRACE_X11
    printf("drwPLine(%lu, %ld, %ld, %ld, %ld, %08lx)\n", actual_window, x1, y1, x2, y2, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawLine(mydisplay, to_window(actual_window), mygc, x1, y1, x2, y2);
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc, x1, y1, x2, y2);
    } /* if */
  } /* drwPLine */



#ifdef ANSI_C

wintype drwNewPixmap (const_wintype actual_window, inttype width, inttype height)
#else

wintype drwNewPixmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  {
    x11_wintype result;

  /* drwNewPixmap */
#ifdef TRACE_X11
    printf("drwNewPixmap(%ld, %ld)\n", width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (!ALLOC_RECORD(result, x11_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), (unsigned int) width, (unsigned int) height,
          DefaultDepth(mydisplay, myscreen));
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
      result->next = NULL;
    } /* if */
    return((wintype) result);
  } /* drwNewPixmap */



#ifdef ANSI_C

wintype drwNewBitmap (const_wintype actual_window, inttype width, inttype height)
#else

wintype drwNewBitmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  {
    x11_wintype result;

  /* drwNewBitmap */
#ifdef TRACE_X11
    printf("drwNewBitmap(%ld, %ld)\n", width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (!ALLOC_RECORD(result, x11_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), (unsigned int) width, (unsigned int) height, 1);
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
      result->next = NULL;
    } /* if */
    return((wintype) result);
  } /* drwNewBitmap */



#ifdef ANSI_C

static void dra_init (void)
#else

static void dra_init ()
#endif

  {
    const_cstritype class_text;

  /* dra_init */
#ifdef TRACE_X11
    printf("BEGIN dra_init()\n");
#endif
    /* When linking with a profiling standard library XOpenDisplay */
    /* deadlocked. Be careful to avoid this situation.             */
    mydisplay = XOpenDisplay("");
    /* printf("mydisplay = %lu\n", (long unsigned) mydisplay); */
    myscreen = DefaultScreen(mydisplay);
    /* printf("myscreen = %lu\n", (long unsigned) myscreen); */

    default_visual = XDefaultVisual(mydisplay, myscreen);
    if (default_visual->c_class == PseudoColor) {
      class_text = "PseudoColor";
    } else if (default_visual->c_class == DirectColor) {
      class_text = "DirectColor";
    } else if (default_visual->c_class == GrayScale) {
      class_text = "GrayScale";
    } else if (default_visual->c_class == StaticColor) {
      class_text = "StaticColor";
    } else if (default_visual->c_class == TrueColor) {
      class_text = "TrueColor";
    } else if (default_visual->c_class == StaticGray) {
      class_text = "StaticGray";
    } else {
      class_text = "unknown";
    } /* if */
#ifdef OUT_OF_ORDER
    printf("visualid:     %lX\n", (unsigned long) default_visual->visualid);
    printf("class:        %s\n",  class_text);
    printf("red_mask:     %08lx\n", default_visual->red_mask);
    printf("green_mask:   %08lx\n", default_visual->green_mask);
    printf("blue_mask:    %08lx\n", default_visual->blue_mask);
    printf("bits_per_rgb: %d\n",  default_visual->bits_per_rgb);
    printf("map_entries:  %d\n",  default_visual->map_entries);

    /* printf("extension:     %lX\n", (unsigned long) default_visual->extension); */

    printf("highest red bit:   %d\n", get_highest_bit(default_visual->red_mask));
    printf("highest green bit: %d\n", get_highest_bit(default_visual->green_mask));
    printf("highest blue bit:  %d\n", get_highest_bit(default_visual->blue_mask));
#endif
    lshift_red   = get_highest_bit(default_visual->red_mask) - 16;
    rshift_red   = -lshift_red;
    lshift_green = get_highest_bit(default_visual->green_mask) - 16;
    rshift_green = -lshift_green;
    lshift_blue  = get_highest_bit(default_visual->blue_mask) - 16;
    rshift_blue  = -lshift_blue;

    lshift_red   = lshift_red   < 0 ? 0 : lshift_red;
    rshift_red   = rshift_red   < 0 ? 0 : rshift_red;
    lshift_green = lshift_green < 0 ? 0 : lshift_green;
    rshift_green = rshift_green < 0 ? 0 : rshift_green;
    lshift_blue  = lshift_blue  < 0 ? 0 : lshift_blue;
    rshift_blue  = rshift_blue  < 0 ? 0 : rshift_blue;
#ifdef OUT_OF_ORDER
    printf("lshift_red:   %d\n", lshift_red);
    printf("rshift_red:   %d\n", rshift_red);
    printf("lshift_green: %d\n", lshift_green);
    printf("rshift_green: %d\n", rshift_green);
    printf("lshift_blue:  %d\n", lshift_blue);
    printf("rshift_blue:  %d\n", rshift_blue);
#endif
    mybackground = WhitePixel(mydisplay, myscreen);
    myforeground = BlackPixel(mydisplay, myscreen);
#ifdef TRACE_X11
    printf("END dra_init\n");
#endif
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
    unsigned long valuemask;
    Screen *x_screen;
    XSetWindowAttributes attributes;
    XWindowAttributes attribs;
    x11_wintype result;

  /* drwOpen */
#ifdef TRACE_X11
    printf("BEGIN drwOpen(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
#ifdef DO_HEAP_STATISTIC
    count.size_winrecord = SIZ_REC(x11_winrecord);
#endif
    result = NULL;
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay != NULL) {
        win_name = cp_to_cstri(window_name);
        if (win_name == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
            memset(result, 0, sizeof(struct x11_winstruct));
            result->usage_count = 1;
            result->next = window_list;
            window_list = result;

            myhint.x = xPos;
            myhint.y = yPos;
            myhint.width = width;
            myhint.height = height;
            myhint.flags = PPosition | PSize;
            mywmhint.flags = InputHint;
            mywmhint.input = True;

            result->window = XCreateSimpleWindow(mydisplay,
                DefaultRootWindow(mydisplay),
                myhint.x, myhint.y, (unsigned) myhint.width, (unsigned) myhint.height,
                5, myforeground, mybackground);

            result->backup = 0;
            result->clip_mask = 0;
            result->is_pixmap = FALSE;
            result->width = (unsigned int) width;
            result->height = (unsigned int) height;

            XSetStandardProperties(mydisplay, result->window,
                win_name, win_name,
                None, /* argv, argc, */ NULL, 0, &myhint);
            XSetWMHints(mydisplay, result->window, &mywmhint);

            x_screen = XScreenOfDisplay(mydisplay, myscreen);
            /* printf("backing_store=%d (NotUseful=%d/WhenMapped=%d/Always=%d)\n",
                x_screen->backing_store, NotUseful, WhenMapped, Always); */
            if (x_screen->backing_store != NotUseful) {
              valuemask = CWBackingStore;
              attributes.backing_store = Always;
              XChangeWindowAttributes(mydisplay, result->window, valuemask, &attributes);

              /* printf("backing_store=%d %d\n", attributes.backing_store, Always); */
              XGetWindowAttributes(mydisplay, result->window, &attribs);
              /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
              if (attribs.backing_store != Always) {
                result->backup = XCreatePixmap(mydisplay, result->window,
                    (unsigned int) width, (unsigned int) height,
                    DefaultDepth(mydisplay, myscreen));
                /* printf("backup=%lu\n", (long unsigned) result->backup); */
              } /* if */
            } else {
              result->backup = XCreatePixmap(mydisplay, result->window,
                  (unsigned int) width, (unsigned int) height,
                  DefaultDepth(mydisplay, myscreen));
              /* printf("backup=%lu\n", (long unsigned) result->backup); */
            } /* if */

            mygc = XCreateGC(mydisplay, result->window, 0, 0);
            XSetBackground(mydisplay, mygc, mybackground);
            XSetForeground(mydisplay, mygc, myforeground);

            XSelectInput(mydisplay, result->window,
                ButtonPressMask | KeyPressMask | ExposureMask);

            XMapRaised(mydisplay, result->window);
            drwClear((wintype) result, (inttype) myforeground);
          } /* if */
          free_cstri(win_name, window_name);
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
#ifdef TRACE_X11
    printf("END drwOpen ==> %lu\n", (long unsigned) result);
#endif
    return((wintype) result);
  } /* drwOpen */



#ifdef ANSI_C

wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)
#else

  wintype drwOpenSubWindow (parent_window, xPos, yPos, width, height)
wintype parent_window;
inttype xPos;
inttype yPos;
inttype width;
inttype height;
#endif

  {
    unsigned long valuemask;
    Screen *x_screen;
    XSetWindowAttributes attributes;
    XWindowAttributes attribs;
    x11_wintype result;

  /* drwOpenSubWindow */
#ifdef TRACE_X11
    printf("BEGIN drwOpenSubWindow(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
#ifdef DO_HEAP_STATISTIC
    count.size_winrecord = SIZ_REC(x11_winrecord);
#endif
    result = NULL;
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay != NULL) {
        if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
          memset(result, 0, sizeof(struct x11_winstruct));
          result->usage_count = 1;
          result->next = window_list;
          window_list = result;

          myhint.x = xPos;
          myhint.y = yPos;
          myhint.width = width;
          myhint.height = height;
          myhint.flags = PPosition | PSize;
          mywmhint.flags = InputHint;
          mywmhint.input = True;

          result->window = XCreateSimpleWindow(mydisplay,
              to_window(parent_window),
              xPos, yPos, (unsigned) width, (unsigned) height,
              0, myforeground, mybackground);

          result->backup = 0;
          result->clip_mask = 0;
          result->is_pixmap = FALSE;
          result->width = (unsigned int) width;
          result->height = (unsigned int) height;

          XSetStandardProperties(mydisplay, result->window,
              "", "",
              None, /* argv, argc, */ NULL, 0, &myhint);
          /* XSetWMHints(mydisplay, result->window, &mywmhint); */

          x_screen = XScreenOfDisplay(mydisplay, myscreen);
          /* printf("backing_store=%d (NotUseful=%d/WhenMapped=%d/Always=%d)\n",
              x_screen->backing_store, NotUseful, WhenMapped, Always); */
          if (x_screen->backing_store != NotUseful) {
            valuemask = CWBackingStore;
            attributes.backing_store = Always;
            XChangeWindowAttributes(mydisplay, result->window, valuemask, &attributes);

            /* printf("backing_store=%d %d\n", attributes.backing_store, Always); */
            XGetWindowAttributes(mydisplay, result->window, &attribs);
            /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
            if (attribs.backing_store != Always) {
              result->backup = XCreatePixmap(mydisplay, result->window,
                  (unsigned int) width, (unsigned int) height,
                  DefaultDepth(mydisplay, myscreen));
              /* printf("backup=%lu\n", (long unsigned) result->backup); */
            } /* if */
          } else {
            result->backup = XCreatePixmap(mydisplay, result->window,
                (unsigned int) width, (unsigned int) height,
                DefaultDepth(mydisplay, myscreen));
            /* printf("backup=%lu\n", (long unsigned) result->backup); */
          } /* if */

          /*
          mygc = XCreateGC(mydisplay, result->window, 0, 0);
          XSetBackground(mydisplay, mygc, mybackground);
          XSetForeground(mydisplay, mygc, myforeground);
          */

          XSelectInput(mydisplay, result->window,
              ButtonPressMask | KeyPressMask | ExposureMask);

          XMapRaised(mydisplay, result->window);
          drwClear((wintype) result, (inttype) myforeground);
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
#ifdef TRACE_X11
    printf("END drwOpenSubWindow ==> %lu\n", (long unsigned) result);
#endif
    return((wintype) result);
  } /* drwOpenSubWindow */



#ifdef ANSI_C

void drwPoint (const_wintype actual_window, inttype x, inttype y)
#else

void drwPoint (actual_window, x, y)
wintype actual_window;
inttype x, y;
#endif

  { /* drwPoint */
#ifdef TRACE_X11
    printf("point(%lu, %ld, %ld)\n", actual_window, x, y); 
#endif
    XDrawPoint(mydisplay, to_window(actual_window), mygc, x, y);
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, x, y);
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
#ifdef TRACE_X11
    printf("drwPPoint(%lu, %ld, %ld, %08lx)\n", actual_window, x, y, col); 
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawPoint(mydisplay, to_window(actual_window), mygc, x, y);
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, x, y);
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
    XPoint *points;
    memsizetype pos;

  /* drwConvPointList */
    len = pointList->size / sizeof(XPoint);
    if (len > 0) {
      points = (XPoint *) pointList->mem;
      xy[0] = points[0].x;
      xy[1] = points[0].y;
      for (pos = 1; pos < len; pos ++) {
        xy[pos << 1]       = points[pos].x + xy[(pos << 1) - 2];
        xy[(pos << 1) + 1] = points[pos].y + xy[(pos << 1) - 1];
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
    XPoint *points;
    memsizetype pos;
    bstritype result;

  /* drwGenPointList */
    /* printf("drwGenPointList(%ld .. %ld)\n", xyArray->min_position, xyArray->max_position); */
    num_elements = (uinttype) (xyArray->max_position - xyArray->min_position) + 1;
    if (num_elements & 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (num_elements >> 1 > MAX_BSTRI_LEN / sizeof(XPoint)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      len = (memsizetype) num_elements >> 1;
      if (!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(XPoint))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = len * sizeof(XPoint);
        if (len > 0) {
          points = (XPoint *) result->mem;
          points[0].x = xyArray->arr[0].value.intvalue;
          points[0].y = xyArray->arr[1].value.intvalue;
          for (pos = 1; pos < len; pos ++) {
            points[pos].x = xyArray->arr[ pos << 1     ].value.intvalue -
                            xyArray->arr[(pos << 1) - 2].value.intvalue;
            points[pos].y = xyArray->arr[(pos << 1) + 1].value.intvalue -
                            xyArray->arr[(pos << 1) - 1].value.intvalue;
          } /* for */
        } /* if */
      } /* if */
    } /* if */
    return(result);
  } /* drwGenPointList */



#ifdef ANSI_C

inttype drwLngPointList (bstritype point_list)
#else

inttype drwLngPointList (point_list)
bstritype point_list;
#endif

  { /* drwLngPointList */
    return(point_list->size / sizeof(XPoint));
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
    XPoint *points;
    int npoints;
    XPoint startBackup;

  /* drwPolyLine */
    points = (XPoint *) point_list->mem;
    npoints = point_list->size / sizeof(XPoint);
    memcpy(&startBackup, &points[0], sizeof(XPoint));
    points[0].x += x1;
    points[0].y += y1;
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawLines(mydisplay, to_window(actual_window), mygc, points, npoints, CoordModePrevious);
    if (to_backup(actual_window) != 0) {
      XDrawLines(mydisplay, to_backup(actual_window), mygc, points, npoints, CoordModePrevious);
    } /* if */
    memcpy(&points[0], &startBackup, sizeof(XPoint));
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
    XPoint *points;
    int npoints;
    XPoint startBackup;

  /* drwFPolyLine */
    points = (XPoint *) point_list->mem;
    npoints = point_list->size / sizeof(XPoint);
    memcpy(&startBackup, &points[0], sizeof(XPoint));
    points[0].x += x1;
    points[0].y += y1;
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillPolygon(mydisplay, to_window(actual_window), mygc, points, npoints,
        Nonconvex, CoordModePrevious);
    if (to_backup(actual_window) != 0) {
      XFillPolygon(mydisplay, to_backup(actual_window), mygc, points, npoints,
          Nonconvex, CoordModePrevious);
    } /* if */
    memcpy(&points[0], &startBackup, sizeof(XPoint));
  } /* drwFPolyLine */



#ifdef ANSI_C

void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x1, inttype y1)
#else

void drwPut (actual_window, pixmap, x1, y1)
wintype actual_window;
wintype pixmap;
inttype x1;
inttype y1;
#endif

  { /* drwPut */
#ifdef TRACE_X11
    printf("put(%lu, %lu, %ld, %ld)\n", actual_window, pixmap, x1, y1);
    printf("actual_window=%lu, pixmap=%lu\n", to_window(actual_window),
        pixmap != NULL ? to_window(pixmap) : NULL);
#endif
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    if (pixmap != NULL) {
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, to_clip_mask(pixmap));
        XSetClipOrigin(mydisplay, mygc, x1, y1);
      } /* if */
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
      } /* if */
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, None);
      } /* if */
    } /* if */
  } /* drwPut */



#ifdef ANSI_C

void dra_put_clip (wintype actual_window, wintype pixmap, wintype bitmap,
    inttype x1, inttype y1)
#else

void dra_put_clip (actual_window, pixmap, bitmap, x1, y1)
wintype actual_window;
wintype pixmap;
wintype bitmap;
inttype x1;
inttype y1;
#endif

  { /* dra_put_clip */
#ifdef TRACE_X11
    printf("put(%lu, %lu, %ld, %ld)\n", actual_window, pixmap, x1, y1);
    printf("actual_window=%lu, pixmap=%lu\n", to_window(actual_window), to_window(pixmap));
#endif
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    if (pixmap != NULL) {
      XSetClipMask(mydisplay, mygc, to_window(bitmap));
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
      } /* if */
      XSetClipMask(mydisplay, mygc, None);
    } /* if */
  } /* dra_put_clip */



#ifdef ANSI_C

void drwRect (const_wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y)
#else

void drwRect (actual_window, x1, y1, length_x, length_y)
wintype actual_window;
inttype x1, y1, length_x, length_y;
#endif

  { /* drwRect */
#ifdef TRACE_X11
    printf("rect(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x1, y1, length_x, length_y);
#endif
    XFillRectangle(mydisplay, to_window(actual_window), mygc, x1, y1,
        (unsigned) length_x, (unsigned) length_y);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, x1, y1,
          (unsigned) length_x, (unsigned) length_y);
    } /* if */
  } /* drwRect */



#ifdef ANSI_C

void drwPRect (const_wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y, inttype col)
#else

void drwPRect (actual_window, x1, y1, length_x, length_y, col)
wintype actual_window;
inttype x1, y1, length_x, length_y;
inttype col;
#endif

  { /* drwPRect */
#ifdef TRACE_X11
    printf("drwPRect(%lu, %ld, %ld, %ld, %ld, %08lx)\n", actual_window, x1, y1, length_x, length_y, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillRectangle(mydisplay, to_window(actual_window), mygc, x1, y1,
        (unsigned) length_x, (unsigned) length_y);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, x1, y1,
          (unsigned) length_x, (unsigned) length_y);
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

  {
    static color_entry_type *color_hash = NULL;
    static unsigned long *pixels = NULL;
    static unsigned int num_pixels = 0;
    static unsigned int act_pixel = 0;
    unsigned int new_num_pixels;
    unsigned int pixel_incr;
    unsigned long *new_pixels;
    unsigned int hash_num;
    color_entry_type nearest_entry;
    color_entry_type *insert_place;
    color_entry_type entry;
    Colormap cmap;
    XColor col;
    int okay;

  /* drwRgbColor */
#ifdef TRACE_X11
    printf("drwRgbColor(%lu, %ld, %ld)\n", red_val, green_val, blue_val);
#endif
/*    printf("search [%ld, %ld, %ld]\n",
        red_val, green_val, blue_val); */
    if (default_visual->c_class == TrueColor) {
      col.pixel =
          ((((unsigned long) red_val)   << lshift_red   >> rshift_red)   & default_visual->red_mask)   |
          ((((unsigned long) green_val) << lshift_green >> rshift_green) & default_visual->green_mask) |
          ((((unsigned long) blue_val)  << lshift_blue  >> rshift_blue)  & default_visual->blue_mask);
      /*
      printf("((unsigned long) red_val): %08lx\n",
          ((unsigned long) red_val));
      printf("((unsigned long) red_val) << lshift_red: %08lx\n",
          ((unsigned long) red_val) << lshift_red);
      printf("((unsigned long) red_val) << lshift_red >> rshift_red: %08lx\n",
          ((unsigned long) red_val) << lshift_red >> rshift_red);
      printf("((unsigned long) green_val): %08lx\n",
          ((unsigned long) green_val));
      printf("((unsigned long) green_val) << lshift_green: %08lx\n",
          ((unsigned long) green_val) << lshift_green);
      printf("((unsigned long) green_val) << lshift_green >> rshift_green: %08lx\n",
          ((unsigned long) green_val) << lshift_green >> rshift_green);
      printf("((unsigned long) blue_val): %08lx\n",
          ((unsigned long) blue_val));
      printf("((unsigned long) blue_val) << lshift_blue: %08lx\n",
          ((unsigned long) blue_val) << lshift_blue);
      printf("((unsigned long) blue_val) << lshift_blue >> rshift_blue: %08lx\n",
          ((unsigned long) blue_val) << lshift_blue >> rshift_blue);
      printf("allocated [%04lx, %04lx, %04lx] color = %08lx\n",
          red_val, green_val, blue_val, col.pixel);
      */
      return(col.pixel);
    } /* if */
    if (color_hash == NULL) {
      color_hash = (color_entry_type *) malloc(4096 * sizeof(color_entry_type));
      if (color_hash == NULL) {
        printf("malloc color_hash failed for (%lu, %lu, %lu)\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val);
        return(0);
      } /* if */
      memset(color_hash, 0, 4096 * sizeof(color_entry_type));
    } /* if */
    col.red = red_val;
    col.green = green_val;
    col.blue = blue_val;
    hash_num = ((col.red   & 0xF000) >> 4) ^
               ((col.green & 0xF000) >> 8) ^
               ((col.blue  & 0xF000) >> 12);
    nearest_entry = NULL;
    insert_place = &color_hash[hash_num];
    entry = color_hash[hash_num];
    while (entry != NULL) {
/*      printf("compare [%hu, %hu, %hu] with [%hu, %hu, %hu]\n",
          entry->red, entry->green, entry->blue,
          col.red, col.green, col.blue); */
      nearest_entry = entry;
      if (entry->red < col.red) {
        insert_place = &entry->red_less;
        entry = entry->red_less;
      } else if (entry->red > col.red) {
        insert_place = &entry->red_greater;
        entry = entry->red_greater;
      } else if (entry->green < col.green) {
        insert_place = &entry->green_less;
        entry = entry->green_less;
      } else if (entry->green > col.green) {
        insert_place = &entry->green_greater;
        entry = entry->green_greater;
      } else if (entry->blue < col.blue) {
        insert_place = &entry->blue_less;
        entry = entry->blue_less;
      } else if (entry->blue > col.blue) {
        insert_place = &entry->blue_greater;
        entry = entry->blue_greater;
      } else {
/*        printf("found [%ld, %ld, %ld] color = %08lx\n",
            red_val, green_val, blue_val, entry->pixel); */
        return(entry->pixel);
      } /* if */
    } /* while */

#ifdef OUT_OF_ORDER
    if (num_pixels > 1) {
      XStandardColormap stdcolor;

      if (XGetStandardColormap(mydisplay, RootWindow(mydisplay, DefaultScreen(mydisplay)), &stdcolor, XA_RGB_BEST_MAP) == 0) {
        printf("XGetStandardColormap(... XA_RGB_BEST_MAP) not okay\n");
        if (XGetStandardColormap(mydisplay, RootWindow(mydisplay, DefaultScreen(mydisplay)), &stdcolor, XA_RGB_DEFAULT_MAP) == 0) {
          printf("XGetStandardColormap(... XA_RGB_DEFAULT_MAP) not okay\n");
        } else {
          printf("colormap:   %X\n",  (unsigned int) stdcolor.colormap);
          printf("red_max:    %lu\n", stdcolor.red_max);
          printf("red_mult:   %lu\n", stdcolor.red_mult);
          printf("green_max:  %lu\n", stdcolor.green_max);
          printf("green_mult: %lu\n", stdcolor.green_mult);
          printf("blue_max:   %lu\n", stdcolor.blue_max);
          printf("blue_mult:  %lu\n", stdcolor.blue_mult);
          printf("base_pixel: %lu\n", stdcolor.base_pixel);
       } /* if */
      } else {
        printf("colormap:   %X\n",  (unsigned int) stdcolor.colormap);
        printf("red_max:    %lu\n", stdcolor.red_max);
        printf("red_mult:   %lu\n", stdcolor.red_mult);
        printf("green_max:  %lu\n", stdcolor.green_max);
        printf("green_mult: %lu\n", stdcolor.green_mult);
        printf("blue_max:   %lu\n", stdcolor.blue_max);
        printf("blue_mult:  %lu\n", stdcolor.blue_mult);
        printf("base_pixel: %lu\n", stdcolor.base_pixel);
      } /* if */
    } /* if */
#endif

    cmap = DefaultColormap(mydisplay, myscreen);
/*    printf("cmap:   %X\n",  (unsigned int) cmap); */

    okay = 1;
    if (act_pixel >= num_pixels) {
      pixel_incr = 128;
      do {
        new_num_pixels = num_pixels + pixel_incr;
        new_pixels = (unsigned long *) realloc(pixels, new_num_pixels * sizeof(unsigned long));
        if (new_pixels == NULL) {
          printf("malloc pixels %u failed for (%lu, %lu, %lu)\n",
              new_num_pixels, (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val);
        } else {
          memset(&new_pixels[num_pixels], 0, pixel_incr * sizeof(unsigned long));
          if (XAllocColorCells(mydisplay, cmap, 0, NULL, 0, &new_pixels[num_pixels], pixel_incr) == 0) {
            /*            printf("XAllocColorCells(... &new_pixels[%d], %d) not okay\n",
                          num_pixels, pixel_incr); */
            pixel_incr /= 2;
          } else {
            pixels = new_pixels;
            num_pixels = new_num_pixels;
          } /* if */
        } /* if */
      } while (new_pixels != NULL && act_pixel >= num_pixels && pixel_incr >= 1);
      printf("num_pixels: %u\n", num_pixels);
    } /* if */
    if (act_pixel < num_pixels) {
      col.pixel = pixels[act_pixel];
      act_pixel++;
      if (XStoreColor(mydisplay, cmap, &col) == 0) {
        printf("XStoreColor(%lu, %lu, %lu) not okay\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val);
        okay = 0;
      } /* if */
    } else {
      if (nearest_entry != NULL) {
        printf("nearest_entry [%04lx, %04lx, %04lx] [%04x, %04x, %04x] color = %08lx\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val,
            nearest_entry->red, nearest_entry->green, nearest_entry->blue,
            nearest_entry->pixel);
        return(nearest_entry->pixel);
      } else {
        printf("return black [%04lx, %04lx, %04lx] color = %x\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val, 0);
        return(0);
      } /* if */
    } /* if */
    if (!okay) {
      okay = 1;
      if (XAllocColor(mydisplay, cmap, &col) == 0) {
        printf("XAllocColor(%lu, %lu, %lu) not okay\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val);
        okay = 0;
      } /* if */
    } /* if */
    if (okay) {
      entry = (color_entry_type) malloc(sizeof(struct color_entry));
      if (entry == NULL) {
        printf("drwRgbColor: malloc failed\n");
        col.pixel = 0;
      } else {
        memset(entry, 0, sizeof(struct color_entry));
        entry->red = red_val;
        entry->green = green_val;
        entry->blue = blue_val;
        entry->pixel = col.pixel;
        *insert_place = entry;
      } /* if */
    } else {
      col.pixel = 0;
    } /* if */
    printf("allocated [%04lx, %04lx, %04lx] color = %08lx\n",
        (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val, col.pixel);
    return(col.pixel);
  } /* drwRgbColor */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)
#else

inttype drwRgbColor (red_val, green_val, blue_val)
inttype red_val;
inttype green_val;
inttype blue_val;
#endif

  {
    Colormap cmap;
    XColor col;
    char color_string[14];

  /* drwRgbColor */
    cmap = DefaultColormap(mydisplay, myscreen);
    col.red = red_val;
    col.green = green_val;
    col.blue = blue_val;
    if (XAllocColor(mydisplay, cmap, &col) == 0) {
      /* handle failture */
      printf("XAllocColor(%ld, %ld, %ld) not okay\n",
          red_val, green_val, blue_val);
    } /* if */
#ifdef OUT_OF_ORDER
    col.red = RED_VAL;
    col.green = GREEN_VAL;
    col.blue = BLUE_VAL;
    if (XAllocColor(mydisplay, cmap, &col) == 0) {
      /* handle failture */
      printf("XAllocColor(%ld, %ld, %ld) not okay\n",
          RED_VAL, GREEN_VAL, BLUE_VAL);
    } /* if */
#endif
    printf("XParseColor(\"%s\" [%ld, %ld, %ld]) ==> [%ld, %ld, %ld]\n",
           color_string, RED_VAL, GREEN_VAL, BLUE_VAL,
        col.red, col.green, col.blue);
    printf("rgb color = %08lx\n", (long) col.pixel);
    return(col.pixel);
  } /* drwRgbColor */
#endif



#ifdef ANSI_C

void drwBackground (inttype col)
#else

void drwBackground (col)
inttype col;
#endif

  { /* drwBackground */
/*  printf("set color = %ld\n", (long) col); */
    XSetBackground(mydisplay, mygc, (unsigned) col);
  } /* drwBackground */



#ifdef ANSI_C

void drwColor (inttype col)
#else

void drwColor (col)
inttype col;
#endif

  { /* drwColor */
/*  printf("set color = %ld\n", (long) col); */
    XSetForeground(mydisplay, mygc, (unsigned) col);
  } /* drwColor */



#ifdef ANSI_C

void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)
#else

void drwSetPos (actual_window, xPos, yPos)
wintype actual_window;
inttype x1, y1;
#endif

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
    XMoveWindow(mydisplay, to_window(actual_window), xPos, yPos);
    gkbKeyPressed();
    XFlush(mydisplay);
    XSync(mydisplay, 0);
    /* printf("end drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
  } /* drwSetPos */



#ifdef ANSI_C

void drwSetTransparentColor (wintype pixmap, inttype col)
#else

void drwSetTransparentColor (pixmap, col)
wintype pixmap;
inttype col;
#endif

  {
    GC bitmap_gc;
    int depth;
    int plane;
    unsigned long plane_mask;

  /* drwSetTransparentColor */
#ifdef TRACE_X11
    printf("drwSetTransparentColor(%lu, %lu)\n", pixmap, col);
    printf("pixmap=%lu\n", pixmap != NULL ? to_window(pixmap) : NULL);
#endif
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    if (pixmap != NULL) {
      if (to_clip_mask(pixmap) == 0) {
        to_var_clip_mask(pixmap) = XCreatePixmap(mydisplay,
            to_window(pixmap), to_width(pixmap), to_height(pixmap), 1);
        /* printf("clip_mask = %lu\n", to_window(pixmap)); */
      } /* if */
      bitmap_gc = XCreateGC(mydisplay, to_clip_mask(pixmap), 0, 0);
      depth = DefaultDepth(mydisplay, myscreen);
      /* printf("depth=%d\n", depth); */
      XSetForeground(mydisplay, bitmap_gc, 1);
      XSetBackground(mydisplay, bitmap_gc, 0);
      plane_mask = 1;
      if (col & plane_mask) {
        XSetFunction(mydisplay, bitmap_gc, GXcopyInverted);
      } else {
        XSetFunction(mydisplay, bitmap_gc, GXcopy);
      } /* if */
      /* printf("XCopyPlane(%lu, %lu, %lu, %lu, %d, %d, %u, %u, %d, %d, %lu)\n",
          mydisplay, to_window(pixmap), to_clip_mask(pixmap), bitmap_gc,
          0, 0, to_width(pixmap), to_height(pixmap), 0, 0, plane_mask); */
      XCopyPlane(mydisplay, to_window(pixmap), to_clip_mask(pixmap), bitmap_gc,
          0, 0, to_width(pixmap), to_height(pixmap), 0, 0, plane_mask);
      plane_mask <<= 1;
      for (plane = 1; plane < depth; plane++) {
        if (col & plane_mask) {
          XSetFunction(mydisplay, bitmap_gc, GXorInverted);
        } else {
          XSetFunction(mydisplay, bitmap_gc, GXor);
        } /* if */
        /* printf("XCopyPlane(%lu, %lu, %lu, %lu, %d, %d, %u, %u, %d, %d, %lu)\n",
            mydisplay, to_window(pixmap), to_clip_mask(pixmap), bitmap_gc,
            0, 0, to_width(pixmap), to_height(pixmap), 0, 0, plane_mask); */
        XCopyPlane(mydisplay, to_window(pixmap), to_clip_mask(pixmap), bitmap_gc,
            0, 0, to_width(pixmap), to_height(pixmap), 0, 0, plane_mask);
        plane_mask <<= 1;
      } /* for */
      XFreeGC(mydisplay, bitmap_gc);
    } /* if */
  } /* drwSetTransparentColor */



#ifdef ANSI_C

void drwText (const_wintype actual_window, inttype x, inttype y,
    stritype stri, inttype col, inttype bkcol)
#else

void drwText (actual_window, x, y, stri, col, bkcol)
wintype actual_window;
inttype x, y;
stritype stri;
inttype col;
inttype bkcol;
#endif

  { /* drwText */
#ifdef TRACE_X11
    printf("text(%lu, %ld, %ld, ...)\n", actual_window, x, y); 
#endif
#ifdef UTF32_STRINGS
    {
      XChar2b *stri_buffer;
      XChar2b *wstri;
      const strelemtype *strelem;
      memsizetype len;

      stri_buffer = (XChar2b *) malloc(sizeof(XChar2b) * stri->size);
      if (stri_buffer != NULL) {
        wstri = stri_buffer;
        strelem = stri->mem;
        len = stri->size;
        for (; len > 0; wstri++, strelem++, len--) {
          if (*strelem >= 65536) {
            raise_error(RANGE_ERROR);
            return;
          } /* if */
          wstri->byte1 = (*strelem >> 8) & 0xFF;
          wstri->byte2 = *strelem & 0xFF;
        } /* for */

        XSetForeground(mydisplay, mygc, (unsigned) col);
        XSetBackground(mydisplay, mygc, (unsigned) bkcol);
        XDrawImageString16(mydisplay, to_window(actual_window), mygc,
            x, y, stri_buffer, (int) stri->size);
        if (to_backup(actual_window) != 0) {
          XDrawImageString16(mydisplay, to_backup(actual_window), mygc,
              x, y, stri_buffer, (int) stri->size);
        } /* if */
        free(stri_buffer);
      } /* if */
    }
#else
    XDrawImageString(mydisplay, to_window(actual_window), mygc,
        x, y, stri->mem, (int) stri->size);
    if (to_backup(actual_window) != 0) {
      XDrawImageString(mydisplay, to_backup(actual_window), mygc,
          x, y, stri->mem, (int) stri->size);
    } /* if */
#endif
  } /* drwText */



#ifdef ANSI_C

inttype drwWidth (const_wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    Status status;

  /* drwWidth */
    status = XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth);
    return((inttype) width);
  } /* drwWidth */



#ifdef ANSI_C

inttype drwXPos (const_wintype actual_window)
#else

inttype drwXPos (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    Status status;

  /* drwXPos */
    status = XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth);
    return((inttype) x);
  } /* drwXPos */



#ifdef ANSI_C

inttype drwYPos (const_wintype actual_window)
#else

inttype drwYPos (actual_window)
wintype actual_window;
#endif

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    Status status;

  /* drwYPos */
    status = XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth);
    return((inttype) y);
  } /* drwYPos */
