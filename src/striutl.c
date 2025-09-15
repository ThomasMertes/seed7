/********************************************************************/
/*                                                                  */
/*  striutl.c     Functions to work with wide char strings.         */
/*  Copyright (C) 1989 - 2021  Thomas Mertes                        */
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
/*  File: seed7/src/striutl.c                                       */
/*  Changes: 1991 - 1994, 2005 - 2021  Thomas Mertes                */
/*  Content: Functions to work with wide char strings.              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif

#include "common.h"
#include "heaputl.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "striutl.h"


const const_cstriType stri_escape_sequence[] = {
    "\\0;",  "\\1;",  "\\2;",  "\\3;",  "\\4;",
    "\\5;",  "\\6;",  "\\a",   "\\b",   "\\t",
    "\\n",   "\\v",   "\\f",   "\\r",   "\\14;",
    "\\15;", "\\16;", "\\17;", "\\18;", "\\19;",
    "\\20;", "\\21;", "\\22;", "\\23;", "\\24;",
    "\\25;", "\\26;", "\\e",   "\\28;", "\\29;",
    "\\30;", "\\31;"};

const const_cstriType cstri_escape_sequence[] = {
    "\\000", "\\001", "\\002", "\\003", "\\004",
    "\\005", "\\006", "\\007", "\\b",   "\\t",
    "\\n",   "\\013", "\\f",   "\\r",   "\\016",
    "\\017", "\\020", "\\021", "\\022", "\\023",
    "\\024", "\\025", "\\026", "\\027", "\\030",
    "\\031", "\\032", "\\033", "\\034", "\\035",
    "\\036", "\\037"};

static const char null_string_marker[]      = "\\ *NULL_STRING* ";
static const char null_byte_string_marker[] = "\\ *NULL_BYTE_STRING* ";

#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#if EMULATE_ROOT_CWD
const_os_striType current_emulated_cwd = NULL;
#endif
const os_charType emulated_root[] = {'/', '\0'};
#endif

#define USE_DUFFS_UNROLLING 1
#define STACK_ALLOC_SIZE    1000

#ifdef OS_STRI_WCHAR

#define MAX_OS_STRI_SIZE    (((MAX_MEMSIZETYPE / sizeof(os_charType)) - NULL_TERMINATION_LEN) / SURROGATE_PAIR_FACTOR)
#define MAX_OS_BSTRI_SIZE   (((MAX_BSTRI_LEN / sizeof(os_charType)) - NULL_TERMINATION_LEN) / SURROGATE_PAIR_FACTOR)
#define OS_STRI_SIZE(size)  ((size) * SURROGATE_PAIR_FACTOR)
#define OS_BSTRI_SIZE(size) (((size) * SURROGATE_PAIR_FACTOR + NULL_TERMINATION_LEN) * sizeof(os_charType))

#elif defined OS_STRI_USES_CODE_PAGE

#define MAX_OS_STRI_SIZE    ((MAX_MEMSIZETYPE / sizeof(os_charType)) - NULL_TERMINATION_LEN)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN / sizeof(os_charType)) - NULL_TERMINATION_LEN)
#define OS_STRI_SIZE(size)  (size)
#define OS_BSTRI_SIZE(size) (((size) + NULL_TERMINATION_LEN) * sizeof(os_charType))
int code_page = DEFAULT_CODE_PAGE;

#elif defined OS_STRI_UTF8

#define MAX_OS_STRI_SIZE    (MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN - NULL_TERMINATION_LEN) / MAX_UTF8_EXPANSION_FACTOR)
#define OS_STRI_SIZE(size)  max_utf8_size(size)
#define OS_BSTRI_SIZE(size) (max_utf8_size(size) + NULL_TERMINATION_LEN)

#else

#define MAX_OS_STRI_SIZE    ((MAX_MEMSIZETYPE / sizeof(os_charType)) - NULL_TERMINATION_LEN)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN / sizeof(os_charType)) - NULL_TERMINATION_LEN)
#define OS_STRI_SIZE(size)  (size)
#define OS_BSTRI_SIZE(size) (((size) + NULL_TERMINATION_LEN) * sizeof(os_charType))

#endif



/**
 *  Write a Seed7 string to a static C string buffer.
 *  This function is intended to do debug/log output with printf().
 *  Control chars and chars beyond ASCII are written with Seed7
 *  escape sequences. Strings longer than the AND_SO_ON_LIMIT are
 *  truncated and an information about the total length is added.
 *  Since a static buffer is used every call of printf() can only
 *  work correctly with one call of striAsUnquotedCStri(). Because
 *  of the static buffer this function is not thread safe.
 *  @param stri String to be written into the static buffer.
 *  @return a pointer to the static buffer containing the string.
 */
cstriType striAsUnquotedCStri (const const_striType stri)

  {
    memSizeType size;
    strElemType ch;
    memSizeType idx;
    memSizeType pos = 0;
    static char buffer[AND_SO_ON_LIMIT * MAXIMUM_UTF32_ESCAPE_WIDTH +
                       AND_SO_ON_LENGTH];

  /* striAsUnquotedCStri */
    if (stri != NULL) {
      size = stri->size;
      if (size > AND_SO_ON_LIMIT) {
        size = AND_SO_ON_LIMIT;
      } /* if */
      for (idx = 0; idx < size; idx++) {
        ch = stri->mem[idx];
        if (ch < 127) {
          if (ch < ' ') {
            buffer[pos] = '\\';
            if (stri_escape_sequence[ch][1] <= '9') {
              /* Numeric escape sequence with one or two digits. */
              if (ch <= 9) {
                buffer[pos + 1] = (char) (ch + '0');
                buffer[pos + 2] = ';';
                pos += 3;
              } else {
                buffer[pos + 1] = stri_escape_sequence[ch][1];
                buffer[pos + 2] = stri_escape_sequence[ch][2];
                buffer[pos + 3] = ';';
                pos += 4;
              } /* if */
            } else {
              /* Character escape sequence. */
              buffer[pos + 1] = stri_escape_sequence[ch][1];
              pos += 2;
            } /* if */
          } else if (ch == (charType) '\\') {
            memcpy(&buffer[pos], "\\\\", 2);
            pos += 2;
          } else if (ch == (charType) '\"') {
            memcpy(&buffer[pos], "\\\"", 2);
            pos += 2;
          } else {
            buffer[pos] = (char) ch;
            pos++;
          } /* if */
        } else if (ch == (charType) -1) {
          memcpy(&buffer[pos], "\\-1;", 4);
          pos += 4;
        } else {
          pos += (memSizeType) sprintf(&buffer[pos], "\\%lu;", (unsigned long) ch);
        } /* if */
      } /* for */
      if (stri->size > AND_SO_ON_LIMIT) {
        pos += (memSizeType) sprintf(&buffer[pos], AND_SO_ON_TEXT FMT_U_MEM, stri->size);
      } /* if */
    } else {
      MEMCPY_STRING(buffer, null_string_marker);
      pos = STRLEN(null_string_marker);
    } /* if */
    buffer[pos] = '\0';
    return buffer;
  } /* striAsUnquotedCStri */



#if LOG_FUNCTIONS || LOG_FUNCTIONS_EVERYWHERE
static cstriType striCharsAsUnquotedCStri (const strElemType *const striChars,
    memSizeType striSize)

  {
    striRecord striSlice;

  /* striCharsAsUnquotedCStri */
#if ALLOW_STRITYPE_SLICES
    striSlice.size = striSize;
    striSlice.mem = (strElemType *) striChars;
    return striAsUnquotedCStri(&striSlice);
#else
    return "\\ STRING_WITHOUT_DETAILS \\";
#endif
  } /* striCharsAsUnquotedCStri */
#endif



/**
 *  Write a Seed7 bstring to a static C string buffer.
 *  This function is intended to do debug/log output with printf().
 *  Control chars and chars beyond ASCII are written with Seed7
 *  escape sequences. Strings longer than the AND_SO_ON_LIMIT are
 *  truncated and an information about the total length is added.
 *  Since a static buffer is used every call of printf() can only
 *  work correctly with one call of bstriAsUnquotedCStri(). Because
 *  of the static buffer this function is not thread safe.
 *  @param bstri Bstring to be written into the static buffer.
 *  @return a pointer to the static buffer containing the string.
 */
cstriType bstriAsUnquotedCStri (const const_bstriType bstri)

  {
    memSizeType size;
    ucharType ch;
    memSizeType idx;
    memSizeType pos = 0;
    memSizeType len;
    static char buffer[AND_SO_ON_LIMIT * MAXIMUM_BYTE_ESCAPE_WIDTH +
                       AND_SO_ON_LENGTH];

  /* bstriAsUnquotedCStri */
    if (bstri != NULL) {
      size = bstri->size;
      if (size > AND_SO_ON_LIMIT) {
        size = AND_SO_ON_LIMIT;
      } /* if */
      for (idx = 0; idx < size; idx++) {
        ch = bstri->mem[idx];
        if (ch < 127) {
          if (ch < ' ') {
            len = strlen(stri_escape_sequence[ch]);
            memcpy(&buffer[pos], stri_escape_sequence[ch], len);
            pos += len;
          } else if (ch == '\\') {
            memcpy(&buffer[pos], "\\\\", 2);
            pos += 2;
          } else if (ch == '\"') {
            memcpy(&buffer[pos], "\\\"", 2);
            pos += 2;
          } else {
            buffer[pos] = (char) ch;
            pos++;
          } /* if */
        } else {
          pos += (memSizeType) sprintf(&buffer[pos], "\\%u;", (unsigned int) ch);
        } /* if */
      } /* for */
      if (bstri->size > AND_SO_ON_LIMIT) {
        pos += (memSizeType) sprintf(&buffer[pos], AND_SO_ON_TEXT FMT_U_MEM, bstri->size);
      } /* if */
    } else {
      MEMCPY_STRING(buffer, null_byte_string_marker);
      pos = STRLEN(null_byte_string_marker);
    } /* if */
    buffer[pos] = '\0';
    return buffer;
  } /* bstriAsUnquotedCStri */



#if !STRINGIFY_WORKS
cstriType stringify (intType number)

  {
    static char buffer[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];

  /* stringify */
    sprintf(buffer, FMT_D, number);
    return buffer;
  } /* stringify */
#endif



#if USE_DUFFS_UNROLLING
/**
 *  Copy len bytes to Seed7 characters in a string.
 *  This function works also correct if 'src' and 'dest' point
 *  to the same address. In other words it works correct for:
 *    memcpy_to_strelem(mem, (ustriType) mem, num);
 *  This function uses loop unrolling inspired by Duff's device.
 *  @param dest Destination array with UTF-32 encoded characters.
 *  @param src Source array with ISO-8859-1 encoded bytes.
 *  @param len Number of bytes in 'src' and UTF-32 characters in 'dest'.
 */
void memcpy_to_strelem (register strElemType *const dest,
    register const const_ustriType src, memSizeType len)

  {
    register memSizeType pos;

  /* memcpy_to_strelem */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: dest[pos -  1] = src[pos -  1];
          case 31: dest[pos -  2] = src[pos -  2];
          case 30: dest[pos -  3] = src[pos -  3];
          case 29: dest[pos -  4] = src[pos -  4];
          case 28: dest[pos -  5] = src[pos -  5];
          case 27: dest[pos -  6] = src[pos -  6];
          case 26: dest[pos -  7] = src[pos -  7];
          case 25: dest[pos -  8] = src[pos -  8];
          case 24: dest[pos -  9] = src[pos -  9];
          case 23: dest[pos - 10] = src[pos - 10];
          case 22: dest[pos - 11] = src[pos - 11];
          case 21: dest[pos - 12] = src[pos - 12];
          case 20: dest[pos - 13] = src[pos - 13];
          case 19: dest[pos - 14] = src[pos - 14];
          case 18: dest[pos - 15] = src[pos - 15];
          case 17: dest[pos - 16] = src[pos - 16];
          case 16: dest[pos - 17] = src[pos - 17];
          case 15: dest[pos - 18] = src[pos - 18];
          case 14: dest[pos - 19] = src[pos - 19];
          case 13: dest[pos - 20] = src[pos - 20];
          case 12: dest[pos - 21] = src[pos - 21];
          case 11: dest[pos - 22] = src[pos - 22];
          case 10: dest[pos - 23] = src[pos - 23];
          case  9: dest[pos - 24] = src[pos - 24];
          case  8: dest[pos - 25] = src[pos - 25];
          case  7: dest[pos - 26] = src[pos - 26];
          case  6: dest[pos - 27] = src[pos - 27];
          case  5: dest[pos - 28] = src[pos - 28];
          case  4: dest[pos - 29] = src[pos - 29];
          case  3: dest[pos - 30] = src[pos - 30];
          case  2: dest[pos - 31] = src[pos - 31];
          case  1: dest[pos - 32] = src[pos - 32];
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
  } /* memcpy_to_strelem */



/**
 *  Fill an array of len Seed7 characters with the character ch.
 *  This function uses loop unrolling inspired by Duff's device.
 *  Up to a length of 6 a simple loop is faster than calling
 *  this function. With a length of 7 a simple loop is as fast
 *  as calling this function.
 *  The use of indices relative to pos allows the C compiler
 *  to do more optimizations.
 *  @param dest Destination array with UTF-32 encoded characters.
 *  @param ch UTF-32 encoded character to be filled into 'dest'.
 *  @param len Specifies how often 'ch' is filled into 'dest'.
 */
void memset_to_strelem (register strElemType *const dest,
    register const strElemType ch, memSizeType len)

  {
    register memSizeType pos;

  /* memset_to_strelem */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: dest[pos -  1] = ch;
          case 31: dest[pos -  2] = ch;
          case 30: dest[pos -  3] = ch;
          case 29: dest[pos -  4] = ch;
          case 28: dest[pos -  5] = ch;
          case 27: dest[pos -  6] = ch;
          case 26: dest[pos -  7] = ch;
          case 25: dest[pos -  8] = ch;
          case 24: dest[pos -  9] = ch;
          case 23: dest[pos - 10] = ch;
          case 22: dest[pos - 11] = ch;
          case 21: dest[pos - 12] = ch;
          case 20: dest[pos - 13] = ch;
          case 19: dest[pos - 14] = ch;
          case 18: dest[pos - 15] = ch;
          case 17: dest[pos - 16] = ch;
          case 16: dest[pos - 17] = ch;
          case 15: dest[pos - 18] = ch;
          case 14: dest[pos - 19] = ch;
          case 13: dest[pos - 20] = ch;
          case 12: dest[pos - 21] = ch;
          case 11: dest[pos - 22] = ch;
          case 10: dest[pos - 23] = ch;
          case  9: dest[pos - 24] = ch;
          case  8: dest[pos - 25] = ch;
          case  7: dest[pos - 26] = ch;
          case  6: dest[pos - 27] = ch;
          case  5: dest[pos - 28] = ch;
          case  4: dest[pos - 29] = ch;
          case  3: dest[pos - 30] = ch;
          case  2: dest[pos - 31] = ch;
          case  1: dest[pos - 32] = ch;
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
  } /* memset_to_strelem */



/**
 *  Copy len Seed7 characters to a byte string.
 *  This function uses loop unrolling inspired by Duff's device
 *  and a trick with a binary or (|=) to check for allowed values.
 *  @param dest Destination array with ISO-8859-1 encoded bytes.
 *  @param src Source array with UTF-32 encoded characters.
 *  @param len Number of UTF-32 characters in 'src' and bytes in 'dest'.
 *  @return TRUE if one of the characters does not fit into a byte,
 *          FALSE otherwise.
 */
boolType memcpy_from_strelem (register const ustriType dest,
    register const strElemType *const src, memSizeType len)

  {
    register memSizeType pos;
    register strElemType check = 0;

  /* memcpy_from_strelem */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: check |= src[pos -  1]; dest[pos -  1] = (ucharType) src[pos -  1];
          case 31: check |= src[pos -  2]; dest[pos -  2] = (ucharType) src[pos -  2];
          case 30: check |= src[pos -  3]; dest[pos -  3] = (ucharType) src[pos -  3];
          case 29: check |= src[pos -  4]; dest[pos -  4] = (ucharType) src[pos -  4];
          case 28: check |= src[pos -  5]; dest[pos -  5] = (ucharType) src[pos -  5];
          case 27: check |= src[pos -  6]; dest[pos -  6] = (ucharType) src[pos -  6];
          case 26: check |= src[pos -  7]; dest[pos -  7] = (ucharType) src[pos -  7];
          case 25: check |= src[pos -  8]; dest[pos -  8] = (ucharType) src[pos -  8];
          case 24: check |= src[pos -  9]; dest[pos -  9] = (ucharType) src[pos -  9];
          case 23: check |= src[pos - 10]; dest[pos - 10] = (ucharType) src[pos - 10];
          case 22: check |= src[pos - 11]; dest[pos - 11] = (ucharType) src[pos - 11];
          case 21: check |= src[pos - 12]; dest[pos - 12] = (ucharType) src[pos - 12];
          case 20: check |= src[pos - 13]; dest[pos - 13] = (ucharType) src[pos - 13];
          case 19: check |= src[pos - 14]; dest[pos - 14] = (ucharType) src[pos - 14];
          case 18: check |= src[pos - 15]; dest[pos - 15] = (ucharType) src[pos - 15];
          case 17: check |= src[pos - 16]; dest[pos - 16] = (ucharType) src[pos - 16];
          case 16: check |= src[pos - 17]; dest[pos - 17] = (ucharType) src[pos - 17];
          case 15: check |= src[pos - 18]; dest[pos - 18] = (ucharType) src[pos - 18];
          case 14: check |= src[pos - 19]; dest[pos - 19] = (ucharType) src[pos - 19];
          case 13: check |= src[pos - 20]; dest[pos - 20] = (ucharType) src[pos - 20];
          case 12: check |= src[pos - 21]; dest[pos - 21] = (ucharType) src[pos - 21];
          case 11: check |= src[pos - 22]; dest[pos - 22] = (ucharType) src[pos - 22];
          case 10: check |= src[pos - 23]; dest[pos - 23] = (ucharType) src[pos - 23];
          case  9: check |= src[pos - 24]; dest[pos - 24] = (ucharType) src[pos - 24];
          case  8: check |= src[pos - 25]; dest[pos - 25] = (ucharType) src[pos - 25];
          case  7: check |= src[pos - 26]; dest[pos - 26] = (ucharType) src[pos - 26];
          case  6: check |= src[pos - 27]; dest[pos - 27] = (ucharType) src[pos - 27];
          case  5: check |= src[pos - 28]; dest[pos - 28] = (ucharType) src[pos - 28];
          case  4: check |= src[pos - 29]; dest[pos - 29] = (ucharType) src[pos - 29];
          case  3: check |= src[pos - 30]; dest[pos - 30] = (ucharType) src[pos - 30];
          case  2: check |= src[pos - 31]; dest[pos - 31] = (ucharType) src[pos - 31];
          case  1: check |= src[pos - 32]; dest[pos - 32] = (ucharType) src[pos - 32];
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
    return check >= 256;
  } /* memcpy_from_strelem */



#if !HAS_WMEMCHR || WCHAR_T_SIZE != 32
/**
 *  Scan the first len Seed7 characters for the character ch.
 *  This function uses loop unrolling inspired by Duff's device.
 *  @param mem Array with UTF-32 characters.
 *  @param ch UTF-32 character to be searched in 'mem'.
 *  @param len Number of UTF-32 characters in 'mem'.
 *  @return a pointer to the matching character, or NULL if the
 *          character does not occur in the given string area.
 */
const strElemType *memchr_strelem (register const strElemType *mem,
    const strElemType ch, memSizeType len)

  {
    register memSizeType blockCount;

  /* memchr_strelem */
    if (len != 0) {
      blockCount = (len + 31) >> 5;
      switch (len & 31) {
        do {
          case  0: if (unlikely(*mem == ch)) return mem; mem++;
          case 31: if (unlikely(*mem == ch)) return mem; mem++;
          case 30: if (unlikely(*mem == ch)) return mem; mem++;
          case 29: if (unlikely(*mem == ch)) return mem; mem++;
          case 28: if (unlikely(*mem == ch)) return mem; mem++;
          case 27: if (unlikely(*mem == ch)) return mem; mem++;
          case 26: if (unlikely(*mem == ch)) return mem; mem++;
          case 25: if (unlikely(*mem == ch)) return mem; mem++;
          case 24: if (unlikely(*mem == ch)) return mem; mem++;
          case 23: if (unlikely(*mem == ch)) return mem; mem++;
          case 22: if (unlikely(*mem == ch)) return mem; mem++;
          case 21: if (unlikely(*mem == ch)) return mem; mem++;
          case 20: if (unlikely(*mem == ch)) return mem; mem++;
          case 19: if (unlikely(*mem == ch)) return mem; mem++;
          case 18: if (unlikely(*mem == ch)) return mem; mem++;
          case 17: if (unlikely(*mem == ch)) return mem; mem++;
          case 16: if (unlikely(*mem == ch)) return mem; mem++;
          case 15: if (unlikely(*mem == ch)) return mem; mem++;
          case 14: if (unlikely(*mem == ch)) return mem; mem++;
          case 13: if (unlikely(*mem == ch)) return mem; mem++;
          case 12: if (unlikely(*mem == ch)) return mem; mem++;
          case 11: if (unlikely(*mem == ch)) return mem; mem++;
          case 10: if (unlikely(*mem == ch)) return mem; mem++;
          case  9: if (unlikely(*mem == ch)) return mem; mem++;
          case  8: if (unlikely(*mem == ch)) return mem; mem++;
          case  7: if (unlikely(*mem == ch)) return mem; mem++;
          case  6: if (unlikely(*mem == ch)) return mem; mem++;
          case  5: if (unlikely(*mem == ch)) return mem; mem++;
          case  4: if (unlikely(*mem == ch)) return mem; mem++;
          case  3: if (unlikely(*mem == ch)) return mem; mem++;
          case  2: if (unlikely(*mem == ch)) return mem; mem++;
          case  1: if (unlikely(*mem == ch)) return mem; mem++;
        } while (--blockCount > 0);
      } /* switch */
    } /* if */
    return NULL;
  } /* memchr_strelem */

#endif
#else



/**
 *  Copy len bytes to Seed7 characters in a string.
 *  This function works also correct if 'src' and 'dest' point
 *  to the same address. In other words it works correct for:
 *    memcpy_to_strelem(mem, (ustriType) mem, num);
 *  @param dest Destination array with UTF-32 encoded characters.
 *  @param src Source array with ISO-8859-1 encoded bytes.
 *  @param len Number of bytes in 'src' and UTF-32 characters in 'dest'.
 */
void memcpy_to_strelem (register strElemType *const dest,
    register const const_ustriType src, memSizeType len)

  { /* memcpy_to_strelem */
    while (len != 0) {
      len--;
      dest[len] = src[len];
    } /* while */
  } /* memcpy_to_strelem */



/**
 *  Fill len Seed7 characters with the character ch.
 *  @param dest Destination array with UTF-32 encoded characters.
 *  @param ch UTF-32 encoded character to be filled into 'dest'.
 *  @param len Specifies how often 'ch' is filled into 'dest'.
 */
void memset_to_strelem (register strElemType *const dest,
    register const strElemType ch, memSizeType len)

  { /* memset_to_strelem */
    while (len != 0) { \
      len--;
      dest[len] = (strElemType) ch; \
    } /* while */
  } /* memset_to_strelem */



/**
 *  Copy len Seed7 characters to a byte string.
 *  @param dest Destination array with ISO-8859-1 encoded bytes.
 *  @param src Source array with UTF-32 encoded characters.
 *  @param len Number of UTF-32 characters in 'src' and bytes in 'dest'.
 *  @return TRUE if one of the characters does not fit into a byte,
 *          FALSE otherwise.
 */
boolType memcpy_from_strelem (register const ustriType dest,
    register const strElemType *const src, memSizeType len)

  {
    register strElemType check = 0;

  /* memcpy_from_strelem */
    while (len != 0) {
      len--;
      check |= src[len]
      dest[len] = src[len];
    } /* while */
    return check >= 256;
  } /* memcpy_from_strelem */



#if !HAS_WMEMCHR || WCHAR_T_SIZE != 32
/**
 *  Scan the first len Seed7 characters for the character ch.
 *  @param mem Array with UTF-32 characters.
 *  @param ch UTF-32 character to be searched in 'mem'.
 *  @param len Number of UTF-32 characters in 'mem'.
 *  @return a pointer to the matching character, or NULL if the
 *          character does not occur in the given string area.
 */
const strElemType *memchr_strelem (register const strElemType *mem,
    const strElemType ch, memSizeType len)

  { /* memchr_strelem */
    for (; len > 0; mem++, len--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* memchr_strelem */

#endif
#endif



#if STACK_LIKE_ALLOC_FOR_OS_STRI
os_striType heapAllocOsStri (memSizeType len)

  {
    memSizeType size;
    stackAllocType new_stack_alloc;
    os_striType var;

  /* heapAllocOsStri */
    logFunction(printf("heapAllocOsStri(" FMT_U_MEM ")\n", len););
    if (len < STACK_ALLOC_SIZE) {
      size = STACK_ALLOC_SIZE;
    } else {
      size = len;
    } /* if */
    if (unlikely(size > MAX_STACK_ALLOC ||
        !ALLOC_HEAP(new_stack_alloc, stackAllocType, SIZ_STACK_ALLOC(size)))) {
      var = NULL;
    } else {
      /* printf("new_stack_alloc=%08lx\n", new_stack_alloc); */
      if (stack_alloc->curr_free == stack_alloc->start) {
        new_stack_alloc->previous = stack_alloc->previous;
        free(stack_alloc);
      } else {
        new_stack_alloc->previous = stack_alloc;
      } /* if */
      /* printf("previous=%08lx\n", new_stack_alloc->previous); */
      new_stack_alloc->beyond = &new_stack_alloc->start[SIZ_OS_STRI(size)];
      /* printf("beyond=%08lx\n", new_stack_alloc->beyond); */
      new_stack_alloc->curr_free = new_stack_alloc->start;
      stack_alloc = new_stack_alloc;
      (void) POP_OS_STRI(var, SIZ_OS_STRI(len));
    } /* if */
    logFunction(printf("heapAllocOsStri(" FMT_U_MEM ") --> " FMT_X_MEM "\n",
                       len, (memSizeType) var););
    return var;
  } /* heapAllocOsStri */



void heapFreeOsStri (const_os_striType var)

  {
    stackAllocType old_stack_alloc;

  /* heapFreeOsStri */
    logFunction(printf("heapFreeOsStri(" FMT_X_MEM ")\n",
                       (memSizeType) var););
    old_stack_alloc = stack_alloc;
    stack_alloc = old_stack_alloc->previous;
    free(old_stack_alloc);
  } /* heapFreeOsStri */
#endif



/**
 *  Convert an UTF-8 encoded string to an UTF-32 encoded string.
 *  The memory for the destination dest_stri is not allocated.
 *  @param dest_stri Destination of the UTF-32 encoded string.
 *  @param dest_len Place to return the length of dest_stri.
 *  @param stri8 UTF-8 encoded string to be converted.
 *  @param len Number of bytes in stri8.
 *  @return the number of bytes in stri8 that are left unconverted, or
 *          0 if stri8 has been successfully converted.
 */
memSizeType stri8_to_stri (strElemType *const dest_stri,
    memSizeType *const dest_len, const strElemType *stri8, memSizeType len)

  {
    strElemType *stri;

  /* stri8_to_stri */
    stri = dest_stri;
    for (; len > 0; len--) {
      if (*stri8 <= 0x7F) {
        *stri++ = (strElemType) *stri8++;
      } else if (stri8[0] >= 0xC0 && stri8[0] <= 0xDF && len >= 2 &&
                 stri8[1] >= 0x80 && stri8[1] <= 0xBF) {
        /* stri8[0]   range 192 to 223 (leading bits 110.....) */
        /* stri8[1]   range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (stri8[0] & 0x1F) << 6 |
                  (strElemType) (stri8[1] & 0x3F);
        stri8 += 2;
        len--;
      } else if (stri8[0] >= 0xE0 && stri8[0] <= 0xEF && len >= 3 &&
                 stri8[1] >= 0x80 && stri8[1] <= 0xBF &&
                 stri8[2] >= 0x80 && stri8[2] <= 0xBF) {
        /* stri8[0]   range 224 to 239 (leading bits 1110....) */
        /* stri8[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (stri8[0] & 0x0F) << 12 |
                  (strElemType) (stri8[1] & 0x3F) <<  6 |
                  (strElemType) (stri8[2] & 0x3F);
        stri8 += 3;
        len -= 2;
      } else if (stri8[0] >= 0xF0 && stri8[0] <= 0xF7 && len >= 4 &&
                 stri8[1] >= 0x80 && stri8[1] <= 0xBF &&
                 stri8[2] >= 0x80 && stri8[2] <= 0xBF &&
                 stri8[3] >= 0x80 && stri8[3] <= 0xBF) {
        /* stri8[0]   range 240 to 247 (leading bits 11110...) */
        /* stri8[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (stri8[0] & 0x07) << 18 |
                  (strElemType) (stri8[1] & 0x3F) << 12 |
                  (strElemType) (stri8[2] & 0x3F) <<  6 |
                  (strElemType) (stri8[3] & 0x3F);
        stri8 += 4;
        len -= 3;
      } else if (stri8[0] >= 0xF8 && stri8[0] <= 0xFB && len >= 5 &&
                 stri8[1] >= 0x80 && stri8[1] <= 0xBF &&
                 stri8[2] >= 0x80 && stri8[2] <= 0xBF &&
                 stri8[3] >= 0x80 && stri8[3] <= 0xBF &&
                 stri8[4] >= 0x80 && stri8[4] <= 0xBF) {
        /* stri8[0]   range 248 to 251 (leading bits 111110..) */
        /* stri8[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (stri8[0] & 0x03) << 24 |
                  (strElemType) (stri8[1] & 0x3F) << 18 |
                  (strElemType) (stri8[2] & 0x3F) << 12 |
                  (strElemType) (stri8[3] & 0x3F) <<  6 |
                  (strElemType) (stri8[4] & 0x3F);
        stri8 += 5;
        len -= 4;
      } else if (stri8[0] >= 0xFC && stri8[0] <= 0xFF && len >= 6 &&
                 stri8[1] >= 0x80 && stri8[1] <= 0xBF &&
                 stri8[2] >= 0x80 && stri8[2] <= 0xBF &&
                 stri8[3] >= 0x80 && stri8[3] <= 0xBF &&
                 stri8[4] >= 0x80 && stri8[4] <= 0xBF &&
                 stri8[5] >= 0x80 && stri8[5] <= 0xBF) {
        /* stri8[0]   range 252 to 255 (leading bits 111111..) */
        /* stri8[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (stri8[0] & 0x03) << 30 |
                  (strElemType) (stri8[1] & 0x3F) << 24 |
                  (strElemType) (stri8[2] & 0x3F) << 18 |
                  (strElemType) (stri8[3] & 0x3F) << 12 |
                  (strElemType) (stri8[4] & 0x3F) <<  6 |
                  (strElemType) (stri8[5] & 0x3F);
        stri8 += 6;
        len -= 5;
      } else {
        /* stri8[0] not in range 0xC0 to 0xFF (192 to 255) */
        /* or not enough continuation bytes found.         */
        *dest_len = (memSizeType) (stri - dest_stri);
        return len;
      } /* if */
    } /* for */
    *dest_len = (memSizeType) (stri - dest_stri);
    return 0;
  } /* stri8_to_stri */



/**
 *  Convert an UTF-8 encoded string to an UTF-32 encoded string.
 *  The source and destination strings are not '\0' terminated.
 *  The memory for the destination dest_stri is not allocated.
 *  @param dest_stri Destination of the UTF-32 encoded string.
 *  @param dest_len Place to return the length of dest_stri.
 *  @param ustri UTF-8 encoded string to be converted.
 *  @param len Number of bytes in ustri.
 *  @return the number of bytes in ustri that are left unconverted, or
 *          0 if ustri has been successfully converted.
 */
memSizeType utf8_to_stri (strElemType *const dest_stri,
    memSizeType *const dest_len, const_ustriType ustri, memSizeType len)

  {
    strElemType *stri;

  /* utf8_to_stri */
    stri = dest_stri;
    for (; len > 0; len--) {
      if (*ustri <= 0x7F) {
        *stri++ = (strElemType) *ustri++;
      } else if (ustri[0] >= 0xC0 && ustri[0] <= 0xDF && len >= 2 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
        /* ustri[0]   range 192 to 223 (leading bits 110.....) */
        /* ustri[1]   range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (ustri[0] & 0x1F) << 6 |
                  (strElemType) (ustri[1] & 0x3F);
        ustri += 2;
        len--;
      } else if (ustri[0] >= 0xE0 && ustri[0] <= 0xEF && len >= 3 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
        /* ustri[0]   range 224 to 239 (leading bits 1110....) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (ustri[0] & 0x0F) << 12 |
                  (strElemType) (ustri[1] & 0x3F) <<  6 |
                  (strElemType) (ustri[2] & 0x3F);
        ustri += 3;
        len -= 2;
      } else if (ustri[0] >= 0xF0 && ustri[0] <= 0xF7 && len >= 4 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
        /* ustri[0]   range 240 to 247 (leading bits 11110...) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (ustri[0] & 0x07) << 18 |
                  (strElemType) (ustri[1] & 0x3F) << 12 |
                  (strElemType) (ustri[2] & 0x3F) <<  6 |
                  (strElemType) (ustri[3] & 0x3F);
        ustri += 4;
        len -= 3;
      } else if (ustri[0] >= 0xF8 && ustri[0] <= 0xFB && len >= 5 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF &&
                 ustri[4] >= 0x80 && ustri[4] <= 0xBF) {
        /* ustri[0]   range 248 to 251 (leading bits 111110..) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (ustri[0] & 0x03) << 24 |
                  (strElemType) (ustri[1] & 0x3F) << 18 |
                  (strElemType) (ustri[2] & 0x3F) << 12 |
                  (strElemType) (ustri[3] & 0x3F) <<  6 |
                  (strElemType) (ustri[4] & 0x3F);
        ustri += 5;
        len -= 4;
      } else if (ustri[0] >= 0xFC && len >= 6 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF &&
                 ustri[4] >= 0x80 && ustri[4] <= 0xBF &&
                 ustri[5] >= 0x80 && ustri[5] <= 0xBF) {
        /* ustri[0]   range 252 to 255 (leading bits 111111..) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strElemType) (ustri[0] & 0x03) << 30 |
                  (strElemType) (ustri[1] & 0x3F) << 24 |
                  (strElemType) (ustri[2] & 0x3F) << 18 |
                  (strElemType) (ustri[3] & 0x3F) << 12 |
                  (strElemType) (ustri[4] & 0x3F) <<  6 |
                  (strElemType) (ustri[5] & 0x3F);
        ustri += 6;
        len -= 5;
      } else {
        /* ustri[0] not in range 0xC0 to 0xFF (192 to 255) */
        /* or not enough continuation bytes found.         */
        *dest_len = (memSizeType) (stri - dest_stri);
        return len;
      } /* if */
    } /* for */
    *dest_len = (memSizeType) (stri - dest_stri);
    return 0;
  } /* utf8_to_stri */



/**
 *  Get number of missing bytes in incomplete UTF-8 byte sequence.
 *  The first byte of an UTF-8 byte sequence indicates the
 *  number of bytes in the sequence. The function checks, if
 *  the bytes after the first byte are UTF-8 continuation bytes.
 *  @param ustri Incomplete UTF-8 byte sequence, to be examined.
 *  @param len Length of incomplete UTF-8 byte sequence 'ustri'.
 *  @return the number of bytes needed to get an UTF-8 character, or
 *          0 if 'ustri' is not the start of a UTF-8 byte sequence.
 */
memSizeType utf8_bytes_missing (const const_ustriType ustri, const memSizeType len)

  {
    memSizeType result = 0;

  /* utf8_bytes_missing */
    if (len >= 1 && *ustri > 0x7F) {
      if (ustri[0] >= 0xC0 && ustri[0] <= 0xDF) {
        /* ustri[0]   range 192 to 223 (leading bits 110.....) */
        if (len == 1) {
          result = 1;
        } /* if */
      } else if (ustri[0] >= 0xE0 && ustri[0] <= 0xEF) {
        /* ustri[0]   range 224 to 239 (leading bits 1110....) */
        if (len == 1) {
          result = 2;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 1;
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xF0 && ustri[0] <= 0xF7) {
        /* ustri[0]   range 240 to 247 (leading bits 11110...) */
        if (len == 1) {
          result = 3;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 2;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 1;
            } /* if */
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xF8 && ustri[0] <= 0xFB) {
        /* ustri[0]   range 248 to 251 (leading bits 111110..) */
        if (len == 1) {
          result = 4;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 3;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 2;
            } else if (ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
              if (len == 4) {
                result = 1;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xFC) {
        /* ustri[0]   range 252 to 255 (leading bits 111111..) */
        if (len == 1) {
          result = 5;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 4;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 3;
            } else if (ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
              if (len == 4) {
                result = 2;
              } else if (ustri[4] >= 0x80 && ustri[4] <= 0xBF) {
                if (len == 5) {
                  result = 1;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* utf8_bytes_missing */



/**
 *  Convert an UTF-32 encoded string to an UTF-8 encoded string.
 *  The source and destination strings are not '\0' terminated.
 *  The memory for the destination out_stri is not allocated.
 *  @param out_stri Destination of the UTF-8 encoded string.
 *  @param strelem UTF-32 encoded string to be converted.
 *  @param len Number of UTF-32 characters in strelem.
 *  @return the length of the converted UTF-8 string.
 */
memSizeType stri_to_utf8 (const ustriType out_stri,
    const strElemType *strelem, memSizeType len)

  {
    register ustriType ustri;
    register strElemType ch;

  /* stri_to_utf8 */
    ustri = out_stri;
    for (; len > 0; strelem++, len--) {
      ch = *strelem;
      if (ch <= 0x7F) {
        *ustri++ = (ucharType) ch;
      } else if (ch <= 0x7FF) {
        ustri[0] = (ucharType) (0xC0 | (ch >>  6));
        ustri[1] = (ucharType) (0x80 |( ch        & 0x3F));
        ustri += 2;
      } else if (ch <= 0xFFFF) {
        ustri[0] = (ucharType) (0xE0 | (ch >> 12));
        ustri[1] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        ustri[2] = (ucharType) (0x80 |( ch        & 0x3F));
        ustri += 3;
      } else if (ch <= 0x1FFFFF) {
        ustri[0] = (ucharType) (0xF0 | (ch >> 18));
        ustri[1] = (ucharType) (0x80 |((ch >> 12) & 0x3F));
        ustri[2] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        ustri[3] = (ucharType) (0x80 |( ch        & 0x3F));
        ustri += 4;
      } else if (ch <= 0x3FFFFFF) {
        ustri[0] = (ucharType) (0xF8 | (ch >> 24));
        ustri[1] = (ucharType) (0x80 |((ch >> 18) & 0x3F));
        ustri[2] = (ucharType) (0x80 |((ch >> 12) & 0x3F));
        ustri[3] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        ustri[4] = (ucharType) (0x80 |( ch        & 0x3F));
        ustri += 5;
      } else {
        ustri[0] = (ucharType) (0xFC | (ch >> 30));
        ustri[1] = (ucharType) (0x80 |((ch >> 24) & 0x3F));
        ustri[2] = (ucharType) (0x80 |((ch >> 18) & 0x3F));
        ustri[3] = (ucharType) (0x80 |((ch >> 12) & 0x3F));
        ustri[4] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        ustri[5] = (ucharType) (0x80 |( ch        & 0x3F));
        ustri += 6;
      } /* if */
    } /* for */
    return (memSizeType) (ustri - out_stri);
  } /* stri_to_utf8 */



static inline boolType stri_to_os_utf8 (register ustriType out_stri,
    register const strElemType *strelem, register memSizeType len,
    errInfoType *err_info)

  {
    register strElemType ch;

  /* stri_to_os_utf8 */
    for (; len > 0; strelem++, len--) {
      ch = *strelem;
      if (ch <= 0x7F) {
        if (unlikely(ch == '\0')) {
          logError(printf("stri_to_os_utf8: Null character ('\\0;') in string.\n"););
          *err_info = RANGE_ERROR;
          return FALSE;
        } else {
          *out_stri++ = (ucharType) ch;
        } /* if */
      } else if (ch <= 0x7FF) {
        out_stri[0] = (ucharType) (0xC0 | (ch >>  6));
        out_stri[1] = (ucharType) (0x80 |( ch        & 0x3F));
        out_stri += 2;
      } else if (ch <= 0xFFFF) {
        out_stri[0] = (ucharType) (0xE0 | (ch >> 12));
        out_stri[1] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        out_stri[2] = (ucharType) (0x80 |( ch        & 0x3F));
        out_stri += 3;
      } else if (ch <= 0x10FFFF) {
        out_stri[0] = (ucharType) (0xF0 | (ch >> 18));
        out_stri[1] = (ucharType) (0x80 |((ch >> 12) & 0x3F));
        out_stri[2] = (ucharType) (0x80 |((ch >>  6) & 0x3F));
        out_stri[3] = (ucharType) (0x80 |( ch        & 0x3F));
        out_stri += 4;
      } else {
        logError(printf("stri_to_os_utf8: "
                        "Non-Unicode character ('\\" FMT_U32 ";') in string.\n",
                        ch););
        *err_info = RANGE_ERROR;
        return FALSE;
      } /* if */
    } /* for */
    *out_stri = '\0';
    return TRUE;
  } /* stri_to_os_utf8 */



/**
 *  Copy a Seed7 UTF-32 string to an ISO-8859-1 encoded C string buffer.
 *  The buffer 'cstri' must be provided by the caller. The
 *  size of the 'cstri' buffer can be calculated (in bytes) with
 *  stri->size+1. If a fixed size 'cstri' buffer is used
 *  (e.g.: char out_buffer[BUF_SIZE];) the condition
 *    stri->size < BUF_SIZE
 *  must hold. The C string written to 'out_buf' is zero byte
 *  terminated. This function is intended to copy to temporary
 *  string buffers, that are used as parameters. This function
 *  is useful, if stri->size is somehow limited, such that
 *  a fixed size 'cstri' buffer can be used.
 *  @param cstri Caller provided buffer to which an ISO-8859-1
 *         encoded null terminated C string is written.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @result cstri if the function succeeds, and
 *          NULL if stri contains a null character
 *          or a character that is higher than the
 *          highest allowed ISO-8859-1 character (255).
 */
cstriType conv_to_cstri (cstriType cstri, const const_striType stri)

  {
    const strElemType *str;
    memSizeType pos;

  /* conv_to_cstri */
    str = stri->mem;
    for (pos = stri->size; pos != 0; ) {
      pos--;
      if (unlikely(str[pos] == 0 || str[pos] >= 256)) {
        logError(printf("conv_to_cstri: "
                        "Null character or non-ISO-8859-1 character "
                        "in string ('\\" FMT_U32 ";').\n",
                        str[pos]););
        return NULL;
      } /* if */
      cstri[pos] = (char) (ucharType) str[pos];
    } /* for */
    cstri[stri->size] = '\0';
    return cstri;
  } /* conv_to_cstri */



/**
 *  Copy a Seed7 UTF-32 string to an UTF-8 encoded C string buffer.
 *  The buffer 'cstri' must be provided by the caller. The
 *  size of the 'cstri' buffer can be calculated (in bytes) with
 *  max_utf8_size(stri->size)+1. If a fixed size 'cstri'
 *  buffer is used (e.g.: char out_buffer[BUF_SIZE];) the condition
 *    stri->size < BUF_SIZE / MAX_UTF8_EXPANSION_FACTOR
 *  must hold. The C string written to 'out_buf' is zero byte
 *  terminated. This function is intended to copy to temporary
 *  string buffers, that are used as parameters. This function
 *  is useful, if stri->size is somehow limited, such that
 *  a fixed size 'cstri' buffer can be used.
 *  @param cstri Caller provided buffer to which an UTF-8 encoded
 *         null terminated C string is written.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if stri contains a null character
 *                        or a character that is higher than the
 *                        highest allowed Unicode character (U+10FFFF).
 */
void conv_to_cstri8 (cstriType cstri, const const_striType stri,
    errInfoType *err_info)

  { /* conv_to_cstri8 */
    stri_to_os_utf8((ustriType) cstri, stri->mem, stri->size, err_info);
  } /* conv_to_cstri8 */



/**
 *  Convert an UTF-32 encoded string to an UTF-16 encoded string.
 *  The source and destination strings are not '\0' terminated.
 *  The memory for the destination out_wstri is not allocated.
 *  @param out_wstri Destination of the UTF-16 encoded string.
 *  @param strelem UTF-32 encoded string to be converted.
 *  @param len Number of UTF-32 characters in strelem.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if *strelem contains a character
 *                        that is higher than the highest allowed
 *                        Unicode character (U+10FFFF).
 *  @return the length of the converted UTF-16 string in characters.
 */
memSizeType stri_to_utf16 (const utf16striType out_wstri,
    register const strElemType *strelem, memSizeType len,
    errInfoType *const err_info)

  {
    register strElemType ch;
    register utf16striType wstri;

  /* stri_to_utf16 */
    wstri = out_wstri;
    for (; len > 0; wstri++, strelem++, len--) {
      ch = *strelem;
      if (likely(ch <= 0xFFFF)) {
        *wstri = (utf16charType) ch;
      } else if (ch <= 0x10FFFF) {
        ch -= 0x10000;
        *wstri = (utf16charType) (0xD800 | (ch >> 10));
        wstri++;
        *wstri = (utf16charType) (0xDC00 | (ch & 0x3FF));
      } else {
        logError(printf("stri_to_utf16: Non-Unicode character "
                        "in string ('\\" FMT_U32 ";').\n",
                        ch););
        *err_info = RANGE_ERROR;
        len = 1;
      } /* if */
    } /* for */
    return (memSizeType) (wstri - out_wstri);
  } /* stri_to_utf16 */



#ifdef OS_STRI_WCHAR
static inline boolType conv_to_os_stri (register os_striType os_stri,
    register const strElemType *strelem, memSizeType len,
    errInfoType *const err_info)

  {
    register strElemType ch;
    boolType okay = TRUE;

  /* conv_to_os_stri */
    logFunction(printf("conv_to_os_stri(*, \"%s\")\n",
                striCharsAsUnquotedCStri(strelem, len)););
    for (; len > 0; os_stri++, strelem++, len--) {
      ch = *strelem;
      if (likely(ch <= 0xFFFF)) {
        if (unlikely(ch == '\0')) {
          logError(printf("conv_to_os_stri: Null character ('\\0;') in string.\n"););
          *err_info = RANGE_ERROR;
          okay = FALSE;
          len = 1;
        } else {
          *os_stri = (os_charType) ch;
        } /* if */
      } else if (ch <= 0x10FFFF) {
        ch -= 0x10000;
        *os_stri = (os_charType) (0xD800 | (ch >> 10));
        os_stri++;
        *os_stri = (os_charType) (0xDC00 | (ch & 0x3FF));
      } else {
        logError(printf("conv_to_os_stri: "
                        "Non-Unicode character ('\\" FMT_U32 ";') in string.\n",
                        ch););
        *err_info = RANGE_ERROR;
        okay = FALSE;
        len = 1;
      } /* if */
    } /* for */
    *os_stri = '\0';
    logFunction(printf("conv_to_os_stri --> %d (err_info=%d)\n",
                       okay, *err_info););
    return okay;
  } /* conv_to_os_stri */



#elif defined OS_STRI_USES_CODE_PAGE

static const unsigned char map_to_437_160[] = {
/*  160 */  255,  173,  155,  156,  '?',  157,  '?',  '?',  '?',  '?',
/*  170 */  166,  174,  170,  '?',  '?',  '?',  248,  241,  253,  '?',
/*  180 */  '?',  230,  '?',  250,  '?',  '?',  167,  175,  172,  171,
/*  190 */  '?',  168,  '?',  '?',  '?',  '?',  142,  143,  146,  128,
/*  200 */  '?',  144,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  165,
/*  210 */  '?',  '?',  '?',  '?',  153,  '?',  '?',  '?',  '?',  '?',
/*  220 */  154,  '?',  '?',  225,  133,  160,  131,  '?',  132,  134,
/*  230 */  145,  135,  138,  130,  136,  137,  141,  161,  140,  139,
/*  240 */  '?',  164,  149,  162,  147,  '?',  148,  246,  '?',  151,
/*  250 */  163,  150,  129,  '?',  '?',  152};

static const unsigned char map_to_437_915[] = {
/*  910 */                                226,  '?',  '?',  '?',  '?',
/*  920 */  233,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/*  930 */  '?',  228,  '?',  '?',  232,  '?',  '?',  234,  '?',  '?',
/*  940 */  '?',  '?',  '?',  '?',  '?',  224,  '?',  '?',  235,  238,
/*  950 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/*  960 */  227,  '?',  '?',  229,  231,  '?',  237};

static const unsigned char map_to_437_9472[] = {
/* 9470 */              196,  '?',  179,  '?',  '?',  '?',  '?',  '?',
/* 9480 */  '?',  '?',  '?',  '?',  218,  '?',  '?',  '?',  191,  '?',
/* 9490 */  '?',  '?',  192,  '?',  '?',  '?',  217,  '?',  '?',  '?',
/* 9500 */  195,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  180,  '?',
/* 9510 */  '?',  '?',  '?',  '?',  '?',  '?',  194,  '?',  '?',  '?',
/* 9520 */  '?',  '?',  '?',  '?',  193,  '?',  '?',  '?',  '?',  '?',
/* 9530 */  '?',  '?',  197,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9540 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9550 */  '?',  '?',  205,  186,  213,  214,  201,  184,  183,  187,
/* 9560 */  212,  211,  200,  190,  189,  188,  198,  199,  204,  181,
/* 9570 */  182,  185,  209,  210,  203,  207,  208,  202,  216,  215,
/* 9580 */  206,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9590 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9600 */  223,  '?',  '?',  '?',  220,  '?',  '?',  '?',  219,  '?',
/* 9610 */  '?',  '?',  221,  '?',  '?',  '?',  222,  176,  177,  178,
/* 9620 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9630 */  '?',  '?',  254};

static const unsigned char map_to_850_160[] = {
/*  160 */  255,  173,  189,  156,  207,  190,  221,  245,  249,  184,
/*  170 */  166,  174,  170,  240,  169,  238,  248,  241,  253,  252,
/*  180 */  239,  230,  244,  250,  247,  251,  167,  175,  172,  171,
/*  190 */  243,  168,  183,  181,  182,  199,  142,  143,  146,  128,
/*  200 */  212,  144,  210,  211,  222,  214,  215,  216,  209,  165,
/*  210 */  227,  224,  226,  229,  153,  158,  157,  235,  233,  234,
/*  220 */  154,  237,  232,  225,  133,  160,  131,  198,  132,  134,
/*  230 */  145,  135,  138,  130,  136,  137,  141,  161,  140,  139,
/*  240 */  208,  164,  149,  162,  147,  228,  148,  246,  155,  151,
/*  250 */  163,  150,  129,  236,  231,  152};

static const unsigned char map_to_850_9472[] = {
/* 9470 */              196,  '?',  179,  '?',  '?',  '?',  '?',  '?',
/* 9480 */  '?',  '?',  '?',  '?',  218,  '?',  '?',  '?',  191,  '?',
/* 9490 */  '?',  '?',  192,  '?',  '?',  '?',  217,  '?',  '?',  '?',
/* 9500 */  195,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  180,  '?',
/* 9510 */  '?',  '?',  '?',  '?',  '?',  '?',  194,  '?',  '?',  '?',
/* 9520 */  '?',  '?',  '?',  '?',  193,  '?',  '?',  '?',  '?',  '?',
/* 9530 */  '?',  '?',  197,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9540 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9550 */  '?',  '?',  205,  186,  '?',  '?',  201,  '?',  '?',  187,
/* 9560 */  '?',  '?',  200,  '?',  '?',  188,  '?',  '?',  204,  '?',
/* 9570 */  '?',  185,  '?',  '?',  203,  '?',  '?',  202,  '?',  '?',
/* 9580 */  206,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9590 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9600 */  223,  '?',  '?',  '?',  220,  '?',  '?',  '?',  219,  '?',
/* 9610 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  176,  177,  178,
/* 9620 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9630 */  '?',  '?',  254};



static inline boolType conv_to_os_stri (os_striType os_stri,
    const strElemType *strelem, memSizeType len, errInfoType *err_info)

  {
    unsigned char ch;
    boolType okay = TRUE;

  /* conv_to_os_stri */
    logFunction(printf("conv_to_os_stri(*, \"%s\")\n",
                striCharsAsUnquotedCStri(strelem, len)););
    if (code_page == 437) {
      for (; len > 0; os_stri++, strelem++, len--) {
        if (*strelem <= 127) {
          if (unlikely(*strelem == '\0')) {
            logError(printf("conv_to_os_stri: Null character ('\\0;') in string.\n"););
            *err_info = RANGE_ERROR;
            okay = FALSE;
            len = 1;
          } else {
            *os_stri = (os_charType) *strelem;
          } /* if */
        } else {
          if (*strelem >= 160 && *strelem <= 255) {
            ch = map_to_437_160[*strelem - 160];
          } else if (*strelem >= 915 && *strelem <= 966) {
            ch = map_to_437_915[*strelem - 915];
          } else if (*strelem >= 9472 && *strelem <= 9632) {
            ch = map_to_437_9472[*strelem - 9472];
          } else {
            switch (*strelem) {
              case  402: ch = 159; break;
              case 8319: ch = 252; break;
              case 8359: ch = 158; break;
              case 8729: ch = 249; break;
              case 8730: ch = 251; break;
              case 8734: ch = 236; break;
              case 8745: ch = 239; break;
              case 8776: ch = 247; break;
              case 8801: ch = 240; break;
              case 8804: ch = 243; break;
              case 8805: ch = 242; break;
              case 8976: ch = 169; break;
              case 8992: ch = 244; break;
              case 8993: ch = 245; break;
              default:   ch = '?'; break;
            } /* switch */
          } /* if */
          *os_stri = (os_charType) ch;
          if (unlikely(ch == '?')) {
            *err_info = RANGE_ERROR;
            okay = FALSE;
            /* The conversion continues. The caller  */
            /* can decide to use the question marks. */
          } /* if */
        } /* if */
      } /* for */
    } else if (code_page == 850 || code_page == 858) {
      for (; len > 0; os_stri++, strelem++, len--) {
        if (*strelem <= 127) {
          if (unlikely(*strelem == '\0')) {
            logError(printf("conv_to_os_stri: Null character ('\\0;') in string.\n"););
            *err_info = RANGE_ERROR;
            okay = FALSE;
            len = 1;
          } else {
            *os_stri = (os_charType) *strelem;
          } /* if */
        } else {
          if (*strelem >= 160 && *strelem <= 255) {
            ch = map_to_850_160[*strelem - 160];
          } else if (*strelem >= 9472 && *strelem <= 9632) {
            ch = map_to_850_9472[*strelem - 9472];
          } else {
            switch (*strelem) {
              case 305:  ch = code_page == 850 ? 213 : '?'; break;
              case 402:  ch = 159; break;
              case 8215: ch = 242; break;
              case 8364: ch = code_page == 858 ? 213 : '?'; break;
              default:   ch = '?'; break;
            } /* switch */
          } /* if */
          *os_stri = (os_charType) ch;
          if (unlikely(ch == '?')) {
            *err_info = RANGE_ERROR;
            okay = FALSE;
            /* The conversion continues. The caller  */
            /* can decide to use the question marks. */
          } /* if */
        } /* if */
      } /* for */
    } else {
      *err_info = RANGE_ERROR;
      okay = FALSE;
    } /* if */
    *os_stri = '\0';
    logFunction(printf("conv_to_os_stri --> %d (err_info=%d)\n",
                       okay, *err_info););
    return okay;
  } /* conv_to_os_stri */

#elif defined OS_STRI_UTF8



static inline boolType conv_to_os_stri (const os_striType os_stri,
    const strElemType *const strelem, const memSizeType len,
    errInfoType *err_info)

  {
    boolType okay;

  /* conv_to_os_stri */
    logFunction(printf("conv_to_os_stri(*, \"%s\")\n",
                striCharsAsUnquotedCStri(strelem, len)););
    okay = stri_to_os_utf8((ustriType) os_stri, strelem, len, err_info);
    logFunction(printf("conv_to_os_stri --> %d (err_info=%d)\n",
                       okay, *err_info););
    return okay;
  } /* conv_to_os_stri */

#else



static inline boolType conv_to_os_stri (const os_striType os_stri,
    const strElemType *const strelem, memSizeType len, errInfoType *err_info)

  {
    boolType okay = TRUE;

  /* conv_to_os_stri */
    logFunction(printf("conv_to_os_stri(*, \"%s\")\n",
                striCharsAsUnquotedCStri(strelem, len)););
    while (len != 0) {
      len--;
      if (unlikely(strelem[len] == '\0' || strelem[len] >= 256)) {
        logError(printf("conv_to_os_stri: "
                        "Null character or non-ISO-8859-1 character "
                        "in string ('\\" FMT_U32 ";').\n",
                        str[len]););
        *err_info = RANGE_ERROR;
        okay = FALSE;
      } /* if */
      os_stri[len] = (os_charType) strelem[len];
    } /* while */
    *os_stri = '\0';
    logFunction(printf("conv_to_os_stri --> %d (err_info=%d)\n",
                       okay, *err_info););
    return okay;
  } /* conv_to_os_stri */

#endif



static memSizeType wstri_expand (strElemType *const dest_stri,
    const_utf16striType wstri, memSizeType len)

  {
    strElemType *stri;
    utf16charType ch1;
    utf16charType ch2;

  /* wstri_expand */
    stri = dest_stri;
    for (; len > 0; stri++, wstri++, len--) {
      ch1 = *wstri;
      if (unlikely(ch1 >= 0xD800 && ch1 <= 0xDBFF)) {
        ch2 = wstri[1];
        if (likely(ch2 >= 0xDC00 && ch2 <= 0xDFFF)) {
          *stri = ((((strElemType) ch1 - 0xD800) << 10) +
                    ((strElemType) ch2 - 0xDC00) + 0x10000);
          wstri++;
          len--;
        } else {
          *stri = (strElemType) ch1;
        } /* if */
      } else {
        *stri = (strElemType) ch1;
      } /* if */
    } /* for */
    return (memSizeType) (stri - dest_stri);
  } /* wstri_expand */



#if defined OS_STRI_WCHAR
/**
 *  Convert an os_striType string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_striType data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_striType. Depending on the
 *  operating system os_striType can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri in characters.
 *  @return a Seed7 UTF-32 string, or
 *          NULL if an error occurred.
 */
striType conv_from_os_stri (const const_os_striType os_stri,
    memSizeType length)

  {
    memSizeType stri_size;
    striType resized_stri;
    striType stri;

  /* conv_from_os_stri */
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      stri_size = wstri_expand(stri->mem, (const_utf16striType) os_stri, length);
      stri->size = stri_size;
      if (stri_size != length) {
        REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
        if (unlikely(resized_stri == NULL)) {
          FREE_STRI2(stri, length);
          stri = NULL;
        } else {
          stri = resized_stri;
        } /* if */
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */



#elif defined OS_STRI_USES_CODE_PAGE

static const strElemType map_from_437[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  162,  163,  165, 8359,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191, 8976,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508, 9569, 9570, 9558, 9557, 9571, 9553, 9559, 9565, 9564,
/* 190 */ 9563, 9488, 9492, 9524, 9516, 9500, 9472, 9532, 9566, 9567,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580, 9575, 9576, 9572,
/* 210 */ 9573, 9561, 9560, 9554, 9555, 9579, 9578, 9496, 9484, 9608,
/* 220 */ 9604, 9612, 9616, 9600,  945,  223,  915,  960,  931,  963,
/* 230 */  181,  964,  934,  920,  937,  948, 8734,  966,  949, 8745,
/* 240 */ 8801,  177, 8805, 8804, 8992, 8993,  247, 8776,  176, 8729,
/* 250 */  183, 8730, 8319,  178, 9632,  160};

static const strElemType map_from_850[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  248,  163,  216,  215,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191,  174,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508,  193,  194,  192,  169, 9571, 9553, 9559, 9565,  162,
/* 190 */  165, 9488, 9492, 9524, 9516, 9500, 9472, 9532,  227,  195,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580,  164,  240,  208,
/* 210 */  202,  203,  200,  305,  205,  206,  207, 9496, 9484, 9608,
/* 220 */ 9604,  166,  204, 9600,  211,  223,  212,  210,  245,  213,
/* 230 */  181,  254,  222,  218,  219,  217,  253,  221,  175,  180,
/* 240 */  173,  177, 8215,  190,  182,  167,  247,  184,  176,  168,
/* 250 */  183,  185,  179,  178, 9632,  160};

static const strElemType map_from_858[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  248,  163,  216,  215,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191,  174,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508,  193,  194,  192,  169, 9571, 9553, 9559, 9565,  162,
/* 190 */  165, 9488, 9492, 9524, 9516, 9500, 9472, 9532,  227,  195,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580,  164,  240,  208,
/* 210 */  202,  203,  200, 8364,  205,  206,  207, 9496, 9484, 9608,
/* 220 */ 9604,  166,  204, 9600,  211,  223,  212,  210,  245,  213,
/* 230 */  181,  254,  222,  218,  219,  217,  253,  221,  175,  180,
/* 240 */  173,  177, 8215,  190,  182,  167,  247,  184,  176,  168,
/* 250 */  183,  185,  179,  178, 9632,  160};



charType mapFromCodePage (unsigned char key)

  {
    charType ch;

  /* mapFromCodePage */
    logFunction(printf("mapFromCodePage(%u)\n", key););
    if (code_page == 850) {
      ch = map_from_850[key];
    } else if (code_page == 858) {
      ch = map_from_858[key];
    } else {
      ch = map_from_437[key];
    } /* if */
    logFunction(printf("mapFromCodePage --> " FMT_U32 "\n", ch););
    return ch;
  } /* mapFromCodePage */



/**
 *  Convert an os_striType string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_striType data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_striType. Depending on the
 *  operating system os_striType can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri in characters.
 *  @return a Seed7 UTF-32 string, or
 *          NULL if an error occurred.
 */
striType conv_from_os_stri (const const_os_striType os_stri,
    memSizeType length)

  {
    memSizeType pos;
    striType stri;

  /* conv_from_os_stri */
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      stri->size = length;
      if (code_page == 437) {
        for (pos = 0; pos < length; pos++) {
          stri->mem[pos] = map_from_437[(unsigned char) os_stri[pos]];
        } /* for */
      } else if (code_page == 850) {
        for (pos = 0; pos < length; pos++) {
          stri->mem[pos] = map_from_850[(unsigned char) os_stri[pos]];
        } /* for */
      } else if (code_page == 858) {
        for (pos = 0; pos < length; pos++) {
          stri->mem[pos] = map_from_858[(unsigned char) os_stri[pos]];
        } /* for */
      } else {
        FREE_STRI2(stri, length);
        stri = NULL;
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#elif defined OS_STRI_UTF8



/**
 *  Convert an os_striType string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_striType data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_striType. Depending on the
 *  operating system os_striType can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri in characters.
 *  @return a Seed7 UTF-32 string, or
 *          NULL if an error occurred.
 */
striType conv_from_os_stri (const const_os_striType os_stri,
    memSizeType length)

  {
    memSizeType stri_size;
    striType resized_stri;
    striType stri;

  /* conv_from_os_stri */
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      if (likely(utf8_to_stri(stri->mem, &stri_size, (const_ustriType) os_stri, length) == 0)) {
        stri->size = stri_size;
        if (stri_size != length) {
          REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
          if (unlikely(resized_stri == NULL)) {
            FREE_STRI2(stri, length);
            stri = NULL;
          } else {
            stri = resized_stri;
          } /* if */
        } /* if */
      } else {
        /* Assume that os_stri is encoded in Latin-1 instead of UTF-8. */
        stri->size = length;
        memcpy_to_strelem(stri->mem, (const_ustriType) os_stri, length);
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#else



/**
 *  Convert an os_striType string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_striType data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_striType. Depending on the
 *  operating system os_striType can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri in characters.
 *  @return a Seed7 UTF-32 string, or
 *          NULL if an error occurred.
 */
striType conv_from_os_stri (const const_os_striType os_stri,
    memSizeType length)

  {
    striType stri;

  /* conv_from_os_stri */
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      memcpy_to_strelem(stri->mem, (const_ustriType) os_stri, length);
      stri->size = length;
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#endif



/**
 *  Create an ISO-8859-1 encoded C string from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated C string is allocated.
 *  The C string result must be freed with the macro free_cstri().
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if stri contains a null character
 *                        or a character that is higher than the
 *                        highest allowed ISO-8859-1 character (255).
 *  @return an ISO-8859-1 encoded null terminated C string, or
 *          NULL if the memory allocation failed or the
 *          conversion failed (the error is indicated by err_info).
 */
cstriType stri_to_cstri (const const_striType stri, errInfoType *err_info)

  {
    const strElemType *str;
    memSizeType pos;
    cstriType cstri;

  /* stri_to_cstri */
    if (unlikely(!ALLOC_CSTRI(cstri, stri->size))) {
      *err_info = MEMORY_ERROR;
    } else {
      str = stri->mem;
      for (pos = stri->size; pos != 0; ) {
        pos--;
        if (unlikely(str[pos] == 0 || str[pos] >= 256)) {
          UNALLOC_CSTRI(cstri, stri->size);
          logError(printf("stri_to_cstri: "
                          "Null character or non-ISO-8859-1 character "
                          "in string ('\\" FMT_U32 ";').\n",
                          str[pos]););
          *err_info = RANGE_ERROR;
          return NULL;
        } /* if */
        cstri[pos] = (char) (ucharType) str[pos];
      } /* for */
      cstri[stri->size] = '\0';
    } /* if */
    return cstri;
  } /* stri_to_cstri */



/**
 *  Create an UTF-8 encoded C string from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated C string is allocated.
 *  The C string result must be freed with the macro free_cstri8().
 *  This function is intended to create temporary strings, that
 *  are used as parameters. To get good performance the allocated
 *  memory for the C string is oversized.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if stri contains a null character
 *                        or a character that is higher than the
 *                        highest allowed Unicode character (U+10FFFF).
 *  @return an UTF-8 encoded null terminated C string, or
 *          NULL if the memory allocation failed or the
 *          conversion failed (the error is indicated by err_info).
 */
cstriType stri_to_cstri8 (const const_striType stri, errInfoType *err_info)

  {
    cstriType cstri;

  /* stri_to_cstri8 */
    if (unlikely(stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_CSTRI(cstri, max_utf8_size(stri->size)))) {
      *err_info = MEMORY_ERROR;
      cstri = NULL;
    } else {
      if (unlikely(!stri_to_os_utf8((ustriType) cstri, stri->mem,
                                    stri->size, err_info))) {
        free_cstri8(cstri, stri);
        cstri = NULL;
      } /* if */
    } /* if */
    return cstri;
  } /* stri_to_cstri8 */



/**
 *  Create an UTF-8 encoded C string buffer from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated C string is allocated.
 *  Zero bytes inside the string are copied to the C string.
 *  The C string result must be freed with the macro free_cstri8().
 *  This function is intended to create temporary strings, that
 *  are used as parameters. To get good performance the allocated
 *  memory for the C string is oversized.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param length Place to return the length of the result (without '\0').
 *  @return an UTF-8 encoded null terminated C string, or
 *          NULL if the memory allocation failed.
 */
cstriType stri_to_cstri8_buf (const const_striType stri, memSizeType *length)

  {
    cstriType cstri;

  /* stri_to_cstri8_buf */
    if (unlikely(stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_CSTRI(cstri, max_utf8_size(stri->size)))) {
      cstri = NULL;
    } else {
      *length = stri_to_utf8((ustriType) cstri, stri->mem, stri->size);
      cstri[*length] = '\0';
    } /* if */
    return cstri;
  } /* stri_to_cstri8_buf */



/**
 *  Create an ISO-8859-1 encoded bstring from a Seed7 UTF-32 string.
 *  The memory for the bstring is allocated. No zero byte is added
 *  to the end of the bstring. No special action is done, if the
 *  UTF-32 string contains a null character.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if stri contains a character
 *                        that is higher than the highest
 *                        allowed ISO-8859-1 character (255).
 *  @return an ISO-8859-1 encoded bstring, or
 *          NULL if the memory allocation failed or the
 *          conversion failed (the error is indicated by err_info).
 */
bstriType stri_to_bstri (const const_striType stri, errInfoType *err_info)

  {
    bstriType bstri;

  /* stri_to_bstri */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri, stri->size))) {
      *err_info = MEMORY_ERROR;
    } else {
      bstri->size = stri->size;
      if (unlikely(memcpy_from_strelem(bstri->mem, stri->mem, stri->size))) {
        FREE_BSTRI(bstri, bstri->size);
        *err_info = RANGE_ERROR;
        return NULL;
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstri */



/**
 *  Create an UTF-8 encoded bstring from a Seed7 UTF-32 string.
 *  The memory for the bstring is allocated. No zero byte is added
 *  to the end of the bstring. No special action is done, if
 *  the original string contains a null character.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @return an UTF-8 encoded bstring, or
 *          NULL if the memory allocation failed.
 */
bstriType stri_to_bstri8 (const const_striType stri)

  {
    bstriType resized_bstri;
    bstriType bstri;

  /* stri_to_bstri8 */
    if (unlikely(stri->size > MAX_BSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR)) {
      bstri = NULL;
    } else if (likely(ALLOC_BSTRI_SIZE_OK(bstri, max_utf8_size(stri->size)))) {
      bstri->size = stri_to_utf8(bstri->mem, stri->mem, stri->size);
      REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri, max_utf8_size(stri->size), bstri->size);
      if (unlikely(resized_bstri == NULL)) {
        FREE_BSTRI(bstri, max_utf8_size(stri->size));
        bstri = NULL;
      } else {
        bstri = resized_bstri;
        COUNT3_BSTRI(max_utf8_size(stri->size), bstri->size);
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstri8 */



#ifdef CONSOLE_WCHAR
/**
 *  Create an UTF-16 encoded bstring from a Seed7 UTF-32 string.
 *  The memory for the bstring is allocated. No zero byte is added
 *  to the end of the bstring. No special action is done, if
 *  the original string contains a null character.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @return an UTF-8 encoded bstring, or
 *          NULL if the memory allocation failed.
 */
bstriType stri_to_bstriw (const const_striType stri, errInfoType *err_info)

  {
    memSizeType wstri_size;
    bstriType resized_bstri;
    bstriType bstri;

  /* stri_to_bstriw */
    if (stri->size > ((MAX_BSTRI_LEN / sizeof(os_charType)) / SURROGATE_PAIR_FACTOR)) {
      *err_info = MEMORY_ERROR;
      bstri = NULL;
    } else if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri,
        stri->size * SURROGATE_PAIR_FACTOR * sizeof(os_charType)))) {
      *err_info = MEMORY_ERROR;
    } else {
      wstri_size = stri_to_utf16((utf16striType) bstri->mem, stri->mem, stri->size, err_info);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        FREE_BSTRI(bstri, stri->size * SURROGATE_PAIR_FACTOR * sizeof(os_charType));
        bstri = NULL;
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri,
            stri->size * SURROGATE_PAIR_FACTOR * sizeof(os_charType),
            wstri_size * sizeof(os_charType));
        if (resized_bstri == NULL) {
          FREE_BSTRI(bstri, stri->size * SURROGATE_PAIR_FACTOR * sizeof(os_charType));
          bstri = NULL;
        } else {
          bstri = resized_bstri;
          COUNT3_BSTRI(stri->size * SURROGATE_PAIR_FACTOR * sizeof(os_charType),
              wstri_size * sizeof(os_charType));
          bstri->size = wstri_size * sizeof(os_charType);
        } /* if */
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstriw */
#endif



/**
 *  Create an UTF-16 encoded wide string buffer from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated wide string is allocated.
 *  This function is intended to create temporary strings, that
 *  are used as parameters. To get good performance the allocated
 *  memory for the wide string is oversized.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param length Place to return the character length of the result (without '\0').
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if stri contains a character
 *                        that is higher than the highest
 *                        allowed Unicode character (U+10FFFF).
 *  @return an UTF-16 encoded null terminated wide string, or
 *          NULL if the memory allocation failed or the
 *          conversion failed (the error is indicated by err_info).
 */
utf16striType stri_to_wstri16 (const const_striType stri, memSizeType *length,
      errInfoType *err_info)

  {
    utf16striType wstri;

  /* stri_to_wstri16 */
    if (unlikely(stri->size > MAX_UTF16_LEN / SURROGATE_PAIR_FACTOR ||
                 !ALLOC_UTF16(wstri, SURROGATE_PAIR_FACTOR * stri->size))) {
      *err_info = MEMORY_ERROR;
      wstri = NULL;
    } else {
      *length = stri_to_utf16(wstri, stri->mem, stri->size, err_info);
      wstri[*length] = '\0';
    } /* if */
    return wstri;
  } /* stri_to_wstri16 */



utf32striType stri_to_wstri32 (const const_striType stri, memSizeType *length,
    errInfoType *err_info)

  {
    utf32striType wstri;

  /* stri_to_wstri32 */
    if (unlikely(stri->size > MAX_UTF32_LEN ||
                 !ALLOC_UTF32(wstri, stri->size))) {
      *err_info = MEMORY_ERROR;
      wstri = NULL;
    } else {
      /* Assume that sizeof(strElemType) == sizeof(utf32charType). */
      memcpy(wstri, stri->mem, stri->size * sizeof(utf32charType));
      wstri[stri->size] = '\0';
      *length = stri->size;
    } /* if */
    return wstri;
  } /* stri_to_wstri32 */



/**
 *  Copy a Seed7 string .
 *  @return a copy of the source string, or
 *          NULL if the memory allocation failed.
 */
striType copy_stri (const const_striType source)

  {
    memSizeType new_size;
    striType result;

  /* copy_stri */
    new_size = source->size;
    if (likely(ALLOC_STRI_SIZE_OK(result, new_size))) {
      result->size = new_size;
      if (new_size != 0) {
        memcpy(result->mem, source->mem, new_size * sizeof(strElemType));
      } /* if */
    } /* if */
    return result;
  } /* copy_stri */



/**
 *  Copy an ISO-8859-1 (Latin-1) encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated ISO-8859-1 encoded C string.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed.
 */
striType cstri_to_stri (const_cstriType cstri)

  {
    memSizeType length;
    striType stri;

  /* cstri_to_stri */
    length = strlen(cstri);
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      stri->size = length;
      memcpy_to_strelem(stri->mem, (const_ustriType) cstri, length);
    } /* if */
    return stri;
  } /* cstri_to_stri */



/**
 *  Copy an ISO-8859-1 (Latin-1) encoded C string buffer to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri ISO-8859-1 encoded C string buffer (not null terminated).
 *  @param length Byte length of the ISO-8859-1 encoded C string buffer.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed.
 */
striType cstri_buf_to_stri (const_cstriType cstri, memSizeType length)

  {
    striType stri;

  /* cstri_buf_to_stri */
    if (likely(ALLOC_STRI_CHECK_SIZE(stri, length))) {
      stri->size = length;
      memcpy_to_strelem(stri->mem, (const_ustriType) cstri, length);
    } /* if */
    return stri;
  } /* cstri_buf_to_stri */



/**
 *  Copy an UTF-8 encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated UTF-8 encoded C string.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if the conversion failed.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed or
 *          invalid UTF-8 encodings are used.
 */
striType cstri8_to_stri (const_cstriType cstri, errInfoType *err_info)

  {
    memSizeType length;
    memSizeType stri_size;
    striType resized_stri;
    striType stri;

  /* cstri8_to_stri */
    length = strlen(cstri);
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else {
      if (likely(utf8_to_stri(stri->mem, &stri_size, (const_ustriType) cstri, length) == 0)) {
        stri->size = stri_size;
        if (stri_size != length) {
          REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
          if (unlikely(resized_stri == NULL)) {
            FREE_STRI2(stri, length);
            *err_info = MEMORY_ERROR;
            stri = NULL;
          } else {
            stri = resized_stri;
          } /* if */
        } /* if */
      } else {
        FREE_STRI2(stri, length);
        *err_info = RANGE_ERROR;
        stri = NULL;
      } /* if */
    } /* if */
    return stri;
  } /* cstri8_to_stri */



/**
 *  Copy an UTF-8 encoded C string buffer to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri UTF-8 encoded C string buffer (not null terminated).
 *  @param length Byte length of the UTF-8 encoded C string buffer.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if the conversion failed.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed or
 *          invalid UTF-8 encodings are used.
 */
striType cstri8_buf_to_stri (const_cstriType cstri, memSizeType length,
    errInfoType *err_info)

  {
    memSizeType stri_size;
    striType resized_stri;
    striType stri;

  /* cstri8_buf_to_stri */
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else {
      if (likely(utf8_to_stri(stri->mem, &stri_size, (const_ustriType) cstri, length) == 0)) {
        stri->size = stri_size;
        if (stri_size != length) {
          REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
          if (unlikely(resized_stri == NULL)) {
            FREE_STRI2(stri, length);
            *err_info = MEMORY_ERROR;
            stri = NULL;
          } else {
            stri = resized_stri;
          } /* if */
        } /* if */
      } else {
        FREE_STRI2(stri, length);
        *err_info = RANGE_ERROR;
        stri = NULL;
      } /* if */
    } /* if */
    return stri;
  } /* cstri8_buf_to_stri */



/**
 *  Copy an UTF-8 or ISO-8859-1 encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated UTF-8 or ISO-8859-1 encoded C string.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed.
 */
striType cstri8_or_cstri_to_stri (const_cstriType cstri)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType stri;

  /* cstri8_or_cstri_to_stri */
    stri = cstri8_to_stri(cstri, &err_info);
    if (stri == NULL) {
      stri = cstri_to_stri(cstri);
    } /* if */
    return stri;
  } /* cstri8_or_cstri_to_stri */



/**
 *  Copy an UTF-16 encoded wide string buffer to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param wstri UTF-16 encoded wide string buffer (not null terminated).
 *  @param length Character length of the UTF-16 encoded wide string buffer.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed.
 *  @return an UTF-32 encoded Seed7 string, or
 *          NULL if the memory allocation failed.
 */
striType wstri16_to_stri (const_utf16striType wstri, memSizeType length,
    errInfoType *err_info)

  {
    memSizeType stri_size;
    striType resized_stri;
    striType stri;

  /* wstri16_to_stri */
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else {
      stri_size = wstri_expand(stri->mem, wstri, length);
      stri->size = stri_size;
      if (stri_size != length) {
        REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
        if (unlikely(resized_stri == NULL)) {
          FREE_STRI2(stri, length);
          *err_info = MEMORY_ERROR;
          stri = NULL;
        } else {
          stri = resized_stri;
        } /* if */
      } /* if */
    } /* if */
    return stri;
  } /* wstri16_to_stri */



striType wstri32_to_stri (const_utf32striType wstri, memSizeType length,
    errInfoType *err_info)

  {
    striType stri;

  /* wstri32_to_stri */
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else {
      stri->size = length;
      /* Assume that sizeof(strElemType) == sizeof(utf32charType). */
      memcpy(stri->mem, wstri, length * sizeof(utf32charType));
    } /* if */
    return stri;
  } /* wstri32_to_stri */



/**
 *  Convert a Seed7 UTF-32 string to a null terminated os_striType string.
 *  The memory for the null terminated os_striType string is allocated.
 *  The os_striType result is allocated with the macro os_stri_alloc()
 *  and it must be freed with the macro os_stri_free(). Strings allocated
 *  with os_stri_alloc() must be freed in the reverse order of their
 *  creation. This allows that allocations work in a stack like manner.
 *  Many system calls have parameters with null terminated os_striType
 *  strings. System calls are defined in "version.h" and "os_decls.h".
 *  They are prefixed with os_ and use strings of the type os_striType.
 *  Depending on the operating system os_striType can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if the conversion failed.
 *  @return a null terminated os_striType value used by system calls, or
 *          NULL if an error occurred.
 */
os_striType stri_to_os_stri (const_striType stri, errInfoType *err_info)

  {
    os_striType result;

  /* stri_to_os_stri */
    if (unlikely(stri->size > MAX_OS_STRI_SIZE)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(stri->size)))) {
      *err_info = MEMORY_ERROR;
    } else if (unlikely(!conv_to_os_stri(result, stri->mem,
                                         stri->size, err_info))) {
      os_stri_free(result);
      result = NULL;
    } /* if */
    return result;
  } /* stri_to_os_stri */



/**
 *  Convert a null terminated os_striType string to a Seed7 UTF-32 string.
 *  Many system calls return null terminated os_striType strings. System
 *  calls are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_striType. Depending on the
 *  operating system os_striType can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri null terminated os_striType string to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed.
 *  @return a Seed7 UTF-32 string, or
 *          NULL if an error occurred.
 */
striType os_stri_to_stri (const const_os_striType os_stri, errInfoType *err_info)

  {
    striType stri;

  /* os_stri_to_stri */
    logFunction(printf("os_stri_to_stri(\"" FMT_S_OS "\", *)\n", os_stri););
    stri = conv_from_os_stri(os_stri, os_stri_strlen(os_stri));
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } /* if */
    return stri;
  } /* os_stri_to_stri */



striType stri_to_standard_path (const striType stri)

  {
    memSizeType pathLength;
    memSizeType stdPathSize;
    striType resized_stdPath;
    striType stdPath;

  /* stri_to_standard_path */
    stdPath = stri;
    if (stdPath != NULL) {
      pathLength = stdPath->size;
#if PATH_DELIMITER != '/'
      {
        memSizeType pos;

        for (pos = 0; pos < pathLength; pos++) {
          if (stdPath->mem[pos] == PATH_DELIMITER) {
            stdPath->mem[pos] = (strElemType) '/';
          } /* if */
        } /* for */
      }
#endif
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (pathLength >= 2 && stdPath->mem[1] == ':' &&
          ((stdPath->mem[0] >= 'a' && stdPath->mem[0] <= 'z') ||
           (stdPath->mem[0] >= 'A' && stdPath->mem[0] <= 'Z'))) {
        stdPath->mem[1] = (strElemType) tolower((int) stdPath->mem[0]);
        stdPath->mem[0] = (strElemType) '/';
        if (pathLength >= 3) {
          if (stdPath->mem[2] != '/') {
            if (likely(ALLOC_STRI_CHECK_SIZE(stdPath, pathLength + 1))) {
              /* Stri points to the old stdPath. */
              stdPath->mem[0] = stri->mem[0];
              stdPath->mem[1] = stri->mem[1];
              stdPath->mem[2] = (strElemType) '/';
              memcpy(&stdPath->mem[3], &stri->mem[2],
                     (pathLength - 2) * sizeof(strElemType));
              FREE_STRI2(stri, pathLength);
              pathLength++;
              stdPath->size = pathLength;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
#endif
      if (likely(stdPath != NULL)) {
        {
          memSizeType pos;
          memSizeType pos2;

          for (pos = 0; pos < stdPath->size; pos++) {
            if (stdPath->mem[pos] == '/') {
              pos2 = pos + 1;
              while (pos2 < stdPath->size && stdPath->mem[pos2] == '/') {
                pos2++;
              } /* while */
              if (pos2 > pos + 1) {
                if (pos2 < stdPath->size) {
                  memmove(&stdPath->mem[pos + 1], &stdPath->mem[pos2],
                          (stdPath->size - pos2) * sizeof(strElemType));
                } /* if */
                stdPath->size -= pos2 - pos - 1;
              } /* if */
            } /* if */
          } /* for */
        }
        if (stdPath->size > 1 &&
               stdPath->mem[stdPath->size - 1] == (charType) '/') {
          stdPath->size--;
        } /* if */
        if (unlikely(stdPath->size != pathLength)) {
          stdPathSize = stdPath->size;
          REALLOC_STRI_SIZE_SMALLER2(resized_stdPath, stdPath, pathLength, stdPathSize);
          if (unlikely(resized_stdPath == NULL)) {
            FREE_STRI2(stdPath, pathLength);
            stdPath = NULL;
          } else {
            stdPath = resized_stdPath;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return stdPath;
  } /* stri_to_standard_path */



/**
 *  Convert a path returned by a system call to a Seed7 standard path.
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_striType.
 *  Depending on the operating system os_striType can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion from os_striType a
 *  mapping from drive letters might take place on some operating
 *  systems.
 *  @param os_path null terminated os_striType path to be converted.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed.
 *  @return an UTF-32 encoded Seed7 standard path, or
 *          NULL if the memory allocation failed.
 */
striType cp_from_os_path (const_os_striType os_path, errInfoType *err_info)

  {
    striType stdPath;

  /* cp_from_os_path */
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
    if (memcmp(os_path, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType)) == 0) {
      /* For extended path omit the prefix. */
      os_path = &os_path[PREFIX_LEN];
    } /* if */
#endif
    stdPath = os_stri_to_stri(os_path, err_info);
    if (likely(stdPath != NULL)) {
      stdPath = stri_to_standard_path(stdPath);
      if (unlikely(stdPath == NULL)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    return stdPath;
  } /* cp_from_os_path */



/**
 *  Convert a path returned by a system call to a Seed7 standard path.
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_striType.
 *  Depending on the operating system os_striType can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion from os_striType a
 *  mapping from drive letters might take place on some operating
 *  systems.
 *  @param os_path os_striType path to be converted.
 *  @param length Number of chars in os_path.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed.
 *  @return an UTF-32 encoded Seed7 standard path, or
 *          NULL if the memory allocation failed.
 */
striType cp_from_os_path_buffer (const_os_striType os_path,
    memSizeType length, errInfoType *err_info)

  {
    striType stdPath;

  /* cp_from_os_path_buffer */
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
    if (memcmp(os_path, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType)) == 0) {
      /* For extended path omit the prefix. */
      os_path = &os_path[PREFIX_LEN];
    } /* if */
#endif
    stdPath = conv_from_os_stri(os_path, length);
    if (unlikely(stdPath == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      stdPath = stri_to_standard_path(stdPath);
      if (unlikely(stdPath == NULL)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    return stdPath;
  } /* cp_from_os_path_buffer */



#ifdef MAP_LONG_FILE_NAMES_TO_SHORT
static boolType isShortFileName (os_striType fileName)

  {
    os_charType ch;
    int pos = 0;
    int dotPos = -1;
    boolType shortFileName = TRUE;

  /* isShortFileName */
    logFunction(printf("isShortFileName(\"" FMT_S_OS "\")\n", fileName););
    ch = fileName[0];
    do {
      switch (ch) {
        case '!':  case '#':  case '$':  case '%':  case '&':
        case '\'': case '(':  case ')':  case '-':
        case '0':  case '1':  case '2':  case '3':  case '4':
        case '5':  case '6':  case '7':  case '8':  case '9':
        case '@':
        case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
        case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
        case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
        case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
        case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
        case 'Z':
        case '^':  case '_':  case '`':
        case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
        case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
        case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
        case 'p':  case 'q':  case 'r':  case 's':  case 't':
        case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
        case 'z':
        case '{':  case '}':  case '~':
          /* Characters allowed in 8.3 file names. */
          break;
        case '.':
          if (dotPos == -1) {
            dotPos = pos;
          } else {
            shortFileName = FALSE;
          } /* if */
          break;
        default:
          shortFileName = FALSE;
          break;
      } /* switch */
      pos++;
      ch = fileName[pos];
    } while (ch != PATH_DELIMITER && ch != '\0');
    if (dotPos == -1) {
      if (pos > 8) {
        shortFileName = FALSE;
      } /* if */
    } else {
      if (dotPos == 0 || dotPos > 8 || pos - dotPos > 4) {
        shortFileName = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("isShortFileName --> %d\n", shortFileName););
    return shortFileName;
  } /* isShortFileName */



static boolType findDot (os_striType name)

  {
    os_charType ch;
    int pos = 0;
    boolType found = FALSE;

  /* findDot */
    ch = name[0];
    while (ch != PATH_DELIMITER && ch != '\0' && !found) {
      if (ch == '.') {
        found = TRUE;
      } /* if */
      pos++;
      ch = name[pos];
    } /* while */
    return found;
  } /* findDot */



static os_striType toShortFileName (os_striType dest, os_striType *sourceAddr)

  {
    os_striType source;
    os_charType ch;
    int sourcePos = 0;
    int destPos = 0;
    int dotPos = -1;
    boolType writeToDest = TRUE;

  /* toShortFileName */
    source = *sourceAddr;
    ch = source[0];
    while (ch != PATH_DELIMITER && ch != '\0') {
      switch (ch) {
        case '!':  case '#':  case '$':  case '%':  case '&':
        case '\'': case '(':  case ')':  case '-':
        case '0':  case '1':  case '2':  case '3':  case '4':
        case '5':  case '6':  case '7':  case '8':  case '9':
        case '@':
        case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
        case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
        case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
        case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
        case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
        case 'Z':
        case '^':  case '_':  case '`':
        case '{':  case '}':  case '~':
          if (writeToDest) {
            dest[destPos] = ch;
            destPos++;
          } /* if */
          break;
        case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
        case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
        case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
        case 'p':  case 'q':  case 'r':  case 's':  case 't':
        case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
        case 'z':
          if (writeToDest) {
            dest[destPos] = (os_charType) toupper((char) ch);
            destPos++;
          } /* if */
          break;
        case '+':  case ',':  case ';':  case '=':  case '[':
        case ']':
          if (writeToDest) {
            dest[destPos] = '_';
            destPos++;
          } /* if */
          break;
        case '.':
          if (!findDot(&source[sourcePos + 1])) {
            if (!writeToDest) {
              dest[destPos] = '~';
              destPos++;
              dest[destPos] = '1';
              destPos++;
              writeToDest = TRUE;
            } /* if */
            dotPos = destPos;
            dest[destPos] = '.';
            destPos++;
          } /* if */
          break;
        default:
          /* Ignore other characters */
          break;
      } /* switch */
      if (dotPos == -1) {
        if (destPos >= 6) {
          writeToDest = FALSE;
        } /* if */
      } else {
        if (destPos - dotPos >= 4) {
          writeToDest = FALSE;
        } /* if */
      } /* if */
      sourcePos++;
      ch = source[sourcePos];
    } /* while */
    if (dotPos == -1) {
      if (!writeToDest) {
        dest[destPos] = '~';
        destPos++;
        dest[destPos] = '1';
        destPos++;
      } /* if */
    } /* if */
    *sourceAddr = &source[sourcePos];
    return &dest[destPos];
  } /* toShortFileName */



static os_striType copyFileName (os_striType dest, os_striType *sourceAddr)

  {
    os_striType source;
    os_charType ch;

  /* copyFileName */
    source = *sourceAddr;
    ch = *source;
    while (ch != PATH_DELIMITER && ch != '\0') {
      *dest = ch;
      dest++;
      source++;
      ch = *source;
    } /* while */
    *sourceAddr = source;
    return dest;
  } /* copyFileName */



static void mapLongFileNamesToShort (os_striType path)

  {
    os_striType source;
    os_striType dest;

  /* mapLongFileNamesToShort */
    logFunction(printf("mapLongFileNamesToShort(\"" FMT_S_OS "\")\n", path););
    /* Start after the colon of the device letter ( e.g.: A: ) */
    source = &path[PREFIX_LEN + 2];
    dest = source;
    while (*source == PATH_DELIMITER) {
      source++;
      *dest = PATH_DELIMITER;
      dest++;
      if (isShortFileName(source)) {
        dest = copyFileName(dest, &source);
      } else {
        dest = toShortFileName(dest, &source);
      } /* if */
    } /* while */
    *dest = '\0';
    logFunction(printf("mapLongFileNamesToShort --> \"" FMT_S_OS "\"\n", path););
  } /* mapLongFileNamesToShort */
#endif



#if OS_PATH_HAS_DRIVE_LETTERS
#if EMULATE_ROOT_CWD
void setEmulatedCwdToRoot (void)

  { /* setEmulatedCwdToRoot */
    logFunction(printf("setEmulatedCwdToRoot\n"););
    if (current_emulated_cwd != NULL &&
        current_emulated_cwd != emulated_root) {
      FREE_OS_STRI((os_striType) current_emulated_cwd);
    } /* if */
    current_emulated_cwd = emulated_root;
    logFunction(printf("setEmulatedCwdToRoot --> current_emulated_cwd=\"" FMT_S_OS "\"\n",
                        current_emulated_cwd););
  } /* setEmulatedCwdToRoot */



void setEmulatedCwd (os_striType os_path, errInfoType *err_info)

  {
    memSizeType cwd_len;
    os_striType new_cwd;
    memSizeType position;

  /* setEmulatedCwd */
    logFunction(printf("setEmulatedCwd(\"" FMT_S_OS "\")\n", os_path););
#if USE_EXTENDED_LENGTH_PATH
    if (memcmp(os_path, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType)) == 0) {
      /* For extended path omit the prefix. */
      os_path = &os_path[PREFIX_LEN];
    } /* if */
#endif
    cwd_len = os_stri_strlen(os_path);
    if (unlikely(!ALLOC_OS_STRI(new_cwd, cwd_len))) {
      *err_info = MEMORY_ERROR;
    } else {
      memcpy(new_cwd, os_path, (cwd_len + NULL_TERMINATION_LEN) * sizeof(os_charType));
      position = os_stri_strlen(new_cwd);
      if (position >= 2 && new_cwd[position - 1] == OS_PATH_DELIMITER) {
        new_cwd[position - 1] = '\0';
      } /* if */
      if (((new_cwd[0] >= 'a' && new_cwd[0] <= 'z') ||
           (new_cwd[0] >= 'A' && new_cwd[0] <= 'Z')) &&
          new_cwd[1] == ':') {
        new_cwd[1] = (os_charType) tolower(new_cwd[0]);
        new_cwd[0] = OS_PATH_DELIMITER;
      } /* if */
      if (current_emulated_cwd != NULL &&
          current_emulated_cwd != emulated_root) {
        FREE_OS_STRI((os_striType) current_emulated_cwd);
      } /* if */
      current_emulated_cwd = new_cwd;
    } /* if */
    logFunction(printf("setEmulatedCwd --> current_emulated_cwd=\"" FMT_S_OS "\"\n",
                        current_emulated_cwd););
  } /* setEmulatedCwd */



static os_striType append_path (const const_os_striType absolutePath,
    const strElemType *const relativePathChars,
    memSizeType relativePathSize, int *path_info, errInfoType *err_info)

  {
    memSizeType abs_path_length;
    memSizeType result_len;
    os_striType abs_path_end;
    const_os_striType rel_path_start;
    os_striType result;

  /* append_path */
    logFunction(printf("append_path(\"" FMT_S_OS "\", \"%s\")\n",
                       absolutePath,
                       striCharsAsUnquotedCStri(relativePathChars,
                                                relativePathSize)););
    /* absolutePath[0] is always '/'. */
    if (absolutePath[1] == '\0') {
      abs_path_length = 0;
    } else {
      abs_path_length = os_stri_strlen(absolutePath);
    } /* if */
    result_len = PREFIX_LEN + abs_path_length +
                 OS_STRI_SIZE(relativePathSize) + 3;
    if (unlikely(!os_stri_alloc(result, result_len))) {
      *err_info = MEMORY_ERROR;
    } else {
#if USE_EXTENDED_LENGTH_PATH
      memcpy(result, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType));
#endif
      memcpy(&result[PREFIX_LEN], absolutePath,
             abs_path_length * sizeof(os_charType));
      abs_path_length += PREFIX_LEN;
      result[abs_path_length] = OS_PATH_DELIMITER;
      /* Leave one char free between absolute and relative path. */
      if (unlikely(!conv_to_os_stri(&result[abs_path_length + 2],
          relativePathChars, relativePathSize, err_info))) {
        os_stri_free(result);
        result = NULL;
      } else {
        abs_path_end = &result[abs_path_length];
        rel_path_start = &result[abs_path_length + 2];
        while (*rel_path_start != '\0') {
          if (rel_path_start[0] == '.' && rel_path_start[1] == '.' &&
              (rel_path_start[2] == '/' || rel_path_start[2] == '\0')) {
            rel_path_start += 2;
            if (abs_path_end > &result[PREFIX_LEN]) {
              do {
                abs_path_end--;
              } while (*abs_path_end != OS_PATH_DELIMITER);
            } /* if */
          } else if (rel_path_start[0] == '.' &&
              (rel_path_start[1] == '/' || rel_path_start[1] == '\0')) {
            rel_path_start++;
          } else if (*rel_path_start == '/') {
            rel_path_start++;
          } else {
            do {
              abs_path_end++;
              *abs_path_end = *rel_path_start;
              rel_path_start++;
            } while (*rel_path_start != '/' && *rel_path_start != '\0');
            abs_path_end++;
            *abs_path_end = OS_PATH_DELIMITER;
          } /* if */
        } /* while */
        if (unlikely(abs_path_end == &result[PREFIX_LEN])) {
          *err_info = RANGE_ERROR;
          *path_info = PATH_IS_EMULATED_ROOT;
          os_stri_free(result);
          result = NULL;
        } else {
          *abs_path_end = '\0';
          if (likely(result[PREFIX_LEN + 1] >= 'a' &&
                     result[PREFIX_LEN + 1] <= 'z')) {
            if (result[PREFIX_LEN + 2] == '\0') {
              /* "/c"   is mapped to "c:\"  */
              result[PREFIX_LEN + 0] = result[PREFIX_LEN + 1];
              result[PREFIX_LEN + 1] = ':';
              result[PREFIX_LEN + 2] = '\\';
              result[PREFIX_LEN + 3] = '\0';
            } else if (unlikely(result[PREFIX_LEN + 2] != OS_PATH_DELIMITER)) {
              /* "/cd"  cannot be mapped to a drive letter. */
              logError(printf("append_path(\"" FMT_S_OS "\"): "
                              "\"/cd\" cannot be mapped to a drive letter.\n",
                              absolutePath););
              *err_info = RANGE_ERROR;
              *path_info = PATH_NOT_MAPPED;
              os_stri_free(result);
              result = NULL;
            } else {
              /* "/c/d" is mapped to "c:\d" */
              result[PREFIX_LEN + 0] = result[PREFIX_LEN + 1];
              result[PREFIX_LEN + 1] = ':';
              result[PREFIX_LEN + 2] = '\\';
            } /* if */
          } else {
            /* "/C"  cannot be mapped to a drive letter. */
            logError(printf("append_path(\"" FMT_S_OS "\"): "
                            "\"/C\" cannot be mapped to a drive letter.\n",
                            absolutePath););
            *err_info = RANGE_ERROR;
            *path_info = PATH_NOT_MAPPED;
            os_stri_free(result);
            result = NULL;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("append_path(\"" FMT_S_OS "\") --> "
                       "\"" FMT_S_OS "\", path_info=%d, err_info=%d\n",
                       absolutePath, result, *path_info, *err_info););
    return result;
  } /* append_path */

#else



static os_striType map_to_drive_letter (const strElemType *const pathChars,
    memSizeType pathSize, int *path_info, errInfoType *err_info)

  {
    os_striType result;

  /* map_to_drive_letter */
    if (unlikely(pathSize == 0)) {
      /* "/"    cannot be mapped to a drive letter */
      *err_info = RANGE_ERROR;
      *path_info = PATH_IS_EMULATED_ROOT;
      result = NULL;
    } else if (likely(pathChars[0] >= 'a' && pathChars[0] <= 'z')) {
      if (pathSize == 1) {
        /* "/c"   is mapped to "c:\"  */
        if (unlikely(!os_stri_alloc(result, 3))) {
          *err_info = MEMORY_ERROR;
        } else {
          result[0] = (os_charType) pathChars[0];
          result[1] = ':';
          result[2] = '\\';
          result[3] = '\0';
        } /* if */
      } else if (unlikely(pathChars[1] != '/')) {
        /* "/cd"  cannot be mapped to a drive letter */
        *err_info = RANGE_ERROR;
        *path_info = PATH_NOT_MAPPED;
        result = NULL;
      } else {
        /* "/c/d" is mapped to "c:\d" */
        if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(pathSize - 2) + 3))) {
          *err_info = MEMORY_ERROR;
        } else {
          result[0] = (os_charType) pathChars[0];
          result[1] = ':';
          result[2] = '\\';
          if (unlikely(!conv_to_os_stri(&result[3], &pathChars[2],
                                        pathSize - 2, err_info))) {
            os_stri_free(result);
            result = NULL;
          } /* if */
        } /* if */
      } /* if */
    } else {
      /* "/C"  cannot be mapped to a drive letter */
      *err_info = RANGE_ERROR;
      *path_info = PATH_NOT_MAPPED;
      result = NULL;
    } /* if */
    return result;
  } /* map_to_drive_letter */

#endif



/**
 *  Convert a Seed7 standard path to a path used by system calls.
 *  The memory for the null terminated os_striType path is allocated.
 *  The os_striType result is allocated with the macro os_stri_alloc()
 *  and it must be freed with the macro os_stri_free(). Strings allocated
 *  with os_stri_alloc() must be freed in the reverse order of their
 *  creation. This allows that allocations work in a stack like manner.
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_striType.
 *  Depending on the operating system os_striType can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion to os_striType a
 *  mapping to drive letters might take place on some operating systems.
 *  @param std_path UTF-32 encoded Seed7 standard path to be converted.
 *  @param path_info Unchanged if the function succeeds, and
 *                   PATH_IS_EMULATED_ROOT if the path is "/", and
 *                   PATH_NOT_MAPPED if the path cannot be mapped.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if the path is not a standard path.
 *  @return a null terminated os_striType path used by system calls, or
 *          NULL if an error occurred.
 */
os_striType cp_to_os_path (const_striType std_path, int *path_info,
    errInfoType *err_info)

  {
    os_striType result;

  /* cp_to_os_path */
    logFunction(printf("cp_to_os_path(\"%s\", %d)\n",
                       striAsUnquotedCStri(std_path), *err_info););
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#if FORBID_DRIVE_LETTERS
    if (unlikely(std_path->size >= 2 && (std_path->mem[std_path->size - 1] == '/' ||
                 (std_path->mem[1] == ':' &&
                 ((std_path->mem[0] >= 'a' && std_path->mem[0] <= 'z') ||
                  (std_path->mem[0] >= 'A' && std_path->mem[0] <= 'Z')))))) {
#else
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/')) {
#endif
#else
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/' &&
                 (std_path->size != 3 || std_path->mem[1] != ':' ||
                 ((std_path->mem[0] < 'a' || std_path->mem[0] > 'z') &&
                  (std_path->mem[0] < 'A' || std_path->mem[0] > 'Z'))))) {
#endif
      logError(printf("cp_to_os_path(\"%s\"): Path with drive letters or not legal.\n",
                      striAsUnquotedCStri(std_path)););
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (unlikely(memchr_strelem(std_path->mem, '\\', std_path->size) != NULL)) {
      logError(printf("cp_to_os_path(\"%s\"): Path contains a backslash.\n",
                      striAsUnquotedCStri(std_path)););
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (std_path->size == 0) {
      if (unlikely(!os_stri_alloc(result, 0))) {
        *err_info = MEMORY_ERROR;
      } else {
        result[0] = '\0';
      } /* if */
    } else {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (std_path->size >= 1 && std_path->mem[0] == '/') {
        /* Absolute path: Try to map the path to a drive letter */
#if EMULATE_ROOT_CWD
        result = append_path(emulated_root, &std_path->mem[1], std_path->size - 1,
                             path_info, err_info);
#else
        result = map_to_drive_letter(&std_path->mem[1], std_path->size - 1, err_info);
#endif
      } else {
#if EMULATE_ROOT_CWD
        result = append_path(current_emulated_cwd, std_path->mem, std_path->size,
                             path_info, err_info);
#else
        if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
          *err_info = MEMORY_ERROR;
        } else if (unlikely(!conv_to_os_stri(result, std_path->mem,
                                             std_path->size, err_info))) {
          os_stri_free(result);
          result = NULL;
        } /* if */
#endif
      } /* if */
#else
      if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
        *err_info = MEMORY_ERROR;
      } else if (unlikely(!conv_to_os_stri(result, std_path->mem,
                                           std_path->size, err_info))) {
        os_stri_free(result);
        result = NULL;
      } /* if */
#endif
    } /* if */
#ifdef MAP_LONG_FILE_NAMES_TO_SHORT
    if (result != NULL) {
      mapLongFileNamesToShort(result);
    } /* if */
#endif
    logFunction(printf("cp_to_os_path(\"%s\", %d) --> \"" FMT_S_OS "\"\n",
                       striAsUnquotedCStri(std_path), *err_info, result););
    return result;
  } /* cp_to_os_path */

#else



/**
 *  Convert a Seed7 standard path to a path used by system calls.
 *  The memory for the null terminated os_striType path is allocated.
 *  The os_striType result is allocated with the macro os_stri_alloc()
 *  and it must be freed with the macro os_stri_free(). Strings allocated
 *  with os_stri_alloc() must be freed in the reverse order of their
 *  creation. This allows that allocations work in a stack like manner.
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_striType.
 *  Depending on the operating system os_striType can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion to os_striType a
 *  mapping to drive letters might take place on some operating systems.
 *  @param std_path UTF-32 encoded Seed7 standard path to be converted.
 *  @param path_info Unchanged if the function succeeds, and
 *                   PATH_IS_EMULATED_ROOT if the path is "/", and
 *                   PATH_NOT_MAPPED if the path cannot be mapped.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if the memory allocation failed, and
 *                  RANGE_ERROR if the path is not a standard path.
 *  @return a null terminated os_striType path used by system calls, or
 *          NULL if an error occurred.
 */
os_striType cp_to_os_path (const_striType std_path, int *path_info,
    errInfoType *err_info)

  {
    os_striType result;

  /* cp_to_os_path */
    logFunction(printf("cp_to_os_path(\"%s\", *)\n",
                       striAsUnquotedCStri(std_path)););
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/')) {
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (unlikely(std_path->size > MAX_OS_STRI_SIZE ||
                        !os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else if (unlikely(!conv_to_os_stri(result, std_path->mem,
                                         std_path->size, err_info))) {
      os_stri_free(result);
      result = NULL;
    } /* if */
    logFunction(printf("cp_to_os_path --> \"" FMT_S_OS "\" "
                       "(path_info=%d, err_info=%d)\n",
                       result, *path_info, *err_info););
    return result;
  } /* cp_to_os_path */

#endif



os_striType temp_name_in_dir (const const_os_striType path)

  {
    memSizeType path_length;
    memSizeType pos;
    uintType random_value;
    unsigned int digit;
    memSizeType temp_length;
    os_striType temp_name;

  /* temp_name_in_dir */
    logFunction(printf("temp_name_in_dir(\"" FMT_S_OS "\")\n", path););
    path_length = os_stri_strlen(path);
    pos = path_length;
    while (pos > 0 && path[pos - 1] != '/' && path[pos - 1] != '\\') {
      pos--;
    } /* while */
    temp_length = pos + 10;
    /* printf("temp_length: %lu\n", temp_length); */
    if (likely(os_stri_alloc(temp_name, temp_length))) {
      memcpy(temp_name, path, pos * sizeof(os_charType));
      random_value = uintRand();
      for (; pos < temp_length; pos++) {
        digit = (unsigned int) (random_value % 36);
        random_value /= 36;
        temp_name[pos] = (os_charType) "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[digit];
      } /* for */
      temp_name[pos] = '\0';
    } /* if */
    logFunction(printf("temp_name_in_dir --> \"" FMT_S_OS "\"\n", temp_name););
    return temp_name;
  } /* temp_name_in_dir */



/**
 *  Add escapes and quotes to a command for system() and popen().
 *  @param inBuffer Null terminated string with the shell command.
 *  @param outBuffer Destination for the processed command.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if there are invalid chars in inBuffer.
 */
static void escape_command (const const_os_striType inBuffer, os_striType outBuffer,
    errInfoType *err_info)

  {
    memSizeType inPos;
    memSizeType outPos;
    boolType quote_path = FALSE;

  /* escape_command */
    logFunction(printf("escape_command(\"" FMT_S_OS "\", *, %d)\n",
                       inBuffer, *err_info););
    for (inPos = 0, outPos = 0; inBuffer[inPos] != '\0'; inPos++, outPos++) {
      switch (inBuffer[inPos]) {
#ifdef ESCAPE_SHELL_COMMANDS
        case '\t': case ' ':  case '!':  case '\"': case '#':
        case '$':  case '&':  case '\'': case '(':  case ')':
        case '*':  case ',':  case ':':  case ';':  case '<':
        case '=':  case '>':  case '?':  case '[':  case '\\':
        case ']':  case '^':  case '`':  case '{':  case '|':
        case '}':  case '~':
          outBuffer[outPos] = '\\';
          outPos++;
          outBuffer[outPos] = inBuffer[inPos];
          break;
        case '\n':
          *err_info = RANGE_ERROR;
          break;
#else
        case ' ':  case '%':  case '&':  case '\'': case '(':
        case ')':  case ',':  case ';':  case '=':  case '^':
        case '~':  case (os_charType) 160:
          quote_path = TRUE;
          outBuffer[outPos] = inBuffer[inPos];
          break;
        case ':':
          if (likely(inPos == 1 &&
                     inBuffer[0] >= 'a' && inBuffer[0] <= 'z')) {
            /* After the drive letter a colon is allowed. */
            outBuffer[outPos] = inBuffer[inPos];
          } else {
            *err_info = RANGE_ERROR;
          } /* if */
          break;
        case '\"': case '*':  case '<':  case '>':  case '?':
        case '|':  case '\n': case '\r':
          *err_info = RANGE_ERROR;
          break;
#endif
#if PATH_DELIMITER != '/'
        case '/':
          outBuffer[outPos] = PATH_DELIMITER;
          break;
#endif
        default:
          outBuffer[outPos] = inBuffer[inPos];
          break;
      } /* switch */
    } /* for */
    if (quote_path) {
      memmove(&outBuffer[1], outBuffer, sizeof(os_charType) * outPos);
      outBuffer[0] = '\"';
      outBuffer[outPos + 1] = '\"';
      outBuffer[outPos + 2] = '\0';
    } else {
      outBuffer[outPos] = '\0';
    } /* if */
    logFunction(printf("escape_command(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d)\n",
                       inBuffer, outBuffer, *err_info););
  } /* escape_command */



/**
 *  Create a command string that is usable for system() and popen().
 *  @param command Name of the command to be executed. A path must
 *                 use the standard path representation.
 *  @param parameters Space separated list of parameters for the
 *                    'command', or "" if there are no parameters.
 *                    Parameters which contain a space must be
 *                    enclosed in double quotes.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  RANGE_ERROR if command or parameters are not okay.
 *  @return command string with all necessary escapes and quotes
 *          such that it can be used for system() and popen(), or
 *          NULL if an error occurred.
 */
os_striType cp_to_command (const const_striType command,
    const const_striType parameters, errInfoType *err_info)

  {
    os_striType os_commandPath;
    os_striType os_parameters;
    memSizeType command_len;
    memSizeType param_len;
    memSizeType result_len;
    int path_info;
    os_striType result;

  /* cp_to_command */
    logFunction(printf("cp_to_command(\"%s\", ",
                       striAsUnquotedCStri(command));
                printf("\"%s\", *)\n",
                       striAsUnquotedCStri(parameters)););
#if EMULATE_ROOT_CWD
    if (memchr_strelem(command->mem, '/', command->size) != NULL) {
      os_commandPath = cp_to_os_path(command, &path_info, err_info);
    } else if (unlikely(memchr_strelem(command->mem, '\\',
                                       command->size) != NULL)) {
      *err_info = RANGE_ERROR;
      os_commandPath = NULL;
    } else {
      os_commandPath = stri_to_os_stri(command, err_info);
    } /* if */
#else
    os_commandPath = cp_to_os_path(command, &path_info, err_info);
#endif
    logMessage(printf("cp_to_command: os_commandPath: \"" FMT_S_OS "\"\n",
                      os_commandPath););
    if (unlikely(os_commandPath == NULL)) {
      result = NULL;
    } else {
      os_parameters = stri_to_os_stri(parameters, err_info);
      if (unlikely(os_parameters == NULL)) {
        result = NULL;
      } else {
        command_len = os_stri_strlen(os_commandPath);
        param_len = os_stri_strlen(os_parameters);
        if (unlikely(MAX_OS_STRI_SIZE - 4 < param_len ||
                     command_len > (MAX_OS_STRI_SIZE - 4 - param_len) / 3)) {
          *err_info = MEMORY_ERROR;
          result = NULL;
        } else {
          result_len = 3 * command_len + param_len + 4;
          if (unlikely(!ALLOC_OS_STRI(result, result_len))) {
            *err_info = MEMORY_ERROR;
          } else {
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
            if (memcmp(os_commandPath, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType)) == 0) {
              escape_command(&os_commandPath[PREFIX_LEN], result, err_info);
            } else {
              escape_command(os_commandPath, result, err_info);
            } /* if */
#else
            escape_command(os_commandPath, result, err_info);
#endif
            if (unlikely(*err_info != OKAY_NO_ERROR)) {
              FREE_OS_STRI(result);
              result = NULL;
            } else {
              result_len = os_stri_strlen(result);
#ifdef QUOTE_WHOLE_SHELL_COMMAND
              if (result[0] == '\"') {
                memmove(&result[1], result, sizeof(os_charType) * result_len);
                result[0] = '\"';
                result_len++;
              } /* if */
#endif
              if (os_parameters[0] != ' ' && os_parameters[0] != '\0') {
                result[result_len] = ' ';
                result_len++;
              } /* if */
              memcpy(&result[result_len], os_parameters,
                     sizeof(os_charType) * (param_len + 1));
#ifdef QUOTE_WHOLE_SHELL_COMMAND
              if (result[0] == '\"' && result[1] == '\"') {
                result_len = os_stri_strlen(result);
                result[result_len] = '\"';
                result[result_len + 1] = '\0';
              } /* if */
#endif
            } /* if */
          } /* if */
        } /* if */
        os_stri_free(os_parameters);
      } /* if */
      os_stri_free(os_commandPath);
    } /* if */
    logFunction(printf("cp_to_command -> " FMT_S_OS "\n", result););
    return result;
  } /* cp_to_command */
