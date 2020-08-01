/********************************************************************/
/*                                                                  */
/*  scr_dos.c     Driver for dos screen access                      */
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
/*  File: seed7/src/scr_dos.c                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Driver for dos screen access                           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "dos.h"

#include "common.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "scr_drv.h"

#define atexit(x)


#define SCRHEIGHT 25
#define SCRWIDTH 80

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

typedef struct {
  char character, attribute;
} cell;

typedef cell lineofscreen[SCRWIDTH];

typedef struct {
    cell screen[SCRHEIGHT][SCRWIDTH];
  } screentype;

static booltype monochrom;
static screentype *current_screen;
static char currentattribute;
static lineofscreen outbuffer;
static booltype screen_initialized = FALSE;
static booltype cursor_on = FALSE;

static unsigned char cursor_startline;
static unsigned char cursor_endline;


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


static int map_key[] = {
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
    return((char) bdos(0xB, 0, 0) & 1);
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    union REGS r;
    chartype key;

  /* kbdGetc */
    r.h.ah = (unsigned char) 0;
    int86(0x16, &r, &r);
    key = (chartype) r.h.al;
    if (key == 0) {
      key = map_key[r.h.ah];
    } /* if */
    if (key == 13) {
      key = 10;
    } /* if */
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

void snd_beep (void)
#else

void snd_beep ()
#endif

  { /* snd_beep */
    fputc('\007', stderr);
  } /* snd_beep */



#ifdef ANSI_C

void setcolour (inttype foreground, inttype background)
#else

void setcolour (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* setcolour */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* setcolour */



#ifdef ANSI_C

void standardcolour (void)
#else

void standardcolour ()
#endif

  { /* standardcolour */
    setcolour(lightgray,black);
  } /* standardcolour */



#ifdef ANSI_C

void normalcolour (void)
#else

void normalcolour ()
#endif

  { /* normalcolour */
    setcolour(lightgray,black);
  } /* normalcolour */



#ifdef ANSI_C

void setfont (char *fontname)
#else

void setfont (fontname)
char *fontname;
#endif

  { /* setfont */
  } /* setfont */



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

  {
    union REGS r;

  /* scrCursor */
    cursor_on = on;
    if (on) {
      r.h.ah = (unsigned char) 1; /* set cursor type */
      r.h.ch = (unsigned char) cursor_startline;
      r.h.cl = (unsigned char) cursor_endline;
      int86(0x10, &r, &r);
    } else {
      r.h.ah = (unsigned char) 1; /* set cursor type */
      r.h.ch = (unsigned char) 32;
      r.h.cl = (unsigned char) 0;
      int86(0x10, &r, &r);
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

  {
    union REGS r;

  /* scrSetCursor */
    r.h.ah = (unsigned char) 2; /* cursor addressing function */
    r.h.dh = (unsigned char) (lin - 1);
    r.h.dl = (unsigned char) (col - 1);
    r.h.bh = (unsigned char) 0; /* video page */
    int86(0x10, &r, &r);
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

  {
    inttype pos;

  /* scrText */
    for (pos = 0; pos < length; pos++) {
#ifdef MAP_TO_ISO
      outbuffer[pos].character = MAP[stri[pos] & 0xFF];
#else
      outbuffer[pos].character = stri[pos];
#endif
      outbuffer[pos].attribute = currentattribute;
    } /* for */
    memcpy(&current_screen->screen[lin - 1][col - 1],
      outbuffer, 2 * length);
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

  {
    union REGS r;

  /* scrClear */
    r.h.ah = (unsigned char) 6; /* scroll up code */
    r.h.al = (unsigned char) 0; /* clear screen code */
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) currentattribute; /* blank line colour */
    int86(0x10, &r, &r);
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

  {
    union REGS r;

  /* scrUpScroll */
    r.h.ah = (unsigned char) 6; /* scroll up code */
    r.h.al = (unsigned char) count;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
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

  {
    union REGS r;

  /* scrDownScroll */
    r.h.ah = (unsigned char) 7; /* scroll down code */
    r.h.al = (unsigned char) count;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
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

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* scrLeftScroll */
    if (count > 0) {
      num_bytes = 2 * (stopcol - startcol - count + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol + count - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
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

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* scrRightScroll */
    if (count > 0) {
      num_bytes = 2 * (stopcol - startcol - count + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol + count - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
  } /* scrRightScroll */



#ifdef ANSI_C

void scrShut (void)
#else

void scrShut ()
#endif

  { /* scrShut */
    if (screen_initialized) {
      standardcolour();
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

  {
    union REGS r;

  /* scrOpen */
    normalcolour();
    scrClear(1, 1, 25, 80);

    /* Lowlevel request to find out the video state */
    r.h.ah = (unsigned char) 15; /* read video state */
    int86(0x10, &r, &r);
    if (r.h.al == 7) {
      monochrom = TRUE;
      current_screen = (screentype *) 0xB0000000;
    } else {
      monochrom = FALSE;
      current_screen = (screentype *) 0xB8000000;
    } /* if */
    r.h.ah = (unsigned char) 3; /* read cursor position */
    r.h.bh = (unsigned char) 0;
    int86(0x10, &r, &r);
    cursor_startline = r.h.ch;
    cursor_endline = r.h.cl;
    scrCursor(FALSE);
    screen_initialized = TRUE;
    atexit(scrShut);
    return(1);
  } /* scrOpen */
