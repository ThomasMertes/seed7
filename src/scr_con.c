/********************************************************************/
/*                                                                  */
/*  scr_con.c     Driver for conio screen access                    */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/scr_con.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Driver for conio screen access                         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "conio.h"

#include "common.h"
#include "scr_drv.h"
#include "kbd_drv.h"


/* #define atexit(x) */

#define SCRHEIGHT 25
#define SCRWIDTH 80

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

static char currentattribute;
static char outbuffer[SCRWIDTH];
static booltype screen_initialized = FALSE;
static booltype cursor_on = FALSE;


#ifdef MAP_TO_ISO
static char MAP[] = {
/*   0 */ '\000','\001','\002','\003','\004','\005','\006','\007','\010','\011',
/*  10 */ '\012','\013','\014','\015','\016','\017','\020','\021','\022','\023',
/*  20 */ '\024','\025','\026','\027','\030','\031','\032','\033','\034','\035',
/*  30 */ '\036','\037',' ',   '!',   '\"',  '#',   '$',   '%',   '&',   '\'',
/*  40 */ '(',   ')',   '*',   '+',   ',',   '-',   '.',   '/',   '0',   '1',
/*  50 */ '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   ':',   ';',
/*  60 */ '<',   '=',   '>',   '?',   '@',   'A',   'B',   'C',   'D',   'E',
/*  70 */ 'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',   'N',   'O',
/*  80 */ 'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',
/*  90 */ 'Z',   '[',   '\\',  ']',   '^',   '_',   '`',   'a',   'b',   'c',
/* 100 */ 'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',
/* 110 */ 'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',
/* 120 */ 'x',   'y',   'z',   '{',   '|',   '}',   '~',   '#',   ' ',   ' ',
/* 130 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 140 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 150 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 160 */ ' ',   '≠',   'õ',   'ú',   '',   'ù',   '|',   '',   '\"',  '?',
/* 170 */ '¶',   'Æ',   '™',   '-',   '?',   '?',   '¯',   'Ò',   '˝',   '?',
/* 180 */ '\'',  'Ê',   '',   '˘',   ',',   '?',   'ß',   'Ø',   '¨',   '´',
/* 190 */ '?',   '®',   '?',   '?',   '?',   '?',   'é',   'è',   'í',   'Ä',
/* 200 */ '?',   'ê',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '•',
/* 210 */ '?',   '?',   '?',   '?',   'ô',   '?',   '?',   '?',   '?',   '?',
/* 220 */ 'ö',   '?',   '?',   '·',   'Ö',   '†',   'É',   '?',   'Ñ',   'Ü',
/* 230 */ 'ë',   'á',   'ä',   'Ç',   'à',   'â',   'ç',   '°',   'å',   'ã',
/* 240 */ 'Î',   '§',   'ï',   '¢',   'ì',   '?',   'î',   'ˆ',   '?',   'ó',
/* 250 */ '£',   'ñ',   'Å',   '?',   '?',   'ò'};
#endif


static int MAP_KEY[] = {
/*   0 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_NULCHAR,  K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/*   8 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_BACKTAB,
/*  16 */ K_ALT_Q,  K_ALT_W,   K_ALT_E,  K_ALT_R,    K_ALT_T, K_ALT_Y, K_ALT_U, K_ALT_I,
/*  24 */ K_ALT_O,  K_ALT_P,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_ALT_A, K_ALT_S,
/*  32 */ K_ALT_D,  K_ALT_F,   K_ALT_G,  K_ALT_H,    K_ALT_J, K_ALT_K, K_ALT_L, K_UNDEF,
/*  40 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_ALT_Z, K_ALT_X, K_ALT_C, K_ALT_V,
/*  48 */ K_ALT_B,  K_ALT_N,   K_ALT_M,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/*  56 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_F1,       K_F2, K_F3, K_F4, K_F5,
/*  64 */ K_F6,     K_F7,      K_F8,     K_F9,       K_F10, K_UNDEF, K_UNDEF, K_HOME,
/*  72 */ K_UP,     K_PGUP,    K_UNDEF,  K_LEFT,     K_UNDEF, K_RIGHT, K_UNDEF, K_END,
/*  80 */ K_DOWN,   K_PGDN,    K_INS,    K_DEL,      K_SFT_F1, K_SFT_F2, K_SFT_F3, K_SFT_F4,
/*  88 */ K_SFT_F5, K_SFT_F6,  K_SFT_F7, K_SFT_F8,   K_SFT_F9, K_SFT_F10, K_CTL_F1, K_CTL_F2,
/*  96 */ K_CTL_F3, K_CTL_F4,  K_CTL_F5, K_CTL_F6,   K_CTL_F7, K_CTL_F8, K_CTL_F9, K_CTL_F10,
/* 104 */ K_ALT_F1, K_ALT_F2,  K_ALT_F3, K_ALT_F4,   K_ALT_F5, K_ALT_F6, K_ALT_F7, K_ALT_F8,
/* 112 */ K_ALT_F9, K_ALT_F10, K_UNDEF,  K_CTL_LEFT, K_CTL_RIGHT, K_CTL_END, K_CTL_PGDN, K_CTL_HOME,
/* 120 */ K_ALT_1,  K_ALT_2,   K_ALT_3,  K_ALT_4,    K_ALT_5, K_ALT_6, K_ALT_7, K_ALT_8,
/* 128 */ K_ALT_9,  K_ALT_0,   K_UNDEF,  K_UNDEF,    K_CTL_PGUP, K_UNDEF, K_UNDEF, K_UNDEF,
/* 136 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 144 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 152 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 160 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_CTL_INS, K_CTL_DEL, K_UNDEF,
/* 168 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 176 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 184 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 192 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 200 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 208 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 216 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 224 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 232 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 240 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 248 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF};



#ifdef ANSI_C

void kbdShut (void)
#else

void kbdShut ()
#endif

  { /* kbdShut */
  } /* kbdShut */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  { /* kbdKeyPressed */
    return(kbhit());
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    chartype key;

  /* kbdGetc */
    key = getch();
    if (key == 0) {
      key = getch();
      key = MAP_KEY[key];
    } /* if */
    if (key == 13) {
      key = 10;
    } /* if */
#ifdef OUT_OF_ORDER
      switch (key) {
        case 59: key = K_F1;     break;
        case 60: key = K_F2;     break;
        case 61: key = K_F3;     break;
        case 62: key = K_F4;     break;
        case 63: key = K_F5;     break;
        case 64: key = K_F6;     break;
        case 65: key = K_F7;     break;
        case 66: key = K_F8;     break;
        case 67: key = K_F9;     break;
        case 68: key = K_F10;    break;
        case 71: key = K_HOME;   break;
        case 72: key = K_UP;     break;
        case 73: key = K_PGUP;   break;
        case 75: key = K_LEFT;   break;
        case 77: key = K_RIGHT;  break;
        case 79: key = K_END;    break;
        case 80: key = K_DOWN;   break;
        case 81: key = K_PGDN;   break;
        case 84: key = K_SFT_F1; break;
        case 85: key = K_SFT_F2; break;
        case 86: key = K_SFT_F3; break;
        case 87: key = K_SFT_F4; break;
        case 88: key = K_SFT_F5; break;
        case 89: key = K_SFT_F6; break;
        case 90: key = K_SFT_F7; break;
        case 91: key = K_SFT_F8; break;
        case 92: key = K_SFT_F9; break;
        case 93: key = K_SFT_F10;break;
    } /* if */
#endif
    return(key);
  } /* kbdGetc */



#ifdef ANSI_C

chartype kbdRawGetc (void)
#else

chartype kbdRawGetc ()
#endif

  { /* kbdRawGetc */
    return(kbdGetc());
  } /* kbdRawGetc */



#ifdef ANSI_C

static void con_beep (void)
#else

static void con_beep ()
#endif

  { /* con_beep */
    fputc('\007', stderr);
  } /* con_beep */



#ifdef ANSI_C

static void con_setcolor (inttype foreground, inttype background)
#else

static void con_setcolor (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* con_setcolor */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* con_setcolor */



#ifdef ANSI_C

static void con_standardcolour (void)
#else

static void con_standardcolour ()
#endif

  { /* con_standardcolour */
    con_setcolor(lightgray, black);
  } /* con_standardcolour */



#ifdef ANSI_C

static void con_normalcolour (void)
#else

static void con_normalcolour ()
#endif

  { /* con_normalcolour */
    con_setcolor(lightgray, black);
  } /* con_normalcolour */



#ifdef ANSI_C

static void con_setfont (char *fontname)
#else

static void con_setfont (fontname)
char *fontname;
#endif

  { /* con_setfont */
  } /* con_setfont */



#ifdef ANSI_C

inttype textheight (void)
#else

inttype textheight ()
#endif

  { /* textheight */
    return 1;
  } /* textheight */



#ifdef ANSI_C

inttype textwidth (stritype stri,
    inttype startcol, inttype stopcol)
#else

inttype textwidth (stri, startcol, stopcol)
stritype stri;
inttype startcol;
inttype stopcol;
#endif

  { /* textwidth */
    return(stopcol + 1 - startcol);
  } /* textwidth */



#ifdef ANSI_C

void textcolumns (stritype stri, inttype striwidth,
    inttype * cols, inttype *rest)
#else

void textcolumns (stri, striwidth, cols, rest)
stritype stri;
inttype striwidth;
inttype *cols;
inttype *rest;
#endif

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



#ifdef ANSI_C

int scrHeight (void)
#else

int scrHeight ()
#endif

  { /* scrHeight */
    return(SCRHEIGHT);
  } /* scrHeight */



#ifdef ANSI_C

int scrWidth (void)
#else

int scrWidth ()
#endif

  { /* scrWidth */
    return(SCRWIDTH);
  } /* scrWidth */



#ifdef ANSI_C

void scrFlush (void)
#else

void scrFlush ()
#endif

  { /* scrFlush */
  } /* scrFlush */



#ifdef ANSI_C

void scrCursor (booltype on)
#else

void scrCursor (on)
booltype on;
#endif

  { /* scrCursor */
    cursor_on = on;
    if (on) {
      _setcursortype(_SOLIDCURSOR);
    } else {
      _setcursortype(_NOCURSOR);
    } /* if */
  } /* scrCursor */



#ifdef ANSI_C

void scrSetCursor (inttype lin, inttype col)
#else

void scrSetCursor (lin, col)
inttype lin;
inttype col;
#endif

  /* Moves the system curser to the given place of the screen.      */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  { /* scrSetCursor */
    gotoxy(col, lin);
  } /* scrSetCursor */



#ifdef ANSI_C

void scrText (inttype lin, inttype col, ustritype stri,
memsizetype length)
#else

void scrText (lin, col, stri, length)
inttype lin;
inttype col;
ustritype stri;
memsizetype length;
#endif

  /* This function writes the string stri to the screen at the      */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the screen. The string stri is not allowed to go   */
  /* beyond the right border of the screen. All screen output       */
  /* must be done with this function.                               */

  { /* scrText */
    memcpy(outbuffer, stri, length);
    outbuffer[length] = '\0';
    scrSetCursor(lin, col);
    cputs(outbuffer);
  } /* scrText */



#ifdef ANSI_C

void scrClear (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol)
#else

void scrClear (startlin, startcol, stoplin, stopcol)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
#endif

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  { /* scrClear */
    clrscr();
  } /* scrClear */



#ifdef ANSI_C

void scrUpScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrUpScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol upward by count lines. The upper count lines of the    */
  /* area are overwritten. At the lower end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* scrUpScroll */
  } /* scrUpScroll */



#ifdef ANSI_C

void scrDownScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrDownScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol downward by count lines. The lower count lines of the  */
  /* area are overwritten. At the upper end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* scrDownScroll */
  } /* scrDownScroll */



#ifdef ANSI_C

void scrLeftScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrLeftScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol leftward by count lines. The left count lines of the   */
  /* area are overwritten. At the right end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* scrLeftScroll */
  } /* scrLeftScroll */



#ifdef ANSI_C

void scrRightScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrRightScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol rightward by count lines. The right count lines of the */
  /* area are overwritten. At the left end of the area blank lines  */
  /* are inserted. Nothing is changed outside the area.             */

  { /* scrRightScroll */
  } /* scrRightScroll */



#ifdef ANSI_C

void scrShut (void)
#else

void scrShut ()
#endif

  { /* scrShut */
    if (screen_initialized) {
      con_standardcolour();
      scrCursor(TRUE);
      scrClear(1, 1, 25, 80);
      scrSetCursor(1, 24);
      screen_initialized = FALSE;
    } /* if */
  } /* scrShut */



#ifdef ANSI_C

int scrOpen (void)
#else

int scrOpen ()
#endif

  /* Initializes and clears the screen.                             */

  { /* scrOpen */
    con_normalcolour();
    scrClear(1, 1, 25, 80);
    scrCursor(FALSE);
    screen_initialized = TRUE;
    atexit(scrShut);
    return(1);
  } /* scrOpen */
