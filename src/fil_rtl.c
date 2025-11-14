/********************************************************************/
/*                                                                  */
/*  fil_rtl.c     Primitive actions for the C library file type.    */
/*  Copyright (C) 1992 - 1994, 2009 - 2021  Thomas Mertes           */
/*                2023 - 2025  Thomas Mertes                        */
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
/*  Changes: 1992 - 1994, 2009 - 2021, 2023 - 2025  Thomas Mertes   */
/*  Content: Primitive actions for the C library file type.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "signal.h"
#include "setjmp.h"
#if UNISTD_H_PRESENT
#include "unistd.h"
#endif
#if defined ISATTY_INCLUDE_IO_H || defined FTRUNCATE_INCLUDE_IO_H || defined LSEEK_INCLUDE_IO_H
#include "io.h"
#endif
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "sigutl.h"
#include "ut8_rtl.h"
#include "cmd_rtl.h"
#include "stat_drv.h"
#include "big_drv.h"
#include "fil_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "fil_rtl.h"


#ifdef C_PLUS_PLUS
#define C "C"
#else
#define C
#endif

#ifdef DEFINE_FSEEKI64_PROTOTYPE
extern C int __cdecl _fseeki64 (FILE *, __int64, int);
#endif

#ifdef DEFINE_FTELLI64_PROTOTYPE
extern C __int64 __cdecl _ftelli64 (FILE *);
#endif

#ifdef DEFINE_WPOPEN
DEFINE_WPOPEN
#endif

#ifdef DEFINE_CHSIZE_S_PROTOTYPE
extern int _chsize_s (int fd, int64Type size);
#endif

#ifdef DEFINE_CHSIZE_PROTOTYPE
extern int _chsize (int fd, long size);
#endif

#define MAX_MODE_LEN               5
#define BUFFER_SIZE             4096
#define GETS_DEFAULT_SIZE    1048576
#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048



/**
 *  Translate a Seed7 file open mode to a C file open mode.
 *  The following modes are accepted
 *   Seed7 mode | C mode | Comment
 *   "r"        | "rb"   | Open file for reading.
 *   "w"        | "wb"   | Open or create file for writing and truncate to zero length.
 *   "a"        | "ab"   | Open or create file for appending (writing at end-of-file).
 *   "r+"       | "rb+"  | Open file for update (reading and writing).
 *   "w+"       | "wb+"  | Open or create file for update and truncate to zero length.
 *   "a+"       | "ab+"  | Open or create file for appending and reading.
 *   "rt"       | "r"    | Open file for reading.
 *   "wt"       | "w"    | Open or create file for writing and truncate to zero length.
 *   "at"       | "a"    | Open or create file for appending (writing at end-of-file).
 *   "rt+"      | "r+"   | Open file for update (reading and writing).
 *   "wt+"      | "w+"   | Open or create file for update and truncate to zero length.
 *   "at+"      | "a+"   | Open or create file for appending and reading.
 *  Other Seed7 modes correspond to the C mode "".
 *  The Seed7 modes with t are text modes and the modes
 *  without t are binary modes.
 *  If there is a mode character to set the O_CLOEXEC flag (FOPEN_SUPPORTS_CLOEXEC_MODE),
 *  it is appended to os_mode.
 */
static void get_mode (const const_striType file_mode, os_charType os_mode[MAX_MODE_LEN],
    boolType *readingAllowed, boolType *writingAllowed)

  {
    int mode_pos = 0;

  /* get_mode */
    logFunction(printf("get_mode(\"%s\", *)\n",
                       striAsUnquotedCStri(file_mode)););
    if (file_mode->size >= 1 &&
        (file_mode->mem[0] == 'r' ||
         file_mode->mem[0] == 'w' ||
         file_mode->mem[0] == 'a')) {
      if (file_mode->size == 1) {
        /* Binary mode
           r ... Open file for reading.
           w ... Open or create file for writing and truncate to zero length.
           a ... Open or create file for appending (writing at end-of-file).
        */
        os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
        os_mode[mode_pos++] = 'b';
        *readingAllowed = file_mode->mem[0] == 'r';
        *writingAllowed = file_mode->mem[0] != 'r';
      } else if (file_mode->size == 2) {
        if (file_mode->mem[1] == '+') {
          /* Binary mode
             r+ ... Open file for update (reading and writing).
             w+ ... Open or create file for update and truncate to zero length.
             a+ ... Open or create file for appending and reading.
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
          os_mode[mode_pos++] = 'b';
          os_mode[mode_pos++] = '+';
          *readingAllowed = TRUE;
          *writingAllowed = TRUE;
        } else if (file_mode->mem[1] == 't') {
          /* Text mode
             rt ... Open file for reading.
             wt ... Open or create file for writing and truncate to zero length.
             at ... Open or create file for appending (writing at end-of-file).
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
          *readingAllowed = file_mode->mem[0] == 'r';
          *writingAllowed = file_mode->mem[0] != 'r';
        } /* if */
      } else if (file_mode->size == 3) {
        if (file_mode->mem[1] == 't' &&
            file_mode->mem[2] == '+') {
          /* Text mode
             rt+ ... Open file for update (reading and writing).
             wt+ ... Open or create file for update and truncate to zero length.
             at+ ... Open or create file for appending and reading.
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
          os_mode[mode_pos++] = '+';
          *readingAllowed = TRUE;
          *writingAllowed = TRUE;
        } /* if */
      } /* if */
#if FOPEN_SUPPORTS_CLOEXEC_MODE
      if (mode_pos != 0) {
        os_mode[mode_pos++] = 'e';
      } /* if */
#endif
    } /* if */
    os_mode[mode_pos] = '\0';
    logFunction(printf("get_mode(\"%s\", \"" FMT_S_OS "\") -->\n",
                       striAsUnquotedCStri(file_mode), os_mode););
  } /* get_mode */



/**
 *  Determine the length of a file by using a seek function.
 *  The file length is measured in bytes.
 *  The file position is moved to the end of the file and the
 *  end position is used as file length. Afterwards the file
 *  position is moved back to the previous position.
 *  This function returns an os_off_t result. The size of
 *  os_off_t might be different from the size of intType.
 *  @return the length of the file, or
 *          -1 if the length could not be obtained.
 */
static os_off_t seekFileLength (cFileType aFile)

  {
    os_off_t file_length;

  /* seekFileLength */
    logFunction(printf("seekFileLength(%d)\n", safe_fileno(aFile)););
#if FTELL_SUCCEEDS_FOR_PIPE
    {
      int file_no;
      os_fstat_struct stat_buf;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        logFunction(printf("seekFileLength --> -1 (not regular file)\n"););
        return (os_off_t) -1;
      } /* if */
    }
#endif
#if FSEEK_SUCCEEDS_FOR_STDIN
    {
      int file_no;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_isatty(file_no)) {
        logFunction(printf("seekFileLength --> -1 (isatty)\n"););
        return (os_off_t) -1;
      } /* if */
    }
#endif
#if defined(os_fseek) && defined(os_ftell)
    {
      os_off_t current_file_position;

      current_file_position = os_ftell(aFile);
      if (current_file_position == (os_off_t) -1) {
        file_length = -1;
      } else if (os_fseek(aFile, (os_off_t) 0, SEEK_END) != 0) {
        file_length = -1;
      } else {
        file_length = os_ftell(aFile);
        if (os_fseek(aFile, current_file_position, SEEK_SET) != 0) {
          file_length = -1;
        } /* if */
      } /* if */
    }
#elif defined os_fsetpos && defined os_fgetpos
    {
      fpos_t current_file_pos;
      fpos_t file_pos;

      if (os_fgetpos(aFile, &current_file_pos) != 0) {
        file_length = -1;
      } else if (fseek(aFile, 0, SEEK_END) != 0) {
        file_length = -1;
      } else {
        if (os_fgetpos(aFile, &file_pos) != 0) {
          file_length = -1;
        } else {
          file_length = file_pos;
        } /* if */
        if (os_fsetpos(aFile, &current_file_pos) != 0) {
          file_length = -1;
        } /* if */
      } /* if */
    }
#else
    {
      int file_no;
      os_off_t current_file_position;

      file_no = os_fileno(aFile);
      if (file_no == -1) {
        file_length = -1;
      } else {
        fflush(aFile);
        current_file_position = os_lseek(file_no, (os_off_t) 0, SEEK_CUR);
        if (current_file_position == (os_off_t) -1) {
          file_length = -1;
        } else {
          file_length = os_lseek(file_no, (os_off_t) 0, SEEK_END);
          if (file_length != (os_off_t) -1) {
            if (os_lseek(file_no, current_file_position, SEEK_SET) == (os_off_t) -1) {
              file_length = -1;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    }
#endif
    logFunction(printf("seekFileLength --> " FMT_D_OFF "\n", file_length););
    return file_length;
  } /* seekFileLength */



/**
 *  Determine the current file position.
 *  The file position is measured in bytes.
 *  This function uses 0 as the position of the first byte in the file.
 *  The function returns an os_off_t result. The size of
 *  os_off_t might be different from the size of intType.
 *  @return the current file position, or
 *          -1 if the file position could not be obtained.
 */
static os_off_t offsetTell (cFileType aFile)

  {
    os_off_t current_file_position;

  /* offsetTell */
    logFunction(printf("offsetTell(%d)\n", safe_fileno(aFile)););
#if FTELL_SUCCEEDS_FOR_PIPE
    {
      int file_no;
      os_fstat_struct stat_buf;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        logFunction(printf("offsetTell --> -1 (not regular file)\n"););
        return (os_off_t) -1;
      } /* if */
    }
#endif
#if FSEEK_SUCCEEDS_FOR_STDIN
    {
      int file_no;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_isatty(file_no)) {
        logFunction(printf("offsetTell --> -1 (isatty)\n"););
        return (os_off_t) -1;
      } /* if */
    }
#endif
#ifdef os_ftell
    current_file_position = os_ftell(aFile);
    logErrorIfTrue(current_file_position == -1,
                   printf("offsetTell: os_ftell(fd of file = %d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(aFile), errno, strerror(errno)););
#elif defined os_fgetpos
    {
      fpos_t file_pos;

      if (os_fgetpos(aFile, &file_pos) != 0) {
        current_file_position = -1;
      } else {
        current_file_position = file_pos;
      } /* if */
    }
#else
    {
      int file_no;

      file_no = os_fileno(aFile);
      if (file_no == -1) {
        current_file_position = -1;
      } else {
        fflush(aFile);
        current_file_position = os_lseek(file_no, (os_off_t) 0, SEEK_CUR);
      } /* if */
    }
#endif
    logFunction(printf("offsetTell --> " FMT_D_OFF "\n", current_file_position););
    return current_file_position;
  } /* offsetTell */



/**
 *  Set the current file position.
 *  The file position is measured in bytes.
 *  This function uses 0 as the position of the first byte in the file.
 *  The parameter 'anOffset' uses the type os_off_t. The size of
 *  os_off_t might be different from the size of intType.
 *  @return 0 upon successful completion, or
 *          -1 if the file position could not be set.
 */
int offsetSeek (cFileType aFile, const os_off_t anOffset, const int origin)

  {
    int result;

  /* offsetSeek */
    logFunction(printf("offsetSeek(%d, " FMT_D_OFF ", %d)\n",
                       safe_fileno(aFile), anOffset, origin););
#ifdef OS_FSEEK_OFFSET_BITS
#if OS_FSEEK_OFFSET_BITS == 32
    if (anOffset > (os_off_t) INT32TYPE_MAX ||
        anOffset < (os_off_t) INT32TYPE_MIN) {
      logFunction(printf("offsetSeek --> -1\n"););
      return -1;
    } /* if */
#endif
#endif
#if FTELL_SUCCEEDS_FOR_PIPE
    {
      int file_no;
      os_fstat_struct stat_buf;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_fstat(file_no, &stat_buf) != 0 ||
          !S_ISREG(stat_buf.st_mode)) {
        logFunction(printf("offsetSeek --> -1 (not regular file)\n"););
        return -1;
      } /* if */
    }
#endif
#if FSEEK_SUCCEEDS_FOR_STDIN
    {
      int file_no;

      file_no = os_fileno(aFile);
      if (file_no == -1 || os_isatty(file_no)) {
        logFunction(printf("offsetSeek --> -1 (isatty)\n"););
        return -1;
      } /* if */
    }
#endif
#ifdef os_fseek
    result = os_fseek(aFile, anOffset, origin);
    logErrorIfTrue(result != 0,
                   printf("offsetSeek: os_fseek(fd of file = %d, "
                          FMT_D_OFF ", %d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(aFile), anOffset, origin,
                          errno, strerror(errno)););
#elif defined os_fsetpos && defined os_fgetpos
    {
      fpos_t file_pos;

      switch (origin) {
        case SEEK_SET:
          file_pos = anOffset;
          break;
        case SEEK_CUR:
          if (os_fgetpos(aFile, &file_pos) != 0) {
            logFunction(printf("offsetSeek --> -1\n"););
            return -1;
          } else {
            file_pos += anOffset;
          } /* if */
          break;
        case SEEK_END:
          if (fseek(aFile, 0, SEEK_END) != 0) {
            logFunction(printf("offsetSeek --> -1\n"););
            return -1;
          } else if (os_fgetpos(aFile, &file_pos) != 0) {
            logFunction(printf("offsetSeek --> -1\n"););
            return -1;
          } else {
            file_pos += anOffset;
          } /* if */
          break;
      } /* switch */
      if (os_fsetpos(aFile, &file_pos) != 0) {
        result = -1;
      } else {
        result = 0;
      } /* if */
    }
#else
    {
      int file_no;

      file_no = os_fileno(aFile);
      if (file_no == -1) {
        result = -1;
      } else {
        fflush(aFile);
        if (os_lseek(file_no, anOffset, origin) == (os_off_t) -1) {
          result = -1;
        } else {
          result = 0;
        } /* if */
      } /* if */
    }
#endif
    logFunction(printf("offsetSeek --> %d\n", result););
    return result;
  } /* offsetSeek */



/**
 *  Compute how many bytes can be read from the current position.
 *  Returns 0 if the current position or the file size cannot be
 *  determined or if the current position is beyond the filesize.
 *  Returns MAX_MEMSIZETYPE if the result does not fit into
 *  memSizeType.
 */
memSizeType remainingBytesInFile (cFileType aFile)

  {
    int file_no;
    os_fstat_struct stat_buf;
    os_off_t file_length;
    os_off_t current_file_position;
    memSizeType remainingBytes;

  /* remainingBytesInFile */
    current_file_position = offsetTell(aFile);
    if (current_file_position == (os_off_t) -1) {
      remainingBytes = 0;
    } else {
      file_no = os_fileno(aFile);
      if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
          S_ISREG(stat_buf.st_mode)) {
        /* Using stat_buf.st_size, which is filled by os_fstat() */
        /* is okay, because we do read from a file.              */
        file_length = stat_buf.st_size;
      } else {
        file_length = seekFileLength(aFile);
      } /* if */
      if (file_length == (os_off_t) -1) {
        remainingBytes = 0;
      } else if (file_length < current_file_position) {
        remainingBytes = 0;
      } else if ((unsigned_os_off_t) (file_length - current_file_position) >= MAX_MEMSIZETYPE) {
        remainingBytes = MAX_MEMSIZETYPE;
      } else {
        remainingBytes = (memSizeType) (file_length - current_file_position);
      } /* if */
    } /* if */
    return remainingBytes;
  } /* remainingBytesInFile */



intType getFileLengthUsingSeek (cFileType aFile)

  {
    os_off_t file_length;
    intType length;

  /* getFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (unlikely(file_length < (os_off_t) 0)) {
      logError(printf("getFileLengthUsingSeek: seekFileLength(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
      length = 0;
    } else if (unlikely(file_length > INTTYPE_MAX)) {
      logError(printf("getFileLengthUsingSeek(%d): "
                      "File length does not fit into an integer: " FMT_D_OFF ".\n",
                      safe_fileno(aFile), file_length););
      raise_error(RANGE_ERROR);
      length = 0;
    } else {
      length = (intType) file_length;
    } /* if */
    return length;
  } /* getFileLengthUsingSeek */



bigIntType getBigFileLengthUsingSeek (cFileType aFile)

  {
    os_off_t file_length;
    bigIntType length;

  /* getBigFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (unlikely(file_length < (os_off_t) 0)) {
      logError(printf("getBigFileLengthUsingSeek: seekFileLength(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
      length = NULL;
    } else {
#if OS_OFF_T_SIZE == 32
      length = bigFromUInt32((uint32Type) file_length);
#elif OS_OFF_T_SIZE == 64
      length = bigFromUInt64((uint64Type) file_length);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
    } /* if */
    return length;
  } /* getBigFileLengthUsingSeek */



/**
 *  Read a string, if we do not know how many bytes are available.
 *  This function reads data into a list of buffers. This is done
 *  until enough characters are read or EOF has been reached.
 *  Afterwards the string is allocated, the data is copied from the
 *  buffers and the list of buffers is freed.
 */
static striType read_and_alloc_stri (cFileType inFile, memSizeType chars_missing,
    errInfoType *err_info)

  {
    struct bufferStruct buffer;
    bufferList currBuffer = &buffer;
    bufferList oldBuffer;
    memSizeType bytes_in_buffer = LIST_BUFFER_SIZE;
    memSizeType result_pos;
    memSizeType result_size = 0;
    striType result;

  /* read_and_alloc_stri */
    logFunction(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *)\n",
                       safe_fileno(inFile), chars_missing););
    buffer.next = NULL;
    while (chars_missing - result_size >= LIST_BUFFER_SIZE &&
           bytes_in_buffer == LIST_BUFFER_SIZE) {
      bytes_in_buffer = (memSizeType) fread(currBuffer->buffer, 1, LIST_BUFFER_SIZE, inFile);
      logMessage(printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n",
                        bytes_in_buffer););
      if (unlikely(bytes_in_buffer == 0 && result_size == 0 && ferror(inFile))) {
        logError(printf("read_and_alloc_stri: "
                        "fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        (memSizeType) LIST_BUFFER_SIZE, safe_fileno(inFile),
                        errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } else {
        result_size += bytes_in_buffer;
        if (chars_missing > result_size && bytes_in_buffer == LIST_BUFFER_SIZE) {
          currBuffer->next = (bufferList) malloc(sizeof(struct bufferStruct));
          if (unlikely(currBuffer->next == NULL)) {
            logError(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *): "
                            "malloc(" FMT_U_MEM ") failed.\n",
                            safe_fileno(inFile), chars_missing,
                            (memSizeType) sizeof(struct bufferStruct)););
            *err_info = MEMORY_ERROR;
            /* Leave the while loop by setting bytes_in_buffer to zero. */
            bytes_in_buffer = 0;
          } else {
            currBuffer = currBuffer->next;
            currBuffer->next = NULL;
          } /* if */
        } /* if */
      } /* if */
    } /* while */
    if (chars_missing > result_size &&
        bytes_in_buffer == LIST_BUFFER_SIZE) {
      bytes_in_buffer = (memSizeType) fread(currBuffer->buffer, 1,
                                            chars_missing - result_size, inFile);
      logMessage(printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n",
                        bytes_in_buffer););
      if (unlikely(bytes_in_buffer == 0 && result_size == 0 && ferror(inFile))) {
        logError(printf("read_and_alloc_stri: "
                        "fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        chars_missing - result_size, safe_fileno(inFile),
                        errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } else {
        result_size += bytes_in_buffer;
      } /* if */
    } /* if */
    if (unlikely(*err_info != OKAY_NO_ERROR)) {
      result = NULL;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        logError(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *): "
                        "ALLOC_STRI_SIZE_OK(*, " FMT_U_MEM ") failed.\n",
                        safe_fileno(inFile), chars_missing, result_size););
        *err_info = MEMORY_ERROR;
      } else {
        result->size = result_size;
        currBuffer = &buffer;
        result_pos = 0;
        while (result_size - result_pos >= LIST_BUFFER_SIZE) {
          memcpy_to_strelem(&result->mem[result_pos], currBuffer->buffer, LIST_BUFFER_SIZE);
          currBuffer = currBuffer->next;
          result_pos += LIST_BUFFER_SIZE;
        } /* while */
        memcpy_to_strelem(&result->mem[result_pos], currBuffer->buffer,
                          result_size - result_pos);
      } /* if */
    } /* if */
    currBuffer = buffer.next;
    while (currBuffer != NULL) {
      oldBuffer = currBuffer;
      currBuffer = currBuffer->next;
      free(oldBuffer);
    } /* while */
    logFunction(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", %d) -->\n",
                       safe_fileno(inFile), chars_missing, *err_info););
    return result;
  } /* read_and_alloc_stri */



#if !HAS_SIGACTION && !HAS_SIGNAL
/**
 *  Read a character from 'inFile' and check if ctrl-c is pressed.
 *  @param inFile File from which the character is read.
 *  @param sigintReceived Flag indicating if ctrl-c has been pressed.
 *  @return the character read.
 */
int readCharChkCtrlC (cFileType inFile, boolType *sigintReceived)

  {
    int ch;

  /* readCharChkCtrlC */
    logFunction(printf("readCharChkCtrlC(%d, %d)\n",
                       safe_fileno(inFile), *sigintReceived););
    ch = getc(inFile);
    logFunction(printf("readCharChkCtrlC(%d, %d) --> %d\n",
                       safe_fileno(inFile), *sigintReceived, ch););
    return ch;
  } /* readCharChkCtrlC */
#endif



/**
 *  Read a character from 'inFile' and handle when ctrl-c is pressed.
 *  If the user decides that the program should resume a prompt to
 *  re-enter the input is written. This is the central function to read from
 *  a terminal. It is used by doGetcFromTerminal(), doGetsFromTerminal(),
 *  doLineReadFromTerminal() and doWordReadFromTerminal().
 *  @param inFile File from which the character is read.
 *  @param sigintReceived Flag indicating that ctrl-c has been pressed and
 *         no resume is done.
 *  @return the character read.
 */
static int readCharFromTerminal (cFileType inFile, boolType *sigintReceived)

  {
    boolType resume;
    int ch;

  /* readCharFromTerminal */
    do {
      resume = FALSE;
      ch = readCharChkCtrlC(inFile, sigintReceived);
      if (unlikely(*sigintReceived)) {
        resume = callSignalHandler(SIGINT);
        if (resume) {
          printf("re-enter input> ");
          fflush(stdout);
        } /* if */
      } /* if */
    } while (resume);
    return ch;
  } /* readCharFromTerminal */



static charType doGetcFromTerminal (cFileType inFile)

  {
    boolType sigintReceived = FALSE;
    charType ch;

  /* doGetcFromTerminal */
    ch = (charType) readCharFromTerminal(inFile, &sigintReceived);
    if (unlikely(sigintReceived)) {
      ch = (charType) 3;
    } /* if */
    return ch;
  } /* doGetcFromTerminal */



static striType doGetsFromTerminal (fileType inFile, intType length)

  {
    cFileType cInFile;
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doGetsFromTerminal */
    logFunction(printf("doGetsFromTerminal(%s%d, " FMT_D ")\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       length););
    cInFile = inFile->cFile;
    if (unlikely(length <= 0)) {
      if (unlikely(length != 0)) {
        logError(printf("doGetsFromTerminal(%d, " FMT_D "): Negative length.\n",
                        safe_fileno(cInFile), length););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } /* if */
    } else {
      ch = readCharFromTerminal(cInFile, &sigintReceived);
      if (unlikely(sigintReceived)) {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } else {
        if (ch != EOF) {
          ungetc(ch, cInFile);
        } /* if */
        result = filGets(inFile, length);
      } /* if */
    } /* if */
    return result;
  } /* doGetsFromTerminal */



static striType doLineReadFromTerminal (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doLineReadFromTerminal */
    logFunction(printf("doLineReadFromTerminal(%s%d, '\\" FMT_U32 ";')\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    ch = readCharFromTerminal(cInFile, &sigintReceived);
    if (unlikely(sigintReceived)) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } else {
      if (ch != EOF) {
        ungetc(ch, cInFile);
      } /* if */
      result = filLineRead(inFile, terminationChar);
    } /* if */
    return result;
  } /* doLineReadFromTerminal */



static striType doWordReadFromTerminal (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doWordReadFromTerminal */
    logFunction(printf("doWordReadFromTerminal(%s%d, '\\" FMT_U32 ";')\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    ch = readCharFromTerminal(cInFile, &sigintReceived);
    if (unlikely(sigintReceived)) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } else {
      if (ch != EOF) {
        ungetc(ch, cInFile);
      } /* if */
      result = filWordRead(inFile, terminationChar);
    } /* if */
    return result;
  } /* doWordReadFromTerminal */



/**
 *  Determine the size of a file and return it as bigInteger.
 *  The file length is measured in bytes.
 *  @return the size of the given file.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file length reported by the system is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType filBigLng (fileType aFile)

  {
    cFileType cFile;
    bigIntType length;

  /* filBigLng */
    logFunction(printf("filBigLng(%s%d)\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filBigLng: Attempt to get the length of a closed file.\n"););
      raise_error(FILE_ERROR);
      length = NULL;
    } else {
      /* os_fstat() is not used, because when writing to a */
      /* file the stat data is only updated after a flush. */
      length = getBigFileLengthUsingSeek(cFile);
    } /* if */
    logFunction(printf("filBigLng --> %s\n", bigHexCStri(length)););
    return length;
  } /* filBigLng */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @exception RANGE_ERROR The file position is negative or zero or
 *             the file position is not representable in the system
 *             file position type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void filBigSeek (fileType aFile, const const_bigIntType position)

  {
    cFileType cFile;
    errInfoType err_info = OKAY_NO_ERROR;
    os_off_t file_position;

  /* filBigSeek */
    logFunction(printf("filBigSeek(%s%d, %s)\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       bigHexCStri(position)););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filBigSeek: Attempt to set the current position of a closed file.\n"););
      raise_error(FILE_ERROR);
    } else {
#if OS_OFF_T_SIZE == 32
      file_position = (os_off_t) bigToInt32(position, &err_info);
#elif OS_OFF_T_SIZE == 64
      file_position = (os_off_t) bigToInt64(position, &err_info);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
      if (unlikely(file_position <= 0)) {
        logError(printf("filBigSeek(%d, " FMT_D_OFF "): Position <= 0%s.\n",
                        safe_fileno(cFile), file_position, file_position == 0 ?
                            " or conversion from bigInteger failed" : ""););
        raise_error(RANGE_ERROR);
      } else if (unlikely(offsetSeek(cFile, file_position - 1, SEEK_SET) != 0)) {
        logError(printf("filBigSeek(%d, %s): "
                        "offsetSeek(%d, " FMT_D64 ", SEEK_SET) failed.\n",
                        safe_fileno(cFile), bigHexCStri(position),
                        safe_fileno(cFile), (int64Type) (file_position - 1)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* filBigSeek */



/**
 *  Obtain the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @return the current file position.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file position reported by the system is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType filBigTell (fileType aFile)

  {
    cFileType cFile;
    os_off_t current_file_position;
    bigIntType position;

  /* filBigTell */
    logFunction(printf("filBigTell(%s%d)\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filBigTell: Attempt to get the current position of a closed file.\n"););
      raise_error(FILE_ERROR);
      position = NULL;
    } else {
      current_file_position = offsetTell(cFile);
      if (unlikely(current_file_position < (os_off_t) 0)) {
        logError(printf("filBigTell(%d): offsetTell(%d) "
                        "returns negative offset: " FMT_D64 ".\n",
                        safe_fileno(cFile), safe_fileno(cFile),
                        (int64Type) current_file_position););
        raise_error(FILE_ERROR);
        position = NULL;
      } else {
#if OS_OFF_T_SIZE == 32
        position = bigFromUInt32((uint32Type) current_file_position + 1);
#elif OS_OFF_T_SIZE == 64
        position = bigFromUInt64((uint64Type) current_file_position + 1);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
      } /* if */
    } /* if */
    logFunction(printf("filBigTell --> %s\n", bigHexCStri(position)););
    return position;
  } /* filBigTell */



/**
 *  Close a clib_file.
 *  @exception FILE_ERROR A system function returns an error.
 */
void filClose (fileType aFile)

  { /* filClose */
    logFunction(printf("filClose(" FMT_U_MEM " %s%d (usage=" FMT_U "))\n",
                       (memSizeType) aFile,
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       aFile != NULL ? aFile->usage_count : (uintType) 0););
    if (unlikely(aFile->cFile == NULL)) {
      logError(printf("filClose: Called with a closed file.\n"););
      raise_error(FILE_ERROR);
    } else {
#if FCLOSE_FAILS_AFTER_PREVIOUS_ERROR
      clearerr(aFile->cFile);
#endif
      if (unlikely(fclose(aFile->cFile) != 0)) {
        logError(printf("filClose: fclose(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(aFile->cFile),
                        errno, strerror(errno)););
        /* After the call to fclose(), any use of */
        /* cFile results in undefined behavior.   */
        aFile->cFile = NULL;
        raise_error(FILE_ERROR);
      } else {
        aFile->cFile = NULL;
      } /* if */
    } /* if */
    logFunction(printf("filClose(" FMT_U_MEM " %d (usage=" FMT_U ")) -->\n",
                       (memSizeType) aFile, safe_fileno(aFile->cFile),
                       aFile->usage_count););
  } /* filClose */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 */
void filCpy (fileType *const dest, const fileType source)

  { /* filCpy */
    logFunction(printf("filCpy(" FMT_U_MEM " %s%d (usage=" FMT_U "), "
                       FMT_U_MEM " %s%d (usage=" FMT_U "))\n",
                       (memSizeType) *dest,
                       *dest == NULL ? "NULL " : "",
                       *dest != NULL ? safe_fileno((*dest)->cFile) : 0,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source == NULL ? "NULL " : "",
                       source != NULL ? safe_fileno(source->cFile) : 0,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    if (*dest != NULL && (*dest)->usage_count != 0) {
      (*dest)->usage_count--;
      if ((*dest)->usage_count == 0) {
        filFree(*dest);
      } /* if */
    } /* if */
    *dest = source;
    logFunction(printf("filCpy(" FMT_U_MEM " %s%d (usage=" FMT_U "), "
                       FMT_U_MEM " %s%d (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest == NULL ? "NULL " : "",
                       *dest != NULL ? safe_fileno((*dest)->cFile) : 0,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source == NULL ? "NULL " : "",
                       source != NULL ? safe_fileno(source->cFile) : 0,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* filCpy */



/**
 *  Reinterpret the generic parameters as fileType and call filCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(fileType).
 */
void filCpyGeneric (genericType *const dest, const genericType source)

  { /* filCpyGeneric */
    filCpy(&((rtlObjectType *) dest)->value.fileValue,
           ((const_rtlObjectType *) &source)->value.fileValue);
  } /* filCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
fileType filCreate (const fileType source)

  { /* filCreate */
    logFunction(printf("filCreate(" FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source == NULL ? "NULL " : "",
                       source != NULL ? safe_fileno(source->cFile) : 0,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    logFunction(printf("filCreate --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source == NULL ? "NULL " : "",
                       source != NULL ? safe_fileno(source->cFile) : 0,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* filCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(fileType).
 */
genericType filCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* filCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.fileValue =
        filCreate(((const_rtlObjectType *) &from_value)->value.fileValue);
    return result.value.genericValue;
  } /* filCreateGeneric */



/**
 *  Maintain a usage count and free an unused file 'oldFile'.
 *  After a file is freed 'oldFile' refers to not existing memory.
 *  The memory where 'oldFile' is stored can be freed after filDestr.
 */
void filDestr (const fileType oldFile)

  { /* filDestr */
    logFunction(printf("filDestr(" FMT_U_MEM " %s%d (usage=" FMT_U "))\n",
                       (memSizeType) oldFile,
                       oldFile == NULL ? "NULL " : "",
                       oldFile != NULL ? safe_fileno(oldFile->cFile) : 0,
                       oldFile != NULL ? oldFile->usage_count : (uintType) 0););
    if (oldFile != NULL && oldFile->usage_count != 0) {
      oldFile->usage_count--;
      if (oldFile->usage_count == 0) {
        filFree(oldFile);
      } /* if */
    } /* if */
  } /* filDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(fileType).
 */
void filDestrGeneric (const genericType old_value)

  { /* filDestrGeneric */
    filDestr(((const_rtlObjectType *) &old_value)->value.fileValue);
  } /* filDestrGeneric */



boolType filEof (fileType inFile)

  {
    cFileType cInFile;
    boolType eofIndicator;

  /* filEof */
    logFunction(printf("filEof(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filEof: Attempt to test a closed file.\n"););
      raise_error(FILE_ERROR);
      eofIndicator = FALSE;
    } else {
      eofIndicator = feof(cInFile) != 0;
    } /* if */
    logFunction(printf("filEof --> %d\n", eofIndicator););
    return eofIndicator;
  } /* filEof */



#ifdef OUT_OF_ORDER
intType filFileType (fileType aFile)

  {
    cFileType cFile;
    int file_no;
    os_fstat_struct stat_buf;
    intType result;

  /* filFileType */
    cFile = aFile->cFile;
    file_no = os_fileno(cFile);
    if (unlikely(file_no == -1 ||
                 os_fstat(file_no, &stat_buf) != 0)) {
      result = 0;
      raise_error(FILE_ERROR);
    } else {
      if (S_ISREG(stat_buf.st_mode)) {
        result = 2;
      } else if (S_ISDIR(stat_buf.st_mode)) {
        result = 3;
      } else if (S_ISCHR(stat_buf.st_mode)) {
        result = 4;
      } else if (S_ISBLK(stat_buf.st_mode)) {
        result = 5;
      } else if (S_ISFIFO(stat_buf.st_mode)) {
        result = 6;
      } else if (S_ISLNK(stat_buf.st_mode)) {
        result = 7;
      } else if (S_ISSOCK(stat_buf.st_mode)) {
        result = 8;
      } else {
        result = 1;
      } /* if */
    } /* if */
    return result;
  } /* filFileType */
#endif



void filFlush (fileType outFile)

  {
    cFileType cOutFile;

  /* filFlush */
    logFunction(printf("filFlush(%s%d)\n",
                       outFile == NULL ? "NULL " : "",
                       outFile != NULL ? safe_fileno(outFile->cFile) : 0););
    cOutFile = outFile->cFile;
    if (unlikely(cOutFile == NULL)) {
      logError(printf("filFlush: Attempt to flush a closed file.\n"););
      raise_error(FILE_ERROR);
    } else {
      fflush(cOutFile);
    } /* if */
  } /* filFlush */



void filFree (fileType oldFile)

  { /* filFree */
    logFunction(printf("filFree(" FMT_U_MEM " %s%d (usage=" FMT_U "))\n",
                       (memSizeType) oldFile,
                       oldFile == NULL ? "NULL " : "",
                       oldFile != NULL ? safe_fileno(oldFile->cFile) : 0,
                       oldFile != NULL ? oldFile->usage_count : (uintType) 0););
    if (oldFile->cFile != NULL) {
      fclose(oldFile->cFile);
    } /* if */
    FREE_RECORD(oldFile, fileRecord, count.files);
  } /* filFree */



/**
 *  Read a character from a clib_file.
 *  @return the character read, or EOF at the end of the file.
 */
charType filGetcChkCtrlC (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    charType result;

  /* filGetcChkCtrlC */
    logFunction(printf("filGetcChkCtrlC(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile)
                                      : 0););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filGetcChkCtrlC: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = 0;
    } else {
      file_no = os_fileno(cInFile);
      if (file_no != -1 && os_isatty(file_no)) {
        result = doGetcFromTerminal(cInFile);
      } else {
        result = (charType) getc(cInFile);
      } /* if */
    } /* if */
    logFunction(printf("filGetcChkCtrlC --> '\\" FMT_U32 ";'\n",
                       result););
    return result;
  } /* filGetcChkCtrlC */



/**
 *  Read a string with 'length' characters from 'inFile'.
 *  In order to work reasonable good for the common case (reading
 *  just some characters), memory for 'length' characters is requested
 *  with malloc(). After the data is read the result string is
 *  shrunk to the actual size (with realloc()). If 'length' is
 *  larger than GETS_DEFAULT_SIZE or the memory cannot be requested
 *  a different strategy is used. In this case the function tries to
 *  find out the number of available characters (this is possible
 *  for a regular file but not for a pipe). If this fails a third
 *  strategy is used. In this case data is read into a list of buffers.
 *  After the data has been read memory is requested, the data is
 *  copied from the buffers and the list of buffers is freed.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType filGets (fileType inFile, intType length)

  {
    cFileType cInFile;
    memSizeType chars_requested;
    memSizeType bytes_there;
    memSizeType allocated_size;
    memSizeType num_of_chars_read;
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* filGets */
    logFunction(printf("filGets(" FMT_U_MEM " %s%d, " FMT_D ")\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       length););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filGets: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
    } else if (unlikely(length <= 0)) {
      if (unlikely(length != 0)) {
        logError(printf("filGets(%d, " FMT_D "): Negative length.\n",
                        safe_fileno(cInFile), length););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } /* if */
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("filGets(%d, " FMT_D "): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile), length););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        chars_requested = MAX_MEMSIZETYPE;
      } else {
        chars_requested = (memSizeType) length;
      } /* if */
      if (chars_requested > GETS_DEFAULT_SIZE) {
        /* Avoid requesting too much */
        result = NULL;
      } else {
        allocated_size = chars_requested;
        (void) ALLOC_STRI_SIZE_OK(result, allocated_size);
      } /* if */
      if (result == NULL) {
        bytes_there = remainingBytesInFile(cInFile);
        logMessage(printf("filGets: bytes_there=" FMT_U_MEM "\n",
                          bytes_there););
        if (bytes_there != 0) {
          /* Now we know that bytes_there bytes are available in cInFile */
          if (chars_requested <= bytes_there) {
            allocated_size = chars_requested;
          } else {
            allocated_size = bytes_there;
          } /* if */
          logMessage(printf("filGets: allocated_size=" FMT_U_MEM "\n",
                            allocated_size););
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, allocated_size))) {
            /* printf("MAX_STRI_LEN= " FMT_U_MEM
                ", SIZ_STRI(MAX_STRI_LEN)=" FMT_U_MEM "\n",
                MAX_STRI_LEN, SIZ_STRI(MAX_STRI_LEN)); */
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
      } /* if */
      if (result != NULL) {
        /* We have allocated a buffer for the requested number of chars
           or for the number of bytes which are available in the file */
        result->size = allocated_size;
        logMessage(printf("filGets: allocated_size=" FMT_U_MEM "\n",
                          allocated_size););
        num_of_chars_read = (memSizeType) fread(result->mem, 1,
            (size_t) allocated_size, cInFile);
        logMessage(printf("filGets: num_of_chars_read=" FMT_U_MEM "\n",
                          num_of_chars_read););
        if (unlikely(num_of_chars_read == 0 && ferror(cInFile))) {
          logError(printf("filGets: fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          allocated_size, safe_fileno(cInFile),
                          errno, strerror(errno)););
          err_info = FILE_ERROR;
        } else {
          memcpy_to_strelem(result->mem, (ustriType) result->mem,
                            num_of_chars_read);
          if (num_of_chars_read < result->size) {
            REALLOC_STRI_SIZE_SMALLER(resized_result, result,
                                      num_of_chars_read);
            if (unlikely(resized_result == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              result = resized_result;
              result->size = num_of_chars_read;
            } /* if */
          } /* if */
        } /* if */
      } else {
        /* Read a string, if we do not know how many bytes are available. */
        result = read_and_alloc_stri(cInFile, chars_requested, &err_info);
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        if (result != NULL) {
          FREE_STRI(result);
        } /* if */
        raise_error(err_info);
        result = NULL;
      } /* if */
    } /* if */
    logFunction(printf("filGets(%d, " FMT_D ") --> \"%s\"\n",
                       safe_fileno(cInFile), length,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filGets */



striType filGetsChkCtrlC (fileType inFile, intType length)

  {
    cFileType cInFile;
    int file_no;
    striType result;

  /* filGetsChkCtrlC */
    logFunction(printf("filGetsChkCtrlC(" FMT_U_MEM " %s%d, " FMT_D ")\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       length););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filGetsChkCtrlC: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      file_no = os_fileno(cInFile);
      if (file_no != -1 && os_isatty(file_no)) {
        result = doGetsFromTerminal(inFile, length);
      } else {
        result = filGets(inFile, length);
      } /* if */
    } /* if */
    logFunction(printf("filGetsChkCtrlC(%d, " FMT_D ") --> \"%s\"\n",
                       safe_fileno(cInFile), length, striAsUnquotedCStri(result)););
    return result;
  } /* filGetsChkCtrlC */



/**
 *  Determine if at least one character can be read successfully.
 *  This function allows a file to be handled like an iterator.
 *  @return FALSE if 'getc' would return EOF, TRUE otherwise.
 */
boolType filHasNext (fileType inFile)

  {
    cFileType cInFile;
    int next_char;
    boolType hasNext;

  /* filHasNext */
    logFunction(printf("filHasNext(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filHasNext: Attempt to test a closed file.\n"););
      raise_error(FILE_ERROR);
      hasNext = FALSE;
    } else if (feof(cInFile)) {
      hasNext = FALSE;
    } else {
      next_char = getc(cInFile);
      if (next_char != EOF) {
        if (unlikely(ungetc(next_char, cInFile) != next_char)) {
          logError(printf("filHasNext: ungetc('\\" FMT_U32 ";', %d) "
                          "does not return '\\" FMT_U32 ";'.\n",
                          next_char, safe_fileno(cInFile), next_char););
          raise_error(FILE_ERROR);
          hasNext = FALSE;
        } else {
          hasNext = TRUE;
        } /* if */
      } else {
        clearerr(cInFile);
        hasNext = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filHasNext(%d) --> %d\n",
                       safe_fileno(cInFile), hasNext););
    return hasNext;
  } /* filHasNext */



boolType filHasNextChkCtrlC (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    int next_char;
    boolType hasNext;

  /* filHasNextChkCtrlC */
    logFunction(printf("filHasNextChkCtrlC(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filHasNextChkCtrlC: Attempt to test a closed file.\n"););
      raise_error(FILE_ERROR);
      hasNext = FALSE;
    } else if (feof(cInFile)) {
      hasNext = FALSE;
    } else {
      file_no = os_fileno(cInFile);
      if (file_no != -1 && os_isatty(file_no)) {
        next_char = (int) (scharType) doGetcFromTerminal(cInFile);
      } else {
        next_char = getc(cInFile);
      } /* if */
      if (next_char != EOF) {
        if (unlikely(ungetc(next_char, cInFile) != next_char)) {
          logError(printf("filHasNextChkCtrlC: "
                          "ungetc('\\" FMT_U32 ";', %d) "
                          "does not return '\\" FMT_U32 ";'.\n",
                          next_char, safe_fileno(cInFile), next_char););
          raise_error(FILE_ERROR);
          hasNext = FALSE;
        } else {
          hasNext = TRUE;
        } /* if */
      } else {
        clearerr(cInFile);
        hasNext = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filHasNextChkCtrlC(%d) --> %d\n",
                       safe_fileno(cInFile), hasNext););
    return hasNext;
  } /* filHasNextChkCtrlC */



/**
 *  Read a line from a clib_file.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains '\n' or EOF.
 *  @return the line read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType filLineRead (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    register int ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* filLineRead */
    logFunction(printf("filLineRead(" FMT_U_MEM " %s%d, '\\"
                       FMT_U32 ";')\n", (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filLineRead: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("filLineRead(%d, *): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile)););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      memlength = READ_STRI_INIT_SIZE;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
        raise_error(MEMORY_ERROR);
      } else {
        memory = result->mem;
        position = 0;
        flockfile(cInFile);
        while ((ch = getc_unlocked(cInFile)) != '\n' && ch != EOF) {
          if (position >= memlength) {
            newmemlength = memlength + READ_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE2(resized_result, result, memlength, newmemlength);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI2(result, memlength);
              funlockfile(cInFile);
              raise_error(MEMORY_ERROR);
              return NULL;
            } /* if */
            result = resized_result;
            memory = result->mem;
            memlength = newmemlength;
          } /* if */
          memory[position++] = (strElemType) ch;
        } /* while */
        funlockfile(cInFile);
        if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
          position--;
        } /* if */
        if (unlikely(ch == EOF && position == 0 && ferror(cInFile))) {
          FREE_STRI2(result, memlength);
          logError(printf("filLineRead(%d, '\\" FMT_U32 ";'): "
                          "getc_unlocked(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(cInFile), *terminationChar,
                          safe_fileno(cInFile), errno, strerror(errno)););
          raise_error(FILE_ERROR);
          result = NULL;
        } else {
          REALLOC_STRI_SIZE_SMALLER2(resized_result, result, memlength, position);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI2(result, memlength);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            result->size = position;
            *terminationChar = (charType) ch;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filLineRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filLineRead */



striType filLineReadChkCtrlC (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    int file_no;
    striType result;

  /* filLineReadChkCtrlC */
    logFunction(printf("filLineReadChkCtrlC(" FMT_U_MEM " %s%d, '\\"
                       FMT_U32 ";')\n", (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filLineReadChkCtrlC: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      file_no = os_fileno(cInFile);
      if (file_no != -1 && os_isatty(file_no)) {
        result = doLineReadFromTerminal(inFile, terminationChar);
      } else {
        result = filLineRead(inFile, terminationChar);
      } /* if */
    } /* if */
    logFunction(printf("filLineReadChkCtrlC(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filLineReadChkCtrlC */



striType filLit (fileType aFile)

  {
    cFileType cFile;
    const_cstriType file_name;
    striType result;

  /* filLit */
    cFile = aFile->cFile;
    if (cFile == NULL) {
      file_name = "NULL";
    } else if (cFile == stdin) {
      file_name = "stdin";
    } else if (cFile == stdout) {
      file_name = "stdout";
    } else if (cFile == stderr) {
      file_name = "stderr";
    } else {
      file_name = "file";
    } /* if */
    result = cstri_to_stri(file_name);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* filLit */



/**
 *  Obtain the length of a file.
 *  The file length is measured in bytes.
 *  @return the size of the given file.
 *  @exception RANGE_ERROR The file length does not fit into
 *             an integer value.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file length reported by the system is negative.
 */
intType filLng (fileType aFile)

  {
    cFileType cFile;
    intType length;

  /* filLng */
    logFunction(printf("filLng(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) aFile,
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filLng: Attempt to get the length of a closed file.\n"););
      raise_error(FILE_ERROR);
      length = 0;
    } else {
      /* os_fstat() is not used, because when writing to a */
      /* file the stat data is only updated after a flush. */
      length = getFileLengthUsingSeek(cFile);
    } /* if */
    logFunction(printf("filLng --> " FMT_D "\n", length););
    return length;
  } /* filLng */



/**
 *  Opens a file with the specified 'path' and 'os_mode'.
 *  The 'os_mode' is used by the C function fopen().
 *  There are text modes and binary modes:
 *  - Binary modes:
 *   - "rb"  Open file for reading.
 *   - "wb"  Open or create file for writing and truncate to zero length.
 *   - "ab"  Open or create file for appending (writing at end-of-file).
 *   - "rb+" Open file for update (reading and writing).
 *   - "wb+" Open or create file for update and truncate to zero length.
 *   - "ab+" Open or create file for appending and reading.
 *  - Text modes:
 *   - "r"   Open file for reading.
 *   - "w"   Open or create file for writing and truncate to zero length.
 *   - "a"   Open or create file for appending (writing at end-of-file).
 *   - "r+"  Open file for update (reading and writing).
 *   - "w+"  Open or create file for update and truncate to zero length.
 *   - "a+"  Open or create file for appending and reading.
 *
 *  Unicode characters in 'path' are converted to the
 *  representation used by the fopen() function of the operating system.
 *  @param path Path of the file to be opened. The path must
 *         use the standard path representation.
 *  @param os_mode Mode of the file to be opened.
 *  @param err_info Unchanged if the function succeeds, or
 *                  MEMORY_ERROR if there is not enough memory to convert
 *                        the path to the system path type, or
 *                  RANGE_ERROR if 'path' does not use the standard path
 *                        representation or 'path' cannot be converted
 *                        to the system path type.
 *  @return the file opened, or NULL if it could not be opened or
 *          if 'path' refers to a directory.
 */
static cFileType cFileOpen (const const_striType path,
    const const_os_striType os_mode, errInfoType *err_info)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
#if FOPEN_OPENS_DIRECTORIES
    int file_no;
    os_fstat_struct stat_buf;
#endif
    cFileType result;

  /* cFileOpen */
    logFunction(printf("cFileOpen(\"%s\", \"" FMT_S_OS "\", *)\n",
                       striAsUnquotedCStri(path), os_mode););
    os_path = cp_to_os_path(path, &path_info, err_info);
    /* printf("os_path \"%ls\" %d %d\n", os_path, path_info, *err_info); */
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT ||
          path_info == PATH_NOT_MAPPED) {
        /* Cannot open this file. Do not raise an exception. */
        *err_info = OKAY_NO_ERROR;
      } else
#endif
      {
        logError(printf("cFileOpen: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(path), path_info, *err_info););
      }
      result = NULL;
    } else {
      logMessage(printf("os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                        os_path, os_mode););
      result = os_fopen(os_path, os_mode);
      if (unlikely(result == NULL)) {
        logError(printf("cFileOpen: "
                        "fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, os_mode, errno, strerror(errno)););
      } else {
#if !FOPEN_SUPPORTS_CLOEXEC_MODE && HAS_FCNTL_SETFD_CLOEXEC
        file_no = os_fileno(result);
        if (file_no != -1) {
          fcntl(file_no, F_SETFD, fcntl(file_no, F_GETFD) | FD_CLOEXEC);
        } /* if */
#endif
#if FOPEN_OPENS_DIRECTORIES
        file_no = os_fileno(result);
        if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
            S_ISDIR(stat_buf.st_mode)) {
          /* An attempt to open a directory with cFileOpen()   */
          /* returns NULL even if fopen() succeeds. On many    */
          /* modern operating systems functions like fgetc()   */
          /* and readf() fail to read from a directory anyway. */
          /* So it is better to fail early, when the file is   */
          /* opened, instead of later at an unexpected place.  */
          /* Even if reading a directory as file succeeds      */
          /* there is another issue: Reading a directory as    */
          /* file is not portable, since it delivers an        */
          /* operating system specific representation of the   */
          /* directory. So reading a directory as file should  */
          /* be avoided altogether. The functions dirOpen(),   */
          /* dirRead() and dirClose() provide a portable way   */
          /* to open, read and close a directory.              */
          fclose(result);
          logError(printf("cFileOpen: "
                          "fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") "
                          "opened a directory. Close it and return NULL.\n",
                          os_path, os_mode););
          result = NULL;
        } /* if */
#endif
      } /* if */
      os_stri_free(os_path);
    } /* if */
    logFunction(printf("cFileOpen(\"%s\", \"" FMT_S_OS "\", %d) --> %d\n",
                       striAsUnquotedCStri(path), os_mode,
                       *err_info, safe_fileno(result)););
    return result;
  } /* cFileOpen */



/**
 *  Opens a file with the specified 'path' and 'mode'.
 *  There are text modes and binary modes:
 *  - Binary modes:
 *   - "r"   Open file for reading.
 *   - "w"   Open or create file for writing and truncate to zero length.
 *   - "a"   Open or create file for appending (writing at end-of-file).
 *   - "r+"  Open file for update (reading and writing).
 *   - "w+"  Open or create file for update and truncate to zero length.
 *   - "a+"  Open or create file for appending and reading.
 *  - Text modes:
 *   - "rt"  Open file for reading.
 *   - "wt"  Open or create file for writing and truncate to zero length.
 *   - "at"  Open or create file for appending (writing at end-of-file).
 *   - "rt+" Open file for update (reading and writing).
 *   - "wt+" Open or create file for update and truncate to zero length.
 *   - "at+" Open or create file for appending and reading.
 *
 *  Note that this modes differ from the ones used by the C function
 *  fopen(). Unicode characters in 'path' are converted to the
 *  representation used by the fopen() function of the operating system.
 *  @param path Path of the file to be opened. The path must
 *         use the standard path representation.
 *  @param mode Mode of the file to be opened.
 *  @return the file opened, or CLIB_NULL_FILE if it could not be opened or
 *          if 'path' refers to a directory.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system path type.
 *  @exception RANGE_ERROR The 'mode' is not one of the allowed
 *             values or 'path' does not use the standard path
 *             representation or 'path' cannot be converted
 *             to the system path type.
 */
fileType filOpen (const const_striType path, const const_striType mode)

  {
    os_charType os_mode[MAX_MODE_LEN];
    boolType readingAllowed = FALSE;
    boolType writingAllowed = FALSE;
    cFileType cFile;
    errInfoType err_info = OKAY_NO_ERROR;
    fileType fileOpened;

  /* filOpen */
    logFunction(printf("filOpen(\"%s\", ", striAsUnquotedCStri(path));
                printf("\"%s\")\n", striAsUnquotedCStri(mode)););
    get_mode(mode, os_mode, &readingAllowed, &writingAllowed);
    if (unlikely(os_mode[0] == '\0')) {
      logError(printf("filOpen(\"%s\", ", striAsUnquotedCStri(path));
               printf("\"%s\"): Illegal mode.\n", striAsUnquotedCStri(mode)););
      raise_error(RANGE_ERROR);
      fileOpened = NULL;
    } else {
      cFile = cFileOpen(path, os_mode, &err_info);
      if (unlikely(cFile == NULL)) {
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
          fileOpened = NULL;
        } else {
          fileOpened = &nullFileRecord;
        } /* if */
      } else {
        if (unlikely(!ALLOC_RECORD(fileOpened, fileRecord, count.files))) {
          fclose(cFile);
          raise_error(MEMORY_ERROR);
        } else {
          initFileType(fileOpened, readingAllowed, writingAllowed);
          fileOpened->cFile = cFile;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filOpen(\"%s\", ", striAsUnquotedCStri(path));
                printf("\"%s\") --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       striAsUnquotedCStri(mode),
                       (memSizeType) fileOpened,
                       fileOpened == NULL ? "NULL " : "",
                       fileOpened != NULL ? safe_fileno(fileOpened->cFile) : 0,
                       fileOpened != NULL ? fileOpened->usage_count : (uintType) 0););
    return fileOpened;
  } /* filOpen */



/**
 *  Open the null device of the operation system for reading and writing.
 *  @return the null device opened, or CLIB_NULL_FILE if it could not be opened.
 */
fileType filOpenNullDevice (void)

  {
    cFileType cFile;
    fileType fileOpened;

  /* filOpenNullDevice */
    logFunction(printf("filOpenNullDevice()\n"););
    cFile = fopen(NULL_DEVICE, "r+");
    if (unlikely(cFile == NULL)) {
      fileOpened = &nullFileRecord;
    } else {
      if (unlikely(!ALLOC_RECORD(fileOpened, fileRecord, count.files))) {
        fclose(cFile);
        raise_error(MEMORY_ERROR);
      } else {
        initFileType(fileOpened, TRUE, TRUE);
        fileOpened->cFile = cFile;
      } /* if */
    } /* if */
    logFunction(printf("filOpenNullDevice() --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) fileOpened,
                       fileOpened == NULL ? "NULL " : "",
                       fileOpened != NULL ? safe_fileno(fileOpened->cFile) : 0,
                       fileOpened != NULL ? fileOpened->usage_count : (uintType) 0););
    return fileOpened;
  } /* filOpenNullDevice */



/**
 *  Wait for the process associated with 'aPipe' to terminate.
 *  @param aPipe Pipe file to be closed (created by 'filPopen').
 *  @exception FILE_ERROR A system function returned an error.
 */
void filPclose (fileType aPipe)

  { /* filPclose */
    logFunction(printf("filPclose(%s%d)\n",
                       aPipe == NULL ? "NULL " : "",
                       aPipe != NULL ? safe_fileno(aPipe->cFile) : 0););
#if HAS_POPEN
    if (unlikely(aPipe->cFile == NULL)) {
      logError(printf("filPclose: Called with a closed file.\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(os_pclose(aPipe->cFile) == -1)) {
      logError(printf("filPclose: pclose(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aPipe->cFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } else {
      aPipe->cFile = NULL;
    } /* if */
#endif
    logFunction(printf("filPclose -->\n"););
  } /* filPclose */



/**
 *  Open a pipe to a shell 'command', with 'parameters'.
 *  The pipe can be used to read, respectively write data
 *  with Latin-1 or UTF-8 encoding. Parameters which contain
 *  a space must be enclosed in double quotes. The commands
 *  supported and the format of the 'parameters' are not
 *  covered by the description of the 'filPopen' function.
 *  Due to the usage of the operating system shell and external
 *  programs, it is hard to write portable programs, which use
 *  the 'filPopen' function.
 *  @param command Name of the command to be executed. A path must
 *         use the standard path representation.
 *  @param parameters Space separated list of parameters for
 *         the 'command', or "" if there are no parameters.
 *  @param mode A pipe can be opened with the binary modes
 *         "r" (read) and "w" (write) or with the text modes
 *         "rt" (read) and "wt" (write).
 *  @return the pipe file opened, or NULL if it could not be opened.
 *  @exception RANGE_ERROR 'command' is not representable as
 *             operating system path, or 'mode' is illegal.
 */
fileType filPopen (const const_striType command,
    const const_striType parameters, const const_striType mode)

  {
#if HAS_POPEN
    os_striType os_command;
    os_charType os_mode[MAX_MODE_LEN];
    boolType readingAllowed = FALSE;
    boolType writingAllowed = FALSE;
    int mode_pos = 0;
    errInfoType err_info = OKAY_NO_ERROR;
    cFileType cFile;
#endif
    fileType fileOpened;

  /* filPopen */
    logFunction(printf("filPopen(\"%s\", ", striAsUnquotedCStri(command));
                printf("\"%s\", ", striAsUnquotedCStri(parameters));
                printf("\"%s\")\n", striAsUnquotedCStri(mode)););
#if HAS_POPEN
    os_command = cp_to_command(command, parameters, &err_info);
    if (unlikely(os_command == NULL)) {
      logError(printf("filPopen: cp_to_command(\"%s\", ",
                      striAsUnquotedCStri(command));
               printf("\"%s\", *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(parameters), err_info););
      raise_error(err_info);
      fileOpened = NULL;
    } else {
      if (mode->size == 1 &&
          (mode->mem[0] == 'r' ||
           mode->mem[0] == 'w')) {
        os_mode[mode_pos++] = (os_charType) mode->mem[0];
#if POPEN_SUPPORTS_BINARY_MODE
        os_mode[mode_pos++] = 'b';
#endif
        readingAllowed = mode->mem[0] == 'r';
        writingAllowed = mode->mem[0] == 'w';
      } else if (mode->size == 2 &&
          (mode->mem[0] == 'r' ||
           mode->mem[0] == 'w') &&
           mode->mem[1] == 't') {
        os_mode[mode_pos++] = (os_charType) mode->mem[0];
#if POPEN_SUPPORTS_TEXT_MODE
        os_mode[mode_pos++] = 't';
#endif
        readingAllowed = mode->mem[0] == 'r';
        writingAllowed = mode->mem[0] == 'w';
      } /* if */
      if (unlikely(mode_pos == 0)) {
        logError(printf("filPopen: Illegal mode: \"%s\".\n",
                        striAsUnquotedCStri(mode)););
        raise_error(RANGE_ERROR);
        fileOpened = NULL;
      } else {
#if POPEN_SUPPORTS_CLOEXEC_MODE
        os_mode[mode_pos++] = 'e';
#endif
        os_mode[mode_pos] = '\0';
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
        adjustCwdForShell(&err_info);
#endif
        logMessage(printf("os_popen(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                          os_command, os_mode););
        cFile = os_popen(os_command, os_mode);
        if (unlikely(cFile == NULL)) {
          logError(printf("filPopen: os_popen(\"" FMT_S_OS "\","
                          " \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_command, os_mode,
                          errno, strerror(errno)););
          fileOpened = &nullFileRecord;
        } else {
          FREE_OS_STRI(os_command);
          if (unlikely(!ALLOC_RECORD(fileOpened, fileRecord, count.files))) {
            os_pclose(cFile);
            raise_error(MEMORY_ERROR);
            fileOpened = NULL;
          } else {
            initFileType(fileOpened, readingAllowed, writingAllowed);
            fileOpened->cFile = cFile;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#else
    fileOpened = &nullFileRecord;
#endif
    logFunction(printf("filPopen(\"%s\", ", striAsUnquotedCStri(command));
                printf("\"%s\", ", striAsUnquotedCStri(parameters));
                printf("\"%s\") --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       striAsUnquotedCStri(mode),
                       (memSizeType) fileOpened,
                       fileOpened == NULL ? "NULL " : "",
                       fileOpened != NULL ? safe_fileno(fileOpened->cFile) : 0,
                       fileOpened != NULL ? fileOpened->usage_count : (uintType) 0););
    return fileOpened;
  } /* filPopen */



void filPrint (const const_striType stri)

  { /* filPrint */
    ut8Write(&stdoutFileRecord, stri);
    putchar('\n');
    fflush(stdout);
  } /* filPrint */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @exception RANGE_ERROR The file position is negative or zero or
 *             the file position is not representable in the system
 *             file position type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void filSeek (fileType aFile, intType position)

  {
    cFileType cFile;

  /* filSeek */
    logFunction(printf("filSeek(" FMT_U_MEM " %s%d, " FMT_D ")\n",
                       (memSizeType) aFile,
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       position););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filSeek: Attempt to set the current position of a closed file.\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(position <= 0)) {
      logError(printf("filSeek(%d, " FMT_D "): Position <= 0.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#if OS_OFF_T_SIZE < INTTYPE_SIZE
#if OS_OFF_T_SIZE == 32
    } else if (unlikely(position > INT32TYPE_MAX)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#elif OS_OFF_T_SIZE == 64
    } else if (unlikely(position > INT64TYPE_MAX)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
#endif
    } else if (unlikely(offsetSeek(cFile, (os_off_t) (position - 1), SEEK_SET) != 0)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "offsetSeek(%d, " FMT_D ", SEEK_SET) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(cFile), position,
                      safe_fileno(cFile), position - 1,
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
    logFunction(printf("filSeek(%s%d, " FMT_D ") -->\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       position););
  } /* filSeek */



/**
 *  Determine if the file 'aFile' is seekable.
 *  If a file is seekable the functions filSeek() and filTell()
 *  can be used to set and and obtain the current file position.
 *  @return TRUE, if 'aFile' is seekable, FALSE otherwise.
 */
boolType filSeekable (fileType aFile)

  {
    cFileType cFile;
    int file_no;
    os_fstat_struct stat_buf;
    boolType seekable;

  /* filSeekable */
    logFunction(printf("filSeekable(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) aFile,
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filSeekable: Attempt to get propery of a closed file.\n"););
      raise_error(FILE_ERROR);
      seekable = FALSE;
    } else {
      file_no = os_fileno(cFile);
      if (file_no != -1) {
        if (os_fstat(file_no, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode)) {
          seekable = TRUE;
        } else if (os_lseek(file_no, (off_t) 0, SEEK_CUR) != (off_t) -1) {
          seekable = TRUE;
        } else {
          seekable = FALSE;
        } /* if */
      } else {
        seekable = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filSeekable(%s%d) --> %d\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       seekable););
    return seekable;
  } /* filSeekable */



void filSetbuf (fileType aFile, intType mode, intType size)

  {
    cFileType cFile;

  /* filSetbuf */
    logFunction(printf("filSetbuf(%s%d, " FMT_D ", " FMT_D ")\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       mode, size););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filSetbuf: Attempt to set the file buffering of a closed file.\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(mode < 0 || mode > 2 || size < 0 || (uintType) size > MAX_MEMSIZETYPE)) {
      logError(printf("filSetbuf(%d, " FMT_D ", " FMT_D "): "
                      "Mode or size not in allowed range.\n",
                      safe_fileno(cFile), mode, size););
      raise_error(RANGE_ERROR);
    } else if (unlikely(setvbuf(cFile, NULL, (int) mode, (memSizeType) size) != 0)) {
      logError(printf("filSetbuf: "
                      "setvbuf(%d, NULL, %d, " FMT_U_MEM ") failed.:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(cFile), (int) mode, (memSizeType) size,
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
  } /* filSetbuf */



/**
 *  Obtain the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @return the current file position.
 *  @exception RANGE_ERROR The file position does not fit into
 *             an integer value.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file position reported by the system is negative.
 */
intType filTell (fileType aFile)

  {
    cFileType cFile;
    os_off_t current_file_position;
    intType position;

  /* filTell */
    logFunction(printf("filTell(" FMT_U_MEM " %s%d)\n",
                       (memSizeType) aFile,
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filTell: Attempt to get the current position of a closed file.\n"););
      raise_error(FILE_ERROR);
      position = 0;
    } else {
      current_file_position = offsetTell(cFile);
      if (unlikely(current_file_position < (os_off_t) 0)) {
        logError(printf("filTell(%d): "
                        "offsetTell(%d) returns negative offset: " FMT_D64 ".\n",
                        safe_fileno(cFile), safe_fileno(cFile),
                        (int64Type) current_file_position););
        raise_error(FILE_ERROR);
        position = 0;
      } else if (unlikely(current_file_position >= INTTYPE_MAX)) {
        logError(printf("filTell(%d): "
                        "File position does not fit into an integer: " FMT_D_OFF ".\n",
                        safe_fileno(cFile), current_file_position););
        raise_error(RANGE_ERROR);
        position = 0;
      } else {
        position = (intType) (current_file_position + 1);
      } /* if */
    } /* if */
    logFunction(printf("filTell --> " FMT_D "\n", position););
    return position;
  } /* filTell */



/**
 *  Read a string from 'inFile' until the 'terminator' character is found.
 *  If a 'terminator' is found the string before the 'terminator' is
 *  returned and the 'terminator' character is assigned to 'terminationChar'.
 *  The file position is advanced after the 'terminator' character.
 *  If no 'terminator' is found the rest of 'inFile' is returned and
 *  EOF is assigned to the 'terminationChar'.
 *  @param inFile File from which the string is read.
 *  @param terminator Character which terminates the string.
 *  @param terminationChar Variable to receive the actual termination character
 *         (either 'terminator' or EOF).
 *  @return the string read without the 'terminator' or the rest of the
 *          file if no 'terminator' is found.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType filTerminatedRead (fileType inFile, charType terminator,
    charType *terminationChar)

  {
    cFileType cInFile;
    register int termCh;
    register int ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* filTerminatedRead */
    logFunction(printf("filTerminatedRead(%s%d, '\\" FMT_U32 ";', '\\" FMT_U32 ";')\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       terminator, *terminationChar););
    if (unlikely(terminator > 255)) {
      termCh = EOF;
    } else {
      termCh = (int) terminator;
    } /* if */
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filTerminatedRead: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("filTerminatedRead(%d, '\\" FMT_U32 ";', *): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile), terminator););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      memlength = READ_STRI_INIT_SIZE;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
        raise_error(MEMORY_ERROR);
      } else {
        memory = result->mem;
        position = 0;
        flockfile(cInFile);
        while ((ch = getc_unlocked(cInFile)) != termCh && ch != EOF) {
          if (position >= memlength) {
            newmemlength = memlength + READ_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE2(resized_result, result, memlength, newmemlength);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI2(result, memlength);
              funlockfile(cInFile);
              raise_error(MEMORY_ERROR);
              return NULL;
            } /* if */
            result = resized_result;
            memory = result->mem;
            memlength = newmemlength;
          } /* if */
          memory[position++] = (strElemType) ch;
        } /* while */
        funlockfile(cInFile);
        if (unlikely(ch == EOF && position == 0 && ferror(cInFile))) {
          FREE_STRI2(result, memlength);
          logError(printf("filTerminatedRead(%d, '\\" FMT_U32 ";', '\\" FMT_U32 ";'): "
                          "getc_unlocked(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(cInFile), terminator, *terminationChar,
                          safe_fileno(cInFile), errno, strerror(errno)););
          raise_error(FILE_ERROR);
          result = NULL;
        } else {
          REALLOC_STRI_SIZE_SMALLER2(resized_result, result, memlength, position);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI2(result, memlength);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            result->size = position;
            *terminationChar = (charType) ch;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filTerminatedRead(%d, '\\" FMT_U32 ";', '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), terminator, *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filTerminatedRead */



/**
 *  Truncate 'aFile' to the given 'length'.
 *  If the file previously was larger than 'length', the extra data is lost.
 *  If the file previously was shorter, it is extended, and the extended
 *  part is filled with null bytes ('\0;').
 *  @param aFile File to be truncated.
 *  @param length Requested length of 'aFile' in bytes.
 *  @exception RANGE_ERROR The requested length is negative or
 *             the length is not representable in the type
 *             used by the system function.
 *  @exception FILE_ERROR A system function returns an error.
 */
void filTruncate (fileType aFile, intType length)

  {
    cFileType cFile;
    int file_no;
    fpos_t pos;

  /* filTruncate */
    logFunction(printf("filTruncate(%s%d, " FMT_D ")\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       length););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("filTruncate: Attempt to truncate a closed file.\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(length < 0)) {
      logError(printf("filTruncate(%d, " FMT_D "): Length < 0.\n",
                      safe_fileno(cFile), length););
      raise_error(RANGE_ERROR);
#if FTRUNCATE_SIZE < INTTYPE_SIZE
#if FTRUNCATE_SIZE == 32
    } else if (unlikely(length > INT32TYPE_MAX)) {
      logError(printf("filTruncate(%d, " FMT_D "): "
                      "Length not representable in the system file length type.\n",
                      safe_fileno(cFile), length););
      raise_error(RANGE_ERROR);
#elif FTRUNCATE_SIZE == 64
    } else if (unlikely(length > INT64TYPE_MAX)) {
      logError(printf("filTruncate(%d, " FMT_D "): "
                      "Length not representable in the system file length type.\n",
                      safe_fileno(cFile), length););
      raise_error(RANGE_ERROR);
#else
#error "FTRUNCATE_SIZE is neither 32 nor 64."
#endif
#endif
    } else {
      file_no = os_fileno(cFile);
      if (unlikely(file_no == -1)) {
        logError(printf("filTruncate(%d): os_fileno(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(cFile), safe_fileno(cFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } else {
        /* Use fgetpos() and fsetpos() to ensure, that the internal buffer */
        /* of cFile is synchronized with the underlying file descriptor file. */
        /* This way nothing written is lost. */
        if (unlikely(fgetpos(cFile, &pos) != 0 || fsetpos(cFile, &pos) != 0)) {
          logError(printf("filTruncate(%d): fgetpos/fsetpos(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          file_no, file_no, errno, strerror(errno)););
          raise_error(FILE_ERROR);
        } else if (unlikely(os_ftruncate(file_no, length) != 0)) {
          logError(printf("filTruncate(%d, " FMT_D "): "
                          "ftruncate(%d, " FMT_D ") failed:\n"
                          "errno=%d\nerror: %s\n",
                          file_no, length, file_no, length,
                          errno, strerror(errno)););
          raise_error(FILE_ERROR);
        } /* if */
      } /* if */
    } /* if */
  } /* filTruncate */



/**
 *  Read a word from a clib_file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains ' ', '\t', '\n' or
 *  EOF.
 *  @return the word read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType filWordRead (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    register int ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* filWordRead */
    logFunction(printf("filWordRead(" FMT_U_MEM " %s%d, '\\"
                       FMT_U32 ";')\n", (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filWordRead: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("filWordRead(%d, *): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile)););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      memlength = READ_STRI_INIT_SIZE;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
        raise_error(MEMORY_ERROR);
      } else {
        memory = result->mem;
        position = 0;
        flockfile(cInFile);
        do {
          ch = getc_unlocked(cInFile);
        } while (ch == ' ' || ch == '\t');
        while (ch != ' ' && ch != '\t' &&
            ch != '\n' && ch != EOF) {
          if (position >= memlength) {
            newmemlength = memlength + READ_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE2(resized_result, result, memlength, newmemlength);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI2(result, memlength);
              funlockfile(cInFile);
              raise_error(MEMORY_ERROR);
              return NULL;
            } /* if */
            result = resized_result;
            memory = result->mem;
            memlength = newmemlength;
          } /* if */
          memory[position++] = (strElemType) ch;
          ch = getc_unlocked(cInFile);
        } /* while */
        funlockfile(cInFile);
        if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
          position--;
        } /* if */
        if (unlikely(ch == EOF && position == 0 && ferror(cInFile))) {
          FREE_STRI2(result, memlength);
          logError(printf("filWordRead(%d, '\\" FMT_U32 ";'): "
                          "getc_unlocked(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(cInFile), *terminationChar,
                          safe_fileno(cInFile), errno, strerror(errno)););
          raise_error(FILE_ERROR);
          result = NULL;
        } else {
          REALLOC_STRI_SIZE_SMALLER2(resized_result, result, memlength, position);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI2(result, memlength);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            result->size = position;
            *terminationChar = (charType) ch;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filWordRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filWordRead */



striType filWordReadChkCtrlC (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    int file_no;
    striType result;

  /* filWordReadChkCtrlC */
    logFunction(printf("filWordReadChkCtrlC(" FMT_U_MEM " %s%d, '\\"
                       FMT_U32 ";')\n", (memSizeType) inFile,
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filWordReadChkCtrlC: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      file_no = os_fileno(cInFile);
      if (file_no != -1 && os_isatty(file_no)) {
        result = doWordReadFromTerminal(inFile, terminationChar);
      } else {
        result = filWordRead(inFile, terminationChar);
      } /* if */
    } /* if */
    logFunction(printf("filWordReadChkCtrlC(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filWordReadChkCtrlC */



/**
 *  Write a string to a clib_file.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception RANGE_ERROR The string contains a character that does
 *             not fit into a byte.
 */
void filWrite (fileType outFile, const const_striType stri)

  {
    cFileType cOutFile;
    const strElemType *striElems;
    memSizeType len;
    ucharType buffer[BUFFER_SIZE];

  /* filWrite */
    logFunction(printf("filWrite(" FMT_U_MEM " %s%d, \"%s\")\n",
                       (memSizeType) outFile,
                       outFile == NULL ? "NULL " : "",
                       outFile != NULL ? safe_fileno(outFile->cFile) : 0,
                       striAsUnquotedCStri(stri)););
    cOutFile = outFile->cFile;
    if (unlikely(cOutFile == NULL)) {
      logError(printf("filWrite: Attempt to write to closed file.\n"););
      raise_error(FILE_ERROR);
      return;
    } /* if */
#if FWRITE_WRONG_FOR_READ_ONLY_FILES
    if (unlikely(stri->size > 0 && !outFile->writingAllowed)) {
      logError(printf("filWrite: Attempt to write to read only file: %d.\n",
                      safe_fileno(cOutFile)););
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    striElems = stri->mem;
    for (len = stri->size; len >= BUFFER_SIZE; len -= BUFFER_SIZE) {
      if (unlikely(memcpy_from_strelem(buffer, striElems, BUFFER_SIZE))) {
        logError(printf("filWrite(%d, \"%s\"): "
                        "At least one character does not fit into a byte.\n",
                        safe_fileno(cOutFile), striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        return;
      } /* if */
      striElems = &striElems[BUFFER_SIZE];
      if (unlikely(BUFFER_SIZE != fwrite(buffer, 1, BUFFER_SIZE, cOutFile))) {
        logError(printf("filWrite: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        (memSizeType) BUFFER_SIZE, safe_fileno(cOutFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* for */
    if (len > 0) {
      if (unlikely(memcpy_from_strelem(buffer, striElems, len))) {
        logError(printf("filWrite(%d, \"%s\"): "
                        "At least one character does not fit into a byte.\n",
                        safe_fileno(cOutFile), striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        return;
      } /* if */
      if (unlikely(len != fwrite(buffer, 1, len, cOutFile))) {
        logError(printf("filWrite: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        len, safe_fileno(cOutFile), errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* if */
    logFunction(printf("filWrite -->\n"););
  } /* filWrite */
