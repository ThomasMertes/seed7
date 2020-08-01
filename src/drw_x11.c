/********************************************************************/
/*                                                                  */
/*  drw_x11.c     Graphic access using the X11 capabilities.        */
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
/*  File: seed7/src/drw_x11.c                                       */
/*  Changes: 1994, 2000 - 2011, 2013  Thomas Mertes                 */
/*  Content: Graphic access using the X11 capabilities.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#undef WITH_XSHAPE_EXTENSION

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef WITH_XSHAPE_EXTENSION
#include <X11/extensions/shape.h>
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"

#undef TRACE_X11

#ifndef C_PLUS_PLUS
#define c_class class
#endif


#define PI 3.141592653589793238462643383279502884197

Display *mydisplay = NULL;
Atom wm_delete_window;
unsigned long myforeground;
unsigned long mybackground;
GC mygc;
XSizeHints myhint;
XWMHints mywmhint;
int myscreen;

typedef struct x11_winstruct {
    uinttype usage_count;
    /* Up to here the structure is identical to struct winstruct */
    Window window;
    Pixmap backup;
    Pixmap clip_mask;
    booltype is_pixmap;
    unsigned int width;
    unsigned int height;
    inttype clear_col;
  } x11_winrecord, *x11_wintype;

typedef const struct x11_winstruct *const_x11_wintype;

#ifdef DO_HEAP_STATISTIC
size_t sizeof_winrecord = sizeof(x11_winrecord);
#endif

#define to_window(win)    (((const_x11_wintype) win)->window)
#define to_backup(win)    (((const_x11_wintype) win)->backup)
#define to_clip_mask(win) (((const_x11_wintype) win)->clip_mask)
#define is_pixmap(win)    (((const_x11_wintype) win)->is_pixmap)
#define to_width(win)     (((const_x11_wintype) win)->width)
#define to_height(win)    (((const_x11_wintype) win)->height)
#define to_clear_col(win) (((const_x11_wintype) win)->clear_col)

#define to_var_window(win)    (((x11_wintype) win)->window)
#define to_var_backup(win)    (((x11_wintype) win)->backup)
#define to_var_clip_mask(win) (((x11_wintype) win)->clip_mask)
#define is_var_pixmap(win)    (((x11_wintype) win)->is_pixmap)
#define to_var_width(win)     (((x11_wintype) win)->width)
#define to_var_height(win)    (((x11_wintype) win)->height)
#define to_var_clear_col(win) (((x11_wintype) win)->clear_col)

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



wintype find_window (Window curr_window);
void enter_window (wintype curr_window, Window xWin);
void remove_window (wintype curr_window, Window xWin);



void redraw (wintype redraw_window, int xPos, int yPos, int width, int height)

  {
    x11_wintype expose_window;
    int xClear, yClear, clearWidth, clearHeight;

  /* redraw */
#ifdef TRACE_X11
    printf("begin redraw\n");
#endif
    /* printf("redraw(%lx, %d, %d, %d, %d)\n",
        redraw_window, xPos, yPos, width, height); */
    expose_window = (x11_wintype) redraw_window;
    /* XFlush(mydisplay);
       XSync(mydisplay, 0);
       getchar(); */
    XCopyArea(mydisplay, expose_window->backup,
        expose_window->window, mygc, xPos, yPos,
        width, height, xPos, yPos);
    /* printf("xPos + width=%d, to_width(expose_window)=%d\n",
           xPos + width, to_width(expose_window));
       printf("yPos + height=%d, to_height(expose_window)=%d\n",
           yPos + height, to_height(expose_window)); */
    if (xPos + width > to_width(expose_window)) {
      XSetForeground(mydisplay, mygc, to_clear_col(redraw_window));
      if (xPos >= to_width(expose_window)) {
        xClear = xPos;
        clearWidth = width;
      } else {
        xClear = to_width(expose_window);
        clearWidth = xPos + width - to_width(expose_window);
        if (yPos + height > to_height(expose_window)) {
          if (yPos >= to_height(expose_window)) {
            yClear = yPos;
            clearHeight = height;
          } else {
            yClear = to_height(expose_window);
            clearHeight = yPos + height - to_height(expose_window);
          } /* if */
          /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
              xPos, yClear, to_width(expose_window) - xPos, clearHeight); */
          XFillRectangle(mydisplay, to_window(expose_window), mygc,
              xPos, yClear, to_width(expose_window) - xPos, clearHeight);
        } /* if */
      } /* if */
      /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
          xClear, yPos, clearWidth, height); */
      XFillRectangle(mydisplay, to_window(expose_window), mygc,
          xClear, yPos, clearWidth, height);
    } else if (yPos + height > to_height(expose_window)) {
      XSetForeground(mydisplay, mygc, to_clear_col(redraw_window));
      if (yPos >= to_height(expose_window)) {
        yClear = yPos;
        clearHeight = height;
      } else {
        yClear = to_height(expose_window);
        clearHeight = yPos + height - to_height(expose_window);
      } /* if */
      /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
          xPos, yClear, width, clearHeight); */
      XFillRectangle(mydisplay, to_window(expose_window), mygc,
          xPos, yClear, width, clearHeight);
    } /* if */
#ifdef TRACE_X11
    printf("end redraw\n");
#endif
  } /* redraw */



void doFlush (void)

  { /* doFlush */
#ifdef TRACE_X11
    printf("doFlush\n");
#endif
    XFlush(mydisplay);
    /* XSync(mydisplay, 0); */
  } /* doFlush */



void flushBeforeRead (void)

  { /* flushBeforeRead */
    /* Not necessary, since XNextEvent() does a flush */
  } /* flushBeforeRead */



#ifdef OUT_OF_ORDER
void configure (XConfigureEvent *xconfigure)

  {
    x11_wintype configure_window;
    Window root;
    /* int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth; */
    Status status;
    int num_events;
    XEvent peekEvent;

  /* configure */
#ifdef TRACE_X11
    printf("begin configure\n");
#endif
    configure_window = (x11_wintype) find_window(xconfigure->window);
    if (configure_window->actual_width != xconfigure->width ||
        configure_window->actual_height != xconfigure->height) {
      printf("XConfigureEvent x=%d, y=%d, width=%d, height=%d\n",
          xconfigure->x, xconfigure->y, xconfigure->width, xconfigure->height);
      /* status = XGetGeometry(mydisplay, xconfigure->window, &root,
          &x, &y, &width, &height, &border_width, &depth); */
      /* printf("XGetGeometry    x=%d, y=%d, width=%d, height=%d\n",
          x, y, width, height); */
      num_events = XEventsQueued(mydisplay, QueuedAfterReading);
      if (num_events != 0) {
        XPeekEvent(mydisplay, &peekEvent);
        if (peekEvent.type == Expose &&
            peekEvent.xexpose.x == 0 &&
            peekEvent.xexpose.y == 0 &&
            peekEvent.xexpose.width == xconfigure->width &&
            peekEvent.xexpose.height == xconfigure->height) {
          printf("XExposeEvent x=%d, y=%d, width=%d, height=%d, count=%d\n",
              peekEvent.xexpose.x, peekEvent.xexpose.y, peekEvent.xexpose.width, peekEvent.xexpose.height, peekEvent.xexpose.count);
          /* XNextEvent(mydisplay, &peekEvent); */
        } else {
          printf("peekEvent.type=%d\n", peekEvent.type);
        } /* if */
      } else {
        printf("num_events == 0\n");
      } /* if */
      configure_window->actual_width = xconfigure->width;
      configure_window->actual_height = xconfigure->height;
    } /* if */
#ifdef TRACE_X11
    printf("end configure\n");
#endif
  } /* configure */
#endif



static int get_highest_bit (unsigned long number)

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
    return result + highest_bit[number];
  } /* get_highest_bit */



static void dra_init (void)

  {
#ifdef OUT_OF_ORDER
    const_cstritype class_text;
#endif

  /* dra_init */
#ifdef TRACE_X11
    printf("BEGIN dra_init()\n");
#endif
    /* When linking with a profiling standard library XOpenDisplay */
    /* deadlocked. Be careful to avoid this situation.             */
    mydisplay = XOpenDisplay("");
    /* printf("mydisplay = %lu\n", (long unsigned) mydisplay); */
    if (mydisplay != NULL) {
      myscreen = DefaultScreen(mydisplay);
      /* printf("myscreen = %lu\n", (long unsigned) myscreen); */

      default_visual = XDefaultVisual(mydisplay, myscreen);
#ifdef OUT_OF_ORDER
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

      mygc = XCreateGC(mydisplay, DefaultRootWindow(mydisplay), 0, 0);
      XSetBackground(mydisplay, mygc, mybackground);
      XSetForeground(mydisplay, mygc, myforeground);

      wm_delete_window = XInternAtom(mydisplay, "WM_DELETE_WINDOW", False);
    } /* if */
#ifdef TRACE_X11
    printf("END dra_init\n");
#endif
  } /* dra_init */



inttype drwPointerXpos (const_wintype actual_window)

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
    return win_x;
  } /* drwPointerXpos */



inttype drwPointerYpos (const_wintype actual_window)

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
    return win_y;
  } /* drwPointerYpos */



void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  {
    int startAng, sweepAng;

  /* drwArc */
#ifdef TRACE_X11
    printf("drwArc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
  } /* drwArc */



void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  {
    int startAng, sweepAng;

  /* drwPArc */
#ifdef TRACE_X11
    printf("drwPArc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
  } /* drwPArc */



void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcChord */
#ifdef TRACE_X11
    printf("drwFArcChord(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetArcMode(mydisplay, mygc, ArcChord);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
  } /* drwFArcChord */



void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  {
    int startAng, sweepAng;

  /* drwPFArcChord */
#ifdef TRACE_X11
    printf("drwPFArcChord(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XSetArcMode(mydisplay, mygc, ArcChord);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
  } /* drwPFArcChord */



void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcPieSlice */
#ifdef TRACE_X11
    printf("drwFArcPieSlice(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col)

  {
    int startAng, sweepAng;

  /* drwPFArcPieSlice */
#ifdef TRACE_X11
    printf("drwPFArcPieSlice(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    if (!inIntRange(x - radius) || !inIntRange(y - radius) ||
        !inIntRange(radius) || radius < 0) {
      raise_error(RANGE_ERROR);
    } else {
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
          startAng, sweepAng);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
            startAng, sweepAng);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
            startAng, sweepAng);
      } /* if */
    } /* if */
  } /* drwPFArcPieSlice */



void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)

  { /* drwArc2 */
/*  printf("drwArc2(%d, %d, %d, %d)\n", x1, y1, radius); */
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x1 - radius), castToInt(y1 - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XDrawLine(mydisplay, to_window(actual_window), mygc, x1, y1, x2, y2);
  } /* drwArc2 */



void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwCircle */
#ifdef TRACE_X11
    printf("drwCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwCircle */



void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  { /* drwPCircle */
#ifdef TRACE_X11
    printf("drwPCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwPCircle */



void drwClear (wintype actual_window, inttype col)

  { /* drwClear */
#ifdef TRACE_X11
    printf("drwClear(%lu, %08lx)\n", actual_window, col);
#endif
    to_var_clear_col(actual_window) = col;
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    /* The main window is cleared with the real window size. */
    XFillRectangle(mydisplay, to_window(actual_window), mygc, 0, 0,
        (unsigned int) drwWidth(actual_window), (unsigned int) drwHeight(actual_window));
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, 0, 0,
          to_width(actual_window), to_height(actual_window));
    } /* if */
  } /* drwClear */



void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y)

  { /* drwCopyArea */
#ifdef TRACE_X11
    printf("drwCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
        src_window, dest_window, src_x, src_y, width, height, dest_x, dest_y);
#endif
    if (!inIntRange(src_x) || !inIntRange(src_y) ||
        !inIntRange(width) || !inIntRange(height) ||
        !inIntRange(dest_x) || !inIntRange(dest_y) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else if (to_backup(src_window) != 0) {
      XCopyArea(mydisplay, to_backup(src_window), to_window(dest_window),
                mygc, (int) src_x, (int) src_y,
                (unsigned int) width, (unsigned int) height,
                (int) dest_x, (int) dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_backup(src_window), to_backup(dest_window),
                  mygc, (int) src_x, (int) src_y,
                  (unsigned int) width, (unsigned int) height,
                  (int) dest_x, (int) dest_y);
      } /* if */
    } else {
      XCopyArea(mydisplay, to_window(src_window), to_window(dest_window),
                mygc, (int) src_x, (int) src_y,
                (unsigned int) width, (unsigned int) height,
                (int) dest_x, (int) dest_y);
      if (to_backup(dest_window) != 0) {
        XCopyArea(mydisplay, to_window(src_window), to_backup(dest_window),
                  mygc, (int) src_x, (int) src_y,
                  (unsigned int) width, (unsigned int) height,
                  (int) dest_x, (int) dest_y);
      } /* if */
    } /* if */
  } /* drwCopyArea */



void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwFCircle */
#ifdef TRACE_X11
    printf("drwFCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwFCircle */



void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  { /* drwPFCircle */
#ifdef TRACE_X11
    printf("drwPFCircle(%lu, %ld, %ld, %ld, %08lx)\n",
        actual_window, x, y, radius, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwPFCircle */



void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwFEllipse */
#ifdef TRACE_X11
    printf("drwFEllipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
  } /* drwFEllipse */



void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  { /* drwPFEllipse */
#ifdef TRACE_X11
    printf("drwPFEllipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y),(unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
  } /* drwPFEllipse */



void drwFree (wintype old_window)

  { /* drwFree */
#ifdef TRACE_X11
    printf("drwFree(%lu)\n", old_window);
#endif
    if (is_pixmap(old_window)) {
      XFreePixmap(mydisplay, to_window(old_window));
    } else {
      XDestroyWindow(mydisplay, to_window(old_window));
      remove_window(old_window, to_window(old_window));
    } /* if */
    FREE_RECORD(old_window, x11_winrecord, count.win);
  } /* drwFree */



wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height)

  {
    x11_wintype result;

  /* drwGet */
#ifdef TRACE_X11
    printf("drwGet(%lu, %ld, %ld, %ld, %ld)\n", actual_window, left, upper, width, height);
#endif
    if (!inIntRange(left) || !inIntRange(upper) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (!ALLOC_RECORD(result, x11_winrecord, count.win)) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(struct x11_winstruct));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), (unsigned int) width, (unsigned int) height,
          (unsigned int) DefaultDepth(mydisplay, myscreen));
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_backup(actual_window),
            result->window, mygc, (int) left, (int) upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } else {
        XCopyArea(mydisplay, to_window(actual_window),
            result->window, mygc, (int) left, (int) upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } /* if */
      /* printf("XCopyArea(%ld, %ld, %ld, %ld)\n", left, upper, width, height); */
    } /* if */
    return (wintype) result;
  } /* drwGet */



inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y)

  {
    XImage *image;
    inttype pixel;

  /* drwGetPixel */
#ifdef TRACE_X11
    printf("drwGetPixel(%lu, %ld, %ld)\n", actual_window, x, y);
#endif
    if (to_backup(actual_window) != 0) {
      image = XGetImage(mydisplay, to_backup(actual_window),
                        castToInt(x), castToInt(y), 1, 1,
                        (unsigned long) -1, ZPixmap);
    } else {
      image = XGetImage(mydisplay, to_window(actual_window),
                        castToInt(x), castToInt(y), 1, 1,
                        (unsigned long) -1, ZPixmap);
    } /* if */
    pixel = (inttype) XGetPixel(image, 0, 0);
    XDestroyImage(image);
    /* printf("drwGetPixel --> %lx\n", pixel); */
    return pixel;
  } /* drwGetPixel */



inttype drwHeight (const_wintype actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwHeight */
    if (XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth) == 0) {
      raise_error(RANGE_ERROR);
      height = 0;
    } /* if */
#ifdef TRACE_X11
    printf("drwHeight(%lu) -> %u\n", actual_window, height);
#endif
    return (inttype) height;
  } /* drwHeight */



wintype drwImage (int32type *image_data, inttype width, inttype height)

  {
    XImage *image;
    x11_wintype result;

  /* drwImage */
#ifdef TRACE_X11
    printf("drwImage(%ld, %ld)\n", width, height);
#endif
    if (!inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        image = XCreateImage(mydisplay, default_visual,
            (unsigned int) DefaultDepth(mydisplay, myscreen),
            ZPixmap, 0, (char *) image_data,
            (unsigned int) width, (unsigned int) height, 32, 0);
        if (image == NULL) {
          result = NULL;
        } else {
          if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
            memset(result, 0, sizeof(struct x11_winstruct));
            result->usage_count = 1;
            result->window = XCreatePixmap(mydisplay,
                DefaultRootWindow(mydisplay), (unsigned int) width, (unsigned int) height,
                (unsigned int) DefaultDepth(mydisplay, myscreen));
            result->backup = 0;
            result->clip_mask = 0;
            result->is_pixmap = TRUE;
            result->width = (unsigned int) width;
            result->height = (unsigned int) height;
            XPutImage(mydisplay, result->window, mygc, image, 0, 0, 0, 0,
                (unsigned int) width, (unsigned int) height);
          } /* if */
          XFree(image);
        } /* if */
      } /* if */
    } /* if */
    return (wintype) result;
  } /* drwImage */



void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)

  { /* drwLine */
#ifdef TRACE_X11
    printf("drwLine(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x1, y1, x2, y2);
#endif
    XDrawLine(mydisplay, to_window(actual_window), mygc,
              castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc,
                castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwLine */



void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)

  { /* drwPLine */
#ifdef TRACE_X11
    printf("drwPLine(%lu, %ld, %ld, %ld, %ld, %08lx)\n", actual_window, x1, y1, x2, y2, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawLine(mydisplay, to_window(actual_window), mygc,
              castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc,
                castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwPLine */



wintype drwNewPixmap (inttype width, inttype height)

  {
    x11_wintype result;

  /* drwNewPixmap */
#ifdef TRACE_X11
    printf("drwNewPixmap(%ld, %ld)\n", width, height);
#endif
    if (!inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        if (!ALLOC_RECORD(result, x11_winrecord, count.win)) {
          raise_error(MEMORY_ERROR);
        } else {
          memset(result, 0, sizeof(struct x11_winstruct));
          result->usage_count = 1;
          result->window = XCreatePixmap(mydisplay,
              DefaultRootWindow(mydisplay), (unsigned int) width, (unsigned int) height,
              (unsigned int) DefaultDepth(mydisplay, myscreen));
          result->backup = 0;
          result->clip_mask = 0;
          result->is_pixmap = TRUE;
          result->width = (unsigned int) width;
          result->height = (unsigned int) height;
        } /* if */
      } /* if */
    } /* if */
    return (wintype) result;
  } /* drwNewPixmap */



wintype drwNewBitmap (const_wintype actual_window, inttype width, inttype height)

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
    } /* if */
    return (wintype) result;
  } /* drwNewBitmap */



wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name)

  {
    char *win_name;
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
    if (!inIntRange(xPos) || !inIntRange(yPos) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
      } else {
        win_name = cp_to_cstri8(window_name);
        if (win_name == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
            memset(result, 0, sizeof(struct x11_winstruct));
            result->usage_count = 1;

            myhint.x = (int) xPos;
            myhint.y = (int) yPos;
            myhint.width = (int) width;
            myhint.height = (int) height;
            myhint.flags = PPosition | PSize;
            mywmhint.flags = InputHint;
            mywmhint.input = True;

            result->window = XCreateSimpleWindow(mydisplay,
                DefaultRootWindow(mydisplay),
                myhint.x, myhint.y, (unsigned) myhint.width, (unsigned) myhint.height,
                5, myforeground, mybackground);
            enter_window((wintype) result, result->window);

            result->backup = 0;
            result->clip_mask = 0;
            result->is_pixmap = FALSE;
            result->width = (unsigned int) width;
            result->height = (unsigned int) height;

            XSetStandardProperties(mydisplay, result->window,
                win_name, win_name,
                None, /* argv, argc, */ NULL, 0, &myhint);
            XSetWMHints(mydisplay, result->window, &mywmhint);

            XSetWMProtocols(mydisplay, result->window, &wm_delete_window, 1);

            x_screen = XScreenOfDisplay(mydisplay, myscreen);
            /* printf("backing_store=%d (NotUseful=%d/WhenMapped=%d/Always=%d)\n",
                x_screen->backing_store, NotUseful, WhenMapped, Always); */
            if (x_screen->backing_store != NotUseful) {
              attributes.backing_store = Always;
              XChangeWindowAttributes(mydisplay, result->window,
                  CWBackingStore, &attributes);

              /* printf("backing_store=%d %d\n", attributes.backing_store, Always); */
              XGetWindowAttributes(mydisplay, result->window, &attribs);
              /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
              if (attribs.backing_store != Always) {
                result->backup = XCreatePixmap(mydisplay, result->window,
                    (unsigned int) width, (unsigned int) height,
                    (unsigned int) DefaultDepth(mydisplay, myscreen));
                /* printf("backup=%lu\n", (long unsigned) result->backup); */
              } /* if */
            } else {
              result->backup = XCreatePixmap(mydisplay, result->window,
                  (unsigned int) width, (unsigned int) height,
                  (unsigned int) DefaultDepth(mydisplay, myscreen));
              /* printf("backup=%lu\n", (long unsigned) result->backup); */
            } /* if */

            /* Avoid Expose events for the whole window when */
            /* it is resized:                                */
            attributes.bit_gravity = StaticGravity;
            XChangeWindowAttributes(mydisplay, result->window, CWBitGravity, &attributes);

            /* Avoid that an exposed area is cleared before  */
            /* the Expose event is delivered:                */
            attributes.background_pixmap = None;
            XChangeWindowAttributes(mydisplay, result->window, CWBackPixmap, &attributes);

            XSelectInput(mydisplay, result->window,
                ButtonPressMask | KeyPressMask | ExposureMask);
            /* currently not used: StructureNotifyMask */

            XMapRaised(mydisplay, result->window);
            drwClear((wintype) result, (inttype) myforeground);
            XFlush(mydisplay);
            XSync(mydisplay, 0);
          } /* if */
          free_cstri(win_name, window_name);
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
#ifdef TRACE_X11
    printf("END drwOpen ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpen */



wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)

  {
    Screen *x_screen;
    XSetWindowAttributes attributes;
    XWindowAttributes attribs;
    x11_wintype result;

  /* drwOpenSubWindow */
#ifdef TRACE_X11
    printf("BEGIN drwOpenSubWindow(%ld, %ld, %ld, %ld)\n",
        xPos, yPos, width, height);
#endif
    result = NULL;
    if (!inIntRange(xPos) || !inIntRange(yPos) ||
        !inIntRange(width) || !inIntRange(height) ||
        width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
      } else {
        if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
          memset(result, 0, sizeof(struct x11_winstruct));
          result->usage_count = 1;

          myhint.x = (int) xPos;
          myhint.y = (int) yPos;
          myhint.width = (int) width;
          myhint.height = (int) height;
          myhint.flags = PPosition | PSize;
          mywmhint.flags = InputHint;
          mywmhint.input = True;

          result->window = XCreateSimpleWindow(mydisplay,
              to_window(parent_window),
              (int) xPos, (int) yPos, (unsigned) width, (unsigned) height,
              0, myforeground, mybackground);
          enter_window((wintype) result, result->window);

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
            attributes.backing_store = Always;
            XChangeWindowAttributes(mydisplay, result->window,
                CWBackingStore, &attributes);

            /* printf("backing_store=%d %d\n", attributes.backing_store, Always); */
            XGetWindowAttributes(mydisplay, result->window, &attribs);
            /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
            if (attribs.backing_store != Always) {
              result->backup = XCreatePixmap(mydisplay, result->window,
                  (unsigned int) width, (unsigned int) height,
                  (unsigned int) DefaultDepth(mydisplay, myscreen));
              /* printf("backup=%lu\n", (long unsigned) result->backup); */
            } /* if */
          } else {
            result->backup = XCreatePixmap(mydisplay, result->window,
                (unsigned int) width, (unsigned int) height,
                (unsigned int) DefaultDepth(mydisplay, myscreen));
            /* printf("backup=%lu\n", (long unsigned) result->backup); */
          } /* if */

          /* Avoid Expose events for the whole window when */
          /* it is resized:                                */
          attributes.bit_gravity = StaticGravity;
          XChangeWindowAttributes(mydisplay, result->window, CWBitGravity, &attributes);

          /* Avoid that an exposed area is cleared before  */
          /* the Expose event is delivered:                */
          attributes.background_pixmap = None;
          XChangeWindowAttributes(mydisplay, result->window, CWBackPixmap, &attributes);

          XSelectInput(mydisplay, result->window,
              ButtonPressMask | KeyPressMask | ExposureMask);

          XMapRaised(mydisplay, result->window);
          drwClear((wintype) result, (inttype) myforeground);
          /* XFlush(mydisplay);
             XSync(mydisplay, 0); */
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
#ifdef TRACE_X11
    printf("END drwOpenSubWindow ==> %lu\n", (long unsigned) result);
#endif
    return (wintype) result;
  } /* drwOpenSubWindow */



void drwPoint (const_wintype actual_window, inttype x, inttype y)

  { /* drwPoint */
#ifdef TRACE_X11
    printf("drwPoint(%lu, %ld, %ld)\n", actual_window, x, y);
#endif
    XDrawPoint(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y));
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y));
    } /* if */
  } /* drwPoint */



void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col)

  { /* drwPPoint */
#ifdef TRACE_X11
    printf("drwPPoint(%lu, %ld, %ld, %08lx)\n", actual_window, x, y, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawPoint(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y));
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y));
    } /* if */
  } /* drwPPoint */



void drwConvPointList (bstritype pointList, inttype *xy)

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



bstritype drwGenPointList (const const_rtlArraytype xyArray)

  {
    memsizetype num_elements;
    memsizetype len;
    inttype x;
    inttype y;
    XPoint *points;
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
      if (len > MAX_BSTRI_LEN / sizeof(XPoint) || len > MAX_MEM_INDEX) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(XPoint))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len * sizeof(XPoint);
          if (len > 0) {
            points = (XPoint *) result->mem;
            x = xyArray->arr[0].value.intvalue;
            y = xyArray->arr[1].value.intvalue;
            if (unlikely(x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX)) {
              raise_error(RANGE_ERROR);
              return NULL;
            } else {
              points[0].x = (short) x;
              points[0].y = (short) y;
            } /* if */
            for (pos = 1; pos < len; pos ++) {
              x = xyArray->arr[ pos << 1     ].value.intvalue -
                  xyArray->arr[(pos << 1) - 2].value.intvalue;
              y = xyArray->arr[(pos << 1) + 1].value.intvalue -
                  xyArray->arr[(pos << 1) - 1].value.intvalue;
              if (unlikely(x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX)) {
                raise_error(RANGE_ERROR);
                return NULL;
              } else {
                points[pos].x = (short) x;
                points[pos].y = (short) y;
              } /* if */
            } /* for */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGenPointList */



inttype drwLngPointList (bstritype point_list)

  { /* drwLngPointList */
    return (inttype) (point_list->size / sizeof(XPoint));
  } /* drwLngPointList */



void drwPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  {
    XPoint *points;
    int npoints;
    XPoint startBackup;

  /* drwPolyLine */
    if (unlikely(x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX)) {
      raise_error(RANGE_ERROR);
    } else {
      points = (XPoint *) point_list->mem;
      npoints = (int) (point_list->size / sizeof(XPoint));
      memcpy(&startBackup, &points[0], sizeof(XPoint));
      points[0].x += castToShort(x);
      points[0].y += castToShort(y);
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawLines(mydisplay, to_window(actual_window), mygc, points, npoints, CoordModePrevious);
      if (to_backup(actual_window) != 0) {
        XDrawLines(mydisplay, to_backup(actual_window), mygc, points, npoints, CoordModePrevious);
      } /* if */
      memcpy(&points[0], &startBackup, sizeof(XPoint));
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  {
    XPoint *points;
    int npoints;
    XPoint startBackup;

  /* drwFPolyLine */
    if (unlikely(x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX)) {
      raise_error(RANGE_ERROR);
    } else {
      points = (XPoint *) point_list->mem;
      npoints = (int) (point_list->size / sizeof(XPoint));
      memcpy(&startBackup, &points[0], sizeof(XPoint));
      points[0].x += castToShort(x);
      points[0].y += castToShort(y);
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawLines(mydisplay, to_window(actual_window), mygc, points, npoints, CoordModePrevious);
      XFillPolygon(mydisplay, to_window(actual_window), mygc, points, npoints,
          Nonconvex, CoordModePrevious);
      if (to_backup(actual_window) != 0) {
        XDrawLines(mydisplay, to_backup(actual_window), mygc, points, npoints, CoordModePrevious);
        XFillPolygon(mydisplay, to_backup(actual_window), mygc, points, npoints,
            Nonconvex, CoordModePrevious);
      } /* if */
      memcpy(&points[0], &startBackup, sizeof(XPoint));
    } /* if */
  } /* drwFPolyLine */



void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x, inttype y)

  { /* drwPut */
#ifdef TRACE_X11
    printf("drwPut(%lu, %lu, %ld, %ld)\n", actual_window, pixmap, x, y);
    /* printf("actual_window=%lu, pixmap=%lu\n", to_window(actual_window),
        pixmap != NULL ? to_window(pixmap) : NULL); */
#endif
    /* A pixmap value of NULL is used to describe an empty pixmap. */
    /* In this case nothing should be done.                        */
    if (pixmap != NULL) {
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, to_clip_mask(pixmap));
        XSetClipOrigin(mydisplay, mygc, castToInt(x), castToInt(y));
      } /* if */
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), castToInt(x), castToInt(y));
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), castToInt(x), castToInt(y));
      } /* if */
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, None);
      } /* if */
    } /* if */
  } /* drwPut */



void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwRect */
#ifdef TRACE_X11
    printf("drwRect(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    XFillRectangle(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y),
        (unsigned) width, (unsigned) height);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y),
          (unsigned) width, (unsigned) height);
    } /* if */
  } /* drwRect */



void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  { /* drwPRect */
#ifdef TRACE_X11
    printf("drwPRect(%lu, %ld, %ld, %ld, %ld, %08lx)\n", actual_window, x, y, width, height, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XFillRectangle(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y),
        (unsigned) width, (unsigned) height);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y),
          (unsigned) width, (unsigned) height);
    } /* if */
  } /* drwPRect */



inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)

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
      /* printf("drwRgbColor(%ld, %ld, %ld) --> %lx\n", red_val, green_val, blue_val, (inttype) col.pixel); */
      return (inttype) col.pixel;
    } /* if */
    if (color_hash == NULL) {
      color_hash = (color_entry_type *) malloc(4096 * sizeof(color_entry_type));
      if (color_hash == NULL) {
        printf("malloc color_hash failed for (%lu, %lu, %lu)\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val);
        return 0;
      } /* if */
      memset(color_hash, 0, 4096 * sizeof(color_entry_type));
    } /* if */
    col.red   = (short unsigned int) red_val;
    col.green = (short unsigned int) green_val;
    col.blue  = (short unsigned int) blue_val;
    hash_num = ((unsigned int) (col.red   & 0xF000) >>  4) ^
               ((unsigned int) (col.green & 0xF000) >>  8) ^
               ((unsigned int) (col.blue  & 0xF000) >> 12);
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
        return (inttype) entry->pixel;
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
        return (inttype) nearest_entry->pixel;
      } else {
        printf("return black [%04lx, %04lx, %04lx] color = %x\n",
            (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val, 0);
        return 0;
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
        entry->red    = (unsigned short int) red_val;
        entry->green  = (unsigned short int) green_val;
        entry->blue   = (unsigned short int) blue_val;
        entry->pixel  = col.pixel;
        *insert_place = entry;
      } /* if */
    } else {
      col.pixel = 0;
    } /* if */
    printf("allocated [%04lx, %04lx, %04lx] color = %08lx\n",
        (unsigned long) red_val, (unsigned long) green_val, (unsigned long) blue_val, col.pixel);
    return (inttype) col.pixel;
  } /* drwRgbColor */



#ifdef OUT_OF_ORDER
inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)

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
    return col.pixel;
  } /* drwRgbColor */
#endif



void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)

  {
    Colormap cmap;
    XColor color;

  /* drwPixelToRgb */
    if (default_visual->c_class == TrueColor) {
      *red_val   = (inttype)(((unsigned long) col & default_visual->red_mask)   << rshift_red   >> lshift_red);
      *green_val = (inttype)(((unsigned long) col & default_visual->green_mask) << rshift_green >> lshift_green);
      *blue_val  = (inttype)(((unsigned long) col & default_visual->blue_mask)  << rshift_blue  >> lshift_blue);
    } else {
      cmap = DefaultColormap(mydisplay, myscreen);
      color.pixel = (unsigned long) col;
      XQueryColor(mydisplay, cmap, &color);
      *red_val   = color.red;
      *green_val = color.green;
      *blue_val  = color.blue;
    } /* if */
    /* printf("drwPixelToRgb(%lx) -> %ld %ld %ld\n", col, *red_val, *green_val, *blue_val); */
  } /* drwPixelToRgb */



void drwBackground (inttype col)

  { /* drwBackground */
/*  printf("set color = %ld\n", (long) col); */
    XSetBackground(mydisplay, mygc, (unsigned long) col);
  } /* drwBackground */



void drwColor (inttype col)

  { /* drwColor */
/*  printf("set color = %ld\n", (long) col); */
    XSetForeground(mydisplay, mygc, (unsigned long) col);
  } /* drwColor */



void drwSetContent (const_wintype actual_window, const_wintype pixmap)

  { /* drwSetContent */
    /* printf("begin drwSetContent(%lu, %lu)\n",
        to_window(actual_window), to_window(pixmap)); */
    if (pixmap != NULL) {
#ifdef WITH_XSHAPE_EXTENSION
      if (to_clip_mask(pixmap) != 0) {
        XShapeCombineMask(mydisplay, to_window(actual_window), ShapeBounding,
            0, 0, to_clip_mask(pixmap), ShapeSet);
      } /* if */
#endif
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), 0, 0);
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), 0, 0);
      } /* if */
    } /* if */
    /* printf("end drwSetContent(%lu, %lu)\n",
        to_window(actual_window), to_window(pixmap)); */
  } /* drwSetContent */



void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
    XMoveWindow(mydisplay, to_window(actual_window), castToInt(xPos), castToInt(yPos));
    /* printf("end drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
  } /* drwSetPos */



void drwSetTransparentColor (wintype pixmap, inttype col)

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
      if ((unsigned long) col & plane_mask) {
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
        if ((unsigned long) col & plane_mask) {
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



void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol)

  {
    XChar2b *stri_buffer;
    XChar2b *wstri;
    const strelemtype *strelem;
    memsizetype len;

  /* drwText */
#ifdef TRACE_X11
    printf("drwText(%lu, %ld, %ld, ...)\n", actual_window, x, y);
#endif
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

      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XSetBackground(mydisplay, mygc, (unsigned long) bkcol);
      XDrawImageString16(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y), stri_buffer, (int) stri->size);
      if (to_backup(actual_window) != 0) {
        XDrawImageString16(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y), stri_buffer, (int) stri->size);
      } /* if */
      free(stri_buffer);
    } /* if */
  } /* drwText */



void drwToBottom (const_wintype actual_window)

  { /* drwToBottom */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XLowerWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToBottom */



void drwToTop (const_wintype actual_window)

  { /* drwToTop */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XRaiseWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToTop */



inttype drwWidth (const_wintype actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwWidth */
    if (XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth) == 0) {
      raise_error(RANGE_ERROR);
      width = 0;
    } /* if */
#ifdef TRACE_X11
    printf("drwWidth(%lu) -> %u\n", actual_window, width);
#endif
    return (inttype) width;
  } /* drwWidth */



inttype drwXPos (const_wintype actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwXPos */
    if (XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth) == 0) {
      raise_error(RANGE_ERROR);
      x = 0;
    } /* if */
#ifdef TRACE_X11
    printf("drwXPos(%lu) -> %d\n", actual_window, x);
#endif
    return (inttype) x;
  } /* drwXPos */



inttype drwYPos (const_wintype actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwYPos */
    if (XGetGeometry(mydisplay, to_window(actual_window), &root,
        &x, &y, &width, &height, &border_width, &depth) == 0) {
      raise_error(RANGE_ERROR);
      y = 0;
    } /* if */
#ifdef TRACE_X11
    printf("drwYPos(%lu) -> %d\n", actual_window, y);
#endif
    return (inttype) y;
  } /* drwYPos */
