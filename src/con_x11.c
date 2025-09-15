/********************************************************************/
/*                                                                  */
/*  con_x11.c     Driver for X11 text console access.               */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/con_x11.c                                       */
/*  Changes: 1994 - 1999 Thomas Mertes                              */
/*  Content: Driver for X11 text console access.                    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include "common.h"
#include "os_decls.h"
#include "kbd_drv.h"
#include "con_drv.h"


#define SCROLLBLOCKSIZE 50
#define MAX_POINT_COUNT 2000
#define MAX_POLYLINE_COUNT 2000

typedef int positionType; /* -127 .. 127 */

typedef struct {
    boolType makefill;
    int point_count;
    XPoint start_point;
    XPoint *points;
  } polyLineType;

typedef struct {
    positionType xshift;
    positionType xwidth;
    int polyline_count;
    polyLineType *polylines;
  } charDefType;

typedef struct basicFontStruct *basicFontType;
typedef struct scaledFontStruct *scaledFontType;

typedef struct basicFontStruct {
    char *name;
    positionType xMin, xMax, yMin, yMax, xDiff, yDiff, xDist, yDist, size;
    charDefType characters[256];
    scaledFontType scaledfont;
    basicFontType next;
  } basicFontRecord;

typedef struct scaledFontStruct {
    positionType xMin, xMax, yMin, yMax, xDiff, yDiff, xDist, yDist;
    int slant, bslant;
    charDefType characters[256];
    scaledFontType next;
  } scaledFontRecord;


static Display *mydisplay = NULL;
static Window mywindow;
static GC mygc;
static XEvent myevent;
static KeySym mykey;
static int myscreen;
static unsigned long myforeground, mybackground;


static basicFontType font_list = NULL;
static scaledFontType actual_scaledfont;
static boolType normal;
static int maxx, maxy;
static int button_line = 0;
static int button_column = 0;

static boolType screen_initialized = FALSE;
static boolType keybd_initialized = FALSE;



void kbdShut (void)

  { /* kbdShut */
    if (keybd_initialized) {
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



static void kbd_init (void)

  { /* kbd_init */
    keybd_initialized = TRUE;
    os_atexit(kbdShut);
  } /* kbd_init */



boolType kbdInputReady (void)

  {
    boolType result;

  /* kbdInputReady */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (XEventsQueued(mydisplay, QueuedAfterReading) != 0) {
      result = TRUE;
    } else {
      result = FALSE;
    } /* if */
    /* printf("kbdInputReady --> %d\n", result); */
    return result;
  } /* kbdInputReady */



#undef TRACE_KBDGETC
#undef FLAG_EVENTS



charType kbdGetc (void)

  {
    int count;
    char buffer[21];
    charType result;

  /* kbdGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    result = K_NONE;
    XNextEvent(mydisplay, &myevent);
    switch(myevent.type) {
      case Expose:
#ifdef FLAG_EVENTS
        printf("Expose\n");
#endif
        result = K_REDRAW;
        break;

      case NoExpose:
#ifdef FLAG_EVENTS
        printf("NoExpose\n");
#endif
        if (kbdInputReady()) {
          result = kbdGetc();
        } else {
          result = K_NULLCMD;
        } /* if */
        break;

      case MappingNotify:
#ifdef FLAG_EVENTS
        printf("MappingNotify\n");
#endif
        XRefreshKeyboardMapping(&myevent.xmapping);
        break;

      case ButtonPress:
#ifdef FLAG_EVENTS
        printf("ButtonPress (%d, %d)\n",
            myevent.xbutton.x, myevent.xbutton.y);
#endif
        button_line = myevent.xbutton.y;
        button_column = myevent.xbutton.x;
        result = K_MOUSE1;
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
            case XK_Shift_Lock: result = K_NULLCMD;     break;
            default:
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("undef key: %ld %lx\n", (long) mykey, (long) mykey);
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
            case XK_Shift_Lock: result = K_NULLCMD;     break;
            default:
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("undef key: %ld %lx\n", (long) mykey, (long) mykey);
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
            case XK_Shift_Lock: result = K_NULLCMD;     break;
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
                    printf("undef key: %ld %lx\n", (long) mykey, (long) mykey);
                    break;
                } /* switch */
              } else {
                printf("undef key: %ld %lx\n", (long) mykey, (long) mykey);
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
            case XK_Shift_Lock: result = K_NULLCMD;     break;
            default:
              if (count == 1) {
                result = buffer[0];
              } else {
                printf("undef key: %ld %lx\n", (long) mykey, (long) mykey);
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
#ifdef TRACE_KBDGETC
    printf("key: \"%s\" %ld %lx %d\n",
        buffer, (long) mykey, (long) mykey, result);
#endif
    return result;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    return kbdGetc();
  } /* kbdRawGetc */



static void x11_setcolour (intType foreground, intType background)

  { /* x11_setcolour */
#ifdef TRACE_X11_SETCOLOUR
    printf("BEGIN setcolour(%d, %d)\n", foreground, background);
#endif
    if (foreground == black || background == white) {
      normal = FALSE;
    } else {
      normal = TRUE;
    } /* if */
#ifdef TRACE_X11_SETCOLOUR
    printf("END setcolour(%d, %d)\n", foreground, background);
#endif
  } /* x11_setcolour */



static void x11_standardcolour (void)

  { /* x11_standardcolour */
    normal = TRUE;
  } /* x11_standardcolour */



static void x11_normalcolour (void)

  { /* x11_normalcolour */
    normal = TRUE;
  } /* x11_normalcolour */



static positionType scale (int font_value, int font_scale, int screen_scale)

  {
    positionType result;

  /* scale */
#ifdef TRACE_SCALE
    printf("BEGIN scale(%d, %d, %d)\n",
        font_value, font_scale, screen_scale);
#endif
    if (font_value >= 0) {
      result = (font_value * screen_scale + font_scale / 2) / font_scale;
    } else {
      result = (font_value * screen_scale - font_scale / 2) / font_scale;
    } /* if */
#ifdef TRACE_SCALE
    printf("END scale(%d, %d, %d) --> %d\n",
        font_value, font_scale, screen_scale, result);
#endif
    return result;
  } /* scale */



static void writenumber (FILE *scalefile, positionType number)

  {
    int ch;

  /* writenumber */
    if (number >= 0 && number <= 9) {
      ch = number + ((int) '0');
    } else if (number >= 10 && number <= 35) {
      ch = number - 10 + ((int) 'A');
    } else if (number >= 36 && number <= 50) {
      ch = number - 36 + ((int) '!');
    } else if (number >= -26 && number <= -1) {
      ch = ((int) 'a') - number - 1;
    } else {
      ch = ' ';
    } /* if */
    fputc(ch, scalefile);
  } /* writenumber */



static positionType readnumber (FILE *fontfile)

  {
    positionType number;
    int ch;

  /* readnumber */
    ch = fgetc(fontfile);
    if ((ch >= '0') && (ch <= '9')) {
      number = ch - ((int) '0');
    } else if ((ch >= 'a') && (ch <= 'z')) {
      number = ((int) 'a') - ch - 1;
    } else if ((ch >= 'A') && (ch <= 'Z')) {
      number = ch - ((int) 'A') + 10;
    } else if ((ch >= '!') && (ch <= '/')) {
      number = ch - ((int) '!') + 36;
    } else if (ch == '\\') {
      ch = fgetc(fontfile);
      if ((ch >= '0') && (ch <= '9')) {
        number = 0;
        do {
          number = 10 * number + ch - ((int) '0');
          ch = fgetc(fontfile);
        } while ((ch >= '0') && (ch <= '9'));
        ungetc(ch, fontfile);
      } else {
        number = 0;
      } /* if */
    } else {
      number = 0;
    } /* if */
    return number;
  } /* readnumber */



static basicFontType readfont (char *fontfilename)

  {
    FILE *fontfile;
    memSizeType pos = 0;
    char fontname[256];
    int ch;
    int fontch;
    charDefType *act_ch;
    polyLineType *act_polyline;
    XPoint *act_point;
    short act_x;
    short act_y;
    short old_x;
    short old_y;
    basicFontType result;

  /* readfont */
#ifdef TRACE_FONT
    printf("BEGIN readfont(%s)\n", fontfilename);
#endif
    if ((fontfile = fopen(fontfilename, "r")) != NULL) {
      if ((result = (basicFontType) malloc(sizeof(basicFontRecord))) != NULL) {
        while ((ch = getc(fontfile)) != EOF && ch != '\n') {
          if (pos < sizeof(fontname) - 1) {
            fontname[pos] = ch;
            pos++;
          } /* if */
        } /* while */
        fontname[pos] = '\0';
        result->name = (char *) malloc(strlen(fontname) + 1);
        strcpy(result->name, fontname);
#ifdef TRACE_FONT
        printf("%s\n", fontname);
        while ((ch = fgetc(fontfile)) != EOF && ch != '\n') {
          putc(ch, stdout);
        } /* while */
        putc('\n', stdout);
#else
        while ((ch = fgetc(fontfile)) != EOF && ch != '\n');
#endif
        result->xMin =  readnumber(fontfile);
        result->xMax =  readnumber(fontfile);
        result->yMin =  readnumber(fontfile);
        result->yMax =  readnumber(fontfile);
        result->xDiff = readnumber(fontfile);
        result->yDiff = readnumber(fontfile);
        result->xDist = readnumber(fontfile);
        result->yDist = readnumber(fontfile);
        result->size = readnumber(fontfile);
#ifdef TRACE_FONT
        printf("xMin:  %d\n", result->xMin);
        printf("xMax:  %d\n", result->xMax);
        printf("yMin:  %d\n", result->yMin);
        printf("yMax:  %d\n", result->yMax);
        printf("xDiff: %d\n", result->xDiff);
        printf("yDiff: %d\n", result->yDiff);
        printf("xDist: %d\n", result->xDist);
        printf("yDist: %d\n", result->yDist);
        printf("size: %d\n", result->size);
        while ((ch = fgetc(fontfile)) != EOF && ch != '\n') {
          putc(ch, stdout);
        } /* while */
        putc('\n', stdout);
#else
        while ((ch = fgetc(fontfile)) != EOF && ch != '\n');
#endif
        for (fontch = 0; fontch < 256; fontch++) {
          result->characters[fontch].xshift = 0;
          result->characters[fontch].xwidth = -1;
          result->characters[fontch].polyline_count = 0;
          result->characters[fontch].polylines = NULL;
        } /* for */
        while ((fontch = fgetc(fontfile)) != EOF) {
          if (fontch == ' ') {
            fscanf(fontfile, "%d", &fontch);
          } /* if */
          ch = fgetc(fontfile);
          act_ch = &result->characters[fontch];
          act_ch->xwidth = readnumber(fontfile);
          act_ch->polyline_count = 0;
          act_ch->polylines = (polyLineType *)
              malloc(MAX_POLYLINE_COUNT * sizeof(polyLineType));
#ifdef OUT_OF_ORDER
  if (fontch > 128) {
    printf("fontch: %d  WIDTH: %d  ch: %d\n", fontch, act_ch->xwidth, ch);
  }
#endif
          act_ch->xshift = (result->xDiff - act_ch->xwidth - 1) / 2;
          act_polyline = NULL;
          act_point = NULL;
          while ((ch = getc(fontfile)) != EOF && ch != '\n') {
            switch (ch) {
              case '+':
                if (act_point != NULL) {
                  old_x = act_x;
                  old_y = act_y;
                  act_x = readnumber(fontfile);
                  act_y = result->yMax - readnumber(fontfile);
                  act_point->x = act_x - old_x;
                  act_point->y = act_y - old_y;
  /*              printf("+(%d, %d)\n", act_point->x, act_point->y); */
                  act_point++;
                } /* if */
                break;
              case '-':
              case '/':
                if (act_polyline != NULL) {
                  act_polyline->point_count = act_point - act_polyline->points;
                  act_polyline->points = (XPoint *)
                      realloc(act_polyline->points,
                      act_polyline->point_count * sizeof(XPoint));
                  act_polyline++;
                } else {
                  act_polyline = act_ch->polylines;
                } /* if */
                act_polyline->makefill = ch == '/';
                act_polyline->point_count = 0;
                act_polyline->points = (XPoint *)
                    malloc(MAX_POINT_COUNT * sizeof(XPoint));
                act_point = act_polyline->points;
                if (act_point != NULL) {
                  act_x = readnumber(fontfile);
                  act_y = result->yMax - readnumber(fontfile);
                  act_point->x = act_x;
                  act_point->y = act_y;
  /*              printf("-(%d, %d)\n", act_point->x, act_point->y); */
                  act_point++;
                } /* if */
                break;
            } /* switch */
          } /* while */
          if (act_polyline != NULL) {
            act_polyline->point_count = act_point - act_polyline->points;
            act_polyline->points = (XPoint *)
                realloc(act_polyline->points,
                act_polyline->point_count * sizeof(XPoint));
            act_polyline++;
            act_ch->polyline_count = act_polyline - act_ch->polylines;
            act_ch->polylines = (polyLineType *)
                realloc(act_ch->polylines,
                act_ch->polyline_count * sizeof(polyLineType));
          } else {
#ifdef DEBUG
            printf("act_polyline == NULL for ch=%d %d %lu\n", fontch,
                result->characters[fontch].polyline_count,
                (unsigned long) result->characters[fontch].polylines);
#endif
            free(result->characters[fontch].polylines);
            result->characters[fontch].polylines = NULL;
          } /* if */
        } /* while */
        fclose(fontfile);
        for (fontch = 0; fontch < 256; fontch++) {
          act_ch = &result->characters[fontch];
          if (act_ch->xwidth == -1) {
            act_ch->xshift = 0;
            act_ch->xwidth = result->characters[' '].xwidth;
          } /* if */
        } /* for */
        result->scaledfont = NULL;
        result->next = NULL;
      } /* if */
    } else {
      result = NULL;
    } /* if */
#ifdef TRACE_FONT
    printf("END readfont(%s) --> %s\n", fontfilename,
        result != NULL ? result->name : "NULL");
#endif
    return result;
  } /* readfont */



static scaledFontType scalefont (basicFontType font,
    int wanted_scale, int wanted_slant, boolType filled)

  {
    int font_scale;
    int shift_x;
    int shift_y;
    int fontch;
    charDefType *act_ch;
    charDefType *result_ch;
    polyLineType *act_polyline;
    polyLineType *result_polyline;
    XPoint *act_point;
    XPoint *result_point;
    short act_x;
    short act_y;
    short old_x;
    short old_y;
    short absolute_font_x;
    short absolute_font_y;
    scaledFontType result;

  /* scalefont */
#ifdef TRACE_FONT
    printf("BEGIN scalefont(%d)\n", wanted_scale);
#endif
    if ((result = (scaledFontType) malloc(sizeof(scaledFontRecord))) != NULL) {
      font_scale = font->size;
      if (wanted_scale == -1) {
        wanted_scale = font->size;
      } /* if */
      result->xMin =   scale(font->xMin,  font_scale, wanted_scale);
      result->xMax =   scale(font->xMax,  font_scale, wanted_scale);
      result->yMin =   scale(font->yMin,  font_scale, wanted_scale);
      result->yMax =   scale(font->yMax,  font_scale, wanted_scale);
      result->xDiff =  scale(font->xDiff, font_scale, wanted_scale);
      result->yDiff =  scale(font->yDiff, font_scale, wanted_scale);
      result->xDist =  scale(font->xDist, font_scale, wanted_scale);
      result->yDist =  scale(font->yDist, font_scale, wanted_scale);
      result->yDiff = result->yMax - result->yMin + result->yDist;
      result->slant = wanted_slant;
      result->bslant = scale(result->yDiff, result->yMax, wanted_slant);
      shift_x = result->xDist / 2;
      shift_y = result->yDist / 2;
      for (fontch = 0; fontch < 256; fontch++) {
        act_ch = &font->characters[fontch];
        result_ch = &result->characters[fontch];
        result_ch->xwidth = scale(act_ch->xwidth, font_scale, wanted_scale);
        result_ch->xshift = (result->xDiff - result_ch->xwidth - 1) / 2;
        result_ch->polyline_count = act_ch->polyline_count;
        result_ch->polylines = (polyLineType *)
            malloc(result_ch->polyline_count * sizeof(polyLineType));
        act_polyline = act_ch->polylines;
        result_polyline = result_ch->polylines;
        while (act_polyline < &act_ch->polylines[act_ch->polyline_count]) {
          result_polyline->makefill = act_polyline->makefill;
          result_polyline->point_count = act_polyline->point_count;
          if (act_polyline->point_count == 0) {
            result_polyline->start_point.x = 0;
            result_polyline->start_point.y = 0;
            result_polyline->points = NULL;
          } else {
            result_polyline->points = (XPoint *)
                malloc(result_polyline->point_count * sizeof(XPoint));
            act_point = act_polyline->points;
            result_point = result_polyline->points;
            absolute_font_x = act_point->x;
            absolute_font_y = act_point->y;
            act_x = shift_x +
                scale(absolute_font_x,
                font_scale, wanted_scale);
            act_y = shift_y + wanted_scale -
                scale(font_scale - absolute_font_y,
                font_scale, wanted_scale);
            act_x = act_x +
                scale(result->yMax - act_y,
                result->yMax, wanted_slant);
            result_polyline->start_point.x = act_x;
            result_polyline->start_point.y = act_y;
            result_point->x = 0;
            result_point->y = 0;
            act_point++;
            result_point++;
            while (act_point < &act_polyline->points[act_polyline->point_count]) {
              old_x = act_x;
              old_y = act_y;
              absolute_font_x = absolute_font_x + act_point->x;
              absolute_font_y = absolute_font_y + act_point->y;
              act_x = shift_x +
                  scale(absolute_font_x,
                  font_scale, wanted_scale);
              act_y = shift_y + wanted_scale -
                  scale(font_scale - absolute_font_y,
                  font_scale, wanted_scale);
              act_x = act_x +
                  scale(result->yMax - act_y,
                  result->yMax, wanted_slant);
              result_point->x = act_x - old_x;
              result_point->y = act_y - old_y;
              act_point++;
              result_point++;
            } /* while */
          } /* if */
          act_polyline++;
          result_polyline++;
        } /* while */
      } /* for */
      result->next = NULL;
    } /* if */
#ifdef TRACE_FONT
    printf("END scalefont\n");
#endif
    return result;
  } /* scalefont */



static basicFontType get_font (char *fontname)

  {
    char *home_path;
    memSizeType file_name_size;
    char *file_name;
    basicFontType a_font;
    basicFontType result = NULL;

  /* get_font */
#ifdef TRACE_FONT
    printf("BEGIN get_font(%s)\n", fontname);
#endif
    a_font = font_list;
    while (a_font != NULL && result == NULL) {
      if (strcmp(a_font->name, fontname) == 0) {
        result = a_font;
        a_font = NULL;
      } else {
        a_font = a_font->next;
      } /* if */
    } /* while */
    if (result == NULL) {
      if ((result = readfont(fontname)) == NULL) {
        if ((home_path = getenv("HOME")) != NULL) {
          file_name_size = strlen(home_path) + 6 + strlen(fontname) + 4;
          file_name = malloc(file_name_size + 1);
          if (file_name != NULL) {
            strcpy(file_name, home_path);
            if (file_name[0] != '\0' &&
                file_name[strlen(file_name) - 1] != '/') {
              strcat(file_name, "/");
            } /* if */
            strcat(file_name, "font/");
            strcat(file_name, fontname);
            strcat(file_name, ".fnt");
            result = readfont(file_name);
            free(file_name);
          } /* if */
        } /* if */
      } /* if */
      if (result != NULL) {
        result->next = font_list;
        font_list = result;
      } /* if */
    } /* if */
#ifdef TRACE_FONT
    printf("END get_font(%s) --> %s\n", fontname,
        result != NULL ? result->name : "NULL");
#endif
    return result;
  } /* get_font */



static scaledFontType set_font_size (basicFontType basic_font,
    int wanted_scale, int wanted_slant)

  {
    scaledFontType a_font;
    scaledFontType result;

  /* set_font_size */
#ifdef TRACE_FONT
    printf("BEGIN set_font_size(%s, %d)\n", basic_font->name, wanted_scale);
#endif
    result = NULL;
    a_font = basic_font->scaledfont;
    while (a_font != NULL && result == NULL) {
      if (a_font->yMax == wanted_scale &&
          a_font->slant == wanted_slant) {
        result = a_font;
        a_font = NULL;
      } else {
        a_font = a_font->next;
      } /* if */
    } /* while */
    if (result == NULL) {
      result = scalefont(basic_font, wanted_scale,
          wanted_slant, TRUE);
      if (result != NULL) {
        result->next = basic_font->scaledfont;
        basic_font->scaledfont = result;
      } /* if */
    } /* if */
#ifdef TRACE_FONT
    printf("END set_font_size(%s, %d)\n", basic_font->name, wanted_scale);
#endif
    return result;
  } /* set_font_size */



static int x11_setfont (char *fontname)

  {
    basicFontType basic_font;
    char filename[256];
    int scale;
    int slant;
    int result = 0;

  /* x11_setfont */
#ifdef TRACE_FONT
    printf("BEGIN x11_setfont(%s)\n", fontname);
#endif
    scale = -1;
    slant = 0;
    sscanf(fontname, "%s %d %d", filename, &scale, &slant);
    if ((basic_font = get_font(filename)) == NULL) {
      basic_font = get_font("simple");
    } /* if */
    if (basic_font != NULL) {
      actual_scaledfont =
          set_font_size(basic_font, scale, slant);
      result = 1;
    } /* if */
#ifdef TRACE_FONT
    printf("END x11_setfont(%s)\n", fontname);
#endif
    return result;
  } /* x11_setfont */



#ifdef OUT_OF_ORDER
static void x11_setfont (char *fontname)

  {
    char filename[256];
    int letterheight;
    int letterwidth;
    int letterdistance;
    int lineheigth;
    int font_scale;
    int screen_scale;
    int shift_x;
    int shift_y;
    FILE *fontfile;
    FILE *scalefile;
    int ch;
    int fontch;
    charDefType *act_ch;
    polyLineType *act_polyline;
    XPoint *act_point;
    short act_x;
    short act_y;
    short old_x;
    short old_y;

  /* x11_setfont */
printf("BEGIN x11_setfont(%s)\n", fontname);
    letterheight = -1;
    letterwidth = -1;
    letterdistance = -1;
    lineheigth = -1;
    sscanf(fontname, "%s %d %d %d %d", filename,
        &letterheight, &letterwidth, &letterdistance, &lineheigth);
    scalefile = fopen("scalefile", "w");
    if ((fontfile = fopen(filename, "r")) != NULL) {
      while ((ch = fgetc(fontfile)) != EOF && ch != '\n') {
        putc(ch, stdout);
      } /* while */
      putc('\n', stdout);
      while ((ch = fgetc(fontfile)) != EOF && ch != '\n') {
        putc(ch, stdout);
      } /* while */
      putc('\n', stdout);
#ifdef OUT_OF_ORDER
      fscanf(fontfile, "\n");
      fscanf(fontfile, "\n");
#endif
      FONT.xMin =  readnumber(fontfile);
      FONT.xMax =  readnumber(fontfile);
      FONT.yMin =  readnumber(fontfile);
      FONT.yMax =  readnumber(fontfile);
      FONT.xDiff = readnumber(fontfile);
      FONT.yDiff = readnumber(fontfile);
      FONT.xDist = readnumber(fontfile);
      FONT.yDist = readnumber(fontfile);
printf("xMin:  %d\n", FONT.xMin);
printf("xMax:  %d\n", FONT.xMax);
printf("yMin:  %d\n", FONT.yMin);
printf("yMax:  %d\n", FONT.yMax);
printf("xDiff: %d\n", FONT.xDiff);
printf("yDiff: %d\n", FONT.yDiff);
printf("xDist: %d\n", FONT.xDist);
printf("yDist: %d\n", FONT.yDist);
      font_scale = FONT.yMax;
      if (letterheight == -1) {
        screen_scale = FONT.yMax;
        printf("not scaled %d/%d\n", font_scale, screen_scale);
      } else {
        screen_scale = letterheight;
        printf("scaled %d/%d\n", font_scale, screen_scale);
      } /* if */
      FONT.xMin =   scale(FONT.xMin,  font_scale, screen_scale);
      FONT.xMax =   scale(FONT.xMax,  font_scale, screen_scale);
      FONT.yMin =   scale(FONT.yMin,  font_scale, screen_scale);
      FONT.yMax =   scale(FONT.yMax,  font_scale, screen_scale);
      FONT.xDiff =  scale(FONT.xDiff, font_scale, screen_scale);
      FONT.yDiff =  scale(FONT.yDiff, font_scale, screen_scale);
      FONT.xDist =  scale(FONT.xDist, font_scale, screen_scale);
      FONT.yDist =  scale(FONT.yDist, font_scale, screen_scale);
      FONT.yDiff = FONT.yMax - FONT.yMin + FONT.yDist;
      shift_x = FONT.xDist / 2;
      shift_y = FONT.yDist / 2;
printf("xMin:  %d\n", FONT.xMin);
printf("xMax:  %d\n", FONT.xMax);
printf("yMin:  %d\n", FONT.yMin);
printf("yMax:  %d\n", FONT.yMax);
printf("xDiff: %d\n", FONT.xDiff);
printf("yDiff: %d\n", FONT.yDiff);
printf("xDist: %d\n", FONT.xDist);
printf("yDist: %d\n", FONT.yDist);
      writenumber(scalefile, FONT.xMin);
      writenumber(scalefile, FONT.xMax);
      writenumber(scalefile, FONT.yMin);
      writenumber(scalefile, FONT.yMax);
      writenumber(scalefile, FONT.xDiff);
      writenumber(scalefile, FONT.yDiff);
      writenumber(scalefile, FONT.xDist);
      writenumber(scalefile, FONT.yDist);
      fprintf(scalefile, "\n");
      while ((ch = fgetc(fontfile)) != EOF && ch != '\n') {
        putc(ch, stdout);
      } /* while */
      putc('\n', stdout);
#ifdef OUT_OF_ORDER
      fscanf(fontfile, "\n");
#endif
      for (fontch = 0; fontch < 256; fontch++) {
        FONT.characters[fontch].xshift = 0;
        FONT.characters[fontch].xwidth = -1;
        FONT.characters[fontch].polyline_count = 0;
        FONT.characters[fontch].polylines = NULL;
      } /* for */
      while ((fontch = fgetc(fontfile)) != EOF) {
        fprintf(scalefile, "%c", fontch);
        if (fontch == ' ') {
          fscanf(fontfile, "%d", &fontch);
          fprintf(scalefile, "%d", fontch);
        } /* if */
        ch = fgetc(fontfile);
        act_ch = &FONT.characters[fontch];
        act_ch->xwidth = scale(readnumber(fontfile), font_scale, screen_scale);
        act_ch->polyline_count = 0;
        act_ch->polylines = (polyLineType *)
            malloc(MAX_POLYLINE_COUNT * sizeof(polyLineType));
        fprintf(scalefile, " ");
        writenumber(scalefile, act_ch->xwidth);
#ifdef OUT_OF_ORDER
if (fontch > 128) {
  printf("fontch: %d  WIDTH: %d  ch: %d\n", fontch, act_ch->xwidth, ch);
}
#endif
        act_ch->xshift = (FONT.xDiff - act_ch->xwidth - 1) / 2;
        act_polyline = NULL;
        act_point = NULL;
        while ((ch = getc(fontfile)) != EOF && ch != '\n') {
          switch (ch) {
            case '+':
              if (act_point != NULL) {
                old_x = act_x;
                old_y = act_y;
                act_x = scale(readnumber(fontfile),
                    font_scale, screen_scale) + shift_x;
                act_y = screen_scale - scale(readnumber(fontfile),
                    font_scale, screen_scale) + shift_y;
                act_point->x = act_x - old_x;
                act_point->y = act_y - old_y;
                fprintf(scalefile, "+");
                writenumber(scalefile, act_point->x);
                writenumber(scalefile, act_point->y);
/*              printf("+(%d, %d)\n", act_point->x, act_point->y); */
                act_point++;
              } /* if */
              break;
            case '-':
            case '/':
              if (act_polyline != NULL) {
                act_polyline->point_count = act_point - act_polyline->points;
                act_polyline->points = (XPoint *)
                    realloc(act_polyline->points,
                    act_polyline->point_count * sizeof(XPoint));
                act_polyline++;
              } else {
                act_polyline = act_ch->polylines;
              } /* if */
              act_polyline->makefill = ch == '/';
              act_polyline->point_count = 0;
              act_polyline->points = (XPoint *)
                  malloc(MAX_POINT_COUNT * sizeof(XPoint));
              act_point = act_polyline->points;
              if (act_point != NULL) {
                act_x = scale(readnumber(fontfile),
                    font_scale, screen_scale) + shift_x;
                act_y = screen_scale - scale(readnumber(fontfile),
                    font_scale, screen_scale) + shift_y;
                act_point->x = act_x;
                act_point->y = act_y;
                fprintf(scalefile, "-");
                writenumber(scalefile, act_point->x);
                writenumber(scalefile, act_point->y);
/*              printf("-(%d, %d)\n", act_point->x, act_point->y); */
                act_point++;
              } /* if */
              break;
          } /* switch */
        } /* while */
        if (act_polyline != NULL) {
          act_polyline->point_count = act_point - act_polyline->points;
          act_polyline->points = (XPoint *)
              realloc(act_polyline->points,
              act_polyline->point_count * sizeof(XPoint));
          act_polyline++;
          act_ch->polyline_count = act_polyline - act_ch->polylines;
          act_ch->polylines = (polyLineType *)
              realloc(act_ch->polylines,
              act_ch->polyline_count * sizeof(polyLineType));
        } /* if */
        fprintf(scalefile, "\n");
      } /* while */
      fclose(fontfile);
      fclose(scalefile);
      for (fontch = 0; fontch < 256; fontch++) {
        act_ch = &FONT.characters[fontch];
        if (act_ch->xwidth == -1) {
          act_ch->xshift = 0;
          act_ch->xwidth = FONT.characters[' '].xwidth;
        } /* if */
      } /* for */
    } /* if */
printf("END x11_setfont(%s)\n", fontname);
fflush(stdout);
  } /* x11_setfont */
#endif



static intType txtwidth (unsigned char *stri, intType length,
    intType startidx, intType stopidx)

  {
    intType pos;
    intType width;

  /* txtwidth */
    width = 0;
    if (stopidx < length) {
      for (pos = startidx; pos <= stopidx; pos++) {
        width = width + actual_scaledfont->characters[stri[pos]].xwidth + actual_scaledfont->xDist;
      } /* for */
    } else {
      if (startidx < length) {
        for (pos = startidx; pos < length; pos++) {
          width = width + actual_scaledfont->characters[stri[pos]].xwidth + actual_scaledfont->xDist;
        } /* for */
        width = width + (stopidx - length + 1) *
            (actual_scaledfont->characters[' '].xwidth + actual_scaledfont->xDist);
      } else {
        width = (stopidx - startidx + 1) *
            (actual_scaledfont->characters[' '].xwidth + actual_scaledfont->xDist);
      } /* if */
    } /* if */
    return width;
  } /* txtwidth */



int conHeight (void)

  {
    XWindowAttributes attribs;

  /* conHeight */
    XGetWindowAttributes(mydisplay, mywindow, &attribs);
    return attribs.height/actual_scaledfont->yDiff;
  } /* conHeight */



int conWidth (void)

  {
    XWindowAttributes attribs;

  /* conWidth */
    XGetWindowAttributes(mydisplay, mywindow, &attribs);
    return attribs.width;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
    XFlush(mydisplay);
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
  } /* conCursor */



void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  { /* conClear */
    XClearArea(mydisplay, mywindow,
        startcol - 1, actual_scaledfont->yDiff * (startlin - 1),
        stopcol - startcol + 1, actual_scaledfont->yDiff * (stoplin - startlin + 1),
        False);
#ifdef OUT_OF_ORDER
    XDrawLine(mydisplay, mywindow, mygc,
        startcol - 1, actual_scaledfont->yDiff * (startlin - 1),
        stopcol- 1, actual_scaledfont->yDiff * stoplin - 1);
#endif
  } /* conClear */



static void screenlineclear (int xpos, int ypos, intType len)

  /* Clear a screen area at position xpos, ypos with the given      */
  /* length len and the height of a line.                           */

  {
    XPoint clear_area[5];

  /* screenlineclear */
    if (actual_scaledfont->slant == 0) {
      if (normal) {
        XClearArea(mydisplay, mywindow,
            xpos, ypos, len,
            actual_scaledfont->yDiff, False);
        XSetForeground(mydisplay, mygc, myforeground);
      } else {
        XFillRectangle(mydisplay, mywindow, mygc,
            xpos, ypos, len,
            actual_scaledfont->yDiff);
        XSetForeground(mydisplay, mygc, mybackground);
      } /* if */
    } else {
      if (normal) {
        XSetForeground(mydisplay, mygc, mybackground);
      } else {
        XSetForeground(mydisplay, mygc, myforeground);
      } /* if */
      clear_area[0].x = xpos + actual_scaledfont->slant;
      clear_area[0].y = ypos;
      clear_area[1].x = len;
      clear_area[1].y = 0;
      clear_area[2].x = -actual_scaledfont->bslant;
      clear_area[2].y = actual_scaledfont->yDiff - 1;
      clear_area[3].x = -len;
      clear_area[3].y = 0;
      clear_area[4].x = actual_scaledfont->bslant;
      clear_area[4].y = -actual_scaledfont->yDiff + 1;
        XFillPolygon(mydisplay, mywindow, mygc,
        clear_area, 5, Convex, CoordModePrevious);
      XDrawLines(mydisplay, mywindow, mygc,
        clear_area, 5, CoordModePrevious);
      if (normal) {
        XSetForeground(mydisplay, mygc, myforeground);
      } else {
        XSetForeground(mydisplay, mygc, mybackground);
      } /* if */
    } /* if */
  } /* screenlineclear */



void conSetCursor (intType line, intType column)

  /* Moves the system cursor to the given place of the console.     */
  /* If no system cursor exists this function can be replaced by    */
  /* a dummy function.                                              */

  {
    int charxpos;
    int charypos;

  /* conSetCursor */
#ifdef OUT_OF_ORDER
    printf("BEGIN conSetCursor(%d, %d)\n", line, column);
    charxpos = column - 1;
    charypos = actual_scaledfont->yDiff * (line - 1) + actual_scaledfont->yMax;
    XDrawLine(mydisplay, mywindow, mygc, charxpos, charypos,
        charxpos, charypos + actual_scaledfont->yDiff);
    printf("END conSetCursor(%d, %d)\n", line, column);
#endif
  } /* conSetCursor */



void conText (intType lin, intType col, ustriType stri,
memSizeType length)

  /* This function writes the string stri to the console at the     */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the screen. All console output      */
  /* must be done with this function.                               */

  {
    int charxpos;
    int charypos;
    int pos;
    int vect_num;
    charDefType *act_ch;
    polyLineType *act_polyline;

  /* conText */
#ifdef TRACE_SCRWRITE
    { char buffer[256];
      memcpy(buffer, stri, length);
      buffer[length] = '\0';
      printf("BEGIN conText(%d, %d, \"%s\", %d)\n", lin, col, buffer, length);
      fflush(stdout);
    }
#endif
    if (length >= 1) {
      charxpos = col - 1;
      charypos = lin - 1;
/*    charypos = actual_scaledfont->yDiff * (lin - 1); */
      screenlineclear(charxpos, charypos,
          txtwidth(stri, length, 0, length - 1));
#ifdef OUT_OF_ORDER
      if (normal) {
        XClearArea(mydisplay, mywindow,
            charxpos, charypos,
            txtwidth(stri, length, 0, length - 1),
            actual_scaledfont->yDiff, False);
        XSetForeground(mydisplay, mygc, myforeground);
      } else {
        XFillRectangle(mydisplay, mywindow, mygc,
            charxpos, charypos,
            txtwidth(stri, length, 0, length - 1),
            actual_scaledfont->yDiff);
        XSetForeground(mydisplay, mygc, mybackground);
      } /* if */
#endif
/*    stri = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
      length = 62; */
      for (pos = 0; pos < length; pos++) {
/*      printf("<%c, %d, %d>\n", stri[pos], charxpos, charypos); */
        act_ch = &actual_scaledfont->characters[stri[pos]];
/*
        printf("%c %c ", stri[pos],
            act_ch->xwidth >= 10 ? 'A' + act_ch->xwidth - 10 :
            (act_ch->xwidth < 0 ? 'a' - act_ch->xwidth - 1 :
            '0' + act_ch->xwidth));
        printf("[%d]", act_ch->polyline_count);
        fflush(stdout);
*/
        for (vect_num = 0; vect_num < act_ch->polyline_count; vect_num++) {
          act_polyline = &act_ch->polylines[vect_num];
/*
          printf(" <%d>", act_polyline->point_count);
          fflush(stdout);
*/
          act_polyline->points[0].x = charxpos + act_polyline->start_point.x;
          act_polyline->points[0].y = charypos + act_polyline->start_point.y;
          if (act_polyline->makefill) {
#ifdef OUT_OF_ORDER
            filled (charxpos + ACT_VECT->XPOS, charypos - ACT_VECT->YPOS);
#endif
/*          printf("/"); */
            XFillPolygon(mydisplay, mywindow, mygc, act_polyline->points,
                act_polyline->point_count, Nonconvex, CoordModePrevious);
            XDrawLines(mydisplay, mywindow, mygc, act_polyline->points,
                act_polyline->point_count, CoordModePrevious);
          } else {
#ifdef OUT_OF_ORDER
            not filled (charxpos + ACT_VECT->XPOS, charypos - ACT_VECT->YPOS);
#endif
/*          printf("-"); */
            XDrawLines(mydisplay, mywindow, mygc, act_polyline->points,
                act_polyline->point_count, CoordModePrevious);
          } /* if */
/*        printf("%c%c",
              ACT_VECT->XPOS >= 10 ? 'A' + ACT_VECT->XPOS - 10 :
              (ACT_VECT->XPOS < 0 ? 'a' - ACT_VECT->XPOS - 1 :
              '0' + ACT_VECT->XPOS),
              ACT_VECT->YPOS >= 10 ? 'A' + ACT_VECT->YPOS - 10 :
              (ACT_VECT->YPOS < 0 ? 'a' - ACT_VECT->YPOS - 1 :
              '0' + ACT_VECT->YPOS)); */
        } /* for */
/*
        printf("\n");
        fflush(stdout);
*/
/*      printf("\n"); */
        charxpos = charxpos + act_ch->xwidth + actual_scaledfont->xDist;
      } /* for */
      XSetForeground(mydisplay, mygc, myforeground);
    } /* if */
#ifdef TRACE_SCRWRITE
    { char buffer[256];
      memcpy(buffer, stri, length);
      buffer[length] = '\0';
      printf("END conText(%d, %d, \"%s\", %d)\n", lin, col, buffer, length);
      fflush(stdout);
    }
#endif
  } /* conText */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by numLines lines. The upper numLines lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numLines is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines)

  {
    int line1, line2;
    XImage *image;

  /* conUpScroll */
    line1 = actual_scaledfont->yDiff * (startlin + numLines - 1);
    line2 = actual_scaledfont->yDiff * stoplin - 1;
    XCopyArea(mydisplay, mywindow, mywindow, mygc,
        startcol - 1, line1,
        stopcol - startcol + 1, line2 - line1 + 1,
        startcol - 1, line1 - numLines * actual_scaledfont->yDiff);
#ifdef OUT_OF_ORDER
    while (line1 <= line2) {
      if (line1 + SCROLLBLOCKSIZE <= line2) {
#ifdef OUT_OF_ORDER
        getimage(pred(startcol), line1, pred(stopcol), line1 + SCROLLBLOCKSIZE, SCROLLBUFFER);
        putimage(pred(startcol), line1 - numLines * actual_scaledfont->yDiff, SCROLLBUFFER, normalput);
#endif
        image = XGetImage(mydisplay, mywindow, startcol - 1, line1,
            stopcol - startcol + 1, SCROLLBLOCKSIZE, -1, ZPixmap);
        XPutImage(mydisplay, mywindow, mygc, image, 0, 0,
            startcol - 1, line1 - numLines * actual_scaledfont->yDiff,
            stopcol - startcol + 1, SCROLLBLOCKSIZE);
      } else {
#ifdef OUT_OF_ORDER
        getimage(pred(startcol), line1, pred(stopcol), line2, SCROLLBUFFER);
        putimage(pred(startcol), line1 - numLines * actual_scaledfont->yDiff, SCROLLBUFFER, normalput);
#endif
        image = XGetImage(mydisplay, mywindow, startcol - 1, line1,
            stopcol - startcol + 1, line2 - line1 + 1, -1, ZPixmap);
        XPutImage(mydisplay, mywindow, mygc, image, 0, 0,
            startcol - 1, line1 - numLines * actual_scaledfont->yDiff,
            stopcol - startcol + 1, line2 - line1 + 1);
      } /* if */
      line1 = line1 + SCROLLBLOCKSIZE;
    } /* while */
#endif
    conClear(stoplin - numLines + 1, startcol, stoplin, stopcol);
  } /* conUpScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by numLines lines. The lower numLines lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numLines is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines)

  {
    int line1, line2;
    XImage *image;

  /* conDownScroll */
    line1 = actual_scaledfont->yDiff * (startlin - 1);
    line2 = actual_scaledfont->yDiff * (stoplin - numLines) - 1;
    XCopyArea(mydisplay, mywindow, mywindow, mygc,
        startcol - 1, line1,
        stopcol - startcol + 1, line2 - line1 + 1,
        startcol - 1, line1 + numLines * actual_scaledfont->yDiff);
#ifdef OUT_OF_ORDER
    while (line2 >= line1) {
      if (line2 - SCROLLBLOCKSIZE >= line1) {
#ifdef OUT_OF_ORDER
        getimage(pred(startcol), line2 - SCROLLBLOCKSIZE, pred(stopcol), line2, SCROLLBUFFER);
        putimage(pred(startcol), line2 - SCROLLBLOCKSIZE + actual_scaledfont->yDiff * numLines, SCROLLBUFFER, normalput);
#endif
        image = XGetImage(mydisplay, mywindow, startcol - 1, line2 - SCROLLBLOCKSIZE,
            stopcol - startcol + 1, SCROLLBLOCKSIZE, -1, ZPixmap);
        XPutImage(mydisplay, mywindow, mygc, image, 0, 0,
            startcol - 1, line2 - SCROLLBLOCKSIZE + actual_scaledfont->yDiff * numLines,
            stopcol - startcol + 1, SCROLLBLOCKSIZE);
      } else {
#ifdef OUT_OF_ORDER
        getimage(pred(startcol), line1, pred(stopcol), line2, SCROLLBUFFER);
        putimage(pred(startcol), line1 + actual_scaledfont->yDiff * numLines, SCROLLBUFFER, normalput);
#endif
        image = XGetImage(mydisplay, mywindow, startcol - 1, line1,
            stopcol - startcol + 1, line2 - line1 + 1, -1, ZPixmap);
        XPutImage(mydisplay, mywindow, mygc, image, 0, 0,
            startcol - 1, line1 + actual_scaledfont->yDiff * numLines,
            stopcol - startcol + 1, line2 - line1 + 1);
      } /* if */
      line2 = line2 - SCROLLBLOCKSIZE;
    } /* while */
#endif
    conClear(startlin, startcol, startlin + numLines - 1, stopcol);
  } /* conDownScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by numCols columns. The left numCols columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numCols is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols)

  { /* conLeftScroll */
  } /* conLeftScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by numCols columns. The right numCols columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numCols is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols)

  { /* conRightScroll */
  } /* conRightScroll */



static void x11_beep (void)

  { /* x11_beep */
    XBell(mydisplay, 0);
    XFlush(mydisplay);
  } /* x11_beep */



void conShut (void)

  { /* conShut */
    if (screen_initialized) {
      XFreeGC(mydisplay, mygc);
      XDestroyWindow(mydisplay, mywindow);
    } /* if */
  } /* conShut */



static int do_x11_init (int upper, int left,
    int high, int wide)

  {
    XSizeHints mysizehint;
    XWMHints mywmhint;
    char hello[] = {"SE"};
    int result = 0;

  /* do_x11_init */
#ifdef TRACE_INIT
    printf("BEGIN do_x11_init\n");
    fflush(stdout);
#endif
    mywindow = 0;
    if (mydisplay != NULL) {
/*    printf("mydisplay = %lu\n", (long unsigned) mydisplay); */
      myscreen = DefaultScreen(mydisplay);
/*    printf("myscreen = %lu\n", (long unsigned) myscreen); */

      mybackground = WhitePixel(mydisplay, myscreen);
      myforeground = BlackPixel(mydisplay, myscreen);

      mysizehint.x = left;
      mysizehint.y = upper;
      mysizehint.width = wide;
      mysizehint.height = high;
      mysizehint.flags = PPosition | PSize;
      mywmhint.flags = InputHint;
      mywmhint.input = True;
#ifdef TRACE_INIT
    printf("vor XCreateSimpleWindow\n");
    fflush(stdout);
#endif
      mywindow = XCreateSimpleWindow(mydisplay,
          DefaultRootWindow(mydisplay),
          mysizehint.x, mysizehint.y, mysizehint.width, mysizehint.height,
          5, myforeground, mybackground);
#ifdef TRACE_INIT
    printf("vor XSetStandardProperties\n");
    fflush(stdout);
#endif
      XSetStandardProperties(mydisplay, mywindow, hello, hello,
          None, /* argv, argc, */ NULL, 0, &mysizehint);
#ifdef TRACE_INIT
    printf("vor XSetWMHints\n");
    fflush(stdout);
#endif
      XSetWMHints(mydisplay, mywindow, &mywmhint);
#ifdef TRACE_INIT
    printf("vor XSetWMNormalHints\n");
    fflush(stdout);
#endif
      XSetWMNormalHints(mydisplay, mywindow, &mysizehint);
#ifdef TRACE_INIT
    printf("vor XCreateGC\n");
    fflush(stdout);
#endif
      mygc = XCreateGC(mydisplay, mywindow, 0, 0);
      XSetBackground(mydisplay, mygc, mybackground);
      XSetForeground(mydisplay, mygc, myforeground);

      XSelectInput(mydisplay, mywindow,
          ButtonPressMask | KeyPressMask | ExposureMask);

      XMapRaised(mydisplay, mywindow);

      result = 1;
    } /* if */
#ifdef TRACE_INIT
    printf("END do_x11_init\n");
    fflush(stdout);
#endif
    return result;
  } /* do_x11_init */



int conOpen (void)

  /* Initializes and clears the console.                            */

  {
    char *home_path;
    memSizeType font_name_size;
    char *font_name;
    int result = 0;

  /* conOpen */
#ifdef TRACE_INIT
    printf("BEGIN conOpen\n");
    fflush(stdout);
#endif
    if ((mydisplay = XOpenDisplay("")) != NULL) {
      if ((home_path = getenv("HOME")) != NULL) {
        font_name_size = strlen(home_path) + 16;
        font_name = malloc(font_name_size + 1);
        if (font_name != NULL) {
          strcpy(font_name, home_path);
          if (font_name[0] != '\0' &&
              font_name[strlen(font_name) - 1] != '/') {
            strcat(font_name, "/");
          } /* if */
          strcat(font_name, "/font/simple.fnt");
        } else {
          font_name = "";
        } /* if */
      } else {
        font_name = "";
      } /* if */
      if (x11_setfont(font_name)) {
        if (do_x11_init(100, 100, 640, 960)) {
          maxx = 640;
          maxy = 960;
          x11_normalcolour();
          result = 1;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_INIT
    printf("END conOpen\n");
    fflush(stdout);
#endif
    return result;
  } /* conOpen */
