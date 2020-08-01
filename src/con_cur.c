/********************************************************************/
/*                                                                  */
/*  con_cur.c     Driver for curses console access.                 */
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
/*  Content: Driver for curses console access.                      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
static boolType key_buffer_filled = FALSE;
static int last_key;
boolType changes = FALSE;
static boolType scroll_allowed = FALSE;
static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;

static boolType keybd_initialized = FALSE;


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



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



static void kbd_init (void)

  {
    int position;

  /* kbd_init */
    logFunction(printf("kbd_init\n"););
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
    atexit(kbdShut);
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



boolType kbdKeyPressed (void)

  {
    boolType result;

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



charType kbdGetc (void)

  {
    charType result;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
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
      /* be removed. If no cr follows nothing is done. */
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
    logFunction(printf("kbdGetc --> %d\n", result););
    return result;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    return kbdGetc();
  } /* kbdRawGetc */



static void cur_beep (void)

  { /* cur_beep */
    fputc('\007', stderr);
    fflush(stderr);
  } /* cur_beep */



static void cur_setcolour (intType foreground, intType background)

  { /* cur_setcolour */
    if (foreground == black || background == white) {
      attrset(A_STANDOUT);
    } else {
      attrset(0);
    } /* if */
  } /* cur_setcolour */



static void cur_standardcolour (void)

  { /* cur_standardcolour */
    cur_setcolour(lightgray,black);
/*  CURR_ATTRIBUTE = 0; */
  } /* cur_standardcolour */



static void cur_normalcolour (void)

  { /* cur_normalcolour */
    cur_setcolour(lightgray,black);
/*  CURR_ATTRIBUTE = 0; */
  } /* cur_normalcolour */



static int cur_setfont (char *fontname)

  { /* cur_setfont */
    return 1;
  } /* cur_setfont */




intType textheigth (void)

  { /* textheigth */
    return 1;
  } /* textheigth */



intType textwidth (striType stri,
    intType startcol, intType stopcol)

  { /* textwidth */
    return stopcol + 1 - startcol;
  } /* textwidth */



void textcolumns (striType stri, intType striwidth,
    intType *cols, intType *rest)

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



int conHeight (void)

  { /* conHeight */
    return LINES;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    return COLS;
  } /* conWidth */



void conFlush (void)

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



void conCursor (boolType on)

  { /* conCursor */
    cursor_on = on;
    changes = TRUE;
  } /* conCursor */



/**
 *  Moves the system cursor to the given place of the console.
 *  If no system cursor exists this procedure can be replaced by
 *  a dummy procedure.
 */
void conSetCursor (intType line, intType column)

  { /* conSetCursor */
    move(line - 1, column - 1);
    changes = TRUE;
  } /* conSetCursor */



/**
 *  This function writes the string stri to the console at the
 *  position (lin, col). The position (lin, col) must be a legal
 *  position of the console. The string stri is not allowed to go
 *  beyond the right border of the console. All console output
 *  must be done with this function.
 */
void conText (intType lin, intType col, ustriType stri,
memSizeType length)

  {
    intType pos;

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



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

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



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by count lines. The upper count lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

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



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by count lines. The lower count lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

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



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by count columns. The left count columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

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



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by count columns. The right count columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

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



void conShut (void)

  { /* conShut */
    logFunction(printf("conShut\n"););
    if (console_initialized) {
      erase();
      clearok(stdscr, TRUE);
      refresh();
      endwin();
      console_initialized = FALSE;
    } /* if */
    logFunction(printf("conShut -->\n"););
  } /* conShut */



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  {
    int result = 0;

  /* conOpen */
    logFunction(printf("conOpen\n"););
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
    logFunction(printf("conOpen -->\n"););
    return result;
  } /* conOpen */
