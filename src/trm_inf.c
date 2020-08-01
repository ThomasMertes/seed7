/********************************************************************/
/*                                                                  */
/*  trm_inf.c     Driver for terminfo screen access                 */
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
/*  File: seed7/src/trm_inf.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Driver for terminfo screen access                      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

#include "common.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "trm_drv.h"

#define SETUPTERM_WORKS_OK


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



#ifdef ANSI_C

static void read_cap_name (FILE *fix_file, char *cap_name, int *term_char)
#else

static void read_cap_name (fix_file, cap_name, term_char)
FILE *fix_file;
char *cap_name;
int *term_char;
#endif

  {
    int ch;

  /* read_cap_name */
    do {
      ch = fgetc(fix_file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
    while (ch != ':' && ch != ',' &&
        ch != '=' && ch != '#' && ch != '|' && ch != EOF) {
      *cap_name++ = ch;
      ch = fgetc(fix_file);
    } /* while */
    *cap_name = '\0';
    *term_char = ch;
  } /* read_cap_name */



#ifdef ANSI_C

static void read_int_cap (FILE *fix_file, char **cap_value, int *term_char)
#else

static void read_int_cap (fix_file, cap_value, term_char)
FILE *fix_file;
char **cap_value;
int *term_char;
#endif

  {
    int from;

  /* read_int_cap */
    from = fgetc(fix_file);
    while (from != ',' && from != ':') {
      from = fgetc(fix_file);
    } /* while */
    *term_char = from;
  } /* read_int_cap */



#ifdef ANSI_C

static void read_stri_cap (FILE *fix_file, char **cap_value, int *term_char)
#else

static void read_stri_cap (fix_file, cap_value, term_char)
FILE *fix_file;
char **cap_value;
int *term_char;
#endif

  {
    int from;
    char to_buf[513];
    char *to;

  /* read_stri_cap */
    from = fgetc(fix_file);
    to = to_buf;
    while (from != ',' && from != ':' && from != EOF) {
      if (from == '\\') {
        from = fgetc(fix_file);
        if (from == 'E' || from == 'e') {
          *to++ = '\033';
        } else if (from == 'n') {
          *to++ = '\n';
        } else if (from == 'l') {
          *to++ = '\n';
        } else if (from == 'r') {
          *to++ = '\r';
        } else if (from == 't') {
          *to++ = '\t';
        } else if (from == 'b') {
          *to++ = '\b';
        } else if (from == 'f') {
          *to++ = '\f';
        } else if (from == 's') {
          *to++ = ' ';
        } else if (from == '0') {
          *to++ = '\200';
        } else {
          *to++ = from;
        } /* if */
        from = fgetc(fix_file);
      } else if (from == '^') {
        from = fgetc(fix_file);
        if (from >= 'a' && from <= 'z') {
          *to++ = from - 'a' + 1;
        } else if (from >= 'A' && from <= 'Z') {
          *to++ = from - 'A' + 1;
        } else {
          *to++ = from;
        } /* if */
        from = fgetc(fix_file);
      } else {
        *to++ = from;
        from = fgetc(fix_file);
      } /* if */
    } /* while */
    if (to == to_buf) {
      *cap_value = NULL;
    } else {
      *to = '\0';
      if ((*cap_value = (char *) malloc((size_t) (to - to_buf + 1))) != NULL) {
        strcpy(*cap_value, to_buf);
      } /* if */
    } /* if */
    *term_char = from;
  } /* read_stri_cap */



#ifdef ANSI_C

static int assign_cap (char *cap_name, char *terminfo_name,
    char *termcap_name, char **cap_pointer, char *cap_value)
#else

static int assign_cap (cap_name, terminfo_name,
    termcap_name, cap_pointer, cap_value)
char *cap_name;
char *terminfo_name;
char *termcap_name;
char **cap_pointer;
char *cap_value;
#endif

  { /* assign_cap */
    if (cap_name != NULL &&
        (strcmp(cap_name, terminfo_name) == 0 ||
        strcmp(cap_name, termcap_name) == 0)) {
#ifdef TRACE_CAPS
      fprintf(stderr, "%s=%s:\n", cap_name, cap_value);
#endif
      *cap_pointer = cap_value;
      return(TRUE);
    } else {
      return(FALSE);
    } /* if */
  } /* assign_cap */



#ifdef ANSI_C

static void fix_capability (void)
#else

static void fix_capability ()
#endif

  {
    char *home_dir_path;
    char *terminal_name;
    char fix_file_name[256];
    FILE *fix_file;
    char cap_name[256];
    char *cap_value;
    int term_char;
    int len;

  /* fix_capability */
#ifdef TRACE_CAPS
    fprintf(stderr, "BEGIN fix_capability\n");
#endif
    home_dir_path = getenv("HOME");
    terminal_name = getenv("TERM");
    if (home_dir_path != NULL) {
      strcpy(fix_file_name, home_dir_path);
    } else {
      fix_file_name[0] = '\0';
    } /* if */
    len = strlen(fix_file_name);
    if (len > 0 && fix_file_name[len - 1] != '/') {
      strcat(fix_file_name, "/");
    } /* if */
    strcat(fix_file_name, ".term");
    if (terminal_name != NULL) {
      strcat(fix_file_name, terminal_name);
    } /* if */
    if ((fix_file = fopen(fix_file_name, "r")) != NULL) {
      do {
        read_cap_name(fix_file, cap_name, &term_char);
      } while (term_char != ',' && term_char != ':' && term_char != EOF);
      read_cap_name(fix_file, cap_name, &term_char);
#ifdef TRACE_CAPS
      fprintf(stderr, "cap \"%s\" ", cap_name);
#endif
      while (term_char != EOF) {
        cap_value = NULL;
        switch (term_char) {
          case ',':
          case ':':
            break;
          case '#':
            read_int_cap(fix_file, &cap_value, &term_char);
            break;
          case '=':
            read_stri_cap(fix_file, &cap_value, &term_char);
            break;
        } /* switch */
        if (assign_cap(cap_name, "rmkx",  "ke", &keypad_local, cap_value) ||
            assign_cap(cap_name, "smkx",  "ks", &keypad_xmit,  cap_value) ||
            assign_cap(cap_name, "kcbt",  "kB", &key_btab,     cap_value) ||
            assign_cap(cap_name, "kdch1", "kD", &key_dc,       cap_value) ||
            assign_cap(cap_name, "kcud1", "kd", &key_down,     cap_value) ||
            assign_cap(cap_name, "khome", "kh", &key_home,     cap_value) ||
            assign_cap(cap_name, "kich1", "kI", &key_ic,       cap_value) ||
            assign_cap(cap_name, "kcub1", "kl", &key_left,     cap_value) ||
            assign_cap(cap_name, "knp",   "kN", &key_npage,    cap_value) ||
            assign_cap(cap_name, "kpp",   "kP", &key_ppage,    cap_value) ||
            assign_cap(cap_name, "kind",  "kF", &key_sf,       cap_value) ||
            assign_cap(cap_name, "kri",   "kR", &key_sr,       cap_value) ||
            assign_cap(cap_name, "kcuf1", "kr", &key_right,    cap_value) ||
            assign_cap(cap_name, "kcuu1", "ku", &key_up,       cap_value) ||
            assign_cap(cap_name, "kend",  "@7", &key_end,      cap_value) ||
            assign_cap(cap_name, "kb2",   "K2", &key_b2,       cap_value) ||
            assign_cap(cap_name, "kf0",   "k0", &key_f0,       cap_value) ||
            assign_cap(cap_name, "kf1",   "k1", &key_f1,       cap_value) ||
            assign_cap(cap_name, "kf2",   "k2", &key_f2,       cap_value) ||
            assign_cap(cap_name, "kf3",   "k3", &key_f3,       cap_value) ||
            assign_cap(cap_name, "kf4",   "k4", &key_f4,       cap_value) ||
            assign_cap(cap_name, "kf5",   "k5", &key_f5,       cap_value) ||
            assign_cap(cap_name, "kf6",   "k6", &key_f6,       cap_value) ||
            assign_cap(cap_name, "kf7",   "k7", &key_f7,       cap_value) ||
            assign_cap(cap_name, "kf8",   "k8", &key_f8,       cap_value) ||
            assign_cap(cap_name, "kf9",   "k9", &key_f9,       cap_value) ||
            assign_cap(cap_name, "kf10",  "k;", &key_f10,      cap_value) ||
            assign_cap(cap_name, "kf11",  "F1", &key_f11,      cap_value) ||
            assign_cap(cap_name, "kf12",  "F2", &key_f12,      cap_value) ||
            assign_cap(cap_name, "kf13",  "F3", &key_f13,      cap_value) ||
            assign_cap(cap_name, "kf14",  "F4", &key_f14,      cap_value) ||
            assign_cap(cap_name, "kf15",  "F5", &key_f15,      cap_value) ||
            assign_cap(cap_name, "kf16",  "F6", &key_f16,      cap_value) ||
            assign_cap(cap_name, "kf17",  "F7", &key_f17,      cap_value) ||
            assign_cap(cap_name, "kf18",  "F8", &key_f18,      cap_value) ||
            assign_cap(cap_name, "kf19",  "F9", &key_f19,      cap_value) ||
            assign_cap(cap_name, "kf20",  "FA", &key_f20,      cap_value) ||
            assign_cap(cap_name, "kf21",  "FB", &key_f21,      cap_value) ||
            assign_cap(cap_name, "kf22",  "FC", &key_f22,      cap_value) ||
            assign_cap(cap_name, "kf23",  "FD", &key_f23,      cap_value) ||
            assign_cap(cap_name, "kf24",  "FE", &key_f24,      cap_value) ||
            assign_cap(cap_name, "kf25",  "FF", &key_f25,      cap_value) ||
            assign_cap(cap_name, "kf26",  "FG", &key_f26,      cap_value) ||
            assign_cap(cap_name, "kf27",  "FH", &key_f27,      cap_value) ||
            assign_cap(cap_name, "kf28",  "FI", &key_f28,      cap_value) ||
            assign_cap(cap_name, "kf29",  "FJ", &key_f29,      cap_value) ||
            assign_cap(cap_name, "kf30",  "FK", &key_f30,      cap_value)) {
        } /* if */
        read_cap_name(fix_file, cap_name, &term_char);
#ifdef TRACE_CAPS
      fprintf(stderr, "cap \"%s\" ", cap_name);
#endif
      } /* while */
      fclose(fix_file);
    } /* if */
#ifdef TRACE_CAPS
    fprintf(stderr, "END fix_capability\n");
#endif
  } /* fix_capability */



#ifdef ANSI_C

int getcaps (void)
#else

int getcaps ()
#endif

  {
    char *terminal_name;
    int setup_result;
    int errret;

  /* getcaps */
#ifdef TRACE_CAPS
    fprintf(stderr, "BEGIN getcaps\n");
#endif
    if (!caps_initialized) {
      if ((terminal_name = getenv("TERM")) != NULL) {
#ifdef TRACE_CAPS
        fprintf(stderr, "TERM = \"%s\"\n", terminal_name);
#endif
        errret = 1;
        setup_result = setupterm(terminal_name, fileno(stdout), &errret);
#ifdef TRACE_CAPS
        fprintf(stderr, "setupterm => %d  errret = %d\n",
            setup_result, errret);
#endif
#ifdef SETUPTERM_WORKS_OK
        if (setup_result == 0 /*OK*/  &&  errret == 1) {
#endif
          fix_capability();
          caps_initialized = TRUE;
#ifdef SETUPTERM_WORKS_OK
        } /* if */
#endif
      } /* if */
    } /* if */
#ifdef TRACE_CAPS
    fprintf(stderr, "END getcaps ==> %d\n", caps_initialized);
#endif
    return(caps_initialized);
  } /* getcaps */



#ifdef ANSI_C

int outch (int ch)
#else

int outch (ch)
char ch;
#endif

  { /* outch */
    return(putchar(ch));
  } /* outch */



#ifdef ANSI_C

void putcontrol (char *control)
#else

void putcontrol (control)
char *control;
#endif

  { /* putcontrol */
    if (control != NULL) {
      tputs(control, 1, outch);
    } /* if */
  } /* putcontrol */
