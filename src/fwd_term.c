/********************************************************************/
/*                                                                  */
/*  fwd_term.c    Forward calls to a shared terminfo library.       */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/fwd_term.c                                      */
/*  Changes: 2019  Thomas Mertes                                    */
/*  Content: Forward calls to a shared terminfo library.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#if FORWARD_TERM_CALLS
#include "stdio.h"

#include "common.h"
#include "striutl.h"
#include "dll_drv.h"

#undef EXTERN
#define EXTERN
#include "fwd_term.h"


#define CAPBUF_SIZE 2048


typedef int (*tp_setupterm) (const char *term, int filedes, int *errret);
#ifndef HAS_TERMIOS_H
typedef int (*tp_tcgetattr) (int fd, struct termios *termios_p);
typedef int (*tp_tcsetattr) (int fd, int actions,
                             const struct termios *termios_p);
#endif
typedef char *(*tp_tgoto) (const char *cap, int col, int row);
typedef int (*tp_tputs) (const char *str, int affcnt, int (*putc)(int));

typedef int (*tp_tgetent) (char *bp, const char *name);
typedef int (*tp_tgetflag) (const char *id);
typedef int (*tp_tgetnum) (const char *id);
typedef char *(*tp_tgetstr) (const char *id, char **area);


static tp_setupterm     ptr_setupterm;
#ifndef HAS_TERMIOS_H
static tp_tcgetattr     ptr_tcgetattr;
static tp_tcsetattr     ptr_tcsetattr;
#endif
static tp_tgoto         ptr_tgoto;
static tp_tputs         ptr_tputs;

static tp_tgetent       tgetent;
static tp_tgetflag      tgetflag;
static tp_tgetnum       tgetnum;
static tp_tgetstr       tgetstr;



static boolType setupDll (const char *dllName)

  {
    static void *x11Dll = NULL;

  /* setupDll */
    log2Function(fprintf(stderr, "setupDll(\"%s\")\n", dllName););
    if (x11Dll == NULL) {
      x11Dll = dllOpen(dllName);
      if (x11Dll != NULL) {
        if ((ptr_setupterm    = (tp_setupterm)    dllFunc(x11Dll, "setupterm"))    == NULL ||
#ifndef HAS_TERMIOS_H
            (ptr_tcgetattr    = (tp_tcgetattr)    dllFunc(x11Dll, "tcgetattr"))    == NULL ||
            (ptr_tcsetattr    = (tp_tcsetattr)    dllFunc(x11Dll, "tcsetattr"))    == NULL ||
#endif
            (ptr_tgoto        = (tp_tgoto)        dllFunc(x11Dll, "tgoto"))        == NULL ||
            (ptr_tputs        = (tp_tputs)        dllFunc(x11Dll, "tputs"))        == NULL ||
            (tgetent          = (tp_tgetent)      dllFunc(x11Dll, "tgetent"))      == NULL ||
            (tgetflag         = (tp_tgetflag)     dllFunc(x11Dll, "tgetflag"))     == NULL ||
            (tgetnum          = (tp_tgetnum)      dllFunc(x11Dll, "tgetnum"))      == NULL ||
            (tgetstr          = (tp_tgetstr)      dllFunc(x11Dll, "tgetstr"))      == NULL) {
          logError(printf("setupDll(\"%s\"): "
                          "Opened library successful but some functions are missing.\n",
                          dllName););
          x11Dll = NULL;
        } /* if */
      } /* if */
    } /* if */
    log2Function(fprintf(stderr, "setupDll --> %d\n",
                         x11Dll != NULL););
    return x11Dll != NULL;
  } /* setupDll */



boolType findTermDll (void)

  {
    const char *dllList[] = { CONSOLE_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findTermDll */
    log2Function(fprintf(stderr, "findTermDll()\n"););
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    log2Function(fprintf(stderr, "findTermDll --> %d\n", found););
    return found;
  } /* findTermDll */



static int loadCapabilities (const char *term)

  {
    static int caps_initialized = FALSE;
    static char capbuf[CAPBUF_SIZE + NULL_TERMINATION_LEN];
    int return_code;
    char *area;

  /* loadCapabilities */
    log2Function(fprintf(stderr, "loadCapabilities()\n"););
    if (!caps_initialized) {
      return_code = tgetent(NULL, term);
      /* printf(":%d:\n", return_code); */
      if (return_code == 1) {
        area = capbuf;
        insert_line =           tgetstr("al", &area);
        auto_right_margin =    tgetflag("am");
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
        insert_character =      tgetstr("ic", &area);
        enter_insert_mode =     tgetstr("im", &area);
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
        caps_initialized = TRUE;
      } /* if */
    } /* if */
    log2Function(fprintf(stderr, "loadCapabilities --> %d\n",
                         caps_initialized););
    return caps_initialized;
  } /* loadCapabilities */



int setupterm (const char *term, int filedes, int *errret)

  {
    int funcResult;

  /* setupterm */
    log2Function(fprintf(stderr, "setupterm(\"%s\", %d, *)\n",
                         term, filedes););
    /* funcResult = ptr_setupterm(term, filedes, errret); */
    funcResult = !loadCapabilities(term);
    log2Function(fprintf(stderr, "setupterm(\"%s\", %d, %d) --> %d\n",
                       term, filedes, *errret, funcResult););
    return funcResult;
  } /* setupterm */



#ifndef HAS_TERMIOS_H
int tcgetattr (int fd, struct termios *termios_p)

  {
    int funcResult;

  /* tcgetattr */
    log2Function(fprintf(stderr, "tcgetattr(%d, *)\n", fd););
    funcResult = ptr_tcgetattr(fd, termios_p);
    log2Function(fprintf(stderr, "tcgetattr --> %d\n", funcResult););
    return funcResult;
  } /* tcgetattr */



int tcsetattr (int fd, int actions, const struct termios *termios_p)

  {
    int funcResult;

  /* tcsetattr */
    log2Function(fprintf(stderr, "tcsetattr(%d, %d, *)\n",
                         fd, actions););
    funcResult = ptr_tcsetattr(fd, actions, termios_p);
    log2Function(fprintf(stderr, "tcsetattr --> %d\n", funcResult););
    return funcResult;
  } /* tcsetattr */
#endif



char *tgoto (const char *cap, int col, int row)

  {
    char *str;

  /* tgoto */
    log2Function(fprintf(stderr, "tgoto(\"%s\", %d, %d)\n",
                         cstriAsUnquotedCLiteral(cap), col, row););
    str = ptr_tgoto(cap, col, row);
    log2Function(fprintf(stderr, "tgoto --> \"%s\"\n",
                         cstriAsUnquotedCLiteral(str)););
    return str;
  } /* tgoto */



int tputs (const char *str, int affcnt, int (*putc)(int))

  {
    int funcResult;

  /* tputs */
    log2Function(fprintf(stderr, "tputs(\"%s\", %d, " FMT_U_MEM ")\n",
			 cstriAsUnquotedCLiteral(str), affcnt,
                         (memSizeType) putc););
    funcResult = ptr_tputs(str, affcnt, putc);
    log2Function(fprintf(stderr, "tputs --> %d\n", funcResult););
    return funcResult;
  } /* tputs */

#endif
