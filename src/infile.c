/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Analyzer - Infile                                       */
/*  File: seed7/src/infile.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures to open, close and read the source file.    */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "fatal.h"
#include "error.h"
#include "info.h"
#include "stat.h"
#include "option.h"

#ifdef USE_MMAP
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/mman.h"
#endif

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "infile.h"


#define SIZE_IN_BUFFER 32768


static infiltype file_pointer = NULL;
static filenumtype file_counter = 0;



#ifdef USE_ALTERNATE_NEXT_CHARACTER
#ifdef ANSI_C

int fill_buf (void)
#else

int fill_buf ()
#endif

  {
#ifndef USE_MMAP
    memsizetype chars_read;
#endif
    int result;

  /* fill_buf */
#ifdef TRACE_INFILE
    printf("BEGIN fill_buf\n");
#endif
#ifdef USE_MMAP
    result = EOF;
#else
    if ((chars_read = fread(in_file.start, 1,
        (SIZE_TYPE) in_file.buffer_size, in_file.fil)) != 0) {
      in_file.nextch = in_file.start;
      in_file.byond = in_file.start + chars_read;
      result = next_character();
    } else {
      result = EOF;
    } /* if */
#endif
#ifdef TRACE_INFILE
    printf("END fill_buf\n");
#endif
    return(result);
  } /* fill_buf */
#endif



#ifdef ANSI_C

static INLINE void make_os_file_name (stritype source_file_name,
    ustritype *os_file_name, errinfotype *err_info)
#else

static INLINE void make_os_file_name (source_file_name,
    os_file_name, err_info)
stritype source_file_name;
ustritype *os_file_name;
errinfotype *err_info;
#endif

  /* Copies the source_file_name to os_file_name and replaces all    */
  /* occurances of '/' and '\' in os_file_name by PATH_DELIMITER.    */

  {
    unsigned int position;
    unsigned int len;

  /* make_os_file_name */
#ifdef TRACE_INFILE
    printf("BEGIN make_os_file_name\n");
#endif
    len = source_file_name->size;
    if (!ALLOC_USTRI(*os_file_name, len)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_USTRI(len, count.fnam, count.fnam_bytes);
      for (position = 0; position < len; position++) {
        (*os_file_name)[position] = source_file_name->mem[position];
        if (source_file_name->mem[position] == '/' ||
            source_file_name->mem[position] == '\\') {
          (*os_file_name)[position] = PATH_DELIMITER;
        } /* if */
      } /* for */
      (*os_file_name)[len] = '\0';
    } /* if */
#ifdef TRACE_INFILE
    printf("END make_os_file_name\n");
#endif
  } /* make_os_file_name */



#ifdef ANSI_C

static INLINE booltype speedup (void)
#else

static INLINE booltype speedup ()
#endif

  {
    booltype okay;
#ifdef USE_MMAP
    struct stat file_stat;
    memsizetype file_length;
#endif

  /* speedup */
#ifdef TRACE_INFILE
    printf("BEGIN speedup\n");
#endif
    okay = TRUE;
#ifdef USE_ALTERNATE_NEXT_CHARACTER
#ifdef USE_MMAP
    if (fstat(fileno(in_file.fil), &file_stat) == 0) {
      file_length = file_stat.st_size;
      if ((in_file.start = (ustritype) mmap(NULL, file_length,
          PROT_READ, MAP_PRIVATE, fileno(in_file.fil),
          0)) != (ustritype) -1) {
        in_file.nextch = in_file.start;
        in_file.byond = in_file.start + file_length;
        in_file.buffer_size = 0;
      } else {
        if (ALLOC_BYTES(in_file.start, file_length)) {
          COUNT_BYTES(file_length);
          if (fread(in_file.start, 1, file_length, in_file.fil) ==
              file_length) {
            in_file.nextch = in_file.start;
            in_file.byond = in_file.start + file_length;
            in_file.buffer_size = file_length;
            fseek(in_file.fil, 0, SEEK_SET);
          } else {
            FREE_BYTES(in_file.start, file_length);
            okay = FALSE;
          } /* if */
        } else {
          okay = FALSE;
        } /* if */
      } /* if */
    } else {
      okay = FALSE;
    } /* if */
#else
    if (option.get_infile_buffer) {
      in_file.buffer_size = SIZE_IN_BUFFER;
    } else {
      in_file.buffer_size = 512;
    } /* if */
    if (ALLOC_BYTES(in_file.start, in_file.buffer_size)) {
      COUNT_BYTES(in_file.buffer_size);
      in_file.nextch = in_file.start + in_file.buffer_size;
      in_file.byond = in_file.start;
    } else {
      in_file.buffer_size = 512;
      if (ALLOC_BYTES(in_file.start, in_file.buffer_size)) {
        COUNT_BYTES(in_file.buffer_size);
        in_file.nextch = in_file.start + in_file.buffer_size;
        in_file.byond = in_file.start;
      } else {
        okay = FALSE;
      } /* if */
    } /* if */
#endif
#else
#ifdef USE_INFILE_BUFFER
    if (option.get_infile_buffer) {
      if (ALLOC_BYTES(in_file.buffer, SIZE_IN_BUFFER)) {
        COUNT_BYTES(SIZE_IN_BUFFER);
        setvbuf(in_file.fil, in_file.buffer, _IOFBF,
            (SIZE_TYPE) SIZE_IN_BUFFER);
      } /* if */
    } else {
      in_file.buffer = NULL;
    } /* if */
#endif
#endif
#ifdef TRACE_INFILE
    printf("END speedup\n");
#endif
    return(okay);
  } /* speedup */



#ifdef ANSI_C

void open_infile (stritype source_file_name, errinfotype *err_info)
#else

void open_infile (source_file_name, err_info)
stritype source_file_name;
errinfotype *err_info;
#endif

  {
    ustritype os_file_name;
    infiltype new_file;
    FILE *in_fil;

  /* open_infile */
#ifdef TRACE_INFILE
    printf("BEGIN open_infile\n");
#endif
    make_os_file_name(source_file_name, &os_file_name, err_info);
    if (*err_info == NO_ERROR) {
      in_fil = fopen((cstritype) os_file_name, "rb");
      /* printf("fopen(\"%s\") ==> %lu\n", os_file_name, in_fil); */
      FREE_USTRI(os_file_name, strlen((cstritype) os_file_name),
          count.fnam, count.fnam_bytes);
      if (in_fil == NULL) {
        *err_info = FILE_ERROR;
      } else {
        new_file = NULL;
/*      if (in_file.fil != NULL) { */
        if (!in_file.end_of_file) {
          if (!ALLOC_FILE(new_file)) {
            *err_info = MEMORY_ERROR;
          } else {
            memcpy(new_file, &in_file, sizeof(infilrecord));
            in_file.next = new_file;
          } /* if */
        } /* if */
        if (*err_info == NO_ERROR) {
          in_file.fil = in_fil;
          if (!speedup()) {
            fclose(in_file.fil);
            if (new_file != NULL) {
              memcpy(&in_file, new_file, sizeof(infilrecord));
              FREE_FILE(new_file);
            } else {
              in_file.fil = NULL;
            } /* if */
          } else {
            in_file.name = cp_to_cstri(source_file_name);
            if (in_file.name == NULL) {
              *err_info = MEMORY_ERROR;
            } else {
              in_file.character = next_character();
              in_file.line = 1;
              in_file.next_msg_line = 1 + option.incr_message_line;
              file_counter++;
              in_file.file_number = file_counter;
              open_compilation_info();
              in_file.end_of_file = FALSE;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_INFILE
    printf("END open_infile\n");
#endif
  } /* open_infile */



#ifdef ANSI_C

void close_infile (void)
#else

void close_infile ()
#endif

  {
    infiltype help_file;
    infilrecord in_file_temp;

  /* close_infile */
#ifdef TRACE_INFILE
    printf("BEGIN close_infile\n");
#endif
/*  printf("\nclose(\"%s\");\n", in_file.name); */
#ifdef WITH_COMPILATION_INFO
    if (option.compilation_info) {
      NL_LIN_INFO();
    } else {
      if (option.linecount_info) {
        NL_FIL_LIN_INFO();
      } /* if */
    } /* if */
    total_lines = total_lines + ((unsigned long) in_file.line);
#endif
#ifdef USE_ALTERNATE_NEXT_CHARACTER
    if (in_file.start != NULL) {
      if (in_file.buffer_size == 0) {
#ifdef USE_MMAP
        if (in_file.fil != NULL) {
          munmap(in_file.start, (SIZE_TYPE) (in_file.byond - in_file.start));
        } /* if */
#endif
      } else {
        FREE_BYTES(in_file.start, in_file.buffer_size);
      } /* if */
    } /* if */
#endif
    if (in_file.fil != NULL) {
      fclose(in_file.fil);
      in_file.fil = NULL;
    } /* if */
#ifndef USE_ALTERNATE_NEXT_CHARACTER
#ifdef USE_INFILE_BUFFER
    if (in_file.BUFFER != NULL) {
      FREE_BYTES(in_file.buffer, SIZE_IN_BUFFER);
    } /* if */
#endif
#endif
/*  FREE_USTRI(in_file.name, strlen((cstritype) in_file.name),
        count.fnam, count.fnam_bytes); */
    if (in_file.next != NULL) {
      help_file = in_file.next;
      memcpy(&in_file_temp, &in_file, sizeof(infilrecord));
      memcpy(&in_file, in_file.next, sizeof(infilrecord));
      memcpy(help_file, &in_file_temp, sizeof(infilrecord));
      display_compilation_info();
    } else {
      if (!ALLOC_FILE(help_file)) {
        fatal_memory_error(SOURCE_POSITION(2023));
      } else {
        memcpy(help_file, &in_file, sizeof(infilrecord));
      } /* if */
    } /* if */
    help_file->next = file_pointer;
    file_pointer = help_file;
    in_file.next_msg_line = in_file.line + option.incr_message_line;
#ifdef TRACE_INFILE
    printf("END close_infile\n");
#endif
  } /* close_infile */



#ifdef ANSI_C

void open_string (bstritype input_string, errinfotype *err_info)
#else

void open_string (input_string, err_info)
bstritype input_string;
errinfotype *err_info;
#endif

  {
    ustritype source_file_name = "STRING";
    infiltype new_file;
    unsigned int name_length;

  /* open_string */
#ifdef TRACE_INFILE
    printf("BEGIN open_string\n");
#endif
#ifdef USE_ALTERNATE_NEXT_CHARACTER
/*  if (in_file.fil != NULL) { */
    if (!in_file.end_of_file) {
      if (!ALLOC_FILE(new_file)) {
        *err_info = MEMORY_ERROR;
      } else {
        memcpy(new_file, &in_file, sizeof(infilrecord));
        in_file.next = new_file;
      } /* if */
    } /* if */
    if (*err_info == NO_ERROR) {
      name_length = strlen((cstritype) source_file_name);
      if (!ALLOC_USTRI(in_file.name, name_length)) {
        *err_info = MEMORY_ERROR;
      } else {
        COUNT_USTRI(name_length, count.fnam, count.fnam_bytes);
        strcpy((cstritype) in_file.name, (cstritype) source_file_name);
        in_file.fil = NULL;
        in_file.start = input_string->mem;
        in_file.nextch = in_file.start;
        in_file.byond = in_file.start + input_string->size;
        in_file.buffer_size = 0;
        in_file.character = next_character();
        in_file.line = 1;
        in_file.next_msg_line = 1 + option.incr_message_line;
        file_counter++;
        in_file.file_number = file_counter;
        open_compilation_info();
        in_file.end_of_file = FALSE;
      } /* if */
    } /* if */
#endif
#ifdef TRACE_INFILE
    printf("END open_string\n");
#endif
  } /* open_string */



#ifdef ANSI_C

void next_file (void)
#else

void next_file ()
#endif

  { /* next_file */
#ifdef TRACE_INFILE
    printf("BEGIN next_file\n");
#endif
    in_file.line--;
    if (in_file.next != NULL) {
      close_infile();
    } else {
      in_file.end_of_file = TRUE;
    } /* if */
#ifdef TRACE_INFILE
    printf("END next_file\n");
#endif
  } /* next_file */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

int next_line (void)
#else

int next_line ()
#endif

  {
    register int character;

  /* next_line */
#ifdef TRACE_INFILE
    printf("BEGIN next_line\n");
#endif
    SKIP_CR_SP(character);
    INCR_LINE_COUNT(in_file.line);
    return(character);
#ifdef TRACE_INFILE
    printf("END next_line\n");
#endif
  } /* next_line */
#endif



#ifdef ANSI_C

ustritype file_name (filenumtype file_num)
#else

ustritype file_name (file_num)
filenumtype file_num;
#endif

  {
    infiltype help_file;
    ustritype result;

  /* file_name */
#ifdef TRACE_INFILE
    printf("BEGIN file_name\n");
#endif
    result = (ustritype) "?";
    help_file = file_pointer;
    while (help_file != NULL) {
      if (help_file->file_number == file_num) {
        result = help_file->name;
      } /* if */
      help_file = help_file->next;
    } /* while */
#ifdef TRACE_INFILE
    printf("END file_name\n");
#endif
    return(result);
  } /* file_name */



#ifdef ANSI_C

void find_include_file (stritype include_file_name, errinfotype *err_info)
#else

void find_include_file (include_file_name, err_info)
stritype include_file_name;
errinfotype *err_info;
#endif

  {
    unsigned int length;
    stritype stri;

  /* find_include_file */
#ifdef TRACE_INFILE
    printf("BEGIN find_include_file\n");
#endif
    open_infile(include_file_name, err_info);
    if (*err_info == FILE_ERROR) {
      if (file_path != NULL) {
        *err_info = NO_ERROR;
        length = file_path->size + include_file_name->size + 1;
        if (!ALLOC_STRI(stri, length)) {
          *err_info = MEMORY_ERROR;
        } else {
          COUNT_STRI(length);
          stri->size = length;
          memcpy(stri->mem, file_path->mem,
              (SIZE_TYPE) file_path->size * sizeof(strelemtype));
          stri->mem[file_path->size] = '/';
          memcpy(&stri->mem[file_path->size + 1], include_file_name->mem,
              (SIZE_TYPE) include_file_name->size * sizeof(strelemtype));
          open_infile(stri, err_info);
          FREE_STRI(stri, length);
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_INFILE
    printf("END find_include_file\n");
#endif
  } /* find_include_file */



#ifdef ANSI_C

void set_search_path (stritype new_path)
#else

void set_search_path (new_path)
stritype new_path;
#endif

  { /* set_search_path */
#ifdef TRACE_INFILE
    printf("BEGIN set_search_path\n");
#endif
    if (file_path != NULL) {
      FREE_STRI(file_path, file_path->size);
    } /* if */
    if (!ALLOC_STRI(file_path, new_path->size)) {
      fatal_memory_error(SOURCE_POSITION(2025));
    } /* if */
    COUNT_STRI(new_path->size);
    file_path->size = new_path->size;
    memcpy(file_path->mem, new_path->mem,
        (SIZE_TYPE) new_path->size * sizeof(strelemtype));
#ifdef TRACE_INFILE
    printf("END set_search_path\n");
#endif
  } /* set_search_path */



#ifdef ANSI_C

void set_search_path2 (ustritype new_path, memsizetype length)
#else

void set_search_path2 (new_path, length)
ustritype new_path;
memsizetype length;
#endif

  { /* set_search_path2 */
#ifdef TRACE_INFILE
    printf("BEGIN set_search_path\n");
#endif
    if (file_path != NULL) {
      FREE_STRI(file_path, file_path->size);
    } /* if */
    if (!ALLOC_STRI(file_path, length)) {
      fatal_memory_error(SOURCE_POSITION(2025));
    } /* if */
    COUNT_STRI(length);
    file_path->size = length;
    stri_expand(file_path->mem, new_path, length);
#ifdef TRACE_INFILE
    printf("END set_search_path\n");
#endif
  } /* set_search_path2 */
