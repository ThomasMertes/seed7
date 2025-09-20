/********************************************************************/
/*                                                                  */
/*  trm_inf.c     Driver for terminfo screen access.                */
/*  Copyright (C) 1989 - 2005, 2013, 2015, 2019  Thomas Mertes      */
/*                2025  Thomas Mertes                               */
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
/*  Content: Driver for terminfo screen access.                     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#ifdef USE_TERMINFO
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#ifdef TERM_INCLUDE
#include TERM_INCLUDE
#ifdef HAS_TERMIOS_H
#include "termios.h"
#endif
#else
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
#include "os_decls.h"
#include "striutl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "trm_drv.h"


/* Configuration: */
#define USE_FIX_CAPABILITY 0
#define SETUPTERM_WORKS_OK 1

#define CAP_NAME_BUFFER_SIZE   256
#define CAP_VALUE_BUFFER_SIZE 1024



#if USE_FIX_CAPABILITY
static void read_cap_name (FILE *fix_file, char *const cap_name, int *term_char)

  {
    memSizeType pos = 0;
    int ch;

  /* read_cap_name */
    do {
      ch = fgetc(fix_file);
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
    while (ch != ':' && ch != ',' &&
        ch != '=' && ch != '#' && ch != '|' && ch != EOF) {
      if (pos < CAP_NAME_BUFFER_SIZE - 1) {
        cap_name[pos] = (char) ch;
        pos++;
      } /* if */
      ch = fgetc(fix_file);
    } /* while */
    cap_name[pos] = '\0';
    *term_char = ch;
    log2Function(fprintf(stderr, "read_cap_name(" FMT_X_MEM ", \"%s\", %d) -->\n",
                         (memSizeType) fix_file,
                         cap_name, *term_char););
  } /* read_cap_name */



static int read_int_cap (FILE *fix_file, int *term_char)

  {
    memSizeType pos = 0;
    char value[CAP_VALUE_BUFFER_SIZE];
    int from;
    int cap_value = -1;

  /* read_int_cap */
    from = fgetc(fix_file);
    while (from != ',' && from != ':' && from != EOF) {
      if (pos < CAP_VALUE_BUFFER_SIZE) {
        value[pos] = (char) from;
        pos++;
      } /* if */
      from = fgetc(fix_file);
    } /* while */
    if (pos < CAP_VALUE_BUFFER_SIZE) {
      value[pos] = '\0';
      sscanf(value, "%d", &cap_value);
    } /* if */
    *term_char = from;
    log2Function(fprintf(stderr, "read_int_cap --> %d\n", cap_value););
    return cap_value;
  } /* read_int_cap */



static char *read_stri_cap (FILE *fix_file, int *term_char)

  {
    memSizeType pos = 0;
    char value[CAP_VALUE_BUFFER_SIZE];
    int from;
    char *cap_value = NULL;

  /* read_stri_cap */
    from = fgetc(fix_file);
    while (from != ',' && from != ':' && from != EOF) {
      if (from == '\\') {
        from = fgetc(fix_file);
        if (pos < CAP_VALUE_BUFFER_SIZE) {
          switch (from) {
            case 'E':
            case 'e': value[pos] = '\033';      break;
            case 'n':
            case 'l': value[pos] = '\n';        break;
            case 'r': value[pos] = '\r';        break;
            case 't': value[pos] = '\t';        break;
            case 'b': value[pos] = '\b';        break;
            case 'f': value[pos] = '\f';        break;
            case 's': value[pos] = ' ';         break;
            case '0': value[pos] = '\200';      break;
            case EOF: value[pos] = '\\';        break;
            default:  value[pos] = (char) from; break;
          } /* switch */
          pos++;
        } /* if */
      } else if (from == '^') {
        from = fgetc(fix_file);
        if (pos < CAP_VALUE_BUFFER_SIZE) {
          if (from >= 'a' && from <= 'z') {
            value[pos] = (char) (from - 'a' + 1);
          } else if (from >= 'A' && from <= 'Z') {
            value[pos] = (char) (from - 'A' + 1);
          } else if (from == EOF) {
            value[pos] = '^';
          } else {
            value[pos] = (char) from;
          } /* if */
          pos++;
        } /* if */
      } else {
        if (pos < CAP_VALUE_BUFFER_SIZE) {
          value[pos] = (char) from;
          pos++;
        } /* if */
      } /* if */
      from = fgetc(fix_file);
    } /* while */
    if (pos < CAP_VALUE_BUFFER_SIZE) {
      value[pos] = '\0';
      if ((cap_value = (char *) malloc(pos + 1)) != NULL) {
        strcpy(cap_value, value);
      } /* if */
    } /* if */
    *term_char = from;
    log2Function(fprintf(stderr, "read_stri_cap --> ");
                 if (cap_value == NULL) {
                   fprintf(stderr, "NULL\n");
                 } else {
                   fprintf(stderr, "\"%s\"\n",
                           cstriAsUnquotedCLiteral(cap_value));
                 });
    return cap_value;
  } /* read_stri_cap */



static int assign_cap (char *cap_name, const_cstriType terminfo_name,
    const_cstriType termcap_name, char **cap_pointer, char *cap_value)

  {
    int assigned;

  /* assign_cap */
    /* fprintf(stderr, "assign_cap(\"%s\", \"%s\", \"%s\", *, *)\n",
            cap_name, terminfo_name, termcap_name); */
    if (cap_name != NULL &&
        (strcmp(cap_name, terminfo_name) == 0 ||
        strcmp(cap_name, termcap_name) == 0)) {
      logMessage(fprintf(stderr, "%s=\"%s\"\n", cap_name,
                         cstriAsUnquotedCLiteral(cap_value)););
      *cap_pointer = cap_value;
      assigned = TRUE;
    } else {
      assigned = FALSE;
    } /* if */
    /* fprintf(stderr, "assign_cap -->%d\n", assigned); */
    return assigned;
  } /* assign_cap */



static void fix_capability (void)

  {
    char *home_dir_path;
    char *terminal_name;
    memSizeType file_name_size;
    char *file_name;
    FILE *fix_file;
    char cap_name[CAP_NAME_BUFFER_SIZE];
    char *cap_value;
    int term_char;
    size_t len;

  /* fix_capability */
    log2Function(fprintf(stderr, "fix_capability\n"););
    home_dir_path = getenv("HOME");
    if (home_dir_path == NULL) {
      home_dir_path = "";
    } /* if */
    terminal_name = getenv("TERM");
    if (terminal_name == NULL) {
      terminal_name = "";
    } /* if */
    file_name_size = strlen(home_dir_path) + 6 + strlen(terminal_name);
    file_name = malloc(file_name_size + 1);
    if (file_name != NULL) {
      strcpy(file_name, home_dir_path);
      len = strlen(file_name);
      if (len > 0 && file_name[len - 1] != '/') {
        file_name[len] = '/';
        len++;
      } /* if */
      strcpy(&file_name[len], ".term");
      strcat(file_name, terminal_name);
      if ((fix_file = fopen(file_name, "r")) != NULL) {
        do {
          read_cap_name(fix_file, cap_name, &term_char);
        } while (term_char != ',' && term_char != ':' && term_char != EOF);
        read_cap_name(fix_file, cap_name, &term_char);
        logMessage(fprintf(stderr, "cap \"%s\" ", cap_name););
        while (term_char != EOF) {
          cap_value = NULL;
          switch (term_char) {
            case ',':
            case ':':
              break;
            case '#':
              read_int_cap(fix_file, &term_char);
              break;
            case '=':
              cap_value = read_stri_cap(fix_file, &term_char);
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
              assign_cap(cap_name, "kf30",  "FK", &key_f30,      cap_value) ||
              assign_cap(cap_name, "kf31",  "FL", &key_f31,      cap_value) ||
              assign_cap(cap_name, "kf32",  "FM", &key_f32,      cap_value) ||
              assign_cap(cap_name, "kf33",  "FN", &key_f33,      cap_value) ||
              assign_cap(cap_name, "kf34",  "FO", &key_f34,      cap_value) ||
              assign_cap(cap_name, "kf35",  "FP", &key_f35,      cap_value) ||
              assign_cap(cap_name, "kf36",  "FQ", &key_f36,      cap_value) ||
              assign_cap(cap_name, "kf37",  "FR", &key_f37,      cap_value) ||
              assign_cap(cap_name, "kf38",  "FS", &key_f38,      cap_value) ||
              assign_cap(cap_name, "kf39",  "FT", &key_f39,      cap_value) ||
              assign_cap(cap_name, "kf40",  "FU", &key_f40,      cap_value) ||
              assign_cap(cap_name, "kf41",  "FV", &key_f41,      cap_value) ||
              assign_cap(cap_name, "kf42",  "FW", &key_f42,      cap_value) ||
              assign_cap(cap_name, "kf43",  "FX", &key_f43,      cap_value) ||
              assign_cap(cap_name, "kf44",  "FY", &key_f44,      cap_value) ||
              assign_cap(cap_name, "kf45",  "FZ", &key_f45,      cap_value) ||
              assign_cap(cap_name, "kf46",  "Fa", &key_f46,      cap_value) ||
              assign_cap(cap_name, "kf47",  "Fb", &key_f47,      cap_value) ||
              assign_cap(cap_name, "kf48",  "Fc", &key_f48,      cap_value) ||
              assign_cap(cap_name, "kf49",  "Fd", &key_f49,      cap_value) ||
              assign_cap(cap_name, "kf50",  "Fe", &key_f50,      cap_value) ||
              assign_cap(cap_name, "kf51",  "Ff", &key_f51,      cap_value) ||
              assign_cap(cap_name, "kf52",  "Fg", &key_f52,      cap_value) ||
              assign_cap(cap_name, "kf53",  "Fh", &key_f53,      cap_value) ||
              assign_cap(cap_name, "kf54",  "Fi", &key_f54,      cap_value) ||
              assign_cap(cap_name, "kf55",  "Fj", &key_f55,      cap_value) ||
              assign_cap(cap_name, "kf56",  "Fk", &key_f56,      cap_value) ||
              assign_cap(cap_name, "kf57",  "Fl", &key_f57,      cap_value) ||
              assign_cap(cap_name, "kf58",  "Fm", &key_f58,      cap_value) ||
              assign_cap(cap_name, "kf59",  "Fn", &key_f59,      cap_value) ||
              assign_cap(cap_name, "kf60",  "Fo", &key_f60,      cap_value) ||
              assign_cap(cap_name, "kf61",  "Fp", &key_f61,      cap_value) ||
              assign_cap(cap_name, "kf62",  "Fq", &key_f62,      cap_value) ||
              assign_cap(cap_name, "kf63",  "Fr", &key_f63,      cap_value)) {
          } /* if */
          read_cap_name(fix_file, cap_name, &term_char);
          logMessage(fprintf(stderr, "cap \"%s\" ", cap_name););
        } /* while */
        fclose(fix_file);
      } /* if */
      free(file_name);
    } /* if */
    log2Function(fprintf(stderr, "fix_capability -->\n"););
  } /* fix_capability */
#endif



int getcaps (void)

  {
    char *terminal_name;
    int setup_result;
    int errret;

  /* getcaps */
    log2Function(fprintf(stderr, "getcaps (caps_initialized=%d)\n",
                         caps_initialized););
    if (!caps_initialized) {
      if ((terminal_name = getenv("TERM")) != NULL) {
        logMessage(fprintf(stderr, "TERM = \"%s\"\n", terminal_name););
        errret = 1;
        setup_result = setupterm(terminal_name, os_fileno(stdout), &errret);
        logMessage(fprintf(stderr, "setupterm --> %d  errret = %d\n",
                           setup_result, errret););
#if SETUPTERM_WORKS_OK
        if (setup_result == 0 /*OK*/  &&  errret == 1) {
#endif
#if USE_FIX_CAPABILITY
          fix_capability();
#endif
          caps_initialized = TRUE;
#if SETUPTERM_WORKS_OK
        } /* if */
#endif
      } /* if */
    } /* if */
    log2Function(fprintf(stderr, "getcaps --> %d\n", caps_initialized););
    return caps_initialized;
  } /* getcaps */



int outch (int ch)

  { /* outch */
    return putchar(ch);
  } /* outch */



void putcontrol (char *control)

  { /* putcontrol */
    if (control != NULL) {
      tputs(control, 1, outch);
    } /* if */
  } /* putcontrol */

#endif
