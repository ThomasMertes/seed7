/********************************************************************/
/*                                                                  */
/*  drw_x11.c     Graphic access using the X11 capabilities.        */
/*  Copyright (C) 1989 - 2007, 2009 - 2017  Thomas Mertes           */
/*                2019 - 2024  Thomas Mertes                        */
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
/*  Changes: 1994, 2000 - 2011, 2013 - 2017  Thomas Mertes          */
/*           2019 - 2021  Thomas Mertes                             */
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

#ifdef X11_INCLUDE
#include X11_INCLUDE
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#ifdef WITH_XSHAPE_EXTENSION
#include <X11/extensions/shape.h>
#endif
#endif
#ifdef X11_XRENDER_INCLUDE
#include X11_XRENDER_INCLUDE
#else
#include <X11/extensions/Xrender.h>
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "kbd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"


#ifndef C_PLUS_PLUS
#define c_class class
#endif

#if FORWARD_X11_CALLS
boolType findX11Dll (void);
#else
#define findX11Dll() TRUE
#endif

#define MAX_XCHAR2B_LEN   (MAX_MEMSIZETYPE / sizeof(XChar2b))
#define PI 3.141592653589793238462643383279502884197

Display *mydisplay = NULL;
Atom net_wm_name;
Atom utf8_string;
Atom wm_delete_window;
Atom motifWmHintsProperty;
Atom net_wm_state;
Atom net_wm_state_skip_taskbar;
static unsigned long myforeground;
static unsigned long mybackground;
static Cursor emptyCursor;
static GC mygc;
static int myscreen;

static boolType init_called = FALSE;

typedef struct {
    uintType usage_count;
    /* Up to here the structure is identical to struct winStruct */
    Window window;
    Pixmap backup;
    Pixmap clip_mask;
    boolType is_pixmap;
    boolType is_managed;
    unsigned int width;
    unsigned int height;
    unsigned int backupWidth;
    unsigned int backupHeight;
    intType clear_col;
    boolType resizeReturnsKey;
    int close_action;
  } x11_winRecord, *x11_winType;

typedef const x11_winRecord *const_x11_winType;

#define to_window(win)           (((const_x11_winType) (win))->window)
#define to_backup(win)           (((const_x11_winType) (win))->backup)
#define to_clip_mask(win)        (((const_x11_winType) (win))->clip_mask)
#define is_pixmap(win)           (((const_x11_winType) (win))->is_pixmap)
#define is_managed(win)          (((const_x11_winType) (win))->is_managed)
#define to_width(win)            (((const_x11_winType) (win))->width)
#define to_height(win)           (((const_x11_winType) (win))->height)
#define to_backupWidth(win)      (((const_x11_winType) (win))->backupWidth)
#define to_backupHeight(win)     (((const_x11_winType) (win))->backupHeight)
#define to_clear_col(win)        (((const_x11_winType) (win))->clear_col)
#define to_resizeReturnsKey(win) (((const_x11_winType) (win))->resizeReturnsKey)
#define to_close_action(win)     (((const_x11_winType) (win))->close_action)

#define to_var_window(win)           (((x11_winType) (win))->window)
#define to_var_backup(win)           (((x11_winType) (win))->backup)
#define to_var_clip_mask(win)        (((x11_winType) (win))->clip_mask)
#define is_var_pixmap(win)           (((x11_winType) (win))->is_pixmap)
#define is_var_managed(win)          (((x11_winType) (win))->is_managed)
#define to_var_width(win)            (((x11_winType) (win))->width)
#define to_var_height(win)           (((x11_winType) (win))->height)
#define to_var_clear_col(win)        (((x11_winType) (win))->clear_col)
#define to_var_resizeReturnsKey(win) (((x11_winType) (win))->resizeReturnsKey)
#define to_var_close_action(win)     (((x11_winType) (win))->close_action)

static winType emptyWindow = NULL;
static Visual *default_visual;
static boolType usesTrueColor = FALSE;

#ifdef rgbToPixel
int useRgbToPixel = 0;
#endif

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
void gkbInitKeyboard (void);
void gkbCloseKeyboard (void);



int getCloseAction (winType actual_window)

  { /* getCloseAction */
    logFunction(printf("getCloseAction(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    return actual_window != NULL ? to_close_action(actual_window)
                                 : CLOSE_BUTTON_RAISES_EXCEPTION;
  } /* getCloseAction */



Window getWindowParent (Window window)

  {
    Window root;
    Window parent;
    Window *children = NULL;
    unsigned int numChildren;

  /* getWindowParent */
    if(XQueryTree(mydisplay, window, &root, &parent, &children, &numChildren) == 0) {
      parent = 0;
    } else if (children != NULL) {
      XFree((char *) children);
    } /* if */
    logFunction(printf("getWindowParent(" FMT_U_MEM ") --> " FMT_U_MEM "\n",
                       window, parent););
    return parent;
  } /* getWindowParent */



void redraw (winType redrawWindow, int xPos, int yPos, unsigned int width, unsigned int height)

  {
    x11_winType x11Window;
    int xClear, yClear;
    unsigned int clearWidth, clearHeight;

  /* redraw */
    logFunction(printf("redraw(" FMT_U_MEM ", %d, %d, %d, %d)\n",
                       (memSizeType) redrawWindow, xPos, yPos, width, height););
    x11Window = (x11_winType) redrawWindow;
    /* XFlush(mydisplay);
       XSync(mydisplay, 0);
       getchar(); */
    if (x11Window != NULL) {
      if (x11Window->backup != 0 &&
          xPos < (int) x11Window->width && yPos < (int) x11Window->height) {
        /* printf("XCopyArea: xPos=%d, yPos=%d, width=%d, height=%d\n",
            xPos, yPos, width, height); */
        XCopyArea(mydisplay, x11Window->backup,
            x11Window->window, mygc, xPos, yPos,
            width, height, xPos, yPos);
      } /* if */
      if (xPos + (int) width > (int) x11Window->width) {
        XSetForeground(mydisplay, mygc, (unsigned long) x11Window->clear_col);
        if (xPos >= (int) x11Window->width) {
          xClear = xPos;
          clearWidth = width;
        } else {
          xClear = (int) x11Window-> width;
          clearWidth = (unsigned int) (xPos + (int) width - (int) x11Window->width);
          if (yPos + (int) height > (int) x11Window->height) {
            if (yPos >= (int) x11Window->height) {
              yClear = yPos;
              clearHeight = height;
            } else {
              yClear = (int) x11Window->height;
              clearHeight = (unsigned int) (yPos + (int) height - (int) x11Window->height);
            } /* if */
            /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
                xPos, yClear, x11Window->width - xPos, clearHeight); */
            XFillRectangle(mydisplay, x11Window->window, mygc,
                           xPos, yClear, x11Window->width - (unsigned int) xPos, clearHeight);
          } /* if */
        } /* if */
        /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
            xClear, yPos, clearWidth, height); */
        XFillRectangle(mydisplay, x11Window->window, mygc,
            xClear, yPos, clearWidth, height);
      } else if (yPos + (int) height > (int) x11Window->height) {
        XSetForeground(mydisplay, mygc, (unsigned long) x11Window->clear_col);
        if (yPos >= (int) x11Window->height) {
          yClear = yPos;
          clearHeight = height;
        } else {
          yClear = (int) x11Window->height;
          clearHeight = (unsigned int) (yPos + (int) height - (int) x11Window->height);
        } /* if */
        /* printf("clear x=%d, y=%d, width=%d, height=%d\n",
            xPos, yClear, width, clearHeight); */
        XFillRectangle(mydisplay, x11Window->window, mygc,
            xPos, yClear, width, clearHeight);
      } /* if */
    } /* if */
    logFunction(printf("redraw -->\n"););
  } /* redraw */



boolType resize (winType resizeWindow, unsigned int width, unsigned int height)

  {
    x11_winType x11Window;
    Pixmap newBackup;
    unsigned int newWidth;
    unsigned int newHeight;
    boolType sendResizeNotification = FALSE;

  /* resize */
    logFunction(printf("resize(" FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) resizeWindow, width, height););
    x11Window = (x11_winType) resizeWindow;
    if (x11Window != NULL && x11Window->backup != 0) {
      if (x11Window->backupWidth < width || x11Window->backupHeight < height) {
        newWidth = x11Window->backupWidth > width ? x11Window->backupWidth : width;
        newHeight = x11Window->backupHeight > height ? x11Window->backupHeight : height;
        newBackup = XCreatePixmap(mydisplay, x11Window->window,
            newWidth, newHeight,
            (unsigned int) DefaultDepth(mydisplay, myscreen));
        XSetForeground(mydisplay, mygc, (unsigned long) x11Window->clear_col);
        XFillRectangle(mydisplay, newBackup, mygc, 0, 0, newWidth, newHeight);
        XCopyArea(mydisplay, x11Window->backup, newBackup, mygc,
                  0, 0, x11Window->backupWidth, x11Window->backupHeight, 0, 0);
        XFreePixmap(mydisplay, x11Window->backup);
        x11Window->backup = newBackup;
        x11Window->backupWidth = newWidth;
        x11Window->backupHeight = newHeight;
      } /* if */
      sendResizeNotification = x11Window->resizeReturnsKey &&
          (x11Window->width != width || x11Window->height != height);
      x11Window->width = width;
      x11Window->height = height;
    } /* if */
    logFunction(printf("resize --> %d\n", sendResizeNotification););
    return sendResizeNotification;
  } /* resize */



boolType isResize (winType resizeWindow, unsigned int width, unsigned int height)

  {
    x11_winType x11Window;
    boolType sendResizeNotification = FALSE;

  /* isResize */
    logFunction(printf("isResize(" FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) resizeWindow, width, height););
    x11Window = (x11_winType) resizeWindow;
    if (x11Window != NULL && x11Window->backup != 0 && x11Window->resizeReturnsKey) {
      sendResizeNotification = x11Window->width != width || x11Window->height != height;
    } /* if */
    logFunction(printf("resize --> %d\n", sendResizeNotification););
    return sendResizeNotification;
  } /* isResize */



void setResizeReturnsKey (winType resizeWindow, boolType active)

  { /* setResizeReturnsKey */
    to_var_resizeReturnsKey(resizeWindow) = active;
  } /* setResizeReturnsKey */



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



static winType generateEmptyWindow (void)

  {
    x11_winType newWindow;

  /* generateEmptyWindow */
    logFunction(printf("generateEmptyWindow()\n"););
    if (unlikely(!ALLOC_RECORD2(newWindow, x11_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(newWindow, 0, sizeof(x11_winRecord));
      newWindow->usage_count = 0;  /* Do not use reference counting (will not be freed). */
      newWindow->window = 0;
      newWindow->backup = 0;
      newWindow->clip_mask = 0;
      newWindow->is_pixmap = TRUE;
      newWindow->is_managed = FALSE;
      newWindow->width = 0;
      newWindow->height = 0;
    } /* if */
    logFunction(printf("generateEmptyWindow --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) newWindow,
                       newWindow != NULL ? newWindow->usage_count : (uintType) 0););
    return (winType) newWindow;
  } /* generateEmptyWindow */



void drawClose (void)

  { /* drawClose */
    if (mydisplay != NULL) {
      if (emptyWindow != NULL) {
        XFreeGC(mydisplay, mygc);
        FREE_RECORD2(emptyWindow, x11_winRecord, count.win, count.win_bytes);
        emptyWindow = NULL;
      } /* if */
      XCloseDisplay(mydisplay);
      gkbCloseKeyboard();
    } /* if */
  } /* drawClose */



void drawInit (void)

  {
    char *displayVariable;
#ifdef OUT_OF_ORDER
    const_cstriType class_text;
#endif
    XColor color;
    static const char data[1] = {0};
    Pixmap blankPixmap;

  /* drawInit */
    logFunction(printf("drawInit()\n"););
    if (emptyWindow == NULL) {
      emptyWindow = generateEmptyWindow();
    } /* if */
    if (findX11Dll()) {
      /* After linking with a profiling standard library the function */
      /* XOpenDisplay deadlocked. Be careful to avoid this situation. */
      mydisplay = XOpenDisplay(NULL);
      logMessage(printf("drawInit(): XOpenDisplay(NULL) returned "
                        FMT_U_MEM "\n", (memSizeType) mydisplay););
      if (mydisplay == NULL) {
        displayVariable = getenv("DISPLAY");
        logError(printf("drawInit(): XOpenDisplay(NULL) failed.\n"
                        "DISPLAY=");
                 if (displayVariable == NULL) {
                   printf("NULL\n");
                 } else {
                   printf("\"%s\"\n", displayVariable);
                 });
        if (displayVariable != NULL) {
          /* This should have happened with XOpenDisplay(NULL). */
          mydisplay = XOpenDisplay(displayVariable);
          logMessage(printf("drawInit(): XOpenDisplay(\"%s\") returned "
                            FMT_U_MEM "\n", displayVariable,
                            (memSizeType) mydisplay););
          if (mydisplay == NULL) {
            logError(printf("drawInit(): XOpenDisplay(\"%s\") failed.\n",
                            displayVariable););
          } /* if */
        } /* if */
        if (mydisplay == NULL) {
          mydisplay = XOpenDisplay(":0");
          logMessage(printf("drawInit(): XOpenDisplay(\":0\") returned "
                            FMT_U_MEM "\n", (memSizeType) mydisplay););
          if (mydisplay == NULL) {
            logError(printf("drawInit(): XOpenDisplay(\":0\") failed.\n"););
          } /* if */
        } /* if */
      } /* if */
    } else {
      logError(printf("drawInit(): findX11Dll() failed.\n"););
    } /* if */
    if (emptyWindow != NULL && mydisplay != NULL) {
      myscreen = DefaultScreen(mydisplay);
      logMessage(printf("drawInit(): DefaultScreen(" FMT_U_MEM
                        ") returned %d\n",
                        (memSizeType) mydisplay, myscreen););
      default_visual = XDefaultVisual(mydisplay, myscreen);
      usesTrueColor = default_visual->c_class == TrueColor;
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
#ifdef rgbToPixel
      useRgbToPixel = usesTrueColor &&
                      rgbToPixel(0xffff, 0, 0) == default_visual->red_mask &&
                      rgbToPixel(0, 0xffff, 0) == default_visual->green_mask &&
                      rgbToPixel(0, 0, 0xffff) == default_visual->blue_mask;
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

      net_wm_name = XInternAtom(mydisplay, "_NET_WM_NAME", False);
      utf8_string = XInternAtom(mydisplay, "UTF8_STRING", False);
      wm_delete_window = XInternAtom(mydisplay, "WM_DELETE_WINDOW", False);
      motifWmHintsProperty = XInternAtom(mydisplay, "_MOTIF_WM_HINTS", True);
      net_wm_state = XInternAtom(mydisplay, "_NET_WM_STATE", False);
      net_wm_state_skip_taskbar = XInternAtom(mydisplay, "_NET_WM_STATE_SKIP_TASKBAR", False);
      blankPixmap = XCreateBitmapFromData(mydisplay, DefaultRootWindow(mydisplay), data, 1, 1);
      emptyCursor = XCreatePixmapCursor(mydisplay, blankPixmap, blankPixmap, &color, &color, 0, 0);
      XFreePixmap(mydisplay, blankPixmap);
      gkbInitKeyboard();
      init_called = TRUE;
    } else {
#ifdef rgbToPixel
      useRgbToPixel = 1;
#endif
    } /* if */
    logFunction(printf("drawInit --> (init_called=%d)\n", init_called););
  } /* drawInit */



static void setupBackup (x11_winType result)

  {
    Screen *x_screen;
    XSetWindowAttributes attributes;
    XWindowAttributes attribs;

  /* setupBackup */
    logFunction(printf("setupBackup()\n"););
    x_screen = XScreenOfDisplay(mydisplay, myscreen);
    /* printf("backing_store=%d (NotUseful=%d/WhenMapped=%d/Always=%d)\n",
        x_screen->backing_store, NotUseful, WhenMapped, Always); */
    if (DoesBackingStore(x_screen) != NotUseful) {
      attributes.backing_store = DoesBackingStore(x_screen);
      XChangeWindowAttributes(mydisplay, result->window,
          CWBackingStore, &attributes);
      if (XGetWindowAttributes(mydisplay, result->window, &attribs) == 0 ||
          attribs.backing_store != Always) {
        /* printf("backing_store=%d %d\n", attribs.backing_store, Always); */
        result->backup = XCreatePixmap(mydisplay, result->window,
            result->width, result->height,
            (unsigned int) DefaultDepth(mydisplay, myscreen));
        /* printf("backup=%lu\n", (long unsigned) result->backup); */
        result->backupWidth = result->width;
        result->backupHeight = result->height;
      } /* if */
    } else {
      result->backup = XCreatePixmap(mydisplay, result->window,
          result->width, result->height,
          (unsigned int) DefaultDepth(mydisplay, myscreen));
      /* printf("backup=%lu\n", (long unsigned) result->backup); */
      result->backupWidth = result->width;
      result->backupHeight = result->height;
    } /* if */
    logFunction(printf("setupBackup -->\n"););
  } /* setupBackup */



/**
 *  Return the X position of the pointer relative to the specified window.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'actual_window' (inside of the window decorations).
 *  If 'actual_window' is the empty window the pointer X position is
 *  relative to the top left corner of the screen.
 */
intType drwPointerXpos (const_winType actual_window)

  {
    Window window;
    Window root;
    Window child;
    int root_x, root_y;
    int xPos, yPos;
    unsigned int keys_buttons;

  /* drwPointerXpos */
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    window = to_window(actual_window);
    if (window == 0) {
      window = DefaultRootWindow(mydisplay);
    } /* if */
    XQueryPointer(mydisplay, window, &root, &child,
                  &root_x, &root_y, &xPos, &yPos, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, xPos, yPos, keys_buttons); */
    logFunction(printf("drwPointerXpos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, xPos););
    return (intType) xPos;
  } /* drwPointerXpos */



/**
 *  Return the Y position of the pointer relative to the specified window.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'actual_window' (inside of the window decorations).
 *  If 'actual_window' is the empty window the pointer Y position is
 *  relative to the top left corner of the screen.
 */
intType drwPointerYpos (const_winType actual_window)

  {
    Window window;
    Window root;
    Window child;
    int root_x, root_y;
    int xPos, yPos;
    unsigned int keys_buttons;

  /* drwPointerYpos */
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    window = to_window(actual_window);
    if (window == 0) {
      window = DefaultRootWindow(mydisplay);
    } /* if */
    XQueryPointer(mydisplay, window, &root, &child,
                  &root_x, &root_y, &xPos, &yPos, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, xPos, yPos, keys_buttons); */
    logFunction(printf("drwPointerYpos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, yPos););
    return (intType) yPos;
  } /* drwPointerYpos */



void drwArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwArc */
    logFunction(printf("drwArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f)\n", (memSizeType) actual_window,
                       x, y, radius, startAngle, sweepAngle););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", %.4f, %.4f): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle););
      raise_error(RANGE_ERROR);
    } else {
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
          startAng, sweepAng);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
            startAng, sweepAng);
      } /* if */
    } /* if */
  } /* drwArc */



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPArc */
    logFunction(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", %.4f, %.4f, " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
          startAng, sweepAng);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius),
            startAng, sweepAng);
      } /* if */
    } /* if */
  } /* drwPArc */



void drwPFArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle,
    intType width, intType col)

  {
    int startAng, sweepAng;
    unsigned int diameter;
    unsigned int lineWidth;

  /* drwPFArc */
    logFunction(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, width, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 || width < 1 ||
                 (unsigned int) width > 2 * (unsigned int) (radius) ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPFArc(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", %.4f, %.4f, " FMT_D ", " F_X(08) "): "
                       "Raises RANGE_ERROR\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, width, col););
      raise_error(RANGE_ERROR);
    } else {
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
      if ((width & 1) != 0) {
        diameter = (unsigned int) (2 * radius - width + 1);
        lineWidth = (unsigned int) width;
      } else {
        diameter = (unsigned int) (2 * radius - width + 2);
        lineWidth = (unsigned int) (width - 1);
      } /* if */
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XSetLineAttributes(mydisplay, mygc, lineWidth, LineSolid, CapButt, JoinMiter);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius + lineWidth / 2),
          (int) (y - radius + lineWidth / 2),
          diameter, diameter, startAng, sweepAng);
      if ((width & 1) == 0) {
        XDrawArc(mydisplay, to_window(actual_window), mygc,
            (int) (x - radius + lineWidth / 2 + 1),
            (int) (y - radius + lineWidth / 2 + 1),
            diameter - 2, diameter - 2, startAng, sweepAng);
      } /* if */
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius + lineWidth / 2),
            (int) (y - radius + lineWidth / 2),
            diameter, diameter, startAng, sweepAng);
        if ((width & 1) == 0) {
          XDrawArc(mydisplay, to_backup(actual_window), mygc,
              (int) (x - radius + lineWidth / 2 + 1),
              (int) (y - radius + lineWidth / 2 + 1),
              diameter - 2, diameter - 2, startAng, sweepAng);
        } /* if */
      } /* if */
      XSetLineAttributes(mydisplay, mygc, 0, LineSolid, CapButt, JoinMiter);
    } /* if */
  } /* drwPFArc */



void drwFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcChord */
    logFunction(printf("drwFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", %.4f, %.4f): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle););
      raise_error(RANGE_ERROR);
    } else {
      XSetArcMode(mydisplay, mygc, ArcChord);
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
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
  } /* drwFArcChord */



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPFArcChord */
    logFunction(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPFArcChord(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f, " F_X(08) ")"
                       ": Raises RANGE_ERROR\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XSetArcMode(mydisplay, mygc, ArcChord);
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
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
  } /* drwPFArcChord */



void drwFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle)

  {
    int startAng, sweepAng;

  /* drwFArcPieSlice */
    logFunction(printf("drwFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f)\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", %.4f, %.4f): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle););
      raise_error(RANGE_ERROR);
    } else {
      XSetArcMode(mydisplay, mygc, ArcPieSlice);
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
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
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  {
    int startAng, sweepAng;

  /* drwPFArcPieSlice */
    logFunction(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", %.4f, %.4f, " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius,
                       startAngle, sweepAngle, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPFArcPieSlice(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", %.4f, %.4f, " F_X(08) ")"
                      ": Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius,
                      startAngle, sweepAngle, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XSetArcMode(mydisplay, mygc, ArcPieSlice);
      startAng = (int) (startAngle * (23040.0 / (2 * PI)));
      sweepAng = (int) (sweepAngle * (23040.0 / (2 * PI)));
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



/**
 *  Determine the border widths of a window in pixels.
 *  These are the widths of the window decorations in the succession
 *  top, right, bottom, left.
 *  @return an array with border widths (top, right, bottom, left).
 */
rtlArrayType drwBorder (const_winType actual_window)

  {
    Window window;
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int innerWidth, innerHeight;
    unsigned int border_width;
    unsigned int depth;
    int leftBorder, topBorder;
    rtlArrayType border;

  /* drwBorder */
    logFunction(printf("drwBorder(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    window = to_window(actual_window);
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      border = NULL;
    } else if (is_managed(actual_window)) {
      if (unlikely(XGetGeometry(mydisplay, window, &root, &x, &y,
                                &innerWidth, &innerHeight, &border_width, &depth) == 0)) {
        raise_error(GRAPHIC_ERROR);
        border = NULL;
      } else {
        window = getWindowParent(window);
        width = 0;
        height = 0;
        leftBorder = 0;
        topBorder = 0;
        while (window != root && window != 0) {
          leftBorder += x;
          topBorder += y;
          if (unlikely(XGetGeometry(mydisplay, window, &root,
                       &x, &y, &width, &height, &border_width, &depth) == 0)) {
            raise_error(GRAPHIC_ERROR);
            window = 0;
            border = NULL;
          } else {
            window = getWindowParent(window);
          } /* if */
        } /* while */
        if (unlikely(!ALLOC_RTL_ARRAY(border, 4))) {
          raise_error(MEMORY_ERROR);
          border = NULL;
        } else {
          border->min_position = 1;
          border->max_position = 4;
          border->arr[0].value.intValue = (intType) topBorder;
          border->arr[1].value.intValue = (intType) ((int) (width - innerWidth) - leftBorder);
          border->arr[2].value.intValue = (intType) ((int) (height - innerHeight) - topBorder);
          border->arr[3].value.intValue = (intType) leftBorder;
        } /* if */
      } /* if */
    } else if (unlikely(XGetGeometry(mydisplay, window, &root,
                        &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      border = NULL;
    } else if (unlikely(!ALLOC_RTL_ARRAY(border, 4))) {
      raise_error(MEMORY_ERROR);
    } else {
      border->min_position = 1;
      border->max_position = 4;
      border->arr[0].value.intValue = (intType) border_width;
      border->arr[1].value.intValue = (intType) border_width;
      border->arr[2].value.intValue = (intType) border_width;
      border->arr[3].value.intValue = (intType) border_width;
    } /* if */
    logFunction(printf("drwBorder(" FMT_U_MEM ") -->"
                       " %s[" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D "]\n",
                       (memSizeType) actual_window,
                       border != NULL ? "" : "NULL ",
                       border != NULL ? border->arr[0].value.intValue : 0,
                       border != NULL ? border->arr[1].value.intValue : 0,
                       border != NULL ? border->arr[2].value.intValue : 0,
                       border != NULL ? border->arr[3].value.intValue : 0););
    return border;
  } /* drwBorder */



void drwCircle (const_winType actual_window,
    intType x, intType y, intType radius)

  { /* drwCircle */
    logFunction(printf("drwCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius););
      raise_error(RANGE_ERROR);
    } else {
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      } /* if */
    } /* if */
  } /* drwCircle */



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPCircle */
    logFunction(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      } /* if */
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
          to_backupWidth(actual_window), to_backupHeight(actual_window));
    } /* if */
  } /* drwClear */



/**
 *  Copy a rectangular area from 'src_window' to 'dest_window'.
 *  Coordinates are measured relative to the top left corner of the
 *  corresponding window drawing area (inside of the window decorations).
 *  @param src_window Source window.
 *  @param dest_window Destination window.
 *  @param src_x X-position of the top left corner of the source area.
 *  @param src_y Y-position of the top left corner of the source area.
 *  @param width Width of the rectangular area.
 *  @param height Height of the rectangular area.
 *  @param dest_x X-position of the top left corner of the destination area.
 *  @param dest_y Y-position of the top left corner of the destination area.
 */
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
    logFunction(printf("drwFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, radius););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius););
      raise_error(RANGE_ERROR);
    } else {
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      } /* if */
    } /* if */
  } /* drwFCircle */



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPFCircle */
    logFunction(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, radius, col););
    if (unlikely(radius < 0 || radius > UINT_MAX / 2 ||
                 x < INT_MIN + radius || x > INT_MAX ||
                 y < INT_MIN + radius || y > INT_MAX)) {
      logError(printf("drwPFCircle(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): Raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, radius, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          (int) (x - radius), (int) (y - radius),
          2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            (int) (x - radius), (int) (y - radius),
            2 * (unsigned int) (radius), 2 * (unsigned int) (radius), 0, 23040);
      } /* if */
    } /* if */
  } /* drwPFCircle */



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPFEllipse */
    logFunction(printf("drwPFEllipse(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, width, height, col););
    if (unlikely(width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y), (unsigned int) width, (unsigned int) height, 0, 23040);
      XFillArc(mydisplay, to_window(actual_window), mygc,
          castToInt(x), castToInt(y), (unsigned int) width, (unsigned int) height, 0, 23040);
      if (to_backup(actual_window) != 0) {
        XDrawArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y), (unsigned int) width, (unsigned int) height, 0, 23040);
        XFillArc(mydisplay, to_backup(actual_window), mygc,
            castToInt(x), castToInt(y), (unsigned int) width, (unsigned int) height, 0, 23040);
      } /* if */
    } /* if */
  } /* drwPFEllipse */



winType drwEmpty (void)

  { /* drwEmpty */
    logFunction(printf("drwEmpty()\n"););
    if (!init_called) {
      drawInit();
    } /* if */
    logFunction(printf("drwEmpty --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) emptyWindow,
                       emptyWindow != NULL ? emptyWindow->usage_count : (uintType) 0););
    return emptyWindow;
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
    FREE_RECORD2(old_window, x11_winRecord, count.win, count.win_bytes);
  } /* drwFree */



/**
 *  Capture a rectangular area from the screen.
 *  The function takes a screenshot of the rectangular area.
 *  The 'left' and 'upper' coordinates are measured relative to
 *  the top left corner of the screen.
 *  @param left X-position of the upper left corner of the capture area.
 *  @param upper Y-position of the upper left corner of the capture area.
 *  @param width Width of the capture area.
 *  @param height Height of the capture area.
 *  @return the content of the rectangular screen area as pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative.
 */
winType drwCapture (intType left, intType upper,
    intType width, intType height)

  {
    Window root;
    int x, y;
    unsigned int screenWidth, screenHeight;
    unsigned int border_width;
    unsigned int depth;
    XImage *image;
    x11_winType pixmap;

  /* drwCapture */
    logFunction(printf("drwCapture(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(XGetGeometry(mydisplay, DefaultRootWindow(mydisplay), &root,
                              &x, &y, &screenWidth, &screenHeight, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      pixmap = NULL;
    } else {
      image = XGetImage(mydisplay, DefaultRootWindow(mydisplay),
                        0, 0, screenWidth, screenHeight,
                        (unsigned long) -1, ZPixmap);
      if (unlikely(image == NULL)) {
        logFunction(printf("drwCapture(" FMT_D ", " FMT_D ", "
                           FMT_D ", " FMT_D "): XGetImage failed\n",
                           left, upper, width, height););
        raise_error(GRAPHIC_ERROR);
        pixmap = NULL;
      } else {
        if (unlikely(!ALLOC_RECORD2(pixmap, x11_winRecord, count.win, count.win_bytes))) {
          raise_error(MEMORY_ERROR);
        } else {
          memset(pixmap, 0, sizeof(x11_winRecord));
          pixmap->usage_count = 1;
          pixmap->window = XCreatePixmap(mydisplay, DefaultRootWindow(mydisplay),
              (unsigned int) width, (unsigned int) height,
              (unsigned int) DefaultDepth(mydisplay, myscreen));
          pixmap->backup = 0;
          pixmap->clip_mask = 0;
          pixmap->is_pixmap = TRUE;
          pixmap->is_managed = FALSE;
          pixmap->width = (unsigned int) width;
          pixmap->height = (unsigned int) height;
          XPutImage(mydisplay, pixmap->window, mygc, image, (int) left, (int) upper, 0, 0,
              (unsigned int) width, (unsigned int) height);
        } /* if */
      } /* if */
      XDestroyImage(image);
    } /* if */
    logFunction(printf("drwCapture --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwCapture */



intType drwGetPixel (const_winType sourceWindow, intType x, intType y)

  {
    XImage *image;
    intType pixel;

  /* drwGetPixel */
    logFunction(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, x, y););
    if (unlikely(!inIntRange(x) || !inIntRange(y))) {
      logError(printf("drwGetPixel(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "Raises RANGE_ERROR\n",
                      (memSizeType) sourceWindow, x, y););
      raise_error(RANGE_ERROR);
      pixel = 0;
    } else {
      if (to_backup(sourceWindow) != 0) {
        image = XGetImage(mydisplay, to_backup(sourceWindow),
                          (int) (x), (int) (y), 1, 1,
                          (unsigned long) -1, ZPixmap);
      } else {
        image = XGetImage(mydisplay, to_window(sourceWindow),
                          (int) (x), (int) (y), 1, 1,
                          (unsigned long) -1, ZPixmap);
      } /* if */
      pixel = (intType) XGetPixel(image, 0, 0);
      XDestroyImage(image);
    } /* if */
    logFunction(printf("drwGetPixel --> " F_X(08) "\n", pixel););
    return pixel;
  } /* drwGetPixel */



bstriType drwGetPixelData (const_winType sourceWindow)

  {
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int xPos;
    unsigned int yPos;
    memSizeType result_size;
    int32Type *image_data;
    bstriType result;

  /* drwGetPixelData */
    logFunction(printf("drwGetPixelData(" FMT_U_MEM ")\n", (memSizeType) sourceWindow););
    if (to_window(sourceWindow) == 0) {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } else {
      width = to_width(sourceWindow);
      height = to_height(sourceWindow);
      if (to_backup(sourceWindow) != 0) {
        image = XGetImage(mydisplay, to_backup(sourceWindow),
                          0, 0, width, height, (unsigned long) -1, ZPixmap);
      } else if (to_window(sourceWindow) != 0) {
        image = XGetImage(mydisplay, to_window(sourceWindow),
                          0, 0, width, height, (unsigned long) -1, ZPixmap);
      } else {
        image = NULL;
      } /* if */
      if (unlikely(image == NULL)) {
        logError(printf("drwGetPixelData(" FMT_U_MEM "): XGetImage failed\n",
                        (memSizeType) sourceWindow););
        raise_error(GRAPHIC_ERROR);
        result = NULL;
      } else {
        result_size = width * height * sizeof(int32Type);
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
          XDestroyImage(image);
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          image_data = (int32Type *) result->mem;
          for (yPos = 0; yPos < height; yPos++) {
            for (xPos = 0; xPos < width; xPos++) {
              *image_data = (int32Type) XGetPixel(image, (int) xPos, (int) yPos);
              image_data++;
            } /* for */
          } /* for */
          XDestroyImage(image);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* drwGetPixelData */



/**
 *  Create a new pixmap with the given 'width' and 'height' from 'sourceWindow'.
 *  A rectangle with the upper left corner at ('left', 'upper') and the given
 *  'width' and 'height' is copied from 'sourceWindow' to the new pixmap.
 *  The rectangle may extend to areas outside of 'sourceWindow'. The rectangle
 *  areas outside of 'sourceWindow' are colored with black.
 *  @return the created pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative or zero.
 */
winType drwGetPixmap (const_winType sourceWindow, intType left, intType upper,
    intType width, intType height)

  {
    x11_winType pixmap;

  /* drwGetPixmap */
    logFunction(printf("drwGetPixmap(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sourceWindow, left, upper, width, height););
    if (unlikely(!inIntRange(left) || !inIntRange(upper) ||
                 !inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(!ALLOC_RECORD2(pixmap, x11_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(pixmap, 0, sizeof(x11_winRecord));
      pixmap->usage_count = 1;
      pixmap->window = XCreatePixmap(mydisplay, to_window(sourceWindow),
          (unsigned int) width, (unsigned int) height,
          (unsigned int) DefaultDepth(mydisplay, myscreen));
      pixmap->backup = 0;
      pixmap->clip_mask = 0;
      pixmap->is_pixmap = TRUE;
      pixmap->is_managed = FALSE;
      pixmap->width = (unsigned int) width;
      pixmap->height = (unsigned int) height;
      if (left < 0 || upper < 0 ||
          left + width >= to_width(sourceWindow) ||
          upper + height >= to_height(sourceWindow)) {
        drwClear((winType) pixmap, (intType) myforeground);
      } /* if */
      if (to_backup(sourceWindow) != 0) {
        XCopyArea(mydisplay, to_backup(sourceWindow),
            pixmap->window, mygc, (int) left, (int) upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } else {
        XCopyArea(mydisplay, to_window(sourceWindow),
            pixmap->window, mygc, (int) left, (int) upper,
            (unsigned int) width, (unsigned int) height, 0, 0);
      } /* if */
      /* printf("XCopyArea(%ld, %ld, %ld, %ld)\n", left, upper, width, height); */
    } /* if */
    logFunction(printf("drwGetPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwGetPixmap */



/**
 *  Determine the height of the window drawing area in pixels.
 *  This excludes window decorations at top and bottom. Add top and bottom
 *  border widths to get the height inclusive window decorations.
 */
intType drwHeight (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwHeight */
    logFunction(printf("drwHeight(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    if (is_pixmap(actual_window)) {
      height = to_height(actual_window);
    } else if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                        &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      height = 0;
    } /* if */
    logFunction(printf("drwHeight(" FMT_U_MEM ") --> %u\n",
                       (memSizeType) actual_window, height););
    return (intType) height;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height,
    boolType hasAlphaChannel)

  {
    XImage *image;
    x11_winType pixmap;

  /* drwImage */
    logFunction(printf("drwImage(" FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       width, height, hasAlphaChannel););
    if (unlikely(width < 1 || width > UINT_MAX ||
                 height < 1 || height > UINT_MAX)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!init_called)) {
        logError(printf("drwImage: drawInit() failed to open a display.\n"););
        raise_error(GRAPHIC_ERROR);
        pixmap = NULL;
      } else {
        image = XCreateImage(mydisplay, default_visual,
            (unsigned int) DefaultDepth(mydisplay, myscreen),
            ZPixmap, 0, (char *) image_data,
            (unsigned int) width, (unsigned int) height, 32, 0);
        if (image == NULL) {
          pixmap = NULL;
        } else {
          if (ALLOC_RECORD2(pixmap, x11_winRecord, count.win, count.win_bytes)) {
            memset(pixmap, 0, sizeof(x11_winRecord));
            pixmap->usage_count = 1;
            pixmap->window = XCreatePixmap(mydisplay, DefaultRootWindow(mydisplay),
                (unsigned int) width, (unsigned int) height,
                (unsigned int) DefaultDepth(mydisplay, myscreen));
            pixmap->backup = 0;
            pixmap->clip_mask = 0;
            pixmap->is_pixmap = TRUE;
            pixmap->is_managed = FALSE;
            pixmap->width = (unsigned int) width;
            pixmap->height = (unsigned int) height;
            XPutImage(mydisplay, pixmap->window, mygc, image, 0, 0, 0, 0,
                (unsigned int) width, (unsigned int) height);
          } /* if */
          XFree(image);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("drwImage --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwImage */



void drwLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2)

  { /* drwLine */
    logFunction(printf("drwLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2););
    if (unlikely(!(inIntRange(x1) && inIntRange(y1) &&
                   inIntRange(x2) && inIntRange(y2)))) {
      logError(printf("drwLine(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " FMT_D "): raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x1, y1, x2, y2););
      raise_error(RANGE_ERROR);
    } else {
      XDrawLine(mydisplay, to_window(actual_window), mygc,
                (int) (x1), (int) (y1), (int) (x2), (int) (y2));
      if (to_backup(actual_window) != 0) {
        XDrawLine(mydisplay, to_backup(actual_window), mygc,
                  (int) (x1), (int) (y1), (int) (x2), (int) (y2));
      } /* if */
    } /* if */
  } /* drwLine */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  { /* drwPLine */
    logFunction(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x1, y1, x2, y2, col););
    if (unlikely(!(inIntRange(x1) && inIntRange(y1) &&
                   inIntRange(x2) && inIntRange(y2)))) {
      logError(printf("drwPLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x1, y1, x2, y2, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawLine(mydisplay, to_window(actual_window), mygc,
                (int) (x1), (int) (y1), (int) (x2), (int) (y2));
      if (to_backup(actual_window) != 0) {
        XDrawLine(mydisplay, to_backup(actual_window), mygc,
                  (int) (x1), (int) (y1), (int) (x2), (int) (y2));
      } /* if */
    } /* if */
  } /* drwPLine */



/**
 *  Create a new pixmap with the given 'width' and 'height'.
 *  @return the created pixmap.
 *  @exception RANGE_ERROR If 'height' or 'width' are negative or zero.
 */
winType drwNewPixmap (intType width, intType height)

  {
    x11_winType pixmap;

  /* drwNewPixmap */
    logFunction(printf("drwNewPixmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(!inIntRange(width) || !inIntRange(height) ||
                 width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!init_called)) {
        logError(printf("drwNewPixmap: drawInit() failed to open a display.\n"););
        raise_error(GRAPHIC_ERROR);
        pixmap = NULL;
      } else {
        if (unlikely(!ALLOC_RECORD2(pixmap, x11_winRecord, count.win, count.win_bytes))) {
          raise_error(MEMORY_ERROR);
        } else {
          memset(pixmap, 0, sizeof(x11_winRecord));
          pixmap->usage_count = 1;
          pixmap->window = XCreatePixmap(mydisplay, DefaultRootWindow(mydisplay),
              (unsigned int) width, (unsigned int) height,
              (unsigned int) DefaultDepth(mydisplay, myscreen));
          pixmap->backup = 0;
          pixmap->clip_mask = 0;
          pixmap->is_pixmap = TRUE;
          pixmap->is_managed = FALSE;
          pixmap->width = (unsigned int) width;
          pixmap->height = (unsigned int) height;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("drwNewPixmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwNewPixmap */



winType drwNewBitmap (const_winType actual_window, intType width, intType height)

  {
    x11_winType pixmap;

  /* drwNewBitmap */
    logFunction(printf("drwNewBitmap(" FMT_D ", " FMT_D ")\n", width, height););
    if (unlikely(width < 1 || height < 1)) {
      raise_error(RANGE_ERROR);
      pixmap = NULL;
    } else if (unlikely(!ALLOC_RECORD2(pixmap, x11_winRecord, count.win, count.win_bytes))) {
      raise_error(MEMORY_ERROR);
    } else {
      memset(pixmap, 0, sizeof(x11_winRecord));
      pixmap->usage_count = 1;
      pixmap->window = XCreatePixmap(mydisplay, to_window(actual_window),
          (unsigned int) width, (unsigned int) height, 1);
      pixmap->backup = 0;
      pixmap->clip_mask = 0;
      pixmap->is_pixmap = TRUE;
      pixmap->is_managed = FALSE;
      pixmap->width = (unsigned int) width;
      pixmap->height = (unsigned int) height;
    } /* if */
    logFunction(printf("drwNewBitmap --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) pixmap,
                       pixmap != NULL ? pixmap->usage_count : (uintType) 0););
    return (winType) pixmap;
  } /* drwNewBitmap */



winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType windowName)

  {
    char *winName8;
    memSizeType winName8Length;
    XSizeHints myhint;
    XWMHints mywmhint;
    XSetWindowAttributes attributes;
    x11_winType result = NULL;

  /* drwOpen */
    logFunction(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                       ", \"%s\")\n", xPos, yPos, width, height,
                       striAsUnquotedCStri(windowName)););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwOpen(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D
                      ", \"%s\"): Illegal window dimensions\n",
                      xPos, yPos, width, height,
                      striAsUnquotedCStri(windowName)););
      raise_error(RANGE_ERROR);
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!init_called)) {
        logError(printf("drwOpen: drawInit() failed to open a display.\n"););
        raise_error(GRAPHIC_ERROR);
      } else {
        winName8 = stri_to_cstri8_buf(windowName, &winName8Length);
        if (unlikely(winName8 == NULL)) {
          logError(printf("drwOpen: stri_to_cstri8_buf(\"%s\") failed\n",
                          striAsUnquotedCStri(windowName)););
          raise_error(MEMORY_ERROR);
        } else {
          if (ALLOC_RECORD2(result, x11_winRecord, count.win, count.win_bytes)) {
            memset(result, 0, sizeof(x11_winRecord));
            result->usage_count = 1;

            myhint.x = (int) xPos;
            myhint.y = (int) yPos;
            myhint.width = (int) width;
            myhint.height = (int) height;
            myhint.flags = PPosition | PSize;

            result->window = XCreateSimpleWindow(mydisplay,
                DefaultRootWindow(mydisplay),
                myhint.x, myhint.y, (unsigned) myhint.width, (unsigned) myhint.height,
                5, myforeground, mybackground);
            enter_window((winType) result, result->window);

            result->backup = 0;
            result->clip_mask = 0;
            result->is_pixmap = FALSE;
            result->is_managed = TRUE;
            result->width = (unsigned int) width;
            result->height = (unsigned int) height;

            XSetStandardProperties(mydisplay, result->window,
                winName8, winName8,
                None, /* argv, argc, */ NULL, 0, &myhint);
            XChangeProperty(mydisplay, result->window, net_wm_name,
                utf8_string, 8, PropModeReplace, (unsigned char *) winName8,
                (int) winName8Length);

            mywmhint.flags = InputHint;
            mywmhint.input = True;
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
                ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
                ExposureMask | FocusChangeMask | StructureNotifyMask);

            XMapRaised(mydisplay, result->window);
            drwClear((winType) result, (intType) myforeground);
            XFlush(mydisplay);
            XSync(mydisplay, 0);
          } /* if */
          free_cstri8(winName8, windowName);
        } /* if */
      } /* if */
    } /* if */
    /* printf("result=%lu\n", (long unsigned) result); */
    logFunction(printf("drwOpen --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return (winType) result;
  } /* drwOpen */



static void omitWindowDecorationsAndTaskbarEntry (x11_winType aWindow)

  {
    XWMHints mywmhint;
    typedef struct {
      unsigned long   flags;
      unsigned long   functions;
      unsigned long   decorations;
      long            inputMode;
      unsigned long   status;
    } motifHintType;
    motifHintType motifHints;
    Atom states[1];

  /* omitWindowDecorationsAndTaskbarEntry */
    aWindow->is_managed = TRUE;

    mywmhint.flags = InputHint;
    mywmhint.input = True;
    XSetWMHints(mydisplay, aWindow->window, &mywmhint);

    XSetWMProtocols(mydisplay, aWindow->window, &wm_delete_window, 1);

    /* Make sure that the window does not have decorations. */
    memset(&motifHints, 0, sizeof(motifHintType));
    motifHints.flags = 2;
    motifHints.decorations = 0;
    XChangeProperty(mydisplay, aWindow->window, motifWmHintsProperty,
                    motifWmHintsProperty, 32, PropModeReplace,
                    (unsigned char *) &motifHints, 5);

    states[0] = net_wm_state_skip_taskbar;
    XChangeProperty(mydisplay, aWindow->window, net_wm_state,
                    XA_ATOM, 32, PropModeReplace,
                    (unsigned char*) &states, 1);

    XMapWindow(mydisplay, aWindow->window);
  } /* omitWindowDecorationsAndTaskbarEntry */



/**
 *  Create a sub window inside of 'parent_window'.
 *  The new sub window has no window decorations and is not managed by
 *  the window manager. If the empty window is used as 'parent_window'
 *  an unmanaged top level window without window decorations is generated.
 *  The coordinates 'xPos' and 'yPos' are measured relative to the top
 *  left corner of the 'parent_window' drawing area (inside of the window
 *  decorations). If the empty window is used as 'parent_window' the
 *  coordinates 'xPos' and 'yPos' are measured relative to the top left
 *  corner of the screen.
 *  @param parent-window Parent window (can be the empty window).
 *  @param xPos X-position of the left corner of the new window.
 *  @param yPos Y-position of the left corner of the new window.
 *  @param width Width of the new window.
 *  @param height Height of the new window.
 *  @return the new generated window.
 */
winType drwOpenSubWindow (const_winType parent_window, intType xPos, intType yPos,
    intType width, intType height)

  {
    Window parent;
    XSizeHints myhint;
    XSetWindowAttributes attributes;
    x11_winType result = NULL;

  /* drwOpenSubWindow */
    logFunction(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D ")\n",
                       xPos, yPos, width, height););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos) ||
                 width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwOpenSubWindow(" FMT_D ", " FMT_D ", " FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      xPos, yPos, width, height););
      raise_error(RANGE_ERROR);
    } else {
      if (!init_called) {
        drawInit();
      } /* if */
      if (unlikely(!init_called)) {
        logError(printf("drwOpenSubWindow: drawInit() failed to open a display.\n"););
        raise_error(GRAPHIC_ERROR);
      } else {
        if (ALLOC_RECORD2(result, x11_winRecord, count.win, count.win_bytes)) {
          memset(result, 0, sizeof(x11_winRecord));
          result->usage_count = 1;

          myhint.x = (int) xPos;
          myhint.y = (int) yPos;
          myhint.width = (int) width;
          myhint.height = (int) height;
          myhint.flags = PPosition | PSize;

          parent = to_window(parent_window);
          if (parent == 0) {
            parent = DefaultRootWindow(mydisplay);
          } /* if */
          result->window = XCreateSimpleWindow(mydisplay, parent,
              (int) xPos, (int) yPos, (unsigned) width, (unsigned) height,
              0, myforeground, mybackground);
          enter_window((winType) result, result->window);

          result->backup = 0;
          result->clip_mask = 0;
          result->is_pixmap = FALSE;
          result->is_managed = FALSE;
          result->width = (unsigned int) width;
          result->height = (unsigned int) height;

          XSetStandardProperties(mydisplay, result->window,
              "", "",
              None, /* argv, argc, */ NULL, 0, &myhint);

          if (to_window(parent_window) == 0) {
            /* The parent of this window is the empty window (root window). */
            omitWindowDecorationsAndTaskbarEntry(result);
          } /* if */

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
              ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
              ExposureMask | FocusChangeMask);

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



void drwSetCloseAction (winType actual_window, intType closeAction)

  { /* drwSetCloseAction */
    logFunction(printf("drwSetCloseAction(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) actual_window, closeAction););
    if (closeAction < 0 || closeAction > 2) {
      raise_error(RANGE_ERROR);
    } else {
      to_var_close_action(actual_window) = (int) closeAction;
    } /* if */
  } /* drwSetCloseAction */



/**
 *  Set the visibility of the mouse cursor in 'aWindow'.
 *  @param aWindow Window for which the mouse cursor visibility is set.
 *  @param visible TRUE, if the mouse cursor should be visible in 'aWindow', or
 *                 FALSE, if the mouse curser should be invisible in 'aWindow'.
 */
void drwSetCursorVisible (winType aWindow, boolType visible)

  { /* drwSetCursorVisible */
    logFunction(printf("drwSetCursorVisible(" FMT_U_MEM ", %d)\n",
                       (memSizeType) aWindow, visible););
    if (visible) {
      XUndefineCursor(mydisplay, to_window(aWindow));
    } else {
      XDefineCursor(mydisplay, to_window(aWindow), emptyCursor);
    } /* if */
    XFlush(mydisplay);
  } /* drwSetCursorVisible */



/**
 *  Set the pointer x and y position relative to the specified 'aWindow'.
 *  The point of origin is the top left corner of the drawing area
 *  of the given 'aWindow' (inside of the window decorations).
 *  If 'aWindow' is the empty window the pointer x and y position
 *  is relative to the top left corner of the screen.
 */
void drwSetPointerPos (const_winType aWindow, intType xPos, intType yPos)

  {
    Window window;

  /* drwSetPointerPos */
    logFunction(printf("drwSetPointerPos(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) aWindow, xPos, yPos););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos))) {
      raise_error(RANGE_ERROR);
    } else {
      window = to_window(aWindow);
      if (window == 0) {
        window = DefaultRootWindow(mydisplay);
      } /* if */
      XWarpPointer(mydisplay, None, window, 0, 0, 0, 0, (int) xPos, (int) yPos);
    } /* if */
  } /* drwSetPointerPos */



void drwPoint (const_winType actual_window, intType x, intType y)

  { /* drwPoint */
    logFunction(printf("drwPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y););
    if (unlikely(!(inIntRange(x) && inIntRange(y)))) {
      logError(printf("drwPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y););
      raise_error(RANGE_ERROR);
    } else {
      XDrawPoint(mydisplay, to_window(actual_window), mygc, (int) (x), (int) (y));
      if (to_backup(actual_window) != 0) {
        XDrawPoint(mydisplay, to_backup(actual_window), mygc, (int) (x), (int) (y));
      } /* if */
    } /* if */
  } /* drwPoint */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  { /* drwPPoint */
    logFunction(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, col););
    if (unlikely(!(inIntRange(x) && inIntRange(y)))) {
      logError(printf("drwPPoint(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " F_X(08) "): raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawPoint(mydisplay, to_window(actual_window), mygc, (int) (x), (int) (y));
      if (to_backup(actual_window) != 0) {
        XDrawPoint(mydisplay, to_backup(actual_window), mygc, (int) (x), (int) (y));
      } /* if */
    } /* if */
  } /* drwPPoint */



rtlArrayType drwConvPointList (const const_bstriType pointList)

  {
    memSizeType len;
    XPoint *points;
    memSizeType pos;
    rtlArrayType xyArray;

  /* drwConvPointList */
    logFunction(printf("drwConvPointList(\"%s\")\n",
                       bstriAsUnquotedCStri(pointList)););
    len = pointList->size / sizeof(XPoint);
    if (unlikely(!ALLOC_RTL_ARRAY(xyArray, len << 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      xyArray->min_position = 1;
      xyArray->max_position = (intType) (len << 1);
      if (len != 0) {
        points = (XPoint *) pointList->mem;
        xyArray->arr[0].value.intValue = (intType) points[0].x;
        xyArray->arr[1].value.intValue = (intType) points[0].y;
        for (pos = 1; pos < len; pos++) {
          xyArray->arr[ pos << 1     ].value.intValue =
              xyArray->arr[(pos << 1) - 2].value.intValue +
              (intType) points[pos].x;
          xyArray->arr[(pos << 1) + 1].value.intValue =
              xyArray->arr[(pos << 1) - 1].value.intValue +
              (intType) points[pos].y;
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("drwConvPointList --> arr (size=" FMT_U_MEM ")\n",
                       arraySize(xyArray)););
    return xyArray;
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
    logFunction(printf("drwGenPointList(" FMT_D " .. " FMT_D ")\n",
                       xyArray->min_position, xyArray->max_position););
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
            for (pos = 1; pos < len; pos++) {
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
    int numPoints;
    XPoint startBackup;

  /* drwPolyLine */
    logFunction(printf("drwPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, (memSizeType) point_list, col););
    if (unlikely(!inShortRange(x) || !inShortRange(y))) {
      raise_error(RANGE_ERROR);
    } else {
      points = (XPoint *) point_list->mem;
      numPoints = (int) (point_list->size / sizeof(XPoint));
      memcpy(&startBackup, &points[0], sizeof(XPoint));
      points[0].x += (short int) x;
      points[0].y += (short int) y;
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawLines(mydisplay, to_window(actual_window), mygc, points, numPoints, CoordModePrevious);
      if (to_backup(actual_window) != 0) {
        XDrawLines(mydisplay, to_backup(actual_window), mygc, points, numPoints, CoordModePrevious);
      } /* if */
      memcpy(&points[0], &startBackup, sizeof(XPoint));
    } /* if */
  } /* drwPolyLine */



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  {
    XPoint *points;
    int numPoints;
    XPoint startBackup;

  /* drwFPolyLine */
    logFunction(printf("drwFPolyLine(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y, (memSizeType) point_list, col););
    if (unlikely(!inShortRange(x) || !inShortRange(y))) {
      raise_error(RANGE_ERROR);
    } else {
      points = (XPoint *) point_list->mem;
      numPoints = (int) (point_list->size / sizeof(XPoint));
      memcpy(&startBackup, &points[0], sizeof(XPoint));
      points[0].x += (short int) x;
      points[0].y += (short int) y;
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XDrawLines(mydisplay, to_window(actual_window), mygc, points, numPoints, CoordModePrevious);
      XFillPolygon(mydisplay, to_window(actual_window), mygc, points, numPoints,
          Nonconvex, CoordModePrevious);
      if (to_backup(actual_window) != 0) {
        XDrawLines(mydisplay, to_backup(actual_window), mygc, points, numPoints, CoordModePrevious);
        XFillPolygon(mydisplay, to_backup(actual_window), mygc, points, numPoints,
            Nonconvex, CoordModePrevious);
      } /* if */
      memcpy(&points[0], &startBackup, sizeof(XPoint));
    } /* if */
  } /* drwFPolyLine */



void drwPut (const_winType destWindow, intType xDest, intType yDest,
    const_winType pixmap)

  { /* drwPut */
    logFunction(printf("drwPut(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_U_MEM ")\n",
                       (memSizeType) destWindow, xDest, yDest, (memSizeType) pixmap););
    /* printf("destWindow=%lu, pixmap=%lu\n", to_window(destWindow),
        pixmap != NULL ? to_window(pixmap) : NULL); */
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest))) {
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL && to_window(pixmap) != 0) {
      /* A pixmap value of NULL or a pixmap with a window of 0 */
      /* is used to describe an empty pixmap. In this case     */
      /* nothing should be done.                               */
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, to_clip_mask(pixmap));
        XSetClipOrigin(mydisplay, mygc, (int) xDest, (int) yDest);
      } /* if */
      XCopyArea(mydisplay, to_window(pixmap), to_window(destWindow),
                mygc, 0, 0, to_width(pixmap), to_height(pixmap), (int) xDest, (int) yDest);
      if (to_backup(destWindow) != 0) {
        XCopyArea(mydisplay, to_window(pixmap), to_backup(destWindow),
                  mygc, 0, 0, to_width(pixmap), to_height(pixmap), (int) xDest, (int) yDest);
      } /* if */
      if (to_clip_mask(pixmap) != 0) {
        XSetClipMask(mydisplay, mygc, None);
      } /* if */
    } /* if */
  } /* drwPut */



void drwPutScaled (const_winType destWindow, intType xDest, intType yDest,
    intType width, intType height, const_winType pixmap)

  {
#ifdef HAS_XRENDER_EXTENSION
    Picture picture;
    Picture dest;
#endif

  /* drwPutScaled */
    logFunction(printf("drwPutScaled(" FMT_U_MEM  ", " FMT_D ", " FMT_D ", "
                       FMT_D ", " FMT_D ", " FMT_U_MEM")\n",
                       (memSizeType) destWindow, xDest, yDest,
                       width, height, (memSizeType) pixmap););
    if (unlikely(!inIntRange(xDest) || !inIntRange(yDest) ||
                 !inIntRange(width) || width < 0 ||
                 !inIntRange(height) || height < 0)) {
      raise_error(RANGE_ERROR);
    } else if (pixmap != NULL && to_window(pixmap) != 0) {
      /* A pixmap value of NULL or a pixmap with a window of 0 */
      /* is used to describe an empty pixmap. In this case     */
      /* nothing should be done.                               */
#ifdef HAS_XRENDER_EXTENSION
      double xScale = (double) to_width(pixmap) / (double) width;
      double yScale = (double) to_height(pixmap) / (double) height;
      XTransform transform = {{
          { XDoubleToFixed(1.0), XDoubleToFixed(0.0), XDoubleToFixed(0.0) },
          { XDoubleToFixed(0.0), XDoubleToFixed(1.0), XDoubleToFixed(0.0) },
          { XDoubleToFixed(0.0), XDoubleToFixed(0.0), XDoubleToFixed(1.0) }
        }};
      XRenderPictureAttributes pictureAttributes;
      /* The assignments below are done, because the Watcom C compiler */
      /* cannot handle variables in the initialization of transform.   */
      transform.matrix[0][0] = XDoubleToFixed(xScale);
      transform.matrix[1][1] = XDoubleToFixed(yScale);
      memset(&pictureAttributes, 0, sizeof(pictureAttributes));
      picture = XRenderCreatePicture(mydisplay, to_window(pixmap),
                                     XRenderFindVisualFormat(mydisplay, default_visual),
                                     CPSubwindowMode, &pictureAttributes);
      dest = XRenderCreatePicture(mydisplay, to_backup(destWindow) != 0 ?
                                  to_backup(destWindow) : to_window(destWindow),
                                  XRenderFindVisualFormat(mydisplay, default_visual),
                                  CPSubwindowMode, &pictureAttributes);
      XRenderSetPictureTransform(mydisplay, picture, &transform);
      XRenderComposite(mydisplay, PictOpOver, picture, 0, dest,
                       0, 0, /* source (x, y) */ 0, 0, /* mask (x, y) */
                       (int) xDest, (int) yDest, (unsigned int) width, (unsigned int) height);
      XRenderFreePicture(mydisplay, picture);
      XRenderFreePicture(mydisplay, dest);
      if (to_backup(destWindow) != 0) {
        XCopyArea(mydisplay, to_backup(destWindow), to_window(destWindow),
                  mygc, (int) xDest, (int) yDest, (unsigned int) width,
                  (unsigned int) height, (int) xDest, (int) yDest);
      } /* if */
#endif
    } /* if */
    logFunction(printf("drwPutScaled -->\n"););
  } /* drwPutScaled */



void drwRect (const_winType actual_window,
    intType x, intType y, intType width, intType height)

  { /* drwRect */
    logFunction(printf("drwRect(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, x, y, width, height););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 width < 0 || width > UINT_MAX ||
                 height < 0 || height > UINT_MAX)) {
      logError(printf("drwRect(" FMT_U_MEM ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " FMT_D "): raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y, width, height););
      raise_error(RANGE_ERROR);
    } else {
      XFillRectangle(mydisplay, to_window(actual_window), mygc,
          (int) (x), (int) (y), (unsigned int) width, (unsigned int) height);
      if (to_backup(actual_window) != 0) {
        XFillRectangle(mydisplay, to_backup(actual_window), mygc,
            (int) (x), (int) (y), (unsigned int) width, (unsigned int) height);
      } /* if */
    } /* if */
  } /* drwRect */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPRect */
    logFunction(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D
                       ", " FMT_D ", " FMT_D ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       width, height, col););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 width < 0 || width > UINT_MAX ||
                 height < 0 || height > UINT_MAX)) {
      logError(printf("drwPRect(" FMT_U_MEM ", " FMT_D ", " FMT_D ", " FMT_D
                      ", " FMT_D ", " F_X(08) "): raises RANGE_ERROR\n",
                      (memSizeType) actual_window, x, y,
                      width, height, col););
      raise_error(RANGE_ERROR);
    } else {
      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XFillRectangle(mydisplay, to_window(actual_window), mygc,
          (int) (x), (int) (y), (unsigned int) width, (unsigned int) height);
      if (to_backup(actual_window) != 0) {
        XFillRectangle(mydisplay, to_backup(actual_window), mygc,
            (int) (x), (int) (y), (unsigned int) width, (unsigned int) height);
      } /* if */
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
    logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ")\n",
                       redLight, greenLight, blueLight););
#ifdef rgbToPixel
    if (useRgbToPixel) {
      logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> " F_X(08) "\n",
                         redLight, greenLight, blueLight,
                         (intType) rgbToPixel(redLight, greenLight, blueLight)););
      return (intType) rgbToPixel(redLight, greenLight, blueLight);
    } else
#endif
    if (usesTrueColor) {
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
      logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> " F_X(08) "\n",
                         redLight, greenLight, blueLight,
                         (intType) col.pixel););
      return (intType) col.pixel;
    } /* if */
    if (unlikely(color_hash == NULL)) {
      if (unlikely(!init_called)) {
        logError(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D "): "
                        "drawInit() failed to open a display.\n",
                        redLight, greenLight, blueLight););
        raise_error(GRAPHIC_ERROR);
        return 0;
      } else {
        color_hash = (colorEntryType *) malloc(4096 * sizeof(colorEntryType));
        if (unlikely(color_hash == NULL)) {
          logError(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D "): "
                          "malloc() color_hash failed.\n",
                          redLight, greenLight, blueLight););
          raise_error(MEMORY_ERROR);
          return 0;
        } else {
          memset(color_hash, 0, 4096 * sizeof(colorEntryType));
        } /* if */
      } /* if */
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
        logFunction(printf("drwRgbColor(" FMT_D ", " FMT_D ", " FMT_D ") --> found " F_X(08) "\n",
                           redLight, greenLight, blueLight,
                           (intType) entry->pixel););
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
      /* handle failure */
      printf("XAllocColor(%ld, %ld, %ld) not okay\n",
          redLight, greenLight, blueLight);
    } /* if */
#ifdef OUT_OF_ORDER
    col.red = RED_VAL;
    col.green = GREEN_VAL;
    col.blue = BLUE_VAL;
    if (XAllocColor(mydisplay, cmap, &col) == 0) {
      /* handle failure */
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
    if (usesTrueColor) {
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
    logFunction(printf("drwPixelToRgb(" F_X(08) ", " FMT_D ", " FMT_D ", " FMT_D ") -->\n",
                       col, *redLight, *greenLight, *blueLight););
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



/**
 *  Determine the height of the screen in pixels.
 */
intType drwScreenHeight (void)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwScreenHeight */
    logFunction(printf("drwScreenHeight()\n"););
    if (!init_called) {
      drawInit();
    } /* if */
    if (unlikely(!init_called ||
                 XGetGeometry(mydisplay, DefaultRootWindow(mydisplay),
                              &root, &x, &y, &width, &height,
                              &border_width, &depth) == 0)) {
      logError(printf("drwScreenHeight: drawInit() or XGetGeometry() failed.\n"););
      raise_error(GRAPHIC_ERROR);
      height = 0;
    } /* if */
    logFunction(printf("drwScreenHeight() --> %u\n", height););
    return (intType) height;
  } /* drwScreenHeight */



/**
 *  Determine the width of the screen in pixels.
 */
intType drwScreenWidth (void)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwScreenWidth */
    logFunction(printf("drwScreenWidth()\n"););
    if (!init_called) {
      drawInit();
    } /* if */
    if (unlikely(!init_called ||
                 XGetGeometry(mydisplay, DefaultRootWindow(mydisplay),
                              &root, &x, &y, &width, &height,
                              &border_width, &depth) == 0)) {
      logError(printf("drwScreenWidth: drawInit() or XGetGeometry() failed.\n"););
      raise_error(GRAPHIC_ERROR);
      width = 0;
    } /* if */
    logFunction(printf("drwScreenWidth() --> %u\n", width););
    return (intType) width;
  } /* drwScreenWidth */



void drwSetContent (const_winType actual_window, const_winType pixmap)

  { /* drwSetContent */
    logFunction(printf("drwSetContent(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) actual_window, (memSizeType) pixmap););
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



/**
 *  Move the top left corner of a window to the coordinates x/y.
 *  If window decorations are present the top left corner of the
 *  window decorations will be at the position x/y. For a sub window
 *  the position is relative to the top left corner of the parent window
 *  drawing area (inside of the window decorations). For top level windows
 *  the position is relative to the top left corner of the screen.
 */
void drwSetPos (const_winType actual_window, intType xPos, intType yPos)

  { /* drwSetPos */
    logFunction(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, xPos, yPos););
    if (unlikely(!inIntRange(xPos) || !inIntRange(yPos))) {
      logError(printf("drwSetPos(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "raises RANGE_ERROR\n",
                      (memSizeType) actual_window, xPos, yPos););
      raise_error(RANGE_ERROR);
    } else {
      XMoveWindow(mydisplay, to_window(actual_window), (int) (xPos), (int) (yPos));
    } /* if */
  } /* drwSetPos */



void drwSetSize (winType actual_window, intType width, intType height)

  { /* drwSetSize */
    logFunction(printf("drwSetSize(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) actual_window, width, height););
    if (unlikely(width < 1 || width > INT_MAX ||
                 height < 1 || height > INT_MAX)) {
      logError(printf("drwSetSize(" FMT_D ", " FMT_D "): "
                      "Illegal window dimensions\n",
                      width, height););
      raise_error(RANGE_ERROR);
    } else {
      XResizeWindow(mydisplay, to_window(actual_window),
                    (unsigned int) width, (unsigned int) height);
      to_var_width(actual_window) = (unsigned int) width;
      to_var_height(actual_window) = (unsigned int) height;
    } /* if */
  } /* drwSetSize */



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



void drwSetWindowName (winType aWindow, const const_striType windowName)

  {
    char *winName8;
    memSizeType winName8Length;

  /* drwSetWindowName */
    logFunction(printf("drwSetWindowName(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) aWindow,
                       striAsUnquotedCStri(windowName)););
    winName8 = stri_to_cstri8_buf(windowName, &winName8Length);
    if (unlikely(winName8 == NULL)) {
      logError(printf("drwOpen: stri_to_cstri8_buf(\"%s\") failed\n",
                      striAsUnquotedCStri(windowName)););
      raise_error(MEMORY_ERROR);
    } else {
      XChangeProperty(mydisplay, to_window(aWindow), net_wm_name,
          utf8_string, 8, PropModeReplace, (unsigned char *) winName8,
          (int) winName8Length);
      free_cstri8(winName8, windowName);
    } /* if */
    logFunction(printf("drwSetWindowName -->\n"););
  } /* drwSetWindowName */



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  {
    XChar2b *stri_buffer;
    XChar2b *wstri;
    const strElemType *strelem;
    memSizeType len;

  /* drwText */
    logFunction(printf("drwText(" FMT_U_MEM ", " FMT_D ", " FMT_D ", \"%s\", "
                       F_X(08) ", " F_X(08) ")\n",
                       (memSizeType) actual_window, x, y,
                       striAsUnquotedCStri(stri), col, bkcol););
    if (unlikely(!inIntRange(x) || !inIntRange(y) ||
                 stri->size >= (unsigned int) INT_MAX)) {
      raise_error(RANGE_ERROR);
    } else if (unlikely(stri->size > MAX_XCHAR2B_LEN ||
                        (stri_buffer = (XChar2b *) malloc(
                         sizeof(XChar2b) * stri->size)) == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      wstri = stri_buffer;
      strelem = stri->mem;
      len = stri->size;
      for (; len > 0; wstri++, strelem++, len--) {
        if (unlikely(*strelem >= 65536)) {
          raise_error(RANGE_ERROR);
          return;
        } /* if */
        wstri->byte1 = (unsigned char) ((*strelem >> 8) & 0xFF);
        wstri->byte2 = (unsigned char) (*strelem & 0xFF);
      } /* for */

      XSetForeground(mydisplay, mygc, (unsigned long) col);
      XSetBackground(mydisplay, mygc, (unsigned long) bkcol);
      XDrawImageString16(mydisplay, to_window(actual_window), mygc,
          (int) x, (int) y, stri_buffer, (int) stri->size);
      if (to_backup(actual_window) != 0) {
        XDrawImageString16(mydisplay, to_backup(actual_window), mygc,
            (int) x, (int) y, stri_buffer, (int) stri->size);
      } /* if */
      free(stri_buffer);
    } /* if */
  } /* drwText */



/**
 *  Lower a window to the bottom so that it does not obscure any other window.
 */
void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
    logFunction(printf("drwToBottom(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    XLowerWindow(mydisplay, to_window(actual_window));
    logFunction(printf("drwToBottom(" FMT_U_MEM ") -->\n",
                       (memSizeType) actual_window););
  } /* drwToBottom */



/**
 *  Raise a window to the top so that no other window obscures it.
 */
void drwToTop (const_winType actual_window)

  { /* drwToTop */
    logFunction(printf("drwToTop(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    XRaiseWindow(mydisplay, to_window(actual_window));
    logFunction(printf("drwToTop(" FMT_U_MEM ") -->\n",
                       (memSizeType) actual_window););
  } /* drwToTop */



/**
 *  Determine the width of the window drawing area in pixels.
 *  This excludes window declarations left and right. Add left and right
 *  border widths to get the width inclusive window decorations.
 */
intType drwWidth (const_winType actual_window)

  {
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;

  /* drwWidth */
    logFunction(printf("drwWidth(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) actual_window,
                       actual_window != NULL ? actual_window->usage_count
                                             : (uintType) 0););
    if (is_pixmap(actual_window)) {
      width = to_width(actual_window);
    } else if (unlikely(XGetGeometry(mydisplay, to_window(actual_window), &root,
                        &x, &y, &width, &height, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      width = 0;
    } /* if */
    logFunction(printf("drwWidth(" FMT_U_MEM ") --> %u\n",
                       (memSizeType) actual_window, width););
    return (intType) width;
  } /* drwWidth */



/**
 *  Determine the X position of the top left corner of a window in pixels.
 *  If window decorations are present this uses the top left corner of
 *  the window decorations. For a sub window the X position is relative
 *  to the top left corner of the parent window drawing area (inside of
 *  the window decorations). For top level windows the X position is
 *  relative to the top left corner of the screen.
 *  @exception RANGE_ERROR If 'actual_window' is a pixmap.
 */
intType drwXPos (const_winType actual_window)

  {
    Window window;
    Window root;
    int y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    int xPos;

  /* drwXPos */
    logFunction(printf("drwXPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    window = to_window(actual_window);
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      xPos = 0;
    } else if (is_managed(actual_window)) {
      do {
        if (unlikely(XGetGeometry(mydisplay, window, &root, &xPos, &y,
                     &width, &height, &border_width, &depth) == 0)) {
          raise_error(GRAPHIC_ERROR);
          window = 0;
          xPos = 0;
        } else {
          window = getWindowParent(window);
        } /* if */
      } while (window != root && window != 0);
    } else if (unlikely(XGetGeometry(mydisplay, window, &root, &xPos, &y,
                        &width, &height, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      xPos = 0;
    } /* if */
    logFunction(printf("drwXPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, xPos););
    return (intType) xPos;
  } /* drwXPos */



/**
 *  Determine the Y position of the top left corner of a window in pixels.
 *  If window decorations are present this uses the top left corner of
 *  the window decorations. For a sub window the Y position is relative
 *  to the top left corner of the parent window drawing area (inside of
 *  the window decorations). For top level windows the Y position is
 *  relative to the top left corner of the screen.
 *  @exception RANGE_ERROR If 'actual_window' is a pixmap.
 */
intType drwYPos (const_winType actual_window)

  {
    Window window;
    Window root;
    int x;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    int yPos;

  /* drwYPos */
    logFunction(printf("drwYPos(" FMT_U_MEM ")\n",
                       (memSizeType) actual_window););
    window = to_window(actual_window);
    if (is_pixmap(actual_window)) {
      raise_error(RANGE_ERROR);
      yPos = 0;
    } else if (is_managed(actual_window)) {
      do {
        if (unlikely(XGetGeometry(mydisplay, window, &root, &x, &yPos,
                     &width, &height, &border_width, &depth) == 0)) {
          raise_error(GRAPHIC_ERROR);
          window = 0;
          yPos = 0;
        } else {
          window = getWindowParent(window);
        } /* if */
      } while (window != root && window != 0);
    } else if (unlikely(XGetGeometry(mydisplay, window, &root, &x, &yPos,
                        &width, &height, &border_width, &depth) == 0)) {
      raise_error(GRAPHIC_ERROR);
      yPos = 0;
    } /* if */
    logFunction(printf("drwYPos(" FMT_U_MEM ") --> %d\n",
                       (memSizeType) actual_window, yPos););
    return (intType) yPos;
  } /* drwYPos */
