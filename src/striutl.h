/********************************************************************/
/*                                                                  */
/*  striutl.h     Procedures to work with wide char strings.        */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2005 - 2014  Thomas Mertes                */
/*  Content: Procedures to work with wide char strings.             */
/*                                                                  */
/********************************************************************/

extern const_cstriType stri_escape_sequence[];
extern const_cstriType cstri_escape_sequence[];

#define MAX_UTF8_EXPANSION_FACTOR 6
#define max_utf8_size(size) (MAX_UTF8_EXPANSION_FACTOR * (size))
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, (stri)->size);
#define free_cstri8(cstri,stri) UNALLOC_CSTRI(cstri, max_utf8_size((stri)->size));
#define free_wstri(wstri,stri) free(wstri);


#ifdef OS_STRI_WCHAR
typedef wchar_t          os_charType;
typedef uint16Type       os_ucharType;
typedef wchar_t         *os_striType;
typedef const wchar_t   *const_os_striType;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
#define os_stri_strchr   wcschr
#define ALLOC_OS_STRI(var,len)    ALLOC_HEAP(var, os_striType, SIZ_WSTRI(len))
#define REALLOC_OS_STRI(var,len)  ((os_striType) REALLOC_HEAP(var, ustriType, SIZ_WSTRI(len)))
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_WSTRI
#define MAX_OS_STRI_LEN  MAX_WSTRI_LEN
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
#define ALLOC_OS_STRI    ALLOC_CSTRI
#define REALLOC_OS_STRI  REALLOC_CSTRI
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_CSTRI
#define MAX_OS_STRI_LEN  MAX_CSTRI_LEN
#define FMT_S_OS "%s"
#endif


#ifdef STACK_LIKE_ALLOC_FOR_OS_STRI
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

#define SIZ_STACK_ALLOC(len)   ((sizeof(stackAllocRecord) - sizeof(char)) + (len) * sizeof(os_charType))
/* One is subtracted in the macro MAX_STACK_ALLOC to make */
/* sure that stack_alloc->beyond does not wrap around.    */
#define MAX_STACK_ALLOC        (MAX_MEMSIZETYPE - (sizeof(stackAllocRecord) - sizeof(char)) / sizeof(os_charType) - 1)
#define POP_OS_STRI_OK(len)    (memSizeType) stack_alloc->beyond >= (len) && \
                               (memSizeType) (stack_alloc->beyond - (len)) >= (memSizeType) stack_alloc->curr_free
#define PUSH_OS_STRI_OK(var)   (memSizeType) (var) >= (memSizeType) stack_alloc->start && \
                               (memSizeType) (var) < (memSizeType) stack_alloc->curr_free
#define POP_OS_STRI(var,len)   (var = (os_striType) stack_alloc->curr_free, stack_alloc->curr_free += (len), TRUE)
#define PUSH_OS_STRI(var)      { stack_alloc->curr_free = (cstriType) (var); }
#define os_stri_alloc(var,len) (POP_OS_STRI_OK(SIZ_OS_STRI(len)) ? POP_OS_STRI(var, SIZ_OS_STRI(len)) : heapAllocOsStri(&(var), len))
#define os_stri_free(var)      if (PUSH_OS_STRI_OK(var)) PUSH_OS_STRI(var) else heapFreeOsStri(var);
#else
#define os_stri_alloc ALLOC_OS_STRI
#define os_stri_free  FREE_OS_STRI
#endif


#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef EMULATE_ROOT_CWD
extern os_striType current_emulated_cwd;
#endif
extern os_charType emulated_root[];

#define IS_EMULATED_ROOT(os_path) (os_path == emulated_root)
#endif

#define PATH_IS_NORMAL        0
#define PATH_IS_EMULATED_ROOT 1
#define PATH_NOT_MAPPED       2


void memcpy_to_strelem (register strElemType *const dest,
                        register const const_ustriType src, memSizeType len);
void memset_to_strelem (register strElemType *const dest,
                        register const strElemType ch, memSizeType len);
#ifdef STACK_LIKE_ALLOC_FOR_OS_STRI
boolType heapAllocOsStri (os_striType *var, memSizeType len);
void heapFreeOsStri (const_os_striType var);
#endif
memSizeType utf8_to_stri (strElemType *const dest_stri, memSizeType *const dest_len,
                          const_ustriType ustri, size_t len);
memSizeType utf8_bytes_missing (const const_ustriType ustri, const size_t len);
memSizeType stri_to_utf8 (const ustriType out_stri,
                          const strElemType *strelem, memSizeType len);
void conv_to_cstri (cstriType cstri, const const_striType stri,
                    errInfoType *err_info);
void conv_to_cstri8 (cstriType cstri, const const_striType stri,
                     errInfoType *err_info);
memSizeType stri_to_utf16 (const wstriType out_wstri,
                           register const strElemType *strelem, memSizeType len,
                           errInfoType *const err_info);
cstriType stri_to_cstri (const const_striType stri, errInfoType *err_info);
cstriType stri_to_cstri8 (const const_striType stri, errInfoType *err_info);
cstriType stri_to_cstri8_buf (const const_striType stri, memSizeType *length,
                              errInfoType *err_info);
bstriType stri_to_bstri (const const_striType stri, errInfoType *err_info);
bstriType stri_to_bstri8 (const_striType stri);
#ifdef CONSOLE_WCHAR
bstriType stri_to_bstriw (const_striType stri);
#endif
wstriType stri_to_wstri_buf (const const_striType stri, memSizeType *length,
                             errInfoType *err_info);
striType cstri_to_stri (const_cstriType cstri);
striType cstri_buf_to_stri (const_cstriType cstri, memSizeType length);
striType cstri8_to_stri (const_cstriType cstri, errInfoType *err_info);
striType cstri8_buf_to_stri (const_cstriType cstri, memSizeType length,
                             errInfoType *err_info);
striType cstri8_or_cstri_to_stri (const_cstriType cstri);
striType wstri_buf_to_stri (const_wstriType wstri, memSizeType length,
                            errInfoType *err_info);
striType conv_from_os_stri (const const_os_striType os_stri, memSizeType length);
os_striType stri_to_os_stri (const_striType stri, errInfoType *err_info);
striType os_stri_to_stri (const_os_striType os_stri, errInfoType *err_info);
striType stri_to_standard_path (const striType stri);
striType cp_from_os_path (const_os_striType os_path, errInfoType *err_info);
#ifdef EMULATE_ROOT_CWD
void setEmulatedCwd (const os_striType os_path, errInfoType *err_info);
#endif
os_striType cp_to_os_path (const_striType std_path, int *path_info,
    errInfoType *err_info);
os_striType temp_name_in_dir (const const_os_striType path);
os_striType cp_to_command (const const_striType commandPath,
    const const_striType parameters, errInfoType *err_info);
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
striType relativeToProgramPath (const const_striType basePath,
    const const_cstriType dir);
#endif
