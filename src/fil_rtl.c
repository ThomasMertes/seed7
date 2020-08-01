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
#include "big_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "fil_rtl.h"


#if   defined USE_LSEEK
/* typedef off_t           offsettype; */
#define myLseek            lseek

#elif defined USE_LSEEKI64
/* typedef __int64         offsettype; */
#define myLseek            _lseeki64

#elif defined USE_FSEEKO
/* typedef off_t           offsettype; */
#define myFseek            fseeko
#define myFtell            ftello

#elif defined USE_FSEEKO64
/* typedef off64_t         offsettype; */
#define myFseek            fseeko64
#define myFtell            ftello64

#elif defined USE_FSEEKI64
/* typedef __int64         offsettype; */
#define myFseek            _fseeki64
#define myFtell            _ftelli64
extern int __cdecl _fseeki64(FILE *, __int64, int);
extern __int64 __cdecl _ftelli64(FILE *);

#else
/* typedef long            offsettype; */
#define myFseek            fseek
#define myFtell            ftell

#endif


#define BUFFER_SIZE             4096
#define GETS_DEFAULT_SIZE    1048576
#define GETS_STRI_SIZE_DELTA    4096
#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048



#ifdef ANSI_C

static void get_mode (os_chartype mode[4], stritype file_mode)
#else

static void get_mode (mode, file_mode)
os_chartype mode[4];
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
        mode[0] = (os_chartype) file_mode->mem[0];
        mode[1] = 'b';
        mode[2] = '\0';
      } else if (file_mode->size == 2) {
        if (file_mode->mem[1] == '+') {
          /* Binary mode
             r+ ... Open file for update (reading and writing). 
             w+ ... Truncate to zero length or create file for update. 
             a+ ... Append; open or create file for update, writing at end-of-file.
          */
          mode[0] = (os_chartype) file_mode->mem[0];
          mode[1] = 'b';
          mode[2] = '+';
          mode[3] = '\0';
        } else if (file_mode->mem[1] == 't') {
          /* Text mode
             rt ... Open file for reading.
             wt ... Truncate to zero length or create file for writing.
             at ... Append; open or create file for writing at end-of-file.
          */
          mode[0] = (os_chartype) file_mode->mem[0];
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
          mode[0] = (os_chartype) file_mode->mem[0];
          mode[1] = '+';
          mode[2] = '\0';
        } /* if */
      } /* if */
    } /* if */
  } /* get_mode */



#ifdef ANSI_C

static os_off_t seekFileLength (filetype aFile)
#else

static os_off_t seekFileLength (aFile)
filetype aFile;
#endif

  {
    os_off_t current_file_position;
    os_off_t file_length;

  /* seekFileLength */
#ifdef FTELL_WRONG_FOR_PIPE
    {
      int file_no;
      os_stat_struct stat_buf;

      file_no = fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        return (os_off_t) -1;
      } /* if */
    }
#endif
#ifdef myLseek
    {
      int file_no;

      file_no = fileno(aFile);
      if (file_no == -1) {
        file_length = -1;
      } else {
        fflush(aFile);
        current_file_position = myLseek(file_no, (os_off_t) 0, SEEK_CUR);
        if (current_file_position == (os_off_t) -1) {
          file_length = -1;
        } else {
          file_length = myLseek(file_no, (os_off_t) 0, SEEK_END);
          if (file_length != (os_off_t) -1) {
            if (myLseek(file_no, current_file_position, SEEK_SET) == (os_off_t) -1) {
              file_length = -1;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    }
#else
    current_file_position = myFtell(aFile);
    if (current_file_position == (os_off_t) -1) {
      file_length = -1;
    } else if (myFseek(aFile, (os_off_t) 0, SEEK_END) != 0) {
      file_length = -1;
    } else {
      file_length = myFtell(aFile);
      if (file_length != (os_off_t) -1) {
        if (myFseek(aFile, current_file_position, SEEK_SET) != 0) {
          file_length = -1;
        } /* if */
      } /* if */
    } /* if */
#endif
    return file_length;
  } /* seekFileLength */



#ifdef ANSI_C

static os_off_t offsetTell (filetype aFile)
#else

static os_off_t offsetTell (aFile)
filetype aFile;
#endif

  {
    os_off_t current_file_position;

  /* offsetTell */
#ifdef FTELL_WRONG_FOR_PIPE
    {
      int file_no;
      os_stat_struct stat_buf;

      file_no = fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        return (os_off_t) -1;
      } /* if */
    }
#endif
#ifdef myLseek
    {
      int file_no;

      file_no = fileno(aFile);
      if (file_no == -1) {
        current_file_position = -1;
      } else {
        fflush(aFile);
        current_file_position = myLseek(file_no, (os_off_t) 0, SEEK_CUR);
      } /* if */
    }
#else
    current_file_position = myFtell(aFile);
#endif
    return current_file_position;
  } /* offsetTell */



#ifdef ANSI_C

int offsetSeek (filetype aFile, const os_off_t offset, const int origin)
#else

int offsetSeek (aFile, offset)
filetype aFile;
os_off_t offset;
#endif

  {
    int result;

  /* offsetSeek */
#ifdef FTELL_WRONG_FOR_PIPE
    {
      int file_no;
      os_stat_struct stat_buf;

      file_no = fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        return -1;
      } /* if */
    }
#endif
#ifdef myLseek
    {
      int file_no;

      file_no = fileno(aFile);
      if (file_no == -1) {
        result = -1;
      } else {
        fflush(aFile);
        if (myLseek(file_no, offset, origin) == (os_off_t) -1) {
          result = -1;
        } else {
          result = 0;
        } /* if */
      } /* if */
    }
#else
    result = myFseek(aFile, offset, origin);
#endif
    return result;
  } /* offsetSeek */



/**
 *  Compute how many bytes can be read from the current position.
 *  Returns 0 when the current position or the file size cannot be
 *  determined or when the current position is byond the filesize.
 *  Returns MAX_MEMSIZETYPE when the result does not fit into
 *  memsizetype.
 */
#ifdef ANSI_C

memsizetype remainingBytesInFile (filetype aFile)
#else

memsizetype remainingBytesInFile (aFile)
filetype aFile;
#endif

  {
    int file_no;
    os_stat_struct stat_buf;
    os_off_t file_length;
    os_off_t current_file_position;
    memsizetype result;

  /* remainingBytesInFile */
    current_file_position = offsetTell(aFile);
    if (current_file_position == (os_off_t) -1) {
      result = 0;
    } else {
      file_no = fileno(aFile);
      if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
          S_ISREG(stat_buf.st_mode)) {
        file_length = stat_buf.st_size;
      } else {
        file_length = seekFileLength(aFile);
      } /* if */
      if (file_length == (os_off_t) -1) {
        result = 0;
      } else if (file_length < current_file_position) {
        result = 0;
      } else if (file_length - current_file_position >= MAX_MEMSIZETYPE) {
        result = MAX_MEMSIZETYPE;
      } else {
        result = (memsizetype) (file_length - current_file_position);
      } /* if */
    } /* if */
    return result;
  } /* remainingBytesInFile */



#ifdef ANSI_C

inttype getFileLengthUsingSeek (filetype aFile)
#else

inttype getFileLengthUsingSeek (aFile)
filetype aFile;
#endif

  {
    os_off_t file_length;
    inttype result;

  /* getFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (file_length == (os_off_t) -1) {
      raise_error(FILE_ERROR);
      result = 0;
    } else if (file_length > INTTYPE_MAX || file_length < (os_off_t) 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (inttype) file_length;
    } /* if */
    return result;
  } /* getFileLengthUsingSeek */



#ifdef ANSI_C

biginttype getBigFileLengthUsingSeek (filetype aFile)
#else

biginttype getBigFileLengthUsingSeek (aFile)
filetype aFile;
#endif

  {
    os_off_t file_length;
    biginttype result;

  /* getBigFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (file_length == (os_off_t) -1) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else if (sizeof(os_off_t) == 8) {
      result = bigFromUInt64((uint64type) file_length);
    } else if (sizeof(os_off_t) == 4) {
      result = bigFromUInt32((uint32type) file_length);
    } else {
      raise_error(RANGE_ERROR);
      result = NULL;
    } /* if */
    return result;
  } /* getBigFileLengthUsingSeek */



#ifdef ANSI_C

static memsizetype read_string (filetype aFile, stritype stri, errinfotype *err_info)
#else

static memsizetype read_string (aFile, stri, err_info)
filetype aFile;
stritype stri;
errinfotype *err_info;
#endif

  {
    uchartype buffer[BUFFER_SIZE];
    memsizetype bytes_in_buffer = 1;
    memsizetype stri_pos;
    register uchartype *from;
    register strelemtype *to;
    register memsizetype number;

  /* read_string */
    /* printf("stri->size=%lu\n", stri->size); */
    stri_pos = 0;
    while (stri->size - stri_pos >= BUFFER_SIZE && bytes_in_buffer > 0 &&
        *err_info == OKAY_NO_ERROR) {
      /* printf("read_size=%ld\n", BUFFER_SIZE); */
      bytes_in_buffer = (memsizetype) fread(buffer, 1, BUFFER_SIZE, aFile);
      if (bytes_in_buffer == 0 && stri_pos == 0 && ferror(aFile)) {
        *err_info = FILE_ERROR;
      } else {
        /* printf("#A# bytes_in_buffer=%d stri_pos=%d\n",
            bytes_in_buffer, stri_pos); */
        from = buffer;
        to = &stri->mem[stri_pos];
        number = bytes_in_buffer;
        for (; number > 0; from++, to++, number--) {
          *to = *from;
        } /* for */
        stri_pos += bytes_in_buffer;
      } /* if */
    } /* while */
    if (stri->size > stri_pos && bytes_in_buffer > 0 &&
        *err_info == OKAY_NO_ERROR) {
      /* printf("read_size=%ld\n", stri->size - stri_pos); */
      bytes_in_buffer = (memsizetype) fread(buffer, 1, stri->size - stri_pos, aFile);
      if (bytes_in_buffer == 0 && stri_pos == 0 && ferror(aFile)) {
        *err_info = FILE_ERROR;
      } else {
        /* printf("#B# bytes_in_buffer=%d stri_pos=%d\n",
            bytes_in_buffer, stri_pos); */
        from = buffer;
        to = &stri->mem[stri_pos];
        number = bytes_in_buffer;
        for (; number > 0; from++, to++, number--) {
          *to = *from;
        } /* for */
        stri_pos += bytes_in_buffer;
      } /* if */
    } /* if */
    /* printf("stri_pos=%lu\n", stri_pos); */
    return stri_pos;
  } /* read_string */



#ifdef ANSI_C

static stritype read_and_alloc_stri (filetype aFile, memsizetype chars_missing,
    memsizetype *num_of_chars_read, errinfotype *err_info)
#else

static stritype read_and_alloc_stri (aFile, chars_missing, num_of_chars_read, err_info)
filetype aFile;
memsizetype chars_missing;
memsizetype *num_of_chars_read;
errinfotype *err_info;
#endif

  {
    uchartype buffer[BUFFER_SIZE];
    memsizetype bytes_in_buffer = 1;
    memsizetype result_pos;
    register uchartype *from;
    register strelemtype *to;
    register memsizetype number;
    memsizetype new_size;
    stritype resized_result;
    stritype result;

  /* read_and_alloc_stri */
    /* printf("read_and_alloc_stri(%d, %d, *, *)\n", fileno(aFile), chars_missing); */
    if (!ALLOC_STRI_SIZE_OK(result, GETS_STRI_SIZE_DELTA)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      result->size = GETS_STRI_SIZE_DELTA;
      /* printf("chars_missing=%lu\n", chars_missing); */
      result_pos = 0;
      while (chars_missing - result_pos >= BUFFER_SIZE && bytes_in_buffer > 0 &&
          *err_info == OKAY_NO_ERROR) {
        /* printf("read_size=%ld\n", BUFFER_SIZE); */
        bytes_in_buffer = (memsizetype) fread(buffer, 1, BUFFER_SIZE, aFile);
        if (bytes_in_buffer == 0 && result_pos == 0 && ferror(aFile)) {
          *err_info = FILE_ERROR;
        } else {
          /* printf("#A# bytes_in_buffer=%d result_pos=%d\n",
              bytes_in_buffer, result_pos); */
          if (result_pos + bytes_in_buffer > result->size) {
            new_size = result->size + GETS_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE(resized_result, result, result->size, new_size);
            if (resized_result == NULL) {
              *err_info = MEMORY_ERROR;
              return result;
            } else {
              result = resized_result;
              COUNT3_STRI(result->size, new_size);
              result->size = new_size;
            } /* if */
          } /* if */
          from = buffer;
          to = &result->mem[result_pos];
          number = bytes_in_buffer;
          for (; number > 0; from++, to++, number--) {
            *to = *from;
          } /* for */
          result_pos += bytes_in_buffer;
        } /* if */
      } /* while */
      if (chars_missing > result_pos && bytes_in_buffer > 0 &&
          *err_info == OKAY_NO_ERROR) {
        /* printf("read_size=%ld\n", chars_missing - result_pos); */
        bytes_in_buffer = (memsizetype) fread(buffer, 1, chars_missing - result_pos, aFile);
        if (bytes_in_buffer == 0 && result_pos == 0 && ferror(aFile)) {
          *err_info = FILE_ERROR;
        } else {
          /* printf("#B# bytes_in_buffer=%d result_pos=%d\n",
              bytes_in_buffer, result_pos); */
          if (result_pos + bytes_in_buffer > result->size) {
            new_size = result->size + GETS_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE(resized_result, result, result->size, new_size);
            if (resized_result == NULL) {
              *err_info = MEMORY_ERROR;
              return result;
            } else {
              result = resized_result;
              COUNT3_STRI(result->size, new_size);
              result->size = new_size;
            } /* if */
          } /* if */
          from = buffer;
          to = &result->mem[result_pos];
          number = bytes_in_buffer;
          for (; number > 0; from++, to++, number--) {
            *to = *from;
          } /* for */
          result_pos += bytes_in_buffer;
        } /* if */
      } /* if */
      /* printf("result_pos=%lu\n", result_pos); */
      *num_of_chars_read = result_pos;
    } /* if */
    return result;
  } /* read_and_alloc_stri */



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
      } else if (sizeof(stat_buf.st_size) == 4) {
        result = bigFromUInt32((uint32type) stat_buf.st_size);
      } else {
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } else {
      result = getBigFileLengthUsingSeek(aFile);
    } /* if */
    return result;
  } /* filBigLng */



#ifdef ANSI_C

void filBigSeek (filetype aFile, biginttype big_position)
#else

void filBigSeek (aFile, big_position)
filetype aFile;
biginttype big_position;
#endif

  {
    os_off_t file_position;

  /* filBigSeek */
    if (sizeof(os_off_t) == 8) {
      file_position = (os_off_t) bigToInt64(big_position);
    } else if (sizeof(os_off_t) == 4) {
      file_position = (os_off_t) bigToInt32(big_position);
    } else {
      raise_error(RANGE_ERROR);
    } /* if */
    if (file_position <= 0) {
      raise_error(RANGE_ERROR);
    } else if (offsetSeek(aFile, file_position - 1, SEEK_SET) != 0) {
      raise_error(FILE_ERROR);
    } /* if */
  } /* filBigSeek */



#ifdef ANSI_C

biginttype filBigTell (filetype aFile)
#else

biginttype filBigTell (aFile)
filetype aFile;
#endif

  {
    os_off_t current_file_position;

  /* filBigTell */
    current_file_position = offsetTell(aFile);
    if (current_file_position == (os_off_t) -1) {
      raise_error(FILE_ERROR);
      return NULL;
    } else if (sizeof(os_off_t) == 8) {
      return bigFromUInt64((uint64type) current_file_position + 1);
    } else if (sizeof(os_off_t) == 4) {
      return bigFromUInt32((uint32type) current_file_position + 1);
    } else {
      raise_error(RANGE_ERROR);
      return NULL;
    } /* if */
  } /* filBigTell */



#ifdef ANSI_C

void filClose (filetype aFile)
#else

void filClose (aFile)
filetype aFile;
#endif

  { /* filClose */
    if (fclose(aFile) != 0) {
      raise_error(FILE_ERROR);
    } /* if */
  } /* filClose */



/**
 *  Return a string with 'length' characters read from 'aFile'.
 *  In order to work reasonable good for the common case (reading
 *  just some characters) memory for 'length' characters is requested
 *  with malloc(). After the data is read the result string is
 *  shrinked to the actual size (with realloc()). When 'length' is
 *  larger than GETS_DEFAULT_SIZE or the memory cannot be requested
 *  a different strategy is used. In this case the function trys to
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
    memsizetype chars_requested;
    memsizetype bytes_there;
    memsizetype allocated_size;
    errinfotype err_info = OKAY_NO_ERROR;
    memsizetype num_of_chars_read;
    stritype resized_result;
    stritype result;

  /* filGets */
    /* printf("filGets(%d, %d)\n", fileno(aFile), length); */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if ((uinttype) length > MAX_MEMSIZETYPE) {
        chars_requested = MAX_MEMSIZETYPE;
      } else {
        chars_requested = (memsizetype) length;
      } /* if */
      if (chars_requested > GETS_DEFAULT_SIZE) {
        /* Avoid requesting too much */
        result = NULL;
      } else {
        allocated_size = chars_requested;
        ALLOC_STRI_SIZE_OK(result, allocated_size);
      } /* if */
      if (result == NULL) {
        bytes_there = remainingBytesInFile(aFile);
        /* printf("bytes_there=%lu\n", bytes_there); */
        if (bytes_there != 0) {
          /* Now we know that bytes_there bytes are available in aFile */
          if (chars_requested <= bytes_there) {
            allocated_size = chars_requested;
          } else {
            allocated_size = bytes_there;
          } /* if */
          /* printf("allocated_size=%lu\n", allocated_size); */
          if (!ALLOC_STRI_CHECK_SIZE(result, allocated_size)) {
            /* printf("MAX_STRI_LEN=%lu, SIZ_STRI(MAX_STRI_LEN)=%lu\n",
                MAX_STRI_LEN, SIZ_STRI(MAX_STRI_LEN)); */
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
      } /* if */
      if (result != NULL) {
        /* We have allocated a buffer for the requested number of char
           or for the number of bytes which are available in the file */
        result->size = allocated_size;
#ifndef UTF32_STRINGS
        /* printf("read_size=%ld\n", allocated_size); */
        num_of_chars_read = (memsizetype) fread(result->mem, 1,
            (size_t) allocated_size, aFile);
        /* printf("num_of_chars_read=%lu\n", num_of_chars_read); */
        if (num_of_chars_read == 0 && ferror(aFile)) {
          err_info = FILE_ERROR;
        } /* if */
#else
        if (allocated_size <= BUFFER_SIZE) {
          /* printf("read_size=%ld\n", allocated_size); */
          num_of_chars_read = (memsizetype) fread(result->mem, 1,
              (size_t) allocated_size, aFile);
          /* printf("num_of_chars_read=%lu\n", num_of_chars_read); */
          if (num_of_chars_read == 0 && ferror(aFile)) {
            err_info = FILE_ERROR;
          } else {
            uchartype *from = &((uchartype *) result->mem)[num_of_chars_read - 1];
            strelemtype *to = &result->mem[num_of_chars_read - 1];
            memsizetype number = num_of_chars_read;

            for (; number > 0; from--, to--, number--) {
              *to = *from;
            } /* for */
          } /* if */
        } else {
          num_of_chars_read = read_string(aFile, result, &err_info);
        } /* if */
#endif
      } else {
        /* We do not know how many bytes are avaliable therefore
           result is resized with GETS_STRI_SIZE_DELTA until we
           have read enough or we reach EOF */
        result = read_and_alloc_stri(aFile, chars_requested, &num_of_chars_read, &err_info);
      } /* if */
      if (err_info != OKAY_NO_ERROR) {
        if (result != NULL) {
          FREE_STRI(result, result->size);
        } /* if */
        raise_error(err_info);
        result = NULL;
      } else if (num_of_chars_read < result->size) {
        REALLOC_STRI_SIZE_OK(resized_result, result, result->size, num_of_chars_read);
        if (resized_result == NULL) {
          FREE_STRI(result, result->size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(result->size, num_of_chars_read);
          result->size = num_of_chars_read;
        } /* if */
      } /* if */
    } /* if */
    /* printf("filGets(%d, %d) ==> ", fileno(aFile), length);
        prot_stri(result);
        printf("\n"); */
    return result;
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
    return result;
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
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      while ((ch = getc(aFile)) != (int) '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
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
      if (ch == EOF && position == 0 && ferror(aFile)) {
        FREE_STRI(result, memlength);
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
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
    } /* if */
    return result;
  } /* filLineRead */



#ifdef ANSI_C

stritype filLit (filetype aFile)
#else

stritype filLit (aFile)
filetype aFile;
#endif

  {
    const_cstritype file_name;
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
    result = cstri_to_stri(file_name);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
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
      if (stat_buf.st_size > INTTYPE_MAX || stat_buf.st_size < 0) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (inttype) stat_buf.st_size;
      } /* if */
    } else {
      result = getFileLengthUsingSeek(aFile);
    } /* if */
    return result;
  } /* filLng */



#ifdef ANSI_C

filetype filOpen (stritype file_name, stritype file_mode)
#else

filetype filOpen (file_name, file_mode)
stritype file_name;
stritype file_mode;
#endif

  {
    os_stritype os_path;
    os_chartype mode[4];
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
        result = wide_fopen(os_path, mode);
#else
        result = fopen(os_path, mode);
#endif
        os_stri_free(os_path);
      } /* if */
    } /* if */
    /* printf("END filOpen(%lX, %lX) => %lX\n", file_name, file_mode, result); */
    return result;
  } /* filOpen */



#ifdef ANSI_C

void filPclose (filetype aFile)
#else

void filPclose (aFile)
filetype aFile;
#endif

  { /* filPclose */
    if (os_pclose(aFile) == -1) {
      raise_error(FILE_ERROR);
    } /* if */
  } /* filPclose */



#ifdef ANSI_C

filetype filPopen (stritype command, stritype file_mode)
#else

filetype filPopen (command, file_mode)
stritype command;
stritype file_mode;
#endif

  {
    os_stritype cmd;
    os_chartype mode[4];
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
        mode[0] = (os_chartype) file_mode->mem[0];
        mode[1] = '\0';
      } /* if */
      /* The mode "rb" is not allowed under unix/linux */
      /* therefore get_mode(mode, file_mode); cannot be called */
      if (mode[0] == '\0') {
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        result = os_popen(cmd, mode);
      } /* if */
      free_cstri(cmd, command);
    } /* if */
    return result;
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
    } else if (offsetSeek(aFile, (os_off_t) (file_position - 1), SEEK_SET) != 0) {
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
    if (mode < 0 || mode > 2 || size < 0 || (uinttype) size > MAX_MEMSIZETYPE) {
      raise_error(RANGE_ERROR);
    } else if (setvbuf(aFile, NULL, (int) mode, (memsizetype) size) != 0) {
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
    os_off_t current_file_position;
    inttype result;

  /* filTell */
    current_file_position = offsetTell(aFile);
    if (current_file_position == (os_off_t) -1) {
      raise_error(FILE_ERROR);
      result = 0;
    } else if (current_file_position >= INTTYPE_MAX ||
        current_file_position < (os_off_t) 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (inttype) (current_file_position + 1);
    } /* if */
    return result;
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
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
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
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
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
      if (ch == EOF && position == 0 && ferror(aFile)) {
        FREE_STRI(result, memlength);
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
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
    } /* if */
    return result;
  } /* filWordRead */



#ifdef ANSI_C

void filWrite (filetype aFile, stritype stri)
#else

void filWrite (aFile, stri)
filetype aFile;
stritype stri;
#endif

  { /* filWrite */
#ifdef FWRITE_WRONG_FOR_READ_ONLY_FILES
    if (stri->size > 0 && (aFile->flags & _F_WRIT) == 0) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
#ifdef UTF32_STRINGS
    {
      register strelemtype *str;
      memsizetype len;
      register uchartype *ustri;
      register uint16type buf_len;
      uchartype buffer[BUFFER_SIZE];

      for (str = stri->mem, len = stri->size;
          len >= BUFFER_SIZE; len -= BUFFER_SIZE) {
        for (ustri = buffer, buf_len = BUFFER_SIZE; buf_len > 0; buf_len--) {
          if (*str >= 256) {
            raise_error(RANGE_ERROR);
            return;
          } /* if */
          *ustri++ = (uchartype) *str++;
        } /* for */
        if (BUFFER_SIZE != fwrite(buffer, sizeof(uchartype), BUFFER_SIZE, aFile)) {
          raise_error(FILE_ERROR);
          return;
        } /* if */
      } /* for */
      if (len > 0) {
        for (ustri = buffer, buf_len = (uint16type) len; buf_len > 0; buf_len--) {
          if (*str >= 256) {
            raise_error(RANGE_ERROR);
            return;
          } /* if */
          *ustri++ = (uchartype) *str++;
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
