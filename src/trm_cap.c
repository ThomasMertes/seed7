/********************************************************************/
/*                                                                  */
/*  trm_cap.c     Driver for termcap screen access.                 */
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
/*  File: seed7/src/trm_cap.c                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver for termcap screen access.                      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
/* #include "curses.h" */
/* #include "term.h" */

#include "common.h"
#include "os_decls.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "trm_drv.h"
#include "cap_def.h"

/* #define atexit(x) */



/* Configuration: */
#undef EMULATE_TERMCAP



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
int tputs (char *, int, int (*) (char ch));

#endif



#ifdef EMULATE_TERMCAP
#define tgetent  my_tgetent
#define tgetnum  my_tgetnum
#define tgetflag my_tgetflag
#define tgetstr  my_tgetstr

#define CAPBUF_SIZE 2048

char capabilities[CAPBUF_SIZE + NULL_TERMINATION_LEN];



int my_tgetent (char *capbuf, char *terminal_name)

  {
    FILE *term_descr_file;
    int len;
    int result;

  /* my_tgetent */
    if ((term_descr_file = fopen(terminal_name, "r")) != NULL) {
      len = fread(capbuf, 1, CAPBUF_SIZE, term_descr_file);
      capbuf[len] = '\0';
      fclose(term_descr_file);
      result = 0;
    } else {
      capbuf[0] = '\0';
      result = -1;
    } /* if */
    strcpy(capabilities, capbuf);
    return result;
  } /* my_tgetent */



int my_tgetnum (char *code)

  {
    char buffer[33];
    char *found;
    int result;

  /* my_tgetnum */
    result = -1;
    buffer[0] = ':';
    strcpy(&buffer[1], code);
    strcat(buffer, "#");
    if ((found = strstr(capabilities, buffer)) != NULL) {
      sscanf(found + 4, "%d", &result);
    } /* if */
#ifdef TRACE_CAPS
    printf("%s#%d\n", code, result);
#endif
    return result;
  } /* my_tgetnum */



int my_tgetflag (char *code)

  {
    char buffer[33];
    char *found;
    int result;

  /* my_tgetflag */
    result = FALSE;
    buffer[0] = ':';
    strcpy(&buffer[1], code);
    strcat(buffer, ":");
    if ((found = strstr(capabilities, buffer)) != NULL) {
      result = TRUE;
    } /* if */
#ifdef TRACE_CAPS
    printf("%s:%d\n", code, result);
#endif
    return result;
  } /* my_tgetflag */



char *my_tgetstr(char *code, char **area)

  {
    char buffer[33];
    char *found;
    char *end;
    char *from;
    char to_buf[513];
    char *to;
    char *result;

  /* my_tgetstr */
    result = NULL;
    buffer[0] = ':';
    strcpy(&buffer[1], code);
    strcat(buffer, "=");
    if ((found = strstr(capabilities, buffer)) != NULL) {
      if ((end = strchr(found + 4, ':')) != NULL) {
        from = found + 4;
        to = to_buf;
        while (from != end) {
          if (*from == '\\') {
            from++;
            if (*from == 'E' || *from == 'e') {
              *to++ = '\033';
            } else if (*from == 'n') {
              *to++ = '\n';
            } else if (*from == 'l') {
              *to++ = '\n';
            } else if (*from == 'r') {
              *to++ = '\r';
            } else if (*from == 't') {
              *to++ = '\t';
            } else if (*from == 'b') {
              *to++ = '\b';
            } else if (*from == 'f') {
              *to++ = '\f';
            } else if (*from == 's') {
              *to++ = ' ';
            } else if (*from == '0') {
              *to++ = '\200';
            } else {
              *to++ = *from;
            } /* if */
            from++;
          } else if (*from == '^') {
            from++;
            if (*from >= 'a' && *from <= 'z') {
              *to++ = *from - 'a' + 1;
            } else if (*from >= 'A' && *from <= 'Z') {
              *to++ = *from - 'A' + 1;
            } else {
              *to++ = *from;
            } /* if */
            from++;
          } else {
            *to++ = *from++;
          } /* if */
        } /* while */
        *to = '\0';
        if ((result = (char *) malloc(to - to_buf + 1)) != NULL) {
          strcpy(result, to_buf);
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CAPS
    printf("%s=\"%s\"\n", code, result);
#endif
    return result;
  } /* my_tgetstr */
#endif



int getcaps (void)

  {
    static char capbuf[CAPBUF_SIZE + NULL_TERMINATION_LEN];
    int return_code;
    char *area;
    char *home_dir_path;
    char *terminal_name;
    char term_descr_file_name[256];
    int len;

  /* getcaps */
    if (!caps_initialized) {
      terminal_name = getenv("TERM");
#ifdef EMULATE_TERMCAP
      home_dir_path = getenv("HOME");
      if (home_dir_path != NULL) {
        strcpy(term_descr_file_name, home_dir_path);
        len = strlen(term_descr_file_name);
      } else {
        len = 0;
      } /* if */
      if (len > 0 && term_descr_file_name[len - 1] != '/') {
        term_descr_file_name[len] = '/';
        len++;
      } /* if */
      strcpy(&term_descr_file_name[len], ".term");
      if (terminal_name != NULL) {
        strcat(term_descr_file_name, terminal_name);
      } /* if */
/*    setupterm(terminal_name, fileno(stdout), &err); */
/* printf(":%s:%s:\n", terminal_name, term_descr_file_name); */
      return_code = tgetent(capbuf, term_descr_file_name);
#else
      return_code = tgetent(capbuf, terminal_name);
#endif
/*
printf(":%d:\n", return_code);
printf(":%d:\n", strlen(capbuf));
printf("\"%s\"\n", capbuf);
*/
      area = capbuf;
      insert_line =           tgetstr("al", &area);
      auto_right_margin =    tgetflag("am");
      BC =                    tgetstr("bc", &area);
      clr_eos =               tgetstr("cd", &area);
      clr_eol =               tgetstr("ce", &area);
      clear_screen =          tgetstr("cl", &area);
      columns =               tgetnum("co");
      cursor_address =        tgetstr("cm", &area);
      carriage_return =       tgetstr("cr", &area);
      delete_character =      tgetstr("dc", &area);
      delete_line =           tgetstr("dl", &area);
      enter_delete_mode =     tgetstr("dm", &area);
      cursor_down =           tgetstr("do", &area);
      exit_delete_mode =      tgetstr("ed", &area);
      exit_insert_mode =      tgetstr("ei", &area);
      hard_copy =            tgetflag("hc");
      insert_character =      tgetstr("ic", &area);
      enter_insert_mode =     tgetstr("im", &area);
      IP =                    tgetstr("ip", &area);
      cursor_left =           tgetstr("le", &area);
      lines =                 tgetnum("li");
      move_insert_mode =     tgetflag("mi");
      cursor_right =          tgetstr("nd", &area);
      pad_char =              tgetstr("pc", &area);
      cursor_normal =         tgetstr("ve", &area);
      exit_standout_mode =    tgetstr("se", &area);
      enter_standout_mode =   tgetstr("so", &area);
      exit_ca_mode =          tgetstr("te", &area);
      enter_ca_mode =         tgetstr("ti", &area);
      cursor_up =             tgetstr("up", &area);
      cursor_visible =        tgetstr("vs", &area);
      cursor_invisible =      tgetstr("vi", &area);
      enter_reverse_mode =    tgetstr("mr", &area);
      ceol_standout_glitch = tgetflag("xs");
      parm_insert_line =      tgetstr("AL", &area);
      parm_delete_line =      tgetstr("DL", &area);
      keypad_local =          tgetstr("ke", &area);
      keypad_xmit =           tgetstr("ks", &area);
      key_backspace =         tgetstr("kb", &area);
      key_btab =              tgetstr("kB", &area);
      key_b2 =                tgetstr("K2", &area);
      key_catab =             tgetstr("ka", &area);
      key_clear =             tgetstr("kC", &area);
      key_ctab =              tgetstr("kt", &area);
      key_dc =                tgetstr("kD", &area);
      key_dl =                tgetstr("kL", &area);
      key_down =              tgetstr("kd", &area);
      key_eic =               tgetstr("kM", &area);
      key_end =               tgetstr("@7", &area);
      key_enter =             tgetstr("@8", &area);
      key_eol =               tgetstr("kE", &area);
      key_eos =               tgetstr("kS", &area);
      key_f0 =                tgetstr("k0", &area);
      key_f1 =                tgetstr("k1", &area);
      key_f2 =                tgetstr("k2", &area);
      key_f3 =                tgetstr("k3", &area);
      key_f4 =                tgetstr("k4", &area);
      key_f5 =                tgetstr("k5", &area);
      key_f6 =                tgetstr("k6", &area);
      key_f7 =                tgetstr("k7", &area);
      key_f8 =                tgetstr("k8", &area);
      key_f9 =                tgetstr("k9", &area);
      key_f10 =               tgetstr("k;", &area);
      key_f11 =               tgetstr("F1", &area);
      key_f12 =               tgetstr("F2", &area);
      key_f13 =               tgetstr("F3", &area);
      key_f14 =               tgetstr("F4", &area);
      key_f15 =               tgetstr("F5", &area);
      key_f16 =               tgetstr("F6", &area);
      key_f17 =               tgetstr("F7", &area);
      key_f18 =               tgetstr("F8", &area);
      key_f19 =               tgetstr("F9", &area);
      key_f20 =               tgetstr("FA", &area);
      key_f21 =               tgetstr("FB", &area);
      key_f22 =               tgetstr("FC", &area);
      key_f23 =               tgetstr("FD", &area);
      key_f24 =               tgetstr("FE", &area);
      key_f25 =               tgetstr("FF", &area);
      key_f26 =               tgetstr("FG", &area);
      key_f27 =               tgetstr("FH", &area);
      key_f28 =               tgetstr("FI", &area);
      key_f29 =               tgetstr("FJ", &area);
      key_f30 =               tgetstr("FK", &area);
      key_f30 =               tgetstr("FK", &area);
      key_f31 =               tgetstr("FL", &area);
      key_f32 =               tgetstr("FM", &area);
      key_f33 =               tgetstr("FN", &area);
      key_f34 =               tgetstr("FO", &area);
      key_f35 =               tgetstr("FP", &area);
      key_f36 =               tgetstr("FQ", &area);
      key_f37 =               tgetstr("FR", &area);
      key_f38 =               tgetstr("FS", &area);
      key_f39 =               tgetstr("FT", &area);
      key_f40 =               tgetstr("FU", &area);
      key_f41 =               tgetstr("FV", &area);
      key_f42 =               tgetstr("FW", &area);
      key_f43 =               tgetstr("FX", &area);
      key_f44 =               tgetstr("FY", &area);
      key_f45 =               tgetstr("FZ", &area);
      key_f46 =               tgetstr("Fa", &area);
      key_f47 =               tgetstr("Fb", &area);
      key_f48 =               tgetstr("Fc", &area);
      key_f49 =               tgetstr("Fd", &area);
      key_f50 =               tgetstr("Fe", &area);
      key_f51 =               tgetstr("Ff", &area);
      key_f52 =               tgetstr("Fg", &area);
      key_f53 =               tgetstr("Fh", &area);
      key_f54 =               tgetstr("Fi", &area);
      key_f55 =               tgetstr("Fj", &area);
      key_f56 =               tgetstr("Fk", &area);
      key_f57 =               tgetstr("Fl", &area);
      key_f58 =               tgetstr("Fm", &area);
      key_f59 =               tgetstr("Fn", &area);
      key_f60 =               tgetstr("Fo", &area);
      key_f61 =               tgetstr("Fp", &area);
      key_f62 =               tgetstr("Fq", &area);
      key_f63 =               tgetstr("Fr", &area);
      key_find =              tgetstr("@0", &area);
      key_home =              tgetstr("kh", &area);
      key_ic =                tgetstr("kI", &area);
      key_il =                tgetstr("kA", &area);
      key_left =              tgetstr("kl", &area);
      key_ll =                tgetstr("kH", &area);
      key_npage =             tgetstr("kN", &area);
      key_ppage =             tgetstr("kP", &area);
      key_right =             tgetstr("kr", &area);
      key_select=             tgetstr("*6", &area);
      key_sf =                tgetstr("kF", &area);
      key_sr =                tgetstr("kR", &area);
      key_stab =              tgetstr("kT", &area);
      key_up =                tgetstr("ku", &area);
      if (cursor_address == NULL) {
        printf("cursor_address == NULL\n");
      } /* if */
      caps_initialized = TRUE;
    } /* if */
/*  printf("%d <%s>\n", strlen(cursor_address), cursor_address); */
    return caps_initialized;
  } /* getcaps */



int outch (char ch)

  { /* outch */
    return putchar(ch);
  } /* outch */



void putcontrol (char *control)

  { /* putcontrol */
    if (control != NULL) {
      tputs(control, 1, outch);
    } /* if */
  } /* putcontrol */
