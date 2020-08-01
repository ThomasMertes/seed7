/********************************************************************/
/*                                                                  */
/*  scr_win.c     Driver for windows screen access                  */
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
/*  File: seed7/src/scr_win.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Driver for windows screen access                       */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "conio.h"

#include "version.h"
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


#ifdef OUT_OF_ORDER
static char MAP_CHAR[] = {
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


static chartype MAP_CHAR[] = {
/*   0 */ 0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007, 0010, 0011, 
/*  10 */ 0012, 0013, 0014, 0015, 0016, 0017, 0020, 0021, 0022, 0023, 
/*  20 */ 0024, 0025, 0026, 0027, 0030, 0031, 0032, 0033, 0034, 0035, 
/*  30 */ 0036, 0037, ' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'',
/*  40 */ '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */ '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */ '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */ 'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */ 'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */ 'Z',  '[',  '\\', ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */ 'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */ 'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */ 'x',  'y',  'z',  '{',  '|',  '}',  '~',  0177, 0307, 0374, 
/* 130 */ 0351, 0342, 0344, 0340, 0345, 0347, 0352, 0353, 0350, 0357, 
/* 140 */ 0356, 0354, 0304, 0305, 0311, 0346, 0306, 0364, 0366, 0362, 
/* 150 */ 0373, 0371, 0377, 0326, 0334, 0370, 0243, 0330, 0327, '?',
/* 160 */ 0341, 0355, 0363, 0372, 0361, 0321, 0252, 0272, 0277, 0256, 
/* 170 */ 0254, 0275, 0274, 0241, 0253, 0273, '?',  '?',  '?',  '?',
/* 180 */ '?',  0301, 0302, 0300, 0251, '?',  '?',  '?',  '?',  0242, 
/* 190 */ 0245, '?',  '?',  '?',  '?',  '?',  '?',  '?',  0343, 0303, 
/* 200 */ '?',  '?',  '?',  '?',  '?',  '?',  '?',  0244, 0360, 0320, 
/* 210 */ 0312, 0313, 0310, 0271, 0315, 0316, 0317, '?',  '?',  '?',
/* 220 */ '?',  0246, 0314, '?',  0363, 0337, 0324, 0322, 0365, 0325, 
/* 230 */ 0265, 0376, 0336, 0332, 0333, 0331, 0375, 0335, 0257, 0264, 
/* 240 */ 0255, 0261, '?',  0276, 0266, 0247, 0367, 0270, 0260, 0250, 
/* 250 */ 0267, 0271, 0263, 0262, '?',  0240};


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
    } else if (key == 224) {
      key = getch();
      key = MAP_KEY[key];
    } else {
      key = MAP_CHAR[key];
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

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO scr_info;

  /* scrHeight */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &scr_info)) {
        return(scr_info.dwSize.Y);
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) ==> Error %d\n",
            hConsole, GetLastError()); */
        return(SCRHEIGHT);
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
      return(SCRHEIGHT);
    } /* if */
  } /* scrHeight */



#ifdef ANSI_C

int scrWidth (void)
#else

int scrWidth ()
#endif

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO scr_info;

  /* scrWidth */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &scr_info)) {
        return(scr_info.dwSize.X);
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) ==> Error %d\n",
            hConsole, GetLastError()); */
        return(SCRWIDTH);
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
      return(SCRWIDTH);
    } /* if */
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
#ifdef OUT_OF_ORDER
    if (on) {
      _setcursortype(_SOLIDCURSOR);
    } else {
      _setcursortype(_NOCURSOR);
    } /* if */
#endif
  } /* scrCursor */



#ifdef ANSI_C

void scrSetpos (inttype lin, inttype col)
#else

void scrSetpos (lin, col)
inttype lin;
inttype col;
#endif

  /* Moves the system curser to the given place of the screen.      */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  {
    HANDLE hConsole;
    COORD position;

  /* scrSetpos */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      position.X = col - 1;
      position.Y = lin - 1;
      if (!SetConsoleCursorPosition(hConsole, position)) {
        /* printf("SetConsoleCursorPosition(%d, (%d, %d)) ==> Error %d\n",
            hConsole, col - 1, lin - 1, GetLastError()); */
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* scrSetpos */



#ifdef ANSI_C

void scrWrite (inttype lin, inttype col, ustritype stri,
memsizetype length)
#else

void scrWrite (lin, col, stri, length)
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
    HANDLE hConsole;
    COORD position;
    inttype numchars;

  /* scrWrite */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      position.X = col - 1;
      position.Y = lin - 1;
      if (SetConsoleCursorPosition(hConsole, position)) {
        WriteConsole(hConsole, stri, length, &numchars, NULL);
      } else {
        /* printf("SetConsoleCursorPosition(%d, (%d, %d)) ==> Error %d\n",
            hConsole, col - 1, lin - 1, GetLastError()); */
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* scrWrite */



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
    HANDLE hConsole;
    COORD position;
    inttype numchars;

  /* scrClear */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    position.X = startcol - 1;
    position.Y = startlin - 1;
    while (position.Y < stoplin) {
      FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
          stopcol - startcol + 1, position, &numchars);
      position.Y++;
    } /* while */
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
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* scrUpScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin + count - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
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
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* scrDownScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - count - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin + count - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
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
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* scrLeftScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol + count - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
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
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* scrRightScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - count - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol + count - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
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
      scrClear(1, 1, scrHeight(), scrWidth());
      scrSetpos(1, 24);
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
    scrClear(1, 1, scrHeight(), scrWidth());
    scrCursor(FALSE);
    screen_initialized = TRUE;
    atexit(scrShut);
    return(1);
  } /* scrOpen */
