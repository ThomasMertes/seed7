/********************************************************************/
/*                                                                  */
/*  scr_cap.c     Driver for termcap screen access                  */
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
/*  File: seed7/src/scr_cap.c                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver for termcap screen access                       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "termios.h"

#include "common.h"
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
int tputs (char *, int, int (*) (char CH));

#endif
#else

int tgetent ();
int tgetnum ();
int tgetflag ();
char *tgetstr();
char *tgoto ();
int tputs ();

#endif


#define MAP_CHARS

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

/* int curr_attribute = 0; */
booltype key_buffer_filled = FALSE;
static int last_key;
struct termios term_descr;
tcflag_t lflag_bak;
cc_t min_bak;
cc_t time_bak;
cc_t erase_ch;

booltype changes = FALSE;
booltype keybd_initialized = FALSE;
booltype screen_initialized = FALSE;
booltype cursor_on = FALSE;


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

char *insert_line;          /* al insert line */
booltype auto_right_margin; /* am automatic margins (warp around) */
char *BC;                   /* backspace (if not BS) */
char *clr_eos;              /* cd clear to end of display */
char *clr_eol;              /* ce clear to end of line */
char *clear_screen;         /* cl clear screen */
int columns;                /* co number of columns */
char *cursor_address;       /* cm cursor motion */
char *carriage_return;      /* cr cursor return */
char *delete_character;     /* dc delete character */
char *delete_line;          /* dl delete line */
char *enter_delete_mode;    /* dm enter delete mode (not used now) */
char *cursor_down;          /* do cursor down */
char *exit_delete_mode;     /* ed end delete mode (not used now) */
char *exit_insert_mode;     /* ei end insert mode */
booltype hard_copy;         /* hc hardcopy terminal (not used now) */
char *insert_character;     /* ic insert character */
char *enter_insert_mode;    /* im enter insert mode */
char *IP;                   /* post-insert mode */
char *cursor_left;          /* le cursor left (not used now) */
int lines;                  /* li number of lines */
booltype move_insert_mode;  /* mi safe to move while in insert mode */
char *cursor_right;         /* nd cursor right (not used now) */
char single_pad_char;       /* pc pad character */
char *pad_char;             /* pc pad character */
char *cursor_normal;        /* ve cursor normal */
char *exit_standout_mode;   /* se end standout mode (not used now) */
char *enter_standout_mode;  /* so enter standout mode (not used now) */
char *exit_ca_mode;         /* te end cursor addressing mode */
char *enter_ca_mode;        /* ti enter cursor addressing mode */
char *cursor_up;            /* up cursor up (not used now) */
char *cursor_visible;       /* vs makes cursor very visible */
char *cursor_invisible;     /* vi makes cursor invisible */

static unsigned char **whole_screen;
static unsigned char *scrn;
static unsigned char *space;



#ifdef ANSI_C

static void getcaps (void)
#else

static void getcaps ()
#endif

  {
    static char capbuf[1024];
    char *area;
    char *terminal_name;

  /* getcaps */
    terminal_name = getenv("TERM");
/*  printf(":%s:\n", terminal_name); */
    tgetent(capbuf, terminal_name);
    area = capbuf;
    insert_line =         tgetstr("al", &area);
    auto_right_margin =  tgetflag("am");
    BC =                  tgetstr("bc", &area);
    clr_eos =             tgetstr("cd", &area);
    clr_eol =             tgetstr("ce", &area);
    clear_screen =        tgetstr("cl", &area);
    columns =             tgetnum("co");
    cursor_address =      tgetstr("cm", &area);
    carriage_return =     tgetstr("cr", &area);
    delete_character =    tgetstr("dc", &area);
    delete_line =         tgetstr("dl", &area);
    enter_delete_mode =   tgetstr("dm", &area);
    cursor_down =         tgetstr("do", &area);
    exit_delete_mode =    tgetstr("ed", &area);
    exit_insert_mode =    tgetstr("ei", &area);
    hard_copy =          tgetflag("hc");
    insert_character =    tgetstr("ic", &area);
    enter_insert_mode =   tgetstr("im", &area);
    IP =                  tgetstr("ip", &area);
    cursor_left =         tgetstr("le", &area);
    lines =               tgetnum("li");
    move_insert_mode =   tgetflag("mi");
    cursor_right =        tgetstr("nd", &area);
    pad_char =            tgetstr("pc", &area);
    cursor_normal =       tgetstr("ve", &area);
    exit_standout_mode =  tgetstr("se", &area);
    enter_standout_mode = tgetstr("so", &area);
    exit_ca_mode =        tgetstr("te", &area);
    enter_ca_mode =       tgetstr("ti", &area);
    cursor_up =           tgetstr("up", &area);
    cursor_visible =      tgetstr("vs", &area);
    cursor_invisible =    tgetstr("vi", &area);
    if (cursor_address == NULL) {
      printf("cursor_address == NULL\n");
    } /* if */
/*  printf("%d <%s>\n", strlen(cursor_address), cursor_address); */
  } /* getcaps */



#ifdef ANSI_C

static int outch (char ch)
#else

static int outch (ch)
char ch;
#endif

  { /* outch */
    return(putchar(ch));
  } /* outch */



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
      putctl(tgoto(cursor_address, col, lin)); /* cursor motion */
    } /* if */
  } /* downleft */



#ifdef ANSI_C

void kbdShut (void)
#else

void kbdShut ()
#endif

  { /* kbdShut */
    if (keybd_initialized) {
      term_descr.c_lflag = lflag_bak;
      term_descr.c_cc[VMIN] = min_bak;
      term_descr.c_cc[VTIME] = time_bak;
      tcsetattr(fileno(stdin), TCSANOW, &term_descr);
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



#ifdef ANSI_C

static void kbd_init (void)
#else

static void kbd_init ()
#endif

  { /* kbd_init */
    tcgetattr(fileno(stdin), &term_descr);
    lflag_bak = term_descr.c_lflag;
    min_bak = term_descr.c_cc[VMIN];
    time_bak = term_descr.c_cc[VTIME];
    erase_ch = term_descr.c_cc[VERASE];
    term_descr.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    term_descr.c_cc[VMIN] = 1;
    term_descr.c_cc[VTIME] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &term_descr);
    keybd_initialized = TRUE;
    atexit(kbdShut);
  } /* kbd_init */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  {
    booltype result;
    char buffer;

  /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      result = TRUE;
    } else {
      term_descr.c_cc[VMIN] = 0;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(fileno(stdin), TCSANOW, &term_descr);
      if (fread(&buffer, 1, 1, stdin) == 1) {
        result = TRUE;
        last_key = buffer;
        key_buffer_filled = TRUE;
      } else {
        result = FALSE;
      } /* if */
      term_descr.c_cc[VMIN] = 1;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(fileno(stdin), TCSANOW, &term_descr);
    } /* if */
    return(result);
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
      result = getc(stdin);
    } /* if */
    if (result == erase_ch) {
      result = '\b';
    } /* if */
    return(result);
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
    fflush(stderr);
  } /* snd_beep */



#ifdef ANSI_C

void setcolour (inttype foreground, inttype background)
#else

void setcolour (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* setcolour */
/*  curr_attribute = (char) (foreground + 16 * (background % 8)); */
  } /* setcolour */



#ifdef ANSI_C

void standardcolour (void)
#else

void standardcolour ()
#endif

  { /* standardcolour */
    setcolour(lightgray,black);
/*  curr_attribute = 0; */
  } /* standardcolour */



#ifdef ANSI_C

void normalcolour (void)
#else

void normalcolour ()
#endif

  { /* normalcolour */
    setcolour(lightgray,black);
/*  curr_attribute = 0; */
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
    return(1);
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
  } /* scrFlush */



#ifdef ANSI_C

void scrCursor (booltype on)
#else

void scrCursor (on)
booltype on;
#endif

  { /* scrCursor */
    cursor_on = on;
    changes = TRUE;
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

  { /* scrSetpos */
    putctl(tgoto(cursor_address, col - 1, lin - 1)); /* cursor motion */
    changes = TRUE;
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
    int start_pos;
    int end_pos;
    unsigned char *new_line;

  /* scrWrite */
    if (lin <= lines) {
      if (col + length - 1 <= columns) {
        end_pos = length - 1;
        new_line = &whole_screen[lin - 1][col - 1];
        while (end_pos >= 0 && new_line[end_pos] == stri[end_pos]) {
          end_pos--;
        } /* while */
        if (end_pos >= 0) {
          start_pos = 0;
          while (start_pos <= end_pos && new_line[start_pos] == stri[start_pos]) {
            start_pos++;
          } /* while */
          if (start_pos <= end_pos) {
            putctl(tgoto(cursor_address, col + start_pos - 1, lin - 1)); /* cursor motion */
            fwrite(&stri[start_pos], 1, end_pos - start_pos + 1, stdout);
            memcpy(new_line, stri, end_pos + 1);
          } /* if */
        } /* if */
      } else {
        if (col <= columns) {
          putctl(tgoto(cursor_address, col - 1, lin - 1)); /* cursor motion */
          fwrite(stri, 1, columns - col + 1, stdout);
          memcpy(&whole_screen[lin - 1][col - 1], stri, columns - col + 1);
        } /* if */
      } /* if */
      if (!cursor_on && cursor_invisible == NULL) {
        putctl(tgoto(cursor_address, 0, 0)); /* cursor motion */
      } /* if */
      fflush(stdout);
      changes = TRUE;
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
    int lin;
    int column;
    unsigned char *new_line;

  /* scrClear */
    if (startlin == 1 && stoplin == lines &&
        startcol == 1 && stopcol == columns && clear_screen != NULL) {
      putctl(clear_screen); /* clear screen */
      memset(scrn, ' ', lines * columns);
    } else {
      if (stopcol == columns && clr_eol != NULL) {
        putctl(tgoto(cursor_address, startcol - 1, startlin - 1)); /* cursor motion */
        for (lin = startlin - 1; lin < stoplin; lin++) {
          putctl(clr_eol); /* clear to end of line */
          memset(&whole_screen[lin][startcol - 1], ' ', stopcol - startcol + 1);
          if (lin < stoplin - 1) {
            downleft(startcol - 1, lin + 1);
          } /* if */
        } /* for */
      } else {
        for (lin = startlin - 1; lin < stoplin; lin++) {
          column = stopcol;
          new_line = whole_screen[lin];
          while (column >= startcol && new_line[column - 1] == ' ') {
            column--;
          } /* while */
          if (column >= startcol) {
            putctl(tgoto(cursor_address, startcol - 1, lin)); /* cursor motion */
            fwrite(space, 1, column - startcol + 1, stdout);
            memset(&new_line[startcol - 1], ' ', column - startcol + 1);
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    fflush(stdout);
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
    unsigned char *new_line;

  /* scrUpScroll */
    if (insert_line == NULL || delete_line == NULL ||
        startcol != 1 || stopcol != columns) {
      for (line = startlin - 1; line < stoplin - count; line++) {
        column = stopcol;
        new_line = whole_screen[line];
        old_line = whole_screen[line + count];
        while (column >= startcol && new_line[column - 1] ==
            old_line[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(cursor_address, startcol - 1, line)); /* cursor motion */
          fwrite(&old_line[startcol - 1], 1,
              column - startcol + 1, stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              column - startcol + 1);
        } /* if */
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        column = stopcol;
        new_line = whole_screen[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ') {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(cursor_address, startcol - 1, line)); /* cursor motion */
          fwrite(space, 1, column - startcol + 1, stdout);
          memset(&new_line[startcol - 1], ' ',
              column - startcol + 1);
        } /* if */
      } /* for */
    } else {
      putctl(tgoto(cursor_address, 0, startlin - 1)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(delete_line); /* delete line */
      } /* for */
      putctl(tgoto(cursor_address, 0, stoplin - count)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(insert_line); /* insert line */
      } /* for */
      for (line = startlin - 1; line < stoplin - count; line++) {
        memcpy(&whole_screen[line][startcol - 1],
            &whole_screen[line + count][startcol - 1],
            stopcol - startcol + 1);
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        memset(&whole_screen[line][startcol - 1], ' ',
            stopcol - startcol + 1);
      } /* for */
    } /* if */
    fflush(stdout);
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
    unsigned char *new_line;

  /* scrDownScroll */
    if (insert_line == NULL || delete_line == NULL ||
        startcol != 1 || stopcol != columns) {
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        column = stopcol;
        new_line = whole_screen[line];
        old_line = whole_screen[line - count];
        while (column >= startcol && new_line[column - 1] ==
            old_line[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(cursor_address, startcol - 1, line)); /* cursor motion */
          fwrite(&old_line[startcol - 1], 1,
              column - startcol + 1, stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              column - startcol + 1);
        } /* if */
      } /* for */
      for (line = startlin + count - 2; line >= startlin - 1; line--) {
        column = stopcol;
        new_line = whole_screen[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ') {
          column--;
        } /* while */
        if (column >= startcol) {
          putctl(tgoto(cursor_address, startcol - 1, line)); /* cursor motion */
          fwrite(space, 1, column - startcol + 1, stdout);
          memset(&new_line[startcol - 1], ' ',
              column - startcol + 1);
        } /* if */
      } /* for */
    } else {
      putctl(tgoto(cursor_address, 0, stoplin - count)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(delete_line); /* delete line */
      } /* for */
      putctl(tgoto(cursor_address, 0, startlin - 1)); /* cursor motion */
      for (number = 1; number <= count; number++) {
        putctl(insert_line); /* insert line */
      } /* for */
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        memcpy(&whole_screen[line][startcol - 1],
            &whole_screen[line - count][startcol - 1],
            stopcol - startcol + 1);
      } /* for */
/*    for (line = startlin + count - 2; line >= startlin - 1; line--) { */
      for (line = startlin - 1; line < startlin + count - 1; line++) {
        memset(&whole_screen[line][startcol - 1], ' ',
            stopcol - startcol + 1);
      } /* for */
    } /* if */
    fflush(stdout);
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
      putctl(tgoto(cursor_address, startcol - 1, startlin - 1)); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = whole_screen[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol - 1], &new_line[startcol + count - 1],
            stopcol - startcol - count + 1);
        if (line < stoplin - 1) {
          if (cursor_down != NULL) {
            putctl(cursor_down); /* cursor down */
          } else {
            putctl(tgoto(cursor_address, startcol - 1, line + 1)); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      if (insert_character != NULL) {
        putctl(tgoto(cursor_address, stopcol - count, startlin - 1)); /* cursor motion */
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(insert_character); /* insert character */
          } /* for */
          memset(&whole_screen[line][stopcol - count], ' ', count);
          if (line < stoplin - 1) {
            if (cursor_down != NULL) {
              putctl(cursor_down); /* cursor down */
            } else {
              putctl(tgoto(cursor_address, stopcol - count, line + 1)); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (move_insert_mode) { /* safe to move while in insert mode */
          putctl(enter_insert_mode); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(tgoto(cursor_address, stopcol - count, line)); /* cursor motion */
            fwrite(space, 1, count, stdout);
            memset(&whole_screen[line][stopcol - count], ' ', count);
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(tgoto(cursor_address, stopcol - count, line)); /* cursor motion */
            putctl(enter_insert_mode); /* enter insert mode */
            fwrite(space, 1, count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            memset(&whole_screen[line][stopcol - count], ' ', count);
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
          putctl(tgoto(cursor_address, startcol + start_pos - 1, line)); /* cursor motion */
          fwrite(&old_line[start_pos], 1, end_pos - start_pos + 1, stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              end_pos - start_pos + 1);
        } /* if */
        start_pos = 0;
        new_line = &whole_screen[line][stopcol - count];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putctl(tgoto(cursor_address, stopcol - count + start_pos, line)); /* cursor motion */
          fwrite(space, 1, count - start_pos, stdout);
          memset(&new_line[start_pos], ' ', count - start_pos);
        } /* if */
      } /* for */
    } /* if */
    fflush(stdout);
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
      putctl(tgoto(cursor_address, stopcol - count, startlin - 1)); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = whole_screen[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol + count - 1], &new_line[startcol - 1],
            stopcol - startcol - count + 1);
        if (line < stoplin - 1) {
          if (cursor_down != NULL) {
            putctl(cursor_down); /* cursor down */
          } else {
            putctl(tgoto(cursor_address, stopcol - count, line + 1)); /* cursor motion */
          } /* if */
        } /* if */
      } /* for */
      putctl(tgoto(cursor_address, startcol - 1, startlin - 1)); /* cursor motion */
      if (insert_character != NULL) {
        for (line = startlin - 1; line < stoplin; line++) {
          for (number = 1; number <= count; number++) {
            putctl(insert_character); /* insert character */
          } /* for */
          memset(&whole_screen[line][startcol - 1], ' ', count);
          if (line < stoplin - 1) {
            if (cursor_down != NULL) {
              putctl(cursor_down); /* cursor down */
            } else {
              putctl(tgoto(cursor_address, startcol - 1, line + 1)); /* cursor motion */
            } /* if */
          } /* if */
        } /* for */
      } else {
        if (move_insert_mode) { /* safe to move while in insert mode */
          putctl(enter_insert_mode); /* enter insert mode */
          for (line = startlin - 1; line < stoplin; line++) {
            fwrite(space, 1, count, stdout);
            memset(&whole_screen[line][startcol - 1], ' ', count);
            if (line < stoplin - 1) {
              downleft(startcol - 1, line + 1);
            } /* if */
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(enter_insert_mode); /* enter insert mode */
            fwrite(space, 1, count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            memset(&whole_screen[line][startcol - 1], ' ', count);
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
          putctl(tgoto(cursor_address, startcol + count + start_pos - 1, line)); /* cursor motion */
          fwrite(&old_line[start_pos], 1, end_pos - start_pos + 1, stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              end_pos - start_pos + 1);
        } /* if */
        start_pos = 0;
        new_line = &whole_screen[line][startcol - 1];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putctl(tgoto(cursor_address, startcol + start_pos - 1, line)); /* cursor motion */
          fwrite(space, 1, count - start_pos, stdout);
          memset(&new_line[start_pos], ' ', count - start_pos);
        } /* if */
      } /* for */
    } /* if */
    fflush(stdout);
    changes = TRUE;
  } /* scrRightScroll */



#ifdef ANSI_C

void scrShut (void)
#else

void scrShut ()
#endif

  { /* scrShut */
    if (screen_initialized) {
      putctl(tgoto(cursor_address, 0, lines - 1)); /* cursor motion */
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
    getcaps();
    if (lines == -1) {
      lines = 24;
    } /* if */
    if (columns == -1) {
      columns = 80;
    } /* if */
    scrn = (unsigned char *) malloc(lines * columns * sizeof(unsigned char));
    whole_screen = (unsigned char **) malloc (lines * sizeof(unsigned char *));
    space = (unsigned char *) malloc(columns * sizeof(unsigned char));
    if (scrn != NULL && whole_screen != NULL && space != NULL) {
      memset(scrn, ' ', lines * columns);
      for (line = 0; line < lines; line++) {
        whole_screen[line] = &scrn[line * columns];
      } /* for */
      memset(space, ' ', columns);
      putctl(enter_ca_mode); /* enter cursor addressing mode */
      putctl(cursor_invisible); /* makes cursor invisible */
      putctl(clear_screen); /* clear screen */
      normalcolour();
      cursor_on = FALSE;
      changes = TRUE;
      screen_initialized = TRUE;
      atexit(scrShut);
      result = 1;
    } /* if */
    return(result);
  } /* scrOpen */
