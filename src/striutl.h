/********************************************************************/
/*                                                                  */
/*  striutl.h     Functions to work with wide char strings.         */
/*  Copyright (C) 1989 - 2015  Thomas Mertes                        */
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
/*  File: seed7/src/striutl.h                                       */
/*  Changes: 1991 - 1994, 2005 - 2015  Thomas Mertes                */
/*  Content: Functions to work with wide char strings.              */
/*                                                                  */
/********************************************************************/

/**
 *  Compute strlen() for normal or wide strings at compile time.
 */
#define STRLEN(s) (sizeof(s) / sizeof(s[0]) - 1)

#define MEMCPY_STRING(dest, src) memcpy(dest, src, STRLEN(src));

/* Maximum escape sequence length in string literal: */
#define ESC_SEQUENCE_MAX_LEN STRLEN("\\4294967295;")

#if STRINGIFY_WORKS
#define STRINGIFY(s) STRINGIFY_HELPER(s)
#define STRINGIFY_HELPER(s) #s
#else
#define STRINGIFY(s) stringify((intType) s)
#endif

#define CSTRI_LITERAL_TO_STRI(literal) cstri_buf_to_stri(literal, STRLEN(literal))

#define toStri(name) #name

/** Strings longer than the AND_SO_ON_LIMIT are truncated. */
#define AND_SO_ON_LIMIT     128
#define AND_SO_ON_TEXT      "\\ *AND_SO_ON* SIZE="

/** The AND_SO_ON_LENGTH includes the length of the terminating \0 byte. */
#define AND_SO_ON_LENGTH    (STRLEN(AND_SO_ON_TEXT) + MEMSIZETYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN)

/**
 *  The maximum width when an UTF-32 character is displayed
 *  in a literal is 12 characters (e.g.: \4294967295; ).
 */
#define MAXIMUM_UTF32_ESCAPE_WIDTH STRLEN("\\4294967295;")

/**
 *  The maximum width when an unsigned char (byte) is displayed
 *  in a literal is 5 characters (e.g.: \255; ).
 */
#define MAXIMUM_BYTE_ESCAPE_WIDTH STRLEN("\\255;")

extern const const_cstriType stri_escape_sequence[];
extern const const_cstriType cstri_escape_sequence[];

#define MAX_UTF8_EXPANSION_FACTOR 6
#define max_utf8_size(size) (MAX_UTF8_EXPANSION_FACTOR * (size))
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, (stri)->size);
#define free_cstri8(cstri,stri) UNALLOC_CSTRI(cstri, max_utf8_size((stri)->size));

/**
 *  UTF-16 encodes characters > 0xffff with surrogate pairs.
 *  When converting to UTF-16 it might be necessary to store
 *  every character with surrogate pairs (= two UTF-16 chars).
 */
#define SURROGATE_PAIR_FACTOR  2


#ifdef OS_STRI_WCHAR
typedef wchar_t          os_charType;
typedef uint16Type       os_ucharType;
typedef wchar_t         *os_striType;
typedef const wchar_t   *const_os_striType;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
#define os_stri_strchr   wcschr
#define os_stri_strrchr  wcsrchr
#define ALLOC_OS_STRI(var,len)    ALLOC_HEAP(var, os_striType, SIZ_UTF16(len))
#define REALLOC_OS_STRI(var,len)  ((os_striType) REALLOC_HEAP(var, ustriType, SIZ_UTF16(len)))
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_UTF16
#define MAX_OS_STRI_LEN  MAX_UTF16_LEN
#define FMT_S_OS "%ls"
#else
typedef char             os_charType;
typedef unsigned char    os_ucharType;
typedef cstriType        os_striType;
typedef const_cstriType  const_os_striType;
#define os_stri_strlen   strlen
#define os_stri_strcpy   strcpy
#define os_stri_strcat   strcat
#define os_stri_strchr   strchr
#define os_stri_strrchr  strrchr
#define ALLOC_OS_STRI    ALLOC_CSTRI
#define REALLOC_OS_STRI  REALLOC_CSTRI
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_CSTRI
#define MAX_OS_STRI_LEN  MAX_CSTRI_LEN
#define FMT_S_OS "%s"
#endif


#if STACK_LIKE_ALLOC_FOR_OS_STRI
typedef struct stackAllocStruct *stackAllocType;

typedef struct stackAllocStruct {
    stackAllocType previous;
    cstriType beyond;
    cstriType curr_free;
    char start[1];
  } stackAllocRecord;

#ifdef DO_INIT
stackAllocRecord stack_alloc_base = {NULL, NULL, NULL, {' '}};
stackAllocType   stack_alloc = &stack_alloc_base;
#else
EXTERN stackAllocRecord stack_alloc_base;
EXTERN stackAllocType   stack_alloc;
#endif

#define SIZ_STACK_ALLOC(len)   ((sizeof(stackAllocRecord) - sizeof(char)) + \
                               (len) * sizeof(os_charType))
/* One is subtracted in the macro MAX_STACK_ALLOC to make */
/* sure that stack_alloc->beyond does not wrap around.    */
#define MAX_STACK_ALLOC        (MAX_MEMSIZETYPE - 1 - \
                               (sizeof(stackAllocRecord) - sizeof(char)) / sizeof(os_charType))
#define POP_OS_STRI_OK(len)    (memSizeType) stack_alloc->beyond >= (len) && \
                               (memSizeType) (stack_alloc->beyond - (len)) >= \
                               (memSizeType) stack_alloc->curr_free
#define PUSH_OS_STRI_OK(var)   (memSizeType) (var) >= (memSizeType) stack_alloc->start && \
                               (memSizeType) (var) < (memSizeType) stack_alloc->curr_free
#define POP_OS_STRI(var,len)   (var = (os_striType) stack_alloc->curr_free, \
                               stack_alloc->curr_free += (len), TRUE)
#define PUSH_OS_STRI(var)      { stack_alloc->curr_free = (cstriType) (var); }
#define os_stri_alloc(var,len) (POP_OS_STRI_OK(SIZ_OS_STRI(len)) ? \
                               POP_OS_STRI(var, SIZ_OS_STRI(len)) : \
                               (var = heapAllocOsStri(len)) != NULL)
#define os_stri_free(var)      if (PUSH_OS_STRI_OK(var)) PUSH_OS_STRI(var) \
                               else heapFreeOsStri(var);
#else
#define os_stri_alloc ALLOC_OS_STRI
#define os_stri_free  FREE_OS_STRI
#endif


#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#if EMULATE_ROOT_CWD
/* Assume that drive letters are used only with a backslash as path delimiter. */
#define OS_PATH_DELIMITER '\\'
extern const_os_striType current_emulated_cwd;
#endif
extern const os_charType emulated_root[];

#define IS_EMULATED_ROOT(os_path) (os_path == emulated_root)

#if USE_EXTENDED_LENGTH_PATH
#define PATH_PREFIX L"\\\\?\\"
#else
#define PATH_PREFIX L""
#endif
#define PREFIX_LEN STRLEN(PATH_PREFIX)
#endif

#define PATH_IS_NORMAL        0
#define PATH_IS_EMULATED_ROOT 1
#define PATH_NOT_MAPPED       2


cstriType striAsUnquotedCStri (const const_striType stri);
cstriType bstriAsUnquotedCStri (const const_bstriType bstri);
#if !STRINGIFY_WORKS
cstriType stringify (intType number);
#endif
void memcpy_to_strelem (register strElemType *const dest,
                        register const const_ustriType src, memSizeType len);
void memset_to_strelem (register strElemType *const dest,
                        register const strElemType ch, memSizeType len);
boolType memcpy_from_strelem (register const ustriType dest,
                              register const strElemType *const src,
                              memSizeType len);
#if HAS_WMEMCHR && WCHAR_T_SIZE == 32
#define memchr_strelem(mem, ch, len) \
    (const strElemType *) wmemchr((const wchar_t *) mem, (wchar_t) ch, (size_t) len)
#else
const strElemType *memchr_strelem (register const strElemType *mem,
    const strElemType ch, memSizeType len);
#endif
#if STACK_LIKE_ALLOC_FOR_OS_STRI
os_striType heapAllocOsStri (memSizeType len);
void heapFreeOsStri (const_os_striType var);
#endif
#ifdef OS_STRI_USES_CODE_PAGE
charType mapFromCodePage (unsigned char key);
#endif
memSizeType stri8_to_stri (strElemType *const dest_stri, memSizeType *const dest_len,
                           const strElemType *stri8, memSizeType len);
memSizeType utf8_to_stri (strElemType *const dest_stri, memSizeType *const dest_len,
                          const_ustriType ustri, memSizeType len);
memSizeType utf8_bytes_missing (const const_ustriType ustri, const memSizeType len);
memSizeType stri_to_utf8 (const ustriType out_stri,
                          const strElemType *strelem, memSizeType len);
cstriType conv_to_cstri (cstriType cstri, const const_striType stri);
void conv_to_cstri8 (cstriType cstri, const const_striType stri,
                     errInfoType *err_info);
memSizeType stri_to_utf16 (const utf16striType out_wstri,
                           register const strElemType *strelem, memSizeType len,
                           errInfoType *const err_info);
cstriType stri_to_cstri (const const_striType stri, errInfoType *err_info);
cstriType stri_to_cstri8 (const const_striType stri, errInfoType *err_info);
cstriType stri_to_cstri8_buf (const const_striType stri, memSizeType *length);
bstriType stri_to_bstri (const const_striType stri, errInfoType *err_info);
bstriType stri_to_bstri8 (const const_striType stri);
#ifdef CONSOLE_WCHAR
bstriType stri_to_bstriw (const const_striType stri, errInfoType *err_info);
#endif
utf16striType stri_to_wstri16 (const const_striType stri, memSizeType *length,
                               errInfoType *err_info);
utf32striType stri_to_wstri32 (const const_striType stri, memSizeType *length,
                               errInfoType *err_info);
striType copy_stri (const const_striType source);
striType cstri_to_stri (const_cstriType cstri);
striType cstri_buf_to_stri (const_cstriType cstri, memSizeType length);
striType cstri8_to_stri (const_cstriType cstri, errInfoType *err_info);
striType cstri8_buf_to_stri (const_cstriType cstri, memSizeType length,
                             errInfoType *err_info);
striType cstri8_or_cstri_to_stri (const_cstriType cstri);
striType wstri16_to_stri (const_utf16striType wstri, memSizeType length,
                          errInfoType *err_info);
striType wstri32_to_stri (const_utf32striType wstri, memSizeType length,
                          errInfoType *err_info);
striType conv_from_os_stri (const const_os_striType os_stri, memSizeType length);
os_striType stri_to_os_stri (const_striType stri, errInfoType *err_info);
striType os_stri_to_stri (const const_os_striType os_stri, errInfoType *err_info);
striType stri_to_standard_path (const striType stri);
striType cp_from_os_path (const_os_striType os_path, errInfoType *err_info);
striType cp_from_os_path_buffer (const_os_striType os_path,
                                 memSizeType length, errInfoType *err_info);
#if EMULATE_ROOT_CWD
void setEmulatedCwdToRoot (void);
void setEmulatedCwd (os_striType os_path, errInfoType *err_info);
#endif
os_striType cp_to_os_path (const_striType std_path, int *path_info,
    errInfoType *err_info);
os_striType temp_name_in_dir (const const_os_striType path);
os_striType cp_to_command (const const_striType command,
    const const_striType parameters, errInfoType *err_info);
