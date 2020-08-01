/********************************************************************/
/*                                                                  */
/*  con_inf.c     Driver for terminfo console access.               */
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
/*  File: seed7/src/con_inf.c                                       */
/*  Changes: 1994 - 1999  Thomas Mertes                             */
/*  Content: Driver for terminfo console access.                    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#ifdef UNISTD_H_PRESENT
#include "unistd.h"
#endif

#ifdef USE_TERMINFO
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

#include "signal.h"
#include "sys/ioctl.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"
#include "trm_drv.h"

#ifdef USE_TERMCAP
#include "cap_def.h"
#endif

#include "con_drv.h"


/* #define atexit(x) */


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



#ifdef USE_TERMCAP
#ifdef C_PLUS_PLUS

extern "C" int tgetent (char *, char *);
extern "C" int tgetnum (char *);
extern "C" int tgetflag (char *);
extern "C" char *tgetstr(char *, char **);
extern "C" char *tgoto (char *, int, int);
extern "C" int tputs (char *, int, int (*) (char ch));

#else

int tgetent (char *, char *);
int tgetnum (char *);
int tgetflag (char *);
char *tgetstr(char *, char **);
char *tgoto (char *, int, int);
void tputs (char *, int, int (*) (char ch));

#endif
#endif


#undef TPARM_PRESENT

#define TEXT_NORMAL 0
#define TEXT_INVERSE 1
#define WRITE_STRI_BLOCK_SIZE    256

static unsigned char curr_attr = TEXT_NORMAL;
boolType changes = FALSE;

static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;
static boolType cursor_position_okay = FALSE;
static int cursor_line = 1;
static int cursor_column = 1;

typedef struct {
    strElemType **chars;
    strElemType *char_data;
    unsigned char **attributes;
    unsigned char *attrib_data;
    strElemType *space;
    int line_capacity;
    int column_capacity;
    int height;
    int width;
    boolType size_changed;
  } consoleRecord,  *consoleType;

static consoleType con;



static inline void strelem_memset (strElemType *mem,
    const strElemType ch, size_t number)

  { /* strelem_memcmp */
    for (; number > 0; number--, mem++) {
      *mem = ch;
    } /* for */
  } /* strelem_memcmp */



static void strelem_fwrite (const strElemType *stri, memSizeType length,
    fileType outFile)

  {
    memSizeType size;
    ucharType stri_buffer[max_utf8_size(WRITE_STRI_BLOCK_SIZE)];

  /* strelem_fwrite */
    for (; length >= WRITE_STRI_BLOCK_SIZE;
        stri += WRITE_STRI_BLOCK_SIZE, length -= WRITE_STRI_BLOCK_SIZE) {
      size = stri_to_utf8(stri_buffer, stri, WRITE_STRI_BLOCK_SIZE);
      if (size != fwrite(stri_buffer, sizeof(ucharType), (size_t) size, outFile)) {
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* for */
    if (length > 0) {
      size = stri_to_utf8(stri_buffer, stri, length);
      if (size != fwrite(stri_buffer, sizeof(ucharType), (size_t) size, outFile)) {
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* if */
  } /* strelem_fwrite */



static void free_console (consoleType old_con)

  { /* free_console */
    if (old_con != NULL) {
      free(old_con->char_data);
      free(old_con->chars);
      free(old_con->attrib_data);
      free(old_con->attributes);
      free(old_con->space);
      free(old_con);
    } /* if */
  } /* free_console */



static consoleType create_console (int height, int width)

  {
    int line;
    consoleType new_con;

  /* create_console */
    new_con = (consoleType) malloc(sizeof(consoleRecord));
    if (new_con != NULL) {
      new_con->char_data = (strElemType *) malloc((size_t) (height * width) * sizeof(strElemType));
      new_con->chars = (strElemType **) malloc ((size_t) height * sizeof(strElemType *));
      new_con->attrib_data = (unsigned char *) malloc((size_t) (height * width) * sizeof(unsigned char));
      new_con->attributes = (unsigned char **) malloc ((size_t) height * sizeof(unsigned char *));
      new_con->space = (strElemType *) malloc((size_t) width * sizeof(strElemType));
      if (new_con->char_data != NULL && new_con->chars != NULL && new_con->attrib_data != NULL &&
          new_con->attributes != NULL &&new_con->space != NULL) {
        for (line = 0; line < height; line++) {
          new_con->chars[line] = &new_con->char_data[line * width];
          new_con->attributes[line] = &new_con->attrib_data[line * width];
        } /* for */
        strelem_memset(new_con->char_data, ' ', (unsigned int) (height * width));
        memset(new_con->attrib_data, ' ', (unsigned int) (height * width));
        strelem_memset(new_con->space, ' ', (size_t) width);
        new_con->line_capacity = height;
        new_con->column_capacity = width;
        new_con->height = height;
        new_con->width = width;
        new_con->size_changed = FALSE;
      } else {
        free_console(new_con);
        new_con = NULL;
      } /* if */
    } /* if */
    return new_con;
  } /* create_console */



static void resize_console (void)

  {
    struct winsize window_size;
    int height;
    int width;
    int line_capacity;
    int column_capacity;
    int line;
    consoleType new_con;

  /* resize_console */
    ioctl(0, TIOCGWINSZ, &window_size);
    /* printf("handle_winch_signal: lines=%d, columns=%d\n", w.ws_row, w.ws_col); */
    height = window_size.ws_row;
    width = window_size.ws_col;
    if (height > con->line_capacity ||
        width > con->column_capacity) {
      if (height > con->line_capacity) {
        line_capacity = height;
      } else {
        line_capacity = con->line_capacity;
      } /* if */
      if (width > con->column_capacity) {
        column_capacity = width;
      } else {
        column_capacity = con->column_capacity;
      } /* if */
      new_con = create_console(line_capacity, column_capacity);
      if (new_con != NULL) {
        for (line = 0; line < con->height; line++) {
          memcpy(new_con->chars[line], con->chars[line],
              sizeof(strElemType) * (unsigned int) con->width);
          memcpy(new_con->attributes[line], con->attributes[line],
              (unsigned int) con->width);
        } /* for */
        new_con->height = height;
        new_con->width = width;
        free_console(con);
        con = new_con;
      } /* if */
    } else {
      con->height = height;
      con->width = width;
    } /* if */
    con->size_changed = FALSE;
  } /* resize_console */



static void downleft (int col, int lin)

  { /* downleft */
    if (col == 0 && carriage_return != NULL && cursor_down != NULL) {
      putctl(carriage_return); /* cursor return */
      putctl(cursor_down); /* cursor down */
    } else {
      putgoto(cursor_address, col, lin); /* cursor motion */
    } /* if */
  } /* downleft */



static void inf_beep (void)

  { /* inf_beep */
    fputc('\007', stderr);
    fflush(stderr);
  } /* inf_beep */



static void setattr (unsigned char attribute)

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



static void inf_setcolor (intType foreground, intType background)

  { /* inf_setcolor */
    if (foreground == black || background == white) {
      curr_attr = TEXT_INVERSE;
    } else {
      curr_attr = TEXT_NORMAL;
    } /* if */
  } /* inf_setcolor */



static void inf_standardcolour (void)

  { /* inf_standardcolour */
    inf_setcolor(lightgray,black);
/*  curr_attribute = 0; */
  } /* inf_standardcolour */



void inf_normalcolour (void)

  { /* inf_normalcolour */
    inf_setcolor(lightgray,black);
/*  curr_attribute = 0; */
  } /* inf_normalcolour */



static int inf_setfont (char *fontname)

  { /* inf_setfont */
    return 1;
  } /* inf_setfont */



intType inf_textheight (void)

  { /* inf_textheight */
    return 1;
  } /* inf_textheight */



intType textwidth (striType stri,
    intType startcol, intType stopcol)

  { /* textwidth */
    return stopcol + 1 - startcol;
  } /* textwidth */



void textcolumns (striType stri, intType striwidth,
    intType * cols, intType *rest)

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



int conHeight (void)

  { /* conHeight */
    if (con->size_changed) {
      resize_console();
    } /* if */
    return con->height;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    if (con->size_changed) {
      resize_console();
    } /* if */
    return con->width;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
    /* fprintf(stderr, "conFlush\n"); */
    if (console_initialized) {
      if (con->size_changed) {
        resize_console();
      } /* if */
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
      changes = FALSE;
    } /* if */
    fflush(stdout);
    /* fsync(fileno(stdout)); */
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
    /* fprintf(stderr, "scrCursor(%d)\n", on); */
    cursor_on = on;
    if (on) {
      putctl(cursor_normal); /* cursor normal */
    } else {
      putctl(cursor_invisible); /* makes cursor invisible */
    } /* if */
    changes = TRUE;
  } /* conCursor */



/**
 *  Moves the system curser to the given place of the console.
 *  When no system cursor exists this procedure can be replaced by
 *  a dummy procedure.
 */
void conSetCursor (intType line, intType column)

  { /* conSetCursor */
    /* fprintf(stderr, "scrSetCursor(%d, %d)\n", line, column); */
    if (line <= 0 || column <= 0) {
      raise_error(RANGE_ERROR);
    } else if (line <= INT_MAX && column <= INT_MAX) {
      if (cursor_line != line || cursor_column != column) {
        cursor_position_okay = FALSE;
        cursor_line = (int) line;
        cursor_column = (int) column;
      } /* if */
    } /* if */
/*  putgoto(cursor_address, column - 1, line - 1); cursor motion */
    changes = TRUE;
  } /* conSetCursor */



/**
 *  Writes the string stri to the console at the current position.
 */
void conWrite (const const_striType stri)

  {
    int start_pos;
    int end_pos;
    int position;
    strElemType *new_line;
    unsigned char *new_attr;

  /* conWrite */
    if (console_initialized) {
      if (con->size_changed) {
        resize_console();
      } /* if */
      if (cursor_line <= con->height && stri->size != 0) {
        new_line = &con->chars[cursor_line - 1][cursor_column - 1];
        new_attr = &con->attributes[cursor_line - 1][cursor_column - 1];
        if (cursor_column <= con->width) {
          if (stri->size - 1 > con->width - cursor_column) {
            end_pos = con->width - cursor_column;
          } else {
            end_pos = (int) (stri->size - 1);
          } /* if */
          while (end_pos >= 0 &&
              new_line[end_pos] == stri->mem[end_pos] &&
              new_attr[end_pos] == curr_attr) {
            end_pos--;
          } /* while */
          if (end_pos >= 0) {
            start_pos = 0;
            while (start_pos <= end_pos &&
                new_line[start_pos] == stri->mem[start_pos] &&
                new_attr[start_pos] == curr_attr) {
              start_pos++;
            } /* while */
            if (start_pos <= end_pos) {
              memcpy(&new_line[start_pos], &stri->mem[start_pos],
                     sizeof(strElemType) * (unsigned int) (end_pos - start_pos + 1));
              if (cursor_position_okay) {
                start_pos = 0;
              } else {
                putgoto(cursor_address, cursor_column + start_pos - 1, cursor_line - 1); /* cursor motion */
              } /* if */
              if (ceol_standout_glitch) {
                for (position = 0; position <= end_pos - start_pos; position++) {
                  if (new_attr[start_pos + position] != curr_attr) {
                    setattr(curr_attr);
                  } /* if */
                  strelem_fwrite(&new_line[start_pos + position], 1, stdout);
                } /* for */
                if (cursor_column + end_pos < con->width && new_attr[end_pos + 1] != curr_attr) {
                  setattr(new_attr[end_pos + 1]);
                } /* if */
              } else {
                if (curr_attr != TEXT_NORMAL) {
                  setattr(curr_attr);
                } /* if */
                strelem_fwrite(&new_line[start_pos],
                    (unsigned int) (end_pos - start_pos + 1), stdout);
                if (curr_attr != TEXT_NORMAL) {
                  setattr(TEXT_NORMAL);
                } /* if */
              } /* if */
              memset(&new_attr[start_pos], curr_attr,
                  (unsigned int) (end_pos - start_pos + 1));
              cursor_position_okay = TRUE;
              cursor_column += end_pos + 1;
            } /* if */
          } /* if */
          changes = TRUE;
        } /* if */
      } /* if */
    } else {
      strelem_fwrite(stri->mem, stri->size, stdout);
    } /* if */
  } /* conWrite */



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 *  The calling function assures that startlin, stoplin, startcol and
 *  stopcol are greater or equal 1.
 */
static void doClear (int startlin, int startcol,
    int stoplin, int stopcol)

  {
    int line;
    int column;
    strElemType *new_line;
    unsigned char *new_attr;

  /* doClear */
    /* fprintf(stderr, "doClear(%ld, %ld, %ld, %ld)\n",
       startlin, startcol, stoplin, stopcol); */
    if (startlin == 1 && stoplin == con->height &&
        startcol == 1 && stopcol == con->width && clear_screen != NULL) {
      putctl(clear_screen); /* clear screen */
      for (line = 0; line < stoplin; line++) {
        strelem_memset(con->chars[line], ' ',
            (unsigned int) con->width);
        memset(con->attributes[line], curr_attr,
            (unsigned int) con->width);
      } /* for */
    } else {
      if (stopcol == con->width && clr_eol != NULL) {
        putgoto(cursor_address, startcol - 1, startlin - 1); /* cursor motion */
        for (line = startlin - 1; line < stoplin; line++) {
          if (curr_attr != TEXT_NORMAL) {
            setattr(curr_attr);
          } /* if */
          putctl(clr_eol); /* clear to end of line */
          if (curr_attr != TEXT_NORMAL) {
            setattr(TEXT_NORMAL);
          } /* if */
          strelem_memset(&con->chars[line][startcol - 1], ' ',
              (unsigned int) (stopcol - startcol + 1));
          memset(&con->attributes[line][startcol - 1], curr_attr,
              (unsigned int) (stopcol - startcol + 1));
          if (line < stoplin - 1) {
            downleft(startcol - 1, line + 1);
          } /* if */
        } /* for */
      } else {
        for (line = startlin - 1; line < stoplin; line++) {
          column = stopcol;
          new_line = con->chars[line];
          new_attr = con->attributes[line];
          while (column >= startcol &&
              new_line[column - 1] == ' ' &&
              new_attr[column - 1] == curr_attr) {
            column--;
          } /* while */
          if (column >= startcol) {
            putgoto(cursor_address, startcol - 1, line); /* cursor motion */
            if (curr_attr != TEXT_NORMAL) {
              setattr(curr_attr);
            } /* if */
            strelem_fwrite(con->space, (unsigned int) (column - startcol + 1), stdout);
            if (curr_attr != TEXT_NORMAL) {
              setattr(TEXT_NORMAL);
            } /* if */
            strelem_memset(&new_line[startcol - 1], ' ',
                (unsigned int) (column - startcol + 1));
            memset(&new_attr[startcol - 1], curr_attr,
                (unsigned int) (column - startcol + 1));
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* doClear */



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  { /* conClear */
    /* fprintf(stderr, "conClear(%ld, %ld, %ld, %ld)\n",
       startlin, startcol, stoplin, stopcol); */
    if (con->size_changed) {
      resize_console();
    } /* if */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= con->height && startcol <= con->width) {
      if (stoplin > con->height) {
        stoplin = con->height;
      } /* if */
      if (stopcol > con->width) {
        stopcol = con->width;
      } /* if */
      doClear((int) startlin, (int) startcol, (int) stoplin, (int) stopcol);
    } /* if */
  } /* conClear */



/**
 *  Scrolls the given area stopcol upward by count lines.
 *  The calling function assures that startlin, stoplin, startcol,
 *  stopcol and count are greater or equal 1.
 */
static void doUpScroll (int startlin, int startcol,
    int stoplin, int stopcol, int count)

  {
    int number;
    int line;
    int column;
    strElemType *old_line;
    unsigned char *old_attr;
    strElemType *new_line;
    unsigned char *new_attr;

  /* doUpScroll */
    if (count > stoplin - startlin + 1) {
      count = stoplin - startlin + 1;
    } /* if */
    if ((insert_line == NULL && parm_insert_line == NULL) ||
        (delete_line == NULL && parm_delete_line == NULL) ||
        startcol != 1 || stopcol != con->width) {
      for (line = startlin - 1; line < stoplin - count; line++) {
        column = stopcol;
        new_line = con->chars[line];
        new_attr = con->attributes[line];
        old_line = con->chars[line + count];
        old_attr = con->attributes[line + count];
        while (column >= startcol &&
            new_line[column - 1] == old_line[column - 1] &&
            new_attr[column - 1] == old_attr[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              sizeof(strElemType) * (unsigned int) (column - startcol + 1));
          memcpy(&new_attr[startcol - 1],
              &old_attr[startcol - 1],
              (unsigned int) (column - startcol + 1));
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          strelem_fwrite(&new_line[startcol - 1],
              (unsigned int) (column - startcol + 1), stdout);
        } /* if */
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        column = stopcol;
        new_line = con->chars[line];
        new_attr = con->attributes[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ' &&
            new_attr[column - 1] == curr_attr) {
          column--;
        } /* while */
        if (column >= startcol) {
          strelem_memset(&new_line[startcol - 1], ' ',
              (unsigned int) (column - startcol + 1));
          memset(&new_attr[startcol - 1], curr_attr,
              (unsigned int) (column - startcol + 1));
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          if (curr_attr != TEXT_NORMAL) {
            setattr(curr_attr);
          } /* if */
          strelem_fwrite(con->space, (unsigned int) (column - startcol + 1), stdout);
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
        memcpy(&con->chars[line][startcol - 1],
            &con->chars[line + count][startcol - 1],
            sizeof(strElemType) * (unsigned int) (stopcol - startcol + 1));
        memcpy(&con->attributes[line][startcol - 1],
            &con->attributes[line + count][startcol - 1],
            (unsigned int) (stopcol - startcol + 1));
      } /* for */
      for (line = stoplin - count; line < stoplin; line++) {
        strelem_memset(&con->chars[line][startcol - 1], ' ',
            (unsigned int) (stopcol - startcol + 1));
        memset(&con->attributes[line][startcol - 1], curr_attr,
            (unsigned int) (stopcol - startcol + 1));
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* doUpScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by count lines. The upper count lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conUpScroll */
    if (con->size_changed) {
      resize_console();
    } /* if */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= con->height && startcol <= con->width) {
      if (stoplin > con->height) {
        stoplin = con->height;
      } /* if */
      if (stopcol > con->width) {
        stopcol = con->width;
      } /* if */
      doUpScroll((int) startlin, (int) startcol,
                 (int) stoplin, (int) stopcol, (int) count);
    } /* if */
  } /* conUpScroll */



/**
 *  Scrolls the given area stopcol downward by count lines.
 *  The calling function assures that startlin, stoplin, startcol,
 *  stopcol and count are greater or equal 1.
 */
static void doDownScroll (int startlin, int startcol,
    int stoplin, int stopcol, int count)

  {
    int number;
    int line;
    int column;
    strElemType *old_line;
    unsigned char *old_attr;
    strElemType *new_line;
    unsigned char *new_attr;

  /* doDownScroll */
    if (count > stoplin - startlin + 1) {
      count = stoplin - startlin + 1;
    } /* if */
    if ((insert_line == NULL && parm_insert_line == NULL) ||
        (delete_line == NULL && parm_delete_line == NULL) ||
        startcol != 1 || stopcol != con->width) {
      for (line = stoplin - 1; line >= startlin + count - 1; line--) {
        column = stopcol;
        new_line = con->chars[line];
        new_attr = con->attributes[line];
        old_line = con->chars[line - count];
        old_attr = con->attributes[line - count];
        while (column >= startcol &&
            new_line[column - 1] == old_line[column - 1] &&
            new_attr[column - 1] == old_attr[column - 1]) {
          column--;
        } /* while */
        if (column >= startcol) {
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          strelem_fwrite(&old_line[startcol - 1],
              (unsigned int) (column - startcol + 1), stdout);
          memcpy(&new_line[startcol - 1],
              &old_line[startcol - 1],
              sizeof(strElemType) * (unsigned int) (column - startcol + 1));
          memcpy(&new_attr[startcol - 1],
              &old_attr[startcol - 1],
              (unsigned int) (column - startcol + 1));
        } /* if */
      } /* for */
      for (line = startlin + count - 2; line >= startlin - 1; line--) {
        column = stopcol;
        new_line = con->chars[line];
        new_attr = con->attributes[line];
        while (column >= startcol &&
            new_line[column - 1] == ' ' &&
            new_attr[column - 1] == curr_attr) {
          column--;
        } /* while */
        if (column >= startcol) {
          putgoto(cursor_address, startcol - 1, line); /* cursor motion */
          strelem_fwrite(con->space, (unsigned int) (column - startcol + 1), stdout);
          strelem_memset(&new_line[startcol - 1], ' ',
              (unsigned int) (column - startcol + 1));
          memset(&new_attr[startcol - 1], curr_attr,
              (unsigned int) (column - startcol + 1));
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
        memcpy(&con->chars[line][startcol - 1],
            &con->chars[line - count][startcol - 1],
            sizeof(strElemType) * (unsigned int) (stopcol - startcol + 1));
        memcpy(&con->attributes[line][startcol - 1],
            &con->attributes[line - count][startcol - 1],
            (unsigned int) (stopcol - startcol + 1));
      } /* for */
/*    for (line = startlin + count - 2; line >= startlin - 1; line--) { */
      for (line = startlin - 1; line < startlin + count - 1; line++) {
        strelem_memset(&con->chars[line][startcol - 1], ' ',
            (unsigned int) (stopcol - startcol + 1));
        memset(&con->attributes[line][startcol - 1], curr_attr,
            (unsigned int) (stopcol - startcol + 1));
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* doDownScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by count lines. The lower count lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conDownScroll */
    if (con->size_changed) {
      resize_console();
    } /* if */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= con->height && startcol <= con->width) {
      if (stoplin > con->height) {
        stoplin = con->height;
      } /* if */
      if (stopcol > con->width) {
        stopcol = con->width;
      } /* if */
      doDownScroll((int) startlin, (int) startcol,
                   (int) stoplin, (int) stopcol, (int) count);
    } /* if */
  } /* conDownScroll */




/**
 *  Scrolls the given area stopcol leftward by count columns.
 *  The calling function assures that startlin, stoplin, startcol,
 *  stopcol and count are greater or equal 1.
 */
static void doLeftScroll (int startlin, int startcol,
    int stoplin, int stopcol, int count)

  {
    int number;
    int line;
    int start_pos;
    int end_pos;
    strElemType *new_line;
    strElemType *old_line;

  /* doLeftScroll */
    if (0 && delete_character != NULL && (insert_character != NULL ||
        (enter_insert_mode != NULL && exit_insert_mode != NULL))) {
      putgoto(cursor_address, startcol - 1, startlin - 1); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = con->chars[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol - 1], &new_line[startcol + count - 1],
            sizeof(strElemType) * (unsigned int) (stopcol - startcol - count + 1));
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
          strelem_memset(&con->chars[line][stopcol - count], ' ', (unsigned int) count);
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
            strelem_fwrite(con->space, (unsigned int) count, stdout);
            strelem_memset(&con->chars[line][stopcol - count], ' ', (unsigned int) count);
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putgoto(cursor_address, stopcol - count, line); /* cursor motion */
            putctl(enter_insert_mode); /* enter insert mode */
            strelem_fwrite(con->space, (unsigned int) count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            strelem_memset(&con->chars[line][stopcol - count], ' ', (unsigned int) count);
          } /* for */
        } /* if */
      } /* if */
    } else {
      for (line = startlin - 1; line < stoplin; line++) {
        start_pos = 0;
        end_pos = stopcol - startcol - count;
        new_line = &con->chars[line][startcol - 1];
        old_line = &con->chars[line][startcol + count - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putgoto(cursor_address, startcol + start_pos - 1, line); /* cursor motion */
          strelem_fwrite(&old_line[start_pos],
              (unsigned int) (end_pos - start_pos + 1), stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              sizeof(strElemType) * (unsigned int) (end_pos - start_pos + 1));
        } /* if */
        start_pos = 0;
        new_line = &con->chars[line][stopcol - count];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putgoto(cursor_address, stopcol - count + start_pos, line); /* cursor motion */
          strelem_fwrite(con->space, (unsigned int) (count - start_pos), stdout);
          strelem_memset(&new_line[start_pos], ' ', (unsigned int) (count - start_pos));
        } /* if */
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* doLeftScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by count columns. The left count columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conLeftScroll */
    if (con->size_changed) {
      resize_console();
    } /* if */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= con->height && startcol <= con->width) {
      if (stoplin > con->height) {
        stoplin = con->height;
      } /* if */
      if (stopcol > con->width) {
        stopcol = con->width;
      } /* if */
      doLeftScroll((int) startlin, (int) startcol,
                   (int) stoplin, (int) stopcol, (int) count);
    } /* if */
  } /* conLeftScroll */



/**
 *  Scrolls the given area stopcol rightward by count columns.
 *  The calling function assures that startlin, stoplin, startcol,
 *  stopcol and count are greater or equal 1.
 */
static void doRightScroll (int startlin, int startcol,
    int stoplin, int stopcol, int count)

  {
    int number;
    int line;
    int start_pos;
    int end_pos;
    strElemType *new_line;
    strElemType *old_line;

  /* doRightScroll */
    if (0 && delete_character != NULL && (insert_character != NULL ||
        (enter_insert_mode != NULL && exit_insert_mode != NULL))) {
      putgoto(cursor_address, stopcol - count, startlin - 1); /* cursor motion */
      for (line = startlin - 1; line < stoplin; line++) {
        new_line = con->chars[line];
        for (number = 1; number <= count; number++) {
          putctl(delete_character); /* delete character */
        } /* for */
        memmove(&new_line[startcol + count - 1], &new_line[startcol - 1],
            sizeof(strElemType) * (unsigned int) (stopcol - startcol - count + 1));
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
          strelem_memset(&con->chars[line][startcol - 1], ' ', (unsigned int) count);
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
            strelem_fwrite(con->space, (unsigned int) count, stdout);
            strelem_memset(&con->chars[line][startcol - 1], ' ', (unsigned int) count);
            if (line < stoplin - 1) {
              downleft(startcol - 1, line + 1);
            } /* if */
          } /* for */
          putctl(exit_insert_mode); /* end insert mode */
        } else {
          for (line = startlin - 1; line < stoplin; line++) {
            putctl(enter_insert_mode); /* enter insert mode */
            strelem_fwrite(con->space, (unsigned int) count, stdout);
            putctl(exit_insert_mode); /* end insert mode */
            strelem_memset(&con->chars[line][startcol - 1], ' ', (unsigned int) count);
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
        new_line = &con->chars[line][startcol + count - 1];
        old_line = &con->chars[line][startcol - 1];
        while (start_pos <= end_pos && new_line[start_pos] ==
            old_line[start_pos]) {
          start_pos++;
        } /* while */
        if (start_pos <= end_pos) {
          while (new_line[end_pos] == old_line[end_pos]) {
            end_pos--;
          } /* while */
          putgoto(cursor_address, startcol + count + start_pos - 1, line); /* cursor motion */
          strelem_fwrite(&old_line[start_pos],
              (unsigned int) (end_pos - start_pos + 1), stdout);
          memmove(&new_line[start_pos], &old_line[start_pos],
              sizeof(strElemType) * (unsigned int) (end_pos - start_pos + 1));
        } /* if */
        start_pos = 0;
        new_line = &con->chars[line][startcol - 1];
        while (start_pos < count && new_line[start_pos] == ' ') {
          start_pos++;
        } /* while */
        if (start_pos < count) {
          putgoto(cursor_address, startcol + start_pos - 1, line); /* cursor motion */
          strelem_fwrite(con->space, (unsigned int) (count - start_pos), stdout);
          strelem_memset(&new_line[start_pos], ' ', (unsigned int) (count - start_pos));
        } /* if */
      } /* for */
    } /* if */
#ifdef OUT_OF_ORDER
    conFlush();
#endif
    cursor_position_okay = FALSE;
    changes = TRUE;
  } /* doRightScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by count columns. The right count columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conRightScroll */
    if (con->size_changed) {
      resize_console();
    } /* if */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= con->height && startcol <= con->width) {
      if (stoplin > con->height) {
        stoplin = con->height;
      } /* if */
      if (stopcol > con->width) {
        stopcol = con->width;
      } /* if */
      doRightScroll((int) startlin, (int) startcol,
                    (int) stoplin, (int) stopcol, (int) count);
    } /* if */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    if (console_initialized) {
      putgoto(cursor_address, 0, con->height - 1); /* cursor motion */
      putctl(cursor_normal); /* cursor normal */
      putctl(clear_screen); /* clear screen */
      putctl(exit_ca_mode); /* end cursor addressing mode */
      console_initialized = FALSE;
    } /* if */
  } /* conShut */



#ifdef SIGWINCH
static void handle_winch_signal (int sig_num)

  { /* handle_winch_signal */
    con->size_changed = TRUE;
#ifndef HAS_SIGACTION
    signal(SIGWINCH, handle_winch_signal);
#endif
  } /* handle_winch_signal */
#endif



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  {
    int result = 0;

  /* conOpen */
#ifdef TRACE_INIT
    printf("BEGIN conOpen\n");
    fflush(stdout);
#endif
    if (getcaps()) {
/*    printf("lines: %d  columns: %d\n", lines, columns); */
      if (lines < 0) {
        lines = 24;
      } /* if */
      if (columns < 0) {
        columns = 80;
      } /* if */
      con = create_console(lines, columns);
      if (con != NULL) {
        putctl(enter_ca_mode); /* enter cursor addressing mode */
        putctl(cursor_invisible); /* makes cursor invisible */
        putctl(clear_screen); /* clear screen */
        cursor_on = FALSE;
        inf_normalcolour();
        cursor_position_okay = FALSE;
        cursor_line = 1;
        cursor_column = 1;
        changes = TRUE;
        console_initialized = TRUE;
        atexit(conShut);
#ifdef SIGWINCH
#ifdef HAS_SIGACTION
        {
          struct sigaction sig_act;

          sig_act.sa_handler = handle_winch_signal;
          sigemptyset(&sig_act.sa_mask);
          sig_act.sa_flags = SA_RESTART;
          if (sigaction(SIGWINCH, &sig_act, NULL) == -1) {
            raise_error(FILE_ERROR);
          } /* if */
        }
#else
        signal(SIGWINCH, handle_winch_signal);
#endif
#endif
        result = 1;
      } /* if */
    } /* if */
#ifdef TRACE_INIT
    printf("END conOpen\n");
    fflush(stdout);
#endif
    return result;
  } /* conOpen */
