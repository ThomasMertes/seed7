/********************************************************************/
/*                                                                  */
/*  con_cur.c     Driver for curses console access                  */
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
/*  File: seed7/src/con_cur.c                                       */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Driver for curses console access                       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "curses.h"

#include "common.h"

#include "kbd_drv.h"
#include "con_drv.h"


#define MAP_CHARS

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

static int currentattribute = 0;
static booltype key_buffer_filled = FALSE;
static int last_key;
booltype changes = FALSE;
static booltype scroll_allowed = FALSE;
static booltype console_initialized = FALSE;
static booltype cursor_on = FALSE;

static booltype keybd_initialized = FALSE;


#ifdef MAP_CHARS
#ifdef MAP_TO_ISO
static char map[] = {
/*   0 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/*  10 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/*  20 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/*  30 */ '?',   '?',   ' ',   '!',   '\"',  '#',   '$',   '%',   '&',   '\'',
/*  40 */ '(',   ')',   '*',   '+',   ',',   '-',   '.',   '/',   '0',   '1',
/*  50 */ '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   ':',   ';',
/*  60 */ '<',   '=',   '>',   '?',   '@',   'A',   'B',   'C',   'D',   'E',
/*  70 */ 'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',   'N',   'O',
/*  80 */ 'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',
/*  90 */ 'Z',   '[',   '\\',  ']',   '^',   '_',   '`',   'a',   'b',   'c',
/* 100 */ 'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',
/* 110 */ 'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',
/* 120 */ 'x',   'y',   'z',   '{',   '|',   '}',   '~',   '?',   '?',   '?',
/* 130 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 140 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 150 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 160 */ ' ',   '\270','\277','\273','\272','\274','|',   '\275','\253','?',
/* 170 */ '\371','\373','?',   '\366','?',   '?',   '\263','\376','?',   '?',
/* 180 */ '\250','\363','\364','\362',',',   '?',   '\372','\375','\367','\370',
/* 190 */ '\365','\271','\241','\340','\242','\341','\330','\320','\323','\264',
/* 200 */ '\243','\334','\244','\245','\346','\345','\246','\247','\343','\266',
/* 210 */ '\350','\347','\337','\351','\332','?',   '\322','\255','\355','\256',
/* 220 */ '\333','\261','\360','\336','\310','\304','\300','\342','\314','\324',
/* 230 */ '\327','\265','\311','\305','\301','\315','\331','\325','\321','\335',
/* 240 */ '\344','\267','\312','\306','\302','\352','\316','?',   '\326','\313',
/* 250 */ '\307','\303','\317','\262','\361','\357'};
#else
static char map[] = {
/*   0 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   ' ',
/*  10 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/*  20 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/*  30 */ '?',   '?',   ' ',   '!',   '\"',  '#',   '$',   '%',   '&',   '\'',
/*  40 */ '(',   ')',   '*',   '+',   ',',   '-',   '.',   '/',   '0',   '1',
/*  50 */ '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   ':',   ';',
/*  60 */ '<',   '=',   '>',   '?',   '@',   'A',   'B',   'C',   'D',   'E',
/*  70 */ 'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',   'N',   'O',
/*  80 */ 'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',
/*  90 */ 'Z',   '[',   '\\',  ']',   '^',   '_',   '`',   'a',   'b',   'c',
/* 100 */ 'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',
/* 110 */ 'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',
/* 120 */ 'x',   'y',   'z',   '{',   '|',   '}',   '~',   '?',   '?',   '?',
/* 130 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 140 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 150 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 160 */ '?',   '\241','\242','\243','\244','\245','\246','\247','\250','\251',
/* 170 */ '\252','\253','\254','\255','\256','\257','\260','\261','\262','\263',
/* 180 */ '\264','\265','\266','\267','\270','\271','\272','\273','\274','\275',
/* 190 */ '\276','\277','\300','\301','\302','\303','\304','\305','\306','\307',
/* 200 */ '\310','\311','\312','\313','\314','\315','\316','\317','\320','\321',
/* 210 */ '\322','\323','\324','\325','\326','\327','\330','\331','\332','\333',
/* 220 */ '\334','\335','\336','\337','\340','\341','\342','\343','\344','\345',
/* 230 */ '\346','\347','\350','\351','\352','\353','\354','\355','\356','\357',
/* 240 */ '\360','\361','\362','\363','\364','\365','\366','\367','\370','\371',
/* 250 */ '\372','\373','\374','\375','\376','?'};

/*                                                         '?',   '\200','\201',
          '\202','\203','\204','\205','\206','\207','\210','\211','\212','\213',
          '\214','\215','\216','\217','\220','\221','\222','\223','\224','\225',
          '\226','\227','\230','\231','\232','\233','\234','\235','\236','\237',
*/

#endif
#endif


static int map_key[256];



#ifdef ANSI_C

void kbdShut (void)
#else

void kbdShut ()
#endif

  { /* kbdShut */
  } /* kbdShut */



#ifdef ANSI_C

static void kbd_init (void)
#else

static void kbd_init ()
#endif

  {
    int position;

  /* kbd_init */
    for (position = 0; position <= 255; position++) {
      map_key[position] = K_UNDEF;
    } /* for */
    map_key[KEY_BREAK - KEY_BREAK] =     K_UNDEF;
    map_key[KEY_DOWN - KEY_BREAK] =      K_DOWN;
    map_key[KEY_UP - KEY_BREAK] =        K_UP;
    map_key[KEY_LEFT - KEY_BREAK] =      K_LEFT;
    map_key[KEY_RIGHT - KEY_BREAK] =     K_RIGHT;
    map_key[KEY_HOME - KEY_BREAK] =      K_HOME;
    map_key[KEY_BACKSPACE - KEY_BREAK] = K_BS;
    map_key[KEY_F(1) - KEY_BREAK] =      K_F1;
    map_key[KEY_F(2) - KEY_BREAK] =      K_F2;
    map_key[KEY_F(3) - KEY_BREAK] =      K_F3;
    map_key[KEY_F(4) - KEY_BREAK] =      K_F4;
    map_key[KEY_F(5) - KEY_BREAK] =      K_F5;
    map_key[KEY_F(6) - KEY_BREAK] =      K_F6;
    map_key[KEY_F(7) - KEY_BREAK] =      K_F7;
    map_key[KEY_F(8) - KEY_BREAK] =      K_F8;
    map_key[KEY_F(9) - KEY_BREAK] =      K_F9;
    map_key[KEY_F(10) - KEY_BREAK] =     K_F10;
    map_key[KEY_DL - KEY_BREAK] =        K_DELLN;
    map_key[KEY_IL - KEY_BREAK] =        K_INSLN;
    map_key[KEY_DC - KEY_BREAK] =        K_DEL;
    map_key[KEY_IC - KEY_BREAK] =        K_INS;
    map_key[KEY_EIC - KEY_BREAK] =       K_UNDEF;
    map_key[KEY_CLEAR - KEY_BREAK] =     K_UNDEF;
    map_key[KEY_EOS - KEY_BREAK] =       K_ERASE;
    map_key[KEY_EOL - KEY_BREAK] =       K_ERASE;
    map_key[KEY_SF - KEY_BREAK] =        K_SCRLUP;
    map_key[KEY_SR - KEY_BREAK] =        K_SCRLDN;
    map_key[KEY_NPAGE - KEY_BREAK] =     K_PGDN;
    map_key[KEY_PPAGE - KEY_BREAK] =     K_PGUP;
    map_key[KEY_STAB - KEY_BREAK] =      K_UNDEF;
    map_key[KEY_CTAB - KEY_BREAK] =      K_UNDEF;
    map_key[KEY_CATAB - KEY_BREAK] =     K_UNDEF;
    map_key[KEY_ENTER - KEY_BREAK] =     K_NL;
    map_key[KEY_SRESET - KEY_BREAK] =    K_UNDEF;
    map_key[KEY_RESET - KEY_BREAK] =     K_UNDEF;
    map_key[KEY_PRINT - KEY_BREAK] =     K_UNDEF;
    map_key[KEY_LL - KEY_BREAK] =        K_UNDEF;
    map_key[KEY_A1 - KEY_BREAK] =        K_HOME;
    map_key[KEY_A3 - KEY_BREAK] =        K_PGUP;
    map_key[KEY_B2 - KEY_BREAK] =        K_UNDEF;
    map_key[KEY_C1 - KEY_BREAK] =        K_END;
    map_key[KEY_C3 - KEY_BREAK] =        K_PGDN;
    nonl();
    raw();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    keybd_initialized = TRUE;
  } /* kbd_init */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  {
    booltype result;

  /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      result = TRUE;
    } else {
      if (changes) {
        conFlush();
      } /* if */
      nodelay(stdscr, TRUE);
      last_key = getch();
      if (last_key == ERR) {
        result = FALSE;
      } else {
        key_buffer_filled = TRUE;
        result = TRUE;
      } /* if */
    } /* if */
    return result;
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    chartype result;

  /* kbdGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      key_buffer_filled = FALSE;
      result = last_key;
    } else {
      if (changes) {
        conFlush();
      } /* if */
      nodelay(stdscr, FALSE);
      result = getch();
    } /* if */
    if (result >= KEY_BREAK && result <= KEY_BREAK + 255) {
      result = map_key[result - KEY_BREAK];
      /* The following if-statement is a bug-fix to */
      /* remove carriage-return from function keys */
      /* If the termcap definition contains no cr */
      /* or the function keys send no cr this fix can */
      /* be removed. When no cr follows nothing is done. */
      /* This allows this fix to work also with correct */
      /* function keys. */
      if (result >= K_F1 && result <= K_F10) {
        nodelay(stdscr, TRUE);
        last_key = getch();
        if (last_key != ERR && last_key != '\r') {
          key_buffer_filled = TRUE;
        } /* if */
      } /* if */
    } /* if */
    if (result == 13) {
      result = 10;
    } /* if */
    return result;
  } /* kbdGetc */



#ifdef ANSI_C

chartype kbdRawGetc (void)
#else

chartype kbdRawGetc ()
#endif

  { /* kbdRawGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    return kbdGetc();
  } /* kbdRawGetc */



#ifdef ANSI_C

static void cur_beep (void)
#else

static void cur_beep ()
#endif

  { /* cur_beep */
    fputc('\007', stderr);
    fflush(stderr);
  } /* cur_beep */



#ifdef ANSI_C

static void cur_setcolour (inttype foreground, inttype background)
#else

static void cur_setcolour (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* cur_setcolour */
    if (foreground == black || background == white) {
      attrset(A_STANDOUT);
    } else {
      attrset(0);
    } /* if */
  } /* cur_setcolour */



#ifdef ANSI_C

static void cur_standardcolour (void)
#else

static void cur_standardcolour ()
#endif

  { /* cur_standardcolour */
    cur_setcolour(lightgray,black);
/*  CURR_ATTRIBUTE = 0; */
  } /* cur_standardcolour */



#ifdef ANSI_C

static void cur_normalcolour (void)
#else

static void cur_normalcolour ()
#endif

  { /* cur_normalcolour */
    cur_setcolour(lightgray,black);
/*  CURR_ATTRIBUTE = 0; */
  } /* cur_normalcolour */



#ifdef ANSI_C

static int cur_setfont (char *fontname)
#else

static int cur_setfont (fontname)
char *fontname;
#endif

  { /* cur_setfont */
    return 1;
  } /* cur_setfont */




#ifdef ANSI_C

inttype textheigth (void)
#else

inttype textheigth ()
#endif

  { /* textheigth */
    return 1;
  } /* textheigth */



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
    return stopcol + 1 - startcol;
  } /* textwidth */



#ifdef ANSI_C

void textcolumns (stritype stri, inttype striwidth,
    inttype *cols, inttype *rest)
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

int conHeight (void)
#else

int conHeight ()
#endif

  { /* conHeight */
    return LINES;
  } /* conHeight */



#ifdef ANSI_C

int conWidth (void)
#else

int conWidth ()
#endif

  { /* conWidth */
    return COLS;
  } /* conWidth */



#ifdef ANSI_C

void conFlush (void)
#else

void conFlush ()
#endif

  { /* conFlush */
#ifdef A_UNDERLINE
    idlok(stdscr, scroll_allowed);
#endif
    if (!cursor_on) {
      move(0, 0);
    } /* if */
    refresh();
    scroll_allowed = FALSE;
    changes = FALSE;
  } /* conFlush */



#ifdef ANSI_C

void conCursor (booltype on)
#else

void conCursor (on)
booltype on;
#endif

  { /* conCursor */
    cursor_on = on;
    changes = TRUE;
  } /* conCursor */



#ifdef ANSI_C

void conSetCursor (inttype lin, inttype col)
#else

void conSetCursor (lin, col)
inttype lin;
inttype col;
#endif

  /* Moves the system curser to the given place of the console.     */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  { /* conSetCursor */
    move(lin - 1, col - 1);
    changes = TRUE;
  } /* conSetCursor */



#ifdef ANSI_C

void conText (inttype lin, inttype col, ustritype stri,
memsizetype length)
#else

void conText (lin, col, stri, length)
inttype lin;
inttype col;
ustritype stri;
memsizetype length;
#endif

  /* This function writes the string stri to the console at the     */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the console. All console output     */
  /* must be done with this function.                               */

  {
    inttype pos;

  /* conText */
    move(lin - 1, col - 1);
    for (pos = 0; pos < length; pos++) {
#ifdef MAP_CHARS
      addch(map[stri[pos] & 0xFF]);
#else
      addch(stri[pos]);
#endif
    } /* for */
    changes = TRUE;
  } /* conText */



#ifdef ANSI_C

void conClear (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol)
#else

void conClear (startlin, startcol, stoplin, stopcol)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
#endif

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  {
    int lin, col;

  /* conClear */
    for (lin = startlin - 1; lin < stoplin; lin++) {
      move(lin, startcol - 1);
      if (stopcol == COLS && currentattribute == 0) {
        clrtoeol();
      } else {
        for (col = startcol; col <= stopcol; col++) {
          addch(' ');
        } /* for */
      } /* if */
    } /* for */
    changes = TRUE;
  } /* conClear */



#ifdef ANSI_C

void conUpScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conUpScroll (startlin, startcol, stoplin, stopcol, count)
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
    int number;
    int line;
    int column;
    int ch;

  /* conUpScroll */
    if (startcol != 1 || stopcol != COLS) {
      for (line = startlin - 1; line < stoplin - count; line++) {
        for (column = startcol - 1; column < stopcol; column++) {
          ch = mvinch(line + count, column);
          mvaddch(line, column, ch);
        } /* for */
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        for (column = startcol - 1; column < stopcol; column++) {
          mvaddch(line, column, ' ');
        } /* for */
      } /* for */
    } else {
      move(startlin - 1, 0);
      for (number = 1; number <= count; number++) {
        deleteln();
      } /* for */
      move(stoplin - count, 0);
      for (number = 1; number <= count; number++) {
        insertln();
      } /* for */
      scroll_allowed = TRUE;
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    changes = TRUE;
  } /* conUpScroll */



#ifdef ANSI_C

void conDownScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conDownScroll (startlin, startcol, stoplin, stopcol, count)
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
    int number;
    int line;
    int column;
    int ch;

  /* conDownScroll */
    if (startcol != 1 || stopcol != COLS) {
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        for (column = startcol - 1; column < stopcol; column++) {
          ch = mvinch(line - count, column);
          mvaddch(line, column, ch);
        } /* for */
      } /* for */
      for (line = startlin + count - 2; line >= startlin - 1; line--) {
        for (column = startcol - 1; column < stopcol; column++) {
          mvaddch(line, column, ' ');
        } /* for */
      } /* for */
    } else {
      move(stoplin - count, 0);
      for (number = 1; number <= count; number++) {
        deleteln();
      } /* for */
      move(startlin - 1, 0);
      for (number = 1; number <= count; number++) {
        insertln();
      } /* for */
      scroll_allowed = TRUE;
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    changes = TRUE;
  } /* conDownScroll */



#ifdef ANSI_C

void conLeftScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conLeftScroll (startlin, startcol, stoplin, stopcol, count)
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
    int number;
    int line;

  /* conLeftScroll */
    for (line = startlin - 1; line < stoplin; line++) {
      move(line, startcol - 1);
      for (number = 1; number <= count; number++) {
        delch();
      } /* for */
      move(line, stopcol - count);
      for (number = 1; number <= count; number++) {
        insch(' ');
      } /* for */
    } /* for */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    changes = TRUE;
  } /* conLeftScroll */



#ifdef ANSI_C

void conRightScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conRightScroll (startlin, startcol, stoplin, stopcol, count)
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
    int number;
    int line;

  /* conRightScroll */
    for (line = startlin - 1; line < stoplin; line++) {
      move(line, stopcol - count);
      for (number = 1; number <= count; number++) {
        delch();
      } /* for */
      move(line, startcol - 1);
      for (number = 1; number <= count; number++) {
        insch(' ');
      } /* for */
    } /* for */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    changes = TRUE;
  } /* conRightScroll */



#ifdef ANSI_C

void conShut (void)
#else

void conShut ()
#endif

  { /* conShut */
    if (console_initialized) {
      erase();
      clearok(stdscr, TRUE);
      refresh();
      endwin();
      console_initialized = FALSE;
    } /* if */
  } /* conShut */



#ifdef ANSI_C

int conOpen (void)
#else

int conOpen ()
#endif

  /* Initializes and clears the console.                            */

  {
    int result = 0;

  /* conOpen */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (initscr() != (WINDOW *) ERR) {
      nonl();
      raw();
#ifdef A_UNDERLINE
      idlok(stdscr, FALSE);
#endif
      cur_normalcolour();
      conClear(1, 1, LINES, COLS);
      noecho();
      cbreak();
      keypad(stdscr, TRUE);
      cursor_on = FALSE;
      changes = TRUE;
      console_initialized = TRUE;
      atexit(conShut);
      result = 1;
    } /* if */
    return result;
  } /* conOpen */