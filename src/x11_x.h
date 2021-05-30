/********************************************************************/
/*                                                                  */
/*  x11_x.h       X11 interfaces used by Seed7.                     */
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
/*  File: seed7/src/x11_x.h                                         */
/*  Changes: 2019  Thomas Mertes                                    */
/*  Content: X11 interfaces used by Seed7.                          */
/*                                                                  */
/********************************************************************/

#define Bool int
#define True   1
#define False  0

#define None  0L

#define ArcChord     0
#define ArcPieSlice  1

#define CoordModeOrigin    0
#define CoordModePrevious  1

#define Complex    0
#define Nonconvex  1
#define Convex     2

#define NotUseful   0
#define WhenMapped  1
#define Always      2

#define PropModeReplace  0

#define XYBitmap  0
#define XYPixmap  1
#define ZPixmap   2

#define ForgetGravity      0
#define NorthWestGravity   1
#define NorthGravity       2
#define NorthEastGravity   3
#define WestGravity        4
#define CenterGravity      5
#define EastGravity        6
#define SouthWestGravity   7
#define SouthGravity       8
#define SouthEastGravity   9
#define StaticGravity     10

#define StaticGray   0
#define GrayScale    1
#define StaticColor  2
#define PseudoColor  3
#define TrueColor    4
#define DirectColor  5

#define GXclear         0x0
#define GXand           0x1
#define GXandReverse    0x2
#define GXcopy          0x3
#define GXandInverted   0x4
#define GXnoop          0x5
#define GXxor           0x6
#define GXor            0x7
#define GXnor           0x8
#define GXequiv         0x9
#define GXinvert        0xa
#define GXorReverse     0xb
#define GXcopyInverted  0xc
#define GXorInverted    0xd
#define GXnand          0xe
#define GXset           0xf

#define CWBackPixmap           1L
#define CWBackPixel            2L
#define CWBorderPixmap         4L
#define CWBorderPixel          8L
#define CWBitGravity          16L
#define CWWinGravity          32L
#define CWBackingStore        64L
#define CWBackingPlanes      128L
#define CWBackingPixel       256L
#define CWOverrideRedirect   512L
#define CWSaveUnder         1024L
#define CWEventMask         2048L
#define CWDontPropagate     4096L
#define CWColormap          8192L
#define CWCursor           16384L

#define USPosition     1L
#define USSize         2L
#define PPosition      4L
#define PSize          8L
#define PMinSize      16L
#define PMaxSize      32L
#define PResizeInc    64L
#define PAspect      128L
#define PBaseSize    256L
#define PWinGravity  512L

#define InputHint          1L
#define StateHint          2L
#define IconPixmapHint     4L
#define IconWindowHint     8L
#define IconPositionHint  16L
#define IconMaskHint      32L
#define WindowGroupHint   64L
#define AllHints (InputHint|StateHint|IconPixmapHint|IconWindowHint|IconPositionHint|IconMaskHint|WindowGroupHint)

#define QueuedAlready       0
#define QueuedAfterReading  1
#define QueuedAfterFlush    2

#define KeyPressMask              (1L<<0)
#define KeyReleaseMask            (1L<<1)
#define ButtonPressMask           (1L<<2)
#define ButtonReleaseMask         (1L<<3)
#define EnterWindowMask           (1L<<4)
#define LeaveWindowMask           (1L<<5)
#define PointerMotionMask         (1L<<6)
#define PointerMotionHintMask     (1L<<7)
#define Button1MotionMask         (1L<<8)
#define Button2MotionMask         (1L<<9)
#define Button3MotionMask         (1L<<10)
#define Button4MotionMask         (1L<<11)
#define Button5MotionMask         (1L<<12)
#define ButtonMotionMask          (1L<<13)
#define KeymapStateMask           (1L<<14)
#define ExposureMask              (1L<<15)
#define VisibilityChangeMask      (1L<<16)
#define StructureNotifyMask       (1L<<17)
#define ResizeRedirectMask        (1L<<18)
#define SubstructureNotifyMask    (1L<<19)
#define SubstructureRedirectMask  (1L<<20)
#define FocusChangeMask           (1L<<21)
#define PropertyChangeMask        (1L<<22)
#define ColormapChangeMask        (1L<<23)

#define KeyPress           2
#define KeyRelease         3
#define ButtonPress        4
#define ButtonRelease      5
#define MotionNotify       6
#define EnterNotify        7
#define LeaveNotify        8
#define FocusIn            9
#define FocusOut          10
#define KeymapNotify      11
#define Expose            12
#define GraphicsExpose    13
#define NoExpose          14
#define VisibilityNotify  15
#define CreateNotify      16
#define DestroyNotify     17
#define UnmapNotify       18
#define MapNotify         19
#define MapRequest        20
#define ReparentNotify    21
#define ConfigureNotify   22
#define ConfigureRequest  23
#define GravityNotify     24
#define ResizeRequest     25
#define CirculateNotify   26
#define CirculateRequest  27
#define PropertyNotify    28
#define SelectionClear    29
#define SelectionRequest  30
#define SelectionNotify   31
#define ColormapNotify    32
#define ClientMessage     33
#define MappingNotify     34
#define GenericEvent      35

#define ShiftMask     (1<<0)
#define LockMask      (1<<1)
#define ControlMask   (1<<2)
#define Mod1Mask      (1<<3)
#define Mod2Mask      (1<<4)
#define Mod3Mask      (1<<5)
#define Mod4Mask      (1<<6)
#define Mod5Mask      (1<<7)
#define Button1Mask   (1<<8)
#define Button2Mask   (1<<9)
#define Button3Mask  (1<<10)
#define Button4Mask  (1<<11)
#define Button5Mask  (1<<12)

#define XK_ISO_Level3_Shift  0xfe03
#define XK_ISO_Left_Tab      0xfe20

#define XK_dead_acute        0xfe51
#define XK_dead_circumflex   0xfe52
#define XK_dead_tilde        0xfe53
#define XK_dead_macron       0xfe54
#define XK_dead_breve        0xfe55
#define XK_dead_abovedot     0xfe56
#define XK_dead_diaeresis    0xfe57
#define XK_dead_abovering    0xfe58
#define XK_dead_doubleacute  0xfe59
#define XK_dead_caron        0xfe5a
#define XK_dead_cedilla      0xfe5b

#define XK_BackSpace         0xff08
#define XK_Tab               0xff09
#define XK_Linefeed          0xff0a
#define XK_Clear             0xff0b
#define XK_Return            0xff0d
#define XK_Pause             0xff13
#define XK_Scroll_Lock       0xff14
#define XK_Sys_Req           0xff15
#define XK_Escape            0xff1b
#define XK_Delete            0xffff

#define XK_Home              0xff50
#define XK_Left              0xff51
#define XK_Up                0xff52
#define XK_Right             0xff53
#define XK_Down              0xff54
#define XK_Prior             0xff55
#define XK_Page_Up           0xff55
#define XK_Next              0xff56
#define XK_Page_Down         0xff56
#define XK_End               0xff57
#define XK_Begin             0xff58

#define XK_Select            0xff60
#define XK_Print             0xff61
#define XK_Execute           0xff62
#define XK_Insert            0xff63
#define XK_Undo              0xff65
#define XK_Redo              0xff66
#define XK_Menu              0xff67
#define XK_Find              0xff68
#define XK_Cancel            0xff69
#define XK_Help              0xff6a
#define XK_Break             0xff6b
#define XK_Mode_switch       0xff7e
#define XK_script_switch     0xff7e
#define XK_Num_Lock          0xff7f

#define XK_KP_Space          0xff80
#define XK_KP_Tab            0xff89
#define XK_KP_Enter          0xff8d
#define XK_KP_F1             0xff91
#define XK_KP_F2             0xff92
#define XK_KP_F3             0xff93
#define XK_KP_F4             0xff94
#define XK_KP_Home           0xff95
#define XK_KP_Left           0xff96
#define XK_KP_Up             0xff97
#define XK_KP_Right          0xff98
#define XK_KP_Down           0xff99
#define XK_KP_Prior          0xff9a
#define XK_KP_Page_Up        0xff9a
#define XK_KP_Next           0xff9b
#define XK_KP_Page_Down      0xff9b
#define XK_KP_End            0xff9c
#define XK_KP_Begin          0xff9d
#define XK_KP_Insert         0xff9e
#define XK_KP_Delete         0xff9f
#define XK_KP_Equal          0xffbd
#define XK_KP_Multiply       0xffaa
#define XK_KP_Add            0xffab
#define XK_KP_Separator      0xffac
#define XK_KP_Subtract       0xffad
#define XK_KP_Decimal        0xffae
#define XK_KP_Divide         0xffaf

#define XK_KP_0              0xffb0
#define XK_KP_1              0xffb1
#define XK_KP_2              0xffb2
#define XK_KP_3              0xffb3
#define XK_KP_4              0xffb4
#define XK_KP_5              0xffb5
#define XK_KP_6              0xffb6
#define XK_KP_7              0xffb7
#define XK_KP_8              0xffb8
#define XK_KP_9              0xffb9

#define XK_F1                0xffbe
#define XK_F2                0xffbf
#define XK_F3                0xffc0
#define XK_F4                0xffc1
#define XK_F5                0xffc2
#define XK_F6                0xffc3
#define XK_F7                0xffc4
#define XK_F8                0xffc5
#define XK_F9                0xffc6
#define XK_F10               0xffc7
#define XK_F11               0xffc8
#define XK_F12               0xffc9

#define XK_Shift_L           0xffe1
#define XK_Shift_R           0xffe2
#define XK_Control_L         0xffe3
#define XK_Control_R         0xffe4
#define XK_Caps_Lock         0xffe5
#define XK_Shift_Lock        0xffe6
#define XK_Meta_L            0xffe7
#define XK_Meta_R            0xffe8
#define XK_Alt_L             0xffe9
#define XK_Alt_R             0xffea

#define XK_0                 0x0030
#define XK_1                 0x0031
#define XK_2                 0x0032
#define XK_3                 0x0033
#define XK_4                 0x0034
#define XK_5                 0x0035
#define XK_6                 0x0036
#define XK_7                 0x0037
#define XK_8                 0x0038
#define XK_9                 0x0039

typedef struct { int dummy; } Display;
typedef struct { int dummy; } Screen;
typedef struct { int dummy; } XExtData;
typedef struct { int dummy; } XGCValues;
typedef struct { int dummy; } XImage;
typedef struct { int dummy; } XComposeStatus;
typedef struct { int dummy; } XMappingEvent;

typedef struct { int dummy; } *GC;

typedef int Status;

typedef unsigned char KeyCode;
typedef unsigned long Time;

typedef unsigned long Atom;
typedef unsigned long VisualID;

typedef unsigned long XID;

typedef XID Window;
typedef XID Pixmap;
typedef XID Drawable;
typedef XID Cursor;
typedef XID Colormap;
typedef XID KeySym;

typedef struct {
    short x, y;
  } XPoint;

typedef struct {
    unsigned char byte1;
    unsigned char byte2;
  } XChar2b;

typedef struct {
    unsigned long pixel;
    unsigned short red, green, blue;
    char flags;
    char pad;
  } XColor;

typedef struct {
    XExtData *ext_data;
    VisualID visualid;
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;
#else
    int class;
#endif
    unsigned long red_mask, green_mask, blue_mask;
    int bits_per_rgb;
    int map_entries;
  } Visual;

typedef struct {
    long flags;
    int x, y;
    int width, height;
    int min_width, min_height;
    int max_width, max_height;
    int width_inc, height_inc;
    struct {
        int x;
        int y;
    } min_aspect, max_aspect;
    int base_width, base_height;
    int win_gravity;
  } XSizeHints;

typedef struct {
    long flags;
    Bool input;
    int initial_state;
    Pixmap icon_pixmap;
    Window icon_window;
    int icon_x, icon_y;
    Pixmap icon_mask;
    XID window_group;
  } XWMHints;

typedef struct {
    Pixmap background_pixmap;
    unsigned long background_pixel;
    Pixmap border_pixmap;
    unsigned long border_pixel;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    Bool save_under;
    long event_mask;
    long do_not_propagate_mask;
    Bool override_redirect;
    Colormap colormap;
    Cursor cursor;
  } XSetWindowAttributes;

typedef struct {
    int x, y;
    int width, height;
    int border_width;
    int depth;
    Visual *visual;
    Window root;
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;
#else
    int class;
#endif
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    Bool save_under;
    Colormap colormap;
    Bool map_installed;
    int map_state;
    long all_event_masks;
    long your_event_mask;
    long do_not_propagate_mask;
    Bool override_redirect;
    Screen *screen;
  } XWindowAttributes;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    Time time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int keycode;
    Bool same_screen;
  } XKeyEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    Time time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int button;
    Bool same_screen;
  } XButtonEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    int x, y;
    int width, height;
    int count;
  } XExposeEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window event;
    Window window;
    int x, y;
    int width, height;
    int border_width;
    Window above;
    Bool override_redirect;
  } XConfigureEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Atom message_type;
    int format;
    union {
      char b[20];
      short s[10];
      long l[5];
    } data;
  } XClientMessageEvent;

typedef union _XEvent {
    int type;
    XKeyEvent xkey;
    XButtonEvent xbutton;
    XExposeEvent xexpose;
    XClientMessageEvent xclient;
    XConfigureEvent xconfigure;
    XMappingEvent xmapping;
    long pad[24];
  } XEvent;

typedef struct {
    int key_click_percent;
    int bell_percent;
    unsigned int bell_pitch, bell_duration;
    unsigned long led_mask;
    int global_auto_repeat;
    char auto_repeats[32];
  } XKeyboardState;

#define XA_ATOM ((Atom) 4)


#define BlackPixel(dpy, scr)      XBlackPixel(dpy, scr)
#define DefaultColormap(dpy, scr) XDefaultColormap(dpy, scr)
#define DefaultDepth(dpy, scr)    XDefaultDepth(dpy, scr)
#define DefaultRootWindow(dpy)    XDefaultRootWindow(dpy)
#define DefaultScreen(dpy)        XDefaultScreen(dpy)
#define DoesBackingStore(scr)     XDoesBackingStore(scr)
#define ScreenOfDisplay(dpy, scr) XScreenOfDisplay(dpy, scr)
#define WhitePixel(dpy, scr)      XWhitePixel(dpy, scr)

/* Xrender extension */

#define PictOpOver  3

#define CPSubwindowMode    (1 << 8)

#define XDoubleToFixed(f)    ((XFixed) ((f) * 65536))

typedef struct { int dumme; } XRenderPictFormat;

typedef XID Picture;

typedef int XFixed;

typedef struct {
    XFixed matrix[3][3];
  } XTransform;

typedef struct {
    int     repeat;
    Picture alpha_map;
    int     alpha_x_origin;
    int     alpha_y_origin;
    int     clip_x_origin;
    int     clip_y_origin;
    Pixmap  clip_mask;
    Bool    graphics_exposures;
    int     subwindow_mode;
    int     poly_edge;
    int     poly_mode;
    Atom    dither;
    Bool    component_alpha;
} XRenderPictureAttributes;


extern Status XAllocColor (Display *display,
                           Colormap colormap,
                           XColor *screen_in_out);
extern Status XAllocColorCells (Display *display,
                                Colormap colormap,
                                Bool contig,
                                unsigned long *plane_masks_return,
                                unsigned int nplanes,
                                unsigned long *pixels_return,
                                unsigned int npixels);
extern unsigned long XBlackPixel (Display *display,
                                  int screen_number);
extern int XChangeProperty (Display *display,
                            Window window,
                            Atom property,
                            Atom type,
                            int format,
                            int mode,
                            const unsigned char *data,
                            int nelements);
extern int XChangeWindowAttributes (Display *display,
                                    Window window,
                                    unsigned long valuemask,
                                    XSetWindowAttributes *attributes);
extern int XCopyArea (Display *display,
                      Drawable src,
                      Drawable dest,
                      GC gc,
                      int src_x,
                      int src_y,
                      unsigned int width,
                      unsigned int height,
                      int dest_x,
                      int dest_y);
extern int XCopyPlane (Display *display,
                       Drawable src,
                       Drawable dest,
                       GC gc,
                       int src_x,
                       int src_y,
                       unsigned int width,
                       unsigned int height,
                       int dest_x,
                       int dest_y,
                       unsigned long plane);
extern GC XCreateGC (Display *display,
                     Drawable drawable,
                     unsigned long valuemask,
                     XGCValues *values);
extern XImage *XCreateImage (Display *display,
                             Visual *visual,
                             unsigned int depth,
                             int format,
                             int offset,
                             char *data,
                             unsigned int width,
                             unsigned int height,
                             int bitmap_pad,
                             int bytes_per_line);
extern Pixmap XCreatePixmap (Display *display,
                             Drawable drawable,
                             unsigned int width,
                             unsigned int height,
                             unsigned int depth);
extern Window XCreateSimpleWindow (Display *display,
                                   Window parent,
                                   int x,
                                   int y,
                                   unsigned int width,
                                   unsigned int height,
                                   unsigned int border_width,
                                   unsigned long border,
                                   unsigned long background);
extern Colormap XDefaultColormap (Display *display,
                                  int screen_number);
extern int XDefaultDepth (Display *display,
                          int screen_number);
extern Window XDefaultRootWindow (Display *display);
extern int XDefaultScreen (Display *display);
extern Visual *XDefaultVisual (Display *display,
                               int screen_number);
extern int XDestroyImage (XImage *ximage);
extern int XDestroyWindow (Display *display,
                           Window window);
extern int XDoesBackingStore (Screen *screen);
extern int XDrawArc (Display *display,
                     Drawable drawable,
                     GC gc,
                     int x,
                     int y,
                     unsigned int width,
                     unsigned int height,
                     int angle1,
                     int angle2);
extern int XDrawImageString16 (Display *display,
                               Drawable drawable,
                               GC gc,
                               int x,
                               int y,
                               const XChar2b *string,
                               int length);
extern int XDrawLine (Display *display,
                      Drawable drawable,
                      GC gc,
                      int x1,
                      int y1,
                      int x2,
                      int y2);
extern int XDrawLines (Display *display,
                       Drawable drawable,
                       GC gc,
                       XPoint *points,
                       int npoints,
                       int mode);
extern int XDrawPoint (Display *display,
                       Drawable drawable,
                       GC gc,
                       int x,
                       int y);
extern int XEventsQueued (Display *display, int mode);
extern int XFillArc (Display *display,
                     Drawable drawable,
                     GC gc,
                     int x,
                     int y,
                     unsigned int width,
                     unsigned int height,
                     int angle1,
                     int angle2);
extern int XFillPolygon (Display *display,
                         Drawable drawable,
                         GC gc,
                         XPoint *points,
                         int npoints,
                         int shape,
                         int mode);
extern int XFillRectangle (Display *display,
                           Drawable drawable,
                           GC gc,
                           int x,
                           int y,
                           unsigned int width,
                           unsigned int height);
extern int XFlush (Display *display);
extern int XFree (void *data);
extern int XFreeGC (Display *display,
                    GC gc);
extern int XFreePixmap (Display *display,
                        Pixmap pixmap);
extern Status XGetGeometry (Display *display,
                            Drawable drawable,
                            Window *root_return,
                            int *x_return,
                            int *y_return,
                            unsigned int *width_return,
                            unsigned int *height_return,
                            unsigned int *border_width_return,
                            unsigned int *depth_return);
extern XImage *XGetImage (Display *display,
                          Drawable drawable,
                          int x,
                          int y,
                          unsigned int width,
                          unsigned int height,
                          unsigned long plane_mask,
                          int format);
extern int XGetKeyboardControl (Display *display,
                                XKeyboardState *values_return);
extern unsigned long XGetPixel (XImage *ximage,
                                int x,
                                int y);
extern Status XGetWindowAttributes (Display *display,
                                    Window window,
                                    XWindowAttributes *window_attributes_return);
extern Atom XInternAtom (Display *display,
                         const char *atom_name,
                         Bool only_if_exists);
extern KeyCode XKeysymToKeycode (Display *display, KeySym keysym);
extern int XLookupString (XKeyEvent *event_struct, char *buffer_return,
                          int bytes_buffer, KeySym *keysym_return,
                          XComposeStatus *status_in_out);
extern int XLowerWindow (Display *display,
                         Window window);
extern int XMapRaised (Display *display,
                       Window window);
extern int XMapWindow (Display *display,
                       Window window);
extern int XMoveWindow (Display *display,
                        Window window,
                        int x,
                        int y);
extern int XNextEvent(Display *display, XEvent *event_return);
extern Display *XOpenDisplay (const char *display_name);
extern int XPutImage (Display *display,
                      Drawable drawable,
                      GC gc,
                      XImage *image,
                      int src_x,
                      int src_y,
                      int dest_x,
                      int dest_y,
                      unsigned int width,
                      unsigned int height);
extern int XQueryColor (Display *display,
                        Colormap colormap,
                        XColor *def_in_out);
extern int XQueryKeymap (Display *display, char keys_return[32]);
extern Bool XQueryPointer (Display *display,
                           Window window,
                           Window *root_return,
                           Window *child_return,
                           int *root_x_return,
                           int *root_y_return,
                           int *win_x_return,
                           int *win_y_return,
                           unsigned int *mask_return);
extern Status XQueryTree (Display *display,
                          Window window,
                          Window *root_return,
                          Window *parent_return,
                          Window **children_return,
                          unsigned int *nchildren_return);
extern int XRaiseWindow (Display *display,
                         Window window);
extern int XRefreshKeyboardMapping (XMappingEvent *event_map);
extern Screen *XScreenOfDisplay (Display *display,
                                 int screen_number);
extern int XSelectInput (Display *display,
                         Window window,
                         long event_mask);
extern int XSetArcMode (Display *display,
                        GC gc,
                        int arc_mode);
extern int XSetBackground (Display *display,
                           GC gc,
                           unsigned long background);
extern int XSetClipMask (Display *display,
                         GC gc,
                         Pixmap pixmap);
extern int XSetClipOrigin (Display *display,
                           GC gc,
                           int clip_x_origin,
                           int clip_y_origin);
extern int XSetForeground (Display *display,
                           GC gc,
                           unsigned long foreground);
extern int XSetFunction (Display *display,
                         GC gc,
                         int function);
extern int XSetStandardProperties (Display *display,
                                   Window window,
                                   const char *window_name,
                                   const char *icon_name,
                                   Pixmap icon_pixmap,
                                   char **argv,
                                   int argc,
                                   XSizeHints *hints);
extern int XSetWMHints (Display *display,
                        Window window,
                        XWMHints *wm_hints);
extern Status XSetWMProtocols (Display *display,
                               Window window,
                               Atom *protocols,
                               int count);
extern int XStoreColor (Display *display, Colormap colormap, XColor *color);
extern int XStoreName (Display *display, Window window, const char *window_name);
extern int XSync (Display *display,
                  Bool discard);
extern unsigned long XWhitePixel (Display *display,
                                  int screen_number);

/* Xrender extension */

extern void XRenderComposite (Display *display, int op, Picture src,
                              Picture mask, Picture dst, int src_x, int src_y,
                              int mask_x, int mask_y, int dst_x, int dst_y,
                              unsigned int width, unsigned int height);
extern Picture XRenderCreatePicture (Display *display,
                                     Drawable drawable,
                                     const XRenderPictFormat *format,
                                     unsigned long valuemask,
                                     const XRenderPictureAttributes *attributes);
extern XRenderPictFormat *XRenderFindVisualFormat (Display *display,
                                                   const Visual *visual);
extern void XRenderFreePicture (Display *display, Picture picture);
extern void XRenderSetPictureTransform (Display *display,
                                        Picture picture,
                                        XTransform *transform);
