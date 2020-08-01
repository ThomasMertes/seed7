/********************************************************************/
/*                                                                  */
/*  drw_x11.c     Graphic access using the X11 capabilitys.         */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  File: seed7/src/drw_x11.c                                       */
/*  Changes: 1994, 2000 - 2005  Thomas Mertes                       */
/*  Content: Graphic access using the X11 capabilitys.              */
/*                                                                  */
/********************************************************************/

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

#include "version.h"
#include "common.h"
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


typedef struct x11_winstruct {
  unsigned long usage_count;
  Window window;
  Pixmap backup;
  booltype is_pixmap;
  unsigned int width;
  unsigned int height;
  struct x11_winstruct *next;
} *x11_wintype;

static x11_wintype window_list = NULL;

#define to_window(win) (((x11_wintype) win)->window)
#define to_backup(win) (((x11_wintype) win)->backup)
#define is_pixmap(win) (((x11_wintype) win)->is_pixmap)
#define to_width(win)  (((x11_wintype) win)->width)
#define to_height(win) (((x11_wintype) win)->height)

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
    printf("redraw\n");
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
  } /* redraw */



#ifdef ANSI_C

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  {
    int count;
    unsigned char buffer[21];
    chartype result;

  /* gkbGetc */
#ifdef TRACE_KBD
    printf("begin gkbGetc\n");
#endif
    result = K_NONE;
    XNextEvent(mydisplay, &myevent);
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
        count = XLookupString(&myevent.xkey, buffer, 20, &mykey, 0);
        buffer[count] = '\0';
        if (myevent.xkey.state & ShiftMask) {
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
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("1 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (myevent.xkey.state & ControlMask) {
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
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("2 undef key: %ld %lx\n", (long) mykey, (long) mykey);
                result = K_UNDEF;
              } /* if */
              break;
          } /* switch */
        } else if (myevent.xkey.state & Mod1Mask) {
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
              if (count == 1) {
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
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("5 undef key: %ld %lx\n", (long) mykey, (long) mykey);
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
    int count;
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
          count = XLookupString(&myevent.xkey, buffer, 20, &mykey, 0);
          buffer[count] = '\0';
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

void drwArc (wintype actual_window, inttype x, inttype y,
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

void drwFArcChord (wintype actual_window, inttype x, inttype y,
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

void drwPFArcChord (wintype actual_window, inttype x, inttype y,
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

void drwFArcPieSlice (wintype actual_window, inttype x, inttype y,
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

void drwArc2 (wintype actual_window,
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

void drwCircle (wintype actual_window,
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

void drwPCircle (wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPCircle (actual_window, x, y, col)
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

void drwClear (wintype actual_window, inttype col)
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
#ifdef TRACE_X11
    printf("XCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
        src_window, dest_window, src_x, src_y, width, height, dest_x, dest_y);
#endif
    if (to_backup(src_window) != 0) {
      XCopyArea(mydisplay, to_backup(src_window), to_window(dest_window),
          mygc, src_x, src_y, width, height, dest_x, dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_backup(src_window), to_backup(dest_window),
            mygc, src_x, src_y, width, height, dest_x, dest_y);
      } /* if */
    } else {
      XCopyArea(mydisplay, to_window(src_window), to_window(dest_window),
          mygc, src_x, src_y, width, height, dest_x, dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_window(src_window), to_backup(dest_window),
            mygc, src_x, src_y, width, height, dest_x, dest_y);
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
#ifdef TRACE_X11
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

void drwFCircle (wintype actual_window,
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

void drwPFCircle (wintype actual_window,
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

void drwFEllipse (wintype actual_window,
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
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x, y, width, height, 0, 23040);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x, y, width, height, 0, 23040);
    } /* if */
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

  { /* drwPFEllipse */
#ifdef TRACE_X11
    printf("fellipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x, y, width, height, 0, 23040);
    if (to_backup(actual_window) != 0) {
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x, y, width, height, 0, 23040);
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
    free((x11_wintype) old_window);
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
    x11_wintype result;

  /* drwGet */
#ifdef TRACE_X11
    printf("get(%lu, %ld, %ld, %ld, %ld)\n", actual_window, left, upper, width, height);
#endif
    result = (x11_wintype) malloc(sizeof(struct x11_winstruct));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), width, height,
          DefaultDepth(mydisplay, myscreen));
      result->backup = NULL;
      result->is_pixmap = TRUE;
      result->width = width;
      result->height = height;
      result->next = NULL;
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_backup(actual_window),
            result->window, mygc, left, upper, width, height, 0, 0);
      } else {
        XCopyArea(mydisplay, to_window(actual_window),
            result->window, mygc, left, upper, width, height, 0, 0);
      } /* if */
      /* printf("XCopyArea(%ld, %ld, %ld, %ld)\n", left, upper, width, height); */
    } /* if */
    return((wintype) result);
  } /* drwGet */



#ifdef ANSI_C

inttype drwHeight (wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  {
    /* Display *mydisplay;                        globally defined */
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

wintype drwImage (wintype actual_window, inttype *image_data,
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
    image = XCreateImage(mydisplay, default_visual,
        DefaultDepth(mydisplay, myscreen), ZPixmap, 0, (char *) image_data,
        width, height, 32, 0);
    if (image == NULL) {
      result = NULL;
    } else {
      result = (x11_wintype) malloc(sizeof(struct x11_winstruct));
      if (result != NULL) {
        memset(result, 0, sizeof(struct x11_winstruct));
        result->usage_count = 1;
        result->window = XCreatePixmap(mydisplay,
            to_window(actual_window), width, height,
            DefaultDepth(mydisplay, myscreen));
        result->backup = NULL;
        result->is_pixmap = TRUE;
        result->width = width;
        result->height = height;
        result->next = NULL;
        XPutImage(mydisplay, result->window, mygc, image, 0, 0, 0, 0, width, height);
      } /* if */
      XFree(image);
    } /* if */
    return((wintype) result);
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
#ifdef TRACE_X11
    printf("line(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x1, y1, x2, y2);
#endif
    XDrawLine(mydisplay, to_window(actual_window), mygc, x1, y1, x2, y2);
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc, x1, y1, x2, y2);
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

wintype drwNewPixmap (wintype actual_window, inttype width, inttype height)
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
    result = (x11_wintype) malloc(sizeof(struct x11_winstruct));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), width, height,
          DefaultDepth(mydisplay, myscreen));
      result->backup = NULL;
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

  {
    x11_wintype result;

  /* drwNewBitmap */
#ifdef TRACE_X11
    printf("drwNewBitmap(%ld, %ld)\n", width, height);
#endif
    result = (x11_wintype) malloc(sizeof(struct x11_winstruct));
    if (result != NULL) {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), width, height, 1);
      result->backup = NULL;
      result->is_pixmap = TRUE;
      result->width = width;
      result->height = height;
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
    char *class_text;

  /* dra_init */
    mydisplay = XOpenDisplay("");
/*  printf("mydisplay = %lu\n", (long unsigned) mydisplay); */
    myscreen = DefaultScreen(mydisplay);
/*  printf("myscreen = %lu\n", (long unsigned) myscreen); */

    default_visual = XDefaultVisual(mydisplay, myscreen);
    if (default_visual->class == PseudoColor) {
      class_text = "PseudoColor";
    } else if (default_visual->class == DirectColor) {
      class_text = "DirectColor";
    } else if (default_visual->class == GrayScale) {
      class_text = "GrayScale";
    } else if (default_visual->class == StaticColor) {
      class_text = "StaticColor";
    } else if (default_visual->class == TrueColor) {
      class_text = "TrueColor";
    } else if (default_visual->class == StaticGray) {
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
    result = NULL;
    if (mydisplay == NULL) {
      dra_init();
    } /* if */
    if (mydisplay != NULL) {
      win_name = cp_to_cstri(window_name);
      if (win_name == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        result = (x11_wintype) malloc(sizeof(struct x11_winstruct));
        if (result != NULL) {
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

          result->is_pixmap = FALSE;
          result->width = width;
          result->height = height;

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
              result->backup = XCreatePixmap(mydisplay, result->window, width, height,
                  DefaultDepth(mydisplay, myscreen));
              /* printf("backup=%lu\n", (long unsigned) result->backup); */
            } /* if */
          } else {
            result->backup = XCreatePixmap(mydisplay, result->window, width, height,
                DefaultDepth(mydisplay, myscreen));
            /* printf("backup=%lu\n", (long unsigned) result->backup); */
          } /* if */

          mygc = XCreateGC(mydisplay, result->window, 0, 0);
          XSetBackground(mydisplay, mygc, mybackground);
          XSetForeground(mydisplay, mygc, myforeground);

          XSelectInput(mydisplay, result->window,
              ButtonPressMask | KeyPressMask | ExposureMask);

          XMapRaised(mydisplay, result->window);
          drwClear((wintype) result, myforeground);
        } /* if */
        free_cstri(win_name, window_name);
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
#ifdef TRACE_X11
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
#ifdef TRACE_X11
    printf("point(%lu, %ld, %ld)\n", actual_window, x, y); 
#endif
    XDrawPoint(mydisplay, to_window(actual_window), mygc, x, y);
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, x, y);
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
#ifdef TRACE_X11
    printf("put(%lu, %lu, %ld, %ld)\n", actual_window, pixmap, x1, y1);
    printf("actual_window=%lu, pixmap=%lu\n", to_window(actual_window),
        pixmap != NULL ? to_window(pixmap) : NULL);
#endif
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    if (pixmap != NULL) {
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), x1, y1);
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

void drwRect (wintype actual_window,
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

void drwPRect (wintype actual_window,
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
    XStandardColormap stdcolor;
    Colormap cmap;
    XColor col;
    int okay;

  /* drwRgbColor */
#ifdef TRACE_X11
    printf("drwRgbColor(%lu, %ld, %ld)\n", red_val, green_val, blue_val);
#endif
/*    printf("search [%ld, %ld, %ld]\n",
        red_val, green_val, blue_val); */
    if (default_visual->class == TrueColor) {
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
        printf("malloc color_hash failed for (%ld, %ld, %ld)\n",
            red_val, green_val, blue_val);
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
          printf("malloc pixels %u failed for (%ld, %ld, %ld)\n",
              new_num_pixels, red_val, green_val, blue_val);
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
        printf("XStoreColor(%ld, %ld, %ld) not okay\n",
            red_val, green_val, blue_val);
        okay = 0;
      } /* if */
    } else {
      if (nearest_entry != NULL) {
        printf("nearest_entry [%04lx, %04lx, %04lx] [%04x, %04x, %04x] color = %08lx\n",
            red_val, green_val, blue_val,
            nearest_entry->red, nearest_entry->green, nearest_entry->blue,
            nearest_entry->pixel);
        return(nearest_entry->pixel);
      } else {
        printf("return black [%04lx, %04lx, %04lx] color = %x\n",
            red_val, green_val, blue_val, 0);
        return(0);
      } /* if */
    } /* if */
    if (!okay) {
      okay = 1;
      if (XAllocColor(mydisplay, cmap, &col) == 0) {
        printf("XAllocColor(%ld, %ld, %ld) not okay\n",
            red_val, green_val, blue_val);
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
        red_val, green_val, blue_val, col.pixel);
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
#ifdef TRACE_X11
    printf("text(%lu, %ld, %ld, ...)\n", actual_window, x, y); 
#endif
#ifdef WIDE_CHAR_STRINGS
    {
      XChar2b *stri_buffer;
      XChar2b *wstri;
      strelemtype *strelem;
      memsizetype len;

      stri_buffer = (XChar2b *) malloc(sizeof(XChar2b) * stri->size);
      if (stri_buffer != NULL) {
        wstri = stri_buffer;
        strelem = stri->mem;
        len = stri->size;
        for (; len > 0; wstri++, strelem++, len--) {
          wstri->byte1 = (*strelem >> 8) & 0xFF;
          wstri->byte2 = *strelem & 0xFF;
        } /* while */

        XSetForeground(mydisplay, mygc, (unsigned) col);
        XSetBackground(mydisplay, mygc, (unsigned) bkcol);
        XDrawImageString16(mydisplay, to_window(actual_window), mygc,
            x, y, stri_buffer, stri->size);
        if (to_backup(actual_window) != 0) {
          XDrawImageString16(mydisplay, to_backup(actual_window), mygc,
              x, y, stri_buffer, stri->size);
        } /* if */
        free(stri_buffer);
      } /* if */
    }
#else
    XDrawImageString(mydisplay, to_window(actual_window), mygc,
        x, y, stri->mem, stri->size);
    if (to_backup(actual_window) != 0) {
      XDrawImageString(mydisplay, to_backup(actual_window), mygc,
          x, y, stri->mem, stri->size);
    } /* if */
#endif
  } /* drwText */



#ifdef ANSI_C

inttype drwWidth (wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  {
    /* Display *mydisplay;                        globally defined */
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
