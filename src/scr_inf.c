/********************************************************************/
/*                                                                  */
/*  scr_inf.c     Driver for terminfo screen access                 */
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
/*  File: seed7/src/scr_inf.c                                       */
/*  Changes: 1994 - 1999  Thomas Mertes                             */
/*  Content: Driver for terminfo screen access                      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "inf_conf.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#ifndef USE_TERMCAP

#ifdef INCL_CURSES_BEFORE_TERM
/* The following includes are necessary for RM Machines. */
#include "curses.h"
#include "termios.h"
#endif

#ifdef INCL_NCURSES_TERM
#include "ncurses/term.h"
#else
#include "term.h"
#endif
#endif

#include "common.h"
#include "trm_drv.h"

#ifdef USE_TERMCAP
#include "cap_def.h"
#endif

#include "scr_drv.h"


/* #define atexit(x) */


#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void *memmove (char *to, char *from, size_t num)
#else

void *memmove (to, from, num)
char *to;
char *from;
size_t num;
#endif

  {
    size_t i;

  /* memmove */
    if (to < from) {
      for (i = 1; i <= num; i++) {
        *to = *from;
        to++;
        from++;
      } /* for */
    } else {
      to = to + (num - 1);
      from = from + (num - 1);
      for (i = 1; i <= num; i++) {
        *to = *from;
        to--;
        from--;
      } /* for */
    } /* if */
  } /* memmove */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C
#ifdef C_PLUS_PLUS

extern "C" int tgetent (char *, char *);
extern "C" int tgetnum (char *);
extern "C" int tgetflag (char *);
extern "C" char *tgetstr(char *, char **);
extern "C" char *tgoto (char *, int, int);
extern "C" int tputs (char *, int, int (*) (char CH));

#else

int tgetent (char *, char *);
int tgetnum (char *);
int tgetflag (char *);
char *tgetstr(char *, char **);
char *tgoto (char *, int, int);
void tputs (char *, int, int (*) (char CH));

#endif
#else

int tgetent ();
int tgetnum ();
int tgetflag ();
char *tgetstr();
char *tgoto ();
int tputs ();

#endif
#endif


#undef TPARM_PRESENT
#undef MAP_CHARS
#define MAP_1_1

#define TEXT_NORMAL 0
#define TEXT_INVERSE 1

static unsigned char curr_attr = TEXT_NORMAL;
booltype changes = FALSE;

static booltype screen_initialized = FALSE;
static booltype cursor_on = FALSE;
static booltype cursor_position_okay = FALSE;
static int cursor_line = 1;
static int cursor_column = 1;


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
#ifdef MAP_1_1
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
/* 250 */ '\372','\373','\374','\375','\376','\377'};
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
/* 160 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 170 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 180 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 190 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 200 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 210 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 220 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 230 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 240 */ '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '?',
/* 250 */ '?',   '?',   '?',   '?',   '?',   '?'};
#endif
#endif
#endif

static unsigned char **whole_screen;
static unsigned char *scrn;
static unsigned char **attributes;
static unsigned char *attr;
static unsigned char *space;



#ifdef ANSI_C

static void downleft (int col, int lin)
#else

static void downleft (col, lin)
int col;
int lin;
#endif

  { /* downleft */
    if (col == 0 && carriage_return != NULL && cursor_down != NULL) {
      putctl(carriage_return); /* cursor return */
      putctl(cursor_down); /* cursor down */
    } else {
      putgoto(cursor_address, col, lin); /* cursor motion */
    } /* if */
  } /* downleft */



#ifdef ANSI_C

static void inf_beep (void)
#else

static void inf_beep ()
#endif

  { /* inf_beep */
    fputc('\007', stderr);
    fflush(stderr);
  } /* inf_beep */



#ifdef ANSI_C

static void setattr (unsigned char attribute)
#else

static void setattr (attribute)
unsigned char attribute;
#endif

  { /* setattr */
    if (attribute == TEXT_NORMAL) {
      putctl(exit_standout_mode);
    } else if (attribute == TEXT_INVERSE) {
      if (enter_reverse_mode != NULL) {
        putctl(enter_reverse_mode);
      } else {
        putctl(enter_standout_mode);
      } /* if */
    } /* if */
  } /* setattr */



#ifdef ANSI_C

static void inf_setcolor (inttype foreground, inttype background)
#else

static void inf_setcolor (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* inf_setcolor */
    if (foreground == black || background == white) {
      curr_attr = TEXT_INVERSE;
    } else {
      curr_attr = TEXT_NORMAL;
    } /* if */
  } /* inf_setcolor */



#ifdef ANSI_C

static void inf_standardcolour (void)
#else

static void inf_standardcolour ()
#endif

  { /* inf_standardcolour */
    inf_setcolor(lightgray,black);
/*  curr_attribute = 0; */
  } /* inf_standardcolour */



#ifdef ANSI_C

void inf_normalcolour (void)
#else

void inf_normalcolour ()
#endif

  { /* inf_normalcolour */
    inf_setcolor(lightgray,black);
/*  curr_attribute = 0; */
  } /* inf_normalcolour */



#ifdef ANSI_C

static int inf_setfont (char *fontname)
#else

static int inf_setfont (fontname)
char *fontname;
#endif

  { /* inf_setfont */
    return(1);
  } /* inf_setfont */



#ifdef ANSI_C

inttype inf_textheight (void)
#else

inttype inf_textheight ()
#endif

  { /* inf_textheight */
    return(1);
  } /* inf_textheight */



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
    return(lines);
  } /* scrHeight */



#ifdef ANSI_C

int scrWidth (void)
#else

int scrWidth ()
#endif

  { /* scrWidth */
    return(columns);
  } /* scrWidth */



#ifdef ANSI_C

void scrFlush (void)
#else

void scrFlush ()
#endif

  { /* scrFlush */
    if (!cursor_on) {
      if (cursor_invisible == NULL) {
        putgoto(cursor_address, 0, 0); /* cursor motion */
        cursor_position_okay = FALSE;
      } /* if */
    } else {
      if (!cursor_position_okay) {
        putgoto(cursor_address,
            cursor_column - 1, cursor_line - 1); /* cursor motion */
        cursor_position_okay = TRUE;
      } /* if */
    } /* if */
    fflush(stdout);
    /* fsync(fileno(stdout)); */
    changes = FALSE;
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
      putctl(cursor_normal); /* cursor normal */
    } else {
      putctl(cursor_invisible); /* makes cursor invisible */
    } /* if */
    changes = TRUE;
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
    if (cursor_line != lin || cursor_column != col) {
      cursor_position_okay = FALSE;
      cursor_line = lin;
      cursor_column = col;
    } /* if */
/*  putgoto(cursor_address, col - 1, lin - 1); cursor motion */
    changes = TRUE;
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
    int start_pos;
    int end_pos;
    int position;
    unsigned char *new_line;
    unsigned char *new_attr;

  /* scrText */
    if (lin <= lines) {
      new_line = &whole_screen[lin - 1][col - 1];
      new_attr = &attributes[lin - 1][col - 1];
      if (col + length - 1 <= columns) {
        end_pos = length - 1;
        while (end_pos >= 0 &&
            new_line[end_pos] == stri[end_pos] &&
            new_attr[end_pos] == curr_attr) {
          end_pos--;
        } /* while */
        if (end_pos >= 0) {
          start_pos = 0;
          while (start_pos <= end_pos &&
              new_line[start_pos] == stri[start_pos] &&
              new_attr[start_pos] == curr_attr) {
            start_pos++;
          } /* while */
          if (start_pos <= end_pos) {
#ifdef MAP_CHARS
            for (position = 0; position <= end_pos - start_pos; position++) {
              new_line[start_pos + position] = map[stri[start_pos + position]];
            } /* for */
#else
            memcpy(&new_line[start_pos], &stri[start_pos],
                end_pos - start_pos + 1);
#endif
            if (cursor_position_okay &&
                cursor_line == lin && cursor_column == col) {
              start_pos = 0;
            } else {
              putgoto(cursor_address, col + start_pos - 1, lin - 1); /* cursor motion */
            } /* if */
            if (ceol_standout_glitch) {
              for (position = 0; position <= end_pos - start_pos; position++) {
                if (new_attr[start_pos + position] != curr_attr) {
                  setattr(curr_attr);
                } /* if */
                fputc(new_line[start_pos + position], stdout);
              } /* for */
              if (col + end_pos < columns && new_attr[end_pos + 1] != curr_attr) {
                setattr(new_attr[end_pos + 1]);
              } /* if */
            } else {
              if (curr_attr != TEXT_NORMAL) {
                setattr(curr_attr);
              } /* if */
              fwrite(&new_line[start_pos], 1,
                  (SIZE_TYPE) (end_pos - start_pos + 1), stdout);
              if (curr_attr != TEXT_NORMAL) {
                setattr(TEXT_NORMAL);
              } /* if */
            } /* if */
            memset(&new_attr[start_pos], curr_attr,
                (SIZE_TYPE) (end_pos - start_pos + 1));
            cursor_position_okay = TRUE;
            cursor_line = lin;
            cursor_column = col + end_pos + 1;
          } /* if */
        } /* if */
      } else {
        if (col <= columns) {
#ifdef MAP_CHARS
          for (position = 0; position <= columns - col; position++) {
            new_line[position] = map[stri[position]];
          } /* for */
#else
          memcpy(new_line, stri, columns - col + 1);
#endif
          if (ceol_standout_glitch) {
            for (position = 0; position <= columns - col; position++) {
              if (new_attr[position] != curr_attr) {
                setattr(curr_attr);
              } /* if */
              fputc(new_line[position], stdout);
            } /* for */
          } else {
            putgoto(cursor_address, col - 1, lin - 1); /* cursor motion */
            if (curr_attr != TEXT_NORMAL) {
              setattr(curr_attr);
            } /* if */
            fwrite(new_line, 1, (SIZE_TYPE) (columns - col + 1), stdout);
            if (curr_attr != TEXT_NORMAL) {
              setattr(TEXT_NORMAL);
            } /* if */
          } /* if */
          memset(new_attr, curr_attr, (SIZE_TYPE) (columns - col + 1));
        } /* if */
      } /* if */
      changes = TRUE;
    } /* if */
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
    int lin;
    int column;
    unsigned char *new_line;
    unsigned char *new_attr;

  /* scrClear */
    if (stoplin > lines) {
      stoplin = lines;
    } /* if */
    if (stopcol > columns) {
      stopcol = columns;
    } /* if */
    if (startlin == 1 && stoplin == lines &&
        startcol == 1 && stopcol == columns && clear_screen != NULL) {
      putctl(clear_screen); /* clear screen */
      memset(scrn, ' ', (SIZE_TYPE) (lines * columns));
      memset(attr, curr_attr, (SIZE_TYPE) (lines * columns));
    } else {
      if (stopcol == columns && clr_eol != NULL) {
        putgoto(cursor_address, startcol - 1, startlin - 1); /* cursor motion */
        for (lin = startlin - 1; lin < stoplin; lin++) {
          if (curr_attr != TEXT_NORMAL) {
            setattr(curr_attr);
          } /* if */
          putctl(clr_eol); /* clear to end of line */
          if (curr_attr != TEXT_NORMAL) {
            setattr(TEXT_NORMAL);
          } /* if */
          memset(&whole_screen[lin][startcol - 1], ' ',
              (SIZE_TYPE) (stopcol - startcol + 1));
          memset(&attributes[lin][startcol - 1], curr_attr,
              (SIZE_TYPE) (stopcol - startcol + 1));
          if (lin < stoplin - 1) {
            downleft(startcol - 1, lin + 1);
          } /* if */
        } /* for */
      } else {
        for (lin = startlin - 1; lin < stoplin; lin++) {
          column = stopcol;
          new_line = whole_screen[lin];
          new_attr = attributes[lin];
          while (column >= startcol &&
              new_line[column - 1] == ' ' &&
              new_attr[column - 1] == curr_attr) {
            column--;
          } /* while */
          if (column >= startcol) {
            putgoto(cursor_address, startcol - 1, lin); /* cursor motion */
            if (curr_attr != TEXT_NORMAL) {
              setattr(curr_attr);
            } /* if */
            fwrite(space, 1, (SIZE_TYPE) (column - startcol + 1), stdout);
            if (curr_attr != TEXT_NORMAL) {
              setattr(TEXT_NORMAL);
            } /* if */
            memset(&new_line[startcol - 1], ' ',
                (SIZE_TYPE) (column - startcol + 1));
            memset(&new_attr[startcol - 1], curr_attr,
                (SIZE_TYPE) (column - startcol + 1));
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    cursor_position_okay = FALSE;
    changes = TRUE;
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
    int number;
    int line;
    int column;
    unsigned char *old_line;
    unsigned char *old_attr;
    unsigned char *new_line;
    unsigned char *new_attr;

  /* scrUpScroll */
    if (count > stoplin - startlin + 1) {
      count = stoplin - startlin + 1;
    } /* if */
    if ((insert_line == NULL && parm_insert_line == NULL) ||
        (delete_line == NULL && parm_delete_line == NULL) ||
        startcol != 1 || stopcol != columns) {
      for (line = startlin - 1; line < stoplin - count; line++) {
        column = stopcol;
        new_line = whole_screen[line];
        new_attr = attributes[line];
        old_line = whole_screen[line + count];
        old_attr = attributes[line + count];
        while (column >= startcol &&
            new_line[column - 1] == old_line[column - 1] &&
            new_attr[column - 1] == old_attr[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              (SIZE_TYPE) (column - startcol + 1));
          memcpy(&new_attr[startcol - 1],
              &old_attr[startcol - 1],
              (SIZE_TYPE) (column - startcol + 1));
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          fwrite(&new_line[startcol - 1], 1,
              (SIZE_TYPE) (column - startcol + 1), stdout);
        } /* if */
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        column = stopcol;
        new_line = whole_screen[line];
        new_attr = attributes[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ' &&
            new_attr[column - 1] == curr_attr) {
          column--;
        } /* while */
        if (column >= startcol) {
          memset(&new_line[startcol - 1], ' ',
              (SIZE_TYPE) (column - startcol + 1));
          memset(&new_attr[startcol - 1], curr_attr,
              (SIZE_TYPE) (column - startcol + 1));
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          if (curr_attr != TEXT_NORMAL) {
            setattr(curr_attr);
          } /* if */
          fwrite(space, 1, (SIZE_TYPE) (column - startcol + 1), stdout);
          if (curr_attr != TEXT_NORMAL) {
            setattr(TEXT_NORMAL);
          } /* if */
        } /* if */
      } /* for */
    } else {
      putgoto(cursor_address, 0, startlin - 1); /* cursor motion */
#ifdef TPARM_PRESENT
      if (parm_delete_line != NULL) {
        putctl(tparm(parm_delete_line, count));
      } else {
#endif
        for (number = 1; number <= count; number++) {
          putctl(delete_line); /* delete line */
        } /* for */
#ifdef TPARM_PRESENT
      } /* if */
#endif
      putgoto(cursor_address, 0, stoplin - count); /* cursor motion */
#ifdef TPARM_PRESENT
      if (parm_insert_line != NULL) {
        putctl(tparm(parm_insert_line, count));
      } else {
#endif
        for (number = 1; number <= count; number++) {
          putctl(insert_line); /* insert line */
        } /* for */
#ifdef TPARM_PRESENT
      } /* if */
#endif
      for (line = startlin - 1; line < stoplin - count; line++) {
        memcpy(&whole_screen[line][startcol - 1],
            &whole_screen[line + count][startcol - 1],
            (SIZE_TYPE) (stopcol - startcol + 1));
        memcpy(&attributes[line][startcol - 1],
            &attributes[line + count][startcol - 1],
            (SIZE_TYPE) (stopcol - startcol + 1));
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        memset(&whole_screen[line][startcol - 1], ' ',
            (SIZE_TYPE) (stopcol - startcol + 1));
        memset(&attributes[line][startcol - 1], curr_attr,
            (SIZE_TYPE) (stopcol - startcol + 1));
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    scrFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
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
    int number;
    int line;
    int column;
    unsigned char *old_line;
    unsigned char *old_attr;
    unsigned char *new_line;
    unsigned char *new_attr;

  /* scrDownScroll */
    if (count > stoplin - startlin + 1) {
      count = stoplin - startlin + 1;
    } /* if */
    if ((insert_line == NULL && parm_insert_line == NULL) ||
        (delete_line == NULL && parm_delete_line == NULL) ||
        startcol != 1 || stopcol != columns) {
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        column = stopcol;
        new_line = whole_screen[line];
        new_attr = attributes[line];
        old_line = whole_screen[line - count];
        old_attr = attributes[line - count];
        while (column >= startcol &&
            new_line[column - 1] == old_line[column - 1] &&
            new_attr[column - 1] == old_attr[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          fwrite(&old_line[startcol - 1], 1,
              (SIZE_TYPE) (column - startcol + 1), stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              (SIZE_TYPE) (column - startcol + 1));
          memcpy(&new_attr[startcol - 1],
              &old_attr[startcol - 1],
              (SIZE_TYPE) (column - startcol + 1));
        } /* if */
      } /* for */
      for (line = startlin + count - 2; line >= startlin - 1; line--) {
        column = stopcol;
        new_line = whole_screen[line];
        new_attr = attributes[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ' &&
            new_attr[column - 1] == curr_attr) {
          column--;
        } /* while */
        if (column >= startcol) {
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          fwrite(space, 1, (SIZE_TYPE) (column - startcol + 1), stdout);
          memset(&new_line[startcol - 1], ' ',
              (SIZE_TYPE) (column - startcol + 1));
          memset(&new_attr[startcol - 1], curr_attr,
              (SIZE_TYPE) (column - startcol + 1));
        } /* if */
      } /* for */
    } else {
    /* fprintf(stderr, "scrDownScroll: lin1=%d col1=%d lin*=%d col*=%d count=%d\n",
           startlin, startcol, stoplin, stopcol, count); */
      putgoto(cursor_address, 0, stoplin - count); /* cursor motion */
#ifdef TPARM_PRESENT
      if (parm_delete_line != NULL) {
        putctl(tparm(parm_delete_line, count));
      } else {
#endif
        for (number = 1; number <= count; number++) {
          putctl(delete_line); /* delete line */
        } /* for */
#ifdef TPARM_PRESENT
      } /* if */
#endif
      putgoto(cursor_address, 0, startlin - 1); /* cursor motion */
#ifdef TPARM_PRESENT
      if (parm_insert_line != NULL) {
        putctl(tparm(parm_insert_line, count));
      } else {
#endif
        for (number = 1; number <= count; number++) {
          putctl(insert_line); /* insert line */
        } /* for */
#ifdef TPARM_PRESENT
      } /* if */
#endif
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        memcpy(&whole_screen[line][startcol - 1],
            &whole_screen[line - count][startcol - 1],
            (SIZE_TYPE) (stopcol - startcol + 1));
        memcpy(&attributes[line][startcol - 1],
            &attributes[line - count][startcol - 1],
            (SIZE_TYPE) (stopcol - startcol + 1));
      } /* for */
/*    for (line = startlin + count - 2; line >= startlin - 1; line--) { */
      for (line = startlin - 1; line < startlin + count - 1; line++) {
        memset(&whole_screen[line][startcol - 1], ' ',
            (SIZE_TYPE) (stopcol - startcol + 1));
        memset(&attributes[line][startcol - 1], curr_attr,
            (SIZE_TYPE) (stopcol - startcol + 1));
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    scrFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
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
    int number;
    int line;
    int start_pos;
    int end_pos;
    unsigned char *new_line;
    unsigned char *old_line;

  /* scrLeftScroll */
    if (0 && delete_character != NULL && (insert_character != NULL ||
        (enter_insert_mode != NULL && exit_insert_mode != NULL))) {
      putgoto(cursor_address, startcol - 1, startlin - 1); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = whole_screen[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol - 1], &new_line[startcol + count - 1],
            (SIZE_TYPE) (stopcol - startcol - count + 1));
        if (line < stoplin - 1) {
          if (cursor_down != NULL) {
            putctl(cursor_down); /* cursor down */
          } else {
            putgoto(cursor_address, startcol - 1, line + 1); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      if (insert_character != NULL) {
        putgoto(cursor_address, stopcol - count, startlin - 1); /* cursor motion */
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(insert_character); /* insert character */
          } /* for */
          memset(&whole_screen[line][stopcol - count], ' ', (SIZE_TYPE) count);
          if (line < stoplin - 1) {
            if (cursor_down != NULL) {
              putctl(cursor_down); /* cursor down */
            } else {
              putgoto(cursor_address, stopcol - count, line + 1); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (move_insert_mode) { /* safe to move while in insert mode */
          putctl(enter_insert_mode); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            putgoto(cursor_address, stopcol - count, line); /* cursor motion */
            fwrite(space, 1, (SIZE_TYPE) count, stdout);
            memset(&whole_screen[line][stopcol - count], ' ', (SIZE_TYPE) count);
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putgoto(cursor_address, stopcol - count, line); /* cursor motion */
            putctl(enter_insert_mode); /* enter insert mode */
            fwrite(space, 1, (SIZE_TYPE) count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            memset(&whole_screen[line][stopcol - count], ' ', (SIZE_TYPE) count);
          } /* for */
        } /* if */
      } /* if */
    } else {
      for (line = startlin - 1; line < stoplin; line++) {
        start_pos = 0;
        end_pos = stopcol - startcol - count;
        new_line = &whole_screen[line][startcol - 1];
        old_line = &whole_screen[line][startcol + count - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putgoto(cursor_address, startcol + start_pos - 1, line); /* cursor motion */
          fwrite(&old_line[start_pos], 1,
              (SIZE_TYPE) (end_pos - start_pos + 1), stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              (SIZE_TYPE) (end_pos - start_pos + 1));
        } /* if */
        start_pos = 0;
        new_line = &whole_screen[line][stopcol - count];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putgoto(cursor_address, stopcol - count + start_pos, line); /* cursor motion */
          fwrite(space, 1, (SIZE_TYPE) (count - start_pos), stdout);
          memset(&new_line[start_pos], ' ', (SIZE_TYPE) (count - start_pos));
        } /* if */
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    scrFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
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
    int number;
    int line;
    int start_pos;
    int end_pos;
    unsigned char *new_line;
    unsigned char *old_line;

  /* scrRightScroll */
    if (0 && delete_character != NULL && (insert_character != NULL ||
        (enter_insert_mode != NULL && exit_insert_mode != NULL))) {
      putgoto(cursor_address, stopcol - count, startlin - 1); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = whole_screen[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol + count - 1], &new_line[startcol - 1],
            (SIZE_TYPE) (stopcol - startcol - count + 1));
        if (line < stoplin - 1) {
          if (cursor_down != NULL) {
            putctl(cursor_down); /* cursor down */
          } else {
            putgoto(cursor_address, stopcol - count, line + 1); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      putgoto(cursor_address, startcol - 1, startlin - 1); /* cursor motion */
      if (insert_character != NULL) {
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(insert_character); /* insert character */
          } /* for */
          memset(&whole_screen[line][startcol - 1], ' ', (SIZE_TYPE) count);
          if (line < stoplin - 1) {
            if (cursor_down != NULL) {
              putctl(cursor_down); /* cursor down */
            } else {
              putgoto(cursor_address, startcol - 1, line + 1); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (move_insert_mode) { /* safe to move while in insert mode */
          putctl(enter_insert_mode); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            fwrite(space, 1, (SIZE_TYPE) count, stdout);
            memset(&whole_screen[line][startcol - 1], ' ', (SIZE_TYPE) count);
            if (line < stoplin - 1) {
              downleft(startcol - 1, line + 1);
            } /* if */
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(enter_insert_mode); /* enter insert mode */
            fwrite(space, 1, (SIZE_TYPE) count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            memset(&whole_screen[line][startcol - 1], ' ', (SIZE_TYPE) count);
            if (line < stoplin - 1) {
              downleft(startcol - 1, line + 1);
            } /* if */
          } /* for */
        } /* if */
      } /* if */
    } else {
      for (line = startlin - 1; line < stoplin; line++) {
        start_pos = 0;
        end_pos = stopcol - startcol - count;
        new_line = &whole_screen[line][startcol + count - 1];
        old_line = &whole_screen[line][startcol - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putgoto(cursor_address, startcol + count + start_pos - 1, line); /* cursor motion */
          fwrite(&old_line[start_pos], 1,
              (SIZE_TYPE) (end_pos - start_pos + 1), stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              (SIZE_TYPE) (end_pos - start_pos + 1));
        } /* if */
        start_pos = 0;
        new_line = &whole_screen[line][startcol - 1];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putgoto(cursor_address, startcol + start_pos - 1, line); /* cursor motion */
          fwrite(space, 1, (SIZE_TYPE) (count - start_pos), stdout);
          memset(&new_line[start_pos], ' ', (SIZE_TYPE) (count - start_pos));
        } /* if */
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    scrFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* scrRightScroll */



#ifdef ANSI_C

void scrShut (void)
#else

void scrShut ()
#endif

  { /* scrShut */
    if (screen_initialized) {
      putgoto(cursor_address, 0, lines - 1); /* cursor motion */
      putctl(cursor_normal); /* cursor normal */
      putctl(clear_screen); /* clear screen */
      putctl(exit_ca_mode); /* end cursor addressing mode */
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
    int line;
    int result = 0;

  /* scrOpen */
#ifdef TRACE_INIT
    printf("BEGIN scrOpen\n");
    fflush(stdout);
#endif
    if (getcaps()) {
/*    printf("lines: %d  columns: %d\n", lines, columns); */
      if (lines == -1) {
        lines = 24;
      } /* if */
      if (columns == -1) {
        columns = 80;
      } /* if */
      scrn = (unsigned char *) malloc(lines * columns * sizeof(unsigned char));
      whole_screen = (unsigned char **) malloc (lines * sizeof(unsigned char *));
      attr = (unsigned char *) malloc(lines * columns * sizeof(unsigned char));
      attributes = (unsigned char **) malloc (lines * sizeof(unsigned char *));
      space = (unsigned char *) malloc(columns * sizeof(unsigned char));
      if (scrn != NULL && whole_screen != NULL && space != NULL) {
        memset(scrn, ' ', (SIZE_TYPE) (lines * columns));
        for (line = 0; line < lines; line++) {
          whole_screen[line] = &scrn[line * columns];
        } /* for */
        memset(attr, ' ', (SIZE_TYPE) (lines * columns));
        for (line = 0; line < lines; line++) {
          attributes[line] = &attr[line * columns];
        } /* for */
        memset(space, ' ', (SIZE_TYPE) columns);
        putctl(enter_ca_mode); /* enter cursor addressing mode */
        putctl(cursor_invisible); /* makes cursor invisible */
        putctl(clear_screen); /* clear screen */
        cursor_on = FALSE;
        inf_normalcolour();
        cursor_position_okay = FALSE;
        cursor_line = 1;
        cursor_column = 1;
        changes = TRUE;
        screen_initialized = TRUE;
        atexit(scrShut);
        result = 1;
      } /* if */
    } /* if */
#ifdef TRACE_INIT
    printf("END scrOpen\n");
    fflush(stdout);
#endif
    return(result);
  } /* scrOpen */
