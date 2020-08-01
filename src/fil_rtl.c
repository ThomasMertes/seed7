/********************************************************************/
/*                                                                  */
/*  fil_rtl.c     Primitive actions for the primitive file type.    */
/*  Copyright (C) 1989 - 2009  Thomas Mertes                        */
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
/*  File: seed7/src/fil_rtl.c                                       */
/*  Changes: 1992, 1993, 1994, 2009  Thomas Mertes                  */
/*  Content: Primitive actions for the primitive file type.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#ifdef USE_LSEEKI64
#include "io.h"
#endif

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "cmd_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "fil_rtl.h"


#if   defined USE_LSEEK
typedef off_t              offsettype;
#define myLseek            lseek

#elif defined USE_LSEEKI64
typedef __int64            offsettype;
#define myLseek            _lseeki64

#elif defined USE_FSEEKO
typedef off_t              offsettype;
#define myFseek            fseeko
#define myFtell            ftello

#elif defined USE_FSEEKO64
typedef off64_t            offsettype;
#define myFseek            fseeko64
#define myFtell            ftello64

#elif defined USE_FSEEKI64
typedef __int64            offsettype;
#define myFseek            _fseeki64
#define myFtell            _ftelli64
extern int __cdecl _fseeki64(FILE *, __int64, int);
extern __int64 __cdecl _ftelli64(FILE *);

#else
typedef long               offsettype;
#define myFseek            fseek
#define myFtell            ftell

#endif


#define BUFFER_SIZE 4096
#define READ_BLOCK_SIZE 4096



#ifdef ANSI_C

static void get_mode (char mode[4], stritype file_mode)
#else

static void get_mode (mode, file_mode)
char mode[4];
stritype file_mode;
#endif

  { /* get_mode */
    mode[0] = '\0';
    if (file_mode->size >= 1 &&
        (file_mode->mem[0] == 'r' ||
         file_mode->mem[0] == 'w' ||
         file_mode->mem[0] == 'a')) {
      if (file_mode->size == 1) {
        /* Binary mode
           r ... Open file for reading. 
           w ... Truncate to zero length or create file for writing. 
           a ... Append; open or create file for writing at end-of-file. 
        */
        mode[0] = (char) file_mode->mem[0];
        mode[1] = 'b';
        mode[2] = '\0';
      } else if (file_mode->size == 2) {
        if (file_mode->mem[1] == '+') {
          /* Binary mode
             r+ ... Open file for update (reading and writing). 
             w+ ... Truncate to zero length or create file for update. 
             a+ ... Append; open or create file for update, writing at end-of-file. 
          */
          mode[0] = (char) file_mode->mem[0];
          mode[1] = 'b';
          mode[2] = '+';
          mode[3] = '\0';
        } else if (file_mode->mem[1] == 't') {
          /* Text mode
             rt ... Open file for reading. 
             wt ... Truncate to zero length or create file for writing. 
             at ... Append; open or create file for writing at end-of-file. 
          */
          mode[0] = (char) file_mode->mem[0];
          mode[1] = '\0';
        } /* if */
      } else if (file_mode->size == 3) {
        if (file_mode->mem[1] == 't' &&
            file_mode->mem[2] == '+') {
          /* Text mode
             rt+ ... Open file for update (reading and writing). 
             wt+ ... Truncate to zero length or create file for update. 
             at+ ... Append; open or create file for update, writing at end-of-file. 
          */
          mode[0] = (char) file_mode->mem[0];
          mode[1] = '+';
          mode[2] = '\0';
        } /* if */
      } /* if */
    } /* if */
  } /* get_mode */



#ifdef ANSI_C

inttype getFileLengthUsingSeek (filetype aFile)
#else

inttype getFileLengthUsingSeek (aFile)
filetype aFile;
#endif

  {
    offsettype current_file_position;
    offsettype file_length;
    inttype result;

  /* getFileLengthUsingSeek */
#ifdef myLseek
    fflush(aFile);
    current_file_position = myLseek(fileno(aFile), (offsettype) 0, SEEK_CUR);
    if (current_file_position == (offsettype) -1) {
      raise_error(FILE_ERROR);
      result = 0;
    } else {
      file_length = myLseek(fileno(aFile), (offsettype) 0, SEEK_END);
      if (file_length == (offsettype) -1) {
        raise_error(FILE_ERROR);
        result = 0;
      } else {
        if (myLseek(fileno(aFile), current_file_position, SEEK_SET) == (offsettype) -1) {
          raise_error(FILE_ERROR);
          result = 0;
        } else {
          if (file_length > MAX_INTEGER || file_length < (offsettype) 0) {
            raise_error(RANGE_ERROR);
            result = 0;
          } else {
            result = (inttype) file_length;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#else
    current_file_position = myFtell(aFile);
    if (current_file_position == -1) {
      raise_error(FILE_ERROR);
      result = 0;
    } else if (myFseek(aFile, (offsettype) 0, SEEK_END) != 0) {
      raise_error(FILE_ERROR);
      result = 0;
    } else {
      file_length = myFtell(aFile);
      if (file_length == -1) {
        raise_error(FILE_ERROR);
        result = 0;
      } else if (myFseek(aFile, current_file_position, SEEK_SET) != 0) {
        raise_error(FILE_ERROR);
        result = 0;
      } else {
        if (file_length > MAX_INTEGER || file_length < (offsettype) 0) {
          raise_error(RANGE_ERROR);
          result = 0;
        } else {
          result = (inttype) file_length;
        } /* if */
      } /* if */
    } /* if */
#endif
    return(result);
  } /* getFileLengthUsingSeek */



#ifdef ANSI_C

biginttype getBigFileLengthUsingSeek (filetype aFile)
#else

biginttype getBigFileLengthUsingSeek (aFile)
filetype aFile;
#endif

  {
    offsettype current_file_position;
    offsettype file_length;
    biginttype result;

  /* getBigFileLengthUsingSeek */
#ifdef myLseek
    fflush(aFile);
    current_file_position = myLseek(fileno(aFile), (offsettype) 0, SEEK_CUR);
    if (current_file_position == (offsettype) -1) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      file_length = myLseek(fileno(aFile), (offsettype) 0, SEEK_END);
      if (file_length == (offsettype) -1) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else if (myLseek(fileno(aFile), current_file_position, SEEK_SET) == (offsettype) -1) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else if (sizeof(offsettype) == 8) {
        result = bigFromUInt64((uint64type) file_length);
      } else {
        result = bigFromUInt32((uint32type) file_length);
      } /* if */
    } /* if */
#else
    current_file_position = myFtell(aFile);
    if (current_file_position == -1) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else if (myFseek(aFile, (offsettype) 0, SEEK_END) != 0) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      file_length = myFtell(aFile);
      if (file_length == -1) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else if (myFseek(aFile, current_file_position, SEEK_SET) != 0) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else if (sizeof(offsettype) == 8) {
        result = bigFromUInt64((uint64type) file_length);
      } else {
        result = bigFromUInt32((uint32type) file_length);
      } /* if */
    } /* if */
#endif
    return(result);
  } /* getBigFileLengthUsingSeek */



#ifdef ANSI_C

biginttype filBigLng (filetype aFile)
#else

biginttype filBigLng (aFile)
filetype aFile;
#endif

  {
    int file_no;
    os_stat_struct stat_buf;
    biginttype result;

  /* filBigLng */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
        S_ISREG(stat_buf.st_mode)) {
      if (sizeof(stat_buf.st_size) == 8) {
        result = bigFromUInt64((uint64type) stat_buf.st_size);
      } else {
        result = bigFromUInt32((uint32type) stat_buf.st_size);
      } /* if */
    } else {
      result = getBigFileLengthUsingSeek(aFile);
    } /* if */
    return(result);
  } /* filBigLng */



#ifdef ANSI_C

void filBigSeek (filetype aFile, biginttype big_position)
#else

void filBigSeek (aFile, big_position)
filetype aFile;
biginttype big_position;
#endif

  {
    offsettype file_position;

  /* filBigSeek */
#ifdef myLseek
    fflush(aFile);
    if (sizeof(offsettype) == 8) {
      file_position = (offsettype) bigToInt64(big_position);
    } else {
      file_position = (offsettype) bigToInt32(big_position);
    } /* if */
    if (file_position <= 0) {
      raise_error(RANGE_ERROR);
    } else if (myLseek(fileno(aFile), file_position - 1, SEEK_SET) == (offsettype) -1) {
      raise_error(FILE_ERROR);
    } /* if */
#else
    if (sizeof(offsettype) == 8) {
      file_position = (offsettype) bigToInt64(big_position);
    } else {
      file_position = (offsettype) bigToInt32(big_position);
    } /* if */
    if (file_position <= 0) {
      raise_error(RANGE_ERROR);
    } else if (myFseek(aFile, file_position - 1, SEEK_SET) != 0) {
      raise_error(FILE_ERROR);
    } /* if */
#endif
  } /* filBigSeek */



#ifdef ANSI_C

biginttype filBigTell (filetype aFile)
#else

biginttype filBigTell (aFile)
filetype aFile;
#endif

  {
    offsettype current_file_position;

  /* filBigTell */
#ifdef myLseek
    fflush(aFile);
    current_file_position = myLseek(fileno(aFile), (offsettype) 0, SEEK_CUR);
    if (current_file_position == (offsettype) -1) {
      raise_error(FILE_ERROR);
      return(NULL);
    } else if (sizeof(offsettype) == 8) {
      return(bigFromUInt64((uint64type) current_file_position + 1));
    } else {
      return(bigFromUInt32((uint32type) current_file_position + 1));
    } /* if */
#else
    current_file_position = myFtell(aFile);
    if (current_file_position == -1) {
      raise_error(FILE_ERROR);
      return(NULL);
    } else if (sizeof(offsettype) == 8) {
      return(bigFromUInt64((uint64type) current_file_position + 1));
    } else {
      return(bigFromUInt32((uint32type) current_file_position + 1));
    } /* if */
#endif
  } /* filBigTell */



/**
 *  Return a string with 'length' characters read from 'aFile'.
 *  In order to work reasonable good for the common case (reading
 *  just some characters) memory for 'length' characters is requested
 *  with malloc(). After the data is read the result string is
 *  shrinked to the actual size (with realloc()). When 'length'
 *  is larger than a limit or the memory cannot be requested a
 *  different strategy is used. In this case the function trys to
 *  find out the number of available characters (this is possible
 *  for a regular file but not for a pipe). If this fails a third
 *  strategy is used. In this case a smaller block is requested. This
 *  block is filled with data, resized and filled in a loop.
 */
#ifdef ANSI_C

stritype filGets (filetype aFile, inttype length)
#else

stritype filGets (aFile, length)
filetype aFile;
inttype length;
#endif

  {
    long current_file_position;
    memsizetype bytes_requested;
    memsizetype bytes_there;
    memsizetype read_size_requested;
    memsizetype block_size_read;
    memsizetype allocated_size;
    memsizetype result_size;
    ustritype memory;
    stritype resized_result;
    stritype result;

  /* filGets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      bytes_requested = (memsizetype) length;
      if (bytes_requested > 0x1FFFFFFF) {
        /* Avoid requesting too much */
        result = NULL;
      } else {
        allocated_size = bytes_requested;
        ALLOC_STRI(result, allocated_size);
      } /* if */
      if (result == NULL) {
        /* Determine how many bytes are available in aFile */
        if ((current_file_position = ftell(aFile)) != -1) {
          fseek(aFile, 0, SEEK_END);
          bytes_there = (memsizetype) (ftell(aFile) - current_file_position);
          fseek(aFile, current_file_position, SEEK_SET);
          /* Now we know that bytes_there bytes are available in aFile */
          if (bytes_there < bytes_requested) {
            allocated_size = bytes_there;
            if (!ALLOC_STRI(result, allocated_size)) {
              raise_error(MEMORY_ERROR);
              return(NULL);
            } /* if */
          } else if (bytes_requested <= 0x1FFFFFFF) {
            /* The request for memory already failed */
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
        } /* if */
      } /* if */
      if (result != NULL) {
        /* We have allocated a buffer for the requested number of bytes
           or for the number of bytes which are available in the file */
        result_size = (memsizetype) fread(result->mem, 1,
            (size_t) allocated_size, aFile);
      } else {
        /* We do not know how many bytes are avaliable therefore
           we read blocks of READ_BLOCK_SIZE until we have read
           enough or we reach EOF */
        allocated_size = READ_BLOCK_SIZE;
        if (!ALLOC_STRI(result, allocated_size)) {
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          read_size_requested = READ_BLOCK_SIZE;
          if (read_size_requested > bytes_requested) {
            read_size_requested = bytes_requested;
          } /* if */
          block_size_read = fread(result->mem, 1, read_size_requested, aFile);
          result_size = block_size_read;
          while (block_size_read == READ_BLOCK_SIZE && result_size < bytes_requested) {
            allocated_size = result_size + READ_BLOCK_SIZE;
            REALLOC_STRI(resized_result, result, result_size, allocated_size);
            if (resized_result == NULL) {
              FREE_STRI(result, result_size);
              raise_error(MEMORY_ERROR);
              return(NULL);
            } else {
              result = resized_result;
              COUNT3_STRI(result_size, allocated_size);
              memory = (ustritype) result->mem;
              read_size_requested = READ_BLOCK_SIZE;
              if (result_size + read_size_requested > bytes_requested) {
                read_size_requested = bytes_requested - result_size;
              } /* if */
              block_size_read = fread(&memory[result_size], 1,
                  read_size_requested, aFile);
              result_size += block_size_read;
            } /* if */
          } /* while */
        } /* if */
      } /* if */
#ifdef UTF32_STRINGS
      if (result_size > 0) {
        uchartype *from = &((uchartype *) result->mem)[result_size - 1];
        strelemtype *to = &result->mem[result_size - 1];
        memsizetype number = result_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
#endif
      result->size = result_size;
      if (result_size < allocated_size) {
        REALLOC_STRI(resized_result, result, allocated_size, result_size);
        if (resized_result == NULL) {
          FREE_STRI(result, allocated_size);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          result = resized_result;
          COUNT3_STRI(allocated_size, result_size);
        } /* if */
      } /* if */
    } /* if */
    return(result);
  } /* filGets */



#ifdef ANSI_C

booltype filHasNext (filetype aFile)
#else

booltype filHasNext (aFile)
filetype aFile;
#endif

  {
    int next_char;
    booltype result;

  /* filHasNext */
    if (feof(aFile)) {
      result = FALSE;
    } else {
      next_char = getc(aFile);
      if (next_char != EOF) {
        if (ungetc(next_char, aFile) != next_char) {
          raise_error(FILE_ERROR);
          result = FALSE;
        } else {
          result = TRUE;
        } /* if */
      } else {
        clearerr(aFile);
        result = FALSE;
      } /* if */
    } /* if */
    return(result);
  } /* filHasNext */



#ifdef ANSI_C

stritype filLineRead (filetype aFile, chartype *termination_char)
#else

stritype filLineRead (aFile, termination_char)
filetype aFile;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* filLineRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      while ((ch = getc(aFile)) != (int) '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_STRI(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
      } /* while */
      if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *termination_char = (chartype) ch;
      } /* if */
    } /* if */
    return(result);
  } /* filLineRead */



#ifdef ANSI_C

stritype filLit (filetype aFile)
#else

stritype filLit (aFile)
filetype aFile;
#endif

  {
    const_cstritype file_name;
    memsizetype length;
    stritype result;

  /* filLit */
    if (aFile == NULL) {
      file_name = "NULL";
    } else if (aFile == stdin) {
      file_name = "stdin";
    } else if (aFile == stdout) {
      file_name = "stdout";
    } else if (aFile == stderr) {
      file_name = "stderr";
    } else {
      file_name = "file";
    } /* if */
    length = strlen(file_name);
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      cstri_expand(result->mem, file_name, length);
      return(result);
    } /* if */
  } /* filLit */



#ifdef ANSI_C

inttype filLng (filetype aFile)
#else

inttype filLng (aFile)
filetype aFile;
#endif

  {
    int file_no;
    os_stat_struct stat_buf;
    inttype result;

  /* filLng */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
        S_ISREG(stat_buf.st_mode)) {
      if (stat_buf.st_size > MAX_INTEGER || stat_buf.st_size < 0) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (inttype) stat_buf.st_size;
      } /* if */
    } else {
      result = getFileLengthUsingSeek(aFile);
    } /* if */
    return(result);
  } /* filLng */



#ifdef ANSI_C

filetype filOpen (stritype file_name, stritype file_mode)
#else

filetype filOpen (file_name, file_mode)
stritype file_name;
stritype file_mode;
#endif

  {
    os_path_stri os_path;
    char mode[4];
#ifdef OS_PATH_WCHAR
    wchar_t wide_mode[4];
#endif
    errinfotype err_info = OKAY_NO_ERROR;
    filetype result;

  /* filOpen */
    /* printf("BEGIN filOpen(%lX, %lX)\n", file_name, file_mode); */
    get_mode(mode, file_mode);
    if (mode[0] == '\0') {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      os_path = cp_to_os_path(file_name, &err_info);
      if (os_path == NULL) {
        raise_error(err_info);
        result = NULL;
      } else {
#ifdef OS_PATH_WCHAR
        wide_mode[0] = mode[0];
        wide_mode[1] = mode[1];
        wide_mode[2] = mode[2];
        wide_mode[3] = mode[3];
        result = wide_fopen(os_path, wide_mode);
#else
        result = fopen(os_path, mode);
#endif
        os_path_free(os_path);
      } /* if */
    } /* if */
    /* printf("END filOpen(%lX, %lX) => %lX\n", file_name, file_mode, result); */
    return(result);
  } /* filOpen */



#ifdef ANSI_C

filetype filPopen (stritype command, stritype file_mode)
#else

filetype filPopen (command, file_mode)
stritype command;
stritype file_mode;
#endif

  {
    cstritype cmd;
    char mode[4];
    errinfotype err_info = OKAY_NO_ERROR;
    filetype result;

  /* filPopen */
    cmd = cp_to_command(command, &err_info);
    if (cmd == NULL) {
      raise_error(err_info);
      result = NULL;
    } else {
      mode[0] = '\0';
      if (file_mode->size == 1 &&
          (file_mode->mem[0] == 'r' ||
           file_mode->mem[0] == 'w')) {
        mode[0] = (char) file_mode->mem[0];
        mode[1] = '\0';
      } /* if */
      /* The mode "rb" is not allowed under unix/linux */
      /* therefore get_mode(mode, file_mode); cannot be called */
      if (mode[0] == '\0') {
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        result = popen(cmd, mode);
      } /* if */
      free_cstri(cmd, command);
    } /* if */
    return(result);
  } /* filPopen */



#ifdef ANSI_C

void filPrint (stritype stri)
#else

void filPrint (stri)
stritype stri;
#endif

  {
    cstritype str1;

  /* filPrint */
    str1 = cp_to_cstri(stri);
    if (str1 == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      fputs(str1, stdout);
      fflush(stdout);
      free_cstri(str1, stri);
    } /* if */
  } /* filPrint */



#ifdef ANSI_C

void filSeek (filetype aFile, inttype file_position)
#else

void filSeek (aFile, file_position)
filetype aFile;
inttype file_position;
#endif

  { /* filSeek */
    /* printf("filSeek(%ld, %ld)\n", aFile, file_position); */
    if (file_position <= 0) {
      raise_error(RANGE_ERROR);
    } else if (fseek(aFile, file_position - 1, SEEK_SET) != 0) {
      raise_error(FILE_ERROR);
    } /* if */
  } /* filSeek */



#ifdef ANSI_C

void filSetbuf (filetype aFile, inttype mode, inttype size)
#else

void filSetbuf (aFile, mode, size)
filetype aFile;
inttype mode;
inttype size;
#endif

  { /* filSetbuf */
    if (mode < 0 || mode > 2 || size < 0) {
      raise_error(RANGE_ERROR);
    } else if (setvbuf(aFile, NULL, mode, (uinttype) size) != 0) {
      raise_error(FILE_ERROR);
    } /* if */
  } /* filSetbuf */



#ifdef ANSI_C

inttype filTell (filetype aFile)
#else

inttype filTell (aFile)
filetype aFile;
#endif

  {
    inttype file_position;

  /* filTell */
    file_position = ftell(aFile);
    if (file_position == -1) {
      raise_error(FILE_ERROR);
      return(0);
    } else if (file_position + 1 <= 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      return(file_position + 1);
    } /* if */
  } /* filTell */



#ifdef ANSI_C

stritype filWordRead (filetype aFile, chartype *termination_char)
#else

stritype filWordRead (aFile, termination_char)
filetype aFile;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* filWordRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      do {
        ch = getc(aFile);
      } while (ch == (int) ' ' || ch == (int) '\t');
      while (ch != (int) ' ' && ch != (int) '\t' &&
          ch != (int) '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_STRI(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        ch = getc(aFile);
      } /* while */
      if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *termination_char = (chartype) ch;
      } /* if */
    } /* if */
    return(result);
  } /* filWordRead */



#ifdef ANSI_C

void filWrite (filetype aFile, stritype stri)
#else

void filWrite (aFile, stri)
filetype aFile;
stritype stri;
#endif

  { /* filWrite */
#ifdef UTF32_STRINGS
    {
      register strelemtype *str;
      memsizetype len;
      register uchartype *ustri;
      register memsizetype buf_len;
      uchartype buffer[BUFFER_SIZE];

      for (str = stri->mem, len = stri->size;
          len >= BUFFER_SIZE; len -= BUFFER_SIZE) {
        for (ustri = buffer, buf_len = BUFFER_SIZE;
            buf_len > 0; str++, ustri++, buf_len--) {
          if (*str >= 256) {
            raise_error(RANGE_ERROR);
            return;
          } /* if */
          *ustri = (uchartype) *str;
        } /* for */
        if (BUFFER_SIZE != fwrite(buffer, sizeof(uchartype), BUFFER_SIZE, aFile)) {
          raise_error(FILE_ERROR);
          return;
        } /* if */
      } /* for */
      if (len > 0) {
        for (ustri = buffer, buf_len = len;
            buf_len > 0; str++, ustri++, buf_len--) {
          if (*str >= 256) {
            raise_error(RANGE_ERROR);
            return;
          } /* if */
          *ustri = (uchartype) *str;
        } /* for */
        if (len != fwrite(buffer, sizeof(uchartype), len, aFile)) {
          raise_error(FILE_ERROR);
          return;
        } /* if */
      } /* if */
    }
#else
    if (stri->size != fwrite(stri->mem, sizeof(strelemtype),
        (size_t) stri->size, aFile)) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
  } /* filWrite */
