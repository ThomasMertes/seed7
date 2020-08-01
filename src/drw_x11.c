/********************************************************************/
/*                                                                  */
/*  drw_x11.c     Graphic access using the X11 capabilities.        */
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
/*  File: seed7/src/drw_x11.c                                       */
/*  Changes: 1994, 2000 - 2011  Thomas Mertes                       */
/*  Content: Graphic access using the X11 capabilities.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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
unsigned long myforeground, mybackground;
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
  } x11_winrecord, *x11_wintype;

typedef const struct x11_winstruct *const_x11_wintype;

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

wintype find_window (Window curr_window);
void enter_window (wintype curr_window, Window xWin);
void remove_window (wintype curr_window, Window xWin);

#else

wintype find_window ();
void enter_window ();
void remove_window ();

#endif



#ifdef ANSI_C

void redraw (XExposeEvent *xexpose)
#else

void redraw (xexpose)
XExposeEvent *xexpose;
#endif

  {
    x11_wintype expose_window;
    int x, y, width, height;

  /* redraw */
#ifdef TRACE_X11
    printf("begin redraw\n");
#endif
    /* printf("XExposeEvent x=%d, y=%d, width=%d, height=%d, count=%d\n",
        xexpose->x, xexpose->y, xexpose->width, xexpose->height, xexpose->count); */
    expose_window = (x11_wintype) find_window(xexpose->window);
    /* XFlush(mydisplay);
       XSync(mydisplay, 0);
       getchar(); */
    XCopyArea(mydisplay, expose_window->backup,
        expose_window->window, mygc, xexpose->x, xexpose->y,
        xexpose->width, xexpose->height, xexpose->x, xexpose->y);
    /* printf("xexpose->x + xexpose->width=%d, to_width(expose_window)=%d\n",
           xexpose->x + xexpose->width, to_width(expose_window));
       printf("xexpose->y + xexpose->height=%d, to_height(expose_window)=%d\n",
           xexpose->y + xexpose->height, to_height(expose_window)); */
    if (xexpose->x + xexpose->width > to_width(expose_window)) {
      XSetForeground(mydisplay, mygc, mybackground);
      if (xexpose->x >= to_width(expose_window)) {
        x = xexpose->x;
        width = xexpose->width;
      } else {
        x = to_width(expose_window);
        width = xexpose->x + xexpose->width - to_width(expose_window);
        if (xexpose->y + xexpose->height > to_height(expose_window)) {
          if (xexpose->y >= to_height(expose_window)) {
            y = xexpose->y;
            height = xexpose->height;
          } else {
            y = to_height(expose_window);
            height = xexpose->y + xexpose->height - to_height(expose_window);
          } /* if */
          /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
              xexpose->x, y, to_width(expose_window) - xexpose->x, height); */
          XFillRectangle(mydisplay, to_window(expose_window), mygc,
              xexpose->x, y, to_width(expose_window) - xexpose->x, height);
        } /* if */
      } /* if */
      /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
          x, xexpose->y, width, xexpose->height); */
      XFillRectangle(mydisplay, to_window(expose_window), mygc,
          x, xexpose->y, width, xexpose->height);
    } else if (xexpose->y + xexpose->height > to_height(expose_window)) {
      XSetForeground(mydisplay, mygc, mybackground);
      if (xexpose->y >= to_height(expose_window)) {
        y = xexpose->y;
        height = xexpose->height;
      } else {
        y = to_height(expose_window);
        height = xexpose->y + xexpose->height - to_height(expose_window);
      } /* if */
      /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
          xexpose->x, y, xexpose->width, height); */
      XFillRectangle(mydisplay, to_window(expose_window), mygc,
          xexpose->x, y, xexpose->width, height);
    } /* if */
#ifdef TRACE_X11
    printf("end redraw\n");
#endif
  } /* redraw */



#ifdef ANSI_C

void doFlush (void)
#else

void doFlush ()
#endif

  { /* doFlush */
#ifdef TRACE_X11
    printf("doFlush\n");
#endif
    XFlush(mydisplay);
    /* XSync(mydisplay, 0); */
  } /* doFlush */



#ifdef ANSI_C

void flushBeforeRead (void)
#else

void flushBeforeRead ()
#endif

  { /* flushBeforeRead */
    /* Not necessary, since XNextEvent() does a flush */
  } /* flushBeforeRead */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void configure (XConfigureEvent *xconfigure)
#else

void configure (xconfigure)
XConfigureEvent *xconfigure;
#endif

  {
    x11_wintype configure_window;
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
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



#ifdef ANSI_C

static int get_highest_bit (unsigned long number)
#else

static int get_highest_bit (number)
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
    return result + highest_bit[number];
  } /* get_highest_bit */



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
    if (mydisplay != NULL) {
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

      mygc = XCreateGC(mydisplay, DefaultRootWindow(mydisplay), 0, 0);
      XSetBackground(mydisplay, mygc, mybackground);
      XSetForeground(mydisplay, mygc, myforeground);
    } /* if */
#ifdef TRACE_X11
    printf("END dra_init\n");
#endif
  } /* dra_init */



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
    return win_x;
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
    return win_y;
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
    int startAng, sweepAng;

  /* drwArc */
#ifdef TRACE_X11
    printf("drwArc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
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
    int startAng, sweepAng;

  /* drwPArc */
#ifdef TRACE_X11
    printf("drwPArc(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
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
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
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

  {
    int startAng, sweepAng;

  /* drwPFArcChord */
#ifdef TRACE_X11
    printf("drwPFArcChord(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XSetArcMode(mydisplay, mygc, ArcChord);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
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
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
    } /* if */
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
    int startAng, sweepAng;

  /* drwPFArcPieSlice */
#ifdef TRACE_X11
    printf("drwPFArcPieSlice(%lu, %ld, %ld, %ld, %.4f, %.4f)\n",
        actual_window, x, y, radius, startAngle, sweepAngle);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius),
        startAng, sweepAng);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
      XFillArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius),
          startAng, sweepAng);
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
/*  printf("drwArc2(%d, %d, %d, %d)\n", x1, y1, radius); */
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
    printf("drwCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
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
    printf("drwPCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
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
    printf("drwCopyArea(%lu, %lu, %lu, %ld, %ld, %ld, %ld, %ld)\n",
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
    printf("drwFCircle(%lu, %ld, %ld, %ld)\n", actual_window, x, y, radius);
#endif
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
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
    printf("drwPFCircle(%lu, %ld, %ld, %ld, %08lx)\n",
        actual_window, x, y, radius, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XFillArc(mydisplay, to_window(actual_window), mygc,
        x - radius, y - radius,
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          x - radius, y - radius,
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
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
    printf("drwFEllipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
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
    printf("drwPFEllipse(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            x, y, (unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
  } /* drwPFEllipse */



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
      remove_window(old_window, to_window(old_window));
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
          (unsigned int) DefaultDepth(mydisplay, myscreen));
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
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

  {
    XImage *image;
    long pixel;

  /* drwGetPixel */
#ifdef TRACE_X11
    printf("drwGetPixel(%lu, %ld, %ld)\n", actual_window, x, y);
#endif
    if (to_backup(actual_window) != 0) {
      image = XGetImage(mydisplay, to_backup(actual_window), x, y, 1, 1,
                        (unsigned long) -1, ZPixmap);
    } else {
      image = XGetImage(mydisplay, to_window(actual_window), x, y, 1, 1,
                        (unsigned long) -1, ZPixmap);
    } /* if */
    pixel = XGetPixel(image, 0, 0);
    XDestroyImage(image);
    /* printf("drwGetPixel --> %lx\n", pixel); */
    return (inttype) pixel;
  } /* drwGetPixel */



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
#ifdef TRACE_X11
    printf("drwHeight(%lu) -> %u\n", actual_window, height);
#endif
    return (inttype) height;
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
    printf("drwImage(%ld, %ld)\n", width, height);
#endif
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
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
              to_window(actual_window), (unsigned int) width, (unsigned int) height,
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
    return (wintype) result;
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
    printf("drwLine(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x1, y1, x2, y2);
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

wintype drwNewPixmap (inttype width, inttype height)
#else

wintype drwNewPixmap (width, height)
inttype width;
inttype height;
#endif

  {
    x11_wintype result;

  /* drwNewPixmap */
#ifdef TRACE_X11
    printf("drwNewPixmap(%ld, %ld)\n", width, height);
#endif
    result = NULL;
    if (width < 1 || height < 1) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        dra_init();
      } /* if */
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
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
    } /* if */
    return (wintype) result;
  } /* drwNewBitmap */



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
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
      } else {
        win_name = cp_to_cstri(window_name);
        if (win_name == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
            memset(result, 0, sizeof(struct x11_winstruct));
            result->usage_count = 1;

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
      if (mydisplay == NULL) {
        raise_error(FILE_ERROR);
      } else {
        if (ALLOC_RECORD(result, x11_winrecord, count.win)) {
          memset(result, 0, sizeof(struct x11_winstruct));
          result->usage_count = 1;

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



#ifdef ANSI_C

void drwPoint (const_wintype actual_window, inttype x, inttype y)
#else

void drwPoint (actual_window, x, y)
wintype actual_window;
inttype x, y;
#endif

  { /* drwPoint */
#ifdef TRACE_X11
    printf("drwPoint(%lu, %ld, %ld)\n", actual_window, x, y);
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
    return result;
  } /* drwGenPointList */



#ifdef ANSI_C

inttype drwLngPointList (bstritype point_list)
#else

inttype drwLngPointList (point_list)
bstritype point_list;
#endif

  { /* drwLngPointList */
    return (inttype) (point_list->size / sizeof(XPoint));
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
    npoints = (int) (point_list->size / sizeof(XPoint));
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
    npoints = (int) (point_list->size / sizeof(XPoint));
    memcpy(&startBackup, &points[0], sizeof(XPoint));
    points[0].x += x1;
    points[0].y += y1;
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XDrawLines(mydisplay, to_window(actual_window), mygc, points, npoints, CoordModePrevious);
    XFillPolygon(mydisplay, to_window(actual_window), mygc, points, npoints,
        Nonconvex, CoordModePrevious);
    if (to_backup(actual_window) != 0) {
      XDrawLines(mydisplay, to_backup(actual_window), mygc, points, npoints, CoordModePrevious);
      XFillPolygon(mydisplay, to_backup(actual_window), mygc, points, npoints,
          Nonconvex, CoordModePrevious);
    } /* if */
    memcpy(&points[0], &startBackup, sizeof(XPoint));
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
        XSetClipOrigin(mydisplay, mygc, x, y);
      } /* if */
      XCopyArea(mydisplay, to_window(pixmap), to_window(actual_window),
          mygc, 0, 0, to_width(pixmap), to_height(pixmap), x, y);
      if (to_backup(actual_window) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(actual_window),
            mygc, 0, 0, to_width(pixmap), to_height(pixmap), x, y);
      } /* if */
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, None);
      } /* if */
    } /* if */
  } /* drwPut */



#ifdef ANSI_C

void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype length_x, inttype length_y)
#else

void drwRect (actual_window, x, y, length_x, length_y)
wintype actual_window;
inttype x, y, length_x, length_y;
#endif

  { /* drwRect */
#ifdef TRACE_X11
    printf("drwRect(%lu, %ld, %ld, %ld, %ld)\n", actual_window, x, y, length_x, length_y);
#endif
    XFillRectangle(mydisplay, to_window(actual_window), mygc, x, y,
        (unsigned) length_x, (unsigned) length_y);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, x, y,
          (unsigned) length_x, (unsigned) length_y);
    } /* if */
  } /* drwRect */



#ifdef ANSI_C

void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype length_x, inttype length_y, inttype col)
#else

void drwPRect (actual_window, x, y, length_x, length_y, col)
wintype actual_window;
inttype x, y, length_x, length_y;
inttype col;
#endif

  { /* drwPRect */
#ifdef TRACE_X11
    printf("drwPRect(%lu, %ld, %ld, %ld, %ld, %08lx)\n", actual_window, x, y, length_x, length_y, col);
#endif
    XSetForeground(mydisplay, mygc, (unsigned) col);
    XFillRectangle(mydisplay, to_window(actual_window), mygc, x, y,
        (unsigned) length_x, (unsigned) length_y);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, x, y,
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
    return (inttype) col.pixel;
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
    return col.pixel;
  } /* drwRgbColor */
#endif



#ifdef ANSI_C

void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)
#else

void drwPixelToRgb (col, red_val, green_val, blue_val)
inttype col;
inttype *red_val;
inttype *green_val;
inttype *blue_val;
#endif

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
      color.pixel = col;
      XQueryColor(mydisplay, cmap, &color);
      *red_val   = color.red;
      *green_val = color.green;
      *blue_val  = color.blue;
    } /* if */
  } /* drwPixelToRgb */



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



#ifdef ANSI_C

void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)
#else

void drwSetPos (actual_window, xPos, yPos)
wintype actual_window;
inttype xPos, yPos;
#endif

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
    XMoveWindow(mydisplay, to_window(actual_window), xPos, yPos);
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
  } /* drwText */



#ifdef ANSI_C

void drwToBottom (const_wintype actual_window)
#else

void drwToBottom (actual_window)
wintype actual_window;
#endif

  { /* drwToBottom */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XLowerWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToBottom */



#ifdef ANSI_C

void drwToTop (const_wintype actual_window)
#else

void drwToTop (actual_window)
wintype actual_window;
#endif

  { /* drwToTop */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XRaiseWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToTop */



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
#ifdef TRACE_X11
    printf("drwWidth(%lu) -> %u\n", actual_window, height);
#endif
    return (inttype) width;
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
#ifdef TRACE_X11
    printf("drwXPos(%lu) -> %d\n", actual_window, x);
#endif
    return (inttype) x;
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
#ifdef TRACE_X11
    printf("drwYPos(%lu) -> %d\n", actual_window, y);
#endif
    return (inttype) y;
  } /* drwYPos */
