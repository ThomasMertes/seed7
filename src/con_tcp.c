/********************************************************************/
/*                                                                  */
/*  con_tcp.c     Driver for termcap console access.                */
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
/*  File: seed7/src/con_tcp.c                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver for termcap console access.                     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "termios.h"

#include "common.h"
#include "os_decls.h"
#include "con_drv.h"



#ifdef OUT_OF_ORDER
void *memmove (char *to, char *from, size_t num)

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



int tgetent (char *, char *);
int tgetnum (char *);
int tgetflag (char *);
char *tgetstr(char *, char **);
char *tgoto (char *, int, int);
int tputs (char *, int, int (*) (char ch));

#define CAPBUF_SIZE 2048

#define MAP_CHARS

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

/* int curr_attribute = 0; */
static boolType key_buffer_filled = FALSE;
static int last_key;
static struct termios term_descr;
static tcflag_t lflag_bak;
static cc_t min_bak;
static cc_t time_bak;
static cc_t erase_ch;

boolType changes = FALSE;
boolType keybd_initialized = FALSE;
boolType console_initialized = FALSE;
boolType cursor_on = FALSE;


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


#define putctln(s,n) (s != NULL ? tputs(s, n, outch) : 0)
#define putctl(s) putctln(s, 1)
#define putstr(s) fputs(s, stdout)

char *AL;    /* insert line */
boolType AM; /* automatic margins (warp around) */
char *BC;    /* backspace (if not BS) */
char *CD;    /* clear to end of display */
char *CE;    /* clear to end of line */
char *CL;    /* clear screen */
int CO;      /* number of columns */
char *CM;    /* cursor motion */
char *CR;    /* cursor return */
char *DC;    /* delete character */
char *DL;    /* delete line */
char *DM;    /* enter delete mode */
char *DO;    /* cursor down */
char *ED;    /* end delete mode */
char *EI;    /* end insert mode */
boolType HC; /* hardcopy terminal */
char *IC;    /* insert character */
char *IM;    /* enter insert mode */
char *IP;    /* post-insert mode */
char *LE;    /* cursor left */
int LI;      /* number of lines */
boolType MI; /* safe to move while in insert mode */
char *ND;    /* cursor right */
char PC;     /* pad character */
char *PCS;   /* pad string */
char *SC;    /* show cursor */
char *SE;    /* end standout mode */
char *SO;    /* enter standout mode */
char *TE;    /* end cursor addressing mode */
char *TI;    /* enter cursor addressing mode */
char *UP;    /* cursor up */
char *VE;    /* end visual mode */
char *VS;    /* enter visual mode */
char *ZC;    /* zap cursor (hide cursor) */

static unsigned char **SCREEN;
static unsigned char *SCRN;
static unsigned char *SPACE;



static void getcaps (void)

  {
    static char capbuf[CAPBUF_SIZE];
    char *area;

  /* getcaps */
    tgetent(capbuf, getenv("TERM"));
    area = capbuf;
    AL = tgetstr("al", &area);
    AM = tgetflag("am");
    BC = tgetstr("bc", &area);
    CD = tgetstr("cd", &area);
    CE = tgetstr("ce", &area);
    CL = tgetstr("cl", &area);
    CO = tgetnum("co");
    CM = tgetstr("cm", &area);
    CR = tgetstr("cr", &area);
    DC = tgetstr("dc", &area);
    DL = tgetstr("dl", &area);
    DM = tgetstr("dm", &area);
    DO = tgetstr("do", &area);
    ED = tgetstr("ed", &area);
    EI = tgetstr("ei", &area);
    HC = tgetflag("hc");
    IC = tgetstr("ic", &area);
    IM = tgetstr("im", &area);
    IP = tgetstr("ip", &area);
    LE = tgetstr("le", &area);
    LI = tgetnum("li");
    MI = tgetflag("mi");
    ND = tgetstr("nd", &area);
    PCS = tgetstr("pc", &area);
    SC = tgetstr("sc", &area);
    SE = tgetstr("se", &area);
    SO = tgetstr("so", &area);
    TE = tgetstr("te", &area);
    TI = tgetstr("ti", &area);
    UP = tgetstr("up", &area);
    VE = tgetstr("ve", &area);
    VS = tgetstr("vs", &area);
    ZC = tgetstr("zc", &area);
    LE = tgetstr("le", &area);
  } /* getcaps */



static int outch (char ch)

  { /* outch */
    return putchar(ch);
  } /* outch */



static void downleft (int col, int lin)

  { /* downleft */
    if (col == 0 && CR != NULL && DO != NULL) {
      putctl(CR); /* cursor return */
      putctl(DO); /* cursor down */
    } else {
      putctl(tgoto(CM, col, lin)); /* cursor motion */
    } /* if */
  } /* downleft */



void kbdShut (void)

  { /* kbdShut */
    if (keybd_initialized) {
      term_descr.c_lflag = lflag_bak;
      term_descr.c_cc[VMIN] = min_bak;
      term_descr.c_cc[VTIME] = time_bak;
      tcsetattr(os_fileno(stdin), TCSANOW, &term_descr);
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



static void kbd_init (void)

  {
    int file_no;

  /* kbd_init */
    logFunction(printf("kbd_init\n"););
    file_no = os_fileno(stdin);
    tcgetattr(file_no, &term_descr);
    lflag_bak = term_descr.c_lflag;
    min_bak = term_descr.c_cc[VMIN];
    time_bak = term_descr.c_cc[VTIME];
    erase_ch = term_descr.c_cc[VERASE];
    term_descr.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    term_descr.c_cc[VMIN] = 1;
    term_descr.c_cc[VTIME] = 0;
    tcsetattr(file_no, TCSANOW, &term_descr);
    keybd_initialized = TRUE;
    os_atexit(kbdShut);
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



boolType kbdInputReady (void)

  {
    int file_no;
    boolType result;
    char buffer;

  /* kbdInputReady */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      result = TRUE;
    } else {
      file_no = os_fileno(stdin);
      term_descr.c_cc[VMIN] = 0;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(file_no, TCSANOW, &term_descr);
      if (fread(&buffer, 1, 1, stdin) == 1) {
        result = TRUE;
        last_key = buffer;
        key_buffer_filled = TRUE;
      } else {
        result = FALSE;
      } /* if */
      term_descr.c_cc[VMIN] = 1;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(file_no, TCSANOW, &term_descr);
    } /* if */
    return result;
  } /* kbdInputReady */



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
      result = getc(stdin);
    } /* if */
    if (result == erase_ch) {
      result = '\b';
    } /* if */
    logFunction(printf("kbdGetc --> %d\n", result););
    return result;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    return kbdGetc();
  } /* kbdRawGetc */



void snd_beep (void)

  { /* snd_beep */
    fputc('\007', stderr);
    fflush(stderr);
  } /* snd_beep */



void setcolour (intType foreground, intType background)

  { /* setcolour */
/*  curr_attribute = (char) (foreground + 16 * (background % 8)); */
  } /* setcolour */



void standardcolour (void)

  { /* standardcolour */
    setcolour(lightgray,black);
/*  curr_attribute = 0; */
  } /* standardcolour */



void normalcolour (void)

  { /* normalcolour */
    setcolour(lightgray,black);
/*  curr_attribute = 0; */
  } /* normalcolour */



void setfont (char *fontname)

  { /* setfont */
  } /* setfont */



intType textheight (void)

  { /* textheight */
    return 1;
  } /* textheight */



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
    return LI;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    return CO;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
    cursor_on = on;
    changes = TRUE;
  } /* conCursor */



/**
 *  Moves the system cursor to the given place of the console.
 *  If no system cursor exists this function can be replaced by
 *  a dummy function.
 */
void conSetCursor (intType line, intType column)

  { /* conSetCursor */
    putctl(tgoto(CM, column - 1, line - 1)); /* cursor motion */
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
    int start_pos;
    int end_pos;
    unsigned char *new_line;

  /* conText */
    if (col + length - 1 <= CO) {
      end_pos = length - 1;
      new_line = &SCREEN[lin - 1][col - 1];
      while (end_pos >= 0 && new_line[end_pos] == stri[end_pos]) {
        end_pos--;
      } /* while */
      if (end_pos >= 0) {
        start_pos = 0;
        while (start_pos <= end_pos && new_line[start_pos] == stri[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          putctl(tgoto(CM, col + start_pos - 1, lin - 1)); /* cursor motion */
          fwrite(&stri[start_pos], 1, end_pos - start_pos + 1, stdout);
          memcpy(new_line, stri, end_pos + 1);
        } /* if */
      } /* if */
    } else {
      if (col <= CO) {
        putctl(tgoto(CM, col - 1, lin - 1)); /* cursor motion */
        fwrite(stri, 1, CO - col + 1, stdout);
        memcpy(&SCREEN[lin - 1][col - 1], stri, CO - col + 1);
      } /* if */
    } /* if */
    if (!cursor_on && ZC == NULL) {
      putctl(tgoto(CM, 0, 0)); /* cursor motion */
    } /* if */
    fflush(stdout);
    changes = TRUE;
  } /* conText */



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  {
    int lin;
    int column;
    unsigned char *new_line;

  /* conClear */
    if (startlin == 1 && stoplin == LI &&
        startcol == 1 && stopcol == CO && CL != NULL) {
      putctl(CL); /* clear screen */
      memset(SCRN, ' ', LI * CO);
    } else {
      if (stopcol == CO && CE != NULL) {
        putctl(tgoto(CM, startcol - 1, startlin - 1)); /* cursor motion */
        for (lin = startlin - 1; lin < stoplin; lin++) {
          putctl(CE); /* clear to end of line */
          memset(&SCREEN[lin][startcol - 1], ' ', stopcol - startcol + 1);
          if (lin < stoplin - 1) {
            downleft(startcol - 1, lin + 1);
          } /* if */
        } /* for */
      } else {
        for (lin = startlin - 1; lin < stoplin; lin++) {
          column = stopcol;
          new_line = SCREEN[lin];
          while (column >= startcol && new_line[column - 1] == ' ') {
            column--;
          } /* while */
          if (column >= startcol) {
            putctl(tgoto(CM, startcol - 1, lin)); /* cursor motion */
            fwrite(SPACE, 1, column - startcol + 1, stdout);
            memset(&new_line[startcol - 1], ' ', column - startcol + 1);
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    fflush(stdout);
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
    unsigned char *old_line;
    unsigned char *new_line;

  /* conUpScroll */
    if (AL == NULL || DL == NULL || startcol != 1 || stopcol != CO) {
      for (line = startlin - 1; line < stoplin - count; line++) {
        column = stopcol;
        new_line = SCREEN[line];
        old_line = SCREEN[line + count];
        while (column >= startcol && new_line[column - 1] ==
            old_line[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(CM, startcol - 1, line)); /* cursor motion */
          fwrite(&old_line[startcol - 1], 1,
              column - startcol + 1, stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              column - startcol + 1);
        } /* if */
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        column = stopcol;
        new_line = SCREEN[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ') {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(CM, startcol - 1, line)); /* cursor motion */
          fwrite(SPACE, 1, column - startcol + 1, stdout);
          memset(&new_line[startcol - 1], ' ',
              column - startcol + 1);
        } /* if */
      } /* for */
    } else {
      putctl(tgoto(CM, 0, startlin - 1)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(DL); /* delete line */
      } /* for */
      putctl(tgoto(CM, 0, stoplin - count)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(AL); /* insert line */
      } /* for */
      for (line = startlin - 1; line < stoplin - count; line++) {
        memcpy(&SCREEN[line][startcol - 1],
            &SCREEN[line + count][startcol - 1],
            stopcol - startcol + 1);
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        memset(&SCREEN[line][startcol - 1], ' ',
            stopcol - startcol + 1);
      } /* for */
    } /* if */
    fflush(stdout);
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
    unsigned char *old_line;
    unsigned char *new_line;

  /* conDownScroll */
    if (AL == NULL || DL == NULL || startcol != 1 || stopcol != CO) {
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        column = stopcol;
        new_line = SCREEN[line];
        old_line = SCREEN[line - count];
        while (column >= startcol && new_line[column - 1] ==
            old_line[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(CM, startcol - 1, line)); /* cursor motion */
          fwrite(&old_line[startcol - 1], 1,
              column - startcol + 1, stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              column - startcol + 1);
        } /* if */
      } /* for */
      for (line = startlin + count - 2; line >= startlin - 1; line--) {
        column = stopcol;
        new_line = SCREEN[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ') {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(CM, startcol - 1, line)); /* cursor motion */
          fwrite(SPACE, 1, column - startcol + 1, stdout);
          memset(&new_line[startcol - 1], ' ',
              column - startcol + 1);
        } /* if */
      } /* for */
    } else {
      putctl(tgoto(CM, 0, stoplin - count)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(DL); /* delete line */
      } /* for */
      putctl(tgoto(CM, 0, startlin - 1)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(AL); /* insert line */
      } /* for */
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        memcpy(&SCREEN[line][startcol - 1],
            &SCREEN[line - count][startcol - 1],
            stopcol - startcol + 1);
      } /* for */
/*    for (line = startlin + count - 2; line >= startlin - 1; line--) { */
      for (line = startlin - 1; line < startlin + count - 1; line++) {
        memset(&SCREEN[line][startcol - 1], ' ',
            stopcol - startcol + 1);
      } /* for */
    } /* if */
    fflush(stdout);
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
    int start_pos;
    int end_pos;
    unsigned char *new_line;
    unsigned char *old_line;

  /* conLeftScroll */
    if (0 && DC != NULL && (IC != NULL || (IM != NULL && EI != NULL))) {
      putctl(tgoto(CM, startcol - 1, startlin - 1)); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = SCREEN[line];
        for (number = 1; number <= count; number++) {
          putctl(DC); /* delete character */
        } /* for */
        memmove(&new_line[startcol - 1], &new_line[startcol + count - 1],
            stopcol - startcol - count + 1);
        if (line < stoplin - 1) {
          if (DO != NULL) {
            putctl(DO); /* cursor down */
          } else {
            putctl(tgoto(CM, startcol - 1, line + 1)); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      if (IC != NULL) {
        putctl(tgoto(CM, stopcol - count, startlin - 1)); /* cursor motion */
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(IC); /* insert character */
          } /* for */
          memset(&SCREEN[line][stopcol - count], ' ', count);
          if (line < stoplin - 1) {
            if (DO != NULL) {
              putctl(DO); /* cursor down */
            } else {
              putctl(tgoto(CM, stopcol - count, line + 1)); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (MI) { /* safe to move while in insert mode */
          putctl(IM); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(tgoto(CM, stopcol - count, line)); /* cursor motion */
            fwrite(SPACE, 1, count, stdout);
            memset(&SCREEN[line][stopcol - count], ' ', count);
          } /* for */
          putctl(EI); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(tgoto(CM, stopcol - count, line)); /* cursor motion */
            putctl(IM); /* enter insert mode */
            fwrite(SPACE, 1, count, stdout);
            putctl(EI); /* end insert mode */
            memset(&SCREEN[line][stopcol - count], ' ', count);
          } /* for */
        } /* if */
      } /* if */
    } else {
      for (line = startlin - 1; line < stoplin; line++) {
        start_pos = 0;
        end_pos = stopcol - startcol - count;
        new_line = &SCREEN[line][startcol - 1];
        old_line = &SCREEN[line][startcol + count - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putctl(tgoto(CM, startcol + start_pos - 1, line)); /* cursor motion */
          fwrite(&old_line[start_pos], 1, end_pos - start_pos + 1, stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              end_pos - start_pos + 1);
        } /* if */
        start_pos = 0;
        new_line = &SCREEN[line][stopcol - count];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putctl(tgoto(CM, stopcol - count + start_pos, line)); /* cursor motion */
          fwrite(SPACE, 1, count - start_pos, stdout);
          memset(&new_line[start_pos], ' ', count - start_pos);
        } /* if */
      } /* for */
    } /* if */
    fflush(stdout);
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
    int start_pos;
    int end_pos;
    unsigned char *new_line;
    unsigned char *old_line;

  /* conRightScroll */
    if (0 && DC != NULL && (IC != NULL || (IM != NULL && EI != NULL))) {
      putctl(tgoto(CM, stopcol - count, startlin - 1)); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = SCREEN[line];
        for (number = 1; number <= count; number++) {
          putctl(DC); /* delete character */
        } /* for */
        memmove(&new_line[startcol + count - 1], &new_line[startcol - 1],
            stopcol - startcol - count + 1);
        if (line < stoplin - 1) {
          if (DO != NULL) {
            putctl(DO); /* cursor down */
          } else {
            putctl(tgoto(CM, stopcol - count, line + 1)); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      putctl(tgoto(CM, startcol - 1, startlin - 1)); /* cursor motion */
      if (IC != NULL) {
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(IC); /* insert character */
          } /* for */
          memset(&SCREEN[line][startcol - 1], ' ', count);
          if (line < stoplin - 1) {
            if (DO != NULL) {
              putctl(DO); /* cursor down */
            } else {
              putctl(tgoto(CM, startcol - 1, line + 1)); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (MI) { /* safe to move while in insert mode */
          putctl(IM); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            fwrite(SPACE, 1, count, stdout);
            memset(&SCREEN[line][startcol - 1], ' ', count);
            if (line < stoplin - 1) {
              downleft(startcol - 1, line + 1);
            } /* if */
          } /* for */
          putctl(EI); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(IM); /* enter insert mode */
            fwrite(SPACE, 1, count, stdout);
            putctl(EI); /* end insert mode */
            memset(&SCREEN[line][startcol - 1], ' ', count);
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
        new_line = &SCREEN[line][startcol + count - 1];
        old_line = &SCREEN[line][startcol - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putctl(tgoto(CM, startcol + count + start_pos - 1, line)); /* cursor motion */
          fwrite(&old_line[start_pos], 1, end_pos - start_pos + 1, stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              end_pos - start_pos + 1);
        } /* if */
        start_pos = 0;
        new_line = &SCREEN[line][startcol - 1];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putctl(tgoto(CM, startcol + start_pos - 1, line)); /* cursor motion */
          fwrite(SPACE, 1, count - start_pos, stdout);
          memset(&new_line[start_pos], ' ', count - start_pos);
        } /* if */
      } /* for */
    } /* if */
    fflush(stdout);
    changes = TRUE;
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    logFunction(printf("conShut\n"););
    if (console_initialized) {
      putctl(tgoto(CM, 0, LI - 1)); /* cursor motion */
      putctl(SC); /* show cursor */
      putctl(CL); /* clear screen */
      putctl(VE); /* end visual mode */
      putctl(TE); /* end cursor addressing mode */
      console_initialized = FALSE;
    } /* if */
    logFunction(printf("conShut -->\n"););
  } /* conShut */



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  {
    int line;
    int result = 0;

  /* conOpen */
    logFunction(printf("conOpen\n"););
    getcaps();
    if (LI < 0) {
      LI = 24;
    } /* if */
    if (CO < 0) {
      CO = 80;
    } /* if */
    SCRN = (unsigned char *) malloc((size_t) (LI * CO) * sizeof(unsigned char));
    SCREEN = (unsigned char **) malloc ((size_t) LI * sizeof(unsigned char *));
    SPACE = (unsigned char *) malloc((size_t) CO * sizeof(unsigned char));
    if (SCRN != NULL && SCREEN != NULL && SPACE != NULL) {
      memset(SCRN, ' ', (size_t) (LI * CO));
      for (line = 0; line < LI; line++) {
        SCREEN[line] = &SCRN[line * CO];
      } /* for */
      memset(SPACE, ' ', (size_t) CO);
      putctl(TI); /* enter cursor addressing mode */
      putctl(VS); /* enter visual mode */
      putctl(ZC); /* zap cursor (hide cursor) */
      putctl(CL); /* clear screen */
      normalcolour();
      cursor_on = FALSE;
      changes = TRUE;
      console_initialized = TRUE;
      os_atexit(conShut);
      result = 1;
    } /* if */
    logFunction(printf("conOpen -->\n"););
    return result;
  } /* conOpen */
