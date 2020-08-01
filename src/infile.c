/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer - Infile                                       */
/*  File: seed7/src/infile.c                                        */
/*  Changes: 1990 - 1994, 2013  Thomas Mertes                       */
/*  Content: Procedures to open, close and read the source file.    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "info.h"
#include "stat.h"

#ifdef USE_MMAP
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/mman.h"
#endif

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "infile.h"


#define GET_INFILE_BUFFER TRUE
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
    if (in_file.fil != NULL &&
        (chars_read = fread(in_file.start, 1,
        (size_t) in_file.buffer_size, in_file.fil)) != 0) {
      in_file.nextch = in_file.start;
      in_file.beyond = in_file.start + chars_read;
      result = next_character();
    } else {
      result = EOF;
    } /* if */
#endif
#ifdef TRACE_INFILE
    printf("END fill_buf\n");
#endif
    return result;
  } /* fill_buf */
#endif



#ifdef ANSI_C

static INLINE booltype speedup (void)
#else

static INLINE booltype speedup ()
#endif

  {
    booltype okay;
#ifdef USE_MMAP
    int file_no;
    os_fstat_struct file_stat;
    memsizetype file_length;
#endif

  /* speedup */
#ifdef TRACE_INFILE
    printf("BEGIN speedup\n");
#endif
    okay = TRUE;
#ifdef USE_ALTERNATE_NEXT_CHARACTER
#ifdef USE_MMAP
    file_no = fileno(in_file.fil);
    if (file_no != -1 && os_fstat(file_no, &file_stat) == 0) {
      if (file_stat.st_size >= 0 && (unsigned_os_off_t) file_stat.st_size < MAX_MEMSIZETYPE) {
        file_length = (memsizetype) file_stat.st_size;
        if ((in_file.start = (ustritype) mmap(NULL, file_length,
            PROT_READ, MAP_PRIVATE, fileno(in_file.fil),
            0)) != (ustritype) -1) {
          in_file.nextch = in_file.start;
          in_file.beyond = in_file.start + file_length;
          in_file.buffer_size = 0;
        } else {
          if (ALLOC_UBYTES(in_file.start, file_length)) {
            if (fread(in_file.start, 1, file_length, in_file.fil) ==
                file_length) {
              in_file.nextch = in_file.start;
              in_file.beyond = in_file.start + file_length;
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
    } else {
      okay = FALSE;
    } /* if */
#else
    if (GET_INFILE_BUFFER) {
      in_file.buffer_size = SIZE_IN_BUFFER;
    } else {
      in_file.buffer_size = 512;
    } /* if */
    if (ALLOC_UBYTES(in_file.start, in_file.buffer_size)) {
      in_file.nextch = in_file.start + in_file.buffer_size;
      in_file.beyond = in_file.start;
    } else {
      in_file.buffer_size = 512;
      if (ALLOC_UBYTES(in_file.start, in_file.buffer_size)) {
        in_file.nextch = in_file.start + in_file.buffer_size;
        in_file.beyond = in_file.start;
      } else {
        okay = FALSE;
      } /* if */
    } /* if */
#endif
#else
#ifdef USE_INFILE_BUFFER
    if (GET_INFILE_BUFFER) {
      if (ALLOC_UBYTES(in_file.buffer, SIZE_IN_BUFFER)) {
        setvbuf(in_file.fil, in_file.buffer, _IOFBF,
            (size_t) SIZE_IN_BUFFER);
      } /* if */
    } else {
      in_file.buffer = NULL;
    } /* if */
#endif
#endif
#ifdef TRACE_INFILE
    printf("END speedup\n");
#endif
    return okay;
  } /* speedup */



#ifdef ANSI_C

void open_infile (const_stritype source_file_name, booltype write_library_names,
    booltype write_line_numbers, errinfotype *err_info)
#else

void open_infile (source_file_name, write_library_names, write_line_numbers, err_info)
stritype source_file_name;
booltype write_library_names;
booltype write_line_numbers;
errinfotype *err_info;
#endif

  {
    os_stritype os_path;
    infiltype new_file;
    FILE *in_fil;
    ustritype name_ustri;
    memsizetype name_length;
    stritype in_name;
    int path_info = PATH_IS_NORMAL;

  /* open_infile */
#ifdef TRACE_INFILE
    printf("BEGIN open_infile err_info=%u\n", *err_info);
#endif
    os_path = cp_to_os_path(source_file_name, &path_info, err_info);
    if (likely(*err_info == OKAY_NO_ERROR)) {
      in_fil = os_fopen(os_path, os_mode_rb);
      /* printf("fopen(\"%s\") ==> %lu\n", os_path, in_fil); */
      os_stri_free(os_path);
      if (in_fil == NULL) {
        *err_info = FILE_ERROR;
      } else {
        if (!ALLOC_FILE(new_file)) {
          fclose(in_file.fil);
          *err_info = MEMORY_ERROR;
        } else {
          name_ustri = (ustritype) cp_to_cstri8(source_file_name);
          if (name_ustri != NULL) {
            name_length = strlen((cstritype) name_ustri);
            name_ustri = REALLOC_USTRI(name_ustri, max_utf8_size(source_file_name->size), name_length);
          } /* if */
          if (name_ustri == NULL) {
            fclose(in_file.fil);
            *err_info = MEMORY_ERROR;
          } else if (!ALLOC_STRI_CHECK_SIZE(in_name, source_file_name->size)) {
            free_cstri(name_ustri, source_file_name);
            fclose(in_file.fil);
            *err_info = MEMORY_ERROR;
          } else {
            in_name->size = source_file_name->size;
            memcpy(in_name->mem, source_file_name->mem, source_file_name->size * sizeof(strelemtype));
            if (in_file.curr_infile != NULL) {
              memcpy(in_file.curr_infile, &in_file, sizeof(infilrecord));
            } /* if */
            in_file.fil = in_fil;
            if (!speedup()) {
              fclose(in_file.fil);
              free_cstri(name_ustri, source_file_name);
              FREE_STRI(in_name, source_file_name->size);
              if (in_file.curr_infile != NULL) {
                memcpy(&in_file, in_file.curr_infile, sizeof(infilrecord));
              } else {
                in_file.fil = NULL;
              } /* if */
              *err_info = FILE_ERROR;
            } else {
              COUNT_USTRI(name_length, count.fnam, count.fnam_bytes);
              in_file.name_ustri = name_ustri;
              in_file.name = in_name;
              in_file.character = next_character();
              in_file.line = 1;
              file_counter++;
              in_file.file_number = file_counter;
              if (in_file.curr_infile != NULL) {
                in_file.owningProg = in_file.curr_infile->owningProg;
              } else {
                in_file.owningProg = NULL; /* Is set in analyze_prog() */
              } /* if */
              open_compilation_info(write_library_names, write_line_numbers);
              in_file.end_of_file = FALSE;
              in_file.up_infile = in_file.curr_infile;
              in_file.curr_infile = new_file;
              in_file.next = file_pointer;
              file_pointer = new_file;
              memcpy(new_file, &in_file, sizeof(infilrecord));
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_INFILE
    printf("END open_infile err_info=%u\n", *err_info);
#endif
  } /* open_infile */



#ifdef ANSI_C

void close_infile (void)
#else

void close_infile ()
#endif

  { /* close_infile */
#ifdef TRACE_INFILE
    printf("BEGIN close_infile\n");
#endif
/*  printf("\nclose(\"%s\");\n", in_file.name); */
#ifdef WITH_COMPILATION_INFO
    if (in_file.write_line_numbers) {
      NL_LIN_INFO();
    } else {
      if (in_file.write_library_names) {
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
          munmap(in_file.start, (size_t) (in_file.beyond - in_file.start));
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
    if (in_file.curr_infile != NULL) {
      memcpy(in_file.curr_infile, &in_file, sizeof(infilrecord));
    } /* if */
    if (in_file.up_infile != NULL) {
      memcpy(&in_file, in_file.up_infile, sizeof(infilrecord));
      display_compilation_info();
    } else {
      in_file.curr_infile = NULL;
    } /* if */
    in_file.next_msg_line = in_file.line + in_file.incr_message_line;
#ifdef TRACE_INFILE
    printf("END close_infile\n");
#endif
  } /* close_infile */



#ifdef ANSI_C

void open_string (bstritype input_string, booltype write_library_names,
    booltype write_line_numbers, errinfotype *err_info)
#else

void open_string (input_string, write_library_names, write_line_numbers, err_info)
bstritype input_string;
booltype write_library_names;
booltype write_line_numbers;
errinfotype *err_info;
#endif

  {
    const_cstritype source_file_name = "STRING";
    infiltype new_file;
    memsizetype name_length;
    ustritype name_ustri;
    stritype in_name;

  /* open_string */
#ifdef TRACE_INFILE
    printf("BEGIN open_string\n");
#endif
#ifdef USE_ALTERNATE_NEXT_CHARACTER
    if (*err_info == OKAY_NO_ERROR) {
      if (!ALLOC_FILE(new_file)) {
        *err_info = MEMORY_ERROR;
      } else {
        name_length = strlen(source_file_name);
        if (!ALLOC_USTRI(name_ustri, name_length)) {
          *err_info = MEMORY_ERROR;
        } else if (!ALLOC_STRI_SIZE_OK(in_name, name_length)) {
          UNALLOC_USTRI(name_ustri, name_length);
          *err_info = MEMORY_ERROR;
        } else {
          COUNT_USTRI(name_length, count.fnam, count.fnam_bytes);
          strcpy((cstritype) name_ustri, source_file_name);
          in_name->size = name_length;
          ustri_expand(in_name->mem, name_ustri, name_length);
          if (in_file.curr_infile != NULL) {
            memcpy(in_file.curr_infile, &in_file, sizeof(infilrecord));
          } /* if */
          in_file.fil = NULL;
          in_file.name_ustri = name_ustri;
          in_file.name = in_name;
          in_file.start = input_string->mem;
          in_file.nextch = in_file.start;
          in_file.beyond = in_file.start + input_string->size;
          in_file.buffer_size = 0;
          in_file.character = next_character();
          in_file.line = 1;
          file_counter++;
          in_file.file_number = file_counter;
          in_file.owningProg = NULL; /* Is set in analyze_prog() */
          open_compilation_info(write_library_names, write_line_numbers);
          in_file.end_of_file = FALSE;
          in_file.up_infile = in_file.curr_infile;
          in_file.curr_infile = new_file;
          in_file.next = file_pointer;
          file_pointer = new_file;
          memcpy(new_file, &in_file, sizeof(infilrecord));
        } /* if */
      } /* if */
    } /* if */
#endif
#ifdef TRACE_INFILE
    printf("END open_string\n");
#endif
  } /* open_string */



#ifdef ANSI_C

static void free_file (infiltype old_file)
#else

static void free_file (old_file)
infiltype old_file;
#endif

  {
    memsizetype name_length;

  /* free_file */
#ifdef TRACE_INFILE
    printf("BEGIN free_file\n");
#endif
    name_length = strlen((cstritype) old_file->name_ustri);
    FREE_USTRI(old_file->name_ustri, name_length, count.fnam, count.fnam_bytes);
    FREE_STRI(old_file->name, old_file->name->size);
    FREE_FILE(old_file);
#ifdef TRACE_INFILE
    printf("END free_file\n");
#endif
  } /* free_file */



#ifdef ANSI_C

void remove_prog_files (progtype currentProg)
#else

void remove_prog_files (currentProg)
progtype currentProg;
#endif

  {
    infiltype aFile;
    infiltype *fileAddr;
    infiltype currFile;

  /* remove_prog_files */
#ifdef TRACE_INFILE
    printf("BEGIN remove_prog_files\n");
#endif
    aFile = file_pointer;
    fileAddr = &file_pointer;
    while (aFile != NULL) {
      currFile = aFile;
      aFile = aFile->next;
      /* printf("remove_prog_files: %s %lx %lx\n", currFile->name_ustri,
         (unsigned long) currFile->owningProg, (unsigned long) currentProg); */
      if (currFile->owningProg == currentProg) {
        free_file(currFile);
        *fileAddr = aFile;
      } else {
        fileAddr = &currFile->next;
      } /* if */
    } /* if */
#ifdef TRACE_INFILE
    printf("END remove_prog_files\n");
#endif
  } /* remove_prog_files */



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
    if (in_file.up_infile != NULL) {
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
#ifdef TRACE_INFILE
    printf("END next_line\n");
#endif
    return character;
  } /* next_line */
#endif



#ifdef ANSI_C

stritype get_file_name (filenumtype file_num)
#else

stritype get_file_name (file_num)
filenumtype file_num;
#endif

  {
    static stritype question_mark = NULL;
    infiltype help_file;
    stritype result;

  /* get_file_name */
#ifdef TRACE_INFILE
    printf("BEGIN get_file_name\n");
#endif
    result = NULL;
    help_file = file_pointer;
    while (help_file != NULL) {
      if (help_file->file_number == file_num) {
        result = help_file->name;
      } /* if */
      help_file = help_file->next;
    } /* while */
    if (result == NULL) {
      if (question_mark == NULL) {
        question_mark = cstri_to_stri("?");
      } /* if */
      result = question_mark;
    } /* if */
#ifdef TRACE_INFILE
    printf("END get_file_name\n");
#endif
    return result;
  } /* get_file_name */



#ifdef ANSI_C

const_ustritype get_file_name_ustri (filenumtype file_num)
#else

ustritype get_file_name_ustri (file_num)
filenumtype file_num;
#endif

  {
    infiltype help_file;
    const_ustritype result;

  /* get_file_name_ustri */
#ifdef TRACE_INFILE
    printf("BEGIN get_file_name_ustri\n");
#endif
    result = (const_ustritype) "?";
    help_file = file_pointer;
    while (help_file != NULL) {
      if (help_file->file_number == file_num) {
        result = help_file->name_ustri;
      } /* if */
      help_file = help_file->next;
    } /* while */
#ifdef TRACE_INFILE
    printf("END get_file_name_ustri\n");
#endif
    return result;
  } /* get_file_name_ustri */
