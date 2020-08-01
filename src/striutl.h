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

extern const_cstritype stri_escape_sequence[];
extern const_cstritype cstri_escape_sequence[];

#define MAX_UTF8_EXPANSION_FACTOR 6
#define max_utf8_size(size) (MAX_UTF8_EXPANSION_FACTOR * (size))
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, (stri)->size);
#define free_cstri8(cstri,stri) UNALLOC_CSTRI(cstri, max_utf8_size((stri)->size));
#define free_wstri(wstri,stri) free(wstri);
#define cstri_expand(stri,cstri,size) ustri_expand(stri, (const_ustritype) cstri, size)
#define cstri_expand2(stri,cstri) ustri_expand2(stri, (const_ustritype) cstri)

#define USE_DUFFS_UNROLLING
#ifdef USE_DUFFS_UNROLLING

#define memcpy_to_strelem(dest,src,len) \
    if (len != 0) { \
      register memsizetype pos = (len + 7) & ~(memsizetype) 7; \
      switch (len & 7) { \
        case 0: do { dest[pos - 1] = src[pos - 1]; \
        case 7:      dest[pos - 2] = src[pos - 2]; \
        case 6:      dest[pos - 3] = src[pos - 3]; \
        case 5:      dest[pos - 4] = src[pos - 4]; \
        case 4:      dest[pos - 5] = src[pos - 5]; \
        case 3:      dest[pos - 6] = src[pos - 6]; \
        case 2:      dest[pos - 7] = src[pos - 7]; \
        case 1:      dest[pos - 8] = src[pos - 8]; \
                } while((pos -= 8) > 0); \
      } /* switch */\
    } /* if */

#define memset_to_strelem(dest,ch,len) \
    if (len != 0) { \
      register memsizetype pos = (len + 7) & ~(memsizetype) 7; \
      switch (len & 7) { \
        case 0: do { dest[pos - 1] = ch; \
        case 7:      dest[pos - 2] = ch; \
        case 6:      dest[pos - 3] = ch; \
        case 5:      dest[pos - 4] = ch; \
        case 4:      dest[pos - 5] = ch; \
        case 3:      dest[pos - 6] = ch; \
        case 2:      dest[pos - 7] = ch; \
        case 1:      dest[pos - 8] = ch; \
                } while((pos -= 8) > 0); \
      } /* switch */ \
    } /* if */

#else

#define memcpy_to_strelem(dest,src,len) \
    { register memsizetype pos = len; \
      for (; pos > 0; pos--) { \
        dest[pos - 1] = src[pos - 1]; \
      } /* for */ \
    }

#define memset_to_strelem(dest,ch,len) \
    { register memsizetype pos = len; \
    for (; pos > 0; pos--) { \
      dest[pos - 1] = (strelemtype) ch; \
    } /* for */

#endif


#ifdef OS_STRI_WCHAR
typedef wchar_t          os_chartype;
typedef uint16type       os_uchartype;
typedef wstritype        os_stritype;
typedef const_wstritype  const_os_stritype;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
#define os_stri_strchr   wcschr
#define ALLOC_OS_STRI    ALLOC_WSTRI
#define REALLOC_OS_STRI  REALLOC_WSTRI
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_WSTRI
#define MAX_OS_STRI_LEN  MAX_WSTRI_LEN
#else
typedef char             os_chartype;
typedef unsigned char    os_uchartype;
typedef cstritype        os_stritype;
typedef const_cstritype  const_os_stritype;
#define os_stri_strlen   strlen
#define os_stri_strcpy   strcpy
#define os_stri_strcat   strcat
#define os_stri_strchr   strchr
#define ALLOC_OS_STRI    ALLOC_CSTRI
#define REALLOC_OS_STRI  REALLOC_CSTRI
#define FREE_OS_STRI     free
#define SIZ_OS_STRI      SIZ_CSTRI
#define MAX_OS_STRI_LEN  MAX_CSTRI_LEN
#endif


#ifdef STACK_LIKE_ALLOC_FOR_OS_STRI
typedef struct stackAllocStruct {
    struct stackAllocStruct *previous;
    cstritype beyond;
    cstritype curr_free;
    char start[1];
  } stackAllocRecord, *stackAllocType;

#ifdef DO_INIT
stackAllocRecord stack_alloc_base = {NULL, NULL, NULL, {' '}};
stackAllocType   stack_alloc = &stack_alloc_base;
#else
EXTERN stackAllocRecord stack_alloc_base;
EXTERN stackAllocType   stack_alloc;
#endif

#define SIZ_STACK_ALLOC(len)   ((sizeof(stackAllocRecord) - sizeof(char)) + (len) * sizeof(os_chartype))
/* One is subtracted in the macro MAX_STACK_ALLOC to make */
/* sure that stack_alloc->beyond does not wrap around.    */
#define MAX_STACK_ALLOC        (MAX_MEMSIZETYPE - (sizeof(stackAllocRecord) - sizeof(char)) / sizeof(os_chartype) - 1)
#define POP_OS_STRI_OK(len)    (memsizetype) stack_alloc->beyond >= (len) && \
                               (memsizetype) (stack_alloc->beyond - (len)) >= (memsizetype) stack_alloc->curr_free
#define PUSH_OS_STRI_OK(var)   (memsizetype) (var) >= (memsizetype) stack_alloc->start && \
                               (memsizetype) (var) < (memsizetype) stack_alloc->curr_free
#define POP_OS_STRI(var,len)   (var = (os_stritype) stack_alloc->curr_free, stack_alloc->curr_free += (len), TRUE)
#define PUSH_OS_STRI(var)      { stack_alloc->curr_free = (cstritype) (var); }
#define os_stri_alloc(var,len) (POP_OS_STRI_OK(SIZ_OS_STRI(len)) ? POP_OS_STRI(var, SIZ_OS_STRI(len)) : heapAllocOsStri(&(var), len))
#define os_stri_free(var)      if (PUSH_OS_STRI_OK(var)) PUSH_OS_STRI(var) else heapFreeOsStri(var);
#else
#define os_stri_alloc ALLOC_OS_STRI
#define os_stri_free  FREE_OS_STRI
#endif


#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef EMULATE_ROOT_CWD
extern os_stritype current_emulated_cwd;
#endif
extern os_chartype emulated_root[];

#define IS_EMULATED_ROOT(os_path) (os_path == emulated_root)
#endif

#define PATH_IS_NORMAL        0
#define PATH_IS_EMULATED_ROOT 1
#define PATH_NOT_MAPPED       2


#ifdef STACK_LIKE_ALLOC_FOR_OS_STRI
booltype heapAllocOsStri (os_stritype *var, memsizetype len);
void heapFreeOsStri (os_stritype var);
#endif
memsizetype utf8_to_stri (strelemtype *const dest_stri, memsizetype *const dest_len,
                          const_ustritype ustri, size_t len);
memsizetype utf8_bytes_missing (const const_ustritype ustri, const size_t len);
memsizetype stri_to_utf8 (const ustritype out_stri,
                          const strelemtype *strelem, memsizetype len);
void conv_to_cstri (cstritype cstri, const const_stritype stri,
                    errinfotype *err_info);
void conv_to_cstri8 (cstritype cstri, const const_stritype stri,
                     errinfotype *err_info);
void ustri_expand (strelemtype *const stri, const const_ustritype ustri, size_t len);
size_t ustri_expand2 (strelemtype *const stri, const_ustritype ustri);
#ifdef OS_STRI_WCHAR
memsizetype stri_to_wstri (const wstritype out_wstri,
                           register const strelemtype *strelem, memsizetype len,
                           errinfotype *const err_info);
#endif
cstritype stri_to_cstri (const const_stritype stri, errinfotype *err_info);
cstritype stri_to_cstri8 (const const_stritype stri, errinfotype *err_info);
bstritype stri_to_bstri (const const_stritype stri, errinfotype *err_info);
bstritype stri_to_bstri8 (const_stritype stri);
#ifdef CONSOLE_WCHAR
bstritype stri_to_bstriw (const_stritype stri);
#endif
stritype cstri_to_stri (const_cstritype cstri);
stritype cstri8_to_stri (const_cstritype cstri);
stritype cstri8_or_cstri_to_stri (const_cstritype cstri);
stritype conv_from_os_stri (const const_os_stritype os_stri, memsizetype length);
os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info);
stritype os_stri_to_stri (const_os_stritype os_stri, errinfotype *err_info);
stritype stri_to_standard_path (const stritype stri);
stritype cp_from_os_path (const_os_stritype os_path, errinfotype *err_info);
#ifdef EMULATE_ROOT_CWD
void setEmulatedCwd (const os_stritype os_path, errinfotype *err_info);
#endif
os_stritype cp_to_os_path (const_stritype std_path, int *path_info,
    errinfotype *err_info);
os_stritype cp_to_command (const const_stritype commandPath,
    const const_stritype parameters, errinfotype *err_info);
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir);
#endif
