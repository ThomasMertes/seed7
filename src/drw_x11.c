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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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


#ifndef C_PLUS_PLUS
#define c_class class
#endif


#define PI 3.141592653589793238462643383279502884197

Display *mydisplay = NULL;
Atom wm_delete_window;
static unsigned long myforeground;
static unsigned long mybackground;
static GC mygc;
static int myscreen;

typedef struct {
    uintType usage_count;
    /* Up to here the structure is identical to struct winStruct */
    Window window;
    Pixmap backup;
    Pixmap clip_mask;
    boolType is_pixmap;
    unsigned int width;
    unsigned int height;
    intType clear_col;
  } x11_winRecord, *x11_winType;

typedef const x11_winRecord *const_x11_winType;

#if DO_HEAP_STATISTIC
size_t sizeof_winRecord = sizeof(x11_winRecord);
#endif

#define to_window(win)    (((const_x11_winType) win)->window)
#define to_backup(win)    (((const_x11_winType) win)->backup)
#define to_clip_mask(win) (((const_x11_winType) win)->clip_mask)
#define is_pixmap(win)    (((const_x11_winType) win)->is_pixmap)
#define to_width(win)     (((const_x11_winType) win)->width)
#define to_height(win)    (((const_x11_winType) win)->height)
#define to_clear_col(win) (((const_x11_winType) win)->clear_col)

#define to_var_window(win)    (((x11_winType) win)->window)
#define to_var_backup(win)    (((x11_winType) win)->backup)
#define to_var_clip_mask(win) (((x11_winType) win)->clip_mask)
#define is_var_pixmap(win)    (((x11_winType) win)->is_pixmap)
#define to_var_width(win)     (((x11_winType) win)->width)
#define to_var_height(win)    (((x11_winType) win)->height)
#define to_var_clear_col(win) (((x11_winType) win)->clear_col)

static Visual *default_visual;

static int lshift_red;
static int rshift_red;
static int lshift_green;
static int rshift_green;
static int lshift_blue;
static int rshift_blue;

typedef struct colorEntryStruct *colorEntryType;

typedef struct colorEntryStruct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned long pixel;
    colorEntryType red_less;
    colorEntryType red_greater;
    colorEntryType green_less;
    colorEntryType green_greater;
    colorEntryType blue_less;
    colorEntryType blue_greater;
  } colorEntryRecord;

static const int highest_bit[16] = {
  0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4
};



winType find_window (Window sys_window);
void enter_window (winType curr_window, Window sys_window);
void remove_window (Window sys_window);



void redraw (winType redraw_window, int xPos, int yPos, int width, int height)

  {
    x11_winType expose_window;
    int xClear, yClear, clearWidth, clearHeight;

  /* redraw */
    logFunction(printf("redraw(" FMT_U_MEM ", %d, %d, %d, %d)\n",
                       (memSizeType) redraw_window, xPos, yPos, width, height););
    expose_window = (x11_winType) redraw_window;
    /* XFlush(mydisplay);
       XSync(mydisplay, 0);
       getchar(); */
    if (expose_window != NULL) {
      if (expose_window->backup != 0 &&
          xPos < to_width(expose_window) && yPos < to_height(expose_window)) {
        /* printf("XCopyArea: xPos=%d, yPos=%d, width=%d, height=%d\n",
            xPos, yPos, width, height); */
        XCopyArea(mydisplay, expose_window->backup,
            expose_window->window, mygc, xPos, yPos,
            width, height, xPos, yPos);
      } /* if */
      if (xPos + width > to_width(expose_window)) {
        XSetForeground(mydisplay, mygc,
            (unsigned long) to_clear_col(redraw_window));
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
        XSetForeground(mydisplay, mygc,
            (unsigned long) to_clear_col(redraw_window));
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
    } /* if */
    logFunction(printf("redraw -->\n"););
  } /* redraw */



void doFlush (void)

  { /* doFlush */
    logFunction(printf("doFlush\n"););
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
    x11_winType configure_window;
    Window root;
    /* int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth; */
    Status status;
    int num_events;
    XEvent peekEvent;

  /* configure */
    logFunction(printf("configure\n"););
    configure_window = (x11_winType) find_window(xconfigure->window);
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
              peekEvent.xexpose.x, peekEvent.xexpose.y, peekEvent.xexpose.width,
              peekEvent.xexpose.height, peekEvent.xexpose.count);
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
    logFunction(printf("configure -->\n"););
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



static void drawInit (void)

  {
#ifdef OUT_OF_ORDER
    const_cstriType class_text;
#endif

  /* drawInit */
    logFunction(printf("drawInit()\n"););
    /* When linking with a profiling standard library XOpenDisplay */
    /* deadlocked. Be careful to avoid this situation.             */
    mydisplay = XOpenDisplay("");
    /* printf("mydisplay = %lu\n", (long unsigned) mydisplay); */
    /* printf("DISPLAY=%s\n", getenv("DISPLAY")); */
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
    logFunction(printf("drawInit -->\n"););
  } /* drawInit */



static void setupBackup (x11_winType result)

  {
    Screen *x_screen;
    XSetWindowAttributes attributes;
    XWindowAttributes attribs;

  /* setupBackup */
    x_screen = XScreenOfDisplay(mydisplay, myscreen);
    /* printf("backing_store=%d (NotUseful=%d/WhenMapped=%d/Always=%d)\n",
        x_screen->backing_store, NotUseful, WhenMapped, Always); */
    if (x_screen->backing_store != NotUseful) {
      attributes.backing_store = x_screen->backing_store;
      XChangeWindowAttributes(mydisplay, result->window,
          CWBackingStore, &attributes);
      if (XGetWindowAttributes(mydisplay, result->window, &attribs) == 0 ||
          attribs.backing_store != Always) {
        /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
        result->backup = XCreatePixmap(mydisplay, result->window,
            result->width, result->height,
            (unsigned int) DefaultDepth(mydisplay, myscreen));
        /* printf("backup=%lu\n", (long unsigned) result->backup); */
      } /* if */
    } else {
      result->backup = XCreatePixmap(mydisplay, result->window,
          result->width, result->height,
          (unsigned int) DefaultDepth(mydisplay, myscreen));
      /* printf("backup=%lu\n", (long unsigned) result->backup); */
    } /* if */
  } /* setupBackup */



intType drwPointerXpos (const_winType actual_window)

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
    /* printf("drwPointerXpos --> %ld\n", win_x); */
    return win_x;
  } /* drwPointerXpos */



intType drwPointerYpos (const_winType actual_window)

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
    /* printf("drwPointerYpos --> %ld\n", win_y); */
    return win_y;
  } /* drwPointerYpos */



void drwArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwArc */
    logFunction(printf("drwArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
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



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPArc */
    logFunction(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
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



void drwFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcChord */
    logFunction(printf("drwPArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
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



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPFArcChord */
    logFunction(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
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



void drwFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcPieSlice */
    logFunction(printf("drwFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
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



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPFArcPieSlice */
    logFunction(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius, startAngle, sweepAngle););
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XSetArcMode(mydisplay, mygc, ArcPieSlice);
    startAng = (int) (startAngle * (23040.0 / (2 * PI)));
    sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
    if (unlikely(!inIntRange(x - radius) || !inIntRange(y - radius) ||
                 !inIntRange(radius) || radius < 0)) {
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



void drwArc2 (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType radius)

  { /* drwArc2 */
/*  printf("drwArc2(%d, %d, %d, %d)\n", x1, y1, radius); */
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x1 - radius), castToInt(y1 - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    XDrawLine(mydisplay, to_window(actual_window), mygc,
        castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
  } /* drwArc2 */



void drwCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwCircle */
    logFunction(printf("drwCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
    XDrawArc(mydisplay, to_window(actual_window), mygc,
        castToInt(x - radius), castToInt(y - radius),
        (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    if (to_backup(actual_window) != 0) {
      XDrawArc(mydisplay, to_backup(actual_window), mygc,
          castToInt(x - radius), castToInt(y - radius),
          (unsigned) (2 * radius), (unsigned) (2 * radius), 0, 23040);
    } /* if */
  } /* drwCircle */



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPCircle */
    logFunction(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
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



void drwClear (winType actual_window, intType col)

  { /* drwClear */
    logFunction(printf("drwClear(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, col););
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



void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y)

  { /* drwCopyArea */
    logFunction(printf("drwCopyArea(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) src_window, (memSizeType) dest_window,
                       src_x, src_y, width, height, dest_x, dest_y););
    if (unlikely(!inIntRange(src_x) || !inIntRange(src_y) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 !inIntRange(dest_x) || !inIntRange(dest_y) ||
                 width < 1 || height < 1)) {
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



void drwFCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwFCircle */
    logFunction(printf("drwFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
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



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPFCircle */
    logFunction(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
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



void drwFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwFEllipse */
    logFunction(printf("drwFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
    if (unlikely(width < 1 || height < 1)) {
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



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPFEllipse */
    logFunction(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
    if (unlikely(width < 1 || height < 1)) {
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



winType drwEmpty (void)

  {
    x11_winType result;

  /* drwEmpty */
    logFunction(printf("drwEmpty()\n"););
    if (unlikely(!ALLOC_RECORD(result, x11_winRecord, count.win))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(x11_winRecord));
      result->usage_count = 1;
      result->window = 0;
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = 0;
      result->height = 0;
    } /* if */
    logFunction(printf("drwEmpty --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwEmpty */



void drwFree (winType old_window)

  { /* drwFree */
    logFunction(printf("drwFree(" FMT_U_MEM ") (usage=" FMT_U ")\n",
                       (memSizeType) old_window,
                       old_window != NULL ? old_window->usage_count : (uintType) 0););
    if (is_pixmap(old_window)) {
      if (to_window(old_window) != 0) {
        XFreePixmap(mydisplay, to_window(old_window));
      } else {
        raise_error(MEMORY_ERROR);
      } /* if */
    } else {
      XDestroyWindow(mydisplay, to_window(old_window));
      if (to_backup(old_window) != 0) {
        XFreePixmap(mydisplay, to_backup(old_window));
      } /* if */
      remove_window(to_window(old_window));
    } /* if */
    FREE_RECORD(old_window, x11_winRecord, count.win);
  } /* drwFree */



winType drwGet (const_winType actual_window, intType left, intType upper,
    intType width, intType height)

  {
    x11_winType result;

  /* drwGet */
    logFunction(printf("drwGet(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(!ALLOC_RECORD(result, x11_winRecord, count.win))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(x11_winRecord));
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
    logFunction(printf("drwGet --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwGet */



bstriType drwGetImage (const_winType actual_window)

  {
    XImage *image;
    unsigned int xPos;
    unsigned int yPos;
    memSizeType result_size;
    int32Type *image_data;
    bstriType result;

  /* drwGetImage */
    logFunction(printf("drwGetImage(" FMT_U_MEM ")\n", (memSizeType) actual_window););
    if (to_backup(actual_window) != 0) {
      image = XGetImage(mydisplay, to_backup(actual_window),
                        0, 0, to_width(actual_window), to_height(actual_window),
                        (unsigned long) -1, ZPixmap);
    } else if (to_window(actual_window) != 0) {
      image = XGetImage(mydisplay, to_window(actual_window),
                        0, 0, to_width(actual_window), to_height(actual_window),
                        (unsigned long) -1, ZPixmap);
    } else {
      image = NULL;
    } /* if */
    result_size = to_width(actual_window) * to_height(actual_window) * sizeof(int32Type);
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      image_data = (int32Type *) result->mem;
      for (yPos = 0; yPos < to_height(actual_window); yPos++) {
        for (xPos = 0; xPos < to_width(actual_window); xPos++) {
          image_data[yPos * to_width(actual_window) + xPos] =
              (int32Type) XGetPixel(image, (int) xPos, (int) yPos);
        } /* for */
      } /* for */
    } /* if */
    if (image != NULL) {
      XDestroyImage(image);
    } /* if */
    return result;
  } /* drwGetImage */



intType drwGetPixel (const_winType actual_window, intType x, intType y)

  {
    XImage *image;
    intType pixel;

  /* drwGetPixel */
    logFunction(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y););
    if (to_backup(actual_window) != 0) {
      image = XGetImage(mydisplay, to_backup(actual_window),
                        castToInt(x), castToInt(y), 1, 1,
                        (unsigned long) -1, ZPixmap);
    } else {
      image = XGetImage(mydisplay, to_window(actual_window),
                        castToInt(x), castToInt(y), 1, 1,
                        (unsigned long) -1, ZPixmap);
    } /* if */
    pixel = (intType) XGetPixel(image, 0, 0);
    XDestroyImage(image);
    logFunction(printf("drwGetPixel --> " FMT_U "\n", pixel););
    return pixel;
  } /* drwGetPixel */



intType drwHeight (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwHeight */
    /* printf("drwHeight(" FMT_U_MEM "), usage=" FMT_U "\n",
       actual_window, actual_window != 0 ? actual_window->usage_count: 0); */
    if (is_pixmap(actual_window)) {
      height = to_height(actual_window);
    } else if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                        &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(RANGE_ERROR);
      height = 0;
    } /* if */
    logFunction(printf("drwHeight(" FMT_U_MEM ") --> %u\n",
                       (memSizeType) actual_window, height););
    return (intType) height;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height)

  {
    XImage *image;
    x11_winType result;

  /* drwImage */
    logFunction(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ")\n", width, height););
    if (unlikely(width < 1 || width > UINT_MAX ||
                 height < 1 || height > UINT_MAX)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (mydisplay == NULL) {
        drawInit();
      } /* if */
      if (unlikely(mydisplay == NULL)) {
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
          if (ALLOC_RECORD(result, x11_winRecord, count.win)) {
            memset(result, 0, sizeof(x11_winRecord));
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
    logFunction(printf("drwImage --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwImage */



void drwLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2)

  { /* drwLine */
    logFunction(printf("drwLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2););
    XDrawLine(mydisplay, to_window(actual_window), mygc,
              castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc,
                castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwLine */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  { /* drwPLine */
    logFunction(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2, col););
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawLine(mydisplay, to_window(actual_window), mygc,
              castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    if (to_backup(actual_window) != 0) {
      XDrawLine(mydisplay, to_backup(actual_window), mygc,
                castToInt(x1), castToInt(y1), castToInt(x2), castToInt(y2));
    } /* if */
  } /* drwPLine */



winType drwNewPixmap (intType width, intType height)

  {
    x11_winType result;

  /* drwNewPixmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(!inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (mydisplay == NULL) {
        drawInit();
      } /* if */
      if (unlikely(mydisplay == NULL)) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_RECORD(result, x11_winRecord, count.win))) {
          raise_error(MEMORY_ERROR);
        } else {
          memset(result, 0, sizeof(x11_winRecord));
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
    logFunction(printf("drwNewPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwNewPixmap */



winType drwNewBitmap (const_winType actual_window, intType width, intType height)

  {
    x11_winType result;

  /* drwNewBitmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(!ALLOC_RECORD(result, x11_winRecord, count.win))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(result, 0, sizeof(x11_winRecord));
      result->usage_count = 1;
      result->window = XCreatePixmap(mydisplay,
          to_window(actual_window), (unsigned int) width, (unsigned int) height, 1);
      result->backup = 0;
      result->clip_mask = 0;
      result->is_pixmap = TRUE;
      result->width = (unsigned int) width;
      result->height = (unsigned int) height;
    } /* if */
    logFunction(printf("drwNewBitmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwNewBitmap */



winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType window_name)

  {
    char *win_name;
    XSizeHints myhint;
    XWMHints mywmhint;
    XSetWindowAttributes attributes;
    errInfoType err_info = OKAY_NO_ERROR;
    x11_winType result;

  /* drwOpen */
    logFunction(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    result = NULL;
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        drawInit();
      } /* if */
      if (unlikely(mydisplay == NULL)) {
        logError(printf("drwOpen: drawInit() failed to open a display.\n"););
        raise_error(FILE_ERROR);
      } else {
        win_name = stri_to_cstri8(window_name, &err_info);
        if (unlikely(win_name == NULL)) {
          logError(printf("drwOpen: stri_to_cstri8(\"%s\") failed:\n"
                          "err_info=%d\n",
                          striAsUnquotedCStri(window_name), err_info););
          raise_error(err_info);
        } else {
          if (ALLOC_RECORD(result, x11_winRecord, count.win)) {
            memset(result, 0, sizeof(x11_winRecord));
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
            enter_window((winType) result, result->window);

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

            setupBackup(result);

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
            drwClear((winType) result, (intType) myforeground);
            XFlush(mydisplay);
            XSync(mydisplay, 0);
          } /* if */
          free_cstri8(win_name, window_name);
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
    logFunction(printf("drwOpen --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpen */



winType drwOpenSubWindow (const_winType parent_window, intType xPos, intType yPos,
    intType width, intType height)

  {
    XSizeHints myhint;
    /* XWMHints mywmhint; */
    XSetWindowAttributes attributes;
    x11_winType result;

  /* drwOpenSubWindow */
    logFunction(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    result = NULL;
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else {
      if (mydisplay == NULL) {
        drawInit();
      } /* if */
      if (unlikely(mydisplay == NULL)) {
        raise_error(FILE_ERROR);
      } else {
        if (ALLOC_RECORD(result, x11_winRecord, count.win)) {
          memset(result, 0, sizeof(x11_winRecord));
          result->usage_count = 1;

          myhint.x = (int) xPos;
          myhint.y = (int) yPos;
          myhint.width = (int) width;
          myhint.height = (int) height;
          myhint.flags = PPosition | PSize;
          /* mywmhint.flags = InputHint;
             mywmhint.input = True; */

          result->window = XCreateSimpleWindow(mydisplay,
              to_window(parent_window),
              (int) xPos, (int) yPos, (unsigned) width, (unsigned) height,
              0, myforeground, mybackground);
          enter_window((winType) result, result->window);

          result->backup = 0;
          result->clip_mask = 0;
          result->is_pixmap = FALSE;
          result->width = (unsigned int) width;
          result->height = (unsigned int) height;

          XSetStandardProperties(mydisplay, result->window,
              "", "",
              None, /* argv, argc, */ NULL, 0, &myhint);
          /* XSetWMHints(mydisplay, result->window, &mywmhint); */

          setupBackup(result);

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
          drwClear((winType) result, (intType) myforeground);
          /* XFlush(mydisplay);
             XSync(mydisplay, 0); */
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
    logFunction(printf("drwOpenSubWindow --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpenSubWindow */



void drwPoint (const_winType actual_window, intType x, intType y)

  { /* drwPoint */
    logFunction(printf("drwPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y););
    XDrawPoint(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y));
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y));
    } /* if */
  } /* drwPoint */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  { /* drwPPoint */
    logFunction(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, col););
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XDrawPoint(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y));
    if (to_backup(actual_window) != 0) {
      XDrawPoint(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y));
    } /* if */
  } /* drwPPoint */



void drwConvPointList (bstriType pointList, intType *xy)

  {
    memSizeType len;
    XPoint *points;
    memSizeType pos;

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



bstriType drwGenPointList (const const_rtlArrayType xyArray)

  {
    memSizeType num_elements;
    memSizeType len;
    intType x;
    intType y;
    XPoint *points;
    memSizeType pos;
    bstriType result;

  /* drwGenPointList */
    /* printf("drwGenPointList(%ld .. %ld)\n", xyArray->min_position, xyArray->max_position); */
    num_elements = arraySize(xyArray);
    if (unlikely(num_elements & 1)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      len = num_elements >> 1;
      if (unlikely(len > MAX_BSTRI_LEN / sizeof(XPoint) || len > MAX_MEM_INDEX)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, len * sizeof(XPoint)))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len * sizeof(XPoint);
          if (len > 0) {
            points = (XPoint *) result->mem;
            x = xyArray->arr[0].value.intValue;
            y = xyArray->arr[1].value.intValue;
            if (unlikely(x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX)) {
              raise_error(RANGE_ERROR);
              return NULL;
            } else {
              points[0].x = (short) x;
              points[0].y = (short) y;
            } /* if */
            for (pos = 1; pos < len; pos ++) {
              x = xyArray->arr[ pos << 1     ].value.intValue -
                  xyArray->arr[(pos << 1) - 2].value.intValue;
              y = xyArray->arr[(pos << 1) + 1].value.intValue -
                  xyArray->arr[(pos << 1) - 1].value.intValue;
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



intType drwLngPointList (bstriType point_list)

  { /* drwLngPointList */
    return (intType) (point_list->size / sizeof(XPoint));
  } /* drwLngPointList */



void drwPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

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



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

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



void drwPut (const_winType actual_window, const_winType pixmap,
    intType x, intType y)

  { /* drwPut */
    logFunction(printf("drwPut(" FMT_U_MEM ", " FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, (memSizeType) pixmap, x, y););
    /* printf("actual_window=%lu, pixmap=%lu\n", to_window(actual_window),
        pixmap != NULL ? to_window(pixmap) : NULL); */
    /* A pixmap value of NULL or a pixmap with a window of 0 */
    /* is used to describe an empty pixmap. In this case     */
    /* nothing should be done.                               */
    if (pixmap != NULL && to_window(pixmap) != 0) {
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



void drwRect (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwRect */
    logFunction(printf("drwRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
    XFillRectangle(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y),
        (unsigned) width, (unsigned) height);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y),
          (unsigned) width, (unsigned) height);
    } /* if */
  } /* drwRect */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPRect */
    logFunction(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, width, height, col););
    XSetForeground(mydisplay, mygc, (unsigned long) col);
    XFillRectangle(mydisplay, to_window(actual_window), mygc, castToInt(x), castToInt(y),
        (unsigned) width, (unsigned) height);
    if (to_backup(actual_window) != 0) {
      XFillRectangle(mydisplay, to_backup(actual_window), mygc, castToInt(x), castToInt(y),
          (unsigned) width, (unsigned) height);
    } /* if */
  } /* drwPRect */



intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  {
    static colorEntryType *color_hash = NULL;
    static unsigned long *pixels = NULL;
    static unsigned int num_pixels = 0;
    static unsigned int act_pixel = 0;
    unsigned int new_num_pixels;
    unsigned int pixel_incr;
    unsigned long *new_pixels;
    unsigned int hash_num;
    colorEntryType nearest_entry;
    colorEntryType *insert_place;
    colorEntryType entry;
    Colormap cmap;
    XColor col;
    int okay;

  /* drwRgbColor */
    logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ")\n", redLight, greenLight, blueLight););
    if (default_visual->c_class == TrueColor) {
      col.pixel =
          ((((unsigned long) redLight)   << lshift_red   >> rshift_red)   & default_visual->red_mask)   |
          ((((unsigned long) greenLight) << lshift_green >> rshift_green) & default_visual->green_mask) |
          ((((unsigned long) blueLight)  << lshift_blue  >> rshift_blue)  & default_visual->blue_mask);
      /*
      printf("((unsigned long) redLight): %08lx\n",
          ((unsigned long) redLight));
      printf("((unsigned long) redLight) << lshift_red: %08lx\n",
          ((unsigned long) redLight) << lshift_red);
      printf("((unsigned long) redLight) << lshift_red >> rshift_red: %08lx\n",
          ((unsigned long) redLight) << lshift_red >> rshift_red);
      printf("((unsigned long) greenLight): %08lx\n",
          ((unsigned long) greenLight));
      printf("((unsigned long) greenLight) << lshift_green: %08lx\n",
          ((unsigned long) greenLight) << lshift_green);
      printf("((unsigned long) greenLight) << lshift_green >> rshift_green: %08lx\n",
          ((unsigned long) greenLight) << lshift_green >> rshift_green);
      printf("((unsigned long) blueLight): %08lx\n",
          ((unsigned long) blueLight));
      printf("((unsigned long) blueLight) << lshift_blue: %08lx\n",
          ((unsigned long) blueLight) << lshift_blue);
      printf("((unsigned long) blueLight) << lshift_blue >> rshift_blue: %08lx\n",
          ((unsigned long) blueLight) << lshift_blue >> rshift_blue);
      printf("allocated [%04lx, %04lx, %04lx] color = %08lx\n",
          redLight, greenLight, blueLight, col.pixel);
      */
      /* printf("drwRgbColor(%ld, %ld, %ld) --> %lx\n", redLight, greenLight, blueLight, (intType) col.pixel); */
      return (intType) col.pixel;
    } /* if */
    if (color_hash == NULL) {
      color_hash = (colorEntryType *) malloc(4096 * sizeof(colorEntryType));
      if (color_hash == NULL) {
        printf("malloc color_hash failed for (%lu, %lu, %lu)\n",
            (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight);
        return 0;
      } /* if */
      memset(color_hash, 0, 4096 * sizeof(colorEntryType));
    } /* if */
    col.red   = (short unsigned int) redLight;
    col.green = (short unsigned int) greenLight;
    col.blue  = (short unsigned int) blueLight;
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
            redLight, greenLight, blueLight, entry->pixel); */
        return (intType) entry->pixel;
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
              new_num_pixels, (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight);
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
            (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight);
        okay = 0;
      } /* if */
    } else {
      if (nearest_entry != NULL) {
        printf("nearest_entry [%04lx, %04lx, %04lx] [%04x, %04x, %04x] color = %08lx\n",
            (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight,
            nearest_entry->red, nearest_entry->green, nearest_entry->blue,
            nearest_entry->pixel);
        return (intType) nearest_entry->pixel;
      } else {
        printf("return black [%04lx, %04lx, %04lx] color = %x\n",
            (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight, 0);
        return 0;
      } /* if */
    } /* if */
    if (!okay) {
      okay = 1;
      if (XAllocColor(mydisplay, cmap, &col) == 0) {
        printf("XAllocColor(%lu, %lu, %lu) not okay\n",
            (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight);
        okay = 0;
      } /* if */
    } /* if */
    if (okay) {
      entry = (colorEntryType) malloc(sizeof(colorEntryRecord));
      if (entry == NULL) {
        printf("drwRgbColor: malloc failed\n");
        col.pixel = 0;
      } else {
        memset(entry, 0, sizeof(colorEntryRecord));
        entry->red    = (unsigned short int) redLight;
        entry->green  = (unsigned short int) greenLight;
        entry->blue   = (unsigned short int) blueLight;
        entry->pixel  = col.pixel;
        *insert_place = entry;
      } /* if */
    } else {
      col.pixel = 0;
    } /* if */
    printf("allocated [%04lx, %04lx, %04lx] color = %08lx\n",
        (unsigned long) redLight, (unsigned long) greenLight, (unsigned long) blueLight, col.pixel);
    return (intType) col.pixel;
  } /* drwRgbColor */



#ifdef OUT_OF_ORDER
intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  {
    Colormap cmap;
    XColor col;
    char color_string[14];

  /* drwRgbColor */
    cmap = DefaultColormap(mydisplay, myscreen);
    col.red = redLight;
    col.green = greenLight;
    col.blue = blueLight;
    if (XAllocColor(mydisplay, cmap, &col) == 0) {
      /* handle failture */
      printf("XAllocColor(%ld, %ld, %ld) not okay\n",
          redLight, greenLight, blueLight);
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
    printf("XParseColor(\"%s\" [%ld, %ld, %ld]) --> [%ld, %ld, %ld]\n",
           color_string, RED_VAL, GREEN_VAL, BLUE_VAL,
        col.red, col.green, col.blue);
    printf("rgb color = %08lx\n", (long) col.pixel);
    return col.pixel;
  } /* drwRgbColor */
#endif



void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight)

  {
    Colormap cmap;
    XColor color;

  /* drwPixelToRgb */
    if (default_visual->c_class == TrueColor) {
      *redLight   = (intType)(((unsigned long) col & default_visual->red_mask)   << rshift_red   >> lshift_red);
      *greenLight = (intType)(((unsigned long) col & default_visual->green_mask) << rshift_green >> lshift_green);
      *blueLight  = (intType)(((unsigned long) col & default_visual->blue_mask)  << rshift_blue  >> lshift_blue);
    } else {
      cmap = DefaultColormap(mydisplay, myscreen);
      color.pixel = (unsigned long) col;
      XQueryColor(mydisplay, cmap, &color);
      *redLight   = color.red;
      *greenLight = color.green;
      *blueLight  = color.blue;
    } /* if */
    /* printf("drwPixelToRgb(%lx) --> %ld %ld %ld\n", col, *redLight, *greenLight, *blueLight); */
  } /* drwPixelToRgb */



void drwBackground (intType col)

  { /* drwBackground */
/*  printf("set color = %ld\n", (long) col); */
    XSetBackground(mydisplay, mygc, (unsigned long) col);
  } /* drwBackground */



void drwColor (intType col)

  { /* drwColor */
/*  printf("set color = %ld\n", (long) col); */
    XSetForeground(mydisplay, mygc, (unsigned long) col);
  } /* drwColor */



void drwSetContent (const_winType actual_window, const_winType pixmap)

  { /* drwSetContent */
    logFunction(printf("drwSetContent(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) actual_window, (memSizeType) pixmap););
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



void drwSetPos (const_winType actual_window, intType xPos, intType yPos)

  { /* drwSetPos */
    /* printf("begin drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
    XMoveWindow(mydisplay, to_window(actual_window), castToInt(xPos), castToInt(yPos));
    /* printf("end drwSetPos(%lu, %ld, %ld)\n",
        to_window(actual_window), xPos, yPos); */
  } /* drwSetPos */



void drwSetTransparentColor (winType pixmap, intType col)

  {
    GC bitmap_gc;
    int depth;
    int plane;
    unsigned long plane_mask;

  /* drwSetTransparentColor */
    logFunction(printf("drwSetTransparentColor(" FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) pixmap, col);
                printf("pixmap=" FMT_U_MEM "\n",
                       pixmap != NULL ? to_window(pixmap) : (memSizeType) 0););
    /* A pixmap value of NULL or a pixmap with a window of 0 */
    /* is used to describe an empty pixmap. In this case     */
    /* nothing should be done.                               */
    if (pixmap != NULL && to_window(pixmap) != 0) {
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



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  {
    XChar2b *stri_buffer;
    XChar2b *wstri;
    const strElemType *strelem;
    memSizeType len;

  /* drwText */
    logFunction(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D ", ...)\n",
                       (memSizeType) actual_window, x, y););
    stri_buffer = (XChar2b *) malloc(sizeof(XChar2b) * stri->size);
    if (stri_buffer != NULL) {
      wstri = stri_buffer;
      strelem = stri->mem;
      len = stri->size;
      for (; len > 0; wstri++, strelem++, len--) {
        if (unlikely(*strelem >= 65536)) {
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



void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XLowerWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToBottom */



void drwToTop (const_winType actual_window)

  { /* drwToTop */
    /* printf("begin drwRaise(%lu)\n", to_window(actual_window)); */
    XRaiseWindow(mydisplay, to_window(actual_window));
    /* printf("end drwRaise(%lu)\n", to_window(actual_window)); */
  } /* drwToTop */



intType drwWidth (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwWidth */
    /* printf("drwWidth(" FMT_U_MEM "), usage=" FMT_U "\n",
       actual_window, actual_window != 0 ? actual_window->usage_count: 0); */
    if (is_pixmap(actual_window)) {
      width = to_width(actual_window);
    } else if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                        &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(RANGE_ERROR);
      width = 0;
    } /* if */
    logFunction(printf("drwWidth(" FMT_U_MEM ") --> %u\n",
                       (memSizeType) actual_window, width););
    return (intType) width;
  } /* drwWidth */



intType drwXPos (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwXPos */
    logFunction(printf("drwXPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                 &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(RANGE_ERROR);
      x = 0;
    } /* if */
    logFunction(printf("drwXPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, x););
    return (intType) x;
  } /* drwXPos */



intType drwYPos (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwYPos */
    logFunction(printf("drwYPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                 &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(RANGE_ERROR);
      y = 0;
    } /* if */
    logFunction(printf("drwYPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, y););
    return (intType) y;
  } /* drwYPos */
