/********************************************************************/
/*                                                                  */
/*  fwd_x11.c     Forward X11 calls to a shared X11 library.        */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/fwd_x11.c                                       */
/*  Changes: 2019  Thomas Mertes                                    */
/*  Content: Forward X11 calls to a shared X11 library.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#if FORWARD_X11_CALLS
#include "stdio.h"

#ifdef X11_INCLUDE
#include X11_INCLUDE
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#undef XDestroyImage
#undef XGetPixel
#endif

#include "common.h"
#include "dll_drv.h"


#if XID_SIZE == 32
#define FMT_D_XID  FMT_D32
#define FMT_U_XID  FMT_U32
#define FMT_X_XID  FMT_X32
#elif XID_SIZE == 64
#define FMT_D_XID  FMT_D64
#define FMT_U_XID  FMT_U64
#define FMT_X_XID  FMT_X64
#endif

#if ATOM_SIZE == 32
#define FMT_D_ATOM  FMT_D32
#define FMT_U_ATOM  FMT_U32
#define FMT_X_ATOM  FMT_X32
#elif ATOM_SIZE == 64
#define FMT_D_ATOM  FMT_D64
#define FMT_U_ATOM  FMT_U64
#define FMT_X_ATOM  FMT_X64
#endif


typedef Status (*tp_XAllocColor) (Display *display, Colormap colormap, XColor *screen_in_out);
typedef Status (*tp_XAllocColorCells) (Display *display, Colormap colormap, Bool contig,
                                       unsigned long *plane_masks_return,
                                       unsigned int nplanes, unsigned long *pixels_return,
                                       unsigned int npixels);
typedef unsigned long (*tp_XBlackPixel) (Display *display,
                                         int screen_number);
typedef int (*tp_XChangeWindowAttributes) (Display *display, Window window,
                                           unsigned long valuemask,
                                           XSetWindowAttributes *attributes);
typedef int (*tp_XCopyArea) (Display *display, Drawable src, Drawable dest, GC gc,
                             int src_x, int src_y, unsigned int width, unsigned height,
                             int dest_x, int dest_y);
typedef int (*tp_XCopyPlane) (Display *display, Drawable src, Drawable dest,
                              GC gc, int src_x, int src_y, unsigned int width,
                              unsigned int height, int dest_x, int dest_y,
                              unsigned long plane);
typedef GC (*tp_XCreateGC) (Display *display, Drawable drawable, unsigned long valuemask,
                            XGCValues *values);
typedef XImage *(*tp_XCreateImage) (Display *display, Visual *visual,
                                    unsigned int depth, int format, int offset, char *data,
                                    unsigned int width, unsigned int height,
                                    int bitmap_pad, int bytes_per_line);
typedef Pixmap (*tp_XCreatePixmap) (Display *display, Drawable drawable, unsigned int width,
                                    unsigned int height, unsigned int depth);
typedef Window (*tp_XCreateSimpleWindow) (Display *display, Window parent, int x, int y,
                                          unsigned int width, unsigned int height,
                                          unsigned int border_width,
                                          unsigned long border,
                                          unsigned long background);
typedef Colormap (*tp_XDefaultColormap) (Display *display,
                                         int screen_number);
typedef int (*tp_XDefaultDepth) (Display *display,
                                 int screen_number);
typedef Window (*tp_XDefaultRootWindow) (Display *display);
typedef int (*tp_XDefaultScreen) (Display *display);
typedef Visual *(*tp_XDefaultVisual) (Display *display, int screen_number);
typedef int (*tp_XDestroyImage) (XImage *ximage);
typedef int (*tp_XDestroyWindow) (Display *display, Window window);
typedef int (*tp_XDoesBackingStore) (Screen *screen);
typedef int (*tp_XDrawArc) (Display *display, Drawable drawable, GC gc, int x, int y,
                            unsigned int width, unsigned int height, int angle1,
                            int angle2);
typedef int (*tp_XDrawImageString16) (Display *display, Drawable drawable, GC gc, int x,
                                      int y, const XChar2b *string, int length);
typedef int (*tp_XDrawLine) (Display *display, Drawable drawable, GC gc, int x1, int y1,
                             int x2, int y2);
typedef int (*tp_XDrawLines) (Display *display, Drawable drawable, GC gc, XPoint *points,
                              int npoints, int mode);
typedef int (*tp_XDrawPoint) (Display *display, Drawable drawable, GC gc, int x, int y);
typedef int (*tp_XEventsQueued) (Display *display, int mode);
typedef int (*tp_XFillArc) (Display *display, Drawable drawable, GC gc, int x, int y,
                            unsigned int width, unsigned int height, int angle1,
                            int angle2);
typedef int (*tp_XFillPolygon) (Display *display, Drawable drawable, GC gc,
                                XPoint *points, int npoints, int shape, int mode);
typedef int (*tp_XFillRectangle) (Display *display, Drawable drawable, GC gc, int x, int y,
                                  unsigned int width, unsigned int height);
typedef int (*tp_XFlush) (Display *display);
typedef int (*tp_XFree) (void *data);
typedef int (*tp_XFreeGC) (Display *display, GC gc);
typedef int (*tp_XFreePixmap) (Display *display, Pixmap pixmap);
typedef Status (*tp_XGetGeometry) (Display *display, Drawable drawable,
                                   Window *root_return, int *x_return, int *y_return,
                                   unsigned int *width_return,
                                   unsigned int *height_return,
                                   unsigned int *border_width_return,
                                   unsigned int *depth_return);
typedef XImage *(*tp_XGetImage) (Display *display, Drawable drawable, int x, int y,
                                 unsigned int width, unsigned int height,
                                 unsigned long plane_mask, int format);
typedef unsigned long (*tp_XGetPixel) (XImage *ximage, int x, int y);
typedef Status (*tp_XGetWindowAttributes) (Display *display, Window window,
                                           XWindowAttributes *window_attributes_return);
typedef Atom (*tp_XInternAtom) (Display *display, const char *atom_name, Bool only_if_exists);
typedef KeyCode (*tp_XKeysymToKeycode) (Display *display, KeySym keysym);
typedef int (*tp_XLookupString) (XKeyEvent *event_struct, char *buffer_return,
                                 int bytes_buffer, KeySym *keysym_return,
                                 XComposeStatus *status_in_out);
typedef int (*tp_XLowerWindow) (Display *display, Window window);
typedef int (*tp_XMapRaised) (Display *display, Window window);
typedef int (*tp_XMoveWindow) (Display *display, Window window, int x, int y);
typedef int (*tp_XNextEvent) (Display *display, XEvent *event_return);
typedef Display *(*tp_XOpenDisplay) (const char *display_name);
typedef int (*tp_XPutImage) (Display *display, Drawable drawable, GC gc,
                             XImage *image, int src_x, int src_y,
                             int dest_x, int dest_y, unsigned int width,
                             unsigned int height);
typedef int (*tp_XQueryColor) (Display *display, Colormap colormap,
                               XColor *def_in_out);
typedef int (*tp_XQueryKeymap) (Display *display, char keys_return[32]);
typedef Bool (*tp_XQueryPointer) (Display *display, Window window,
                                  Window *root_return, Window *child_return,
                                  int *root_x_return, int *root_y_return,
                                  int *win_x_return, int *win_y_return,
                                  unsigned int *mask_return);
typedef int (*tp_XRaiseWindow) (Display *display, Window window);
typedef int (*tp_XRefreshKeyboardMapping) (XMappingEvent *event_map);
typedef Screen *(*tp_XScreenOfDisplay) (Display *display, int screen_number);
typedef int (*tp_XSelectInput) (Display *display, Window window,
                                long event_mask);
typedef int (*tp_XSetArcMode) (Display *display, GC gc, int arc_mode);
typedef int (*tp_XSetBackground) (Display *display, GC gc, unsigned long background);
typedef int (*tp_XSetClipMask) (Display *display, GC gc, Pixmap pixmap);
typedef int (*tp_XSetClipOrigin) (Display *display, GC gc, int clip_x_origin,
                                  int clip_y_origin);
typedef int (*tp_XSetForeground) (Display *display, GC gc, unsigned long foreground);
typedef int (*tp_XSetFunction) (Display *display, GC gc, int function);
typedef int (*tp_XSetStandardProperties) (Display *display, Window window,
                                          const char *window_name,
                                          const char *icon_name,
                                          Pixmap icon_pixmap, char **argv,
                                          int argc, XSizeHints *hints);
typedef int (*tp_XSetWMHints) (Display *display, Window window, XWMHints *wm_hints);
typedef Status (*tp_XSetWMProtocols) (Display *display, Window window,
                                      Atom *protocols, int count);
typedef int (*tp_XStoreColor) (Display *display, Colormap colormap, XColor *color);
typedef int (*tp_XSync) (Display *display, Bool discard);
typedef unsigned long (*tp_XWhitePixel) (Display *display,
                                         int screen_number);

static tp_XAllocColor             ptr_XAllocColor;
static tp_XAllocColorCells        ptr_XAllocColorCells;
static tp_XBlackPixel             ptr_XBlackPixel;
static tp_XChangeWindowAttributes ptr_XChangeWindowAttributes;
static tp_XCopyArea               ptr_XCopyArea;
static tp_XCopyPlane              ptr_XCopyPlane;
static tp_XCreateGC               ptr_XCreateGC;
static tp_XCreateImage            ptr_XCreateImage;
static tp_XCreatePixmap           ptr_XCreatePixmap;
static tp_XCreateSimpleWindow     ptr_XCreateSimpleWindow;
static tp_XDefaultColormap        ptr_XDefaultColormap;
static tp_XDefaultDepth           ptr_XDefaultDepth;
static tp_XDefaultRootWindow      ptr_XDefaultRootWindow;
static tp_XDefaultScreen          ptr_XDefaultScreen;
static tp_XDefaultVisual          ptr_XDefaultVisual;
static tp_XDestroyImage           ptr_XDestroyImage;
static tp_XDestroyWindow          ptr_XDestroyWindow;
static tp_XDoesBackingStore       ptr_XDoesBackingStore;
static tp_XDrawArc                ptr_XDrawArc;
static tp_XDrawImageString16      ptr_XDrawImageString16;
static tp_XDrawLine               ptr_XDrawLine;
static tp_XDrawLines              ptr_XDrawLines;
static tp_XDrawPoint              ptr_XDrawPoint;
static tp_XEventsQueued           ptr_XEventsQueued;
static tp_XFillArc                ptr_XFillArc;
static tp_XFillPolygon            ptr_XFillPolygon;
static tp_XFillRectangle          ptr_XFillRectangle;
static tp_XFlush                  ptr_XFlush;
static tp_XFree                   ptr_XFree;
static tp_XFreeGC                 ptr_XFreeGC;
static tp_XFreePixmap             ptr_XFreePixmap;
static tp_XGetGeometry            ptr_XGetGeometry;
static tp_XGetImage               ptr_XGetImage;
static tp_XGetPixel               ptr_XGetPixel;
static tp_XGetWindowAttributes    ptr_XGetWindowAttributes;
static tp_XInternAtom             ptr_XInternAtom;
static tp_XKeysymToKeycode        ptr_XKeysymToKeycode;
static tp_XLookupString           ptr_XLookupString;
static tp_XLowerWindow            ptr_XLowerWindow;
static tp_XMapRaised              ptr_XMapRaised;
static tp_XMoveWindow             ptr_XMoveWindow;
static tp_XNextEvent              ptr_XNextEvent;
static tp_XOpenDisplay            ptr_XOpenDisplay;
static tp_XPutImage               ptr_XPutImage;
static tp_XQueryColor             ptr_XQueryColor;
static tp_XQueryKeymap            ptr_XQueryKeymap;
static tp_XQueryPointer           ptr_XQueryPointer;
static tp_XRaiseWindow            ptr_XRaiseWindow;
static tp_XRefreshKeyboardMapping ptr_XRefreshKeyboardMapping;
static tp_XScreenOfDisplay        ptr_XScreenOfDisplay;
static tp_XSelectInput            ptr_XSelectInput;
static tp_XSetArcMode             ptr_XSetArcMode;
static tp_XSetBackground          ptr_XSetBackground;
static tp_XSetClipMask            ptr_XSetClipMask;
static tp_XSetClipOrigin          ptr_XSetClipOrigin;
static tp_XSetForeground          ptr_XSetForeground;
static tp_XSetFunction            ptr_XSetFunction;
static tp_XSetStandardProperties  ptr_XSetStandardProperties;
static tp_XSetWMHints             ptr_XSetWMHints;
static tp_XSetWMProtocols         ptr_XSetWMProtocols;
static tp_XStoreColor             ptr_XStoreColor;
static tp_XSync                   ptr_XSync;
static tp_XWhitePixel             ptr_XWhitePixel;



static boolType setupDll (const char *dllName)

  {
    static void *x11Dll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (x11Dll == NULL) {
      x11Dll = dllOpen(dllName);
      if (x11Dll != NULL) {
        if ((ptr_XAllocColor             = (tp_XAllocColor)             dllFunc(x11Dll, "XAllocColor"))             == NULL ||
            (ptr_XAllocColorCells        = (tp_XAllocColorCells)        dllFunc(x11Dll, "XAllocColorCells"))        == NULL ||
            (ptr_XBlackPixel             = (tp_XBlackPixel)             dllFunc(x11Dll, "XBlackPixel"))             == NULL ||
            (ptr_XChangeWindowAttributes = (tp_XChangeWindowAttributes) dllFunc(x11Dll, "XChangeWindowAttributes")) == NULL ||
            (ptr_XCopyArea               = (tp_XCopyArea)               dllFunc(x11Dll, "XCopyArea"))               == NULL ||
            (ptr_XCopyPlane              = (tp_XCopyPlane)              dllFunc(x11Dll, "XCopyPlane"))              == NULL ||
            (ptr_XCreateGC               = (tp_XCreateGC)               dllFunc(x11Dll, "XCreateGC"))               == NULL ||
            (ptr_XCreateImage            = (tp_XCreateImage)            dllFunc(x11Dll, "XCreateImage"))            == NULL ||
            (ptr_XCreatePixmap           = (tp_XCreatePixmap)           dllFunc(x11Dll, "XCreatePixmap"))           == NULL ||
            (ptr_XCreateSimpleWindow     = (tp_XCreateSimpleWindow)     dllFunc(x11Dll, "XCreateSimpleWindow"))     == NULL ||
            (ptr_XDefaultColormap        = (tp_XDefaultColormap)        dllFunc(x11Dll, "XDefaultColormap"))        == NULL ||
            (ptr_XDefaultDepth           = (tp_XDefaultDepth)           dllFunc(x11Dll, "XDefaultDepth"))           == NULL ||
            (ptr_XDefaultRootWindow      = (tp_XDefaultRootWindow)      dllFunc(x11Dll, "XDefaultRootWindow"))      == NULL ||
            (ptr_XDefaultScreen          = (tp_XDefaultScreen)          dllFunc(x11Dll, "XDefaultScreen"))          == NULL ||
            (ptr_XDefaultVisual          = (tp_XDefaultVisual)          dllFunc(x11Dll, "XDefaultVisual"))          == NULL ||
            (ptr_XDestroyImage           = (tp_XDestroyImage)           dllFunc(x11Dll, "XDestroyImage"))           == NULL ||
            (ptr_XDestroyWindow          = (tp_XDestroyWindow)          dllFunc(x11Dll, "XDestroyWindow"))          == NULL ||
            (ptr_XDoesBackingStore       = (tp_XDoesBackingStore)       dllFunc(x11Dll, "XDoesBackingStore"))       == NULL ||
            (ptr_XDrawArc                = (tp_XDrawArc)                dllFunc(x11Dll, "XDrawArc"))                == NULL ||
            (ptr_XDrawImageString16      = (tp_XDrawImageString16)      dllFunc(x11Dll, "XDrawImageString16"))      == NULL ||
            (ptr_XDrawLine               = (tp_XDrawLine)               dllFunc(x11Dll, "XDrawLine"))               == NULL ||
            (ptr_XDrawLines              = (tp_XDrawLines)              dllFunc(x11Dll, "XDrawLines"))              == NULL ||
            (ptr_XDrawPoint              = (tp_XDrawPoint)              dllFunc(x11Dll, "XDrawPoint"))              == NULL ||
            (ptr_XEventsQueued           = (tp_XEventsQueued)           dllFunc(x11Dll, "XEventsQueued"))           == NULL ||
            (ptr_XFillArc                = (tp_XFillArc)                dllFunc(x11Dll, "XFillArc"))                == NULL ||
            (ptr_XFillPolygon            = (tp_XFillPolygon)            dllFunc(x11Dll, "XFillPolygon"))            == NULL ||
            (ptr_XFillRectangle          = (tp_XFillRectangle)          dllFunc(x11Dll, "XFillRectangle"))          == NULL ||
            (ptr_XFlush                  = (tp_XFlush)                  dllFunc(x11Dll, "XFlush"))                  == NULL ||
            (ptr_XFree                   = (tp_XFree)                   dllFunc(x11Dll, "XFree"))                   == NULL ||
            (ptr_XFreeGC                 = (tp_XFreeGC)                 dllFunc(x11Dll, "XFreeGC"))                 == NULL ||
            (ptr_XFreePixmap             = (tp_XFreePixmap)             dllFunc(x11Dll, "XFreePixmap"))             == NULL ||
            (ptr_XGetGeometry            = (tp_XGetGeometry)            dllFunc(x11Dll, "XGetGeometry"))            == NULL ||
            (ptr_XGetImage               = (tp_XGetImage)               dllFunc(x11Dll, "XGetImage"))               == NULL ||
            (ptr_XGetPixel               = (tp_XGetPixel)               dllFunc(x11Dll, "XGetPixel"))               == NULL ||
            (ptr_XGetWindowAttributes    = (tp_XGetWindowAttributes)    dllFunc(x11Dll, "XGetWindowAttributes"))    == NULL ||
            (ptr_XInternAtom             = (tp_XInternAtom)             dllFunc(x11Dll, "XInternAtom"))             == NULL ||
            (ptr_XKeysymToKeycode        = (tp_XKeysymToKeycode)        dllFunc(x11Dll, "XKeysymToKeycode"))        == NULL ||
            (ptr_XLookupString           = (tp_XLookupString )          dllFunc(x11Dll, "XLookupString"))           == NULL ||
            (ptr_XLowerWindow            = (tp_XLowerWindow)            dllFunc(x11Dll, "XLowerWindow"))            == NULL ||
            (ptr_XMapRaised              = (tp_XMapRaised)              dllFunc(x11Dll, "XMapRaised"))              == NULL ||
            (ptr_XMoveWindow             = (tp_XMoveWindow)             dllFunc(x11Dll, "XMoveWindow"))             == NULL ||
            (ptr_XNextEvent              = (tp_XNextEvent)              dllFunc(x11Dll, "XNextEvent"))              == NULL ||
            (ptr_XOpenDisplay            = (tp_XOpenDisplay)            dllFunc(x11Dll, "XOpenDisplay"))            == NULL ||
            (ptr_XPutImage               = (tp_XPutImage)               dllFunc(x11Dll, "XPutImage"))               == NULL ||
            (ptr_XQueryColor             = (tp_XQueryColor)             dllFunc(x11Dll, "XQueryColor"))             == NULL ||
            (ptr_XQueryKeymap            = (tp_XQueryKeymap)            dllFunc(x11Dll, "XQueryKeymap"))            == NULL ||
            (ptr_XQueryPointer           = (tp_XQueryPointer)           dllFunc(x11Dll, "XQueryPointer"))           == NULL ||
            (ptr_XRaiseWindow            = (tp_XRaiseWindow)            dllFunc(x11Dll, "XRaiseWindow"))            == NULL ||
            (ptr_XRefreshKeyboardMapping = (tp_XRefreshKeyboardMapping) dllFunc(x11Dll, "XRefreshKeyboardMapping")) == NULL ||
            (ptr_XScreenOfDisplay        = (tp_XScreenOfDisplay)        dllFunc(x11Dll, "XScreenOfDisplay"))        == NULL ||
            (ptr_XSelectInput            = (tp_XSelectInput)            dllFunc(x11Dll, "XSelectInput"))            == NULL ||
            (ptr_XSetArcMode             = (tp_XSetArcMode)             dllFunc(x11Dll, "XSetArcMode"))             == NULL ||
            (ptr_XSetBackground          = (tp_XSetBackground)          dllFunc(x11Dll, "XSetBackground"))          == NULL ||
            (ptr_XSetClipMask            = (tp_XSetClipMask)            dllFunc(x11Dll, "XSetClipMask"))            == NULL ||
            (ptr_XSetClipOrigin          = (tp_XSetClipOrigin)          dllFunc(x11Dll, "XSetClipOrigin"))          == NULL ||
            (ptr_XSetForeground          = (tp_XSetForeground)          dllFunc(x11Dll, "XSetForeground"))          == NULL ||
            (ptr_XSetFunction            = (tp_XSetFunction)            dllFunc(x11Dll, "XSetFunction"))            == NULL ||
            (ptr_XSetStandardProperties  = (tp_XSetStandardProperties)  dllFunc(x11Dll, "XSetStandardProperties"))  == NULL ||
            (ptr_XSetWMHints             = (tp_XSetWMHints)             dllFunc(x11Dll, "XSetWMHints"))             == NULL ||
            (ptr_XSetWMProtocols         = (tp_XSetWMProtocols)         dllFunc(x11Dll, "XSetWMProtocols"))         == NULL ||
            (ptr_XStoreColor             = (tp_XStoreColor)             dllFunc(x11Dll, "XStoreColor"))             == NULL ||
            (ptr_XSync                   = (tp_XSync)                   dllFunc(x11Dll, "XSync"))                   == NULL ||
            (ptr_XWhitePixel             = (tp_XWhitePixel)             dllFunc(x11Dll, "XWhitePixel"))             == NULL) {
          x11Dll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", x11Dll != NULL););
    return x11Dll != NULL;
  } /* setupDll */



boolType findX11Dll (void)

  {
    const char *dllList[] = { X11_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findX11Dll */
    logFunction(printf("findX11Dll()\n"););
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    logFunction(printf("findX11Dll --> %d\n", found););
    return found;
  } /* findX11Dll */



Status XAllocColor (Display *display, Colormap colormap, XColor *screen_in_out)

  {
    Status status;

  /* XAllocColor */
    logFunction(printf("XAllocColor(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM ")\n",
                       (memSizeType) display, colormap,
                       (memSizeType) screen_in_out););
    status = ptr_XAllocColor(display, colormap, screen_in_out);
    logFunction(printf("XAllocColor --> %d\n", status););
    return status;
  } /* XAllocColor */



Status XAllocColorCells (Display *display, Colormap colormap, Bool contig,
                         unsigned long *plane_masks_return,
                         unsigned int nplanes, unsigned long *pixels_return,
                         unsigned int npixels)

  {
    Status status;

  /* XAllocColorCells */
    logFunction(printf("XAllocColorCells(" FMT_U_MEM ", " FMT_U_XID
                       ", %d, *, %u, *, %u)\n",
                       (memSizeType) display, colormap,
                       contig, nplanes, npixels););
    status = ptr_XAllocColorCells(display, colormap, contig,
                                  plane_masks_return, nplanes, pixels_return,
                                  npixels);
    logFunction(printf("XAllocColorCells(" FMT_U_MEM ", " FMT_U_XID
                       ", %d, %lu, %u, %lu, %u) --> %d\n",
                       (memSizeType) display, colormap, contig,
                       *plane_masks_return, nplanes, *pixels_return, npixels,
                       status););
    return status;
  } /* XAllocColorCells */



unsigned long XBlackPixel (Display *display, int screen_number)

  {
    unsigned long blackPixel;

  /* XBlackPixel */
    logFunction(printf("XBlackPixel(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    blackPixel = ptr_XBlackPixel(display, screen_number);
    logFunction(printf("XBlackPixel --> %lu\n", blackPixel););
    return blackPixel;
  } /* XBlackPixel */



int XChangeWindowAttributes (Display *display, Window window,
                             unsigned long valuemask,
                             XSetWindowAttributes *attributes)

  {
    int funcResult;

  /* XChangeWindowAttributes */
    logFunction(printf("XChangeWindowAttributes(" FMT_U_MEM ", " FMT_U_XID
                       ", %lx, " FMT_U_MEM ")\n",
                       (memSizeType) display, window, valuemask,
                       (memSizeType) attributes););
    funcResult = ptr_XChangeWindowAttributes(display, window, valuemask,
                                             attributes);
    logFunction(printf("XChangeWindowAttributes --> %d\n", funcResult););
    return funcResult;
  } /* XChangeWindowAttributes */



int XCopyArea (Display *display, Drawable src, Drawable dest, GC gc,
               int src_x, int src_y, unsigned int width, unsigned height,
               int dest_x, int dest_y)
  {
    int funcResult;

  /* XCopyArea */
    logFunction(printf("XCopyArea(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_XID
                       ", " FMT_U_MEM ", %d, %d, %u, %u, %d, %d)\n",
                       (memSizeType) display, src, dest, (memSizeType) gc,
                       src_x, src_y, width, height, dest_x, dest_y););
    funcResult = ptr_XCopyArea(display, src, dest, gc, src_x, src_y,
                               width, height, dest_x, dest_y);
    logFunction(printf("XCopyArea --> %d\n", funcResult););
    return funcResult;
  } /* XCopyArea */



int XCopyPlane (Display *display, Drawable src, Drawable dest,
                GC gc, int src_x, int src_y, unsigned int width,
                unsigned int height, int dest_x, int dest_y,
                unsigned long plane)

  {
    int funcResult;

  /* XCopyPlane */
    logFunction(printf("XCopyPlane(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_XID
                       ", " FMT_U_MEM ", %d, %d, %u, %u, %d, %d, %lu)\n",
                       (memSizeType) display, src, dest, (memSizeType) gc,
                       src_x, src_y, width, height, dest_x, dest_y, plane););
    funcResult = ptr_XCopyPlane(display, src, dest, gc, src_x, src_y,
                                width, height, dest_x, dest_y, plane);
    logFunction(printf("XCopyPlane --> %d\n", funcResult););
    return funcResult;
  } /* XCopyPlane */



GC XCreateGC (Display *display, Drawable drawable, unsigned long valuemask,
              XGCValues *values)

  {
    GC gc;

  /* XCreateGC */
    logFunction(printf("XCreateGC(" FMT_U_MEM ", " FMT_U_XID ", %lu, " FMT_U_MEM ")\n",
                       (memSizeType) display, drawable, valuemask,
                       (memSizeType) values););
    gc = ptr_XCreateGC(display, drawable, valuemask, values);
    logFunction(printf("XCreateGC --> " FMT_U_MEM "\n", (memSizeType) gc););
    return gc;
  } /* XCreateGC */



XImage *XCreateImage (Display *display, Visual *visual,
                      unsigned int depth, int format, int offset, char *data,
                      unsigned int width, unsigned int height,
                      int bitmap_pad, int bytes_per_line)

  {
    XImage *xImage;

  /* XCreateImage */
    logFunction(printf("XCreateImage(" FMT_U_MEM ", " FMT_U_MEM
                       ", %u, %d, %d, " FMT_U_MEM ", %u, %u, %d, %d)\n",
                       (memSizeType) display, (memSizeType) visual, depth,
                       format, offset, (memSizeType) data, width, height,
                       bitmap_pad, bytes_per_line););
    xImage = ptr_XCreateImage(display, visual, depth, format, offset, data,
                              width, height, bitmap_pad, bytes_per_line);
    logFunction(printf("XCreateImage --> " FMT_U_MEM "\n",
                       (memSizeType) xImage););
    return xImage;
  } /* XCreateImage */



Pixmap XCreatePixmap (Display *display, Drawable drawable, unsigned int width,
                      unsigned int height, unsigned int depth)

  {
    Pixmap pixmap;

  /* XCreatePixmap */
    logFunction(printf("XCreatePixmap(" FMT_U_MEM ", " FMT_U_XID ", %u, %u, %u)\n",
                       (memSizeType) display, drawable, width, height,
                       depth););
    pixmap = ptr_XCreatePixmap(display, drawable, width, height, depth);
    logFunction(printf("XCreatePixmap -> " FMT_U_MEM "\n", pixmap););
    return pixmap;
  } /* XCreatePixmap */



Window XCreateSimpleWindow (Display *display, Window parent, int x, int y,
                            unsigned int width, unsigned int height,
                            unsigned int border_width,
                            unsigned long border,
                            unsigned long background)

  {
    Window window;

  /* XCreateSimpleWindow */
    logFunction(printf("XCreateSimpleWindow(" FMT_U_MEM ", " FMT_U_MEM
                       ", %d, %d, %u, %u, %u, %lu, %lu)\n",
                       (memSizeType) display, parent, x, y, width, height,
                       border_width, border, background););
    window = ptr_XCreateSimpleWindow(display, parent, x, y, width, height,
                                     border_width, border, background);
    logFunction(printf("XCreateSimpleWindow --> " FMT_U_MEM "\n",
                       window););
    return window;
  } /* XCreateSimpleWindow */



Colormap XDefaultColormap (Display *display, int screen_number)

  {
    Colormap defaultColormap;

  /* XDefaultColormap */
    logFunction(printf("XDefaultColormap(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    defaultColormap = ptr_XDefaultColormap(display, screen_number);
    logFunction(printf("XDefaultColormap --> " FMT_D_XID "\n",
                       defaultColormap););
    return defaultColormap;
  } /* XDefaultColormap */



int XDefaultDepth (Display *display, int screen_number)

  {
    int defaultDepth;

  /* XDefaultDepth */
    logFunction(printf("XDefaultDepth(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    defaultDepth = ptr_XDefaultDepth(display, screen_number);
    logFunction(printf("XDefaultDepth --> %d\n", defaultDepth););
    return defaultDepth;
  } /* XDefaultDepth */



Window XDefaultRootWindow (Display *display)

  {
    Window defaultRootWindow;

  /* XDefaultRootWindow */
    logFunction(printf("XDefaultRootWindow(" FMT_U_MEM ")\n",
                       (memSizeType) display););
    defaultRootWindow = ptr_XDefaultRootWindow(display);
    logFunction(printf("XDefaultRootWindow --> " FMT_D_XID "\n",
                       defaultRootWindow););
    return defaultRootWindow;
  } /* XDefaultRootWindow */



int XDefaultScreen (Display *display)

  {
    int defaultScreen;

  /* XDefaultScreen */
    logFunction(printf("XDefaultScreen(" FMT_U_MEM ")\n",
                       (memSizeType) display););
    defaultScreen = ptr_XDefaultScreen(display);
    logFunction(printf("XDefaultScreen --> %d\n", defaultScreen););
    return defaultScreen;
  } /* XDefaultScreen */



Visual *XDefaultVisual (Display *display, int screen_number)

  {
    Visual *defaultVisual;

  /* XDefaultVisual */
    logFunction(printf("XDefaultVisual(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    defaultVisual = ptr_XDefaultVisual(display, screen_number);
    logFunction(printf("XDefaultVisual --> " FMT_U_MEM "\n",
                       (memSizeType) defaultVisual););
    return defaultVisual;
  } /* XDefaultVisual */



int XDestroyImage (XImage *ximage)

  {
    int funcResult;

  /* XDestroyImage */
    logFunction(printf("XDestroyImage(" FMT_U_MEM ")\n",
                       (memSizeType) ximage););
    funcResult = ptr_XDestroyImage(ximage);
    logFunction(printf("XDestroyImage --> %d\n", funcResult););
    return funcResult;
  } /* XDestroyImage */



int XDestroyWindow (Display *display, Window window)

  {
    int funcResult;

  /* XDestroyWindow */
    logFunction(printf("XDestroyWindow(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) display, window););
    funcResult = ptr_XDestroyWindow(display, window);
    logFunction(printf("XDestroyWindow --> %d\n", funcResult););
    return funcResult;
  } /* XDestroyWindow */



int XDoesBackingStore (Screen *screen)

  {
    int backingStore;

  /* XDoesBackingStore */
    logFunction(printf("XDoesBackingStore(" FMT_U_MEM ")\n",
                       (memSizeType) screen););
    backingStore = ptr_XDoesBackingStore(screen);
    logFunction(printf("XDoesBackingStore --> %d\n", backingStore););
    return backingStore;
  } /* XDoesBackingStore */



int XDrawArc (Display *display, Drawable drawable, GC gc, int x, int y,
              unsigned int width, unsigned int height, int angle1, int angle2)

  {
    int funcResult;

  /* XDrawArc */
    logFunction(printf("XDrawArc(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", %d, %d, %u, %u, %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x, y, width, height, angle1, angle2););
    funcResult = ptr_XDrawArc(display, drawable, gc, x, y, width, height,
                              angle1, angle2);
    logFunction(printf("XDrawArc --> %d\n", funcResult););
    return funcResult;
  } /* XDrawArc */



int XDrawImageString16 (Display *display, Drawable drawable, GC gc,
                        int x, int y, const XChar2b *string, int length)

  {
    int funcResult;

  /* XDrawImageString16 */
    logFunction(printf("XDrawImageString16(" FMT_U_MEM ", " FMT_U_XID ", "
                       FMT_U_MEM ", %d, %d, " FMT_U_MEM ", %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x, y, (memSizeType) string, length););
    funcResult = ptr_XDrawImageString16(display, drawable, gc, x, y, string,
                                        length);
    logFunction(printf("XDrawImageString16 --> %d\n", funcResult););
    return funcResult;
  } /* XDrawImageString16 */



int XDrawLine (Display *display, Drawable drawable, GC gc, int x1, int y1,
               int x2, int y2)

  {
    int funcResult;

  /* XDrawLine */
    logFunction(printf("XDrawLine(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", %d, %d, %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x1, y1, x2, y2););
    funcResult = ptr_XDrawLine(display, drawable, gc, x1, y1, x2, y2);
    logFunction(printf("XDrawLine --> %d\n", funcResult););
    return funcResult;
  } /* XDrawLine */



int XDrawLines (Display *display, Drawable drawable, GC gc, XPoint *points,
                int npoints, int mode)

  {
    int funcResult;

  /* XDrawLines */
    logFunction(printf("XDrawLines(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", " FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       (memSizeType) points, npoints, mode););
    funcResult = ptr_XDrawLines(display, drawable, gc, points, npoints, mode);
    logFunction(printf("XDrawLines --> %d\n", funcResult););
    return funcResult;
  } /* XDrawLines */



int XDrawPoint (Display *display, Drawable drawable, GC gc, int x, int y)

  {
    int funcResult;

  /* XDrawPoint */
    logFunction(printf("XDrawPoint(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x, y););
    funcResult = ptr_XDrawPoint(display, drawable, gc, x, y);
    logFunction(printf("XDrawPoint --> %d\n", funcResult););
    return funcResult;
  } /* XDrawPoint */



int XEventsQueued (Display *display, int mode)

  {
    int funcResult;

  /* XEventsQueued */
    logFunction(printf("XEventsQueued(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, mode););
    funcResult = ptr_XEventsQueued(display, mode);
    logFunction(printf("XEventsQueued --> %d\n", funcResult););
    return funcResult;
  } /* XEventsQueued */



int XFillArc (Display *display, Drawable drawable, GC gc, int x, int y,
              unsigned int width, unsigned int height, int angle1, int angle2)

  {
    int funcResult;

  /* XFillArc */
    logFunction(printf("XFillArc(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", %d, %d, %u, %u, %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x, y, width, height, angle1, angle2););
    funcResult = ptr_XFillArc(display, drawable, gc, x, y, width, height,
                        angle1, angle2);
    logFunction(printf("XFillArc --> %d\n", funcResult););
    return funcResult;
  } /* XFillArc */



int XFillPolygon (Display *display, Drawable drawable, GC gc, XPoint *points,
                  int npoints, int shape, int mode)

  {
    int funcResult;

  /* XFillPolygon */
    logFunction(printf("XFillPolygon(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", " FMT_U_MEM ", %d, %d, %d)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       (memSizeType) points, npoints, shape, mode););
    funcResult = ptr_XFillPolygon(display, drawable, gc, points, npoints,
                                  shape, mode);
    logFunction(printf("XFillPolygon --> %d\n", funcResult););
    return funcResult;
  } /* XFillPolygon */



int XFillRectangle (Display *display, Drawable drawable, GC gc, int x, int y,
                    unsigned int width, unsigned int height)

  {
    int funcResult;

  /* XFillRectangle */
    logFunction(printf("XFillRectangle(" FMT_U_MEM ", " FMT_U_XID ", "
                       FMT_U_MEM ", %d, %d, %u, %u)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       x, y, width, height););
    funcResult = ptr_XFillRectangle(display, drawable, gc, x, y, width,
                                    height);
    logFunction(printf("XFillRectangle --> %d\n", funcResult););
    return funcResult;
  } /* XFillRectangle */



int XFlush (Display *display)

  {
    int funcResult;

  /* XFlush */
    logFunction(printf("XFlush(" FMT_U_MEM ")\n", (memSizeType) display););
    funcResult = ptr_XFlush(display);
    logFunction(printf("XFlush --> %d\n", funcResult););
    return funcResult;
  } /* XFlush */



int XFree (void *data)

  {
    int funcResult;

  /* XFree */
    logFunction(printf("XFree(" FMT_U_MEM ")\n", (memSizeType) data););
    funcResult = ptr_XFree(data);
    logFunction(printf("XFree --> %d\n", funcResult););
    return funcResult;
  } /* XFree */



int XFreeGC (Display *display, GC gc)

  {
    int funcResult;

  /* XFreeGC */
    logFunction(printf("XFreeGC(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) display, (memSizeType) gc););
    funcResult = ptr_XFreeGC(display, gc);
    logFunction(printf("XFreeGC --> %d\n", funcResult););
    return funcResult;
  } /* XFreeGC */



int XFreePixmap (Display *display, Pixmap pixmap)

  {
    int funcResult;

  /* XFreePixmap */
    logFunction(printf("XFreePixmap(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) display, pixmap););
    funcResult = ptr_XFreePixmap(display, pixmap);
    logFunction(printf("XFreePixmap --> %d\n", funcResult););
    return funcResult;
  } /* XFreePixmap */



Status XGetGeometry (Display *display, Drawable drawable,
                     Window *root_return,
                     int *x_return, int *y_return,
                     unsigned int *width_return,
                     unsigned int *height_return,
                     unsigned int *border_width_return,
                     unsigned int *depth_return)

  {
    Status status;

  /* XGetGeometry */
    logFunction(printf("XGetGeometry(" FMT_U_MEM ", " FMT_U_XID
                       ", *, *, *, *, *, *, *)\n",
                       (memSizeType) display, drawable););
    status = ptr_XGetGeometry(display, drawable,
                              root_return, x_return, y_return,
                              width_return, height_return,
                              border_width_return, depth_return);
    logFunction(printf("XGetGeometry(" FMT_U_MEM ", " FMT_U_XID
                       ", " FMT_U_XID ", %d, %d, %u, %u, %u, %u) --> %d\n",
                       (memSizeType) display, drawable, *root_return,
                       *x_return, *y_return,
                       *width_return, *height_return,
                       *border_width_return, *depth_return,
                       status););
    return status;
  } /* XGetGeometry */



XImage *XGetImage (Display *display, Drawable drawable, int x, int y,
                   unsigned int width, unsigned int height,
                   unsigned long plane_mask, int format)

  {
    XImage *xImage;

  /* XGetImage */
    logFunction(printf("XGetImage(" FMT_U_MEM ", " FMT_U_XID
                       ", %d, %d, %u, %u, %lu, %d)\n",
                       (memSizeType) display, drawable, x, y, width, height,
                       plane_mask, format););
    xImage = ptr_XGetImage(display, drawable, x, y, width, height,
                           plane_mask, format);
    logFunction(printf("XGetImage --> " FMT_U_MEM "\n",
                       (memSizeType) xImage););
    return xImage;
  } /* XGetImage */



unsigned long XGetPixel (XImage *ximage, int x, int y)

  {
    unsigned long pixel;

  /* XGetPixel */
    logFunction(printf("XGetPixel(" FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) ximage, x, y););
    pixel = ptr_XGetPixel(ximage, x, y);
    logFunction(printf("XGetPixel --> %lu\n", pixel););
    return pixel;
  } /* XGetPixel */



Status XGetWindowAttributes (Display *display, Window window,
                             XWindowAttributes *window_attributes_return)

  {
    Status status;

  /* XGetWindowAttributes */
    logFunction(printf("XGetWindowAttributes(" FMT_U_MEM ", " FMT_U_XID
                       ", " FMT_U_MEM ")\n",
                       (memSizeType) display, window,
                       (memSizeType) window_attributes_return););
    status = ptr_XGetWindowAttributes(display, window,
                                    window_attributes_return);
    logFunction(printf("XGetWindowAttributes --> %d\n", status););
    return status;
  } /* XGetWindowAttributes */



Atom XInternAtom (Display *display, const char *atom_name,
                  Bool only_if_exists)

  {
    Atom atom;

  /* XInternAtom */
    logFunction(printf("XInternAtom(" FMT_U_MEM ", \"%s\", %d)\n",
                       (memSizeType) display, atom_name, only_if_exists););
    atom = ptr_XInternAtom(display, atom_name, only_if_exists);
    logFunction(printf("XInternAtom --> " FMT_U_ATOM "\n", atom););
    return atom;
  } /* XInternAtom */



KeyCode XKeysymToKeycode (Display *display, KeySym keysym)

  {
    KeyCode keyCode;

  /* XKeysymToKeycode */
    logFunction(printf("XKeysymToKeycode(" FMT_U_MEM ", " FMT_U_XID ")\n",
                       (memSizeType) display, keysym););
    keyCode = ptr_XKeysymToKeycode(display, keysym);
    logFunction(printf("XKeysymToKeycode --> '%c' (%d)\n",
                       keyCode >= ' ' && keyCode <= '~' ? keyCode : '?',
                       keyCode););
    return keyCode;
  } /* XKeysymToKeycode */



int XLookupString (XKeyEvent *event_struct, char *buffer_return,
                   int bytes_buffer, KeySym *keysym_return,
                   XComposeStatus *status_in_out)

  {
    int funcResult;

  /* XLookupString */
    logFunction(printf("XLookupString(" FMT_U_MEM ", " FMT_U_MEM
                       ", %d, " FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) event_struct,
                       (memSizeType) buffer_return, bytes_buffer,
                       (memSizeType) keysym_return,
                       (memSizeType) status_in_out););
    funcResult = ptr_XLookupString (event_struct, buffer_return, bytes_buffer,
                                    keysym_return, status_in_out);
    logFunction(printf("XLookupString --> %d\n", funcResult););
    return funcResult;
  } /* XLookupString */



int XLowerWindow (Display *display, Window window)

  {
    int funcResult;

  /* XLowerWindow */
    logFunction(printf("XLowerWindow(" FMT_U_MEM ", " FMT_U_XID ")\n",
                       (memSizeType) display, window););
    funcResult = ptr_XLowerWindow(display, window);
    logFunction(printf("XLowerWindow --> %d\n", funcResult););
    return funcResult;
  } /* XLowerWindow */



int XMapRaised (Display *display, Window window)

  {
    int funcResult;

  /* XMapRaised */
    logFunction(printf("XMapRaised(" FMT_U_MEM ", " FMT_U_XID ")\n",
                       (memSizeType) display, window););
    funcResult = ptr_XMapRaised(display, window);
    logFunction(printf("XMapRaised --> %d\n", funcResult););
    return funcResult;
  } /* XMapRaised */



int XMoveWindow (Display *display, Window window, int x, int y)

  {
    int funcResult;

  /* XMoveWindow */
    logFunction(printf("XMoveWindow(" FMT_U_MEM ", " FMT_U_XID ", %d, %d)\n",
                       (memSizeType) display, window, x, y););
    funcResult = ptr_XMoveWindow(display, window, x, y);
    logFunction(printf("XMoveWindow --> %d\n", funcResult););
    return funcResult;
  } /* XMoveWindow */



int XNextEvent (Display *display, XEvent *event_return)

  {
    int funcResult;

  /* XNextEvent */
    logFunction(printf("XNextEvent(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) display, (memSizeType) event_return););
    funcResult = ptr_XNextEvent(display, event_return);
    logFunction(printf("XNextEvent --> %d\n", funcResult););
    return funcResult;
  } /* XNextEvent */



Display *XOpenDisplay (const char *display_name)

  {
    Display *display;

  /* XOpenDisplay */
    logFunction(printf("XOpenDisplay(\"%s\")\n", display_name););
    display = ptr_XOpenDisplay(display_name);
    logFunction(printf("XOpenDisplay --> " FMT_U_MEM "\n",
                       (memSizeType) display););
    return display;
  } /* XOpenDisplay */



int XPutImage (Display *display, Drawable drawable, GC gc,
               XImage *image, int src_x, int src_y,
               int dest_x, int dest_y, unsigned int width,
               unsigned int height)

  {
    int funcResult;

  /* XPutImage */
    logFunction(printf("XPutImage(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_MEM
                       ", " FMT_U_MEM ", %d, %d, %d, %d, %u, %u)\n",
                       (memSizeType) display, drawable, (memSizeType) gc,
                       (memSizeType) image, src_x, src_y, dest_x, dest_y,
                       width, height););
    funcResult = ptr_XPutImage(display, drawable, gc, image, src_x,
                               src_y, dest_x, dest_y, width, height);
    logFunction(printf("XPutImage --> %d\n", funcResult););
    return funcResult;
  } /* XPutImage */



int XQueryColor (Display *display, Colormap colormap,
                 XColor *def_in_out)

  {
    int funcResult;

  /* XQueryColor */
    logFunction(printf("XQueryColor(" FMT_U_MEM ", " FMT_U_XID
                       ", " FMT_U_MEM ")\n",
                       (memSizeType) display, colormap,
                       (memSizeType) def_in_out););
    funcResult = ptr_XQueryColor(display, colormap, def_in_out);
    logFunction(printf("XQueryColor --> %d\n", funcResult););
    return funcResult;
  } /* XQueryColor */



int XQueryKeymap (Display *display, char keys_return[32])

  {
    int funcResult;

  /* XQueryKeymap */
    logFunction(printf("XQueryKeymap(" FMT_U_MEM ", *)\n",
                       (memSizeType) display););
    funcResult = ptr_XQueryKeymap(display, keys_return);
    logFunction(printf("XQueryKeymap -> %d\n", funcResult););
    return funcResult;
  } /* XQueryKeymap */



Bool XQueryPointer (Display *display, Window window,
                    Window *root_return, Window *child_return,
                    int *root_x_return, int *root_y_return,
                    int *win_x_return, int *win_y_return,
                    unsigned int *mask_return)

  {
    Bool okay;

  /* XQueryPointer */
    logFunction(printf("XQueryPointer(" FMT_U_MEM ", " FMT_U_XID
                       ", *, *, *, *, *, *, *)\n",
                       (memSizeType) display, window););
    okay = ptr_XQueryPointer(display, window, root_return,
                             child_return, root_x_return,
                             root_y_return, win_x_return,
                             win_y_return, mask_return);
    logFunction(printf("XQueryPointer(" FMT_U_MEM ", " FMT_U_XID ", " FMT_U_XID
                       ", " FMT_U_XID ", %d, %d, %d, %d, %u) --> %d\n",
                       (memSizeType) display, window, *root_return,
                       *child_return, *root_x_return, *root_y_return,
                       *win_x_return, *win_y_return, *mask_return, okay););
    return okay;
  } /* XQueryPointer */



int XRaiseWindow (Display *display, Window window)

  {
    int funcResult;

  /* XRaiseWindow */
    logFunction(printf("XRaiseWindow(" FMT_U_MEM ", " FMT_U_XID ")\n",
                       (memSizeType) display, window););
    funcResult = ptr_XRaiseWindow(display, window);
    logFunction(printf("XRaiseWindow -> %d\n", funcResult););
    return funcResult;
  } /* XRaiseWindow */



int XRefreshKeyboardMapping (XMappingEvent *event_map)

  {
    int funcResult;

  /* XRefreshKeyboardMapping */
    logFunction(printf("XRefreshKeyboardMapping(" FMT_U_MEM ")\n",
                       (memSizeType) event_map););
    funcResult = ptr_XRefreshKeyboardMapping(event_map);
    logFunction(printf("XRefreshKeyboardMapping -> %d\n", funcResult););
    return funcResult;
  } /* XRefreshKeyboardMapping */



Screen *XScreenOfDisplay (Display *display, int screen_number)

  {
    Screen *screenOfDisplay;

  /* XScreenOfDisplay */
    logFunction(printf("XScreenOfDisplay(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    screenOfDisplay = ptr_XScreenOfDisplay(display, screen_number);
    logFunction(printf("XScreenOfDisplay -> " FMT_U_MEM "\n",
                       (memSizeType) screenOfDisplay););
    return screenOfDisplay;
  } /* XScreenOfDisplay */



int XSelectInput (Display *display, Window window, long event_mask)

  {
    int funcResult;

  /* XSelectInput */
    logFunction(printf("XSelectInput(" FMT_U_MEM ", " FMT_U_XID ", %ld)\n",
                       (memSizeType) display, window, event_mask););
    funcResult = ptr_XSelectInput(display, window, event_mask);
    logFunction(printf("XSelectInput --> %d\n", funcResult););
    return funcResult;
  } /* XSelectInput */



int XSetArcMode (Display *display, GC gc, int arc_mode)

  {
    int funcResult;

  /* XSetArcMode */
    logFunction(printf("XSetArcMode(" FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       (memSizeType) display, (memSizeType) gc, arc_mode););
    funcResult = ptr_XSetArcMode(display, gc, arc_mode);
    logFunction(printf("XSetArcMode --> %d\n", funcResult););
    return funcResult;
  } /* XSetArcMode */



int XSetBackground (Display *display, GC gc, unsigned long background)

  {
    int funcResult;

  /* XSetBackground */
    logFunction(printf("XSetBackground(" FMT_U_MEM ", " FMT_U_MEM ", %lu)\n",
                       (memSizeType) display, (memSizeType) gc, background););
    funcResult = ptr_XSetBackground(display, gc, background);
    logFunction(printf("XSetBackground --> %d\n", funcResult););
    return funcResult;
  } /* XSetBackground */



int XSetClipMask (Display *display, GC gc, Pixmap pixmap)

  {
    int funcResult;

  /* XSetClipMask */
    logFunction(printf("XSetClipMask(" FMT_U_MEM ", " FMT_U_MEM ", " FMT_U_XID ")\n",
                       (memSizeType) display, (memSizeType) gc, pixmap););
    funcResult = ptr_XSetClipMask(display, gc, pixmap);
    logFunction(printf("XSetClipMask --> %d\n", funcResult););
    return funcResult;
  } /* XSetClipMask */



int XSetClipOrigin (Display *display, GC gc, int clip_x_origin,
                    int clip_y_origin)

  {
    int funcResult;

  /* XSetClipOrigin */
    logFunction(printf("XSetClipOrigin(" FMT_U_MEM ", " FMT_U_MEM ", %d, %d)\n",
                       (memSizeType) display, (memSizeType) gc, clip_x_origin,
                       clip_y_origin););
    funcResult = ptr_XSetClipOrigin(display, gc, clip_x_origin,
                                    clip_y_origin);
    logFunction(printf("XSetClipOrigin --> %d\n", funcResult););
    return funcResult;
  } /* XSetClipOrigin */



int XSetForeground (Display *display, GC gc, unsigned long foreground)

  {
    int funcResult;

  /* XSetForeground */
    logFunction(printf("XSetForeground(" FMT_U_MEM ", " FMT_U_MEM ", %lu)\n",
                       (memSizeType) display, (memSizeType) gc, foreground););
    funcResult = ptr_XSetForeground(display, gc, foreground);
    logFunction(printf("XSetForeground --> %d\n", funcResult););
    return funcResult;
  } /* XSetForeground */



int XSetFunction (Display *display, GC gc, int function)

  {
    int funcResult;

  /* XSetFunction */
    logFunction(printf("XSetFunction(" FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       (memSizeType) display, (memSizeType) gc, function););
    funcResult = ptr_XSetFunction(display, gc, function);
    logFunction(printf("XSetFunction --> %d\n", funcResult););
    return funcResult;
  } /* XSetFunction */



int XSetStandardProperties (Display *display, Window window,
                            const char *window_name, const char *icon_name,
                            Pixmap icon_pixmap, char **argv, int argc,
                            XSizeHints *hints)

  {
    int funcResult;

  /* XSetStandardProperties */
    logFunction(printf("XSetStandardProperties(" FMT_U_MEM ", " FMT_U_XID
                       ", \"%s\", \"%s\", " FMT_U_XID ", " FMT_U_MEM
                       ", %d, " FMT_U_MEM ")\n",
                       (memSizeType) display, window, window_name,
                       icon_name, icon_pixmap, (memSizeType) argv,
                       argc, (memSizeType) hints););
    funcResult = ptr_XSetStandardProperties(display, window, window_name,
                                            icon_name, icon_pixmap, argv,
                                            argc, hints);
    logFunction(printf("XSetStandardProperties --> %d\n", funcResult););
    return funcResult;
  } /* XSetStandardProperties */



int XSetWMHints (Display *display, Window window, XWMHints *wm_hints)

  {
    int funcResult;

  /* XSetWMHints */
    logFunction(printf("XSetWMHints(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_U_MEM ")\n",
                       (memSizeType) display, window, (memSizeType) wm_hints););
    funcResult = ptr_XSetWMHints(display, window, wm_hints);
    logFunction(printf("XSetWMHints --> %d\n", funcResult););
    return funcResult;
  } /* XSetWMHints */



Status XSetWMProtocols (Display *display, Window window, Atom *protocols,
                        int count)

  {
    Status status;

  /* XSetWMProtocols */
    logFunction(printf("XSetWMProtocols(" FMT_U_MEM ", " FMT_U_XID
                       ", " FMT_U_MEM ", %d)\n",
                       (memSizeType) display, window, (memSizeType) protocols,
                       count););
    status = ptr_XSetWMProtocols(display, window, protocols, count);
    logFunction(printf("XSetWMProtocols --> %d\n", status););
    return status;
  } /* XSetWMProtocols */



int XStoreColor (Display *display, Colormap colormap, XColor *color)

  {
    int funcResult;

  /* XStoreColor */
    logFunction(printf("XStoreColor(" FMT_U_MEM ", " FMT_U_XID
                       ", " FMT_U_MEM ")\n",
                       (memSizeType) display, colormap,
                       (memSizeType) color););
    funcResult = ptr_XStoreColor(display, colormap, color);
    logFunction(printf("XStoreColor --> %d\n", funcResult););
    return funcResult;
  } /* XStoreColor */



int XSync (Display *display, Bool discard)

  {
    int funcResult;

  /* XSync */
    logFunction(printf("XSync(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, discard););
    funcResult = ptr_XSync(display, discard);
    logFunction(printf("XSync --> %d\n", funcResult););
    return funcResult;
  } /* XSync */



unsigned long XWhitePixel (Display *display, int screen_number)

  {
    unsigned long whitePixel;

  /* XWhitePixel */
    logFunction(printf("XWhitePixel(" FMT_U_MEM ", %d)\n",
                       (memSizeType) display, screen_number););
    whitePixel = ptr_XWhitePixel(display, screen_number);
    logFunction(printf("XWhitePixel --> %lu\n", whitePixel););
    return whitePixel;
  } /* XWhitePixel */

#endif
