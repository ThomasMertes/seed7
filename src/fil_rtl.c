/********************************************************************/
/*                                                                  */
/*  fil_rtl.c     Primitive actions for the C library file type.    */
/*  Copyright (C) 1989 - 2017  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2009, 2013 - 2015  Thomas Mertes     */
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
#ifdef ISATTY_INCLUDE_IO_H
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
#include "fil_rtl.h"


#ifdef C_PLUS_PLUS
#define C "C"
#else
#define C
#endif

#ifdef DEFINE_FSEEKI64_PROTOTYPE
extern C int __cdecl _fseeki64(FILE *, __int64, int);
#endif

#ifdef DEFINE_FTELLI64_PROTOTYPE
extern C __int64 __cdecl _ftelli64(FILE *);
#endif

#ifdef DEFINE_WPOPEN
DEFINE_WPOPEN
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
 *   "w"        | "rw"   | Truncate to zero length or create file for writing.
 *   "a"        | "ra"   | Append; open or create file for writing at end-of-file.
 *   "r+"       | "rb+"  | Open file for update (reading and writing).
 *   "w+"       | "rw+"  | Truncate to zero length or create file for update.
 *   "a+"       | "ra+"  | Append; open or create file for update, writing at end-of-file.
 *   "rt"       | "r"    | Open file for reading.
 *   "wt"       | "w"    | Truncate to zero length or create file for writing.
 *   "at"       | "a"    | Append; open or create file for writing at end-of-file.
 *   "rt+"      | "r+"   | Open file for update (reading and writing).
 *   "wt+"      | "w+"   | Truncate to zero length or create file for update.
 *   "at+"      | "q+"   | Append; open or create file for update, writing at end-of-file.
 *  Other Seed7 modes correspond to the C mode "".
 *  The Seed7 modes with t are text modes and the modes
 *  without t are binary modes.
 *  If there is a mode character to set the O_CLOEXEC flag (OS_FOPEN_MODE_CLOSE_ON_EXEC),
 *  it is appended to os_mode.
 */
static void get_mode (os_charType os_mode[MAX_MODE_LEN], const const_striType file_mode)

  {
    int mode_pos = 0;

  /* get_mode */
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
        os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
        os_mode[mode_pos++] = 'b';
      } else if (file_mode->size == 2) {
        if (file_mode->mem[1] == '+') {
          /* Binary mode
             r+ ... Open file for update (reading and writing).
             w+ ... Truncate to zero length or create file for update.
             a+ ... Append; open or create file for update, writing at end-of-file.
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
          os_mode[mode_pos++] = 'b';
          os_mode[mode_pos++] = '+';
         } else if (file_mode->mem[1] == 't') {
          /* Text mode
             rt ... Open file for reading.
             wt ... Truncate to zero length or create file for writing.
             at ... Append; open or create file for writing at end-of-file.
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
        } /* if */
      } else if (file_mode->size == 3) {
        if (file_mode->mem[1] == 't' &&
            file_mode->mem[2] == '+') {
          /* Text mode
             rt+ ... Open file for update (reading and writing).
             wt+ ... Truncate to zero length or create file for update.
             at+ ... Append; open or create file for update, writing at end-of-file.
          */
          os_mode[mode_pos++] = (os_charType) file_mode->mem[0];
          os_mode[mode_pos++] = '+';
        } /* if */
      } /* if */
    } /* if */
#if FOPEN_SUPPORTS_CLOEXEC_MODE
    os_mode[mode_pos++] = 'e';
#endif
    os_mode[mode_pos++] = '\0';
  } /* get_mode */



/**
 *  Determine the length of a file by using a seek function.
 *  The file length is measured in bytes.
 *  The file position is moved to the end of the file and the
 *  end position is used as file length. Afterwards the file
 *  position is moved back to the previous position.
 *  This function returns an os_off_t result. The size of
 *  os_off_t might be different from the size of intType.
 *  @return the length of the file or
 *          -1 when the length could not be obtained.
 */
static os_off_t seekFileLength (fileType aFile)

  {
    os_off_t file_length;

  /* seekFileLength */
    logFunction(printf("seekFileLength(%d)\n", safe_fileno(aFile)););
#if FTELL_SUCCEEDS_FOR_PIPE
    {
      int file_no;
      os_fstat_struct stat_buf;

      file_no = fileno(aFile);
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

      file_no = fileno(aFile);
      if (file_no == -1 || isatty(file_no)) {
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

      file_no = fileno(aFile);
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
 *  @return the current file position or
 *          -1 when the file position could not be obtained.
 */
static os_off_t offsetTell (fileType aFile)

  {
    os_off_t current_file_position;

  /* offsetTell */
    logFunction(printf("offsetTell(%d)\n", safe_fileno(aFile)););
#if FTELL_SUCCEEDS_FOR_PIPE
    {
      int file_no;
      os_fstat_struct stat_buf;

      file_no = fileno(aFile);
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

      file_no = fileno(aFile);
      if (file_no == -1 || isatty(file_no)) {
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

      file_no = fileno(aFile);
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
 *  @return 0 upon successful completion or
 *          -1 when the file position could not be set.
 */
int offsetSeek (fileType aFile, const os_off_t anOffset, const int origin)

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

      file_no = fileno(aFile);
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

      file_no = fileno(aFile);
      if (file_no == -1 || isatty(file_no)) {
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

      file_no = fileno(aFile);
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
 *  Returns 0 when the current position or the file size cannot be
 *  determined or when the current position is beyond the filesize.
 *  Returns MAX_MEMSIZETYPE when the result does not fit into
 *  memSizeType.
 */
memSizeType remainingBytesInFile (fileType aFile)

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
      file_no = fileno(aFile);
      if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
          S_ISREG(stat_buf.st_mode)) {
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



intType getFileLengthUsingSeek (fileType aFile)

  {
    os_off_t file_length;
    intType result;

  /* getFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (unlikely(file_length < (os_off_t) 0)) {
      logError(printf("getFileLengthUsingSeek: seekFileLength(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
      result = 0;
    } else if (unlikely(file_length > INTTYPE_MAX)) {
      logError(printf("getFileLengthUsingSeek(%d): "
                      "File length does not fit into an integer: " FMT_D_OFF ".\n",
                      safe_fileno(aFile), file_length););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (intType) file_length;
    } /* if */
    return result;
  } /* getFileLengthUsingSeek */



bigIntType getBigFileLengthUsingSeek (fileType aFile)

  {
    os_off_t file_length;
    bigIntType result;

  /* getBigFileLengthUsingSeek */
    file_length = seekFileLength(aFile);
    if (unlikely(file_length < (os_off_t) 0)) {
      logError(printf("getBigFileLengthUsingSeek: seekFileLength(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
#if OS_OFF_T_SIZE == 32
      result = bigFromUInt32((uint32Type) file_length);
#elif OS_OFF_T_SIZE == 64
      result = bigFromUInt64((uint64Type) file_length);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
    } /* if */
    return result;
  } /* getBigFileLengthUsingSeek */



/**
 *  Read a string, when we do not know how many bytes are avaliable.
 *  This function reads data into a list of buffers. This is done
 *  until enough characters are read or EOF has been reached.
 *  Afterwards the string is allocated, the data is copied from the
 *  buffers and the list of buffers is freed.
 */
static striType read_and_alloc_stri (fileType inFile, memSizeType chars_missing,
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
      /* printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n", bytes_in_buffer); */
      if (unlikely(bytes_in_buffer == 0 && result_size == 0 && ferror(inFile))) {
        logError(printf("read_and_alloc_stri: "
                        "fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        (memSizeType) LIST_BUFFER_SIZE, safe_fileno(inFile),
                        errno, strerror(errno)););
        *err_info = FILE_ERROR;
        result = NULL;
      } else {
        result_size += bytes_in_buffer;
        if (chars_missing > result_size && bytes_in_buffer == LIST_BUFFER_SIZE) {
          currBuffer->next = (bufferList) malloc(sizeof(struct bufferStruct));
          if (unlikely(currBuffer->next == NULL)) {
            logError(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *): "
                            "malloc(" FMT_U_MEM ") failed.\n",
                            safe_fileno(inFile), chars_missing, sizeof(struct bufferStruct)););
            *err_info = MEMORY_ERROR;
            result = NULL;
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
      /* printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n", bytes_in_buffer); */
      if (unlikely(bytes_in_buffer == 0 && result_size == 0 && ferror(inFile))) {
        logError(printf("read_and_alloc_stri: "
                        "fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        chars_missing - result_size, safe_fileno(inFile),
                        errno, strerror(errno)););
        *err_info = FILE_ERROR;
        result = NULL;
      } else {
        result_size += bytes_in_buffer;
      } /* if */
    } /* if */
    if (likely(*err_info == OKAY_NO_ERROR)) {
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
int readCharChkCtrlC (fileType inFile, boolType *sigintReceived)

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



static int readCharFromTerminal (fileType inFile, boolType *sigintReceived)

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



static charType doGetcFromTerminal (fileType inFile)

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
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doGetsFromTerminal */
    if (unlikely(length < 0)) {
      logError(printf("doGetsFromTerminal(%d, " FMT_D "): Negative length.\n",
                      safe_fileno(inFile), length););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      ch = readCharFromTerminal(inFile, &sigintReceived);
      if (unlikely(sigintReceived)) {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } else {
        if (ch != EOF) {
          ungetc(ch, inFile);
        } /* if */
        result = filGets(inFile, length);
      } /* if */
    } /* if */
    return result;
  } /* doGetsFromTerminal */



static striType doLineReadFromTerminal (fileType inFile, charType *terminationChar)

  {
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doLineReadFromTerminal */
    logFunction(printf("doLineReadFromTerminal(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    ch = readCharFromTerminal(inFile, &sigintReceived);
    if (unlikely(sigintReceived)) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } else {
      if (ch != EOF) {
        ungetc(ch, inFile);
      } /* if */
      result = filLineRead(inFile, terminationChar);
    } /* if */
    return result;
  } /* doLineReadFromTerminal */



static striType doWordReadFromTerminal (fileType inFile, charType *terminationChar)

  {
    boolType sigintReceived = FALSE;
    int ch;
    striType result;

  /* doWordReadFromTerminal */
    logFunction(printf("doWordReadFromTerminal(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    ch = readCharFromTerminal(inFile, &sigintReceived);
    if (unlikely(sigintReceived)) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } else {
      if (ch != EOF) {
        ungetc(ch, inFile);
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
    int file_no;
    os_fstat_struct stat_buf;
    bigIntType result;

  /* filBigLng */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
        S_ISREG(stat_buf.st_mode)) {
      if (unlikely(stat_buf.st_size < 0)) {
        logError(printf("filBigLng(%d, *): "
                        "fstat() returns file size less than zero: " FMT_D64 ".\n",
                        safe_fileno(aFile), (int64Type) stat_buf.st_size););
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
#if OS_OFF_T_SIZE == 32
        result = bigFromUInt32((uint32Type) stat_buf.st_size);
#elif OS_OFF_T_SIZE == 64
        result = bigFromUInt64((uint64Type) stat_buf.st_size);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
      } /* if */
    } else {
      result = getBigFileLengthUsingSeek(aFile);
    } /* if */
    return result;
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
    os_off_t file_position;

  /* filBigSeek */
#if OS_OFF_T_SIZE == 32
    file_position = (os_off_t) bigToInt32(position);
#elif OS_OFF_T_SIZE == 64
    file_position = (os_off_t) bigToInt64(position);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
    if (unlikely(file_position <= 0)) {
      logError(printf("filBigSeek(%d, " FMT_D_OFF "): Position <= 0.\n",
                      safe_fileno(aFile), file_position););
      raise_error(RANGE_ERROR);
    } else if (unlikely(offsetSeek(aFile, file_position - 1, SEEK_SET) != 0)) {
      logError(printf("filBigSeek(%d, %s): "
                      "offsetSeek(%d, " FMT_D64 ", SEEK_SET) failed.\n",
                      safe_fileno(aFile), bigHexCStri(position),
                      safe_fileno(aFile), (int64Type) (file_position - 1)););
      raise_error(FILE_ERROR);
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
    os_off_t current_file_position;
    bigIntType result;

  /* filBigTell */
    current_file_position = offsetTell(aFile);
    if (unlikely(current_file_position < (os_off_t) 0)) {
      logError(printf("filBigTell(%d): offsetTell(%d) "
                      "returns negative offset: " FMT_D64 ".\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      (int64Type) current_file_position););
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
#if OS_OFF_T_SIZE == 32
      result = bigFromUInt32((uint32Type) current_file_position + 1);
#elif OS_OFF_T_SIZE == 64
      result = bigFromUInt64((uint64Type) current_file_position + 1);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
    } /* if */
    return result;
  } /* filBigTell */



/**
 *  Close a clib_file.
 *  @exception FILE_ERROR A system function returns an error.
 */
void filClose (fileType aFile)

  { /* filClose */
    if (unlikely(fclose(aFile) != 0)) {
      logError(printf("filClose: fclose(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
  } /* filClose */



#ifdef OUT_OF_ORDER
intType filFileType (fileType aFile)

  {
    int file_no;
    os_fstat_struct stat_buf;
    intType result;

  /* filFileType */
    file_no = fileno(aFile);
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



/**
 *  Read a character from a clib_file.
 *  @return the character read, or EOF at the end of the file.
 */
charType filGetcChkCtrlC (fileType inFile)

  {
    int file_no;
    charType result;

  /* filGetcChkCtrlC */
    file_no = fileno(inFile);
    if (file_no != -1 && isatty(file_no)) {
      result = doGetcFromTerminal(inFile);
    } else {
      result = (charType) getc(inFile);
    } /* if */
    return result;
  } /* filGetcChkCtrlC */



/**
 *  Read a string with 'length' characters from 'inFile'.
 *  In order to work reasonable good for the common case (reading
 *  just some characters), memory for 'length' characters is requested
 *  with malloc(). After the data is read the result string is
 *  shrinked to the actual size (with realloc()). When 'length' is
 *  larger than GETS_DEFAULT_SIZE or the memory cannot be requested
 *  a different strategy is used. In this case the function tries to
 *  find out the number of available characters (this is possible
 *  for a regular file but not for a pipe). If this fails a third
 *  strategy is used. In this case a smaller block is requested. This
 *  block is filled with data, resized and filled in a loop.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType filGets (fileType inFile, intType length)

  {
    memSizeType chars_requested;
    memSizeType bytes_there;
    memSizeType allocated_size;
    memSizeType num_of_chars_read;
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* filGets */
    logFunction(printf("filGets(%d, " FMT_D ")\n", safe_fileno(inFile), length););
    if (unlikely(length <= 0)) {
      if (unlikely(length != 0)) {
        logError(printf("filGets(%d, " FMT_D "): Negative length.\n",
                        safe_fileno(inFile), length););
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
        bytes_there = remainingBytesInFile(inFile);
        /* printf("bytes_there=" FMT_U_MEM "\n", bytes_there); */
        if (bytes_there != 0) {
          /* Now we know that bytes_there bytes are available in inFile */
          if (chars_requested <= bytes_there) {
            allocated_size = chars_requested;
          } else {
            allocated_size = bytes_there;
          } /* if */
          /* printf("allocated_size=" FMT_U_MEM "\n", allocated_size); */
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, allocated_size))) {
            /* printf("MAX_STRI_LEN=%lu, SIZ_STRI(MAX_STRI_LEN)=%lu\n",
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
        /* printf("read_size=%ld\n", allocated_size); */
        num_of_chars_read = (memSizeType) fread(result->mem, 1,
            (size_t) allocated_size, inFile);
        /* printf("num_of_chars_read=" FMT_U_MEM "\n", num_of_chars_read); */
        if (unlikely(num_of_chars_read == 0 && ferror(inFile))) {
          logError(printf("filGets: fread(*, 1, " FMT_U_MEM ", %d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          allocated_size, safe_fileno(inFile),
                          errno, strerror(errno)););
          err_info = FILE_ERROR;
        } else {
          memcpy_to_strelem(result->mem, (ustriType) result->mem, num_of_chars_read);
          if (num_of_chars_read < result->size) {
            REALLOC_STRI_SIZE_SMALLER(resized_result, result, result->size, num_of_chars_read);
            if (unlikely(resized_result == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              result = resized_result;
              COUNT3_STRI(result->size, num_of_chars_read);
              result->size = num_of_chars_read;
            } /* if */
          } /* if */
        } /* if */
      } else {
        /* Read a string, when we do not know how many bytes are avaliable. */
        result = read_and_alloc_stri(inFile, chars_requested, &err_info);
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        if (result != NULL) {
          FREE_STRI(result, result->size);
        } /* if */
        raise_error(err_info);
        result = NULL;
      } /* if */
    } /* if */
    logFunction(printf("filGets(%d, " FMT_D ") --> \"%s\"\n",
                       safe_fileno(inFile), length, striAsUnquotedCStri(result)););
    return result;
  } /* filGets */



striType filGetsChkCtrlC (fileType inFile, intType length)

  {
    int file_no;
    striType result;

  /* filGetsChkCtrlC */
    logFunction(printf("filGetsChkCtrlC(%d, " FMT_D ")\n",
                       safe_fileno(inFile), length););
    file_no = fileno(inFile);
    if (file_no != -1 && isatty(file_no)) {
      result = doGetsFromTerminal(inFile, length);
    } else {
      result = filGets(inFile, length);
    } /* if */
    logFunction(printf("filGetsChkCtrlC(%d, " FMT_D ") --> \"%s\"\n",
                       safe_fileno(inFile), length, striAsUnquotedCStri(result)););
    return result;
  } /* filGetsChkCtrlC */



/**
 *  Determine if at least one character can be read successfully.
 *  This function allows a file to be handled like an iterator.
 *  @return FALSE if 'getc' would return EOF, TRUE otherwise.
 */
boolType filHasNext (fileType inFile)

  {
    int next_char;
    boolType result;

  /* filHasNext */
    logFunction(printf("filHasNext(%d)\n", safe_fileno(inFile)););
    if (feof(inFile)) {
      result = FALSE;
    } else {
      next_char = getc(inFile);
      if (next_char != EOF) {
        if (unlikely(ungetc(next_char, inFile) != next_char)) {
          logError(printf("filHasNext: ungetc('\\" FMT_U32 ";', %d) "
                          "does not return '\\" FMT_U32 ";'.\n",
                          next_char, safe_fileno(inFile), next_char););
          raise_error(FILE_ERROR);
          result = FALSE;
        } else {
          result = TRUE;
        } /* if */
      } else {
        clearerr(inFile);
        result = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filHasNext(%d) --> %d\n",
                       safe_fileno(inFile), result););
    return result;
  } /* filHasNext */



boolType filHasNextChkCtrlC (fileType inFile)

  {
    int file_no;
    int next_char;
    boolType result;

  /* filHasNextChkCtrlC */
    logFunction(printf("filHasNextChkCtrlC(%d)\n",
                       safe_fileno(inFile)););
    if (feof(inFile)) {
      result = FALSE;
    } else {
      file_no = fileno(inFile);
      if (file_no != -1 && isatty(file_no)) {
        next_char = (int) (scharType) doGetcFromTerminal(inFile);
      } else {
        next_char = getc(inFile);
      } /* if */
      if (next_char != EOF) {
        if (unlikely(ungetc(next_char, inFile) != next_char)) {
          logError(printf("filHasNextChkCtrlC: "
                          "ungetc('\\" FMT_U32 ";', %d) "
                          "does not return '\\" FMT_U32 ";'.\n",
                          next_char, safe_fileno(inFile), next_char););
          raise_error(FILE_ERROR);
          result = FALSE;
        } else {
          result = TRUE;
        } /* if */
      } else {
        clearerr(inFile);
        result = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filHasNextChkCtrlC(%d) --> %d\n",
                       safe_fileno(inFile), result););
    return result;
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
    register int ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* filLineRead */
    logFunction(printf("filLineRead(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      flockfile(inFile);
      while ((ch = getc_unlocked(inFile)) != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, memlength);
            funlockfile(inFile);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strElemType) ch;
      } /* while */
      funlockfile(inFile);
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (unlikely(ch == EOF && position == 0 && ferror(inFile))) {
        FREE_STRI(result, memlength);
        logError(printf("filLineRead(%d, '\\" FMT_U32 ";'): "
                        "getc_unlocked(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(inFile), *terminationChar,
                        safe_fileno(inFile), errno, strerror(errno)););
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        REALLOC_STRI_SIZE_SMALLER(resized_result, result, memlength, position);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, memlength);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(memlength, position);
          result->size = position;
          *terminationChar = (charType) ch;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filLineRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(inFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filLineRead */



striType filLineReadChkCtrlC (fileType inFile, charType *terminationChar)

  {
    int file_no;
    striType result;

  /* filLineReadChkCtrlC */
    logFunction(printf("filLineReadChkCtrlC(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    file_no = fileno(inFile);
    if (file_no != -1 && isatty(file_no)) {
      result = doLineReadFromTerminal(inFile, terminationChar);
    } else {
      result = filLineRead(inFile, terminationChar);
    } /* if */
    logFunction(printf("filLineReadChkCtrlC(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(inFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filLineReadChkCtrlC */



striType filLit (fileType aFile)

  {
    const_cstriType file_name;
    striType result;

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
    int file_no;
    os_fstat_struct stat_buf;
    intType result;

  /* filLng */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
        S_ISREG(stat_buf.st_mode)) {
      if (unlikely(stat_buf.st_size < 0)) {
        logError(printf("filLng(%d): "
                        "fstat() returns File size less than zero: " FMT_D64 ".\n",
                        safe_fileno(aFile), (int64Type) stat_buf.st_size););
        raise_error(FILE_ERROR);
        result = 0;
      } else if (unlikely(stat_buf.st_size > INTTYPE_MAX)) {
        logError(printf("filLng(%d): "
                        "File length does not fit into an integer: " FMT_D_OFF ".\n",
                        safe_fileno(aFile), stat_buf.st_size););
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (intType) stat_buf.st_size;
      } /* if */
    } else {
      result = getFileLengthUsingSeek(aFile);
    } /* if */
    return result;
  } /* filLng */



/**
 *  Opens a file with the specified 'path' and 'mode'.
 *  There are text modes and binary modes:
 *  - Binary modes:
 *   - "r"   Open file for reading.
 *   - "w"   Truncate to zero length or create file for writing.
 *   - "a"   Append; open or create file for writing at end-of-file.
 *   - "r+"  Open file for update (reading and writing).
 *   - "w+"  Truncate to zero length or create file for update.
 *   - "a+"  Append; open or create file for update, writing at end-of-file.
 *  - Text modes:
 *   - "rt"  Open file for reading.
 *   - "wt"  Truncate to zero length or create file for writing.
 *   - "at"  Append; open or create file for writing at end-of-file.
 *   - "rt+" Open file for update (reading and writing).
 *   - "wt+" Truncate to zero length or create file for update.
 *   - "at+" Append; open or create file for update, writing at end-of-file.
 *
 *  Note that this modes differ from the ones used by the C function
 *  fopen(). Unicode characters in 'path' are converted to the
 *  representation used by the fopen() function of the operating
 *  system.
 *  @param path Path of the file to be opened. The path must
 *         use the standard path representation.
 *  @param mode Mode of the file to be opened.
 *  @return the file opened, or NULL if it could not be opened or
 *          when 'path' refers to a directory.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system path type.
 *  @exception RANGE_ERROR The 'mode' is not one of the allowed
 *             values or 'path' does not use the standard path
 *             representation or 'path' cannot be converted
 *             to the system path type.
 */
fileType filOpen (const const_striType path, const const_striType mode)

  {
    os_striType os_path;
    os_charType os_mode[MAX_MODE_LEN];
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
#if FOPEN_OPENS_DIRECTORIES
    int file_no;
    os_fstat_struct stat_buf;
#endif
    fileType result;

  /* filOpen */
    logFunction(printf("filOpen(\"%s\", ", striAsUnquotedCStri(path));
                printf("\"%s\")\n", striAsUnquotedCStri(mode)););
    get_mode(os_mode, mode);
    if (unlikely(os_mode[0] == '\0')) {
      logError(printf("filOpen: Illegal mode: \"%s\".\n",
                      striAsUnquotedCStri(mode)););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      os_path = cp_to_os_path(path, &path_info, &err_info);
      /* printf("os_path \"%ls\" %d %d\n", os_path, path_info, err_info); */
      if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
        if (unlikely(path_info == PATH_IS_NORMAL))
#endif
        {
          logError(printf("filOpen: cp_to_os_path(\"%s\", *, *) failed:\n"
                          "path_info=%d, err_info=%d\n",
                          striAsUnquotedCStri(path), path_info, err_info););
          raise_error(err_info);
        }
        result = NULL;
      } else {
        logMessage(printf("os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                          os_path, os_mode););
        result = os_fopen(os_path, os_mode);
        if (unlikely(result == NULL)) {
          logError(printf("filOpen: "
                          "fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, os_mode, errno, strerror(errno)););
        } else {
#if !FOPEN_SUPPORTS_CLOEXEC_MODE && HAS_FCNTL_SETFD_CLOEXEC
          file_no = fileno(result);
          if (file_no != -1) {
            fcntl(file_no, F_SETFD, fcntl(file_no, F_GETFD) | FD_CLOEXEC);
          } /* if */
#endif
#if FOPEN_OPENS_DIRECTORIES
          file_no = fileno(result);
          if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0 &&
              S_ISDIR(stat_buf.st_mode)) {
            /* An attempt to open a directory with filOpen()     */
            /* returns NULL even when fopen() succeeds. On many  */
            /* modern operating systems functions like fgetc()   */
            /* and readf() fail to read from a directory anyway. */
            /* So it is better to fail early, when the file is   */
            /* opened, instead of later at an unexpected place.  */
            /* Even when reading a directory as file succeeds    */
            /* there is another issue: Reading a directory as    */
            /* file is not portable, since it delivers an        */
            /* operating system specific representation of the   */
            /* directory. So reading a directory as file should  */
            /* be avoided altogether. The functions dirOpen(),   */
            /* dirRead() and dirClose() provide a portable way   */
            /* to open, read and close a directory.              */
            fclose(result);
            logError(printf("filOpen: "
                            "fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") "
                            "opened a directory. Close it and return NULL.\n",
                            os_path, os_mode););
            result = NULL;
          } /* if */
#endif
        } /* if */
        os_stri_free(os_path);
      } /* if */
    } /* if */
    logFunction(printf("filOpen(\"%s\", ", striAsUnquotedCStri(path));
                printf("\"%s\") --> %d\n",
                       striAsUnquotedCStri(mode), safe_fileno(result)););
    return result;
  } /* filOpen */



/**
 *  Open the null device of the operation system for reading and writing.
 *  @return the null device opened, or NULL if it could not be opened.
 */
fileType filOpenNullDevice (void)

  {
    fileType fileOpened;

  /* filOpenNullDevice */
    logFunction(printf("filOpenNullDevice()\n"););
    fileOpened = fopen(NULL_DEVICE, "r+");
    logFunction(printf("filOpenNullDevice() --> %d\n",
                       safe_fileno(fileOpened)););
    return fileOpened;
  } /* filOpenNullDevice */



/**
 *  Wait for the process associated with 'aPipe' to terminate.
 *  @param aPipe Pipe file to be closed (created by 'filPopen').
 *  @exception FILE_ERROR A system function returned an error.
 */
void filPclose (fileType aPipe)

  { /* filPclose */
#if HAS_POPEN
    if (unlikely(os_pclose(aPipe) == -1)) {
      logError(printf("filPclose: pclose(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aPipe), errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
#endif
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
 *         the 'command', or "" when there are no parameters.
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
    os_striType os_command;
    os_charType os_mode[MAX_MODE_LEN];
    int mode_pos = 0;
    errInfoType err_info = OKAY_NO_ERROR;
    fileType result;

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
      result = NULL;
    } else {
      if (mode->size == 1 &&
          (mode->mem[0] == 'r' ||
           mode->mem[0] == 'w')) {
        os_mode[mode_pos++] = (os_charType) mode->mem[0];
#if POPEN_SUPPORTS_BINARY_MODE
        os_mode[mode_pos++] = 'b';
#endif
      } else if (mode->size == 2 &&
          (mode->mem[0] == 'r' ||
           mode->mem[0] == 'w') &&
           mode->mem[1] == 't') {
        os_mode[mode_pos++] = (os_charType) mode->mem[0];
#if POPEN_SUPPORTS_TEXT_MODE
        os_mode[mode_pos++] = 't';
#endif
      } /* if */
#if POPEN_SUPPORTS_CLOEXEC_MODE
      os_mode[mode_pos++] = 'e';
#endif
      os_mode[mode_pos++] = '\0';
      if (unlikely(os_mode[0] == '\0')) {
        logError(printf("filPopen: Illegal mode: \"%s\".\n",
                        striAsUnquotedCStri(mode)););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
        adjustCwdForShell(&err_info);
#endif
        logMessage(printf("os_popen(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                          os_command, os_mode););
        result = os_popen(os_command, os_mode);
      } /* if */
      FREE_OS_STRI(os_command);
    } /* if */
#else
    result = NULL;
#endif
    logFunction(printf("filPopen(\"%s\", ", striAsUnquotedCStri(command));
                printf("\"%s\", ", striAsUnquotedCStri(parameters));
                printf("\"%s\") --> %d\n",
                       striAsUnquotedCStri(mode), safe_fileno(result)););
    return result;
  } /* filPopen */



void filPrint (const const_striType stri)

  { /* filPrint */
    ut8Write(stdout, stri);
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

  { /* filSeek */
    logFunction(printf("filSeek(%d, " FMT_D ")\n", safe_fileno(aFile), position););
    if (unlikely(position <= 0)) {
      logError(printf("filSeek(%d, " FMT_D "): Position <= 0.\n",
                      safe_fileno(aFile), position););
      raise_error(RANGE_ERROR);
#if OS_OFF_T_SIZE < INTTYPE_SIZE
#if OS_OFF_T_SIZE == 32
    } else if (unlikely(position > INT32TYPE_MAX)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(aFile), position););
      raise_error(RANGE_ERROR);
#elif OS_OFF_T_SIZE == 64
    } else if (unlikely(position > INT64TYPE_MAX)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(aFile), position););
      raise_error(RANGE_ERROR);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
#endif
    } else if (unlikely(offsetSeek(aFile, (os_off_t) (position - 1), SEEK_SET) != 0)) {
      logError(printf("filSeek(%d, " FMT_D "): "
                      "offsetSeek(%d, " FMT_D ", SEEK_SET) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), position,
                      safe_fileno(aFile), position - 1,
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
  } /* filSeek */



void filSetbuf (fileType aFile, intType mode, intType size)

  { /* filSetbuf */
    if (unlikely(mode < 0 || mode > 2 || size < 0 || (uintType) size > MAX_MEMSIZETYPE)) {
      logError(printf("filSetbuf(%d, " FMT_D ", " FMT_D "): "
                      "Mode or size not in allowed range.\n",
                      safe_fileno(aFile), mode, size););
      raise_error(RANGE_ERROR);
    } else if (unlikely(setvbuf(aFile, NULL, (int) mode, (memSizeType) size) != 0)) {
      logError(printf("filSetbuf: "
                      "setvbuf(%d, NULL, %d, " FMT_U_MEM ") failed.:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), (int) mode, (memSizeType) size,
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
    os_off_t current_file_position;
    intType result;

  /* filTell */
    current_file_position = offsetTell(aFile);
    if (unlikely(current_file_position < (os_off_t) 0)) {
      logError(printf("filTell(%d): "
                      "offsetTell(%d) returns negative offset: " FMT_D64 ".\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      (int64Type) current_file_position););
      raise_error(FILE_ERROR);
      result = 0;
    } else if (unlikely(current_file_position >= INTTYPE_MAX)) {
      logError(printf("filTell(%d): "
                      "File position does not fit into an integer: " FMT_D_OFF ".\n",
                      safe_fileno(aFile), current_file_position););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (intType) (current_file_position + 1);
    } /* if */
    return result;
  } /* filTell */



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
    register int ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* filWordRead */
    logFunction(printf("filWordRead(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      flockfile(inFile);
      do {
        ch = getc_unlocked(inFile);
      } while (ch == ' ' || ch == '\t');
      while (ch != ' ' && ch != '\t' &&
          ch != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, memlength);
            funlockfile(inFile);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strElemType) ch;
        ch = getc_unlocked(inFile);
      } /* while */
      funlockfile(inFile);
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (unlikely(ch == EOF && position == 0 && ferror(inFile))) {
        FREE_STRI(result, memlength);
        logError(printf("filWordRead(%d, '\\" FMT_U32 ";'): "
                        "getc_unlocked(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(inFile), *terminationChar,
                        safe_fileno(inFile), errno, strerror(errno)););
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        REALLOC_STRI_SIZE_SMALLER(resized_result, result, memlength, position);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, memlength);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(memlength, position);
          result->size = position;
          *terminationChar = (charType) ch;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filWordRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(inFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* filWordRead */



striType filWordReadChkCtrlC (fileType inFile, charType *terminationChar)

  {
    int file_no;
    striType result;

  /* filWordReadChkCtrlC */
    logFunction(printf("filWordReadChkCtrlC(%d, '\\" FMT_U32 ";')\n",
                       safe_fileno(inFile), *terminationChar););
    file_no = fileno(inFile);
    if (file_no != -1 && isatty(file_no)) {
      result = doWordReadFromTerminal(inFile, terminationChar);
    } else {
      result = filWordRead(inFile, terminationChar);
    } /* if */
    logFunction(printf("filWordReadChkCtrlC(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(inFile), *terminationChar,
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
    const strElemType *striElems;
    memSizeType len;
    ucharType buffer[BUFFER_SIZE];

  /* filWrite */
    logFunction(printf("filWrite(%d, \"%s\")\n",
                       safe_fileno(outFile), striAsUnquotedCStri(stri)););
#if FWRITE_WRONG_FOR_READ_ONLY_FILES
    if (unlikely(stri->size > 0 && (outFile->flags & _F_WRIT) == 0)) {
      logError(printf("filWrite: Attempt to write to read only file: %d.\n",
                      safe_fileno(outFile)););
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    striElems = stri->mem;
    for (len = stri->size; len >= BUFFER_SIZE; len -= BUFFER_SIZE) {
      if (unlikely(memcpy_from_strelem(buffer, striElems, BUFFER_SIZE))) {
        logError(printf("filWrite(%d, \"%s\"): "
                        "At least one character does not fit into a byte.\n",
                        safe_fileno(outFile), striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        return;
      } /* if */
      striElems = &striElems[BUFFER_SIZE];
      if (unlikely(BUFFER_SIZE != fwrite(buffer, 1, BUFFER_SIZE, outFile))) {
        logError(printf("filWrite: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        (memSizeType) BUFFER_SIZE, safe_fileno(outFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* for */
    if (len > 0) {
      if (unlikely(memcpy_from_strelem(buffer, striElems, len))) {
        logError(printf("filWrite(%d, \"%s\"): "
                        "At least one character does not fit into a byte.\n",
                        safe_fileno(outFile), striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        return;
      } /* if */
      if (unlikely(len != fwrite(buffer, 1, len, outFile))) {
        logError(printf("filWrite: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        len, safe_fileno(outFile), errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* if */
    logFunction(printf("filWrite -->\n"););
  } /* filWrite */
