/********************************************************************/
/*                                                                  */
/*  str_rtl.c     Primitive actions for the string type.            */
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
/*  File: seed7/src/str_rtl.c                                       */
/*  Changes: 1991 - 1994, 2005, 2008 - 2018  Thomas Mertes          */
/*  Content: Primitive actions for the string type.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "wchar.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "arr_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "str_rtl.h"


#define CHAR_DELTA_BEYOND  128
#define INITIAL_ARRAY_SIZE 256
#define ARRAY_SIZE_DELTA   256
#define BOYER_MOORE_SEARCHED_STRI_THRESHOLD    2
#define BOYER_MOORE_MAIN_STRI_THRESHOLD     1400

/* memset_to_strelem is not used because it is */
/* only better for lengths greater than 7.     */
#define LPAD_WITH_MEMSET_TO_STRELEM 0



#if HAS_WMEMCMP && WCHAR_T_SIZE == 32
#define memcmp_strelem(mem1, mem2, len) \
    wmemcmp((const wchar_t *) mem1, (const wchar_t *) mem2, (size_t) len)
#else



static inline int memcmp_strelem (register const strElemType *mem1,
    register const strElemType *mem2, memSizeType len)

  { /* memcmp_strelem */
    for (; len > 0; mem1++, mem2++, len--) {
      if (*mem1 != *mem2) {
        return *mem1 < *mem2 ? -1 : 1;
      } /* if */
    } /* for */
    return 0;
  } /* memcmp_strelem */

#endif



static inline const strElemType *search_strelem2 (const strElemType *mem,
    const strElemType ch, const strElemType *const beyond,
    const memSizeType charDelta[])

  { /* search_strelem2 */
    while (mem < beyond) {
      if (*mem == ch) {
        return mem;
      } else if (*mem < CHAR_DELTA_BEYOND) {
        mem += charDelta[*mem];
      } else {
        mem += charDelta[CHAR_DELTA_BEYOND];
      } /* if */
    } /* while */
    return NULL;
  } /* search_strelem2 */



static inline const strElemType *rsearch_strelem (const strElemType *mem,
    const strElemType ch, size_t len)

  { /* rsearch_strelem */
    for (; len > 0; mem--, len--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* rsearch_strelem */



static inline const strElemType *rsearch_strelem2 (const strElemType *mem,
    const strElemType ch, const strElemType *const beyond,
    const memSizeType charDelta[])

  { /* rsearch_strelem2 */
    while (mem > beyond) {
      if (*mem == ch) {
        return mem;
      } else if (*mem < CHAR_DELTA_BEYOND) {
        mem -= charDelta[*mem];
      } else {
        mem -= charDelta[CHAR_DELTA_BEYOND];
      } /* if */
    } /* while */
    return NULL;
  } /* rsearch_strelem2 */



/**
 *  Copy 'source' character array to 'dest' as lower case characters.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @param source Character array to be copied and converted.
 *  @param length Length of the source character array.
 *  @param dest Destination character array for the lower case chars.
 */
void toLower (const strElemType *const source, memSizeType length,
    strElemType *const dest)

  {
    memSizeType pos;
    strElemType ch;

  /* toLower */
    for (pos = 0; pos < length; pos++) {
      ch = source[pos];
      switch (ch >> 8) {
        case 0:
          if (ch <= '\177') {
            ch = (strElemType) ((unsigned char)
                "\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17"
                "\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37"
                " !\"#$%&'()*+,-./0123456789:;<=>?"
                "@abcdefghijklmnopqrstuvwxyz[\\]^_"
                "`abcdefghijklmnopqrstuvwxyz{|}~\177"[ch]);
          } else if ("\0\0\0\0\0\0\0\0\376\377\377\007\0\0\0\0"
              "\0\0\0\0\0\0\0\0\377\377\177\177\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            ch += 32;
          } /* if */
          break;
        case 1:
          if ("UUUUUUU\252\252TUUUUU+"
              "\326\316\333\261\325\322\256\021\260\255\252JUU\326U"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            switch (ch) {
              case 0x0130: ch = 0x0069; break;
              case 0x0178: ch = 0x00ff; break;
              case 0x0181: ch = 0x0253; break;
              case 0x0186: ch = 0x0254; break;
              case 0x0189: ch = 0x0256; break;
              case 0x018a: ch = 0x0257; break;
              case 0x018e: ch = 0x01dd; break;
              case 0x018f: ch = 0x0259; break;
              case 0x0190: ch = 0x025b; break;
              case 0x0193: ch = 0x0260; break;
              case 0x0194: ch = 0x0263; break;
              case 0x0196: ch = 0x0269; break;
              case 0x0197: ch = 0x0268; break;
              case 0x019c: ch = 0x026f; break;
              case 0x019d: ch = 0x0272; break;
              case 0x019f: ch = 0x0275; break;
              case 0x01a6: ch = 0x0280; break;
              case 0x01a9: ch = 0x0283; break;
              case 0x01ae: ch = 0x0288; break;
              case 0x01b1: ch = 0x028a; break;
              case 0x01b2: ch = 0x028b; break;
              case 0x01b7: ch = 0x0292; break;
              case 0x01c4: ch += 2;     break;
              case 0x01c7: ch += 2;     break;
              case 0x01ca: ch += 2;     break;
              case 0x01f1: ch += 2;     break;
              case 0x01f2: ch = 0x01f3; break;
              case 0x01f6: ch = 0x0195; break;
              case 0x01f7: ch = 0x01bf; break;
              case 0x01f8: ch = 0x01f9; break;
              default: ch += 1;         break;
            } /* switch */
          } /* if */
          break;
        case 2:
          if ("UUUUUU\005lzU\0\0\0\0\0\0"
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            switch (ch) {
              case 0x0220: ch -= 130;   break;
              case 0x023a: ch = 0x2c65; break;
              case 0x023d: ch -= 163;   break;
              case 0x023e: ch = 0x2c66; break;
              case 0x0243: ch -= 195;   break;
              case 0x0244: ch += 69;    break;
              case 0x0245: ch += 71;    break;
              default:     ch += 1;     break;
            } /* switch */
          } /* if */
          break;
        case 3:
          if ("\0\0\0\0\0\0\0\0\0\0\0\0\0\0E\0"
              "@\327\376\377\373\017\0\0\0\200\0UUU\220\346"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            switch (ch) {
              case 0x0370: ch += 1;   break;
              case 0x0372: ch += 1;   break;
              case 0x0376: ch += 1;   break;
              case 0x0386: ch += 38;  break;
              case 0x0388: ch += 37;  break;
              case 0x0389: ch += 37;  break;
              case 0x038a: ch += 37;  break;
              case 0x038c: ch += 64;  break;
              case 0x038e: ch += 63;  break;
              case 0x038f: ch += 63;  break;
              case 0x03cf: ch += 8;   break;
              case 0x03f4: ch -= 60;  break;
              case 0x03f9: ch -= 7;   break;
              case 0x03fd: ch -= 130; break;
              case 0x03fe: ch -= 130; break;
              case 0x03ff: ch -= 130; break;
              default:
                if (ch <= 0x03ab) {
                  ch += 32;
                } else {
                  ch += 1;
                } /* if */
                break;
            } /* switch */
          } /* if */
          break;
        case 4:
          if ("\377\377\377\377\377\377\0\0\0\0\0\0UUUU"
              "\001TUUUUUU\253*UUUUUU"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x040f) {
              ch += 80;
            } else if (ch <= 0x042f) {
              ch += 32;
            } else if (ch == 0x04c0) {
              ch = 0x04cf;
            } else {
              ch += 1;
            } /* if */
          } /* if */
          break;
        case 5:
          if (ch <= 0x0526 && (ch & 1) == 0) {
            ch += 1;
          } else if (ch >= 0x0531 && ch <= 0x0556) {
            ch += 48;
          } /* if */
          break;
        case 16:
          if ("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
              "\0\0\0\0\377\377\377\377\277 \0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            ch += 0x1c60;
          } /* if */
          break;
        case 30:
          if ("UUUUUUUUUUUUUUUUUU\025@UUUUUUUUUUUU"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0x1e9e) {
              ch = 0x00df;
            } else {
              ch += 1;
            } /* if */
          } /* if */
          break;
        case 31:
          if ("\0\377\0?\0\377\0\377\0?\0\252\0\377\0\0"
              "\0\377\0\377\0\377\0\037\0\037\0\017\0\037\0\037"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch >= 0x1fba) {
              ch = (strElemType) ((unsigned char)
                  "\160\161\263\000\000\000\000\000\000\000"
                  "\000\000\000\000\162\163\164\165\303\000"
                  "\000\000\000\000\000\000\000\000\000\000"
                  "\320\321\166\167\000\000\000\000\000\000"
                  "\000\000\000\000\000\000\340\341\172\173"
                  "\345\000\000\000\000\000\000\000\000\000"
                  "\000\000\170\171\174\175\363"[ch - 0x1fba] + 0x1f00);
            } else {
              ch -= 8;
            } /* if */
          } /* if */
          break;
        case 33:
          if ("\0\0\0\0@\f\004\0\0\0\0\0\377\377\0\0"
              "\b\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            switch (ch) {
                case 0x2126: ch = 0x03c9; break;
                case 0x212a: ch = 0x006b; break;
                case 0x212b: ch = 0x00e5; break;
                case 0x2132: ch += 28;    break;
                case 0x2183: ch += 1;     break;
              default: ch += 16;          break;
            } /* switch */
          } /* if */
          break;
        case 36:
          if (ch >= 0x24b6 && ch <= 0x24cf) {
            ch += 26;
          } /* if */
          break;
        case 44:
          if ("\377\377\377\377\377\177\0\0\0\0\0\0\235\352%\300"
              "UUUUUUUUUUUU\005(\004\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x2c2e) {
              ch += 48;
            } else {
              switch (ch) {
                case 0x2c62: ch = 0x026b; break;
                case 0x2c63: ch = 0x1d7d; break;
                case 0x2c64: ch = 0x027d; break;
                case 0x2c6d: ch = 0x0251; break;
                case 0x2c6e: ch = 0x0271; break;
                case 0x2c6f: ch = 0x0250; break;
                case 0x2c70: ch = 0x0252; break;
                case 0x2c7e: ch = 0x023f; break;
                case 0x2c7f: ch = 0x0240; break;
                default: ch += 1; break;
              } /* switch */
            } /* if */
          } /* if */
          break;
        case 166:
          if ("\0\0\0\0\0\0\0\0UUUUU\025\0\0"
              "UUU\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            ch += 1;
          } /* if */
          break;
        case 167:
          if ("\0\0\0\0TUTUUUUUUU\0j"
              "U(\005\0U\005\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0xa77d) {
              ch = 0x1d79;
            } else if (ch == 0xa78d) {
              ch = 0x0265;
            } else if (ch == 0xa7aa) {
              ch = 0x0266;
            } else {
              ch += 1;
            } /* if */
          } /* if */
          break;
        case 255:
          if (ch >= 0xff21 && ch <= 0xff3a) {
            ch += 32;
          } /* if */
          break;
        case 260:
          if (ch >= 0x10400 && ch <= 0x10427) {
            ch += 40;
          } /* if */
          break;
        default:
          break;
      } /* switch */
      dest[pos] = ch;
    } /* for */
  } /* toLower */



static const strElemType toUpperTable2[] = {
    0x2c6f, 0x2c6d, 0x2c70, 0x0181, 0x0186,      0, 0x0189, 0x018a,      0, 0x018f,
         0, 0x0190,      0,      0,      0,      0, 0x0193,      0,      0, 0x0194,
         0, 0xa78d, 0xa7aa,      0, 0x0197, 0x0196,      0, 0x2c62,      0,      0,
         0, 0x019c,      0, 0x2c6e, 0x019d,      0,      0, 0x019f,      0,      0,
         0,      0,      0,      0,      0, 0x2c64,      0,      0, 0x01a6,      0,
         0, 0x01a9,      0,      0,      0,      0, 0x01ae, 0x0244, 0x01b1, 0x01b2,
    0x0245,      0,      0,      0,      0,      0, 0x01b7
  };


/**
 *  Copy 'source' character array to 'dest' as upper case characters.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @param source Character array to be copied and converted.
 *  @param length Length of the source character array.
 *  @param dest Destination character array for the upper case chars.
 */
void toUpper (const strElemType *const source, memSizeType length,
    strElemType *const dest)

  {
    memSizeType pos;
    strElemType ch;

  /* toUpper */
    for (pos = 0; pos < length; pos++) {
      ch = source[pos];
      switch (ch >> 8) {
        case 0:
          if (ch <= '\177') {
            ch = (strElemType) ((unsigned char)
                "\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17"
                "\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37"
                " !\"#$%&'()*+,-./0123456789:;<=>?"
                "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                "`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~\177"[ch]);
          } else if ("\0\0\0\0\0\0\0\0\0\0\0\0\376\377\377\007"
              "\0\0\0\0\0\0 \0\0\0\0\0\377\377\177\377"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0x00b5) {
              ch = 0x039c;
            } else if (ch == 0x00ff) {
              ch = 0x0178;
            } else {
              ch -= 32;
            } /* if */
          } /* if */
          break;
        case 1:
          if ("\252\252\252\252\252\252\252TU\251\252\252\252\252\252\324"
              ")\021$F*!Q\242`[U\265\252\252,\252"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0x0131) {
              ch = 0x0049;
            } else if (ch == 0x017f) {
              ch = 0x0053;
            } else if (ch >= 0x0180 && ch <= 0x019e) {
              ch = (strElemType) ((unsigned char)
                  "\303\0\0\002\0\004\0\0\007\0"
                  "\0\0\013\0\0\0\0\0\021\0"
                  "\0\166\0\0\0\030\275\0\0\0\240"[ch - 0x0180] + 0x0180);
            } else if (ch >= 0x01bf && ch <= 0x01cc) {
              ch = (strElemType) ((unsigned char)
                  "\367\0\0\0\0\0\304\304\0\307"
                  "\307\0\312\312"[ch - 0x01bf] + 0x0100);
            } else if (ch == 0x01dd) {
              ch = 0x018e;
            } else if (ch == 0x01f3) {
              ch = 0x01f1;
            } else {
              ch -= 1;
            } /* if */
          } /* if */
          break;
        case 2:
          if ("\252\252\252\252\250\252\n\220\205\252\337\ni\213& "
              "\t\037\004\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x024f) {
              if (ch == 0x023f) {
                ch = 0x2c7e;
              } else if (ch == 0x0240) {
                ch = 0x2c7f;
              } else {
                ch -= 1;
              } /* if */
            } else {
              ch = toUpperTable2[ch - 0x0250];
            } /* if */
          } /* if */
          break;
        case 3:
          if ("\0\0\0\0\0\0\0\0 \0\0\0\0\0\212"
              "8\0\0\0\0\0\360\376\377\377\177\343\252\252\252'\t"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x03af) {
              if (ch == 0x0345) {
                ch += 84;
              } else if (ch <= 0x0377) {
                ch -= 1;
              } else if (ch <= 0x037d) {
                ch += 130;
              } else if (ch == 0x03ac) {
                ch -= 38;
              } else {
                ch -= 37;
              } /* if */
            } else if (ch <= 0x03cb) {
              if (ch == 0x03c2) {
                ch -= 31;
              } else {
                ch -= 32;
              } /* if */
            } else if (ch <= 0x03d7) {
              ch = (strElemType) ((unsigned char)
                  "\214\216\217\0\222\230\0\0\0\246\240\317"[ch - 0x03cc] + 0x0300);
            } else if (ch <= 0x03ef) {
              ch -= 1;
            } else {
              ch = (strElemType) ((unsigned char)
                  "\232\241\371\0\0\225\0\0\367\0\0\372"[ch - 0x03f0] + 0x0300);
            } /* if */
          } /* if */
          break;
        case 4:
          if ("\0\0\0\0\0\0\377\377\377\377\377\377\252\252\252\252"
              "\002\250\252\252\252\252\252\252T\325\252\252\252\252\252\252"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x044f) {
              ch -= 32;
            } else if (ch <= 0x045f) {
              ch -= 80;
            } else if (ch == 0x04cf) {
              ch -= 15;
            } else {
              ch -= 1;
            } /* if */
          } /* if */
          break;
        case 5:
          if ("\252\252\252\252\252\0\0\0\0\0\0\0\376\377\377\377"
              "\177\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x0527) {
              ch -= 1;
            } else {
              ch -= 48;
            } /* if */
          } /* if */
          break;
        case 29:
          if (ch == 0x1d79) {
            ch = 0xa77d;
          } else if (ch == 0x1d7d) {
            ch = 0x2c63;
          } /* if */
          break;
        case 30:
          if ("\252\252\252\252\252\252\252\252\252\252\252\252\252\252\252\252"
              "\252\252*\b\252\252\252\252\252\252\252\252\252\252\252\252"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0x1e9b) {
              ch = 0x1e60;
            } else {
              ch -= 1;
            } /* if */
          } /* if */
          break;
        case 31:
          if ("\377\0?\0\377\0\377\0?\0\252\0\377\0\377?"
              "\377\0\377\0\377\0\013@\b\0\003\0#\0\b\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch >= 0x1f70 && ch <= 0x1f7d) {
              ch = (strElemType) ((unsigned char)
                  "\272\273\310\311\312\313\332\333\370\371\352\353\372\373"[ch - 0x1f70] + 0x1f00);
            } else if (ch == 0x1fb3 || ch == 0x1fc3 || ch == 0x1ff3) {
              ch += 9;
            } else if (ch == 0x1fe5) {
              ch += 7;
            } else if (ch == 0x1fbe) {
              ch = 0x0399;
            } else {
              ch += 8;
            } /* if */
          } /* if */
          break;
        case 33:
          if ("\0\0\0\0\0\0\0\0\0@\0\0\0\0\377\377"
              "\020\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch == 0x214e) {
              ch -= 28;
            } else if (ch == 0x2184) {
              ch -= 1;
            } else {
              ch -= 16;
            } /* if */
          } /* if */
          break;
        case 36:
          if (ch >= 0x24d0 && ch <= 0x24e9) {
            ch -= 26;
          } /* if */
          break;
        case 44:
          if ("\0\0\0\0\0\0\377\377\377\377\377\177b\025H\0"
              "\252\252\252\252\252\252\252\252\252\252\252\252\nP\b\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            if (ch <= 0x2c5e) {
              ch -= 48;
            } else if (ch == 0x2c65) {
              ch = 0x023a;
            } else if (ch == 0x2c66) {
              ch = 0x023e;
            } else {
              ch -= 1;
            } /* if */
          } /* if */
          break;
        case 45:
          if (ch >= 0x2d00 && ch <= 0x2d2d) {
            ch -= 0x1c60;
          } /* if */
          break;
        case 166:
          if ("\0\0\0\0\0\0\0\0\252\252\252\252\252*\0\0"
              "\252\252\252\0\0\0\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            ch -= 1;
          } /* if */
          break;
        case 167:
          if ("\0\0\0\0\250\252\250\252\252\252\252\252\252\252\0\224"
              "\252\020\n\0\252\002\0\0\0\0\0\0\0\0\0\0"[ch >> 3 & 31] &
              1 << (ch & 7)) {
            ch -= 1;
          } /* if */
          break;
        case 255:
          if (ch >= 0xff41 && ch <= 0xff5a) {
            ch -= 32;
          } /* if */
          break;
        case 260:
          if (ch >= 0x10428 && ch <= 0x1044f) {
            ch -= 40;
          } /* if */
          break;
        default:
          break;
      } /* switch */
      dest[pos] = ch;
    } /* for */
  } /* toUpper */



static rtlArrayType addCopiedStriToRtlArray (const strElemType *const stri_elems,
    const memSizeType length, rtlArrayType work_array, intType used_max_position)

  {
    striType new_stri;
    rtlArrayType resized_work_array;

  /* addCopiedStriToRtlArray */
    if (likely(ALLOC_STRI_SIZE_OK(new_stri, length))) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems, length * sizeof(strElemType));
      if (used_max_position >= work_array->max_position) {
        if (unlikely(work_array->max_position > MAX_RTL_ARR_INDEX - ARRAY_SIZE_DELTA ||
            (resized_work_array = REALLOC_RTL_ARRAY(work_array,
                (uintType) work_array->max_position,
                (uintType) work_array->max_position + ARRAY_SIZE_DELTA)) == NULL)) {
          FREE_STRI(new_stri, new_stri->size);
          freeRtlStriArray(work_array, used_max_position);
          work_array = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_RTL_ARRAY((uintType) work_array->max_position,
                           (uintType) work_array->max_position + ARRAY_SIZE_DELTA);
          work_array->max_position += ARRAY_SIZE_DELTA;
          work_array->arr[used_max_position].value.striValue = new_stri;
        } /* if */
      } else {
        work_array->arr[used_max_position].value.striValue = new_stri;
      } /* if */
    } else {
      freeRtlStriArray(work_array, used_max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* addCopiedStriToRtlArray */



static inline rtlArrayType completeRtlStriArray (rtlArrayType work_array,
    intType used_max_position)

  {
    rtlArrayType resized_work_array;

  /* completeRtlStriArray */
    if (likely(work_array != NULL)) {
      resized_work_array = REALLOC_RTL_ARRAY(work_array,
          (uintType) work_array->max_position, (uintType) used_max_position);
      if (unlikely(resized_work_array == NULL)) {
        freeRtlStriArray(work_array, used_max_position);
        work_array = NULL;
      } else {
        work_array = resized_work_array;
        COUNT3_RTL_ARRAY((uintType) work_array->max_position,
                         (uintType) used_max_position);
        work_array->max_position = used_max_position;
      } /* if */
    } /* if */
    return work_array;
  } /* completeRtlStriArray */



/**
 *  Concatenate a relative path to an absolute path.
 *  In the relative path the special directories "." and ".." are
 *  interpreted according to their conventional meaning. A ".." which
 *  would go above the file system root ("/") is ignored.
 *  @param absolutePath Absolute path in the standard path
 *         representation.
 *  @param relativePath Relative path in the standard path
 *         representation.
 *  @return the concatenated absolute path in the standard path
 *          representation.
 */
striType concatPath (const const_striType absolutePath,
    const const_striType relativePath)

  {
    memSizeType abs_path_length;
    memSizeType estimated_result_size;
    strElemType *abs_path_end;
    const strElemType *rel_path_pos;
    const strElemType *rel_path_beyond;
    memSizeType result_size;
    striType resized_result;
    striType result;

  /* concatPath */
    logFunction(printf("concatPath(\"%s\", ", striAsUnquotedCStri(absolutePath));
                printf("\"%s\")\n", striAsUnquotedCStri(relativePath)););
    /* absolutePath->mem[0] is always '/'. */
    if (absolutePath->size == 1) {
      abs_path_length = 0;
    } else {
      abs_path_length = absolutePath->size;
    } /* if */
    if (unlikely(abs_path_length > MAX_STRI_LEN - relativePath->size - 2)) {
      result = NULL;
    } else {
      /* There is one slash (/) between the two paths. Temporarily    */
      /* there is also a slash at the end of the intermediate result. */
      estimated_result_size = abs_path_length + relativePath->size + 2;
      if (ALLOC_STRI_SIZE_OK(result, estimated_result_size)) {
        memcpy(result->mem, absolutePath->mem, abs_path_length * sizeof(strElemType));
        result->mem[abs_path_length] = '/';
        abs_path_end = &result->mem[abs_path_length];
        rel_path_pos = relativePath->mem;
        rel_path_beyond = &relativePath->mem[relativePath->size];
        while (rel_path_pos < rel_path_beyond) {
          if (&rel_path_pos[1] < rel_path_beyond &&
              rel_path_pos[0] == '.' && rel_path_pos[1] == '.' &&
              (&rel_path_pos[2] >= rel_path_beyond || rel_path_pos[2] == '/')) {
            rel_path_pos += 2;
            if (abs_path_end > result->mem) {
              do {
                abs_path_end--;
              } while (*abs_path_end != '/');
            } /* if */
          } else if (&rel_path_pos[0] < rel_path_beyond &&
                     rel_path_pos[0] == '.' &&
                     (&rel_path_pos[1] >= rel_path_beyond || rel_path_pos[1] == '/')) {
            rel_path_pos++;
          } else if (*rel_path_pos == '/') {
            rel_path_pos++;
          } else {
            do {
              abs_path_end++;
              *abs_path_end = *rel_path_pos;
              rel_path_pos++;
            } while (&rel_path_pos[0] < rel_path_beyond && rel_path_pos[0] != '/');
            abs_path_end++;
            /* The line below adds a temporary slash (/) to the end   */
            /* of the intermediate result. Therefore + 2 is used to   */
            /* compute the estimated_result_size.                     */
            *abs_path_end = '/';
          } /* if */
        } /* while */
        if (abs_path_end == result->mem) {
          result->mem[0] = '/';
          result_size = 1;
        } else {
          result_size = (memSizeType) (abs_path_end - result->mem);
        } /* if */
        REALLOC_STRI_SIZE_SMALLER(resized_result, result, estimated_result_size, result_size);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, estimated_result_size);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(estimated_result_size, result_size);
          result->size = result_size;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("concatPath --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* concatPath */



#if ALLOW_STRITYPE_SLICES
/**
 *  Append the string 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strAppend (striType *const destination, const_striType extension)

  {
    memSizeType new_size;
    striType stri_dest;
    striType new_stri;
    memSizeType extension_size;
    const strElemType *extension_mem;
    const strElemType *extension_origin;

  /* strAppend */
    logFunction(printf("strAppend(\"%s\", ", striAsUnquotedCStri(*destination));
                printf("\"%s\")", striAsUnquotedCStri(extension));
                fflush(stdout););
    stri_dest = *destination;
    extension_size = extension->size;
    extension_mem = extension->mem;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension_size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension_size;
#if WITH_STRI_CAPACITY
      if (new_size > stri_dest->capacity) {
        if (SLICE_OVERLAPPING(extension, stri_dest)) {
          extension_origin = stri_dest->mem;
        } else {
          extension_origin = NULL;
        } /* if */
        new_stri = growStri(stri_dest, new_size);
        if (unlikely(new_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (unlikely(extension_origin != NULL)) {
            /* It is possible that 'extension' is identical to    */
            /* 'stri_dest' or a slice of it. This can be checked  */
            /* with the origin. In this case 'extension_mem' must */
            /* be corrected after realloc() enlarged 'stri_dest'. */
            extension_mem = &new_stri->mem[extension_mem - extension_origin];
            /* Correcting extension->mem is not necessary, since  */
            /* a slice will not be used afterwards. In case       */
            /* 'extension is identical to 'stri_dest' changing    */
            /* extension->mem is dangerous since 'extension'      */
            /* could have been released.                          */
          } /* if */
          stri_dest = new_stri;
          *destination = stri_dest;
        } /* if */
      } /* if */
      COUNT_GROW_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], extension_mem,
             extension_size * sizeof(strElemType));
      stri_dest->size = new_size;
#else
      if (SLICE_OVERLAPPING(extension, stri_dest)) {
        extension_origin = stri_dest->mem;
      } else {
        extension_origin = NULL;
      } /* if */
      GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
      if (unlikely(new_stri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (unlikely(extension_origin != NULL)) {
          /* It is possible that 'extension' is identical to    */
          /* 'stri_dest' or a slice of it. This can be checked  */
          /* with the origin. In this case 'extension_mem' must */
          /* be corrected after realloc() enlarged 'stri_dest'. */
          extension_mem = &new_stri->mem[extension_mem - extension_origin];
          /* Correcting extension->mem is not necessary, since  */
          /* a slice will not be used afterwards. In case       */
          /* 'extension is identical to 'stri_dest' changing    */
          /* extension->mem is dangerous since 'extension'      */
          /* could have been released.                          */
        } /* if */
        COUNT_GROW_STRI(new_stri->size, new_size);
        memcpy(&new_stri->mem[new_stri->size], extension_mem,
               extension_size * sizeof(strElemType));
        new_stri->size = new_size;
        *destination = new_stri;
      } /* if */
#endif
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strAppend */



/**
 *  Append an arbitrary number of strings.
 *  strAppendN is used by the compiler to optimize appending
 *  two or more strings.
 *  @param arraySize Number of strings in extensionArray (>= 2).
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strAppendN (striType *const destination,
    const const_striType extensionArray[], memSizeType arraySize)

  {
    striType stri_dest;
    memSizeType size_limit;
    memSizeType pos;
    memSizeType new_size;
    const strElemType *old_dest_origin;
    const strElemType *old_dest_beyond;
    striType new_stri;
    strElemType *dest;
    memSizeType elem_size;
    const strElemType *extension_mem;

  /* strAppendN */
    logFunction(printf("strAppendN(\"%s\", ",
                       striAsUnquotedCStri(*destination));
                for (pos = 0; pos < arraySize; pos++) {
                  printf("\"%s\", ",
                         striAsUnquotedCStri(extensionArray[pos]));
                } /* for */
                printf(FMT_U_MEM ")", arraySize);
                fflush(stdout););
    stri_dest = *destination;
    size_limit = MAX_STRI_LEN - stri_dest->size;
    pos = arraySize;
    do {
      pos--;
      if (unlikely(extensionArray[pos]->size > size_limit)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        size_limit -= extensionArray[pos]->size;
      } /* if */
    } while (pos != 0);
    new_size = MAX_STRI_LEN - size_limit;
#if WITH_STRI_CAPACITY
    if (new_size > stri_dest->capacity) {
      if (new_size <= MAX_STRI_LEN_IN_FREELIST) {
        if (unlikely(!ALLOC_STRI_SIZE_OK(new_stri, new_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          new_stri->size = new_size;
          dest = new_stri->mem;
          memcpy(dest, stri_dest->mem, stri_dest->size * sizeof(strElemType));
          dest += stri_dest->size;
          for (pos = 0; pos < arraySize; pos++) {
            elem_size = extensionArray[pos]->size;
            memcpy(dest, extensionArray[pos]->mem, elem_size * sizeof(strElemType));
            dest += elem_size;
          } /* for */
          FREE_STRI(stri_dest, stri_dest->size);
          *destination = new_stri;
        } /* if */
      } else {
        old_dest_origin = GET_DESTINATION_ORIGIN(stri_dest);
        old_dest_beyond = GET_DESTINATION_BEYOND(stri_dest);
        new_stri = growStri(stri_dest, new_size);
        if (unlikely(new_stri == NULL)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT_GROW_STRI(new_stri->size, new_size);
          *destination = new_stri;
          dest = &new_stri->mem[new_stri->size];
          for (pos = 0; pos < arraySize; pos++) {
            if (unlikely(stri_dest == extensionArray[pos])) {
              /* The extension (extensionArray[pos]) is identical   */
              /* to the 'destination' (it refers to the memory area */
              /* of it). The resizing of the 'destination' might    */
              /* have moved 'new_stri' to a new memory area.        */
              /* Therefore 'extension_mem' must be corrected after  */
              /* realloc()  enlarged 'new_stri'.                    */
              elem_size = new_stri->size;
              extension_mem = new_stri->mem;
              /* Correcting extensionArray[pos]->mem is not needed, */
              /* since the slice will not be used afterwards.       */
              /* Changing extensionArray[pos]->mem is dangerous,    */
              /* since the memory could have been released.         */
            } else if (unlikely(SLICE_OVERLAPPING2(extensionArray[pos],
                                                   old_dest_origin,
                                                   old_dest_beyond))) {
              /* The extension (extensionArray[pos]) is a slice of  */
              /* the 'destination' (it refers to the memory area of */
              /* it). The resizing of the 'destination' might have  */
              /* moved 'new_stri' to a new memory area. Therefore   */
              /* 'extension_mem' must be corrected after realloc()  */
              /* enlarged 'new_stri'.                               */
              elem_size = extensionArray[pos]->size;
              extension_mem =
                  &new_stri->mem[extensionArray[pos]->mem - old_dest_origin];
              /* Correcting extensionArray[pos]->mem is not needed, */
              /* since the slice will not be used afterwards.       */
            } else {
              elem_size = extensionArray[pos]->size;
              extension_mem = extensionArray[pos]->mem;
            } /* if */
            memcpy(dest, extension_mem, elem_size * sizeof(strElemType));
            dest += elem_size;
          } /* for */
          new_stri->size = new_size;
        } /* if */
      } /* if */
    } else {
      COUNT_GROW2_STRI(stri_dest->size, new_size);
      dest = &stri_dest->mem[stri_dest->size];
      for (pos = 0; pos < arraySize; pos++) {
        elem_size = extensionArray[pos]->size;
        memcpy(dest, extensionArray[pos]->mem,
               elem_size * sizeof(strElemType));
        dest += elem_size;
      } /* for */
      stri_dest->size = new_size;
    } /* if */
#else
    old_dest_origin = GET_DESTINATION_ORIGIN(stri_dest);
    old_dest_beyond = GET_DESTINATION_BEYOND(stri_dest);
    GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
    if (unlikely(new_stri == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_GROW_STRI(new_stri->size, new_size);
      *destination = new_stri;
      dest = &new_stri->mem[new_stri->size];
      for (pos = 0; pos < arraySize; pos++) {
        elem_size = extensionArray[pos]->size;
        if (unlikely(stri_dest == extensionArray[pos])) {
          /* The extension (extensionArray[pos]) is identical   */
          /* to the 'destination' (it refers to the memory area */
          /* of it). The resizing of the 'destination' might    */
          /* have moved 'new_stri' to a new memory area.        */
          /* Therefore 'extension_mem' must be corrected after  */
          /* realloc()  enlarged 'new_stri'.                    */
          elem_size = new_stri->size;
          extension_mem = new_stri->mem;
          /* Correcting extensionArray[pos]->mem is not needed, */
          /* since then slice will not be used afterwards.      */
          /* Changing extensionArray[pos]->mem is dangerous,    */
          /* since the memory could have been released.         */
        } else if (unlikely(SLICE_OVERLAPPING2(extensionArray[pos],
                                               old_dest_origin,
                                               old_dest_beyond))) {
          /* The extension (extensionArray[pos]) is a slice of  */
          /* the 'destination' (it refers to the memory area of */
          /* it). The resizing of the 'destination' might have  */
          /* moved 'new_stri' to a new memory area. Therefore   */
          /* 'extension_mem' must be corrected after realloc()  */
          /* enlarged 'new_stri'.                               */
          elem_size = extensionArray[pos]->size;
          extension_mem =
              &new_stri->mem[extensionArray[pos]->mem - old_dest_origin];
          /* Correcting extensionArray[pos]->mem is not needed, */
          /* since the slice will not be used afterwards.       */
        } else {
          elem_size = extensionArray[pos]->size;
          extension_mem = extensionArray[pos]->mem;
        } /* if */
        memcpy(dest, extension_mem, elem_size * sizeof(strElemType));
        dest += elem_size;
      } /* for */
      new_stri->size = new_size;
    } /* if */
#endif
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strAppendN */

#else



/**
 *  Append the string 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strAppend (striType *const destination, const_striType extension)

  {
    memSizeType new_size;
    striType stri_dest;
    striType new_stri;

  /* strAppend */
    logFunction(printf("strAppend(\"%s\", ", striAsUnquotedCStri(*destination));
                printf("\"%s\")", striAsUnquotedCStri(extension));
                fflush(stdout););
    stri_dest = *destination;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension->size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension->size;
#if WITH_STRI_CAPACITY
      if (new_size > stri_dest->capacity) {
        new_stri = growStri(stri_dest, new_size);
        if (unlikely(new_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (unlikely(stri_dest == extension)) {
            /* It is possible that stri_dest == extension holds. */
            /* In this case 'extension' must be corrected        */
            /* after realloc() enlarged 'stri_dest'.             */
            extension = new_stri;
          } /* if */
          stri_dest = new_stri;
          *destination = stri_dest;
        } /* if */
      } /* if */
      COUNT_GROW_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
             extension->size * sizeof(strElemType));
      stri_dest->size = new_size;
#else
      GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
      if (unlikely(new_stri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (unlikely(stri_dest == extension)) {
          /* It is possible that stri_dest == extension holds. */
          /* In this case 'extension' must be corrected        */
          /* after realloc() enlarged 'stri_dest'.             */
          extension = new_stri;
        } /* if */
        COUNT_GROW_STRI(new_stri->size, new_size);
        memcpy(&new_stri->mem[new_stri->size], extension->mem,
               extension->size * sizeof(strElemType));
        new_stri->size = new_size;
        *destination = new_stri;
      } /* if */
#endif
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strAppend */



/**
 *  Append an arbitrary number of strings.
 *  strAppendN is used by the compiler to optimize appending
 *  two or more strings.
 *  @param arraySize Number of strings in extensionArray (>= 2).
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strAppendN (striType *const destination,
    const const_striType extensionArray[], memSizeType arraySize)

  {
    striType stri_dest;
    memSizeType size_limit;
    memSizeType pos;
    memSizeType new_size;
    striType new_stri;
    strElemType *dest;
    memSizeType elem_size;
    const strElemType *extension_mem;

  /* strAppendN */
    logFunction(printf("strAppendN(\"%s\", ",
                       striAsUnquotedCStri(*destination));
                for (pos = 0; pos < arraySize; pos++) {
                  printf("\"%s\", ",
                         striAsUnquotedCStri(extensionArray[pos]));
                } /* for */
                printf(FMT_U_MEM ")", arraySize);
                fflush(stdout););
    stri_dest = *destination;
    size_limit = MAX_STRI_LEN - stri_dest->size;
    pos = arraySize;
    do {
      pos--;
      if (unlikely(extensionArray[pos]->size > size_limit)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        size_limit -= extensionArray[pos]->size;
      } /* if */
    } while (pos != 0);
    new_size = MAX_STRI_LEN - size_limit;
#if WITH_STRI_CAPACITY
    if (new_size > stri_dest->capacity) {
      new_stri = growStri(stri_dest, new_size);
      if (unlikely(new_stri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_GROW_STRI(new_stri->size, new_size);
        *destination = new_stri;
        dest = &new_stri->mem[new_stri->size];
        for (pos = 0; pos < arraySize; pos++) {
          if (unlikely(stri_dest == extensionArray[pos])) {
            /* It is possible that stri_dest == extension holds. */
            /* In this case 'extension' must be corrected        */
            /* after realloc() enlarged 'stri_dest'.             */
            elem_size = new_stri->size;
            extension_mem = new_stri->mem;
          } else {
            elem_size = extensionArray[pos]->size;
            extension_mem = extensionArray[pos]->mem;
          } /* if */
          memcpy(dest, extension_mem, elem_size * sizeof(strElemType));
          dest += elem_size;
        } /* for */
        new_stri->size = new_size;
      } /* if */
    } else {
      COUNT_GROW2_STRI(stri_dest->size, new_size);
      dest = &stri_dest->mem[stri_dest->size];
      for (pos = 0; pos < arraySize; pos++) {
        elem_size = extensionArray[pos]->size;
        memcpy(dest, extensionArray[pos]->mem,
               elem_size * sizeof(strElemType));
        dest += elem_size;
      } /* for */
      stri_dest->size = new_size;
    } /* if */
#else
    GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
    if (unlikely(new_stri == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_GROW_STRI(new_stri->size, new_size);
      *destination = new_stri;
      dest = &new_stri->mem[new_stri->size];
      for (pos = 0; pos < arraySize; pos++) {
        if (unlikely(stri_dest == extensionArray[pos])) {
          /* It is possible that stri_dest == extension holds. */
          /* In this case 'extension' must be corrected        */
          /* after realloc() enlarged 'stri_dest'.             */
          elem_size = new_stri->size;
          extension_mem = new_stri->mem;
        } else {
          elem_size = extensionArray[pos]->size;
          extension_mem = extensionArray[pos]->mem;
        } /* if */
        memcpy(dest, extension_mem, elem_size * sizeof(strElemType));
        dest += elem_size;
      } /* for */
      new_stri->size = new_size;
    } /* if */
#endif
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strAppendN */

#endif



/**
 *  Append the string 'extension' to 'destination'.
 *  StrAppendTemp is used by the compiler if 'extension' is temporary
 *  value that can be reused.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strAppendTemp (striType *const destination, const striType extension)

  {
    memSizeType new_size;
    striType stri_dest;

  /* strAppendTemp */
    logFunction(printf("strAppendTemp(\"%s\", ", striAsUnquotedCStri(*destination));
                printf("\"%s\")", striAsUnquotedCStri(extension));
                fflush(stdout););
    stri_dest = *destination;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension->size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension->size;
#if WITH_STRI_CAPACITY
      if (new_size <= stri_dest->capacity) {
        COUNT_GROW2_STRI(stri_dest->size, new_size);
        memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
               extension->size * sizeof(strElemType));
        stri_dest->size = new_size;
        FREE_STRI(extension, extension->size);
      } else if (new_size <= extension->capacity) {
        if (stri_dest->size != 0) {
          COUNT_GROW2_STRI(extension->size, new_size);
          memmove(&extension->mem[stri_dest->size], extension->mem,
                  extension->size * sizeof(strElemType));
          memcpy(extension->mem, stri_dest->mem,
                 stri_dest->size * sizeof(strElemType));
          extension->size = new_size;
        } /* if */
        *destination = extension;
        FREE_STRI(stri_dest, stri_dest->size);
      } else {
        stri_dest = growStri(stri_dest, new_size);
        if (unlikely(stri_dest == NULL)) {
          FREE_STRI(extension, extension->size);
          raise_error(MEMORY_ERROR);
        } else {
          *destination = stri_dest;
          COUNT_GROW_STRI(stri_dest->size, new_size);
          memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
                 extension->size * sizeof(strElemType));
          stri_dest->size = new_size;
          FREE_STRI(extension, extension->size);
        } /* if */
      } /* if */
#else
      GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
      if (unlikely(stri_dest == NULL)) {
        FREE_STRI(extension, extension->size);
        raise_error(MEMORY_ERROR);
      } else {
        *destination = stri_dest;
        COUNT_GROW_STRI(stri_dest->size, new_size);
        memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
               extension->size * sizeof(strElemType));
        stri_dest->size = new_size;
        FREE_STRI(extension, extension->size);
      } /* if */
#endif
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strAppendTemp */



/**
 *  Replace all occurrences of char 'searched' in 'mainStri' by 'replacement'.
 *  @return the result of the replacement.
 */
striType strChChRepl (const const_striType mainStri,
    const charType searched, const charType replacement)

  {
    memSizeType main_size;
    memSizeType pos;
    strElemType ch;
    striType result;

  /* strChChRepl */
    main_size = mainStri->size;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, main_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = main_size;
      for (pos = 0; pos < main_size; pos++) {
        ch = mainStri->mem[pos];
        if (ch == searched) {
          ch = replacement;
        } /* if */
        result->mem[pos] = ch;
      } /* for */
    } /* if */
    return result;
  } /* strChRepl */



#ifdef OUT_OF_ORDER
rtlArrayType strChEscSplit (const const_striType mainStri, const charType delimiter,
    const charType escape)

  {
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *curr_pos;
    const strElemType *found_pos;
    striType curr_stri;
    const strElemType *stri_pos;
    memSizeType pos;
    rtlArrayType resized_result_array;
    rtlArrayType result_array;

  /* strChEscSplit */
    if (unlikely(delimiter == escape)) {
      raise_error(RANGE_ERROR);
    } else {
      if (ALLOC_RTL_ARRAY(result_array, INITIAL_ARRAY_SIZE)) {
        result_array->min_position = 1;
        result_array->max_position = INITIAL_ARRAY_SIZE;
        used_max_position = 0;
        search_start = mainStri->mem;
        search_end = &mainStri->mem[mainStri->size];
        old_pos = search_start;
        curr_pos = search_start;
        while (curr_pos != search_end && result_array != NULL) {
          while (curr_pos != search_end && *curr_pos != delimiter) {
            while (curr_pos != search_end && *curr_pos != delimiter && *curr_pos != escape) {
              curr_pos++;
            } /* while */
            memcpy(stri_pos, old_pos, (memSizeType) (curr_pos - old_pos));
            stri_pos += curr_pos - old_pos;
            if (curr_pos != search_end && *curr_pos == escape) {
              curr_pos++;
              if (curr_pos != search_end) {
                *stri_pos = *curr_pos;
                stri_pos++;
              } /* if */
            } /* if */
          } /* while */
          result_array = addCopiedStriToRtlArray(search_start,
              (memSizeType) (found_pos - search_start), result_array,
              used_max_position);
          used_max_position++;
          search_start = found_pos + 1;

      if (result_array != NULL) {
        result_array = addCopiedStriToRtlArray(search_start,
            (memSizeType) (search_end - search_start), result_array,
            used_max_position);
        used_max_position++;
        result_array = completeRtlStriArray(result_array, used_max_position);
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strChEscSplit */
#endif



/**
 *  Search char 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or after 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' <= 0 holds.
 */
intType strChIPos (const const_striType mainStri, const charType searched,
    const intType fromIndex)

  {
    uintType startIndex;
    const strElemType *main_mem;
    const strElemType *found_pos;

  /* strChIPos */
    logFunction(printf("strChIPos(\"%s\", '\\" FMT_U32 ";', " FMT_D ")\n",
                       striAsUnquotedCStri(mainStri), searched, fromIndex););
    startIndex = ((uintType) fromIndex) - 1;
    if (startIndex < mainStri->size) {
      main_mem = mainStri->mem;
      found_pos = memchr_strelem(&main_mem[startIndex], searched,
          mainStri->size - (memSizeType) startIndex);
      if (found_pos != NULL) {
        return ((intType) (found_pos - main_mem)) + 1;
      } /* if */
    } else if (unlikely(fromIndex <= 0)) {
      logError(printf("strChIPos(\"%s\", '\\" FMT_U32 ";', " FMT_D "): "
                      "fromIndex <= 0.\n",
                      striAsUnquotedCStri(mainStri), searched, fromIndex););
      raise_error(RANGE_ERROR);
    } /* if */
    return 0;
  } /* strChIPos */



/**
 *  String multiplication of the character 'ch'.
 *  The character 'ch' is concatenated to itself such that in total
 *  'factor' characters are concatenated.
 *  @return the result of the string multiplication.
 *  @exception RANGE_ERROR If the factor is negative.
 */
striType strChMult (const charType ch, const intType factor)

  {
    striType result;

  /* strChMult */
    logFunction(printf("strChMult('\\" FMT_U32 ";', " FMT_D ")\n",
                       ch, factor););
    if (unlikely(factor < 0)) {
      logError(printf("strChMult('\\" FMT_U32 ";', " FMT_D "): "
                      "Negative factor.\n",
                      ch, factor););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely((uintType) factor > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) factor))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) factor;
        memset_to_strelem(result->mem, ch, (memSizeType) factor);
      } /* if */
    } /* if */
    return result;
  } /* strChMult */



/**
 *  Determine leftmost position of char 'searched' in 'mainStri'.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
intType strChPos (const const_striType mainStri, const charType searched)

  {
    const strElemType *main_mem;
    const strElemType *found_pos;

  /* strChPos */
    logFunction(printf("strChPos(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(mainStri), searched););
    if (mainStri->size >= 1) {
      main_mem = mainStri->mem;
      found_pos = memchr_strelem(main_mem, searched, mainStri->size);
      if (found_pos != NULL) {
        return ((intType) (found_pos - main_mem)) + 1;
      } /* if */
    } /* if */
    return 0;
  } /* strChPos */



/**
 *  Replace all occurrences of char 'searched' in 'mainStri' by 'replacement'.
 *  @return the result of the replacement.
 */
striType strChRepl (const const_striType mainStri,
    const charType searched, const const_striType replacement)

  {
    memSizeType main_size;
    memSizeType guessed_result_size;
    memSizeType result_size;
    const strElemType *main_mem;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *copy_start;
    strElemType *result_end;
    striType resized_result;
    striType result;

  /* strChRepl */
    main_size = mainStri->size;
    /* printf("main_size=" FMT_U_MEM ", replacement->size=" FMT_U_MEM "\n",
        main_size, replacement->size); */
    if (replacement->size > 1) {
      if (unlikely(main_size > MAX_STRI_LEN / replacement->size)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        guessed_result_size = main_size * replacement->size;
      } /* if */
    } else {
      guessed_result_size = main_size;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, guessed_result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      copy_start = mainStri->mem;
      result_end = result->mem;
      if (main_size != 0) {
        main_mem = mainStri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size];
        while (search_start < search_end &&
          (search_start = memchr_strelem(search_start, searched,
              (memSizeType) (search_end - search_start))) != NULL) {
          memcpy(result_end, copy_start,
                 (memSizeType) (search_start - copy_start) * sizeof(strElemType));
          result_end += search_start - copy_start;
          memcpy(result_end, replacement->mem,
                 replacement->size * sizeof(strElemType));
          result_end += replacement->size;
          search_start++;
          copy_start = search_start;
        } /* while */
      } /* if */
      memcpy(result_end, copy_start,
             (memSizeType) (&mainStri->mem[main_size] - copy_start) * sizeof(strElemType));
      result_end += &mainStri->mem[main_size] - copy_start;
      result_size = (memSizeType) (result_end - result->mem);
      /* printf("result=%lu, guessed_result_size=%ld, result_size=%ld\n",
         result, guessed_result_size, result_size); */
      REALLOC_STRI_SIZE_SMALLER(resized_result, result, guessed_result_size, result_size);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, guessed_result_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(guessed_result_size, result_size);
        result->size = result_size;
      } /* if */
    } /* if */
    return result;
  } /* strChRepl */



/**
 *  Split 'mainStri' around matches of 'delimiter' into an array of strings.
 *  The array returned by strSplit() contains each substring of
 *  'mainStri' that is terminated by another substring that is equal
 *  to the 'delimiter' or is terminated by the end of 'mainStri'.
 *  The substrings in the array are in the order in which they occur in
 *  'mainStri'. If 'delimiter' does not match any part of 'mainStri'
 *  then the resulting array has just one element, namely 'mainStri'.
 *   split("", ':')       returns  []("")
 *   split("x", ':')      returns  []("x")
 *   split(":", ':')      returns  []("", "")
 *   split("x:", ':')     returns  []("x", "")
 *   split(":x", ':')     returns  []("", "x")
 *   split("15:30", ':')  returns  []("15", "30")
 *  @return the array of strings computed by splitting 'mainStri' around
 *          matches of the given 'delimiter'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArrayType strChSplit (const const_striType mainStri, const charType delimiter)

  {
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *found_pos;
    rtlArrayType result_array;

  /* strChSplit */
    logFunction(printf("strChSplit(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(mainStri), delimiter););
    if (likely(ALLOC_RTL_ARRAY(result_array, INITIAL_ARRAY_SIZE))) {
      result_array->min_position = 1;
      result_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      search_start = mainStri->mem;
      search_end = &mainStri->mem[mainStri->size];
      while ((found_pos = memchr_strelem(search_start, delimiter,
          (memSizeType) (search_end - search_start))) != NULL &&
          result_array != NULL) {
        result_array = addCopiedStriToRtlArray(search_start,
            (memSizeType) (found_pos - search_start), result_array,
            used_max_position);
        used_max_position++;
        search_start = found_pos + 1;
      } /* while */
      if (likely(result_array != NULL)) {
        result_array = addCopiedStriToRtlArray(search_start,
            (memSizeType) (search_end - search_start), result_array,
            used_max_position);
        used_max_position++;
        result_array = completeRtlStriArray(result_array, used_max_position);
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("strChSplit -->\n"););
    return result_array;
  } /* strChSplit */



striType strCLit (const const_striType stri)

  {
    /* A string literal starts and ends with double quotes ("): */
    const memSizeType numOfQuotes = 2;
    /* Maximum escape sequence length in C string literal: */
    const memSizeType escSequenceMax = STRLEN("\\255");
    register strElemType character;
    register memSizeType position;
    memSizeType striSize;
    memSizeType pos;
    striType resized_literal;
    striType literal;

  /* strCLit */
    logFunction(printf("strCLit(\"%s\")\n", striAsUnquotedCStri(stri)););
    striSize = stri->size;
    if (unlikely(striSize > (MAX_STRI_LEN - numOfQuotes) / escSequenceMax ||
                 !ALLOC_STRI_SIZE_OK(literal, escSequenceMax * striSize + numOfQuotes))) {
      raise_error(MEMORY_ERROR);
      literal = NULL;
    } else {
      literal->mem[0] = (strElemType) '"';
      pos = 1;
      for (position = 0; position < striSize; position++) {
        character = stri->mem[position];
        /* The following comparisons use int literals like 255      */
        /* instead of char literals like '\377'. If char literals   */
        /* are signed (this is C implementation dependent) the      */
        /* integral promotion (conversion to int) triggers a sign   */
        /* extension. In this case the sign extension of '\377'     */
        /* leads to the int value -1 instead of the desired 255.    */
        if (character < 127) {
          if (character < ' ') {
            literal->mem[pos] = (strElemType) '\\';
            if (cstri_escape_sequence[character][1] == '0') {
              /* Always write three octal digits to avoid errors if */
              /* the octal representation is followed by a digit.   */
              literal->mem[pos + 1] = (strElemType) '0';
              /* Write the character as two octal digits. */
              /* This code is much faster than sprintf(). */
              literal->mem[pos + 2] = (strElemType) ((character >> 3 & 0x7) + '0');
              literal->mem[pos + 3] = (strElemType) ((character      & 0x7) + '0');
              pos += 4;
            } else {
              literal->mem[pos + 1] = (strElemType) cstri_escape_sequence[character][1];
              pos += 2;
            } /* if */
#if TRIGRAPH_SEQUENCES_ARE_REPLACED
          } else if (character == '\\' || character == '\"' ||
              (character == '?' && position >= 1 && stri->mem[position - 1] == '?')) {
#else
          } else if (character == '\\' || character == '\"') {
#endif
            literal->mem[pos]     = (strElemType) '\\';
            literal->mem[pos + 1] = character;
            pos += 2;
          } else {
            literal->mem[pos]     = character;
            pos++;
          } /* if */
        } else if (character < 256) {
          literal->mem[pos]     = (strElemType) '\\';
          /* Write the character as three octal digits. */
          /* This code is much faster than sprintf().   */
          literal->mem[pos + 1] = (strElemType) ((character >> 6 & 0x7) + '0');
          literal->mem[pos + 2] = (strElemType) ((character >> 3 & 0x7) + '0');
          literal->mem[pos + 3] = (strElemType) ((character      & 0x7) + '0');
          pos += 4;
        } else {
          FREE_STRI(literal, escSequenceMax * striSize + numOfQuotes);
          logError(printf("strCLit(\"%s\"): Character > '\\255;' found.\n",
                          striAsUnquotedCStri(stri)););
          raise_error(RANGE_ERROR);
          return NULL;
        } /* if */
      } /* for */
      literal->mem[pos] = (strElemType) '"';
      pos++;
      literal->size = pos;
      REALLOC_STRI_SIZE_SMALLER(resized_literal, literal,
          escSequenceMax * striSize + numOfQuotes, pos);
      if (unlikely(resized_literal == NULL)) {
        FREE_STRI(literal, escSequenceMax * striSize + numOfQuotes);
        raise_error(MEMORY_ERROR);
        literal = NULL;
      } else {
        literal = resized_literal;
        COUNT3_STRI(escSequenceMax * striSize + numOfQuotes, pos);
      } /* if */
    } /* if */
    return literal;
  } /* strCLit */



/**
 *  Compare two strings.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType strCompare (const const_striType stri1, const const_striType stri2)

  {
    intType signumValue;

  /* strCompare */
#if !HAS_WMEMCMP || WCHAR_T_SIZE != 32 || WMEMCMP_RETURNS_SIGNUM
    if (stri1->size < stri2->size) {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri1->size);
      if (signumValue == 0) {
        signumValue = -1;
      } /* if */
    } else {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri2->size);
      if (signumValue == 0 && stri1->size > stri2->size) {
        signumValue = 1;
      } /* if */
    } /* if */
#else
    if (stri1->size < stri2->size) {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0) {
        signumValue = -1;
      } else {
        signumValue = 1;
      } /* if */
    } else {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri2->size);
      if (signumValue == 0) {
        if (stri1->size > stri2->size) {
          signumValue = 1;
        } /* if */
      } else if (signumValue > 0) {
        signumValue = 1;
      } else {
        signumValue = -1;
      } /* if */
    } /* if */
#endif
    return signumValue;
  } /* strCompare */



/**
 *  Reinterpret the generic parameters as striType and call strCompare.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(striType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType strCmpGeneric (const genericType value1, const genericType value2)

  { /* strCmpGeneric */
    return strCompare(((const_rtlObjectType *) &value1)->value.striValue,
                      ((const_rtlObjectType *) &value2)->value.striValue);
  } /* strCmpGeneric */



/**
 *  Concatenate two strings.
 *  @return the result of the concatenation.
 */
striType strConcat (const const_striType stri1, const const_striType stri2)

  {
    memSizeType result_size;
    striType result;

  /* strConcat */
    logFunction(printf("strConcat(\"%s\", ", striAsUnquotedCStri(stri1));
                printf("\"%s\")", striAsUnquotedCStri(stri2));
                fflush(stdout););
    if (unlikely(stri1->size > MAX_STRI_LEN - stri2->size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = stri1->size + stri2->size;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        memcpy(result->mem, stri1->mem,
               stri1->size * sizeof(strElemType));
        memcpy(&result->mem[stri1->size], stri2->mem,
               stri2->size * sizeof(strElemType));
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strConcat */



/**
 *  Concatenate an arbitrary number of strings.
 *  StrConcatN is used by the compiler to optimize the concatenation of
 *  three or more strings.
 *  @param arraySize Number of strings in striArray (>= 3).
 *  @return the result of the concatenation.
 */
striType strConcatN (const const_striType striArray[], memSizeType arraySize)

  {
    memSizeType pos;
    memSizeType result_size;
    memSizeType size_limit = MAX_STRI_LEN;
    memSizeType elem_size;
    strElemType *dest;
    striType result;

  /* strConcatN */
    logFunction(printf("strConcatN(");
                for (pos = 0; pos < arraySize; pos++) {
                  printf("\"%s\", ",
                         striAsUnquotedCStri(striArray[pos]));
                } /* if */
                printf(FMT_U_MEM ")", arraySize);
                fflush(stdout););
    pos = arraySize;
    do {
      pos--;
      if (unlikely(striArray[pos]->size > size_limit)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        size_limit -= striArray[pos]->size;
      } /* if */
    } while (pos != 0);
    result_size = MAX_STRI_LEN - size_limit;
    /* printf("result_size=" FMT_U_MEM "\n", result_size); */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      dest = result->mem;
      for (pos = 0; pos < arraySize; pos++) {
        elem_size = striArray[pos]->size;
        memcpy(dest, striArray[pos]->mem, elem_size * sizeof(strElemType));
        dest += elem_size;
      } /* for */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strConcatN */



/**
 *  Concatenate two strings.
 *  The parameter 'stri1' is resized and 'stri2' is copied to the
 *  enlarged area of 'stri1'. StrConcatTemp is used by the compiler
 *  if 'stri1' is temporary value that can be reused.
 *  @return the resized parameter 'stri1.
 */
striType strConcatTemp (striType stri1, const const_striType stri2)

  {
    memSizeType result_size;
    striType resized_stri1;

  /* strConcatTemp */
    logFunction(printf("strConcatTemp(\"%s\", ", striAsUnquotedCStri(stri1));
                printf("\"%s\")", striAsUnquotedCStri(stri2));
                fflush(stdout););
    if (unlikely(stri1->size > MAX_STRI_LEN - stri2->size)) {
      /* number of bytes does not fit into memSizeType */
      FREE_STRI(stri1, stri1->size);
      raise_error(MEMORY_ERROR);
      stri1 = NULL;
    } else {
      result_size = stri1->size + stri2->size;
#if WITH_STRI_CAPACITY
      if (result_size > stri1->capacity) {
        /* Because 'stri1' is a temporary string it cannot happen */
        /* that 'stri2' is identical to 'stri1' or a slice of it. */
        resized_stri1 = growStri(stri1, result_size);
        if (unlikely(resized_stri1 == NULL)) {
          FREE_STRI(stri1, stri1->size);
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          stri1 = resized_stri1;
        } /* if */
      } /* if */
      COUNT_GROW_STRI(stri1->size, result_size);
      memcpy(&stri1->mem[stri1->size], stri2->mem,
             stri2->size * sizeof(strElemType));
      stri1->size = result_size;
#else
      /* Because 'stri1' is a temporary string it cannot happen */
      /* that 'stri2' is identical to 'stri1' or a slice of it.  */
      GROW_STRI(resized_stri1, stri1, stri1->size, result_size);
      if (unlikely(resized_stri1 == NULL)) {
        FREE_STRI(stri1, stri1->size);
        raise_error(MEMORY_ERROR);
        stri1 = NULL;
      } else {
        stri1 = resized_stri1;
        COUNT_GROW_STRI(stri1->size, result_size);
        memcpy(&stri1->mem[stri1->size], stri2->mem,
               stri2->size * sizeof(strElemType));
        stri1->size = result_size;
      } /* if */
#endif
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(stri1)););
    return stri1;
  } /* strConcatTemp */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void strCopy (striType *const dest, const const_striType source)

  {
    memSizeType new_size;
    striType stri_dest;

  /* strCopy */
    logFunction(printf("strCopy(\"%s\", ", striAsUnquotedCStri(*dest));
                printf("\"%s\")", striAsUnquotedCStri(source));
                fflush(stdout););
    stri_dest = *dest;
    new_size = source->size;
    if (stri_dest->size == new_size) {
      /* It is possible that stri_dest and source overlap. */
      memmove(stri_dest->mem, source->mem,
          new_size * sizeof(strElemType));
    } else {
#if WITH_STRI_CAPACITY
      if (stri_dest->capacity >= new_size && !SHRINK_REASON(stri_dest, new_size)) {
        COUNT_GROW2_STRI(stri_dest->size, new_size);
        stri_dest->size = new_size;
        /* It is possible that stri_dest and source overlap. */
        memmove(stri_dest->mem, source->mem,
            new_size * sizeof(strElemType));
#else
      if (stri_dest->size > new_size) {
        /* It is possible that stri_dest and source overlap. */
        /* The move must be done before the shrink, to avoid */
        /* accessing non-existing data.                      */
        memmove(stri_dest->mem, source->mem,
            new_size * sizeof(strElemType));
        SHRINK_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
        if (unlikely(stri_dest == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT_SHRINK_STRI(stri_dest->size, new_size);
          stri_dest->size = new_size;
          *dest = stri_dest;
        } /* if */
#endif
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(stri_dest, new_size))) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          stri_dest->size = new_size;
          memcpy(stri_dest->mem, source->mem,
                 new_size * sizeof(strElemType));
          FREE_STRI(*dest, (*dest)->size);
          *dest = stri_dest;
        } /* if */
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*dest)););
  } /* strCopy */



/**
 *  Reinterpret the generic parameters as striType and call strCopy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(striType).
 */
void strCpyGeneric (genericType *const dest, const genericType source)

  { /* strCpyGeneric */
    strCopy(&((rtlObjectType *) dest)->value.striValue,
            ((const_rtlObjectType *) &source)->value.striValue);
  } /* strCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strCreate (const const_striType source)

  {
    memSizeType new_size;
    striType result;

  /* strCreate */
    logFunction(printf("strCreate(\"%s\")", striAsUnquotedCStri(source));
                fflush(stdout););
    new_size = source->size;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      if (new_size != 0) {
        memcpy(result->mem, source->mem, new_size * sizeof(strElemType));
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(striType).
 */
genericType strCreateGeneric (const genericType source)

  {
    rtlObjectType result;

  /* strCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.striValue =
        strCreate(((const_rtlObjectType *) &source)->value.striValue);
    return result.value.genericValue;
  } /* strCreateGeneric */



/**
 *  Free the memory referred by 'old_string'.
 *  After strDestr is left 'old_string' refers to not existing memory.
 *  The memory where 'old_string' is stored can be freed afterwards.
 */
void strDestr (const const_striType old_string)

  { /* strDestr */
    logFunction(printf("strDestr(\"%s\")\n", striAsUnquotedCStri(old_string)););
    if (old_string != NULL) {
      FREE_STRI(old_string, old_string->size);
    } /* if */
  } /* strDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(striType).
 */
void strDestrGeneric (const genericType old_value)

  { /* strDestrGeneric */
    strDestr(((const_rtlObjectType *) &old_value)->value.striValue);
  } /* strDestrGeneric */



/**
 *  Return an empty string, that can be assigned to a new destination.
 *  This function is used as performance optimization by the compiler.
 *  StrEmpty is used instead of strCreate, if it is known at
 *  compile-time, that the source string is empty.
 *  @return an empty string.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strEmpty (void)

  {
    striType result;

  /* strEmpty */
    logFunction(printf("strEmpty()");
                fflush(stdout););
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = 0;
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strEmpty */



/**
 *  Check if stri1 is greater than or equal to stri2.
 *  @return TRUE if stri1 is greater than or equal to stri2,
 *          FALSE otherwise.
 */
boolType strGe (const const_striType stri1, const const_striType stri2)

  { /* strGe */
    if (stri1->size >= stri2->size) {
      return memcmp_strelem(stri1->mem, stri2->mem, stri2->size) >= 0;
    } else {
      return memcmp_strelem(stri1->mem, stri2->mem, stri1->size) > 0;
    } /* if */
  } /* strGe */



/**
 *  Check if stri1 is greater than stri2.
 *  @return TRUE if stri1 is greater than stri2,
 *          FALSE otherwise.
 */
boolType strGt (const const_striType stri1, const const_striType stri2)

  { /* strGt */
    if (stri1->size > stri2->size) {
      return memcmp_strelem(stri1->mem, stri2->mem, stri2->size) >= 0;
    } else {
      return memcmp_strelem(stri1->mem, stri2->mem, stri1->size) > 0;
    } /* if */
  } /* strGt */



/**
 *  Compute the hash value of a string.
 *  @return the hash value.
 */
intType strHashCode (const const_striType stri)

  { /* strHashCode */
    return hashCode(stri);
  } /* strHashCode */



#if ALLOW_STRITYPE_SLICES
/**
 *  Get a substring ending at a stop position.
 *  The first character in a string has the position 1.
 *  This function is used by the compiler to avoid copying string data.
 *  The 'slice' is initialized to refer to the head of 'stri'
 */
void strHeadSlice (const const_striType stri, const intType stop, striType slice)

  {
    memSizeType striSize;

  /* strHeadSlice */
    logFunction(printf("strHeadSlice(\"%s\", " FMT_D ")",
                       striAsUnquotedCStri(stri), stop);
                fflush(stdout););
    striSize = stri->size;
    if (stop >= 1 && striSize >= 1) {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = stri->mem;
      if (striSize <= (uintType) stop) {
        slice->size = striSize;
      } else {
        slice->size = (memSizeType) stop;
      } /* if */
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(slice)););
  } /* strHeadSlice */

#endif



/**
 *  Get a substring ending at a stop position.
 *  The first character in a string has the position 1.
 *  @return the substring ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strHead (const const_striType stri, const intType stop)

  {
    memSizeType striSize;
    memSizeType headSize;
    striType head;

  /* strHead */
    logFunction(printf("strHead(\"%s\", " FMT_D ")",
                       striAsUnquotedCStri(stri), stop);
                fflush(stdout););
    striSize = stri->size;
    if (stop >= 1 && striSize >= 1) {
      if (striSize <= (uintType) stop) {
        headSize = striSize;
      } else {
        headSize = (memSizeType) stop;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(head, headSize))) {
        raise_error(MEMORY_ERROR);
      } else {
        head->size = headSize;
        memcpy(head->mem, stri->mem, headSize * sizeof(strElemType));
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(head, (memSizeType) 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        head->size = 0;
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(head)););
    return head;
  } /* strHead */



/**
 *  Get a substring ending at a stop position.
 *  The first character in a string has the position 1.
 *  StrHeadTemp is used by the compiler if 'stri' is temporary
 *  value that can be reused.
 *  @return the substring ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strHeadTemp (const striType stri, const intType stop)

  {
    memSizeType striSize;
    memSizeType headSize;
    striType head;

  /* strHeadTemp */
    striSize = stri->size;
    if (stop >= 1 && striSize >= 1) {
      if (striSize <= (uintType) stop) {
        return stri;
      } else {
        headSize = (memSizeType) stop;
      } /* if */
    } else {
      headSize = 0;
    } /* if */
#if WITH_STRI_CAPACITY
    if (!SHRINK_REASON(stri, headSize)) {
      COUNT_GROW2_STRI(striSize, headSize);
      head = stri;
      head->size = headSize;
    } else {
      head = shrinkStri(stri, headSize);
      if (unlikely(head == NULL)) {
        /* Theoretical shrinking a memory area should never fail.  */
        /* For the strange case that it fails we keep stri intact  */
        /* to avoid a heap corruption. Consider this expression:   */
        /* aString = strHeadTemp(aString, anIndex);                */
        /* In compiled programs the assignment to aString would be */
        /* skipped because raise_error triggers a longjmp().       */
        /* Therefore stri would keep the old value.                */
        /* If shrinking a memory area fails with the expression:   */
        /* strHeadTemp(anExpression, anIndex)                      */
        /* the result of anExpression is not freed (memory leak).  */
        /* FREE_STRI(stri, stri->size); */
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_SHRINK_STRI(striSize, headSize);
        head->size = headSize;
      } /* if */
    } /* if */
#else
    SHRINK_STRI(head, stri, striSize, headSize);
    if (unlikely(head == NULL)) {
      /* Theoretical shrinking a memory area should never fail.  */
      /* See above for a description of this situation.          */
      /* FREE_STRI(stri, stri->size); */
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_SHRINK_STRI(striSize, headSize);
      head->size = headSize;
    } /* if */
#endif
    return head;
  } /* strHeadTemp */



/**
 *  Search string 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  This function uses a modified Boyer–Moore string search algorithm.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or after 'fromIndex'.
 */
static intType strIPos2 (const const_striType mainStri, const const_striType searched,
    const intType fromIndex)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_n;
    const strElemType *ch_n_pos;
    memSizeType delta;
    memSizeType charDelta[CHAR_DELTA_BEYOND + 1];
    memSizeType pos;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strIPos2 */
    main_size = mainStri->size - ((memSizeType) fromIndex - 1);
    searched_size = searched->size;
    for (ch_n = 0; ch_n <= CHAR_DELTA_BEYOND; ch_n++) {
      charDelta[ch_n] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = 0; pos < searched_size - 1; pos++) {
      ch_n = searched_mem[pos];
      if (ch_n < CHAR_DELTA_BEYOND) {
        charDelta[ch_n] = searched_size - pos - 1;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = searched_size - pos - 1;
      } /* if */
    } /* for */
    ch_n = searched_mem[searched_size - 1];
    ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
    if (ch_n_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memSizeType) (&searched_mem[searched_size - 1] - ch_n_pos);
    } /* if */
    main_mem = &mainStri->mem[fromIndex - 1];
    search_start = &main_mem[searched_size - 1];
    search_end = &main_mem[main_size];
    while (search_start < search_end) {
      search_start = search_strelem2(search_start, ch_n, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start - searched_size + 1, searched_mem,
            (searched_size - 1) * sizeof(strElemType)) == 0) {
          return ((intType) (search_start - searched_size + 1 - main_mem)) + fromIndex;
        } else {
          search_start += delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strIPos2 */



/**
 *  Search string 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  This function calls strIPos2 if 'mainStri' is long.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or after 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' <= 0 holds.
 */
intType strIPos (const const_striType mainStri, const const_striType searched,
    const intType fromIndex)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_n;
    const strElemType *ch_n_pos;
    memSizeType delta;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strIPos */
    logFunction(printf("strIPos(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(searched), fromIndex););
    if (unlikely(fromIndex <= 0)) {
      logError(printf("strIPos(\"%s\", ",
                      striAsUnquotedCStri(mainStri));
               printf("\"%s\", " FMT_D "): fromIndex <= 0.\n",
                      striAsUnquotedCStri(searched), fromIndex););
      raise_error(RANGE_ERROR);
    } else {
      main_size = mainStri->size;
      searched_size = searched->size;
      if (searched_size != 0 && main_size >= searched_size &&
          (uintType) fromIndex - 1 <= main_size - searched_size) {
        main_size -= (memSizeType) fromIndex - 1;
        if (searched_size >= BOYER_MOORE_SEARCHED_STRI_THRESHOLD &&
            main_size >= BOYER_MOORE_MAIN_STRI_THRESHOLD) {
          return strIPos2(mainStri, searched, fromIndex);
        } else if (searched_size == 1) {
          return strChIPos(mainStri, searched->mem[0], fromIndex);
        } else {
          searched_mem = searched->mem;
          ch_n = searched_mem[searched_size - 1];
          ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
          if (ch_n_pos == NULL) {
            delta = searched_size;
          } else {
            delta = (memSizeType) (&searched_mem[searched_size - 1] - ch_n_pos);
          } /* if */
          main_mem = &mainStri->mem[fromIndex - 1];
          search_start = &main_mem[searched_size - 1];
          search_end = &main_mem[main_size];
          while (search_start < search_end) {
            search_start = memchr_strelem(search_start, ch_n,
                (memSizeType) (search_end - search_start));
            if (search_start == NULL) {
              return 0;
            } else {
              if (memcmp(search_start - searched_size + 1, searched_mem,
                  (searched_size - 1) * sizeof(strElemType)) == 0) {
                return ((intType) (search_start - searched_size + 1 - main_mem)) + fromIndex;
              } else {
                search_start += delta;
              } /* if */
            } /* if */
          } /* while */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strIPos */



/**
 *  Check if stri1 is less than or equal to stri2.
 *  @return TRUE if stri1 is less than or equal to stri2,
 *          FALSE otherwise.
 */
boolType strLe (const const_striType stri1, const const_striType stri2)

  { /* strLe */
    if (stri1->size <= stri2->size) {
      return memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0;
    } else {
      return memcmp_strelem(stri1->mem, stri2->mem, stri2->size) < 0;
    } /* if */
  } /* strLe */



striType strLit (const const_striType stri)

  {
    /* A string literal starts and ends with double quotes ("): */
    const memSizeType numOfQuotes = 2;
    register strElemType character;
    register memSizeType position;
    memSizeType striSize;
    memSizeType pos;
    char escapeBuffer[ESC_SEQUENCE_MAX_LEN + NULL_TERMINATION_LEN];
    memSizeType len;
    striType resized_literal;
    striType literal;

  /* strLit */
    striSize = stri->size;
    if (unlikely(striSize > (MAX_STRI_LEN - numOfQuotes) / ESC_SEQUENCE_MAX_LEN ||
                 !ALLOC_STRI_SIZE_OK(literal, ESC_SEQUENCE_MAX_LEN * striSize + numOfQuotes))) {
      raise_error(MEMORY_ERROR);
      literal = NULL;
    } else {
      literal->mem[0] = (strElemType) '"';
      pos = 1;
      for (position = 0; position < striSize; position++) {
        character = (strElemType) stri->mem[position];
        if (character < 127) {
          if (character < ' ') {
            literal->mem[pos] = (strElemType) '\\';
            if (stri_escape_sequence[character][1] <= '9') {
              /* Numeric escape sequence with one or two digits. */
              if (character <= 9) {
                literal->mem[pos + 1] = character + '0';
                literal->mem[pos + 2] = (strElemType) ';';
                pos += 3;
              } else {
                literal->mem[pos + 1] = (strElemType) stri_escape_sequence[character][1];
                literal->mem[pos + 2] = (strElemType) stri_escape_sequence[character][2];
                literal->mem[pos + 3] = (strElemType) ';';
                pos += 4;
              } /* if */
            } else {
              /* Character escape sequence. */
              literal->mem[pos + 1] = (strElemType) stri_escape_sequence[character][1];
              pos += 2;
            } /* if */
          } else if (character == '\\' || character == '\"') {
            literal->mem[pos]     = (strElemType) '\\';
            literal->mem[pos + 1] = character;
            pos += 2;
          } else {
            literal->mem[pos]     = character;
            pos++;
          } /* if */
        } else if (character <= 160) {
          /* Write characters between 128 and 160 as decimal. */
          /* This code is much faster than sprintf().         */
          literal->mem[pos]     = (strElemType) '\\';
          literal->mem[pos + 3] = character % 10 + '0';
          character /= 10;
          literal->mem[pos + 2] = character % 10 + '0';
          literal->mem[pos + 1] = '1';
          literal->mem[pos + 4] = (strElemType) ';';
          pos += 5;
        } else if (character >= 256) {
          len = (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", character);
          memcpy_to_strelem(&literal->mem[pos], (const_ustriType) escapeBuffer, len);
          pos += len;
        } else {
          literal->mem[pos] = character;
          pos++;
        } /* if */
      } /* for */
      literal->mem[pos] = (strElemType) '"';
      pos++;
      literal->size = pos;
      REALLOC_STRI_SIZE_SMALLER(resized_literal, literal,
          ESC_SEQUENCE_MAX_LEN * striSize + numOfQuotes, pos);
      if (unlikely(resized_literal == NULL)) {
        FREE_STRI(literal, ESC_SEQUENCE_MAX_LEN * striSize + numOfQuotes);
        raise_error(MEMORY_ERROR);
        literal = NULL;
      } else {
        literal = resized_literal;
        COUNT3_STRI(ESC_SEQUENCE_MAX_LEN * striSize + numOfQuotes, pos);
      } /* if */
    } /* if */
    return literal;
  } /* strLit */



/**
 *  Convert a string to lower case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the string converted to lower case.
 */
striType strLow (const const_striType stri)

  {
    memSizeType striSize;
    striType result;

  /* strLow */
    logFunction(printf("strLow(\"%s\")",
                       striAsUnquotedCStri(stri));
                fflush(stdout););
    striSize = stri->size;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = striSize;
      toLower(stri->mem, stri->size, result->mem);
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strLow */



/**
 *  Convert a string to lower case.
 *  StrLowTemp is used by the compiler if 'stri' is temporary
 *  value that can be reused.
 *  @return the string converted to lower case.
 */
striType strLowTemp (const striType stri)

  { /* strLowTemp */
    logFunction(printf("strLowTemp(\"%s\")",
                       striAsUnquotedCStri(stri));
                fflush(stdout););
    toLower(stri->mem, stri->size, stri->mem);
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(stri)););
    return stri;
  } /* strLowTemp */



/**
 *  Pad a string with spaces at the left side up to padSize.
 *  @return the string left padded with spaces.
 */
striType strLpad (const const_striType stri, const intType padSize)

  {
    memSizeType striSize;
    striType result;

  /* strLpad */
    striSize = stri->size;
    if (padSize > 0 && (uintType) padSize > striSize) {
      if (unlikely((uintType) padSize > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) padSize))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) padSize;
#if LPAD_WITH_MEMSET_TO_STRELEM
        memset_to_strelem(result->mem, ' ', (memSizeType) padSize - striSize);
#else
        {
          strElemType *elem = result->mem;
          memSizeType idx = (memSizeType) padSize - striSize - 1;

          do {
            elem[idx] = (strElemType) ' ';
          } while (idx-- != 0);
        }
#endif
        memcpy(&result->mem[(memSizeType) padSize - striSize], stri->mem,
               striSize * sizeof(strElemType));
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = striSize;
        memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
      } /* if */
    } /* if */
    return result;
  } /* strLpad */



/**
 *  Pad a string with spaces at the left side up to padSize.
 *  StrLpadTemp is used by the compiler if 'stri' is temporary
 *  value that can be reused.
 *  @return the string left padded with spaces.
 */
striType strLpadTemp (const striType stri, const intType padSize)

  {
    memSizeType striSize;
    striType result;

  /* strLpadTemp */
    striSize = stri->size;
    if (padSize > 0 && (uintType) padSize > striSize) {
      if (unlikely((uintType) padSize > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) padSize))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) padSize;
#if LPAD_WITH_MEMSET_TO_STRELEM
        memset_to_strelem(result->mem, ' ', (memSizeType) padSize - striSize);
#else
        {
          strElemType *elem = result->mem;
          memSizeType idx = (memSizeType) padSize - striSize - 1;

          do {
            elem[idx] = (strElemType) ' ';
          } while (idx-- != 0);
        }
#endif
        memcpy(&result->mem[(memSizeType) padSize - striSize], stri->mem,
               striSize * sizeof(strElemType));
        FREE_STRI(stri, striSize);
      } /* if */
    } else {
      result = stri;
    } /* if */
    return result;
  } /* strLpadTemp */



/**
 *  Pad a string with zeroes at the left side up to padSize.
 *  @return the string left padded with zeroes.
 */
striType strLpad0 (const const_striType stri, const intType padSize)

  {
    memSizeType striSize;
    const strElemType *sourceElem;
    strElemType *destElem;
    memSizeType len;
    striType result;

  /* strLpad0 */
    striSize = stri->size;
    if (padSize > 0 && (uintType) padSize > striSize) {
      if (unlikely((uintType) padSize > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) padSize))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) padSize;
        sourceElem = stri->mem;
        destElem = result->mem;
        len = (memSizeType) padSize - striSize;
        if (striSize != 0 && (sourceElem[0] == '-' || sourceElem[0] == '+')) {
          *destElem++ = sourceElem[0];
          sourceElem++;
          striSize--;
        } /* if */
        while (len--) {
          *destElem++ = (strElemType) '0';
        } /* while */
        memcpy(destElem, sourceElem, striSize * sizeof(strElemType));
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = striSize;
        memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
      } /* if */
    } /* if */
    return result;
  } /* strLpad0 */



/**
 *  Pad a string with zeroes at the left side up to padSize.
 *  StrLpad0Temp is used by the compiler if 'stri' is temporary
 *  value that can be reused.
 *  @return the string left padded with zeroes.
 */
striType strLpad0Temp (const striType stri, const intType padSize)

  {
    memSizeType striSize;
    const strElemType *sourceElem;
    strElemType *destElem;
    memSizeType len;
    striType result;

  /* strLpad0Temp */
    striSize = stri->size;
    if (padSize > 0 && (uintType) padSize > striSize) {
      if (unlikely((uintType) padSize > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) padSize))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) padSize;
        sourceElem = stri->mem;
        destElem = result->mem;
        len = (memSizeType) padSize - striSize;
        if (striSize != 0 && (sourceElem[0] == '-' || sourceElem[0] == '+')) {
          *destElem++ = sourceElem[0];
          sourceElem++;
          striSize--;
        } /* if */
        while (len--) {
          *destElem++ = (strElemType) '0';
        } /* while */
        memcpy(destElem, sourceElem, striSize * sizeof(strElemType));
        FREE_STRI(stri, striSize);
      } /* if */
    } else {
      result = stri;
    } /* if */
    return result;
  } /* strLpad0Temp */



/**
 *  Check if stri1 is less than stri2.
 *  @return TRUE if stri1 is less than stri2,
 *          FALSE otherwise.
 */
boolType strLt (const const_striType stri1, const const_striType stri2)

  { /* strLt */
    if (stri1->size < stri2->size) {
      return memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0;
    } else {
      return memcmp_strelem(stri1->mem, stri2->mem, stri2->size) < 0;
    } /* if */
  } /* strLt */



/**
 *  Return string with leading whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with leading whitespace omitted.
 */
striType strLtrim (const const_striType stri)

  {
    memSizeType start = 0;
    memSizeType striSize;
    striType result;

  /* strLtrim */
    striSize = stri->size;
    if (striSize >= 1) {
      while (start < striSize && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      striSize -= start;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = striSize;
      memcpy(result->mem, &stri->mem[start], striSize * sizeof(strElemType));
      return result;
    } /* if */
  } /* strLtrim */



/**
 *  String multiplication.
 *  The string 'stri' is concatenated to itself such that in total
 *  'factor' strings are concatenated.
 *   "LA" mult 3     returns "LALALA"
 *   "WORD" mult 0   returns ""
 *  @return the result of the string multiplication.
 *  @exception RANGE_ERROR If the factor is negative.
 */
striType strMult (const const_striType stri, const intType factor)

  {
    memSizeType len;
    memSizeType numOfRepeats;
    memSizeType powerOfTwo;
    strElemType ch;
    memSizeType result_size;
    striType result;

  /* strMult */
    logFunction(printf("strMult(\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(stri), factor););
    if (unlikely(factor < 0)) {
      logError(printf("strMult(\"%s\", " FMT_D "): Negative factor.\n",
                      striAsUnquotedCStri(stri), factor););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      len = stri->size;
      if (unlikely(len == 0)) {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } else if (unlikely((uintType) factor > MAX_STRI_LEN / len)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        numOfRepeats = (memSizeType) factor;
        result_size = numOfRepeats * len;
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          if (len == 1) {
            ch = stri->mem[0];
            memset_to_strelem(result->mem, ch, numOfRepeats);
          } else if (numOfRepeats != 0) {
            /* Use binary method for string multiplication: */
            memcpy(result->mem, stri->mem, len * sizeof(strElemType));
            powerOfTwo = 1;
            while (powerOfTwo << 1 < numOfRepeats) {
              memcpy(&result->mem[powerOfTwo * len], result->mem,
                     powerOfTwo * len * sizeof(strElemType));
              powerOfTwo <<= 1;
            } /* while */
            memcpy(&result->mem[powerOfTwo * len], result->mem,
                   (numOfRepeats - powerOfTwo) * len * sizeof(strElemType));
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* strMult */



/**
 *  Determine leftmost position of string 'searched' in 'mainStri'.
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  This function uses a modified Boyer–Moore string search algorithm.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
static intType strPos2 (const const_striType mainStri, const const_striType searched)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_n;
    const strElemType *ch_n_pos;
    memSizeType delta;
    memSizeType charDelta[CHAR_DELTA_BEYOND + 1];
    memSizeType pos;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strPos2 */
    main_size = mainStri->size;
    searched_size = searched->size;
    for (ch_n = 0; ch_n <= CHAR_DELTA_BEYOND; ch_n++) {
      charDelta[ch_n] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = 0; pos < searched_size - 1; pos++) {
      ch_n = searched_mem[pos];
      if (ch_n < CHAR_DELTA_BEYOND) {
        charDelta[ch_n] = searched_size - pos - 1;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = searched_size - pos - 1;
      } /* if */
    } /* for */
    ch_n = searched_mem[searched_size - 1];
    ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
    if (ch_n_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memSizeType) (&searched_mem[searched_size - 1] - ch_n_pos);
    } /* if */
    main_mem = mainStri->mem;
    search_start = &main_mem[searched_size - 1];
    search_end = &main_mem[main_size];
    while (search_start < search_end) {
      search_start = search_strelem2(search_start, ch_n, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start - searched_size + 1, searched_mem,
            (searched_size - 1) * sizeof(strElemType)) == 0) {
          return ((intType) (search_start - searched_size + 1 - main_mem)) + 1;
        } else {
          search_start += delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strPos2 */



/**
 *  Determine leftmost position of string 'searched' in 'mainStri'.
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  This function calls strPos2 if 'mainStri' is long.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
intType strPos (const const_striType mainStri, const const_striType searched)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_n;
    const strElemType *ch_n_pos;
    memSizeType delta;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strPos */
    logFunction(printf("strPos(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\")\n",
                       striAsUnquotedCStri(searched)););
    main_size = mainStri->size;
    searched_size = searched->size;
    if (searched_size != 0 && main_size >= searched_size) {
      if (searched_size >= BOYER_MOORE_SEARCHED_STRI_THRESHOLD &&
          main_size >= BOYER_MOORE_MAIN_STRI_THRESHOLD) {
        return strPos2(mainStri, searched);
      } else if (searched_size == 1) {
        return strChPos(mainStri, searched->mem[0]);
      } else {
        searched_mem = searched->mem;
        ch_n = searched_mem[searched_size - 1];
        ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
        if (ch_n_pos == NULL) {
          delta = searched_size;
        } else {
          delta = (memSizeType) (&searched_mem[searched_size - 1] - ch_n_pos);
        } /* if */
        main_mem = mainStri->mem;
        search_start = &main_mem[searched_size - 1];
        search_end = &main_mem[main_size];
        while (search_start < search_end) {
          search_start = memchr_strelem(search_start, ch_n,
              (memSizeType) (search_end - search_start));
          if (search_start == NULL) {
            return 0;
          } else {
            if (memcmp(search_start - searched_size + 1, searched_mem,
                (searched_size - 1) * sizeof(strElemType)) == 0) {
              return ((intType) (search_start - searched_size + 1 - main_mem)) + 1;
            } else {
              search_start += delta;
            } /* if */
          } /* if */
        } /* while */
      } /* if */
    } /* if */
    return 0;
  } /* strPos */



/**
 *  Append the char 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
void strPush (striType *const destination, const charType extension)

  {
    memSizeType new_size;
    striType stri_dest;

  /* strPush */
    logFunction(printf("strPush(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(*destination), extension);
                fflush(stdout););
    stri_dest = *destination;
    new_size = stri_dest->size + 1;
#if WITH_STRI_CAPACITY
    if (new_size > stri_dest->capacity) {
      stri_dest = growStri(stri_dest, new_size);
      if (unlikely(stri_dest == NULL)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        *destination = stri_dest;
      } /* if */
    } /* if */
    COUNT_GROW_STRI(stri_dest->size, new_size);
    stri_dest->mem[stri_dest->size] = extension;
    stri_dest->size = new_size;
#else
    GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
    if (unlikely(stri_dest == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_GROW_STRI(stri_dest->size, new_size);
      stri_dest->mem[stri_dest->size] = extension;
      stri_dest->size = new_size;
      *destination = stri_dest;
    } /* if */
#endif
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(*destination)););
  } /* strPush */



#if ALLOW_STRITYPE_SLICES
/**
 *  Get a substring from a start position to a stop position.
 *  The first character in a string has the position 1.
 *  This function is used by the compiler to avoid copying string data.
 *  The 'slice' is initialized to refer to the range of 'stri'
 */
void strRangeSlice (const const_striType stri, intType start, intType stop, striType slice)

  {
    memSizeType striSize;

  /* strRangeSlice */
    logFunction(printf("strRangeSlice(\"%s\", " FMT_D ", " FMT_D ")",
                       striAsUnquotedCStri(stri), start, stop);
                fflush(stdout););
    striSize = stri->size;
    if (unlikely(start < 1)) {
      start = 1;
    } /* if */
    SET_SLICE_CAPACITY(slice, 0);
    if (stop >= start && (uintType) start <= striSize) {
      slice->mem = &stri->mem[start - 1];
      if ((uintType) stop > striSize) {
        slice->size = striSize - (memSizeType) start + 1;
      } else {
        slice->size = (memSizeType) stop - (memSizeType) start + 1;
      } /* if */
    } else {
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(slice)););
  } /* strRangeSlice */

#endif



/**
 *  Get a substring from a start position to a stop position.
 *  The first character in a string has the position 1.
 *  @return the substring from position start to stop.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strRange (const const_striType stri, intType start, intType stop)

  {
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* strRange */
    striSize = stri->size;
    if (unlikely(start < 1)) {
      start = 1;
    } /* if */
    if (stop >= start && (uintType) start <= striSize) {
      if ((uintType) stop > striSize) {
        result_size = striSize - (memSizeType) start + 1;
      } else {
        result_size = (memSizeType) stop - (memSizeType) start + 1;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* if using the -Ozacegilt optimisation option in the     */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
             result_size * sizeof(strElemType));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } /* if */
    return result;
  } /* strRange */



/**
 *  Search char 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or before 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' > length(stri) holds.
 */
intType strRChIPos (const const_striType mainStri, const charType searched,
    const intType fromIndex)

  {
    const strElemType *main_mem;
    const strElemType *found_pos;

  /* strRChIPos */
    logFunction(printf("strRChIPos(\"%s\", '\\" FMT_U32 ";', " FMT_D ")\n",
                       striAsUnquotedCStri(mainStri), searched, fromIndex););
    if (likely(fromIndex >= 1)) {
      if (unlikely((uintType) fromIndex > mainStri->size)) {
        logError(printf("strRChIPos(\"%s\", '\\" FMT_U32 ";', " FMT_D "): "
                        "fromIndex <= 0.\n",
                        striAsUnquotedCStri(mainStri), searched, fromIndex););
        raise_error(RANGE_ERROR);
      } else {
        if (mainStri->size >= 1) {
          main_mem = mainStri->mem;
          found_pos = rsearch_strelem(&main_mem[fromIndex - 1], searched,
              (memSizeType) fromIndex);
          if (found_pos != NULL) {
            return ((intType) (found_pos - main_mem)) + 1;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRChIPos */



/**
 *  Determine rightmost position of char 'searched' in 'mainStri'.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
intType strRChPos (const const_striType mainStri, const charType searched)

  {
    const strElemType *main_mem;
    const strElemType *found_pos;

  /* strRChPos */
    logFunction(printf("strRChPos(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(mainStri), searched););
    if (mainStri->size >= 1) {
      main_mem = mainStri->mem;
      found_pos = rsearch_strelem(&main_mem[mainStri->size - 1], searched,
          mainStri->size);
      if (found_pos != NULL) {
        return ((intType) (found_pos - main_mem)) + 1;
      } /* if */
    } /* if */
    return 0;
  } /* strRChPos */



/**
 *  Replace all occurrences of 'searched' in 'mainStri' by 'replacement'.
 *  This function uses a modified Boyer–Moore string search algorithm.
 *  @return the result of the replacement.
 */
static memSizeType strRepl2 (const const_striType mainStri, const const_striType searched,
    const const_striType replacement, const striType result)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_n;
    const strElemType *ch_n_pos;
    memSizeType delta;
    memSizeType charDelta[CHAR_DELTA_BEYOND + 1];
    memSizeType pos;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *copy_start;
    strElemType *result_end;
    memSizeType result_size;

  /* strRepl2 */
    main_size = mainStri->size;
    searched_size = searched->size;
    for (ch_n = 0; ch_n <= CHAR_DELTA_BEYOND; ch_n++) {
      charDelta[ch_n] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = 0; pos < searched_size - 1; pos++) {
      ch_n = searched_mem[pos];
      if (ch_n < CHAR_DELTA_BEYOND) {
        charDelta[ch_n] = searched_size - pos - 1;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = searched_size - pos - 1;
      } /* if */
    } /* for */
    ch_n = searched_mem[searched_size - 1];
    ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
    if (ch_n_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memSizeType) (&searched_mem[searched_size - 1] - ch_n_pos);
    } /* if */
    main_mem = mainStri->mem;
    search_start = &main_mem[searched_size - 1];
    search_end = &main_mem[main_size];
    copy_start = mainStri->mem;
    result_end = result->mem;
    while (search_start != NULL && search_start < search_end) {
      search_start = search_strelem2(search_start, ch_n, search_end, charDelta);
      if (search_start != NULL) {
        if (memcmp(search_start - searched_size + 1, searched_mem,
            (searched_size - 1) * sizeof(strElemType)) == 0) {
          memcpy(result_end, copy_start,
                 (memSizeType) (search_start - searched_size + 1 - copy_start) *
                 sizeof(strElemType));
          result_end += search_start - searched_size + 1 - copy_start;
          memcpy(result_end, replacement->mem,
                 replacement->size * sizeof(strElemType));
          result_end += replacement->size;
          copy_start = search_start + 1;
          search_start += searched_size;
        } else {
          search_start += delta;
        } /* if */
      } /* if */
    } /* while */
    memcpy(result_end, copy_start,
           (memSizeType) (&mainStri->mem[main_size] - copy_start) * sizeof(strElemType));
    result_end += &mainStri->mem[main_size] - copy_start;
    result_size = (memSizeType) (result_end - result->mem);
    return result_size;
  } /* strRepl2 */



/**
 *  Replace all occurrences of 'searched' in 'mainStri' by 'replacement'.
 *  This function calls strRepl2 if 'mainStri' is long.
 *  @return the result of the replacement.
 */
striType strRepl (const const_striType mainStri,
    const const_striType searched, const const_striType replacement)

  {
    memSizeType main_size;
    memSizeType searched_size;
    memSizeType guessed_result_size;
    memSizeType result_size;
    strElemType ch_1;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *copy_start;
    strElemType *result_end;
    striType resized_result;
    striType result;

  /* strRepl */
    main_size = mainStri->size;
    searched_size = searched->size;
    /* printf("main_size=" FMT_U_MEM ", searched_size=" FMT_U_MEM
        ", replacement->size=" FMT_U_MEM "\n",
        main_size, searched_size, replacement->size); */
    if (searched_size != 0 && replacement->size > searched_size) {
      if (unlikely(main_size / searched_size + 1 > MAX_STRI_LEN / replacement->size)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        guessed_result_size = (main_size / searched_size + 1) * replacement->size;
      } /* if */
    } else {
      guessed_result_size = main_size;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, guessed_result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (searched_size >= BOYER_MOORE_SEARCHED_STRI_THRESHOLD &&
          main_size >= BOYER_MOORE_MAIN_STRI_THRESHOLD) {
        result_size = strRepl2(mainStri, searched, replacement, result);
      } else {
        copy_start = mainStri->mem;
        result_end = result->mem;
        if (searched_size != 0 && searched_size <= main_size) {
          searched_mem = searched->mem;
          ch_1 = searched_mem[0];
          main_mem = mainStri->mem;
          search_start = main_mem;
          search_end = &main_mem[main_size - searched_size + 1];
          while (search_start < search_end &&
              (search_start = memchr_strelem(search_start, ch_1,
                  (memSizeType) (search_end - search_start))) != NULL) {
            if (memcmp(search_start, searched_mem,
                searched_size * sizeof(strElemType)) == 0) {
              memcpy(result_end, copy_start,
                     (memSizeType) (search_start - copy_start) * sizeof(strElemType));
              result_end += search_start - copy_start;
              memcpy(result_end, replacement->mem,
                     replacement->size * sizeof(strElemType));
              result_end += replacement->size;
              search_start += searched_size;
              copy_start = search_start;
            } else {
              search_start++;
            } /* if */
          } /* while */
        } /* if */
        memcpy(result_end, copy_start,
               (memSizeType) (&mainStri->mem[main_size] - copy_start) * sizeof(strElemType));
        result_end += &mainStri->mem[main_size] - copy_start;
        result_size = (memSizeType) (result_end - result->mem);
      } /* if */
      /* printf("result=%lu, guessed_result_size=%ld, result_size=%ld\n",
         result, guessed_result_size, result_size); */
      REALLOC_STRI_SIZE_SMALLER(resized_result, result, guessed_result_size, result_size);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, guessed_result_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(guessed_result_size, result_size);
        result->size = result_size;
      } /* if */
    } /* if */
    return result;
  } /* strRepl */



/**
 *  Search string 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  This function uses a modified Boyer–Moore string search algorithm.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or before 'fromIndex'.
 */
static intType strRIPos2 (const const_striType mainStri, const const_striType searched,
    const intType fromIndex)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_1;
    const strElemType *ch_1_pos;
    memSizeType delta;
    memSizeType charDelta[CHAR_DELTA_BEYOND + 1];
    memSizeType pos;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strRIPos2 */
    main_size = mainStri->size;
    searched_size = searched->size;
    for (ch_1 = 0; ch_1 <= CHAR_DELTA_BEYOND; ch_1++) {
      charDelta[ch_1] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = searched_size - 1; pos > 0; pos--) {
      ch_1 = searched_mem[pos];
      if (ch_1 < CHAR_DELTA_BEYOND) {
        charDelta[ch_1] = pos;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = pos;
      } /* if */
    } /* for */
    ch_1 = searched_mem[0];
    ch_1_pos = memchr_strelem(&searched_mem[1], ch_1, searched_size - 1);
    if (ch_1_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memSizeType) (ch_1_pos - &searched_mem[0]);
    } /* if */
    main_mem = mainStri->mem;
    if ((uintType) fromIndex - 1 <= main_size - searched_size) {
      search_start = &main_mem[fromIndex - 1];
    } else {
      search_start = &main_mem[main_size - searched_size];
    } /* if */
    search_end = &main_mem[-1];
    while (search_start > search_end) {
      search_start = rsearch_strelem2(search_start, ch_1, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start + 1, &searched_mem[1],
            (searched_size - 1) * sizeof(strElemType)) == 0) {
          return ((intType) (search_start - main_mem)) + 1;
        } else {
          search_start -= delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strRIPos2 */



/**
 *  Search string 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  This function calls strRIPos2 if 'mainStri' is long.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched' at or before 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' > length(stri) holds.
 */
intType strRIPos (const const_striType mainStri, const const_striType searched,
    const intType fromIndex)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_1;
    const strElemType *searched_mem;
    const strElemType *main_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strRIPos */
    logFunction(printf("strRIPos(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(searched), fromIndex););
    if (likely(fromIndex >= 1)) {
      if (unlikely((uintType) fromIndex > mainStri->size)) {
        logError(printf("strRIPos(\"%s\", ",
                        striAsUnquotedCStri(mainStri));
                 printf("\"%s\", " FMT_D "): fromIndex <= 0.\n",
                        striAsUnquotedCStri(searched), fromIndex););
        raise_error(RANGE_ERROR);
      } else {
        main_size = mainStri->size;
        searched_size = searched->size;
        if (searched_size != 0 && main_size >= searched_size) {
          if (searched_size >= BOYER_MOORE_SEARCHED_STRI_THRESHOLD &&
              main_size >= BOYER_MOORE_MAIN_STRI_THRESHOLD) {
            return strRIPos2(mainStri, searched, fromIndex);
          } else {
            searched_mem = searched->mem;
            ch_1 = searched_mem[0];
            main_mem = mainStri->mem;
            if ((uintType) fromIndex - 1 <= main_size - searched_size) {
              search_start = &main_mem[fromIndex - 1];
            } else {
              search_start = &main_mem[main_size - searched_size];
            } /* if */
            search_end = &main_mem[-1];
            while ((search_start = rsearch_strelem(search_start,
                ch_1, (memSizeType) (search_start - search_end))) != NULL) {
              if (memcmp(search_start, searched_mem,
                  searched_size * sizeof(strElemType)) == 0) {
                return ((intType) (search_start - main_mem)) + 1;
              } else {
                search_start--;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRIPos */



/**
 *  Pad a string with spaces at the right side up to padSize.
 *  @return the string right padded with spaces.
 */
striType strRpad (const const_striType stri, const intType padSize)

  {
    memSizeType striSize;
    striType result;

  /* strRpad */
    striSize = stri->size;
    if (padSize > 0 && (uintType) padSize > striSize) {
      if (unlikely((uintType) padSize > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) padSize))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) padSize;
        memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
        {
          strElemType *elem = &result->mem[striSize];
          memSizeType len = (memSizeType) padSize - striSize;

          while (len--) {
            *elem++ = (strElemType) ' ';
          } /* while */
        }
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = striSize;
      memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
    } /* if */
    return result;
  } /* strRpad */



/**
 *  Determine rightmost position of string 'searched' in 'mainStri'.
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  This function uses a modified Boyer–Moore string search algorithm.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
static intType strRPos2 (const const_striType mainStri, const const_striType searched)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_1;
    const strElemType *ch_1_pos;
    memSizeType delta;
    memSizeType charDelta[CHAR_DELTA_BEYOND + 1];
    memSizeType pos;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strRPos2 */
    main_size = mainStri->size;
    searched_size = searched->size;
    for (ch_1 = 0; ch_1 <= CHAR_DELTA_BEYOND; ch_1++) {
      charDelta[ch_1] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = searched_size - 1; pos > 0; pos--) {
      ch_1 = searched_mem[pos];
      if (ch_1 < CHAR_DELTA_BEYOND) {
        charDelta[ch_1] = pos;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = pos;
      } /* if */
    } /* for */
    ch_1 = searched_mem[0];
    ch_1_pos = memchr_strelem(&searched_mem[1], ch_1, searched_size - 1);
    if (ch_1_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memSizeType) (ch_1_pos - &searched_mem[0]);
    } /* if */
    main_mem = mainStri->mem;
    search_start = &main_mem[main_size - searched_size];
    search_end = &main_mem[-1];
    while (search_start > search_end) {
      search_start = rsearch_strelem2(search_start, ch_1, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start + 1, &searched_mem[1],
            (searched_size - 1) * sizeof(strElemType)) == 0) {
          return ((intType) (search_start - main_mem)) + 1;
        } else {
          search_start -= delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strRPos2 */



/**
 *  Determine rightmost position of string 'searched' in 'mainStri'.
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  This function calls strRPos2 if 'mainStri' is long.
 *  @return the position of 'searched' or 0 if 'mainStri'
 *          does not contain 'searched'.
 */
intType strRPos (const const_striType mainStri, const const_striType searched)

  {
    memSizeType main_size;
    memSizeType searched_size;
    strElemType ch_1;
    const strElemType *main_mem;
    const strElemType *searched_mem;
    const strElemType *search_start;
    const strElemType *search_end;

  /* strRPos */
    logFunction(printf("strRPos(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\")\n",
                       striAsUnquotedCStri(searched)););
    main_size = mainStri->size;
    searched_size = searched->size;
    if (searched_size != 0 && searched_size <= main_size) {
      if (searched_size >= BOYER_MOORE_SEARCHED_STRI_THRESHOLD &&
          main_size >= BOYER_MOORE_MAIN_STRI_THRESHOLD) {
        return strRPos2(mainStri, searched);
      } else {
        searched_mem = searched->mem;
        ch_1 = searched_mem[0];
        main_mem = mainStri->mem;
        search_start = &main_mem[main_size - searched_size];
        search_end = &main_mem[-1];
        while ((search_start = rsearch_strelem(search_start,
            ch_1, (memSizeType) (search_start - search_end))) != NULL) {
          if (memcmp(search_start, searched_mem,
              searched_size * sizeof(strElemType)) == 0) {
            return ((intType) (search_start - main_mem)) + 1;
          } else {
            search_start--;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRPos */



/**
 *  Return string with trailing whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with trailing whitespace omitted.
 */
striType strRtrim (const const_striType stri)

  {
    memSizeType striSize;
    striType result;

  /* strRtrim */
    striSize = stri->size;
    while (striSize > 0 && stri->mem[striSize - 1] <= ' ') {
      striSize--;
    } /* while */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = striSize;
      memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
      return result;
    } /* if */
  } /* strRtrim */



/**
 *  Split 'mainStri' around matches of 'delimiter' into an array of strings.
 *  The array returned by strSplit() contains each substring of
 *  'mainStri' that is terminated by another substring that is equal
 *  to the 'delimiter' or is terminated by the end of 'mainStri'.
 *  The substrings in the array are in the order in which they occur in
 *  'mainStri'. If 'delimiter' does not match any part of 'mainStri'
 *  then the resulting array has just one element, namely 'mainStri'.
 *   split("", ":")       returns  []("")
 *   split("x", ":")      returns  []("x")
 *   split(":", ":")      returns  []("", "")
 *   split("x:", ":")     returns  []("x", "")
 *   split(":x", ":")     returns  []("", "x")
 *   split("15:30", ":")  returns  []("15", "30")
 *  @return the array of strings computed by splitting 'mainStri' around
 *          matches of the given 'delimiter'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArrayType strSplit (const const_striType mainStri,
    const const_striType delimiter)

  {
    memSizeType delimiter_size;
    const strElemType *delimiter_mem;
    strElemType ch_1;
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *segment_start;
    const strElemType *search_end;
    const strElemType *found_pos;
    rtlArrayType result_array;

  /* strSplit */
    logFunction(printf("strSplit(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\")\n", striAsUnquotedCStri(delimiter)););
    if (likely(ALLOC_RTL_ARRAY(result_array, INITIAL_ARRAY_SIZE))) {
      result_array->min_position = 1;
      result_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      delimiter_size = delimiter->size;
      delimiter_mem = delimiter->mem;
      search_start = mainStri->mem;
      segment_start = search_start;
      if (delimiter_size != 0 && mainStri->size >= delimiter_size) {
        ch_1 = delimiter_mem[0];
        search_end = &mainStri->mem[mainStri->size - delimiter_size + 1];
        while ((found_pos = memchr_strelem(search_start, ch_1,
            (memSizeType) (search_end - search_start))) != NULL &&
            result_array != NULL) {
          if (memcmp(found_pos, delimiter_mem,
              delimiter_size * sizeof(strElemType)) == 0) {
            result_array = addCopiedStriToRtlArray(segment_start,
                (memSizeType) (found_pos - segment_start), result_array,
                used_max_position);
            used_max_position++;
            search_start = found_pos + delimiter_size;
            segment_start = search_start;
            if (search_start > search_end) {
              search_start = search_end;
            } /* if */
          } else {
            search_start = found_pos + 1;
          } /* if */
        } /* while */
      } /* if */
      if (likely(result_array != NULL)) {
        result_array = addCopiedStriToRtlArray(segment_start,
            (memSizeType) (&mainStri->mem[mainStri->size] - segment_start), result_array,
            used_max_position);
        used_max_position++;
        result_array = completeRtlStriArray(result_array, used_max_position);
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("strSplit -->\n"););
    return result_array;
  } /* strSplit */



#if ALLOW_STRITYPE_SLICES
/**
 *  Get a substring from a start position with a given length.
 *  The first character in a string has the position 1.
 *  This function is used by the compiler to avoid copying string data.
 *  The 'slice' is initialized to refer to the substring of 'stri'
 */
void strSubstrSlice (const const_striType stri, intType start, intType length, striType slice)

  {
    memSizeType striSize;

  /* strSubstrSlice */
    logFunction(printf("strSubstrSlice(\"%s\", " FMT_D ", " FMT_D ")",
                       striAsUnquotedCStri(stri), start, length);
                fflush(stdout););
    striSize = stri->size;
    if (unlikely(start < 1)) {
      if (length >= 1 && start > 1 - length) {
        length += start - 1;
        start = 1;
      } else {
        length = 0;
      } /* if */
    } /* if */
    SET_SLICE_CAPACITY(slice, 0);
    if (length >= 1 && (uintType) start <= striSize) {
      slice->mem = &stri->mem[start - 1];
      if ((uintType) length > striSize - (memSizeType) start + 1) {
        slice->size = striSize - (memSizeType) start + 1;
      } else {
        slice->size = (memSizeType) length;
      } /* if */
    } else {
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(slice)););
  } /* strSubstrSlice */

#endif



/**
 *  Get a substring from a start position with a given length.
 *  The first character in a string has the position 1.
 *  @return the substring from the start position with a given length.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strSubstr (const const_striType stri, intType start, intType length)

  {
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* strSubstr */
    striSize = stri->size;
    if (unlikely(start < 1)) {
      if (length >= 1 && start > 1 - length) {
        length += start - 1;
        start = 1;
      } else {
        length = 0;
      } /* if */
    } /* if */
    if (length >= 1 && (uintType) start <= striSize) {
      if ((uintType) length > striSize - (memSizeType) start + 1) {
        result_size = striSize - (memSizeType) start + 1;
      } else {
        result_size = (memSizeType) length;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      memcpy(result->mem, &stri->mem[start - 1],
             result_size * sizeof(strElemType));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } /* if */
    return result;
  } /* strSubstr */



#if ALLOW_STRITYPE_SLICES
/**
 *  Get a substring beginning at a start position.
 *  The first character in a 'string' has the position 1.
 *  This function is used by the compiler to avoid copying string data.
 *  The 'slice' is initialized to refer to the tail of 'stri'
 */
void strTailSlice (const const_striType stri, intType start, striType slice)

  {
    memSizeType striSize;

  /* strTailSlice */
    logFunction(printf("strTailSlice(\"%s\", " FMT_D ")",
                       striAsUnquotedCStri(stri), start);
                fflush(stdout););
    striSize = stri->size;
    if (unlikely(start < 1)) {
      start = 1;
    } /* if */
    if ((uintType) start <= striSize && striSize >= 1) {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = &stri->mem[start - 1];
      slice->size = striSize - (memSizeType) start + 1;
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(slice)););
  } /* strTailSlice */

#endif



/**
 *  Get a substring beginning at a start position.
 *  The first character in a 'string' has the position 1.
 *  @return the substring beginning at the start position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType strTail (const const_striType stri, intType start)

  {
    memSizeType striSize;
    memSizeType tailSize;
    striType tail;

  /* strTail */
    striSize = stri->size;
    if (unlikely(start < 1)) {
      start = 1;
    } /* if */
    if ((uintType) start <= striSize && striSize >= 1) {
      tailSize = striSize - (memSizeType) start + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(tail, tailSize))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* if using the -Ozacegilt optimisation option in the     */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(tail->mem, &stri->mem[start - 1],
             tailSize * sizeof(strElemType));
      tail->size = tailSize;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(tail, (memSizeType) 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        tail->size = 0;
      } /* if */
    } /* if */
    return tail;
  } /* strTail */



/**
 *  Convert a string to an UTF-8 encoded string of bytes.
 *  This function accepts unpaired surrogate halves.
 *   strToUtf8("\16#dc00;")  returns "\16#ed;\16#b0;\16#80;"  (surrogate halve)
 *  @param stri Normal (UTF-32) string to be converted to UTF-8.
 *  @return 'stri' converted to a string of bytes with UTF-8 encoding.
 */
striType strToUtf8 (const const_striType stri)

  {
    register strElemType *dest;
    register strElemType ch;
    register memSizeType pos;
    memSizeType result_size;
    striType resized_result;
    striType result;

  /* strToUtf8 */
    if (unlikely(stri->size > MAX_STRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_STRI_SIZE_OK(result, max_utf8_size(stri->size)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      dest = result->mem;
      for (pos = 0; pos < stri->size; pos++) {
        ch = stri->mem[pos];
        if (ch <= 0x7F) {
          *dest++ = ch;
        } else if (ch <= 0x7FF) {
          dest[0] = 0xC0 | ( ch >>  6);
          dest[1] = 0x80 | ( ch        & 0x3F);
          dest += 2;
        } else if (ch <= 0xFFFF) {
          dest[0] = 0xE0 | ( ch >> 12);
          dest[1] = 0x80 | ((ch >>  6) & 0x3F);
          dest[2] = 0x80 | ( ch        & 0x3F);
          dest += 3;
        } else if (ch <= 0x1FFFFF) {
          dest[0] = 0xF0 | ( ch >> 18);
          dest[1] = 0x80 | ((ch >> 12) & 0x3F);
          dest[2] = 0x80 | ((ch >>  6) & 0x3F);
          dest[3] = 0x80 | ( ch        & 0x3F);
          dest += 4;
        } else if (ch <= 0x3FFFFFF) {
          dest[0] = 0xF8 | ( ch >> 24);
          dest[1] = 0x80 | ((ch >> 18) & 0x3F);
          dest[2] = 0x80 | ((ch >> 12) & 0x3F);
          dest[3] = 0x80 | ((ch >>  6) & 0x3F);
          dest[4] = 0x80 | ( ch        & 0x3F);
          dest += 5;
        } else {
          dest[0] = 0xFC | ( ch >> 30);
          dest[1] = 0x80 | ((ch >> 24) & 0x3F);
          dest[2] = 0x80 | ((ch >> 18) & 0x3F);
          dest[3] = 0x80 | ((ch >> 12) & 0x3F);
          dest[4] = 0x80 | ((ch >>  6) & 0x3F);
          dest[5] = 0x80 | ( ch        & 0x3F);
          dest += 6;
        } /* if */
      } /* for */
      result_size = (memSizeType) (dest - result->mem);
      REALLOC_STRI_SIZE_SMALLER(resized_result, result, max_utf8_size(stri->size), result_size);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, max_utf8_size(stri->size));
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(max_utf8_size(stri->size), result_size);
        result->size = result_size;
      } /* if */
    } /* if */
    return result;
  } /* strToUtf8 */



/**
 *  Return string with leading and trailing whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with leading and trailing whitespace omitted.
 */
striType strTrim (const const_striType stri)

  {
    memSizeType start = 0;
    memSizeType striSize;
    striType result;

  /* strTrim */
    striSize = stri->size;
    if (striSize >= 1) {
      while (start < striSize && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      while (striSize > start && stri->mem[striSize - 1] <= ' ') {
        striSize--;
      } /* while */
      striSize -= start;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = striSize;
      memcpy(result->mem, &stri->mem[start], striSize * sizeof(strElemType));
      return result;
    } /* if */
  } /* strTrim */



/**
 *  Convert a string to upper case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the string converted to upper case.
 */
striType strUp (const const_striType stri)

  {
    memSizeType striSize;
    striType result;

  /* strUp */
    logFunction(printf("strUp(\"%s\")",
                       striAsUnquotedCStri(stri));
                fflush(stdout););
    striSize = stri->size;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = striSize;
      toUpper(stri->mem, stri->size, result->mem);
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* strUp */



/**
 *  Convert a string to upper case.
 *  StrUpTemp is used by the compiler if 'stri' is temporary
 *  value that can be reused.
 *  @return the string converted to lower case.
 */
striType strUpTemp (const striType stri)

  { /* strUpTemp */
    logFunction(printf("strUpTemp(\"%s\")",
                       striAsUnquotedCStri(stri));
                fflush(stdout););
    toUpper(stri->mem, stri->size, stri->mem);
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(stri)););
    return stri;
  } /* strUpTemp */



/**
 *  Convert a string with bytes in UTF-8 encoding to UTF-32.
 *  This function accepts overlong encodings and unpaired surrogate halves.
 *   strUtf8ToStri("\16#c0;\16#80;")         returns "\0;"        (overlong encoding)
 *   strUtf8ToStri("\16#ed;\16#b0;\16#80;")  returns "\16#dc00;"  (surrogate halve)
 *  @param utf8 String of bytes encoded with UTF-8.
 *  @return 'utf8' converted to a normal (UTF-32) string.
 *  @exception RANGE_ERROR If characters beyond '\255;' are present or
 *                         if 'utf8' is not encoded with UTF-8.
 */
striType strUtf8ToStri (const const_striType utf8)

  {
    memSizeType utf8Size;
    memSizeType pos;
    const strElemType *utf8ptr;
    boolType okay = TRUE;
    striType resized_result;
    striType result;

  /* strUtf8ToStri */
    logFunction(printf("strUtf8ToStri(\"%s\")\n",
                       striAsUnquotedCStri(utf8)););
    utf8Size = utf8->size;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, utf8Size))) {
      raise_error(MEMORY_ERROR);
    } else {
      utf8ptr = &utf8->mem[0];
      pos = 0;
      for (; utf8Size > 0; pos++, utf8Size--) {
        if (*utf8ptr <= 0x7F) {
          result->mem[pos] = *utf8ptr++;
        } else if (utf8ptr[0] >= 0xC0 && utf8ptr[0] <= 0xDF && utf8Size >= 2 &&
                   utf8ptr[1] >= 0x80 && utf8ptr[1] <= 0xBF) {
          /* utf8ptr[0]   range 192 to 223 (leading bits 110.....) */
          /* utf8ptr[1]   range 128 to 191 (leading bits 10......) */
          result->mem[pos] = (utf8ptr[0] & 0x1F) << 6 |
                             (utf8ptr[1] & 0x3F);
          utf8ptr += 2;
          utf8Size--;
        } else if (utf8ptr[0] >= 0xE0 && utf8ptr[0] <= 0xEF && utf8Size >= 3 &&
                   utf8ptr[1] >= 0x80 && utf8ptr[1] <= 0xBF &&
                   utf8ptr[2] >= 0x80 && utf8ptr[2] <= 0xBF) {
          /* utf8ptr[0]   range 224 to 239 (leading bits 1110....) */
          /* utf8ptr[1..] range 128 to 191 (leading bits 10......) */
          result->mem[pos] = (utf8ptr[0] & 0x0F) << 12 |
                             (utf8ptr[1] & 0x3F) <<  6 |
                             (utf8ptr[2] & 0x3F);
          utf8ptr += 3;
          utf8Size -= 2;
        } else if (utf8ptr[0] >= 0xF0 && utf8ptr[0] <= 0xF7 && utf8Size >= 4 &&
                   utf8ptr[1] >= 0x80 && utf8ptr[1] <= 0xBF &&
                   utf8ptr[2] >= 0x80 && utf8ptr[2] <= 0xBF &&
                   utf8ptr[3] >= 0x80 && utf8ptr[3] <= 0xBF) {
          /* utf8ptr[0]   range 240 to 247 (leading bits 11110...) */
          /* utf8ptr[1..] range 128 to 191 (leading bits 10......) */
          result->mem[pos] = (utf8ptr[0] & 0x07) << 18 |
                             (utf8ptr[1] & 0x3F) << 12 |
                             (utf8ptr[2] & 0x3F) <<  6 |
                             (utf8ptr[3] & 0x3F);
          utf8ptr += 4;
          utf8Size -= 3;
        } else if (utf8ptr[0] >= 0xF8 && utf8ptr[0] <= 0xFB && utf8Size >= 5 &&
                   utf8ptr[1] >= 0x80 && utf8ptr[1] <= 0xBF &&
                   utf8ptr[2] >= 0x80 && utf8ptr[2] <= 0xBF &&
                   utf8ptr[3] >= 0x80 && utf8ptr[3] <= 0xBF &&
                   utf8ptr[4] >= 0x80 && utf8ptr[4] <= 0xBF) {
          /* utf8ptr[0]   range 248 to 251 (leading bits 111110..) */
          /* utf8ptr[1..] range 128 to 191 (leading bits 10......) */
          result->mem[pos] = (utf8ptr[0] & 0x03) << 24 |
                             (utf8ptr[1] & 0x3F) << 18 |
                             (utf8ptr[2] & 0x3F) << 12 |
                             (utf8ptr[3] & 0x3F) <<  6 |
                             (utf8ptr[4] & 0x3F);
          utf8ptr += 5;
          utf8Size -= 4;
        } else if (utf8ptr[0] >= 0xFC && utf8ptr[0] <= 0xFF && utf8Size >= 6 &&
                   utf8ptr[1] >= 0x80 && utf8ptr[1] <= 0xBF &&
                   utf8ptr[2] >= 0x80 && utf8ptr[2] <= 0xBF &&
                   utf8ptr[3] >= 0x80 && utf8ptr[3] <= 0xBF &&
                   utf8ptr[4] >= 0x80 && utf8ptr[4] <= 0xBF &&
                   utf8ptr[5] >= 0x80 && utf8ptr[5] <= 0xBF) {
          /* utf8ptr[0]   range 252 to 255 (leading bits 111111..) */
          /* utf8ptr[1..] range 128 to 191 (leading bits 10......) */
          result->mem[pos] = (utf8ptr[0] & 0x03) << 30 |
                             (utf8ptr[1] & 0x3F) << 24 |
                             (utf8ptr[2] & 0x3F) << 18 |
                             (utf8ptr[3] & 0x3F) << 12 |
                             (utf8ptr[4] & 0x3F) <<  6 |
                             (utf8ptr[5] & 0x3F);
          utf8ptr += 6;
          utf8Size -= 5;
        } else {
          /* utf8ptr[0] not in range 0xC0 to 0xFF (192 to 255) */
          /* or not enough continuation bytes found.           */
          logError(printf("strUtf8ToStri: "
                          "Invalid byte sequence starting at position "
                          FMT_U_MEM ": \"\\" FMT_U32 ";\\ ...\".\n",
                          (memSizeType) (utf8ptr - &utf8->mem[0]),
                          utf8ptr[0]););
          okay = FALSE;
          utf8Size = 1;
        } /* if */
      } /* for */
      if (likely(okay)) {
        result->size = pos;
        if (pos != utf8->size) {
          REALLOC_STRI_SIZE_SMALLER(resized_result, result, utf8->size, pos);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, utf8->size);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            COUNT3_STRI(utf8->size, pos);
          } /* if */
        } /* if */
      } else {
        FREE_STRI(result, utf8->size);
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    logFunction(printf("strUtf8ToStri --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* strUtf8ToStri */



/**
 *  String multiplication of the character '\0'.
 *  The character '\0' is concatenated to itself such that in total
 *  'factor' characters are concatenated.
 *  @return the result of the string multiplication.
 *  @exception RANGE_ERROR If the factor is negative.
 */
striType strZero (const intType factor)

  {
    striType result;

  /* strZero */
    logFunction(printf("strZero(" FMT_D ")\n", factor););
    if (unlikely(factor < 0)) {
      logError(printf("strZero(" FMT_D "): Negative factor.\n",
                      factor););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely((uintType) factor > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) factor))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = (memSizeType) factor;
        memset(result->mem, 0, (memSizeType) factor * sizeof(strElemType));
      } /* if */
    } /* if */
    return result;
  } /* strZero */
