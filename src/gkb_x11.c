/********************************************************************/
/*                                                                  */
/*  gkb_x11.c     Keyboard and mouse access with X11 capabilities.  */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  File: seed7/src/gkb_x11.c                                       */
/*  Changes: 1994, 2000 - 2011  Thomas Mertes                       */
/*  Content: Keyboard and mouse access with X11 capabilities.       */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef OUT_OF_ORDER
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#endif
#include <X11/keysym.h>

#include "common.h"
#include "data_rtl.h"
#include "hsh_rtl.h"
#include "kbd_drv.h"

#undef FLAG_EVENTS

#define ALLOW_REPARENT_NOTIFY


extern Display *mydisplay;
extern Atom wm_delete_window;
static boolType eventPresent = FALSE;
static XEvent currentEvent;
static intType button_x = 0;
static intType button_y = 0;
static Window button_window = 0;
static rtlHashType window_hash = NULL;


extern void redraw (winType redraw_window, int xPos, int yPos, int width, int height);
extern void doFlush (void);
extern void flushBeforeRead (void);


struct keysymCharPair {
  uint16Type keysym;
  uint16Type unicodeChar;
};

/* The table below maps keysym values to Unicode. The     */
/* table does not use the XK_ macros from the include     */
/* file <X11/keysymdef.h>. This is on purpose. It avoids  */
/* syntax errors, when an older <X11/keysymdef.h> does    */
/* not define a macro. This kind of mapping is not used   */
/* for newer Unicode characters. Newer Unicode characters */
/* use a direct mapping (keysym - 0x01000000) instead.    */
/* The XK_ macros will probably either stay unchanged, or */
/* change to the new direct mapped value. Direct mapped   */
/* keysyms are already handled by mapKeysymToUnicode().   */
/* The table is sorted by keysym.                         */

static const struct keysymCharPair keysymTable[] = {
    {0x01a1, 0x0104}, /* XK_Aogonek                     -> LATIN CAPITAL LETTER A WITH OGONEK */
    {0x01a2, 0x02d8}, /* XK_breve                       -> BREVE */
    {0x01a3, 0x0141}, /* XK_Lstroke                     -> LATIN CAPITAL LETTER L WITH STROKE */
    {0x01a5, 0x013d}, /* XK_Lcaron                      -> LATIN CAPITAL LETTER L WITH CARON */
    {0x01a6, 0x015a}, /* XK_Sacute                      -> LATIN CAPITAL LETTER S WITH ACUTE */
    {0x01a9, 0x0160}, /* XK_Scaron                      -> LATIN CAPITAL LETTER S WITH CARON */
    {0x01aa, 0x015e}, /* XK_Scedilla                    -> LATIN CAPITAL LETTER S WITH CEDILLA */
    {0x01ab, 0x0164}, /* XK_Tcaron                      -> LATIN CAPITAL LETTER T WITH CARON */
    {0x01ac, 0x0179}, /* XK_Zacute                      -> LATIN CAPITAL LETTER Z WITH ACUTE */
    {0x01ae, 0x017d}, /* XK_Zcaron                      -> LATIN CAPITAL LETTER Z WITH CARON */
    {0x01af, 0x017b}, /* XK_Zabovedot                   -> LATIN CAPITAL LETTER Z WITH DOT ABOVE */
    {0x01b1, 0x0105}, /* XK_aogonek                     -> LATIN SMALL LETTER A WITH OGONEK */
    {0x01b2, 0x02db}, /* XK_ogonek                      -> OGONEK */
    {0x01b3, 0x0142}, /* XK_lstroke                     -> LATIN SMALL LETTER L WITH STROKE */
    {0x01b5, 0x013e}, /* XK_lcaron                      -> LATIN SMALL LETTER L WITH CARON */
    {0x01b6, 0x015b}, /* XK_sacute                      -> LATIN SMALL LETTER S WITH ACUTE */
    {0x01b7, 0x02c7}, /* XK_caron                       -> CARON */
    {0x01b9, 0x0161}, /* XK_scaron                      -> LATIN SMALL LETTER S WITH CARON */
    {0x01ba, 0x015f}, /* XK_scedilla                    -> LATIN SMALL LETTER S WITH CEDILLA */
    {0x01bb, 0x0165}, /* XK_tcaron                      -> LATIN SMALL LETTER T WITH CARON */
    {0x01bc, 0x017a}, /* XK_zacute                      -> LATIN SMALL LETTER Z WITH ACUTE */
    {0x01bd, 0x02dd}, /* XK_doubleacute                 -> DOUBLE ACUTE ACCENT */
    {0x01be, 0x017e}, /* XK_zcaron                      -> LATIN SMALL LETTER Z WITH CARON */
    {0x01bf, 0x017c}, /* XK_zabovedot                   -> LATIN SMALL LETTER Z WITH DOT ABOVE */
    {0x01c0, 0x0154}, /* XK_Racute                      -> LATIN CAPITAL LETTER R WITH ACUTE */
    {0x01c3, 0x0102}, /* XK_Abreve                      -> LATIN CAPITAL LETTER A WITH BREVE */
    {0x01c5, 0x0139}, /* XK_Lacute                      -> LATIN CAPITAL LETTER L WITH ACUTE */
    {0x01c6, 0x0106}, /* XK_Cacute                      -> LATIN CAPITAL LETTER C WITH ACUTE */
    {0x01c8, 0x010c}, /* XK_Ccaron                      -> LATIN CAPITAL LETTER C WITH CARON */
    {0x01ca, 0x0118}, /* XK_Eogonek                     -> LATIN CAPITAL LETTER E WITH OGONEK */
    {0x01cc, 0x011a}, /* XK_Ecaron                      -> LATIN CAPITAL LETTER E WITH CARON */
    {0x01cf, 0x010e}, /* XK_Dcaron                      -> LATIN CAPITAL LETTER D WITH CARON */
    {0x01d0, 0x0110}, /* XK_Dstroke                     -> LATIN CAPITAL LETTER D WITH STROKE */
    {0x01d1, 0x0143}, /* XK_Nacute                      -> LATIN CAPITAL LETTER N WITH ACUTE */
    {0x01d2, 0x0147}, /* XK_Ncaron                      -> LATIN CAPITAL LETTER N WITH CARON */
    {0x01d5, 0x0150}, /* XK_Odoubleacute                -> LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
    {0x01d8, 0x0158}, /* XK_Rcaron                      -> LATIN CAPITAL LETTER R WITH CARON */
    {0x01d9, 0x016e}, /* XK_Uring                       -> LATIN CAPITAL LETTER U WITH RING ABOVE */
    {0x01db, 0x0170}, /* XK_Udoubleacute                -> LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
    {0x01de, 0x0162}, /* XK_Tcedilla                    -> LATIN CAPITAL LETTER T WITH CEDILLA */
    {0x01e0, 0x0155}, /* XK_racute                      -> LATIN SMALL LETTER R WITH ACUTE */
    {0x01e3, 0x0103}, /* XK_abreve                      -> LATIN SMALL LETTER A WITH BREVE */
    {0x01e5, 0x013a}, /* XK_lacute                      -> LATIN SMALL LETTER L WITH ACUTE */
    {0x01e6, 0x0107}, /* XK_cacute                      -> LATIN SMALL LETTER C WITH ACUTE */
    {0x01e8, 0x010d}, /* XK_ccaron                      -> LATIN SMALL LETTER C WITH CARON */
    {0x01ea, 0x0119}, /* XK_eogonek                     -> LATIN SMALL LETTER E WITH OGONEK */
    {0x01ec, 0x011b}, /* XK_ecaron                      -> LATIN SMALL LETTER E WITH CARON */
    {0x01ef, 0x010f}, /* XK_dcaron                      -> LATIN SMALL LETTER D WITH CARON */
    {0x01f0, 0x0111}, /* XK_dstroke                     -> LATIN SMALL LETTER D WITH STROKE */
    {0x01f1, 0x0144}, /* XK_nacute                      -> LATIN SMALL LETTER N WITH ACUTE */
    {0x01f2, 0x0148}, /* XK_ncaron                      -> LATIN SMALL LETTER N WITH CARON */
    {0x01f5, 0x0151}, /* XK_odoubleacute                -> LATIN SMALL LETTER O WITH DOUBLE ACUTE */
    {0x01f8, 0x0159}, /* XK_rcaron                      -> LATIN SMALL LETTER R WITH CARON */
    {0x01f9, 0x016f}, /* XK_uring                       -> LATIN SMALL LETTER U WITH RING ABOVE */
    {0x01fb, 0x0171}, /* XK_udoubleacute                -> LATIN SMALL LETTER U WITH DOUBLE ACUTE */
    {0x01fe, 0x0163}, /* XK_tcedilla                    -> LATIN SMALL LETTER T WITH CEDILLA */
    {0x01ff, 0x02d9}, /* XK_abovedot                    -> DOT ABOVE */
    {0x02a1, 0x0126}, /* XK_Hstroke                     -> LATIN CAPITAL LETTER H WITH STROKE */
    {0x02a6, 0x0124}, /* XK_Hcircumflex                 -> LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
    {0x02a9, 0x0130}, /* XK_Iabovedot                   -> LATIN CAPITAL LETTER I WITH DOT ABOVE */
    {0x02ab, 0x011e}, /* XK_Gbreve                      -> LATIN CAPITAL LETTER G WITH BREVE */
    {0x02ac, 0x0134}, /* XK_Jcircumflex                 -> LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
    {0x02b1, 0x0127}, /* XK_hstroke                     -> LATIN SMALL LETTER H WITH STROKE */
    {0x02b6, 0x0125}, /* XK_hcircumflex                 -> LATIN SMALL LETTER H WITH CIRCUMFLEX */
    {0x02b9, 0x0131}, /* XK_idotless                    -> LATIN SMALL LETTER DOTLESS I */
    {0x02bb, 0x011f}, /* XK_gbreve                      -> LATIN SMALL LETTER G WITH BREVE */
    {0x02bc, 0x0135}, /* XK_jcircumflex                 -> LATIN SMALL LETTER J WITH CIRCUMFLEX */
    {0x02c5, 0x010a}, /* XK_Cabovedot                   -> LATIN CAPITAL LETTER C WITH DOT ABOVE */
    {0x02c6, 0x0108}, /* XK_Ccircumflex                 -> LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
    {0x02d5, 0x0120}, /* XK_Gabovedot                   -> LATIN CAPITAL LETTER G WITH DOT ABOVE */
    {0x02d8, 0x011c}, /* XK_Gcircumflex                 -> LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
    {0x02dd, 0x016c}, /* XK_Ubreve                      -> LATIN CAPITAL LETTER U WITH BREVE */
    {0x02de, 0x015c}, /* XK_Scircumflex                 -> LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
    {0x02e5, 0x010b}, /* XK_cabovedot                   -> LATIN SMALL LETTER C WITH DOT ABOVE */
    {0x02e6, 0x0109}, /* XK_ccircumflex                 -> LATIN SMALL LETTER C WITH CIRCUMFLEX */
    {0x02f5, 0x0121}, /* XK_gabovedot                   -> LATIN SMALL LETTER G WITH DOT ABOVE */
    {0x02f8, 0x011d}, /* XK_gcircumflex                 -> LATIN SMALL LETTER G WITH CIRCUMFLEX */
    {0x02fd, 0x016d}, /* XK_ubreve                      -> LATIN SMALL LETTER U WITH BREVE */
    {0x02fe, 0x015d}, /* XK_scircumflex                 -> LATIN SMALL LETTER S WITH CIRCUMFLEX */
    {0x03a2, 0x0138}, /* XK_kra                         -> LATIN SMALL LETTER KRA */
    {0x03a3, 0x0156}, /* XK_Rcedilla                    -> LATIN CAPITAL LETTER R WITH CEDILLA */
    {0x03a5, 0x0128}, /* XK_Itilde                      -> LATIN CAPITAL LETTER I WITH TILDE */
    {0x03a6, 0x013b}, /* XK_Lcedilla                    -> LATIN CAPITAL LETTER L WITH CEDILLA */
    {0x03aa, 0x0112}, /* XK_Emacron                     -> LATIN CAPITAL LETTER E WITH MACRON */
    {0x03ab, 0x0122}, /* XK_Gcedilla                    -> LATIN CAPITAL LETTER G WITH CEDILLA */
    {0x03ac, 0x0166}, /* XK_Tslash                      -> LATIN CAPITAL LETTER T WITH STROKE */
    {0x03b3, 0x0157}, /* XK_rcedilla                    -> LATIN SMALL LETTER R WITH CEDILLA */
    {0x03b5, 0x0129}, /* XK_itilde                      -> LATIN SMALL LETTER I WITH TILDE */
    {0x03b6, 0x013c}, /* XK_lcedilla                    -> LATIN SMALL LETTER L WITH CEDILLA */
    {0x03ba, 0x0113}, /* XK_emacron                     -> LATIN SMALL LETTER E WITH MACRON */
    {0x03bb, 0x0123}, /* XK_gcedilla                    -> LATIN SMALL LETTER G WITH CEDILLA */
    {0x03bc, 0x0167}, /* XK_tslash                      -> LATIN SMALL LETTER T WITH STROKE */
    {0x03bd, 0x014a}, /* XK_ENG                         -> LATIN CAPITAL LETTER ENG */
    {0x03bf, 0x014b}, /* XK_eng                         -> LATIN SMALL LETTER ENG */
    {0x03c0, 0x0100}, /* XK_Amacron                     -> LATIN CAPITAL LETTER A WITH MACRON */
    {0x03c7, 0x012e}, /* XK_Iogonek                     -> LATIN CAPITAL LETTER I WITH OGONEK */
    {0x03cc, 0x0116}, /* XK_Eabovedot                   -> LATIN CAPITAL LETTER E WITH DOT ABOVE */
    {0x03cf, 0x012a}, /* XK_Imacron                     -> LATIN CAPITAL LETTER I WITH MACRON */
    {0x03d1, 0x0145}, /* XK_Ncedilla                    -> LATIN CAPITAL LETTER N WITH CEDILLA */
    {0x03d2, 0x014c}, /* XK_Omacron                     -> LATIN CAPITAL LETTER O WITH MACRON */
    {0x03d3, 0x0136}, /* XK_Kcedilla                    -> LATIN CAPITAL LETTER K WITH CEDILLA */
    {0x03d9, 0x0172}, /* XK_Uogonek                     -> LATIN CAPITAL LETTER U WITH OGONEK */
    {0x03dd, 0x0168}, /* XK_Utilde                      -> LATIN CAPITAL LETTER U WITH TILDE */
    {0x03de, 0x016a}, /* XK_Umacron                     -> LATIN CAPITAL LETTER U WITH MACRON */
    {0x03e0, 0x0101}, /* XK_amacron                     -> LATIN SMALL LETTER A WITH MACRON */
    {0x03e7, 0x012f}, /* XK_iogonek                     -> LATIN SMALL LETTER I WITH OGONEK */
    {0x03ec, 0x0117}, /* XK_eabovedot                   -> LATIN SMALL LETTER E WITH DOT ABOVE */
    {0x03ef, 0x012b}, /* XK_imacron                     -> LATIN SMALL LETTER I WITH MACRON */
    {0x03f1, 0x0146}, /* XK_ncedilla                    -> LATIN SMALL LETTER N WITH CEDILLA */
    {0x03f2, 0x014d}, /* XK_omacron                     -> LATIN SMALL LETTER O WITH MACRON */
    {0x03f3, 0x0137}, /* XK_kcedilla                    -> LATIN SMALL LETTER K WITH CEDILLA */
    {0x03f9, 0x0173}, /* XK_uogonek                     -> LATIN SMALL LETTER U WITH OGONEK */
    {0x03fd, 0x0169}, /* XK_utilde                      -> LATIN SMALL LETTER U WITH TILDE */
    {0x03fe, 0x016b}, /* XK_umacron                     -> LATIN SMALL LETTER U WITH MACRON */
    {0x047e, 0x203e}, /* XK_overline                    -> OVERLINE */
    {0x04a1, 0x3002}, /* XK_kana_fullstop               -> IDEOGRAPHIC FULL STOP */
    {0x04a2, 0x300c}, /* XK_kana_openingbracket         -> LEFT CORNER BRACKET */
    {0x04a3, 0x300d}, /* XK_kana_closingbracket         -> RIGHT CORNER BRACKET */
    {0x04a4, 0x3001}, /* XK_kana_comma                  -> IDEOGRAPHIC COMMA */
    {0x04a5, 0x30fb}, /* XK_kana_conjunctive            -> KATAKANA MIDDLE DOT */
    {0x04a6, 0x30f2}, /* XK_kana_WO                     -> KATAKANA LETTER WO */
    {0x04a7, 0x30a1}, /* XK_kana_a                      -> KATAKANA LETTER SMALL A */
    {0x04a8, 0x30a3}, /* XK_kana_i                      -> KATAKANA LETTER SMALL I */
    {0x04a9, 0x30a5}, /* XK_kana_u                      -> KATAKANA LETTER SMALL U */
    {0x04aa, 0x30a7}, /* XK_kana_e                      -> KATAKANA LETTER SMALL E */
    {0x04ab, 0x30a9}, /* XK_kana_o                      -> KATAKANA LETTER SMALL O */
    {0x04ac, 0x30e3}, /* XK_kana_ya                     -> KATAKANA LETTER SMALL YA */
    {0x04ad, 0x30e5}, /* XK_kana_yu                     -> KATAKANA LETTER SMALL YU */
    {0x04ae, 0x30e7}, /* XK_kana_yo                     -> KATAKANA LETTER SMALL YO */
    {0x04af, 0x30c3}, /* XK_kana_tsu                    -> KATAKANA LETTER SMALL TU */
    {0x04b0, 0x30fc}, /* XK_prolongedsound              -> KATAKANA-HIRAGANA PROLONGED SOUND MARK */
    {0x04b1, 0x30a2}, /* XK_kana_A                      -> KATAKANA LETTER A */
    {0x04b2, 0x30a4}, /* XK_kana_I                      -> KATAKANA LETTER I */
    {0x04b3, 0x30a6}, /* XK_kana_U                      -> KATAKANA LETTER U */
    {0x04b4, 0x30a8}, /* XK_kana_E                      -> KATAKANA LETTER E */
    {0x04b5, 0x30aa}, /* XK_kana_O                      -> KATAKANA LETTER O */
    {0x04b6, 0x30ab}, /* XK_kana_KA                     -> KATAKANA LETTER KA */
    {0x04b7, 0x30ad}, /* XK_kana_KI                     -> KATAKANA LETTER KI */
    {0x04b8, 0x30af}, /* XK_kana_KU                     -> KATAKANA LETTER KU */
    {0x04b9, 0x30b1}, /* XK_kana_KE                     -> KATAKANA LETTER KE */
    {0x04ba, 0x30b3}, /* XK_kana_KO                     -> KATAKANA LETTER KO */
    {0x04bb, 0x30b5}, /* XK_kana_SA                     -> KATAKANA LETTER SA */
    {0x04bc, 0x30b7}, /* XK_kana_SHI                    -> KATAKANA LETTER SI */
    {0x04bd, 0x30b9}, /* XK_kana_SU                     -> KATAKANA LETTER SU */
    {0x04be, 0x30bb}, /* XK_kana_SE                     -> KATAKANA LETTER SE */
    {0x04bf, 0x30bd}, /* XK_kana_SO                     -> KATAKANA LETTER SO */
    {0x04c0, 0x30bf}, /* XK_kana_TA                     -> KATAKANA LETTER TA */
    {0x04c1, 0x30c1}, /* XK_kana_CHI                    -> KATAKANA LETTER TI */
    {0x04c2, 0x30c4}, /* XK_kana_TSU                    -> KATAKANA LETTER TU */
    {0x04c3, 0x30c6}, /* XK_kana_TE                     -> KATAKANA LETTER TE */
    {0x04c4, 0x30c8}, /* XK_kana_TO                     -> KATAKANA LETTER TO */
    {0x04c5, 0x30ca}, /* XK_kana_NA                     -> KATAKANA LETTER NA */
    {0x04c6, 0x30cb}, /* XK_kana_NI                     -> KATAKANA LETTER NI */
    {0x04c7, 0x30cc}, /* XK_kana_NU                     -> KATAKANA LETTER NU */
    {0x04c8, 0x30cd}, /* XK_kana_NE                     -> KATAKANA LETTER NE */
    {0x04c9, 0x30ce}, /* XK_kana_NO                     -> KATAKANA LETTER NO */
    {0x04ca, 0x30cf}, /* XK_kana_HA                     -> KATAKANA LETTER HA */
    {0x04cb, 0x30d2}, /* XK_kana_HI                     -> KATAKANA LETTER HI */
    {0x04cc, 0x30d5}, /* XK_kana_FU                     -> KATAKANA LETTER HU */
    {0x04cd, 0x30d8}, /* XK_kana_HE                     -> KATAKANA LETTER HE */
    {0x04ce, 0x30db}, /* XK_kana_HO                     -> KATAKANA LETTER HO */
    {0x04cf, 0x30de}, /* XK_kana_MA                     -> KATAKANA LETTER MA */
    {0x04d0, 0x30df}, /* XK_kana_MI                     -> KATAKANA LETTER MI */
    {0x04d1, 0x30e0}, /* XK_kana_MU                     -> KATAKANA LETTER MU */
    {0x04d2, 0x30e1}, /* XK_kana_ME                     -> KATAKANA LETTER ME */
    {0x04d3, 0x30e2}, /* XK_kana_MO                     -> KATAKANA LETTER MO */
    {0x04d4, 0x30e4}, /* XK_kana_YA                     -> KATAKANA LETTER YA */
    {0x04d5, 0x30e6}, /* XK_kana_YU                     -> KATAKANA LETTER YU */
    {0x04d6, 0x30e8}, /* XK_kana_YO                     -> KATAKANA LETTER YO */
    {0x04d7, 0x30e9}, /* XK_kana_RA                     -> KATAKANA LETTER RA */
    {0x04d8, 0x30ea}, /* XK_kana_RI                     -> KATAKANA LETTER RI */
    {0x04d9, 0x30eb}, /* XK_kana_RU                     -> KATAKANA LETTER RU */
    {0x04da, 0x30ec}, /* XK_kana_RE                     -> KATAKANA LETTER RE */
    {0x04db, 0x30ed}, /* XK_kana_RO                     -> KATAKANA LETTER RO */
    {0x04dc, 0x30ef}, /* XK_kana_WA                     -> KATAKANA LETTER WA */
    {0x04dd, 0x30f3}, /* XK_kana_N                      -> KATAKANA LETTER N */
    {0x04de, 0x309b}, /* XK_voicedsound                 -> KATAKANA-HIRAGANA VOICED SOUND MARK */
    {0x04df, 0x309c}, /* XK_semivoicedsound             -> KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
    {0x05ac, 0x060c}, /* XK_Arabic_comma                -> ARABIC COMMA */
    {0x05bb, 0x061b}, /* XK_Arabic_semicolon            -> ARABIC SEMICOLON */
    {0x05bf, 0x061f}, /* XK_Arabic_question_mark        -> ARABIC QUESTION MARK */
    {0x05c1, 0x0621}, /* XK_Arabic_hamza                -> ARABIC LETTER HAMZA */
    {0x05c2, 0x0622}, /* XK_Arabic_maddaonalef          -> ARABIC LETTER ALEF WITH MADDA ABOVE */
    {0x05c3, 0x0623}, /* XK_Arabic_hamzaonalef          -> ARABIC LETTER ALEF WITH HAMZA ABOVE */
    {0x05c4, 0x0624}, /* XK_Arabic_hamzaonwaw           -> ARABIC LETTER WAW WITH HAMZA ABOVE */
    {0x05c5, 0x0625}, /* XK_Arabic_hamzaunderalef       -> ARABIC LETTER ALEF WITH HAMZA BELOW */
    {0x05c6, 0x0626}, /* XK_Arabic_hamzaonyeh           -> ARABIC LETTER YEH WITH HAMZA ABOVE */
    {0x05c7, 0x0627}, /* XK_Arabic_alef                 -> ARABIC LETTER ALEF */
    {0x05c8, 0x0628}, /* XK_Arabic_beh                  -> ARABIC LETTER BEH */
    {0x05c9, 0x0629}, /* XK_Arabic_tehmarbuta           -> ARABIC LETTER TEH MARBUTA */
    {0x05ca, 0x062a}, /* XK_Arabic_teh                  -> ARABIC LETTER TEH */
    {0x05cb, 0x062b}, /* XK_Arabic_theh                 -> ARABIC LETTER THEH */
    {0x05cc, 0x062c}, /* XK_Arabic_jeem                 -> ARABIC LETTER JEEM */
    {0x05cd, 0x062d}, /* XK_Arabic_hah                  -> ARABIC LETTER HAH */
    {0x05ce, 0x062e}, /* XK_Arabic_khah                 -> ARABIC LETTER KHAH */
    {0x05cf, 0x062f}, /* XK_Arabic_dal                  -> ARABIC LETTER DAL */
    {0x05d0, 0x0630}, /* XK_Arabic_thal                 -> ARABIC LETTER THAL */
    {0x05d1, 0x0631}, /* XK_Arabic_ra                   -> ARABIC LETTER REH */
    {0x05d2, 0x0632}, /* XK_Arabic_zain                 -> ARABIC LETTER ZAIN */
    {0x05d3, 0x0633}, /* XK_Arabic_seen                 -> ARABIC LETTER SEEN */
    {0x05d4, 0x0634}, /* XK_Arabic_sheen                -> ARABIC LETTER SHEEN */
    {0x05d5, 0x0635}, /* XK_Arabic_sad                  -> ARABIC LETTER SAD */
    {0x05d6, 0x0636}, /* XK_Arabic_dad                  -> ARABIC LETTER DAD */
    {0x05d7, 0x0637}, /* XK_Arabic_tah                  -> ARABIC LETTER TAH */
    {0x05d8, 0x0638}, /* XK_Arabic_zah                  -> ARABIC LETTER ZAH */
    {0x05d9, 0x0639}, /* XK_Arabic_ain                  -> ARABIC LETTER AIN */
    {0x05da, 0x063a}, /* XK_Arabic_ghain                -> ARABIC LETTER GHAIN */
    {0x05e0, 0x0640}, /* XK_Arabic_tatweel              -> ARABIC TATWEEL */
    {0x05e1, 0x0641}, /* XK_Arabic_feh                  -> ARABIC LETTER FEH */
    {0x05e2, 0x0642}, /* XK_Arabic_qaf                  -> ARABIC LETTER QAF */
    {0x05e3, 0x0643}, /* XK_Arabic_kaf                  -> ARABIC LETTER KAF */
    {0x05e4, 0x0644}, /* XK_Arabic_lam                  -> ARABIC LETTER LAM */
    {0x05e5, 0x0645}, /* XK_Arabic_meem                 -> ARABIC LETTER MEEM */
    {0x05e6, 0x0646}, /* XK_Arabic_noon                 -> ARABIC LETTER NOON */
    {0x05e7, 0x0647}, /* XK_Arabic_ha                   -> ARABIC LETTER HEH */
    {0x05e8, 0x0648}, /* XK_Arabic_waw                  -> ARABIC LETTER WAW */
    {0x05e9, 0x0649}, /* XK_Arabic_alefmaksura          -> ARABIC LETTER ALEF MAKSURA */
    {0x05ea, 0x064a}, /* XK_Arabic_yeh                  -> ARABIC LETTER YEH */
    {0x05eb, 0x064b}, /* XK_Arabic_fathatan             -> ARABIC FATHATAN */
    {0x05ec, 0x064c}, /* XK_Arabic_dammatan             -> ARABIC DAMMATAN */
    {0x05ed, 0x064d}, /* XK_Arabic_kasratan             -> ARABIC KASRATAN */
    {0x05ee, 0x064e}, /* XK_Arabic_fatha                -> ARABIC FATHA */
    {0x05ef, 0x064f}, /* XK_Arabic_damma                -> ARABIC DAMMA */
    {0x05f0, 0x0650}, /* XK_Arabic_kasra                -> ARABIC KASRA */
    {0x05f1, 0x0651}, /* XK_Arabic_shadda               -> ARABIC SHADDA */
    {0x05f2, 0x0652}, /* XK_Arabic_sukun                -> ARABIC SUKUN */
    {0x06a1, 0x0452}, /* XK_Serbian_dje                 -> CYRILLIC SMALL LETTER DJE */
    {0x06a2, 0x0453}, /* XK_Macedonia_gje               -> CYRILLIC SMALL LETTER GJE */
    {0x06a3, 0x0451}, /* XK_Cyrillic_io                 -> CYRILLIC SMALL LETTER IO */
    {0x06a4, 0x0454}, /* XK_Ukrainian_ie                -> CYRILLIC SMALL LETTER UKRAINIAN IE */
    {0x06a5, 0x0455}, /* XK_Macedonia_dse               -> CYRILLIC SMALL LETTER DZE */
    {0x06a6, 0x0456}, /* XK_Ukrainian_i                 -> CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
    {0x06a7, 0x0457}, /* XK_Ukrainian_yi                -> CYRILLIC SMALL LETTER YI */
    {0x06a8, 0x0458}, /* XK_Cyrillic_je                 -> CYRILLIC SMALL LETTER JE */
    {0x06a9, 0x0459}, /* XK_Cyrillic_lje                -> CYRILLIC SMALL LETTER LJE */
    {0x06aa, 0x045a}, /* XK_Cyrillic_nje                -> CYRILLIC SMALL LETTER NJE */
    {0x06ab, 0x045b}, /* XK_Serbian_tshe                -> CYRILLIC SMALL LETTER TSHE */
    {0x06ac, 0x045c}, /* XK_Macedonia_kje               -> CYRILLIC SMALL LETTER KJE */
    {0x06ad, 0x0491}, /* XK_Ukrainian_ghe_with_upturn   -> CYRILLIC SMALL LETTER GHE WITH UPTURN */
    {0x06ae, 0x045e}, /* XK_Byelorussian_shortu         -> CYRILLIC SMALL LETTER SHORT U */
    {0x06af, 0x045f}, /* XK_Cyrillic_dzhe               -> CYRILLIC SMALL LETTER DZHE */
    {0x06b0, 0x2116}, /* XK_numerosign                  -> NUMERO SIGN */
    {0x06b1, 0x0402}, /* XK_Serbian_DJE                 -> CYRILLIC CAPITAL LETTER DJE */
    {0x06b2, 0x0403}, /* XK_Macedonia_GJE               -> CYRILLIC CAPITAL LETTER GJE */
    {0x06b3, 0x0401}, /* XK_Cyrillic_IO                 -> CYRILLIC CAPITAL LETTER IO */
    {0x06b4, 0x0404}, /* XK_Ukrainian_IE                -> CYRILLIC CAPITAL LETTER UKRAINIAN IE */
    {0x06b5, 0x0405}, /* XK_Macedonia_DSE               -> CYRILLIC CAPITAL LETTER DZE */
    {0x06b6, 0x0406}, /* XK_Ukrainian_I                 -> CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
    {0x06b7, 0x0407}, /* XK_Ukrainian_YI                -> CYRILLIC CAPITAL LETTER YI */
    {0x06b8, 0x0408}, /* XK_Cyrillic_JE                 -> CYRILLIC CAPITAL LETTER JE */
    {0x06b9, 0x0409}, /* XK_Cyrillic_LJE                -> CYRILLIC CAPITAL LETTER LJE */
    {0x06ba, 0x040a}, /* XK_Cyrillic_NJE                -> CYRILLIC CAPITAL LETTER NJE */
    {0x06bb, 0x040b}, /* XK_Serbian_TSHE                -> CYRILLIC CAPITAL LETTER TSHE */
    {0x06bc, 0x040c}, /* XK_Macedonia_KJE               -> CYRILLIC CAPITAL LETTER KJE */
    {0x06bd, 0x0490}, /* XK_Ukrainian_GHE_WITH_UPTURN   -> CYRILLIC CAPITAL LETTER GHE WITH UPTURN */
    {0x06be, 0x040e}, /* XK_Byelorussian_SHORTU         -> CYRILLIC CAPITAL LETTER SHORT U */
    {0x06bf, 0x040f}, /* XK_Cyrillic_DZHE               -> CYRILLIC CAPITAL LETTER DZHE */
    {0x06c0, 0x044e}, /* XK_Cyrillic_yu                 -> CYRILLIC SMALL LETTER YU */
    {0x06c1, 0x0430}, /* XK_Cyrillic_a                  -> CYRILLIC SMALL LETTER A */
    {0x06c2, 0x0431}, /* XK_Cyrillic_be                 -> CYRILLIC SMALL LETTER BE */
    {0x06c3, 0x0446}, /* XK_Cyrillic_tse                -> CYRILLIC SMALL LETTER TSE */
    {0x06c4, 0x0434}, /* XK_Cyrillic_de                 -> CYRILLIC SMALL LETTER DE */
    {0x06c5, 0x0435}, /* XK_Cyrillic_ie                 -> CYRILLIC SMALL LETTER IE */
    {0x06c6, 0x0444}, /* XK_Cyrillic_ef                 -> CYRILLIC SMALL LETTER EF */
    {0x06c7, 0x0433}, /* XK_Cyrillic_ghe                -> CYRILLIC SMALL LETTER GHE */
    {0x06c8, 0x0445}, /* XK_Cyrillic_ha                 -> CYRILLIC SMALL LETTER HA */
    {0x06c9, 0x0438}, /* XK_Cyrillic_i                  -> CYRILLIC SMALL LETTER I */
    {0x06ca, 0x0439}, /* XK_Cyrillic_shorti             -> CYRILLIC SMALL LETTER SHORT I */
    {0x06cb, 0x043a}, /* XK_Cyrillic_ka                 -> CYRILLIC SMALL LETTER KA */
    {0x06cc, 0x043b}, /* XK_Cyrillic_el                 -> CYRILLIC SMALL LETTER EL */
    {0x06cd, 0x043c}, /* XK_Cyrillic_em                 -> CYRILLIC SMALL LETTER EM */
    {0x06ce, 0x043d}, /* XK_Cyrillic_en                 -> CYRILLIC SMALL LETTER EN */
    {0x06cf, 0x043e}, /* XK_Cyrillic_o                  -> CYRILLIC SMALL LETTER O */
    {0x06d0, 0x043f}, /* XK_Cyrillic_pe                 -> CYRILLIC SMALL LETTER PE */
    {0x06d1, 0x044f}, /* XK_Cyrillic_ya                 -> CYRILLIC SMALL LETTER YA */
    {0x06d2, 0x0440}, /* XK_Cyrillic_er                 -> CYRILLIC SMALL LETTER ER */
    {0x06d3, 0x0441}, /* XK_Cyrillic_es                 -> CYRILLIC SMALL LETTER ES */
    {0x06d4, 0x0442}, /* XK_Cyrillic_te                 -> CYRILLIC SMALL LETTER TE */
    {0x06d5, 0x0443}, /* XK_Cyrillic_u                  -> CYRILLIC SMALL LETTER U */
    {0x06d6, 0x0436}, /* XK_Cyrillic_zhe                -> CYRILLIC SMALL LETTER ZHE */
    {0x06d7, 0x0432}, /* XK_Cyrillic_ve                 -> CYRILLIC SMALL LETTER VE */
    {0x06d8, 0x044c}, /* XK_Cyrillic_softsign           -> CYRILLIC SMALL LETTER SOFT SIGN */
    {0x06d9, 0x044b}, /* XK_Cyrillic_yeru               -> CYRILLIC SMALL LETTER YERU */
    {0x06da, 0x0437}, /* XK_Cyrillic_ze                 -> CYRILLIC SMALL LETTER ZE */
    {0x06db, 0x0448}, /* XK_Cyrillic_sha                -> CYRILLIC SMALL LETTER SHA */
    {0x06dc, 0x044d}, /* XK_Cyrillic_e                  -> CYRILLIC SMALL LETTER E */
    {0x06dd, 0x0449}, /* XK_Cyrillic_shcha              -> CYRILLIC SMALL LETTER SHCHA */
    {0x06de, 0x0447}, /* XK_Cyrillic_che                -> CYRILLIC SMALL LETTER CHE */
    {0x06df, 0x044a}, /* XK_Cyrillic_hardsign           -> CYRILLIC SMALL LETTER HARD SIGN */
    {0x06e0, 0x042e}, /* XK_Cyrillic_YU                 -> CYRILLIC CAPITAL LETTER YU */
    {0x06e1, 0x0410}, /* XK_Cyrillic_A                  -> CYRILLIC CAPITAL LETTER A */
    {0x06e2, 0x0411}, /* XK_Cyrillic_BE                 -> CYRILLIC CAPITAL LETTER BE */
    {0x06e3, 0x0426}, /* XK_Cyrillic_TSE                -> CYRILLIC CAPITAL LETTER TSE */
    {0x06e4, 0x0414}, /* XK_Cyrillic_DE                 -> CYRILLIC CAPITAL LETTER DE */
    {0x06e5, 0x0415}, /* XK_Cyrillic_IE                 -> CYRILLIC CAPITAL LETTER IE */
    {0x06e6, 0x0424}, /* XK_Cyrillic_EF                 -> CYRILLIC CAPITAL LETTER EF */
    {0x06e7, 0x0413}, /* XK_Cyrillic_GHE                -> CYRILLIC CAPITAL LETTER GHE */
    {0x06e8, 0x0425}, /* XK_Cyrillic_HA                 -> CYRILLIC CAPITAL LETTER HA */
    {0x06e9, 0x0418}, /* XK_Cyrillic_I                  -> CYRILLIC CAPITAL LETTER I */
    {0x06ea, 0x0419}, /* XK_Cyrillic_SHORTI             -> CYRILLIC CAPITAL LETTER SHORT I */
    {0x06eb, 0x041a}, /* XK_Cyrillic_KA                 -> CYRILLIC CAPITAL LETTER KA */
    {0x06ec, 0x041b}, /* XK_Cyrillic_EL                 -> CYRILLIC CAPITAL LETTER EL */
    {0x06ed, 0x041c}, /* XK_Cyrillic_EM                 -> CYRILLIC CAPITAL LETTER EM */
    {0x06ee, 0x041d}, /* XK_Cyrillic_EN                 -> CYRILLIC CAPITAL LETTER EN */
    {0x06ef, 0x041e}, /* XK_Cyrillic_O                  -> CYRILLIC CAPITAL LETTER O */
    {0x06f0, 0x041f}, /* XK_Cyrillic_PE                 -> CYRILLIC CAPITAL LETTER PE */
    {0x06f1, 0x042f}, /* XK_Cyrillic_YA                 -> CYRILLIC CAPITAL LETTER YA */
    {0x06f2, 0x0420}, /* XK_Cyrillic_ER                 -> CYRILLIC CAPITAL LETTER ER */
    {0x06f3, 0x0421}, /* XK_Cyrillic_ES                 -> CYRILLIC CAPITAL LETTER ES */
    {0x06f4, 0x0422}, /* XK_Cyrillic_TE                 -> CYRILLIC CAPITAL LETTER TE */
    {0x06f5, 0x0423}, /* XK_Cyrillic_U                  -> CYRILLIC CAPITAL LETTER U */
    {0x06f6, 0x0416}, /* XK_Cyrillic_ZHE                -> CYRILLIC CAPITAL LETTER ZHE */
    {0x06f7, 0x0412}, /* XK_Cyrillic_VE                 -> CYRILLIC CAPITAL LETTER VE */
    {0x06f8, 0x042c}, /* XK_Cyrillic_SOFTSIGN           -> CYRILLIC CAPITAL LETTER SOFT SIGN */
    {0x06f9, 0x042b}, /* XK_Cyrillic_YERU               -> CYRILLIC CAPITAL LETTER YERU */
    {0x06fa, 0x0417}, /* XK_Cyrillic_ZE                 -> CYRILLIC CAPITAL LETTER ZE */
    {0x06fb, 0x0428}, /* XK_Cyrillic_SHA                -> CYRILLIC CAPITAL LETTER SHA */
    {0x06fc, 0x042d}, /* XK_Cyrillic_E                  -> CYRILLIC CAPITAL LETTER E */
    {0x06fd, 0x0429}, /* XK_Cyrillic_SHCHA              -> CYRILLIC CAPITAL LETTER SHCHA */
    {0x06fe, 0x0427}, /* XK_Cyrillic_CHE                -> CYRILLIC CAPITAL LETTER CHE */
    {0x06ff, 0x042a}, /* XK_Cyrillic_HARDSIGN           -> CYRILLIC CAPITAL LETTER HARD SIGN */
    {0x07a1, 0x0386}, /* XK_Greek_ALPHAaccent           -> GREEK CAPITAL LETTER ALPHA WITH TONOS */
    {0x07a2, 0x0388}, /* XK_Greek_EPSILONaccent         -> GREEK CAPITAL LETTER EPSILON WITH TONOS */
    {0x07a3, 0x0389}, /* XK_Greek_ETAaccent             -> GREEK CAPITAL LETTER ETA WITH TONOS */
    {0x07a4, 0x038a}, /* XK_Greek_IOTAaccent            -> GREEK CAPITAL LETTER IOTA WITH TONOS */
    {0x07a5, 0x03aa}, /* XK_Greek_IOTAdieresis          -> GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */
    {0x07a7, 0x038c}, /* XK_Greek_OMICRONaccent         -> GREEK CAPITAL LETTER OMICRON WITH TONOS */
    {0x07a8, 0x038e}, /* XK_Greek_UPSILONaccent         -> GREEK CAPITAL LETTER UPSILON WITH TONOS */
    {0x07a9, 0x03ab}, /* XK_Greek_UPSILONdieresis       -> GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */
    {0x07ab, 0x038f}, /* XK_Greek_OMEGAaccent           -> GREEK CAPITAL LETTER OMEGA WITH TONOS */
    {0x07ae, 0x0385}, /* XK_Greek_accentdieresis        -> GREEK DIALYTIKA TONOS */
    {0x07af, 0x2015}, /* XK_Greek_horizbar              -> HORIZONTAL BAR */
    {0x07b1, 0x03ac}, /* XK_Greek_alphaaccent           -> GREEK SMALL LETTER ALPHA WITH TONOS */
    {0x07b2, 0x03ad}, /* XK_Greek_epsilonaccent         -> GREEK SMALL LETTER EPSILON WITH TONOS */
    {0x07b3, 0x03ae}, /* XK_Greek_etaaccent             -> GREEK SMALL LETTER ETA WITH TONOS */
    {0x07b4, 0x03af}, /* XK_Greek_iotaaccent            -> GREEK SMALL LETTER IOTA WITH TONOS */
    {0x07b5, 0x03ca}, /* XK_Greek_iotadieresis          -> GREEK SMALL LETTER IOTA WITH DIALYTIKA */
    {0x07b6, 0x0390}, /* XK_Greek_iotaaccentdieresis    -> GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
    {0x07b7, 0x03cc}, /* XK_Greek_omicronaccent         -> GREEK SMALL LETTER OMICRON WITH TONOS */
    {0x07b8, 0x03cd}, /* XK_Greek_upsilonaccent         -> GREEK SMALL LETTER UPSILON WITH TONOS */
    {0x07b9, 0x03cb}, /* XK_Greek_upsilondieresis       -> GREEK SMALL LETTER UPSILON WITH DIALYTIKA */
    {0x07ba, 0x03b0}, /* XK_Greek_upsilonaccentdieresis -> GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
    {0x07bb, 0x03ce}, /* XK_Greek_omegaaccent           -> GREEK SMALL LETTER OMEGA WITH TONOS */
    {0x07c1, 0x0391}, /* XK_Greek_ALPHA                 -> GREEK CAPITAL LETTER ALPHA */
    {0x07c2, 0x0392}, /* XK_Greek_BETA                  -> GREEK CAPITAL LETTER BETA */
    {0x07c3, 0x0393}, /* XK_Greek_GAMMA                 -> GREEK CAPITAL LETTER GAMMA */
    {0x07c4, 0x0394}, /* XK_Greek_DELTA                 -> GREEK CAPITAL LETTER DELTA */
    {0x07c5, 0x0395}, /* XK_Greek_EPSILON               -> GREEK CAPITAL LETTER EPSILON */
    {0x07c6, 0x0396}, /* XK_Greek_ZETA                  -> GREEK CAPITAL LETTER ZETA */
    {0x07c7, 0x0397}, /* XK_Greek_ETA                   -> GREEK CAPITAL LETTER ETA */
    {0x07c8, 0x0398}, /* XK_Greek_THETA                 -> GREEK CAPITAL LETTER THETA */
    {0x07c9, 0x0399}, /* XK_Greek_IOTA                  -> GREEK CAPITAL LETTER IOTA */
    {0x07ca, 0x039a}, /* XK_Greek_KAPPA                 -> GREEK CAPITAL LETTER KAPPA */
    {0x07cb, 0x039b}, /* XK_Greek_LAMDA                 -> GREEK CAPITAL LETTER LAMDA */
    {0x07cc, 0x039c}, /* XK_Greek_MU                    -> GREEK CAPITAL LETTER MU */
    {0x07cd, 0x039d}, /* XK_Greek_NU                    -> GREEK CAPITAL LETTER NU */
    {0x07ce, 0x039e}, /* XK_Greek_XI                    -> GREEK CAPITAL LETTER XI */
    {0x07cf, 0x039f}, /* XK_Greek_OMICRON               -> GREEK CAPITAL LETTER OMICRON */
    {0x07d0, 0x03a0}, /* XK_Greek_PI                    -> GREEK CAPITAL LETTER PI */
    {0x07d1, 0x03a1}, /* XK_Greek_RHO                   -> GREEK CAPITAL LETTER RHO */
    {0x07d2, 0x03a3}, /* XK_Greek_SIGMA                 -> GREEK CAPITAL LETTER SIGMA */
    {0x07d4, 0x03a4}, /* XK_Greek_TAU                   -> GREEK CAPITAL LETTER TAU */
    {0x07d5, 0x03a5}, /* XK_Greek_UPSILON               -> GREEK CAPITAL LETTER UPSILON */
    {0x07d6, 0x03a6}, /* XK_Greek_PHI                   -> GREEK CAPITAL LETTER PHI */
    {0x07d7, 0x03a7}, /* XK_Greek_CHI                   -> GREEK CAPITAL LETTER CHI */
    {0x07d8, 0x03a8}, /* XK_Greek_PSI                   -> GREEK CAPITAL LETTER PSI */
    {0x07d9, 0x03a9}, /* XK_Greek_OMEGA                 -> GREEK CAPITAL LETTER OMEGA */
    {0x07e1, 0x03b1}, /* XK_Greek_alpha                 -> GREEK SMALL LETTER ALPHA */
    {0x07e2, 0x03b2}, /* XK_Greek_beta                  -> GREEK SMALL LETTER BETA */
    {0x07e3, 0x03b3}, /* XK_Greek_gamma                 -> GREEK SMALL LETTER GAMMA */
    {0x07e4, 0x03b4}, /* XK_Greek_delta                 -> GREEK SMALL LETTER DELTA */
    {0x07e5, 0x03b5}, /* XK_Greek_epsilon               -> GREEK SMALL LETTER EPSILON */
    {0x07e6, 0x03b6}, /* XK_Greek_zeta                  -> GREEK SMALL LETTER ZETA */
    {0x07e7, 0x03b7}, /* XK_Greek_eta                   -> GREEK SMALL LETTER ETA */
    {0x07e8, 0x03b8}, /* XK_Greek_theta                 -> GREEK SMALL LETTER THETA */
    {0x07e9, 0x03b9}, /* XK_Greek_iota                  -> GREEK SMALL LETTER IOTA */
    {0x07ea, 0x03ba}, /* XK_Greek_kappa                 -> GREEK SMALL LETTER KAPPA */
    {0x07eb, 0x03bb}, /* XK_Greek_lamda                 -> GREEK SMALL LETTER LAMDA */
    {0x07ec, 0x03bc}, /* XK_Greek_mu                    -> GREEK SMALL LETTER MU */
    {0x07ed, 0x03bd}, /* XK_Greek_nu                    -> GREEK SMALL LETTER NU */
    {0x07ee, 0x03be}, /* XK_Greek_xi                    -> GREEK SMALL LETTER XI */
    {0x07ef, 0x03bf}, /* XK_Greek_omicron               -> GREEK SMALL LETTER OMICRON */
    {0x07f0, 0x03c0}, /* XK_Greek_pi                    -> GREEK SMALL LETTER PI */
    {0x07f1, 0x03c1}, /* XK_Greek_rho                   -> GREEK SMALL LETTER RHO */
    {0x07f2, 0x03c3}, /* XK_Greek_sigma                 -> GREEK SMALL LETTER SIGMA */
    {0x07f3, 0x03c2}, /* XK_Greek_finalsmallsigma       -> GREEK SMALL LETTER FINAL SIGMA */
    {0x07f4, 0x03c4}, /* XK_Greek_tau                   -> GREEK SMALL LETTER TAU */
    {0x07f5, 0x03c5}, /* XK_Greek_upsilon               -> GREEK SMALL LETTER UPSILON */
    {0x07f6, 0x03c6}, /* XK_Greek_phi                   -> GREEK SMALL LETTER PHI */
    {0x07f7, 0x03c7}, /* XK_Greek_chi                   -> GREEK SMALL LETTER CHI */
    {0x07f8, 0x03c8}, /* XK_Greek_psi                   -> GREEK SMALL LETTER PSI */
    {0x07f9, 0x03c9}, /* XK_Greek_omega                 -> GREEK SMALL LETTER OMEGA */
    {0x08a1, 0x23b7}, /* XK_leftradical                 -> RADICAL SYMBOL BOTTOM */
    {0x08a2, 0x250c}, /* XK_topleftradical              -> BOX DRAWINGS LIGHT DOWN AND RIGHT */
    {0x08a3, 0x2500}, /* XK_horizconnector              -> BOX DRAWINGS LIGHT HORIZONTAL */
    {0x08a4, 0x2320}, /* XK_topintegral                 -> TOP HALF INTEGRAL */
    {0x08a5, 0x2321}, /* XK_botintegral                 -> BOTTOM HALF INTEGRAL */
    {0x08a6, 0x2502}, /* XK_vertconnector               -> BOX DRAWINGS LIGHT VERTICAL */
    {0x08a7, 0x23a1}, /* XK_topleftsqbracket            -> LEFT SQUARE BRACKET UPPER CORNER */
    {0x08a8, 0x23a3}, /* XK_botleftsqbracket            -> LEFT SQUARE BRACKET LOWER CORNER */
    {0x08a9, 0x23a4}, /* XK_toprightsqbracket           -> RIGHT SQUARE BRACKET UPPER CORNER */
    {0x08aa, 0x23a6}, /* XK_botrightsqbracket           -> RIGHT SQUARE BRACKET LOWER CORNER */
    {0x08ab, 0x239b}, /* XK_topleftparens               -> LEFT PARENTHESIS UPPER HOOK */
    {0x08ac, 0x239d}, /* XK_botleftparens               -> LEFT PARENTHESIS LOWER HOOK */
    {0x08ad, 0x239e}, /* XK_toprightparens              -> RIGHT PARENTHESIS UPPER HOOK */
    {0x08ae, 0x23a0}, /* XK_botrightparens              -> RIGHT PARENTHESIS LOWER HOOK */
    {0x08af, 0x23a8}, /* XK_leftmiddlecurlybrace        -> LEFT CURLY BRACKET MIDDLE PIECE */
    {0x08b0, 0x23ac}, /* XK_rightmiddlecurlybrace       -> RIGHT CURLY BRACKET MIDDLE PIECE */
  /* 0x08b1              XK_topleftsummation               No mapping defined */
  /* 0x08b2              XK_botleftsummation               No mapping defined */
  /* 0x08b3              XK_topvertsummationconnector      No mapping defined */
  /* 0x08b4              XK_botvertsummationconnector      No mapping defined */
  /* 0x08b5              XK_toprightsummation              No mapping defined */
  /* 0x08b6              XK_botrightsummation              No mapping defined */
  /* 0x08b7              XK_rightmiddlesummation           No mapping defined */
    {0x08bc, 0x2264}, /* XK_lessthanequal               -> LESS-THAN OR EQUAL TO */
    {0x08bd, 0x2260}, /* XK_notequal                    -> NOT EQUAL TO */
    {0x08be, 0x2265}, /* XK_greaterthanequal            -> GREATER-THAN OR EQUAL TO */
    {0x08bf, 0x222b}, /* XK_integral                    -> INTEGRAL */
    {0x08c0, 0x2234}, /* XK_therefore                   -> THEREFORE */
    {0x08c1, 0x221d}, /* XK_variation                   -> PROPORTIONAL TO */
    {0x08c2, 0x221e}, /* XK_infinity                    -> INFINITY */
    {0x08c5, 0x2207}, /* XK_nabla                       -> NABLA */
    {0x08c8, 0x223c}, /* XK_approximate                 -> TILDE OPERATOR */
    {0x08c9, 0x2243}, /* XK_similarequal                -> ASYMPTOTICALLY EQUAL TO */
    {0x08cd, 0x21d4}, /* XK_ifonlyif                    -> LEFT RIGHT DOUBLE ARROW */
    {0x08ce, 0x21d2}, /* XK_implies                     -> RIGHTWARDS DOUBLE ARROW */
    {0x08cf, 0x2261}, /* XK_identical                   -> IDENTICAL TO */
    {0x08d6, 0x221a}, /* XK_radical                     -> SQUARE ROOT */
    {0x08da, 0x2282}, /* XK_includedin                  -> SUBSET OF */
    {0x08db, 0x2283}, /* XK_includes                    -> SUPERSET OF */
    {0x08dc, 0x2229}, /* XK_intersection                -> INTERSECTION */
    {0x08dd, 0x222a}, /* XK_union                       -> UNION */
    {0x08de, 0x2227}, /* XK_logicaland                  -> LOGICAL AND */
    {0x08df, 0x2228}, /* XK_logicalor                   -> LOGICAL OR */
    {0x08ef, 0x2202}, /* XK_partialderivative           -> PARTIAL DIFFERENTIAL */
    {0x08f6, 0x0192}, /* XK_function                    -> LATIN SMALL LETTER F WITH HOOK */
    {0x08fb, 0x2190}, /* XK_leftarrow                   -> LEFTWARDS ARROW */
    {0x08fc, 0x2191}, /* XK_uparrow                     -> UPWARDS ARROW */
    {0x08fd, 0x2192}, /* XK_rightarrow                  -> RIGHTWARDS ARROW */
    {0x08fe, 0x2193}, /* XK_downarrow                   -> DOWNWARDS ARROW */
  /* 0x09df              XK_blank                          No mapping defined */
    {0x09e0, 0x25c6}, /* XK_soliddiamond                -> BLACK DIAMOND */
    {0x09e1, 0x2592}, /* XK_checkerboard                -> MEDIUM SHADE */
    {0x09e2, 0x2409}, /* XK_ht                          -> SYMBOL FOR HORIZONTAL TABULATION */
    {0x09e3, 0x240c}, /* XK_ff                          -> SYMBOL FOR FORM FEED */
    {0x09e4, 0x240d}, /* XK_cr                          -> SYMBOL FOR CARRIAGE RETURN */
    {0x09e5, 0x240a}, /* XK_lf                          -> SYMBOL FOR LINE FEED */
    {0x09e8, 0x2424}, /* XK_nl                          -> SYMBOL FOR NEWLINE */
    {0x09e9, 0x240b}, /* XK_vt                          -> SYMBOL FOR VERTICAL TABULATION */
    {0x09ea, 0x2518}, /* XK_lowrightcorner              -> BOX DRAWINGS LIGHT UP AND LEFT */
    {0x09eb, 0x2510}, /* XK_uprightcorner               -> BOX DRAWINGS LIGHT DOWN AND LEFT */
    {0x09ec, 0x250c}, /* XK_upleftcorner                -> BOX DRAWINGS LIGHT DOWN AND RIGHT */
    {0x09ed, 0x2514}, /* XK_lowleftcorner               -> BOX DRAWINGS LIGHT UP AND RIGHT */
    {0x09ee, 0x253c}, /* XK_crossinglines               -> BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
    {0x09ef, 0x23ba}, /* XK_horizlinescan1              -> HORIZONTAL SCAN LINE-1 */
    {0x09f0, 0x23bb}, /* XK_horizlinescan3              -> HORIZONTAL SCAN LINE-3 */
    {0x09f1, 0x2500}, /* XK_horizlinescan5              -> BOX DRAWINGS LIGHT HORIZONTAL */
    {0x09f2, 0x23bc}, /* XK_horizlinescan7              -> HORIZONTAL SCAN LINE-7 */
    {0x09f3, 0x23bd}, /* XK_horizlinescan9              -> HORIZONTAL SCAN LINE-9 */
    {0x09f4, 0x251c}, /* XK_leftt                       -> BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    {0x09f5, 0x2524}, /* XK_rightt                      -> BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    {0x09f6, 0x2534}, /* XK_bott                        -> BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    {0x09f7, 0x252c}, /* XK_topt                        -> BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    {0x09f8, 0x2502}, /* XK_vertbar                     -> BOX DRAWINGS LIGHT VERTICAL */
    {0x0aa1, 0x2003}, /* XK_emspace                     -> EM SPACE */
    {0x0aa2, 0x2002}, /* XK_enspace                     -> EN SPACE */
    {0x0aa3, 0x2004}, /* XK_em3space                    -> THREE-PER-EM SPACE */
    {0x0aa4, 0x2005}, /* XK_em4space                    -> FOUR-PER-EM SPACE */
    {0x0aa5, 0x2007}, /* XK_digitspace                  -> FIGURE SPACE */
    {0x0aa6, 0x2008}, /* XK_punctspace                  -> PUNCTUATION SPACE */
    {0x0aa7, 0x2009}, /* XK_thinspace                   -> THIN SPACE */
    {0x0aa8, 0x200a}, /* XK_hairspace                   -> HAIR SPACE */
    {0x0aa9, 0x2014}, /* XK_emdash                      -> EM DASH */
    {0x0aaa, 0x2013}, /* XK_endash                      -> EN DASH */
    {0x0aac, 0x2423}, /* XK_signifblank                 -> OPEN BOX */
    {0x0aae, 0x2026}, /* XK_ellipsis                    -> HORIZONTAL ELLIPSIS */
    {0x0aaf, 0x2025}, /* XK_doubbaselinedot             -> TWO DOT LEADER */
    {0x0ab0, 0x2153}, /* XK_onethird                    -> VULGAR FRACTION ONE THIRD */
    {0x0ab1, 0x2154}, /* XK_twothirds                   -> VULGAR FRACTION TWO THIRDS */
    {0x0ab2, 0x2155}, /* XK_onefifth                    -> VULGAR FRACTION ONE FIFTH */
    {0x0ab3, 0x2156}, /* XK_twofifths                   -> VULGAR FRACTION TWO FIFTHS */
    {0x0ab4, 0x2157}, /* XK_threefifths                 -> VULGAR FRACTION THREE FIFTHS */
    {0x0ab5, 0x2158}, /* XK_fourfifths                  -> VULGAR FRACTION FOUR FIFTHS */
    {0x0ab6, 0x2159}, /* XK_onesixth                    -> VULGAR FRACTION ONE SIXTH */
    {0x0ab7, 0x215a}, /* XK_fivesixths                  -> VULGAR FRACTION FIVE SIXTHS */
    {0x0ab8, 0x2105}, /* XK_careof                      -> CARE OF */
    {0x0abb, 0x2012}, /* XK_figdash                     -> FIGURE DASH */
    {0x0abc, 0x27e8}, /* XK_leftanglebracket            -> MATHEMATICAL LEFT ANGLE BRACKET */
    {0x0abd, 0x002e}, /* XK_decimalpoint                -> FULL STOP */
    {0x0abe, 0x27e9}, /* XK_rightanglebracket           -> MATHEMATICAL RIGHT ANGLE BRACKET */
  /* 0x0abf              XK_marker                         No mapping defined */
    {0x0ac3, 0x215b}, /* XK_oneeighth                   -> VULGAR FRACTION ONE EIGHTH */
    {0x0ac4, 0x215c}, /* XK_threeeighths                -> VULGAR FRACTION THREE EIGHTHS */
    {0x0ac5, 0x215d}, /* XK_fiveeighths                 -> VULGAR FRACTION FIVE EIGHTHS */
    {0x0ac6, 0x215e}, /* XK_seveneighths                -> VULGAR FRACTION SEVEN EIGHTHS */
    {0x0ac9, 0x2122}, /* XK_trademark                   -> TRADE MARK SIGN */
    {0x0aca, 0x2613}, /* XK_signaturemark               -> SALTIRE */
  /* 0x0acb              XK_trademarkincircle              No mapping defined */
    {0x0acc, 0x25c1}, /* XK_leftopentriangle            -> WHITE LEFT-POINTING TRIANGLE */
    {0x0acd, 0x25b7}, /* XK_rightopentriangle           -> WHITE RIGHT-POINTING TRIANGLE */
    {0x0ace, 0x25cb}, /* XK_emopencircle                -> WHITE CIRCLE */
    {0x0acf, 0x25af}, /* XK_emopenrectangle             -> WHITE VERTICAL RECTANGLE */
    {0x0ad0, 0x2018}, /* XK_leftsinglequotemark         -> LEFT SINGLE QUOTATION MARK */
    {0x0ad1, 0x2019}, /* XK_rightsinglequotemark        -> RIGHT SINGLE QUOTATION MARK */
    {0x0ad2, 0x201c}, /* XK_leftdoublequotemark         -> LEFT DOUBLE QUOTATION MARK */
    {0x0ad3, 0x201d}, /* XK_rightdoublequotemark        -> RIGHT DOUBLE QUOTATION MARK */
    {0x0ad4, 0x211e}, /* XK_prescription                -> PRESCRIPTION TAKE */
    {0x0ad6, 0x2032}, /* XK_minutes                     -> PRIME */
    {0x0ad7, 0x2033}, /* XK_seconds                     -> DOUBLE PRIME */
    {0x0ad9, 0x271d}, /* XK_latincross                  -> LATIN CROSS */
  /* 0x0ada              XK_hexagram                       No mapping defined */
    {0x0adb, 0x25ac}, /* XK_filledrectbullet            -> BLACK RECTANGLE */
    {0x0adc, 0x25c0}, /* XK_filledlefttribullet         -> BLACK LEFT-POINTING TRIANGLE */
    {0x0add, 0x25b6}, /* XK_filledrighttribullet        -> BLACK RIGHT-POINTING TRIANGLE */
    {0x0ade, 0x25cf}, /* XK_emfilledcircle              -> BLACK CIRCLE */
    {0x0adf, 0x25ae}, /* XK_emfilledrect                -> BLACK VERTICAL RECTANGLE */
    {0x0ae0, 0x25e6}, /* XK_enopencircbullet            -> WHITE BULLET */
    {0x0ae1, 0x25ab}, /* XK_enopensquarebullet          -> WHITE SMALL SQUARE */
    {0x0ae2, 0x25ad}, /* XK_openrectbullet              -> WHITE RECTANGLE */
    {0x0ae3, 0x25b3}, /* XK_opentribulletup             -> WHITE UP-POINTING TRIANGLE */
    {0x0ae4, 0x25bd}, /* XK_opentribulletdown           -> WHITE DOWN-POINTING TRIANGLE */
    {0x0ae5, 0x2606}, /* XK_openstar                    -> WHITE STAR */
    {0x0ae6, 0x2022}, /* XK_enfilledcircbullet          -> BULLET */
    {0x0ae7, 0x25aa}, /* XK_enfilledsqbullet            -> BLACK SMALL SQUARE */
    {0x0ae8, 0x25b2}, /* XK_filledtribulletup           -> BLACK UP-POINTING TRIANGLE */
    {0x0ae9, 0x25bc}, /* XK_filledtribulletdown         -> BLACK DOWN-POINTING TRIANGLE */
    {0x0aea, 0x261c}, /* XK_leftpointer                 -> WHITE LEFT POINTING INDEX */
    {0x0aeb, 0x261e}, /* XK_rightpointer                -> WHITE RIGHT POINTING INDEX */
    {0x0aec, 0x2663}, /* XK_club                        -> BLACK CLUB SUIT */
    {0x0aed, 0x2666}, /* XK_diamond                     -> BLACK DIAMOND SUIT */
    {0x0aee, 0x2665}, /* XK_heart                       -> BLACK HEART SUIT */
    {0x0af0, 0x2720}, /* XK_maltesecross                -> MALTESE CROSS */
    {0x0af1, 0x2020}, /* XK_dagger                      -> DAGGER */
    {0x0af2, 0x2021}, /* XK_doubledagger                -> DOUBLE DAGGER */
    {0x0af3, 0x2713}, /* XK_checkmark                   -> CHECK MARK */
    {0x0af4, 0x2717}, /* XK_ballotcross                 -> BALLOT X */
    {0x0af5, 0x266f}, /* XK_musicalsharp                -> MUSIC SHARP SIGN */
    {0x0af6, 0x266d}, /* XK_musicalflat                 -> MUSIC FLAT SIGN */
    {0x0af7, 0x2642}, /* XK_malesymbol                  -> MALE SIGN */
    {0x0af8, 0x2640}, /* XK_femalesymbol                -> FEMALE SIGN */
    {0x0af9, 0x260e}, /* XK_telephone                   -> BLACK TELEPHONE */
    {0x0afa, 0x2315}, /* XK_telephonerecorder           -> TELEPHONE RECORDER */
    {0x0afb, 0x2117}, /* XK_phonographcopyright         -> SOUND RECORDING COPYRIGHT */
    {0x0afc, 0x2038}, /* XK_caret                       -> CARET */
    {0x0afd, 0x201a}, /* XK_singlelowquotemark          -> SINGLE LOW-9 QUOTATION MARK */
    {0x0afe, 0x201e}, /* XK_doublelowquotemark          -> DOUBLE LOW-9 QUOTATION MARK */
  /* 0x0aff              XK_cursor                         No mapping defined */
    {0x0ba3, 0x003c}, /* XK_leftcaret                   -> LESS-THAN SIGN */
    {0x0ba6, 0x003e}, /* XK_rightcaret                  -> GREATER-THAN SIGN */
    {0x0ba8, 0x2228}, /* XK_downcaret                   -> LOGICAL OR */
    {0x0ba9, 0x2227}, /* XK_upcaret                     -> LOGICAL AND */
    {0x0bc0, 0x00af}, /* XK_overbar                     -> MACRON */
    {0x0bc2, 0x22a4}, /* XK_downtack                    -> DOWN TACK */
    {0x0bc3, 0x2229}, /* XK_upshoe                      -> INTERSECTION */
    {0x0bc4, 0x230a}, /* XK_downstile                   -> LEFT FLOOR */
    {0x0bc6, 0x005f}, /* XK_underbar                    -> LOW LINE */
    {0x0bca, 0x2218}, /* XK_jot                         -> RING OPERATOR */
    {0x0bcc, 0x2395}, /* XK_quad                        -> APL FUNCTIONAL SYMBOL QUAD */
    {0x0bce, 0x22a5}, /* XK_uptack                      -> UP TACK */
    {0x0bcf, 0x25cb}, /* XK_circle                      -> WHITE CIRCLE */
    {0x0bd3, 0x2308}, /* XK_upstile                     -> LEFT CEILING */
    {0x0bd6, 0x222a}, /* XK_downshoe                    -> UNION */
    {0x0bd8, 0x2283}, /* XK_rightshoe                   -> SUPERSET OF */
    {0x0bda, 0x2282}, /* XK_leftshoe                    -> SUBSET OF */
    {0x0bdc, 0x22a3}, /* XK_lefttack                    -> LEFT TACK */
    {0x0bfc, 0x22a2}, /* XK_righttack                   -> RIGHT TACK */
    {0x0cdf, 0x2017}, /* XK_hebrew_doublelowline        -> DOUBLE LOW LINE */
    {0x0ce0, 0x05d0}, /* XK_hebrew_aleph                -> HEBREW LETTER ALEF */
    {0x0ce1, 0x05d1}, /* XK_hebrew_bet                  -> HEBREW LETTER BET */
    {0x0ce2, 0x05d2}, /* XK_hebrew_gimel                -> HEBREW LETTER GIMEL */
    {0x0ce3, 0x05d3}, /* XK_hebrew_dalet                -> HEBREW LETTER DALET */
    {0x0ce4, 0x05d4}, /* XK_hebrew_he                   -> HEBREW LETTER HE */
    {0x0ce5, 0x05d5}, /* XK_hebrew_waw                  -> HEBREW LETTER VAV */
    {0x0ce6, 0x05d6}, /* XK_hebrew_zain                 -> HEBREW LETTER ZAYIN */
    {0x0ce7, 0x05d7}, /* XK_hebrew_chet                 -> HEBREW LETTER HET */
    {0x0ce8, 0x05d8}, /* XK_hebrew_tet                  -> HEBREW LETTER TET */
    {0x0ce9, 0x05d9}, /* XK_hebrew_yod                  -> HEBREW LETTER YOD */
    {0x0cea, 0x05da}, /* XK_hebrew_finalkaph            -> HEBREW LETTER FINAL KAF */
    {0x0ceb, 0x05db}, /* XK_hebrew_kaph                 -> HEBREW LETTER KAF */
    {0x0cec, 0x05dc}, /* XK_hebrew_lamed                -> HEBREW LETTER LAMED */
    {0x0ced, 0x05dd}, /* XK_hebrew_finalmem             -> HEBREW LETTER FINAL MEM */
    {0x0cee, 0x05de}, /* XK_hebrew_mem                  -> HEBREW LETTER MEM */
    {0x0cef, 0x05df}, /* XK_hebrew_finalnun             -> HEBREW LETTER FINAL NUN */
    {0x0cf0, 0x05e0}, /* XK_hebrew_nun                  -> HEBREW LETTER NUN */
    {0x0cf1, 0x05e1}, /* XK_hebrew_samech               -> HEBREW LETTER SAMEKH */
    {0x0cf2, 0x05e2}, /* XK_hebrew_ayin                 -> HEBREW LETTER AYIN */
    {0x0cf3, 0x05e3}, /* XK_hebrew_finalpe              -> HEBREW LETTER FINAL PE */
    {0x0cf4, 0x05e4}, /* XK_hebrew_pe                   -> HEBREW LETTER PE */
    {0x0cf5, 0x05e5}, /* XK_hebrew_finalzade            -> HEBREW LETTER FINAL TSADI */
    {0x0cf6, 0x05e6}, /* XK_hebrew_zade                 -> HEBREW LETTER TSADI */
    {0x0cf7, 0x05e7}, /* XK_hebrew_qoph                 -> HEBREW LETTER QOF */
    {0x0cf8, 0x05e8}, /* XK_hebrew_resh                 -> HEBREW LETTER RESH */
    {0x0cf9, 0x05e9}, /* XK_hebrew_shin                 -> HEBREW LETTER SHIN */
    {0x0cfa, 0x05ea}, /* XK_hebrew_taw                  -> HEBREW LETTER TAV */
    {0x0da1, 0x0e01}, /* XK_Thai_kokai                  -> THAI CHARACTER KO KAI */
    {0x0da2, 0x0e02}, /* XK_Thai_khokhai                -> THAI CHARACTER KHO KHAI */
    {0x0da3, 0x0e03}, /* XK_Thai_khokhuat               -> THAI CHARACTER KHO KHUAT */
    {0x0da4, 0x0e04}, /* XK_Thai_khokhwai               -> THAI CHARACTER KHO KHWAI */
    {0x0da5, 0x0e05}, /* XK_Thai_khokhon                -> THAI CHARACTER KHO KHON */
    {0x0da6, 0x0e06}, /* XK_Thai_khorakhang             -> THAI CHARACTER KHO RAKHANG */
    {0x0da7, 0x0e07}, /* XK_Thai_ngongu                 -> THAI CHARACTER NGO NGU */
    {0x0da8, 0x0e08}, /* XK_Thai_chochan                -> THAI CHARACTER CHO CHAN */
    {0x0da9, 0x0e09}, /* XK_Thai_choching               -> THAI CHARACTER CHO CHING */
    {0x0daa, 0x0e0a}, /* XK_Thai_chochang               -> THAI CHARACTER CHO CHANG */
    {0x0dab, 0x0e0b}, /* XK_Thai_soso                   -> THAI CHARACTER SO SO */
    {0x0dac, 0x0e0c}, /* XK_Thai_chochoe                -> THAI CHARACTER CHO CHOE */
    {0x0dad, 0x0e0d}, /* XK_Thai_yoying                 -> THAI CHARACTER YO YING */
    {0x0dae, 0x0e0e}, /* XK_Thai_dochada                -> THAI CHARACTER DO CHADA */
    {0x0daf, 0x0e0f}, /* XK_Thai_topatak                -> THAI CHARACTER TO PATAK */
    {0x0db0, 0x0e10}, /* XK_Thai_thothan                -> THAI CHARACTER THO THAN */
    {0x0db1, 0x0e11}, /* XK_Thai_thonangmontho          -> THAI CHARACTER THO NANGMONTHO */
    {0x0db2, 0x0e12}, /* XK_Thai_thophuthao             -> THAI CHARACTER THO PHUTHAO */
    {0x0db3, 0x0e13}, /* XK_Thai_nonen                  -> THAI CHARACTER NO NEN */
    {0x0db4, 0x0e14}, /* XK_Thai_dodek                  -> THAI CHARACTER DO DEK */
    {0x0db5, 0x0e15}, /* XK_Thai_totao                  -> THAI CHARACTER TO TAO */
    {0x0db6, 0x0e16}, /* XK_Thai_thothung               -> THAI CHARACTER THO THUNG */
    {0x0db7, 0x0e17}, /* XK_Thai_thothahan              -> THAI CHARACTER THO THAHAN */
    {0x0db8, 0x0e18}, /* XK_Thai_thothong               -> THAI CHARACTER THO THONG */
    {0x0db9, 0x0e19}, /* XK_Thai_nonu                   -> THAI CHARACTER NO NU */
    {0x0dba, 0x0e1a}, /* XK_Thai_bobaimai               -> THAI CHARACTER BO BAIMAI */
    {0x0dbb, 0x0e1b}, /* XK_Thai_popla                  -> THAI CHARACTER PO PLA */
    {0x0dbc, 0x0e1c}, /* XK_Thai_phophung               -> THAI CHARACTER PHO PHUNG */
    {0x0dbd, 0x0e1d}, /* XK_Thai_fofa                   -> THAI CHARACTER FO FA */
    {0x0dbe, 0x0e1e}, /* XK_Thai_phophan                -> THAI CHARACTER PHO PHAN */
    {0x0dbf, 0x0e1f}, /* XK_Thai_fofan                  -> THAI CHARACTER FO FAN */
    {0x0dc0, 0x0e20}, /* XK_Thai_phosamphao             -> THAI CHARACTER PHO SAMPHAO */
    {0x0dc1, 0x0e21}, /* XK_Thai_moma                   -> THAI CHARACTER MO MA */
    {0x0dc2, 0x0e22}, /* XK_Thai_yoyak                  -> THAI CHARACTER YO YAK */
    {0x0dc3, 0x0e23}, /* XK_Thai_rorua                  -> THAI CHARACTER RO RUA */
    {0x0dc4, 0x0e24}, /* XK_Thai_ru                     -> THAI CHARACTER RU */
    {0x0dc5, 0x0e25}, /* XK_Thai_loling                 -> THAI CHARACTER LO LING */
    {0x0dc6, 0x0e26}, /* XK_Thai_lu                     -> THAI CHARACTER LU */
    {0x0dc7, 0x0e27}, /* XK_Thai_wowaen                 -> THAI CHARACTER WO WAEN */
    {0x0dc8, 0x0e28}, /* XK_Thai_sosala                 -> THAI CHARACTER SO SALA */
    {0x0dc9, 0x0e29}, /* XK_Thai_sorusi                 -> THAI CHARACTER SO RUSI */
    {0x0dca, 0x0e2a}, /* XK_Thai_sosua                  -> THAI CHARACTER SO SUA */
    {0x0dcb, 0x0e2b}, /* XK_Thai_hohip                  -> THAI CHARACTER HO HIP */
    {0x0dcc, 0x0e2c}, /* XK_Thai_lochula                -> THAI CHARACTER LO CHULA */
    {0x0dcd, 0x0e2d}, /* XK_Thai_oang                   -> THAI CHARACTER O ANG */
    {0x0dce, 0x0e2e}, /* XK_Thai_honokhuk               -> THAI CHARACTER HO NOKHUK */
    {0x0dcf, 0x0e2f}, /* XK_Thai_paiyannoi              -> THAI CHARACTER PAIYANNOI */
    {0x0dd0, 0x0e30}, /* XK_Thai_saraa                  -> THAI CHARACTER SARA A */
    {0x0dd1, 0x0e31}, /* XK_Thai_maihanakat             -> THAI CHARACTER MAI HAN-AKAT */
    {0x0dd2, 0x0e32}, /* XK_Thai_saraaa                 -> THAI CHARACTER SARA AA */
    {0x0dd3, 0x0e33}, /* XK_Thai_saraam                 -> THAI CHARACTER SARA AM */
    {0x0dd4, 0x0e34}, /* XK_Thai_sarai                  -> THAI CHARACTER SARA I */
    {0x0dd5, 0x0e35}, /* XK_Thai_saraii                 -> THAI CHARACTER SARA II */
    {0x0dd6, 0x0e36}, /* XK_Thai_saraue                 -> THAI CHARACTER SARA UE */
    {0x0dd7, 0x0e37}, /* XK_Thai_sarauee                -> THAI CHARACTER SARA UEE */
    {0x0dd8, 0x0e38}, /* XK_Thai_sarau                  -> THAI CHARACTER SARA U */
    {0x0dd9, 0x0e39}, /* XK_Thai_sarauu                 -> THAI CHARACTER SARA UU */
    {0x0dda, 0x0e3a}, /* XK_Thai_phinthu                -> THAI CHARACTER PHINTHU */
  /* 0x0dde              XK_Thai_maihanakat_maitho         No mapping defined */
    {0x0ddf, 0x0e3f}, /* XK_Thai_baht                   -> THAI CURRENCY SYMBOL BAHT */
    {0x0de0, 0x0e40}, /* XK_Thai_sarae                  -> THAI CHARACTER SARA E */
    {0x0de1, 0x0e41}, /* XK_Thai_saraae                 -> THAI CHARACTER SARA AE */
    {0x0de2, 0x0e42}, /* XK_Thai_sarao                  -> THAI CHARACTER SARA O */
    {0x0de3, 0x0e43}, /* XK_Thai_saraaimaimuan          -> THAI CHARACTER SARA AI MAIMUAN */
    {0x0de4, 0x0e44}, /* XK_Thai_saraaimaimalai         -> THAI CHARACTER SARA AI MAIMALAI */
    {0x0de5, 0x0e45}, /* XK_Thai_lakkhangyao            -> THAI CHARACTER LAKKHANGYAO */
    {0x0de6, 0x0e46}, /* XK_Thai_maiyamok               -> THAI CHARACTER MAIYAMOK */
    {0x0de7, 0x0e47}, /* XK_Thai_maitaikhu              -> THAI CHARACTER MAITAIKHU */
    {0x0de8, 0x0e48}, /* XK_Thai_maiek                  -> THAI CHARACTER MAI EK */
    {0x0de9, 0x0e49}, /* XK_Thai_maitho                 -> THAI CHARACTER MAI THO */
    {0x0dea, 0x0e4a}, /* XK_Thai_maitri                 -> THAI CHARACTER MAI TRI */
    {0x0deb, 0x0e4b}, /* XK_Thai_maichattawa            -> THAI CHARACTER MAI CHATTAWA */
    {0x0dec, 0x0e4c}, /* XK_Thai_thanthakhat            -> THAI CHARACTER THANTHAKHAT */
    {0x0ded, 0x0e4d}, /* XK_Thai_nikhahit               -> THAI CHARACTER NIKHAHIT */
    {0x0df0, 0x0e50}, /* XK_Thai_leksun                 -> THAI DIGIT ZERO */
    {0x0df1, 0x0e51}, /* XK_Thai_leknung                -> THAI DIGIT ONE */
    {0x0df2, 0x0e52}, /* XK_Thai_leksong                -> THAI DIGIT TWO */
    {0x0df3, 0x0e53}, /* XK_Thai_leksam                 -> THAI DIGIT THREE */
    {0x0df4, 0x0e54}, /* XK_Thai_leksi                  -> THAI DIGIT FOUR */
    {0x0df5, 0x0e55}, /* XK_Thai_lekha                  -> THAI DIGIT FIVE */
    {0x0df6, 0x0e56}, /* XK_Thai_lekhok                 -> THAI DIGIT SIX */
    {0x0df7, 0x0e57}, /* XK_Thai_lekchet                -> THAI DIGIT SEVEN */
    {0x0df8, 0x0e58}, /* XK_Thai_lekpaet                -> THAI DIGIT EIGHT */
    {0x0df9, 0x0e59}, /* XK_Thai_lekkao                 -> THAI DIGIT NINE */
    {0x0ea1, 0x3131}, /* XK_Hangul_Kiyeog               -> HANGUL LETTER KIYEOK */
    {0x0ea2, 0x3132}, /* XK_Hangul_SsangKiyeog          -> HANGUL LETTER SSANGKIYEOK */
    {0x0ea3, 0x3133}, /* XK_Hangul_KiyeogSios           -> HANGUL LETTER KIYEOK-SIOS */
    {0x0ea4, 0x3134}, /* XK_Hangul_Nieun                -> HANGUL LETTER NIEUN */
    {0x0ea5, 0x3135}, /* XK_Hangul_NieunJieuj           -> HANGUL LETTER NIEUN-CIEUC */
    {0x0ea6, 0x3136}, /* XK_Hangul_NieunHieuh           -> HANGUL LETTER NIEUN-HIEUH */
    {0x0ea7, 0x3137}, /* XK_Hangul_Dikeud               -> HANGUL LETTER TIKEUT */
    {0x0ea8, 0x3138}, /* XK_Hangul_SsangDikeud          -> HANGUL LETTER SSANGTIKEUT */
    {0x0ea9, 0x3139}, /* XK_Hangul_Rieul                -> HANGUL LETTER RIEUL */
    {0x0eaa, 0x313a}, /* XK_Hangul_RieulKiyeog          -> HANGUL LETTER RIEUL-KIYEOK */
    {0x0eab, 0x313b}, /* XK_Hangul_RieulMieum           -> HANGUL LETTER RIEUL-MIEUM */
    {0x0eac, 0x313c}, /* XK_Hangul_RieulPieub           -> HANGUL LETTER RIEUL-PIEUP */
    {0x0ead, 0x313d}, /* XK_Hangul_RieulSios            -> HANGUL LETTER RIEUL-SIOS */
    {0x0eae, 0x313e}, /* XK_Hangul_RieulTieut           -> HANGUL LETTER RIEUL-THIEUTH */
    {0x0eaf, 0x313f}, /* XK_Hangul_RieulPhieuf          -> HANGUL LETTER RIEUL-PHIEUPH */
    {0x0eb0, 0x3140}, /* XK_Hangul_RieulHieuh           -> HANGUL LETTER RIEUL-HIEUH */
    {0x0eb1, 0x3141}, /* XK_Hangul_Mieum                -> HANGUL LETTER MIEUM */
    {0x0eb2, 0x3142}, /* XK_Hangul_Pieub                -> HANGUL LETTER PIEUP */
    {0x0eb3, 0x3143}, /* XK_Hangul_SsangPieub           -> HANGUL LETTER SSANGPIEUP */
    {0x0eb4, 0x3144}, /* XK_Hangul_PieubSios            -> HANGUL LETTER PIEUP-SIOS */
    {0x0eb5, 0x3145}, /* XK_Hangul_Sios                 -> HANGUL LETTER SIOS */
    {0x0eb6, 0x3146}, /* XK_Hangul_SsangSios            -> HANGUL LETTER SSANGSIOS */
    {0x0eb7, 0x3147}, /* XK_Hangul_Ieung                -> HANGUL LETTER IEUNG */
    {0x0eb8, 0x3148}, /* XK_Hangul_Jieuj                -> HANGUL LETTER CIEUC */
    {0x0eb9, 0x3149}, /* XK_Hangul_SsangJieuj           -> HANGUL LETTER SSANGCIEUC */
    {0x0eba, 0x314a}, /* XK_Hangul_Cieuc                -> HANGUL LETTER CHIEUCH */
    {0x0ebb, 0x314b}, /* XK_Hangul_Khieuq               -> HANGUL LETTER KHIEUKH */
    {0x0ebc, 0x314c}, /* XK_Hangul_Tieut                -> HANGUL LETTER THIEUTH */
    {0x0ebd, 0x314d}, /* XK_Hangul_Phieuf               -> HANGUL LETTER PHIEUPH */
    {0x0ebe, 0x314e}, /* XK_Hangul_Hieuh                -> HANGUL LETTER HIEUH */
    {0x0ebf, 0x314f}, /* XK_Hangul_A                    -> HANGUL LETTER A */
    {0x0ec0, 0x3150}, /* XK_Hangul_AE                   -> HANGUL LETTER AE */
    {0x0ec1, 0x3151}, /* XK_Hangul_YA                   -> HANGUL LETTER YA */
    {0x0ec2, 0x3152}, /* XK_Hangul_YAE                  -> HANGUL LETTER YAE */
    {0x0ec3, 0x3153}, /* XK_Hangul_EO                   -> HANGUL LETTER EO */
    {0x0ec4, 0x3154}, /* XK_Hangul_E                    -> HANGUL LETTER E */
    {0x0ec5, 0x3155}, /* XK_Hangul_YEO                  -> HANGUL LETTER YEO */
    {0x0ec6, 0x3156}, /* XK_Hangul_YE                   -> HANGUL LETTER YE */
    {0x0ec7, 0x3157}, /* XK_Hangul_O                    -> HANGUL LETTER O */
    {0x0ec8, 0x3158}, /* XK_Hangul_WA                   -> HANGUL LETTER WA */
    {0x0ec9, 0x3159}, /* XK_Hangul_WAE                  -> HANGUL LETTER WAE */
    {0x0eca, 0x315a}, /* XK_Hangul_OE                   -> HANGUL LETTER OE */
    {0x0ecb, 0x315b}, /* XK_Hangul_YO                   -> HANGUL LETTER YO */
    {0x0ecc, 0x315c}, /* XK_Hangul_U                    -> HANGUL LETTER U */
    {0x0ecd, 0x315d}, /* XK_Hangul_WEO                  -> HANGUL LETTER WEO */
    {0x0ece, 0x315e}, /* XK_Hangul_WE                   -> HANGUL LETTER WE */
    {0x0ecf, 0x315f}, /* XK_Hangul_WI                   -> HANGUL LETTER WI */
    {0x0ed0, 0x3160}, /* XK_Hangul_YU                   -> HANGUL LETTER YU */
    {0x0ed1, 0x3161}, /* XK_Hangul_EU                   -> HANGUL LETTER EU */
    {0x0ed2, 0x3162}, /* XK_Hangul_YI                   -> HANGUL LETTER YI */
    {0x0ed3, 0x3163}, /* XK_Hangul_I                    -> HANGUL LETTER I */
    {0x0ed4, 0x11a8}, /* XK_Hangul_J_Kiyeog             -> HANGUL JONGSEONG KIYEOK */
    {0x0ed5, 0x11a9}, /* XK_Hangul_J_SsangKiyeog        -> HANGUL JONGSEONG SSANGKIYEOK */
    {0x0ed6, 0x11aa}, /* XK_Hangul_J_KiyeogSios         -> HANGUL JONGSEONG KIYEOK-SIOS */
    {0x0ed7, 0x11ab}, /* XK_Hangul_J_Nieun              -> HANGUL JONGSEONG NIEUN */
    {0x0ed8, 0x11ac}, /* XK_Hangul_J_NieunJieuj         -> HANGUL JONGSEONG NIEUN-CIEUC */
    {0x0ed9, 0x11ad}, /* XK_Hangul_J_NieunHieuh         -> HANGUL JONGSEONG NIEUN-HIEUH */
    {0x0eda, 0x11ae}, /* XK_Hangul_J_Dikeud             -> HANGUL JONGSEONG TIKEUT */
    {0x0edb, 0x11af}, /* XK_Hangul_J_Rieul              -> HANGUL JONGSEONG RIEUL */
    {0x0edc, 0x11b0}, /* XK_Hangul_J_RieulKiyeog        -> HANGUL JONGSEONG RIEUL-KIYEOK */
    {0x0edd, 0x11b1}, /* XK_Hangul_J_RieulMieum         -> HANGUL JONGSEONG RIEUL-MIEUM */
    {0x0ede, 0x11b2}, /* XK_Hangul_J_RieulPieub         -> HANGUL JONGSEONG RIEUL-PIEUP */
    {0x0edf, 0x11b3}, /* XK_Hangul_J_RieulSios          -> HANGUL JONGSEONG RIEUL-SIOS */
    {0x0ee0, 0x11b4}, /* XK_Hangul_J_RieulTieut         -> HANGUL JONGSEONG RIEUL-THIEUTH */
    {0x0ee1, 0x11b5}, /* XK_Hangul_J_RieulPhieuf        -> HANGUL JONGSEONG RIEUL-PHIEUPH */
    {0x0ee2, 0x11b6}, /* XK_Hangul_J_RieulHieuh         -> HANGUL JONGSEONG RIEUL-HIEUH */
    {0x0ee3, 0x11b7}, /* XK_Hangul_J_Mieum              -> HANGUL JONGSEONG MIEUM */
    {0x0ee4, 0x11b8}, /* XK_Hangul_J_Pieub              -> HANGUL JONGSEONG PIEUP */
    {0x0ee5, 0x11b9}, /* XK_Hangul_J_PieubSios          -> HANGUL JONGSEONG PIEUP-SIOS */
    {0x0ee6, 0x11ba}, /* XK_Hangul_J_Sios               -> HANGUL JONGSEONG SIOS */
    {0x0ee7, 0x11bb}, /* XK_Hangul_J_SsangSios          -> HANGUL JONGSEONG SSANGSIOS */
    {0x0ee8, 0x11bc}, /* XK_Hangul_J_Ieung              -> HANGUL JONGSEONG IEUNG */
    {0x0ee9, 0x11bd}, /* XK_Hangul_J_Jieuj              -> HANGUL JONGSEONG CIEUC */
    {0x0eea, 0x11be}, /* XK_Hangul_J_Cieuc              -> HANGUL JONGSEONG CHIEUCH */
    {0x0eeb, 0x11bf}, /* XK_Hangul_J_Khieuq             -> HANGUL JONGSEONG KHIEUKH */
    {0x0eec, 0x11c0}, /* XK_Hangul_J_Tieut              -> HANGUL JONGSEONG THIEUTH */
    {0x0eed, 0x11c1}, /* XK_Hangul_J_Phieuf             -> HANGUL JONGSEONG PHIEUPH */
    {0x0eee, 0x11c2}, /* XK_Hangul_J_Hieuh              -> HANGUL JONGSEONG HIEUH */
    {0x0eef, 0x316d}, /* XK_Hangul_RieulYeorinHieuh     -> HANGUL LETTER RIEUL-YEORINHIEUH */
    {0x0ef0, 0x3171}, /* XK_Hangul_SunkyeongeumMieum    -> HANGUL LETTER KAPYEOUNMIEUM */
    {0x0ef1, 0x3178}, /* XK_Hangul_SunkyeongeumPieub    -> HANGUL LETTER KAPYEOUNPIEUP */
    {0x0ef2, 0x317f}, /* XK_Hangul_PanSios              -> HANGUL LETTER PANSIOS */
    {0x0ef3, 0x3181}, /* XK_Hangul_KkogjiDalrinIeung    -> HANGUL LETTER YESIEUNG */
    {0x0ef4, 0x3184}, /* XK_Hangul_SunkyeongeumPhieuf   -> HANGUL LETTER KAPYEOUNPHIEUPH */
    {0x0ef5, 0x3186}, /* XK_Hangul_YeorinHieuh          -> HANGUL LETTER YEORINHIEUH */
    {0x0ef6, 0x318d}, /* XK_Hangul_AraeA                -> HANGUL LETTER ARAEA */
    {0x0ef7, 0x318e}, /* XK_Hangul_AraeAE               -> HANGUL LETTER ARAEAE */
    {0x0ef8, 0x11eb}, /* XK_Hangul_J_PanSios            -> HANGUL JONGSEONG PANSIOS */
    {0x0ef9, 0x11f0}, /* XK_Hangul_J_KkogjiDalrinIeung  -> HANGUL JONGSEONG YESIEUNG */
    {0x0efa, 0x11f9}, /* XK_Hangul_J_YeorinHieuh        -> HANGUL JONGSEONG YEORINHIEUH */
    {0x0eff, 0x20a9}, /* XK_Korean_Won                  -> WON SIGN */
    {0x13a4, 0x20ac}, /*                                -> EURO SIGN */
    {0x13bc, 0x0152}, /* XK_OE                          -> LATIN CAPITAL LIGATURE OE */
    {0x13bd, 0x0153}, /* XK_oe                          -> LATIN SMALL LIGATURE OE */
    {0x13be, 0x0178}, /* XK_Ydiaeresis                  -> LATIN CAPITAL LETTER Y WITH DIAERESIS */
    {0x20a9, 0x20a9}, /*                                -> WON SIGN */
    {0x20ac, 0x20ac}, /*                                -> EURO SIGN */
};



static charType mapKeysymToUnicode (KeySym keysym)
  {
    int lower = 0;
    int upper = sizeof(keysymTable) / sizeof(struct keysymCharPair) - 1;
    int middle;
    charType result;

  /* mapKeysymToUnicode */
    if (keysym >= 0x01000100 && keysym <= 0x0110ffff) {
      result = (charType) (keysym - 0x01000000);
    } else if (keysym >= 0x0100 && keysym <= 0x20ff) {
      result = K_UNDEF;
      while (upper >= lower) {
        middle = (lower + upper) / 2;
        if (keysymTable[middle].keysym < keysym) {
          lower = middle + 1;
        } else if (keysymTable[middle].keysym == keysym) {
          result = keysymTable[middle].unicodeChar;
          lower = upper + 1;
        } else {
          upper = middle - 1;
        } /* if */
      } /* while */
    } else {
      /* printf("1 undef key: %ld %lx\n", (long) keysym, (long) keysym); */
      result = K_UNDEF;
    } /* if */
    return result;
  } /* mapKeysymToUnicode */



winType find_window (Window sys_window)

  {
    winType window;

  /* find_window */
    if (window_hash == NULL) {
      window = NULL;
    } else {
      window = (winType) (memSizeType)
          hshIdxWithDefault(window_hash,
                            (genericType) (memSizeType) sys_window,
                            (genericType) (memSizeType) NULL,
                            (intType) ((memSizeType) sys_window) >> 6,
                            (compareType) &genericCmp);
    } /* if */
    logFunction(printf("find_window(" FMT_X_MEM ") --> %" FMT_X_MEM "\n",
                       (memSizeType) sys_window, (memSizeType) window););
    return window;
  } /* find_window */



void enter_window (winType curr_window, Window sys_window)

  { /* enter_window */
    /* printf("enter_window(%lx, %lx)\n", (unsigned long) curr_window, (unsigned long) sys_window); */
    if (window_hash == NULL) {
      window_hash = hshEmpty();
    } /* if */
    (void) hshIdxEnterDefault(window_hash,
                              (genericType) (memSizeType) sys_window,
                              (genericType) (memSizeType) curr_window,
                              (intType) ((memSizeType) sys_window) >> 6,
                              (compareType) &genericCmp,
                              (createFuncType) &genericCreate,
                              (createFuncType) &genericCreate);
  } /* enter_window */



void remove_window (Window sys_window)

  { /* remove_window */
    /* printf("remove_window(%lx)\n", (unsigned long) sys_window); */
    hshExcl(window_hash,
            (genericType) (memSizeType) sys_window,
            (intType) ((memSizeType) sys_window) >> 6,
            (compareType) &genericCmp,
            (destrFuncType) &genericDestr,
            (destrFuncType) &genericDestr);
  } /* remove_window */



void handleExpose (XExposeEvent *xexpose)

  {
    winType redraw_window;

  /* handleExpose */
    logFunction(printf("handleExpose\n"););
    /* printf("XExposeEvent x=%d, y=%d, width=%d, height=%d, count=%d\n",
        xexpose->x, xexpose->y, xexpose->width, xexpose->height, xexpose->count); */
    redraw_window = find_window(xexpose->window);
    redraw(redraw_window, xexpose->x, xexpose->y, xexpose->width, xexpose->height);
    logFunction(printf("handleExpose -->\n"););
  } /* handleExpose */



#ifdef OUT_OF_ORDER
void waitForReparent (void)

  { /* waitForReparent */
    XNextEvent(mydisplay, &currentEvent);
    switch(currentEvent.type) {
      case ReparentNotify:
      case ConfigureNotify:
        break;
      default:
        printf("Other Event %d\n", currentEvent.type);
        break;
    } /* switch */
    XNextEvent(mydisplay, &currentEvent);
    switch(currentEvent.type) {
      case ReparentNotify:
      case ConfigureNotify:
        break;
      default:
        printf("Other Event %d\n", currentEvent.type);
        break;
    } /* switch */
  } /* waitForReparent */
#endif



charType gkbGetc (void)

  {
    KeySym currentKey;
    int lookup_count;
    unsigned char buffer[21];
    boolType getNextChar;
    charType result;

  /* gkbGetc */
    logFunction(printf("gkbGetc\n"););
    do {
      getNextChar = FALSE;
      flushBeforeRead();
      result = K_NONE;
      if (eventPresent) {
        eventPresent = FALSE;
      } else {
        XNextEvent(mydisplay, &currentEvent);
      } /* if */
      while (currentEvent.type == KeyRelease) {
        XNextEvent(mydisplay, &currentEvent);
      } /* while */
      switch(currentEvent.type) {
        case Expose:
#ifdef FLAG_EVENTS
          printf("Expose\n");
#endif
          handleExpose(&currentEvent.xexpose);
          getNextChar = TRUE;
          break;

#ifdef OUT_OF_ORDER
        case ConfigureNotify:
#ifdef FLAG_EVENTS
          printf("ConfigureNotify");
#endif
          configure(&currentEvent.xconfigure);
          getNextChar = TRUE;
          break;
        case MapNotify:
#endif

#ifdef TRACE_REPARENT_NOTIFY
        case ReparentNotify:
          printf("gkbGetc: Reparent\n");
          getNextChar = TRUE;
          break;

        case ConfigureNotify:
          printf("gkbGetc: Configure %lx %d, %d\n",
                 currentEvent.xconfigure.window,
                 currentEvent.xconfigure.width, currentEvent.xconfigure.height);
          getNextChar = TRUE;
          break;
#endif
#ifdef ALLOW_REPARENT_NOTIFY
        case ReparentNotify:
        case ConfigureNotify:
#endif
        case GraphicsExpose:
        case NoExpose:
#ifdef FLAG_EVENTS
          printf("NoExpose\n");
#endif
          getNextChar = TRUE;
          break;

        case MappingNotify:
#ifdef FLAG_EVENTS
          printf("MappingNotify\n");
#endif
          XRefreshKeyboardMapping(&currentEvent.xmapping);
          break;

#ifdef OUT_OF_ORDER
        case DestroyNotify:
#ifdef FLAG_EVENTS
          printf("DestroyNotify\n");
#endif
          exit(1);
          break;
#endif

        case ClientMessage:
#ifdef FLAG_EVENTS
          printf("ClientMessage\n");
#endif
          if ((Atom) currentEvent.xclient.data.l[0] == wm_delete_window) {
            /* printf("do exit\n"); */
            exit(1);
          } /* if */
          break;

        case ButtonPress:
#ifdef FLAG_EVENTS
          printf("ButtonPress (%d, %d, %u %lu)\n",
              currentEvent.xbutton.x, currentEvent.xbutton.y,
              currentEvent.xbutton.button, (unsigned long) currentEvent.xbutton.window);
#endif
          button_x = currentEvent.xbutton.x;
          button_y = currentEvent.xbutton.y;
          button_window = currentEvent.xbutton.window;
          if (currentEvent.xbutton.button >= 1 && currentEvent.xbutton.button <= 5) {
            result = currentEvent.xbutton.button + K_MOUSE1 - 1;
          } else {
            result = K_UNDEF;
          } /* if */
          break;

        case KeyPress:
#ifdef FLAG_EVENTS
          printf("KeyPress\n");
          printf("xkey.state (%o)\n", currentEvent.xkey.state);
#endif
          lookup_count = XLookupString(&currentEvent.xkey, (cstriType) buffer,
                                       20, &currentKey, 0);
          buffer[lookup_count] = '\0';
          if (currentEvent.xkey.state & ShiftMask) {
            /* printf("ShiftMask\n"); */
            switch (currentKey) {
              case XK_Return:     result = K_NL;          break;
              case XK_BackSpace:  result = K_BS;          break;
              case XK_ISO_Left_Tab:
              case XK_Tab:        result = K_BACKTAB;     break;
              case XK_Linefeed:   result = K_NL;          break;
              case XK_Escape:     result = K_ESC;         break;
              case XK_F1:         result = K_SFT_F1;      break;
              case XK_F2:         result = K_SFT_F2;      break;
              case XK_F3:         result = K_SFT_F3;      break;
              case XK_F4:         result = K_SFT_F4;      break;
              case XK_F5:         result = K_SFT_F5;      break;
              case XK_F6:         result = K_SFT_F6;      break;
              case XK_F7:         result = K_SFT_F7;      break;
              case XK_F8:         result = K_SFT_F8;      break;
              case XK_F9:         result = K_SFT_F9;      break;
              case XK_F10:        result = K_SFT_F10;     break;
              case XK_F11:        result = K_SFT_F11;     break;
              case XK_F12:        result = K_SFT_F12;     break;
              case XK_Left:       result = K_LEFT;        break;
              case XK_Right:      result = K_RIGHT;       break;
              case XK_Up:         result = K_UP;          break;
              case XK_Down:       result = K_DOWN;        break;
              case XK_Home:       result = K_HOME;        break;
              case XK_End:        result = K_END;         break;
              case XK_Prior:      result = K_PGUP;        break;
              case XK_Next:       result = K_PGDN;        break;
              case XK_Insert:     result = K_INS;         break;
              case XK_Delete:     result = K_DEL;         break;
              case XK_KP_Left:    result = K_LEFT;        break;
              case XK_KP_Right:   result = K_RIGHT;       break;
              case XK_KP_Up:      result = K_UP;          break;
              case XK_KP_Down:    result = K_DOWN;        break;
              case XK_KP_Home:    result = K_HOME;        break;
              case XK_KP_End:     result = K_END;         break;
              case XK_KP_Prior:   result = K_PGUP;        break;
              case XK_KP_Next:    result = K_PGDN;        break;
              case XK_KP_Insert:  result = K_INS;         break;
              case XK_KP_Delete:  result = K_DEL;         break;
              case XK_KP_Begin:   result = K_PAD_CENTER;  break;
              case XK_KP_Enter:   result = K_NL;          break;
              case XK_KP_Decimal: result = K_DEL;         break;
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
                getNextChar = TRUE;
                break;
              default:
                if (lookup_count == 1) {
                  result = buffer[0];
                } else {
                  result = mapKeysymToUnicode(currentKey);
                } /* if */
                break;
            } /* switch */
          } else if (currentEvent.xkey.state & ControlMask) {
            /* printf("ControlMask\n"); */
            switch (currentKey) {
              case XK_Return:     result = K_CTL_NL;      break;
              case XK_BackSpace:  result = K_UNDEF;       break;
              case XK_Tab:        result = K_UNDEF;       break;
              case XK_Linefeed:   result = K_CTL_NL;      break;
              case XK_Escape:     result = K_UNDEF;       break;
              case XK_F1:         result = K_CTL_F1;      break;
              case XK_F2:         result = K_CTL_F2;      break;
              case XK_F3:         result = K_CTL_F3;      break;
              case XK_F4:         result = K_CTL_F4;      break;
              case XK_F5:         result = K_CTL_F5;      break;
              case XK_F6:         result = K_CTL_F6;      break;
              case XK_F7:         result = K_CTL_F7;      break;
              case XK_F8:         result = K_CTL_F8;      break;
              case XK_F9:         result = K_CTL_F9;      break;
              case XK_F10:        result = K_CTL_F10;     break;
              case XK_F11:        result = K_CTL_F11;     break;
              case XK_F12:        result = K_CTL_F12;     break;
              case XK_Left:       result = K_CTL_LEFT;    break;
              case XK_Right:      result = K_CTL_RIGHT;   break;
              case XK_Up:         result = K_CTL_UP;      break;
              case XK_Down:       result = K_CTL_DOWN;    break;
              case XK_Home:       result = K_CTL_HOME;    break;
              case XK_End:        result = K_CTL_END;     break;
              case XK_Prior:      result = K_CTL_PGUP;    break;
              case XK_Next:       result = K_CTL_PGDN;    break;
              case XK_Insert:     result = K_CTL_INS;     break;
              case XK_Delete:     result = K_CTL_DEL;     break;
              case XK_KP_Left:    result = K_CTL_LEFT;    break;
              case XK_KP_Right:   result = K_CTL_RIGHT;   break;
              case XK_KP_Up:      result = K_CTL_UP;      break;
              case XK_KP_Down:    result = K_CTL_DOWN;    break;
              case XK_KP_Home:    result = K_CTL_HOME;    break;
              case XK_KP_End:     result = K_CTL_END;     break;
              case XK_KP_Prior:   result = K_CTL_PGUP;    break;
              case XK_KP_Next:    result = K_CTL_PGDN;    break;
              case XK_KP_Insert:  result = K_CTL_INS;     break;
              case XK_KP_Delete:  result = K_CTL_DEL;     break;
              case XK_KP_Begin:   result = K_UNDEF;       break;
              case XK_KP_4:       result = K_CTL_LEFT;    break;
              case XK_KP_6:       result = K_CTL_RIGHT;   break;
              case XK_KP_8:       result = K_CTL_UP;      break;
              case XK_KP_2:       result = K_CTL_DOWN;    break;
              case XK_KP_7:       result = K_CTL_HOME;    break;
              case XK_KP_1:       result = K_CTL_END;     break;
              case XK_KP_9:       result = K_CTL_PGUP;    break;
              case XK_KP_3:       result = K_CTL_PGDN;    break;
              case XK_KP_0:       result = K_CTL_INS;     break;
              case XK_KP_5:       result = K_UNDEF;       break;
              case XK_KP_Enter:   result = K_CTL_NL;      break;
              case XK_KP_Decimal: result = K_CTL_DEL;     break;
              case XK_0:          result = '0';           break;
              case XK_1:          result = '1';           break;
              case XK_2:          result = '2';           break;
              case XK_3:          result = '3';           break;
              case XK_4:          result = '4';           break;
              case XK_5:          result = '5';           break;
              case XK_6:          result = '6';           break;
              case XK_7:          result = '7';           break;
              case XK_8:          result = '8';           break;
              case XK_9:          result = '9';           break;
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
                getNextChar = TRUE;
                break;
              default:
                if (lookup_count == 1) {
                  result = buffer[0];
                } else {
                  result = mapKeysymToUnicode(currentKey);
                } /* if */
                break;
            } /* switch */
          } else if (currentEvent.xkey.state & Mod1Mask) { /* Left ALT modifier */
            /* printf("Mod1Mask\n"); */
            switch (currentKey) {
              case XK_Return:     result = K_UNDEF;       break;
              case XK_BackSpace:  result = K_UNDEF;       break;
              case XK_Tab:        result = K_UNDEF;       break;
              case XK_Linefeed:   result = K_UNDEF;       break;
              case XK_Escape:     result = K_UNDEF;       break;
              case XK_F1:         result = K_ALT_F1;      break;
              case XK_F2:         result = K_ALT_F2;      break;
              case XK_F3:         result = K_ALT_F3;      break;
              case XK_F4:         result = K_ALT_F4;      break;
              case XK_F5:         result = K_ALT_F5;      break;
              case XK_F6:         result = K_ALT_F6;      break;
              case XK_F7:         result = K_ALT_F7;      break;
              case XK_F8:         result = K_ALT_F8;      break;
              case XK_F9:         result = K_ALT_F9;      break;
              case XK_F10:        result = K_ALT_F10;     break;
              case XK_F11:        result = K_ALT_F11;     break;
              case XK_F12:        result = K_ALT_F12;     break;
              case XK_Left:       result = K_LEFT;        break;
              case XK_Right:      result = K_RIGHT;       break;
              case XK_Up:         result = K_UP;          break;
              case XK_Down:       result = K_DOWN;        break;
              case XK_Home:       result = K_HOME;        break;
              case XK_End:        result = K_END;         break;
              case XK_Prior:      result = K_PGUP;        break;
              case XK_Next:       result = K_PGDN;        break;
              case XK_Insert:     result = K_INS;         break;
              case XK_Delete:     result = K_DEL;         break;
              case XK_KP_Left:    result = K_LEFT;        break;
              case XK_KP_Right:   result = K_RIGHT;       break;
              case XK_KP_Up:      result = K_UP;          break;
              case XK_KP_Down:    result = K_DOWN;        break;
              case XK_KP_Home:    result = K_HOME;        break;
              case XK_KP_End:     result = K_END;         break;
              case XK_KP_Prior:   result = K_PGUP;        break;
              case XK_KP_Next:    result = K_PGDN;        break;
              case XK_KP_Insert:  result = K_INS;         break;
              case XK_KP_Delete:  result = K_DEL;         break;
              case XK_KP_Begin:   result = K_PAD_CENTER;  break;
              case XK_KP_0:       result = K_ALT_0;       break;
              case XK_KP_1:       result = K_ALT_1;       break;
              case XK_KP_2:       result = K_ALT_2;       break;
              case XK_KP_3:       result = K_ALT_3;       break;
              case XK_KP_4:       result = K_ALT_4;       break;
              case XK_KP_5:       result = K_ALT_5;       break;
              case XK_KP_6:       result = K_ALT_6;       break;
              case XK_KP_7:       result = K_ALT_7;       break;
              case XK_KP_8:       result = K_ALT_8;       break;
              case XK_KP_9:       result = K_ALT_9;       break;
              case XK_KP_Enter:   result = K_NL;          break;
              case XK_KP_Decimal: result = K_UNDEF;       break;
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
                getNextChar = TRUE;
                break;
              default:
                if (lookup_count == 1) {
                  switch (buffer[0]) {
                    case '0':     result = K_ALT_0;       break;
                    case '1':     result = K_ALT_1;       break;
                    case '2':     result = K_ALT_2;       break;
                    case '3':     result = K_ALT_3;       break;
                    case '4':     result = K_ALT_4;       break;
                    case '5':     result = K_ALT_5;       break;
                    case '6':     result = K_ALT_6;       break;
                    case '7':     result = K_ALT_7;       break;
                    case '8':     result = K_ALT_8;       break;
                    case '9':     result = K_ALT_9;       break;
                    case 'a':     result = K_ALT_A;       break;
                    case 'b':     result = K_ALT_B;       break;
                    case 'c':     result = K_ALT_C;       break;
                    case 'd':     result = K_ALT_D;       break;
                    case 'e':     result = K_ALT_E;       break;
                    case 'f':     result = K_ALT_F;       break;
                    case 'g':     result = K_ALT_G;       break;
                    case 'h':     result = K_ALT_H;       break;
                    case 'i':     result = K_ALT_I;       break;
                    case 'j':     result = K_ALT_J;       break;
                    case 'k':     result = K_ALT_K;       break;
                    case 'l':     result = K_ALT_L;       break;
                    case 'm':     result = K_ALT_M;       break;
                    case 'n':     result = K_ALT_N;       break;
                    case 'o':     result = K_ALT_O;       break;
                    case 'p':     result = K_ALT_P;       break;
                    case 'q':     result = K_ALT_Q;       break;
                    case 'r':     result = K_ALT_R;       break;
                    case 's':     result = K_ALT_S;       break;
                    case 't':     result = K_ALT_T;       break;
                    case 'u':     result = K_ALT_U;       break;
                    case 'v':     result = K_ALT_V;       break;
                    case 'w':     result = K_ALT_W;       break;
                    case 'x':     result = K_ALT_X;       break;
                    case 'y':     result = K_ALT_Y;       break;
                    case 'z':     result = K_ALT_Z;       break;
                    default:
                      result = buffer[0];
                      break;
                  } /* switch */
                } else {
                  result = mapKeysymToUnicode(currentKey);
                } /* if */
                break;
            } /* switch */
          } else if (currentEvent.xkey.state & Mod2Mask) { /* Num Lock modifier */
            /* printf("Mod2Mask\n"); */
            switch (currentKey) {
              case XK_Return:     result = K_NL;          break;
              case XK_BackSpace:  result = K_BS;          break;
              case XK_Tab:        result = K_TAB;         break;
              case XK_Linefeed:   result = K_NL;          break;
              case XK_Escape:     result = K_ESC;         break;
              case XK_F1:         result = K_F1;          break;
              case XK_F2:         result = K_F2;          break;
              case XK_F3:         result = K_F3;          break;
              case XK_F4:         result = K_F4;          break;
              case XK_F5:         result = K_F5;          break;
              case XK_F6:         result = K_F6;          break;
              case XK_F7:         result = K_F7;          break;
              case XK_F8:         result = K_F8;          break;
              case XK_F9:         result = K_F9;          break;
              case XK_F10:        result = K_F10;         break;
              case XK_F11:        result = K_F11;         break;
              case XK_F12:        result = K_F12;         break;
              case XK_Left:       result = K_LEFT;        break;
              case XK_Right:      result = K_RIGHT;       break;
              case XK_Up:         result = K_UP;          break;
              case XK_Down:       result = K_DOWN;        break;
              case XK_Home:       result = K_HOME;        break;
              case XK_End:        result = K_END;         break;
              case XK_Prior:      result = K_PGUP;        break;
              case XK_Next:       result = K_PGDN;        break;
              case XK_Insert:     result = K_INS;         break;
              case XK_Delete:     result = K_DEL;         break;
              case XK_KP_Enter:   result = K_NL;          break;
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
              case XK_ISO_Level3_Shift:
                getNextChar = TRUE;
                break;
              default:
                if (lookup_count == 1) {
                  result = buffer[0];
                } else {
                  result = mapKeysymToUnicode(currentKey);
                } /* if */
                break;
            } /* switch */
          } else {
            switch (currentKey) {
              case XK_Return:     result = K_NL;          break;
              case XK_BackSpace:  result = K_BS;          break;
              case XK_Tab:        result = K_TAB;         break;
              case XK_Linefeed:   result = K_NL;          break;
              case XK_Escape:     result = K_ESC;         break;
              case XK_F1:         result = K_F1;          break;
              case XK_F2:         result = K_F2;          break;
              case XK_F3:         result = K_F3;          break;
              case XK_F4:         result = K_F4;          break;
              case XK_F5:         result = K_F5;          break;
              case XK_F6:         result = K_F6;          break;
              case XK_F7:         result = K_F7;          break;
              case XK_F8:         result = K_F8;          break;
              case XK_F9:         result = K_F9;          break;
              case XK_F10:        result = K_F10;         break;
              case XK_F11:        result = K_F11;         break;
              case XK_F12:        result = K_F12;         break;
              case XK_Left:       result = K_LEFT;        break;
              case XK_Right:      result = K_RIGHT;       break;
              case XK_Up:         result = K_UP;          break;
              case XK_Down:       result = K_DOWN;        break;
              case XK_Home:       result = K_HOME;        break;
              case XK_End:        result = K_END;         break;
              case XK_Prior:      result = K_PGUP;        break;
              case XK_Next:       result = K_PGDN;        break;
              case XK_Insert:     result = K_INS;         break;
              case XK_Delete:     result = K_DEL;         break;
              case XK_KP_Left:    result = K_LEFT;        break;
              case XK_KP_Right:   result = K_RIGHT;       break;
              case XK_KP_Up:      result = K_UP;          break;
              case XK_KP_Down:    result = K_DOWN;        break;
              case XK_KP_Home:    result = K_HOME;        break;
              case XK_KP_End:     result = K_END;         break;
              case XK_KP_Prior:   result = K_PGUP;        break;
              case XK_KP_Next:    result = K_PGDN;        break;
              case XK_KP_Insert:  result = K_INS;         break;
              case XK_KP_Delete:  result = K_DEL;         break;
              case XK_KP_Begin:   result = K_PAD_CENTER;  break;
              case XK_KP_4:       result = K_LEFT;        break;
              case XK_KP_6:       result = K_RIGHT;       break;
              case XK_KP_8:       result = K_UP;          break;
              case XK_KP_2:       result = K_DOWN;        break;
              case XK_KP_7:       result = K_HOME;        break;
              case XK_KP_1:       result = K_END;         break;
              case XK_KP_9:       result = K_PGUP;        break;
              case XK_KP_3:       result = K_PGDN;        break;
              case XK_KP_0:       result = K_INS;         break;
              case XK_KP_5:       result = K_UNDEF;       break;
              case XK_KP_Enter:   result = K_NL;          break;
              case XK_KP_Decimal: result = K_DEL;         break;
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
              case XK_ISO_Level3_Shift:
                getNextChar = TRUE;
                break;
              default:
                if (lookup_count == 1) {
                  result = buffer[0];
                  /* if (result != currentKey) {
                    printf("^ %04lx %04lx\n", currentKey, result);
                  } ** if */
                } else {
                  result = mapKeysymToUnicode(currentKey);
                } /* if */
                break;
            } /* switch */
          } /* if */
          break;
        default:
#ifdef FLAG_EVENTS
          printf("Other Event %d\n", currentEvent.type);
#endif
          getNextChar = TRUE;
          break;
      } /* switch */
    } while (getNextChar);
    logFunction(printf("gkbGetc --> key: \"%s\" %ld %lx %d\n",
                       buffer, (long) currentKey, (long) currentKey, result););
    return result;
  } /* gkbGetc */



boolType processEvents (void)

  {
    KeySym currentKey;
    int num_events;
    int lookup_count;
    char buffer[21];
    boolType result;

  /* processEvents */
    logFunction(printf("processEvents\n"););
    result = FALSE;
    if (!eventPresent) {
      num_events = XEventsQueued(mydisplay, QueuedAfterReading);
      while (num_events != 0) {
        XNextEvent(mydisplay, &currentEvent);
        switch(currentEvent.type) {
          case Expose:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            handleExpose(&currentEvent.xexpose);
            result = TRUE;
            break;
#ifdef OUT_OF_ORDER
          case ConfigureNotify:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            configure(&currentEvent.xconfigure);
            result = TRUE;
            break;
          case MapNotify:
#endif
#ifdef TRACE_REPARENT_NOTIFY
          case ReparentNotify:
            printf("processEvents: Reparent\n");
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
          case ConfigureNotify:
            printf("processEvents: Configure %lx %d, %d\n",
                   currentEvent.xconfigure.window,
                   currentEvent.xconfigure.width, currentEvent.xconfigure.height);
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
#endif
#ifdef ALLOW_REPARENT_NOTIFY
          case ReparentNotify:
          case ConfigureNotify:
#endif
          case GraphicsExpose:
          case NoExpose:
            if (num_events == 1) {
              num_events = XEventsQueued(mydisplay, QueuedAfterReading);
            } else {
              num_events--;
            } /* if */
            break;
          case KeyPress:
            lookup_count = XLookupString(&currentEvent.xkey, buffer,
                                         20, &currentKey, 0);
            buffer[lookup_count] = '\0';
            switch (currentKey) {
              case XK_Shift_L:
              case XK_Shift_R:
              case XK_Control_L:
              case XK_Control_R:
              case XK_Alt_L:
              case XK_Alt_R:
              case XK_Meta_L:
              case XK_Meta_R:
              case XK_Mode_switch:
              case XK_Caps_Lock:
              case XK_Num_Lock:
              case XK_Shift_Lock:
              case XK_ISO_Level3_Shift:
                if (num_events == 1) {
                  num_events = XEventsQueued(mydisplay, QueuedAfterReading);
                } else {
                  num_events--;
                } /* if */
                break;
              default:
                /* printf("currentKey=%d\n", currentKey); */
                num_events = 0;
                eventPresent = TRUE;
                break;
            } /* switch */
            break;
          default:
            /* printf("currentEvent.type=%d\n", currentEvent.type); */
            num_events = 0;
            eventPresent = TRUE;
            break;
        } /* switch */
      } /* while */
    } /* if */
    logFunction(printf("processEvents --> %d\n", result););
    return result;
  } /* processEvents */



boolType gkbKeyPressed (void)

  { /* gkbKeyPressed */
    logFunction(printf("gkbKeyPressed\n"););
    flushBeforeRead();
    processEvents();
    logFunction(printf("gkbKeyPressed --> %d\n", eventPresent););
    return eventPresent;
  } /* gkbKeyPressed */



static boolType mouseButtonPressed (unsigned int button_mask)

  {
    Window root;
    Window child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int keys_buttons;
    boolType result;

  /* mouseButtonPressed */
    /* printf("mouseButtonPressed(%x)\n", button_mask); */
    XQueryPointer(mydisplay, DefaultRootWindow(mydisplay), &root, &child,
                  &root_x, &root_y, &win_x, &win_y, &keys_buttons);
    /* printf("%lx, %lx, %d, %d, %d, %d, %x\n",
       root, child, root_x, root_y, win_x, win_y, keys_buttons); */
    result = (keys_buttons & button_mask) != 0;
    /* printf("mouseButtonPressed(%d) --> %d\n", button_mask, result); */
    return result;
  } /* mouseButtonPressed */



static boolType keyboardButtonPressed (KeySym sym)

  {
    char key_vector[32];
    KeyCode code;
    boolType use_dead_key;
    unsigned int byteindex;
    unsigned int bitindex;
    boolType result;

  /* keyboardButtonPressed */
    code = XKeysymToKeycode(mydisplay, sym);
    /* printf("keyboardButtonPressed: XKeysymToKeycode(%04lx) returns %d\n", sym, code); */
    if (code == 0) {
      /* printf("keyboardButtonPressed: XKeysymToKeycode(%04lx) returns 0\n", sym); */
      if (code == 0 && sym >= 0x0100 && sym <= 0x10FFFF) {
        /* printf("XKeysymToKeycode(%08lx)\n", sym + 0x01000000); */
        code = XKeysymToKeycode(mydisplay, sym + 0x01000000);
        if (code != 0) {
          sym += 0x01000000;
          /* printf("XKeysymToKeycode(%08lx) -> %04lx\n", sym, code); */
        } /* if */
      } /* if */
      if (code == 0) {
        use_dead_key = TRUE;
        switch (sym) {
          case '^':  sym = XK_dead_circumflex; break;
          case '~':  sym = XK_dead_tilde;      break;
          case 0xa8: sym = XK_dead_diaeresis;  break;
          case 0xaf: sym = XK_dead_macron;     break;
          case 0xb0: sym = XK_dead_abovering;  break;
          case 0xb4: sym = XK_dead_acute;      break;
          case 0xb8: sym = XK_dead_cedilla;    break;
          default:   use_dead_key = FALSE;     break;
        } /* switch */
        if (use_dead_key) {
          code = XKeysymToKeycode(mydisplay, sym);
        } /* if */
      } /* if */
    } /* if */
    if (code == 0) {
      result = 0;
    } else {
      byteindex = code >> 3;
      bitindex = code & 0x7;
      XQueryKeymap(mydisplay, key_vector);
      /* {
        unsigned int idx;

        printf("sym=%lx, code=%d, byteindex=%d, bitindex=%d\n", sym, code, byteindex, bitindex);
        for (idx = 0; idx < 32; idx++) {
          printf("%02x", key_vector[idx]);
        }
        printf("\n");
      } */
      result = 1 & (key_vector[byteindex] >> bitindex);
    } /* if */
    return result;
  } /* keyboardButtonPressed */



boolType gkbButtonPressed (charType button)

  {
    unsigned int button_mask = 0;
    KeySym sym1;
    KeySym sym2 = 0;
    KeySym sym3 = 0;
    int keysymIndex;
    boolType finished = FALSE;
    boolType result;

  /* gkbButtonPressed */
    logFunction(printf("gkbButtonPressed(%04x)\n", button););
    switch (button) {
      case K_CTL_A: case K_ALT_A: case 'A': case 'a': sym1 = 'A'; break;
      case K_CTL_B: case K_ALT_B: case 'B': case 'b': sym1 = 'B'; break;
      case K_CTL_C: case K_ALT_C: case 'C': case 'c': sym1 = 'C'; break;
      case K_CTL_D: case K_ALT_D: case 'D': case 'd': sym1 = 'D'; break;
      case K_CTL_E: case K_ALT_E: case 'E': case 'e': sym1 = 'E'; break;
      case K_CTL_F: case K_ALT_F: case 'F': case 'f': sym1 = 'F'; break;
      case K_CTL_G: case K_ALT_G: case 'G': case 'g': sym1 = 'G'; break;
      /* K_CTL_H */ case K_ALT_H: case 'H': case 'h': sym1 = 'H'; break;
      /* K_CTL_I */ case K_ALT_I: case 'I': case 'i': sym1 = 'I'; break;
      /* K_CTL_J */ case K_ALT_J: case 'J': case 'j': sym1 = 'J'; break;
      case K_CTL_K: case K_ALT_K: case 'K': case 'k': sym1 = 'K'; break;
      case K_CTL_L: case K_ALT_L: case 'L': case 'l': sym1 = 'L'; break;
      case K_CTL_M: case K_ALT_M: case 'M': case 'm': sym1 = 'M'; break;
      case K_CTL_N: case K_ALT_N: case 'N': case 'n': sym1 = 'N'; break;
      case K_CTL_O: case K_ALT_O: case 'O': case 'o': sym1 = 'O'; break;
      case K_CTL_P: case K_ALT_P: case 'P': case 'p': sym1 = 'P'; break;
      case K_CTL_Q: case K_ALT_Q: case 'Q': case 'q': sym1 = 'Q'; break;
      case K_CTL_R: case K_ALT_R: case 'R': case 'r': sym1 = 'R'; break;
      case K_CTL_S: case K_ALT_S: case 'S': case 's': sym1 = 'S'; break;
      case K_CTL_T: case K_ALT_T: case 'T': case 't': sym1 = 'T'; break;
      case K_CTL_U: case K_ALT_U: case 'U': case 'u': sym1 = 'U'; break;
      case K_CTL_V: case K_ALT_V: case 'V': case 'v': sym1 = 'V'; break;
      case K_CTL_W: case K_ALT_W: case 'W': case 'w': sym1 = 'W'; break;
      case K_CTL_X: case K_ALT_X: case 'X': case 'x': sym1 = 'X'; break;
      case K_CTL_Y: case K_ALT_Y: case 'Y': case 'y': sym1 = 'Y'; break;
      case K_CTL_Z: case K_ALT_Z: case 'Z': case 'z': sym1 = 'Z'; break;

      case K_ALT_0: case '0': sym1 = '0'; sym2 = XK_KP_0; break;
      case K_ALT_1: case '1': sym1 = '1'; sym2 = XK_KP_1; break;
      case K_ALT_2: case '2': sym1 = '2'; sym2 = XK_KP_2; break;
      case K_ALT_3: case '3': sym1 = '3'; sym2 = XK_KP_3; break;
      case K_ALT_4: case '4': sym1 = '4'; sym2 = XK_KP_4; break;
      case K_ALT_5: case '5': sym1 = '5'; sym2 = XK_KP_5; break;
      case K_ALT_6: case '6': sym1 = '6'; sym2 = XK_KP_6; break;
      case K_ALT_7: case '7': sym1 = '7'; sym2 = XK_KP_7; break;
      case K_ALT_8: case '8': sym1 = '8'; sym2 = XK_KP_8; break;
      case K_ALT_9: case '9': sym1 = '9'; sym2 = XK_KP_9; break;

      case K_F1:  case K_SFT_F1:  case K_CTL_F1:  case K_ALT_F1:  sym1 = XK_F1;  break;
      case K_F2:  case K_SFT_F2:  case K_CTL_F2:  case K_ALT_F2:  sym1 = XK_F2;  break;
      case K_F3:  case K_SFT_F3:  case K_CTL_F3:  case K_ALT_F3:  sym1 = XK_F3;  break;
      case K_F4:  case K_SFT_F4:  case K_CTL_F4:  case K_ALT_F4:  sym1 = XK_F4;  break;
      case K_F5:  case K_SFT_F5:  case K_CTL_F5:  case K_ALT_F5:  sym1 = XK_F5;  break;
      case K_F6:  case K_SFT_F6:  case K_CTL_F6:  case K_ALT_F6:  sym1 = XK_F6;  break;
      case K_F7:  case K_SFT_F7:  case K_CTL_F7:  case K_ALT_F7:  sym1 = XK_F7;  break;
      case K_F8:  case K_SFT_F8:  case K_CTL_F8:  case K_ALT_F8:  sym1 = XK_F8;  break;
      case K_F9:  case K_SFT_F9:  case K_CTL_F9:  case K_ALT_F9:  sym1 = XK_F9;  break;
      case K_F10: case K_SFT_F10: case K_CTL_F10: case K_ALT_F10: sym1 = XK_F10; break;
      case K_F11: case K_SFT_F11: case K_CTL_F11: case K_ALT_F11: sym1 = XK_F11; break;
      case K_F12: case K_SFT_F12: case K_CTL_F12: case K_ALT_F12: sym1 = XK_F12; break;

      case K_LEFT:  case K_CTL_LEFT:  sym1 = XK_Left;   sym2 = XK_KP_Left;   break;
      case K_RIGHT: case K_CTL_RIGHT: sym1 = XK_Right;  sym2 = XK_KP_Right;  break;
      case K_UP:    case K_CTL_UP:    sym1 = XK_Up;     sym2 = XK_KP_Up;     break;
      case K_DOWN:  case K_CTL_DOWN:  sym1 = XK_Down;   sym2 = XK_KP_Down;   break;
      case K_HOME:  case K_CTL_HOME:  sym1 = XK_Home;   sym2 = XK_KP_Home;   break;
      case K_END:   case K_CTL_END:   sym1 = XK_End;    sym2 = XK_KP_End;    break;
      case K_PGUP:  case K_CTL_PGUP:  sym1 = XK_Prior;  sym2 = XK_KP_Prior;  break;
      case K_PGDN:  case K_CTL_PGDN:  sym1 = XK_Next;   sym2 = XK_KP_Next;   break;
      case K_INS:   case K_CTL_INS:   sym1 = XK_Insert; sym2 = XK_KP_Insert; break;
      case K_DEL:   case K_CTL_DEL:   sym1 = XK_Delete; sym2 = XK_KP_Delete; break;

      case K_ESC:                 sym1 = XK_Escape;                break;
      case K_PAD_CENTER:          sym1 = XK_KP_Begin;              break;
      case K_BS:                  sym1 = XK_BackSpace; sym2 = 'H'; break;
      case K_NL:                  sym1 = XK_Return;    sym2 = 'J'; sym3 = XK_KP_Enter; break;
      case K_TAB: case K_BACKTAB: sym1 = XK_Tab;       sym2 = 'I'; break;

      case '*': sym1 = '*'; sym2 = XK_KP_Multiply; break;
      case '+': sym1 = '+'; sym2 = XK_KP_Add;      break;
      case '-': sym1 = '-'; sym2 = XK_KP_Subtract; break;
      case '/': sym1 = '/'; sym2 = XK_KP_Divide;   break;

      case ' ':  case '!':  case '\"': case '#':  case '$':
      case '%':  case '&':  case '\'': case '(':  case ')':
      case ',':  case '.':  case ':':  case ';':  case '<':
      case '=':  case '>':  case '?':  case '@':  case '[':
      case '\\': case ']':  case '^':  case '_':  case '`':
      case '{':  case '|':  case '}':  case '~':
        sym1 = (KeySym) button;
        break;

      /*
      case K_SCRLUP:
      case K_SCRLDN:
      case K_INSLN:
      case K_DELLN:
      case K_ERASE:
      case K_CTL_NL:
      case K_NULLCMD:
      case K_REDRAW:
      case K_NEWWINDOW:
      case K_UNDEF:
      case K_NONE:

      case K_NULCHAR:
      */

      case K_MOUSE1: button_mask = Button1Mask; break;
      case K_MOUSE2: button_mask = Button2Mask; break;
      case K_MOUSE3: button_mask = Button3Mask; break;
      case K_MOUSE4: button_mask = Button4Mask; break;
      case K_MOUSE5: button_mask = Button5Mask; break;

      default:
        if (button <= 0xff) {
          sym1 = (KeySym) button;
        } else if (button <= 0x318e) {
          sym1 = (KeySym) (button + 0x01000000);
          keysymIndex = sizeof(keysymTable) / sizeof(struct keysymCharPair) - 1;
          while (keysymIndex >= 0 && keysymTable[keysymIndex].unicodeChar != button) {
            keysymIndex--;
          } /* while */
          if (keysymIndex > -1) {
            sym2 = (KeySym) keysymTable[keysymIndex].keysym;
          } /* if */
        } else if (button <= 0x10FFFF) {
          sym1 = (KeySym) (button + 0x01000000);
          /* printf("2 button = %04x, keysym= %04x\n", button, sym1); */
        } else {
          result = FALSE;
          finished = TRUE;
        } /* if */
        break;
    } /* switch */

    if (!finished) {
      if (button_mask != 0) {
        result = mouseButtonPressed(button_mask);
      } else {
        result = keyboardButtonPressed(sym1);
        if (!result && sym2 != 0) {
          result = keyboardButtonPressed(sym2);
          if (!result && sym3 != 0) {
            result = keyboardButtonPressed(sym3);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("gkbButtonPressed -> %d\n", result););
    return result;
  } /* gkbButtonPressed */



charType gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



intType gkbButtonXpos (void)

  { /* gkbButtonXpos */
    logFunction(printf("gkbButtonXpos -> " FMT_D "\n", button_x););
    return button_x;
  } /* gkbButtonXpos */



intType gkbButtonYpos (void)

  { /* gkbButtonYpos */
    logFunction(printf("gkbButtonYpos -> " FMT_D "\n", button_y););
    return button_y;
  } /* gkbButtonYpos */



winType gkbWindow (void)

  {
    winType result;

  /* gkbWindow */
    logFunction(printf("gkbWindow\n"););
    result = find_window(button_window);
    if (result != NULL) {
      result->usage_count++;
    } /* if */
    logFunction(printf("gkbWindow -> " FMT_U_MEM "\n", (memSizeType) result););
    return result;
  } /* gkbWindow */



void drwFlush (void)

  { /* drwFlush */
/*  printf("drwFlush()\n"); */
    processEvents();
    do {
      doFlush();
    } while (processEvents());
  } /* drwFlush */
