/********************************************************************/
/*                                                                  */
/*  trm_cap.c     Driver for termcap screen access.                 */
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
/*  File: seed7/src/trm_cap.c                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver for termcap screen access.                      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#ifdef USE_TERMCAP
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
/* #include "curses.h" */
/* #include "term.h" */

#include "common.h"
#include "os_decls.h"
#include "striutl.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "trm_drv.h"
#include "cap_def.h"


/* Configuration: */
#define EMULATE_TERMCAP 0

#define CAPBUF_SIZE 2048


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

/* In old implementations the values below might differ. */
#define TGETENT_SUCCESS      1
#define TGETENT_NO_ENTRY     0
#define TGETENT_NO_TERMCAP (-1)



#if EMULATE_TERMCAP
#define tgetent  my_tgetent
#define tgetnum  my_tgetnum
#define tgetflag my_tgetflag
#define tgetstr  my_tgetstr

#define SEARCHED_BUFFER_SIZE    33
#define CAP_VALUE_BUFFER_SIZE 1024

char *capabilities = NULL;



int my_tgetent (char *capbuf, char *terminal_name)

  {
    char *home_dir_path;
    memSizeType file_name_size;
    char *file_name;
    size_t len;
    FILE *term_descr_file;
    long end_pos;
    size_t bytes_read;
    int tgetent_result = TGETENT_NO_ENTRY;

  /* my_tgetent */
    logFunction(printf("my_tgetent(" FMT_U_MEM ", \"%s\")\n",
                (memSizeType) capbuf, terminal_name););
    home_dir_path = getenv("HOME");
    if (home_dir_path == NULL) {
      home_dir_path = "";
    } /* if */
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
      if ((term_descr_file = fopen(file_name, "r")) != NULL) {
        if (fseek(term_descr_file, 0L, SEEK_END) == 0) {
          end_pos = ftell(term_descr_file);
          /* printf("end_pos=%ld\n", end_pos); */
          if (end_pos >= 0L && end_pos < 2147483647L &&
              fseek(term_descr_file, 0L, SEEK_SET) == 0) {
            if (capabilities != NULL) {
              free(capabilities);
            } /* if */
            capabilities = malloc((size_t) end_pos + 1);
            if (capabilities != NULL) {
              bytes_read = fread(capabilities, 1, (size_t) end_pos, term_descr_file);
              if (bytes_read == end_pos) {
                capabilities[bytes_read] = '\0';
                /* printf("bytes_read=%lu\n", (unsigned long) bytes_read); */
                /* printf("%s\n", capabilities); */
                tgetent_result = TGETENT_SUCCESS;
              } else {
                free(capabilities);
                capabilities = NULL;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
        fclose(term_descr_file);
      } /* if */
      free(file_name);
    } /* if */
    logFunction(printf("my_tgetent --> %d\n", tgetent_result););
    return tgetent_result;
  } /* my_tgetent */



int my_tgetnum (char *code)

  {
    memSizeType pos = 1;
    char searched[SEARCHED_BUFFER_SIZE];
    char *found;
    int cap_value = -1;

  /* my_tgetnum */
    logFunction(printf("my_tgetnum(\"%s\")\n", code););
    searched[0] = ':';
    while (code[pos - 1] != '\0' && pos < sizeof(searched) - 2) {
      searched[pos] = code[pos - 1];
      pos++;
    } /* while */
    searched[pos] = '#';
    pos++;
    searched[pos] = '\0';
    if (capabilities != NULL &&
        (found = strstr(capabilities, searched)) != NULL) {
      sscanf(found + pos, "%d", &cap_value);
    } /* if */
    logFunction(printf("my_tgetnum(\"%s\") --> %d\n",
                       code, cap_value););
    return cap_value;
  } /* my_tgetnum */



int my_tgetflag (char *code)

  {
    memSizeType pos = 1;
    char searched[SEARCHED_BUFFER_SIZE];
    char *found;
    int cap_value = FALSE;

  /* my_tgetflag */
    logFunction(printf("my_tgetflag(\"%s\")\n", code););
    searched[0] = ':';
    while (code[pos - 1] != '\0' && pos < sizeof(searched) - 2) {
      searched[pos] = code[pos - 1];
      pos++;
    } /* while */
    searched[pos] = ':';
    pos++;
    searched[pos] = '\0';
    if (capabilities != NULL &&
        (found = strstr(capabilities, searched)) != NULL) {
      cap_value = TRUE;
    } /* if */
    logFunction(printf("my_tgetflag(\"%s\") --> %d\n",
                       code, cap_value););
    return cap_value;
  } /* my_tgetflag */



char *my_tgetstr(char *code, char **area)

  {
    memSizeType pos = 1;
    char searched[SEARCHED_BUFFER_SIZE];
    char *found;
    char *end;
    char *from;
    char value[CAP_VALUE_BUFFER_SIZE];
    char *cap_value = NULL;

  /* my_tgetstr */
    logFunction(printf("my_tgetstr(\"%s\", %s%s" FMT_U_MEM ")\n",
                       code,
                       area == NULL ? "NULL " : "",
                       area != NULL && *area == NULL ?
                           "(NULL) " : "",
                       area == NULL ?
                           (memSizeType) 0 : (memSizeType) *area););
    searched[0] = ':';
    while (code[pos - 1] != '\0' && pos < sizeof(searched) - 2) {
      searched[pos] = code[pos - 1];
      pos++;
    } /* while */
    searched[pos] = '=';
    pos++;
    searched[pos] = '\0';
    if (capabilities != NULL &&
        (found = strstr(capabilities, searched)) != NULL) {
      if ((end = strchr(found + pos, ':')) != NULL) {
        from = found + pos;
        pos = 0;
        while (from != end && pos < CAP_VALUE_BUFFER_SIZE) {
          if (*from == '\\') {
            from++;
            if (from != end) {
              switch (*from) {
                case 'E':
                case 'e': value[pos] = '\033'; break;
                case 'n':
                case 'l': value[pos] = '\n';   break;
                case 'r': value[pos] = '\r';   break;
                case 't': value[pos] = '\t';   break;
                case 'b': value[pos] = '\b';   break;
                case 'f': value[pos] = '\f';   break;
                case 's': value[pos] = ' ';    break;
                case '0': value[pos] = '\200'; break;
                default:  value[pos] = *from;  break;
              } /* switch */
              from++;
            } else {
              value[pos] = '\\';
            } /* if */
          } else if (*from == '^') {
            from++;
            if (from != end) {
              if (*from >= 'a' && *from <= 'z') {
                value[pos] = *from - 'a' + 1;
              } else if (*from >= 'A' && *from <= 'Z') {
                value[pos] = *from - 'A' + 1;
              } else {
                value[pos] = *from;
              } /* if */
              from++;
            } else {
              value[pos] = '^';
            } /* if */
          } else {
            value[pos] = *from++;
          } /* if */
          pos++;
        } /* while */
        if (pos < CAP_VALUE_BUFFER_SIZE) {
          value[pos] = '\0';
          if ((cap_value = (char *) malloc(pos + 1)) != NULL) {
            strcpy(cap_value, value);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("my_tgetstr(\"%s\", %s%s" FMT_U_MEM ") --> ",
                       code,
                       area == NULL ? "NULL " : "",
                       area != NULL && *area == NULL ?
                           "(NULL) " : "",
                       area == NULL ?
                       (memSizeType) 0 : (memSizeType) *area);
                if (cap_value == NULL) {
                  printf("NULL\n");
                } else {
                  printf("\"%s\"\n",
                         cstriAsUnquotedCLiteral(cap_value));
		});
    return cap_value;
  } /* my_tgetstr */
#endif



#if ANY_LOG_ACTIVE
void printCapEscapedStri (const char *cap_value)

  { /* printCapEscapedStri */
    if (cap_value != NULL) {
      while (*cap_value != '\0') {
        switch (*cap_value) {
          case '\033': printf("\\e"); break;
          case '\n':   printf("\\n"); break;
          case '\r':   printf("\\r"); break;
          case '\t':   printf("\\t"); break;
          case '\b':   printf("\\b"); break;
          case '\f':   printf("\\f"); break;
          case ' ':    printf("\\s"); break;
          case ':':    printf("\\:"); break;
          case '\200': printf("\\0"); break;
          default:
            if (*cap_value <= '\032') {
              printf("^%c", *cap_value - 1 + 'A');
            } else {
              printf("%c", *cap_value);
            } /* if */
            break;
        } /* switch */
        cap_value++;
      } /* while */
    } /* if */
  } /* printCapEscapedStri */



void printStriCap (const char *const cap_name,
                   const char *const cap_value)

  { /* printStriCap */
    if (cap_value != NULL) {
      printf("%s=", cap_name);
      printCapEscapedStri(cap_value);
      printf(":");
    } /* if */
  } /* printStriCap */



void showCapabilities (const char *const terminal_name)

  { /* showCapabilities */
    printf("%s:",    terminal_name);
    printStriCap("al", insert_line);
    printf("%s",       auto_right_margin ? "am:" : "");
    printStriCap("bc", BC);
    printStriCap("cd", clr_eos);
    printStriCap("ce", clr_eol);
    printStriCap("cl", clear_screen);
    printf("co#%d",    columns);
    printStriCap("cm", cursor_address);
    printStriCap("cr", carriage_return);
    printStriCap("dc", delete_character);
    printStriCap("dl", delete_line);
    printStriCap("dm", enter_delete_mode);
    printStriCap("do", cursor_down);
    printStriCap("ed", exit_delete_mode);
    printStriCap("ei", exit_insert_mode);
    printf("%s",       hard_copy ? "hc:" : "");
    printStriCap("ic", insert_character);
    printStriCap("im", enter_insert_mode);
    printStriCap("ip", IP);
    printStriCap("le", cursor_left);
    printf("li#%d",    lines);
    printf("%s",       move_insert_mode ? "mi:" : "");
    printStriCap("nd", cursor_right);
    printStriCap("pc", pad_char);
    printStriCap("ve", cursor_normal);
    printStriCap("se", exit_standout_mode);
    printStriCap("so", enter_standout_mode);
    printStriCap("te", exit_ca_mode);
    printStriCap("ti", enter_ca_mode);
    printStriCap("up", cursor_up);
    printStriCap("vs", cursor_visible);
    printStriCap("vi", cursor_invisible);
    printStriCap("mr", enter_reverse_mode);
    printf("%s",       ceol_standout_glitch ? "xs:" : "");
    printStriCap("AL", parm_insert_line);
    printStriCap("DL", parm_delete_line);
    printStriCap("ke", keypad_local);
    printStriCap("ks", keypad_xmit);
    printStriCap("kb", key_backspace);
    printStriCap("kB", key_btab);
    printStriCap("K2", key_b2);
    printStriCap("ka", key_catab);
    printStriCap("kC", key_clear);
    printStriCap("kt", key_ctab);
    printStriCap("kD", key_dc);
    printStriCap("kL", key_dl);
    printStriCap("kd", key_down);
    printStriCap("kM", key_eic);
    printStriCap("@7", key_end);
    printStriCap("@8", key_enter);
    printStriCap("kE", key_eol);
    printStriCap("kS", key_eos);
    printStriCap("k0", key_f0);
    printStriCap("k1", key_f1);
    printStriCap("k2", key_f2);
    printStriCap("k3", key_f3);
    printStriCap("k4", key_f4);
    printStriCap("k5", key_f5);
    printStriCap("k6", key_f6);
    printStriCap("k7", key_f7);
    printStriCap("k8", key_f8);
    printStriCap("k9", key_f9);
    printStriCap("k;", key_f10);
    printStriCap("F1", key_f11);
    printStriCap("F2", key_f12);
    printStriCap("F3", key_f13);
    printStriCap("F4", key_f14);
    printStriCap("F5", key_f15);
    printStriCap("F6", key_f16);
    printStriCap("F7", key_f17);
    printStriCap("F8", key_f18);
    printStriCap("F9", key_f19);
    printStriCap("FA", key_f20);
    printStriCap("FB", key_f21);
    printStriCap("FC", key_f22);
    printStriCap("FD", key_f23);
    printStriCap("FE", key_f24);
    printStriCap("FF", key_f25);
    printStriCap("FG", key_f26);
    printStriCap("FH", key_f27);
    printStriCap("FI", key_f28);
    printStriCap("FJ", key_f29);
    printStriCap("FK", key_f30);
    printStriCap("FK", key_f30);
    printStriCap("FL", key_f31);
    printStriCap("FM", key_f32);
    printStriCap("FN", key_f33);
    printStriCap("FO", key_f34);
    printStriCap("FP", key_f35);
    printStriCap("FQ", key_f36);
    printStriCap("FR", key_f37);
    printStriCap("FS", key_f38);
    printStriCap("FT", key_f39);
    printStriCap("FU", key_f40);
    printStriCap("FV", key_f41);
    printStriCap("FW", key_f42);
    printStriCap("FX", key_f43);
    printStriCap("FY", key_f44);
    printStriCap("FZ", key_f45);
    printStriCap("Fa", key_f46);
    printStriCap("Fb", key_f47);
    printStriCap("Fc", key_f48);
    printStriCap("Fd", key_f49);
    printStriCap("Fe", key_f50);
    printStriCap("Ff", key_f51);
    printStriCap("Fg", key_f52);
    printStriCap("Fh", key_f53);
    printStriCap("Fi", key_f54);
    printStriCap("Fj", key_f55);
    printStriCap("Fk", key_f56);
    printStriCap("Fl", key_f57);
    printStriCap("Fm", key_f58);
    printStriCap("Fn", key_f59);
    printStriCap("Fo", key_f60);
    printStriCap("Fp", key_f61);
    printStriCap("Fq", key_f62);
    printStriCap("Fr", key_f63);
    printStriCap("@0", key_find);
    printStriCap("kh", key_home);
    printStriCap("kI", key_ic);
    printStriCap("kA", key_il);
    printStriCap("kl", key_left);
    printStriCap("kH", key_ll);
    printStriCap("kN", key_npage);
    printStriCap("kP", key_ppage);
    printStriCap("kr", key_right);
    printStriCap("*6", key_select);
    printStriCap("kF", key_sf);
    printStriCap("kR", key_sr);
    printStriCap("kT", key_stab);
    printStriCap("ku", key_up);
    printf("\n");
  } /* showCapabilities */
#endif



int getcaps (void)

  {
    static char capbuf[CAPBUF_SIZE];
    static char areabuf[CAPBUF_SIZE];
    int tgetent_result;
    char *area;
    char *terminal_name;

  /* getcaps */
    logFunction(printf("getcaps (caps_initialized=%d)\n",
                       caps_initialized););
    if (!caps_initialized) {
      terminal_name = getenv("TERM");
      if (terminal_name != NULL) {
        tgetent_result = tgetent(capbuf, terminal_name);
        /* printf("tgetent -> %d\n", tgetent_result); */
        if (tgetent_result == TGETENT_SUCCESS) {
          area = areabuf;
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
          key_select =            tgetstr("*6", &area);
          key_sf =                tgetstr("kF", &area);
          key_sr =                tgetstr("kR", &area);
          key_stab =              tgetstr("kT", &area);
          key_up =                tgetstr("ku", &area);
          if (cursor_address == NULL) {
            printf("cursor_address == NULL\n");
          } /* if */
          /* showCapabilities(terminal_name); */
          caps_initialized = TRUE;
        } /* if */
      } /* if */
    } /* if */
    /* printf("%d <%s>\n", strlen(cursor_address), cursor_address); */
    logFunction(printf("getcaps --> %d\n", caps_initialized););
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

#endif
