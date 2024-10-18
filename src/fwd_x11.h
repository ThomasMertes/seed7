/********************************************************************/
/*                                                                  */
/*  fwd_x11.h     Forward X11 calls to a shared X11 library.        */
/*  Copyright (C) 1989 - 2021  Thomas Mertes                        */
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
/*  File: seed7/src/fwd_x11.h                                       */
/*  Changes: 2019 - 2021  Thomas Mertes                             */
/*  Content: Forward X11 calls to a shared X11 library.             */
/*                                                                  */
/********************************************************************/

#ifndef FWD_X11_H
#define FWD_X11_H

#ifdef X11_XRENDER_INCLUDE
#include X11_XRENDER_INCLUDE
#endif

typedef Status (*tp_XAllocColor) (Display *display, Colormap colormap, XColor *screen_in_out);
typedef Status (*tp_XAllocColorCells) (Display *display, Colormap colormap, Bool contig,
                                       unsigned long *plane_masks_return,
                                       unsigned int nplanes, unsigned long *pixels_return,
                                       unsigned int npixels);
typedef unsigned long (*tp_XBlackPixel) (Display *display,
                                         int screen_number);
typedef int (*tp_XChangeProperty) (Display *display, Window window, Atom property,
                                   Atom type, int format, int mode,
                                   const unsigned char *data, int nelements);
typedef int (*tp_XChangeWindowAttributes) (Display *display, Window window,
                                           unsigned long valuemask,
                                           XSetWindowAttributes *attributes);
typedef int (*tp_XCloseDisplay) (Display *display);
typedef int (*tp_XConvertSelection) (Display *display, Atom selection, Atom target,
                                     Atom property, Window requestor, Time time);
typedef int (*tp_XCopyArea) (Display *display, Drawable src, Drawable dest, GC gc,
                             int src_x, int src_y, unsigned int width, unsigned height,
                             int dest_x, int dest_y);
typedef int (*tp_XCopyPlane) (Display *display, Drawable src, Drawable dest,
                              GC gc, int src_x, int src_y, unsigned int width,
                              unsigned int height, int dest_x, int dest_y,
                              unsigned long plane);
typedef Pixmap (*tp_XCreateBitmapFromData) (Display *display, Drawable drawable,
                                            const char *data, unsigned int width,
                                            unsigned int height);
typedef GC (*tp_XCreateGC) (Display *display, Drawable drawable, unsigned long valuemask,
                            XGCValues *values);
typedef XImage *(*tp_XCreateImage) (Display *display, Visual *visual,
                                    unsigned int depth, int format, int offset, char *data,
                                    unsigned int width, unsigned int height,
                                    int bitmap_pad, int bytes_per_line);
typedef Pixmap (*tp_XCreatePixmap) (Display *display, Drawable drawable, unsigned int width,
                                    unsigned int height, unsigned int depth);
typedef Cursor (*tp_XCreatePixmapCursor) (Display *display, Pixmap source, Pixmap mask,
                                          XColor *foreground_color, XColor *background_color,
                                          unsigned int x, unsigned int y);
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
typedef int (*tp_XDefineCursor) (Display *display, Window window, Cursor cursor);
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
typedef int (*tp_XGetKeyboardControl) (Display *display,
                                       XKeyboardState *values_return);
typedef unsigned long (*tp_XGetPixel) (XImage *ximage, int x, int y);
typedef Status (*tp_XGetWindowAttributes) (Display *display, Window window,
                                           XWindowAttributes *window_attributes_return);
typedef int (*tp_XGetWindowProperty) (Display  *display, Window window, Atom property,
                                      long long_offset, long long_length, Bool delete,
                                      Atom req_type, Atom *actual_type_return,
                                      int *actual_format_return,
                                      unsigned long *nitems_return,
                                      unsigned long *bytes_after_return,
                                      unsigned char **prop_return);
typedef Atom (*tp_XInternAtom) (Display *display, const char *atom_name, Bool only_if_exists);
typedef KeyCode (*tp_XKeysymToKeycode) (Display *display, KeySym keysym);
typedef int (*tp_XLookupString) (XKeyEvent *event_struct, char *buffer_return,
                                 int bytes_buffer, KeySym *keysym_return,
                                 XComposeStatus *status_in_out);
typedef int (*tp_XLowerWindow) (Display *display, Window window);
typedef int (*tp_XMapRaised) (Display *display, Window window);
typedef int (*tp_XMapWindow) (Display *display, Window window);
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
typedef Status (*tp_XQueryTree) (Display *display, Window window,
                                 Window *root_return, Window *parent_return,
                                 Window **children_return,
                                 unsigned int *nchildren_return);
typedef int (*tp_XRaiseWindow) (Display *display, Window window);
typedef int (*tp_XRefreshKeyboardMapping) (XMappingEvent *event_map);
typedef int (*tp_XResizeWindow) (Display *display, Window window,
                                 unsigned int width, unsigned int height);
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
typedef int (*tp_XSetLineAttributes) (Display *display, GC gc,
                                      unsigned int line_width,
                                      int line_style,
                                      int cap_style,
                                      int join_style);
typedef int (*tp_XSetStandardProperties) (Display *display, Window window,
                                          const char *window_name,
                                          const char *icon_name,
                                          Pixmap icon_pixmap, char **argv,
                                          int argc, XSizeHints *hints);
typedef int (*tp_XSetWMHints) (Display *display, Window window, XWMHints *wm_hints);
typedef Status (*tp_XSetWMProtocols) (Display *display, Window window,
                                      Atom *protocols, int count);
typedef int (*tp_XStoreColor) (Display *display, Colormap colormap, XColor *color);
typedef int (*tp_XStoreName) (Display *display, Window window, const char *window_name);
typedef int (*tp_XSync) (Display *display, Bool discard);
typedef int (*tp_XUndefineCursor) (Display *display, Window window);
typedef int (*tp_XWarpPointer) (Display *display, Window src_w, Window dest_w,
                                int src_x, int src_y, unsigned int src_width,
                                unsigned int src_height, int dest_x, int dest_y);
typedef unsigned long (*tp_XWhitePixel) (Display *display,
                                         int screen_number);

#ifdef HAS_XRENDER_EXTENSION
typedef void (*tp_XRenderComposite) (Display *display, int op, Picture src,
                                     Picture mask, Picture dst, int src_x, int src_y,
                                     int mask_x, int mask_y, int dst_x, int dst_y,
                                     unsigned int width, unsigned int height);
typedef Picture (*tp_XRenderCreatePicture) (Display *display,
                                            Drawable drawable,
                                            const XRenderPictFormat *format,
                                            unsigned long valuemask,
                                            const XRenderPictureAttributes *attributes);
typedef XRenderPictFormat *(*tp_XRenderFindVisualFormat) (Display *display,
                                                          const Visual *visual);
typedef void (*tp_XRenderFreePicture) (Display *display, Picture picture);
typedef void (*tp_XRenderSetPictureTransform) (Display *display,
                                               Picture picture,
                                               XTransform *transform);
#endif

#ifdef FORWARD_X11_FUNCTION_POINTERS
extern tp_XAllocColor             ptr_XAllocColor;
extern tp_XAllocColorCells        ptr_XAllocColorCells;
extern tp_XBlackPixel             ptr_XBlackPixel;
extern tp_XChangeProperty         ptr_XChangeProperty;
extern tp_XChangeWindowAttributes ptr_XChangeWindowAttributes;
extern tp_XCloseDisplay           ptr_XCloseDisplay;
extern tp_XConvertSelection       ptr_XConvertSelection;
extern tp_XCopyArea               ptr_XCopyArea;
extern tp_XCopyPlane              ptr_XCopyPlane;
extern tp_XCreateBitmapFromData   ptr_XCreateBitmapFromData;
extern tp_XCreateGC               ptr_XCreateGC;
extern tp_XCreateImage            ptr_XCreateImage;
extern tp_XCreatePixmap           ptr_XCreatePixmap;
extern tp_XCreatePixmapCursor     ptr_XCreatePixmapCursor;
extern tp_XCreateSimpleWindow     ptr_XCreateSimpleWindow;
extern tp_XDefaultColormap        ptr_XDefaultColormap;
extern tp_XDefaultDepth           ptr_XDefaultDepth;
extern tp_XDefaultRootWindow      ptr_XDefaultRootWindow;
extern tp_XDefaultScreen          ptr_XDefaultScreen;
extern tp_XDefaultVisual          ptr_XDefaultVisual;
extern tp_XDefineCursor           ptr_XDefineCursor;
extern tp_XDestroyImage           ptr_XDestroyImage;
extern tp_XDestroyWindow          ptr_XDestroyWindow;
extern tp_XDoesBackingStore       ptr_XDoesBackingStore;
extern tp_XDrawArc                ptr_XDrawArc;
extern tp_XDrawImageString16      ptr_XDrawImageString16;
extern tp_XDrawLine               ptr_XDrawLine;
extern tp_XDrawLines              ptr_XDrawLines;
extern tp_XDrawPoint              ptr_XDrawPoint;
extern tp_XEventsQueued           ptr_XEventsQueued;
extern tp_XFillArc                ptr_XFillArc;
extern tp_XFillPolygon            ptr_XFillPolygon;
extern tp_XFillRectangle          ptr_XFillRectangle;
extern tp_XFlush                  ptr_XFlush;
extern tp_XFree                   ptr_XFree;
extern tp_XFreeGC                 ptr_XFreeGC;
extern tp_XFreePixmap             ptr_XFreePixmap;
extern tp_XGetGeometry            ptr_XGetGeometry;
extern tp_XGetImage               ptr_XGetImage;
extern tp_XGetKeyboardControl     ptr_XGetKeyboardControl;
extern tp_XGetPixel               ptr_XGetPixel;
extern tp_XGetWindowAttributes    ptr_XGetWindowAttributes;
extern tp_XGetWindowProperty      ptr_XGetWindowProperty;
extern tp_XInternAtom             ptr_XInternAtom;
extern tp_XKeysymToKeycode        ptr_XKeysymToKeycode;
extern tp_XLookupString           ptr_XLookupString;
extern tp_XLowerWindow            ptr_XLowerWindow;
extern tp_XMapRaised              ptr_XMapRaised;
extern tp_XMapWindow              ptr_XMapWindow;
extern tp_XMoveWindow             ptr_XMoveWindow;
extern tp_XNextEvent              ptr_XNextEvent;
extern tp_XOpenDisplay            ptr_XOpenDisplay;
extern tp_XPutImage               ptr_XPutImage;
extern tp_XQueryColor             ptr_XQueryColor;
extern tp_XQueryKeymap            ptr_XQueryKeymap;
extern tp_XQueryPointer           ptr_XQueryPointer;
extern tp_XQueryTree              ptr_XQueryTree;
extern tp_XRaiseWindow            ptr_XRaiseWindow;
extern tp_XRefreshKeyboardMapping ptr_XRefreshKeyboardMapping;
extern tp_XResizeWindow           ptr_XResizeWindow;
extern tp_XScreenOfDisplay        ptr_XScreenOfDisplay;
extern tp_XSelectInput            ptr_XSelectInput;
extern tp_XSetArcMode             ptr_XSetArcMode;
extern tp_XSetBackground          ptr_XSetBackground;
extern tp_XSetClipMask            ptr_XSetClipMask;
extern tp_XSetClipOrigin          ptr_XSetClipOrigin;
extern tp_XSetForeground          ptr_XSetForeground;
extern tp_XSetFunction            ptr_XSetFunction;
extern tp_XSetLineAttributes      ptr_XSetLineAttributes;
extern tp_XSetStandardProperties  ptr_XSetStandardProperties;
extern tp_XSetWMHints             ptr_XSetWMHints;
extern tp_XSetWMProtocols         ptr_XSetWMProtocols;
extern tp_XStoreColor             ptr_XStoreColor;
extern tp_XStoreName              ptr_XStoreName;
extern tp_XSync                   ptr_XSync;
extern tp_XUndefineCursor         ptr_XUndefineCursor;
extern tp_XWarpPointer            ptr_XWarpPointer;
extern tp_XWhitePixel             ptr_XWhitePixel;

#ifdef HAS_XRENDER_EXTENSION
extern tp_XRenderComposite           ptr_XRenderComposite;
extern tp_XRenderCreatePicture       ptr_XRenderCreatePicture;
extern tp_XRenderFindVisualFormat    ptr_XRenderFindVisualFormat;
extern tp_XRenderFreePicture         ptr_XRenderFreePicture;
extern tp_XRenderSetPictureTransform ptr_XRenderSetPictureTransform;
#endif

#define XAllocColor             ptr_XAllocColor
#define XAllocColorCells        ptr_XAllocColorCells
#define XBlackPixel             ptr_XBlackPixel
#define XChangeProperty         ptr_XChangeProperty
#define XChangeWindowAttributes ptr_XChangeWindowAttributes
#define XCloseDisplay           ptr_XCloseDisplay
#define XConvertSelection       ptr_XConvertSelection
#define XCopyArea               ptr_XCopyArea
#define XCopyPlane              ptr_XCopyPlane
#define XCreateBitmapFromData   ptr_XCreateBitmapFromData
#define XCreateGC               ptr_XCreateGC
#define XCreateImage            ptr_XCreateImage
#define XCreatePixmap           ptr_XCreatePixmap
#define XCreatePixmapCursor     ptr_XCreatePixmapCursor
#define XCreateSimpleWindow     ptr_XCreateSimpleWindow
#define XDefaultColormap        ptr_XDefaultColormap
#define XDefaultDepth           ptr_XDefaultDepth
#define XDefaultRootWindow      ptr_XDefaultRootWindow
#define XDefaultScreen          ptr_XDefaultScreen
#define XDefaultVisual          ptr_XDefaultVisual
#define XDefineCursor           ptr_XDefineCursor
#define XDestroyImage           ptr_XDestroyImage
#define XDestroyWindow          ptr_XDestroyWindow
#define XDoesBackingStore       ptr_XDoesBackingStore
#define XDrawArc                ptr_XDrawArc
#define XDrawImageString16      ptr_XDrawImageString16
#define XDrawLine               ptr_XDrawLine
#define XDrawLines              ptr_XDrawLines
#define XDrawPoint              ptr_XDrawPoint
#define XEventsQueued           ptr_XEventsQueued
#define XFillArc                ptr_XFillArc
#define XFillPolygon            ptr_XFillPolygon
#define XFillRectangle          ptr_XFillRectangle
#define XFlush                  ptr_XFlush
#define XFree                   ptr_XFree
#define XFreeGC                 ptr_XFreeGC
#define XFreePixmap             ptr_XFreePixmap
#define XGetGeometry            ptr_XGetGeometry
#define XGetImage               ptr_XGetImage
#define XGetKeyboardControl     ptr_XGetKeyboardControl
#define XGetPixel               ptr_XGetPixel
#define XGetWindowAttributes    ptr_XGetWindowAttributes
#define XGetWindowProperty      ptr_XGetWindowProperty
#define XInternAtom             ptr_XInternAtom
#define XKeysymToKeycode        ptr_XKeysymToKeycode
#define XLookupString           ptr_XLookupString
#define XLowerWindow            ptr_XLowerWindow
#define XMapRaised              ptr_XMapRaised
#define XMapWindow              ptr_XMapWindow
#define XMoveWindow             ptr_XMoveWindow
#define XNextEvent              ptr_XNextEvent
#define XOpenDisplay            ptr_XOpenDisplay
#define XPutImage               ptr_XPutImage
#define XQueryColor             ptr_XQueryColor
#define XQueryKeymap            ptr_XQueryKeymap
#define XQueryPointer           ptr_XQueryPointer
#define XQueryTree              ptr_XQueryTree
#define XRaiseWindow            ptr_XRaiseWindow
#define XRefreshKeyboardMapping ptr_XRefreshKeyboardMapping
#define XResizeWindow           ptr_XResizeWindow
#define XScreenOfDisplay        ptr_XScreenOfDisplay
#define XSelectInput            ptr_XSelectInput
#define XSetArcMode             ptr_XSetArcMode
#define XSetBackground          ptr_XSetBackground
#define XSetClipMask            ptr_XSetClipMask
#define XSetClipOrigin          ptr_XSetClipOrigin
#define XSetForeground          ptr_XSetForeground
#define XSetFunction            ptr_XSetFunction
#define XSetLineAttributes      ptr_XSetLineAttributes
#define XSetStandardProperties  ptr_XSetStandardProperties
#define XSetWMHints             ptr_XSetWMHints
#define XSetWMProtocols         ptr_XSetWMProtocols
#define XStoreColor             ptr_XStoreColor
#define XStoreName              ptr_XStoreName
#define XSync                   ptr_XSync
#define XUndefineCursor         ptr_XUndefineCursor
#define XWarpPointer            ptr_XWarpPointer
#define XWhitePixel             ptr_XWhitePixel

#ifdef HAS_XRENDER_EXTENSION
#define XRenderComposite           ptr_XRenderComposite
#define XRenderCreatePicture       ptr_XRenderCreatePicture
#define XRenderFindVisualFormat    ptr_XRenderFindVisualFormat
#define XRenderFreePicture         ptr_XRenderFreePicture
#define XRenderSetPictureTransform ptr_XRenderSetPictureTransform
#endif
#endif

#endif
