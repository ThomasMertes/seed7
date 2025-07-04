/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015, 2019, 2021, 2025  Thomas Mertes      */
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
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/error.c                                         */
/*  Changes: 1990 - 1994, 2014, 2015, 2019, 2021  Thomas Mertes     */
/*  Content: Submit normal compile time error messages.             */
/*                                                                  */
/*  Normal compile time error messages do not terminate the         */
/*  program.                                                        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "datautl.h"
#include "traceutl.h"
#include "infile.h"
#include "info.h"
#include "symbol.h"
#include "msg_stri.h"
#include "stat.h"
#include "str_rtl.h"
#include "chr_rtl.h"
#include "fil_rtl.h"
#include "ut8_rtl.h"
#include "con_drv.h"
#include "fatal.h"

#undef EXTERN
#define EXTERN
#include "error.h"


#ifndef SEEK_SET
#define SEEK_SET 0
#endif


#define LINE_SIZE_INCREMENT 256
#define MAX_AREA_SIZE 256
#define BUFFER_SIZE 1024
#define TAB_POSITION 8
#define MAX_DEPTH_SHOWN 3

#define ustri_buf_to_stri(ustri, len) cstri_buf_to_stri((const_cstriType)(ustri), len)




static void freeError (parseErrorType oldError)

  { /* freeError */
    /* Not owned: oldError->fileName */
    strDestr(oldError->msg);
    strDestr(oldError->errorLine);
    FREE_RECORD(oldError, parseErrorRecord, count.parseError);
  } /* freeError */



void freeErrorList (parseErrorType error)

  {
    parseErrorType oldError;

  /* freeErrorList */
    while (error != NULL) {
      oldError = error;
      error = error->next;
      freeError(oldError);
    } /* while */
  } /* freeErrorList */



static void showError (parseErrorType error)

  { /* showError */
    printf("err: %d\n", error->err);
    printf("fileName: %s\n", striAsUnquotedCStri(error->fileName));
    printf("lineNumber: %d\n", error->lineNumber);
    printf("columnNumber: " FMT_U_MEM "\n", error->columnNumber);
    printf("msg: %s\n", striAsUnquotedCStri(error->msg));
    printf("errorLine: %s\n", striAsUnquotedCStri(error->errorLine));
  } /* showError */



static parseErrorType newError (errorType err)

  {
    parseErrorType error;

  /* newError */
    if (unlikely(!ALLOC_RECORD(error, parseErrorRecord, count.parseError))) {
      logError(printf("newError(): No more memory.\n"););
      fatal_memory_error(SOURCE_POSITION(2121));
    } else {
      memset(error, 0, sizeof(parseErrorRecord));
      error->err = (int) err;
    } /* if */
    return error;
  } /* newError */



static void appendErrorToProg (parseErrorType error)

  { /* appendErrorToProg */
    if (error->fileName == NULL ||
        error->msg == NULL ||
        error->errorLine == NULL) {
      /* showError(error); */
      freeError(error);
      logError(printf("appendErrorToProg(): No more memory.\n"););
      fatal_memory_error(SOURCE_POSITION(2122));
    } else {
      if (prog != NULL) {
        prog->error_count++;
        if (prog->errorList == NULL) {
          prog->errorList = error;
          prog->errorListEnd = error;
        } else {
          prog->errorListEnd->next = error;
          prog->errorListEnd = error;
        } /* if */
      } /* if */
    } /* if */
  } /* appendErrorToProg */



static striType toOutputString (const const_striType stri)

  {
    memSizeType pos;
    charType ch;
    memSizeType width;
    memSizeType output_length = 0;
    char escapeBuffer[ESC_SEQUENCE_MAX_LEN + NULL_TERMINATION_LEN];
    char tabBuffer[TAB_POSITION + NULL_TERMINATION_LEN];
    striType output;

  /* toOutputString */
    logFunction(printf("toOutputString(\"%s\")\n", striAsUnquotedCStri(stri)););
    copyCStri(&output, "");
    for (pos = 0; pos < stri->size; pos++) {
      ch = stri->mem[pos];
      if ((ch >= 0xd800 && ch <= 0xdfff) || ch > 0x10ffff) {
        /* UTF-16 surrogate character or non Unicode character. */
        output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
        appendCStri(&output, escapeBuffer);
      } else {
        width = (memSizeType) chrWidth(ch);
        if (width >= 1) {
          appendChar(&output, ch);
          output_length += width;
        } else if (ch < ' ') {
          if (ch == '\t') {
            width = TAB_POSITION - output_length % TAB_POSITION;
            memset(tabBuffer, ' ', width);
            tabBuffer[width] = '\0';
            appendCStri(&output, tabBuffer);
            output_length += width;
          } else {
            if (pos > 0 && stri->mem[pos - 1] == '\\') {
              appendChar(&output, '\\');
              output_length++;
            } /* if */
            appendCStri(&output, stri_escape_sequence[ch]);
            output_length += strlen(stri_escape_sequence[ch]);
          } /* if */
        } else {
          output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
          appendCStri(&output, escapeBuffer);
        } /* if */
      } /* if */
    } /* for */
    logFunction(printf("toOutputString(\"%s\")", striAsUnquotedCStri(stri));
                printf(" --> \"%s\"\n", striAsUnquotedCStri(output)););
    return output;
  } /* toOutputString */



static memSizeType calculate_output_length (const const_striType stri)

  {
    memSizeType pos;
    charType ch;
    memSizeType width;
    char escapeBuffer[ESC_SEQUENCE_MAX_LEN + NULL_TERMINATION_LEN];
    memSizeType output_length = 0;

  /* calculate_output_length */
    logFunction(printf("calculate_output_length(\"%s\")\n",
                       striAsUnquotedCStri(stri)););
    for (pos = 0; pos < stri->size; pos++) {
      ch = stri->mem[pos];
      if ((ch >= 0xd800 && ch <= 0xdfff) || ch > 0x10ffff) {
        /* UTF-16 surrogate character or non Unicode character. */
        output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
      } else {
        width = (memSizeType) chrWidth(ch);
        if (width >= 1) {
          output_length += width;
        } else if (ch < ' ') {
          if (ch == '\t') {
            width = TAB_POSITION - output_length % TAB_POSITION;
            output_length += width;
          } else {
            if (pos > 0 && stri->mem[pos - 1] == '\\') {
              output_length++;
            } /* if */
            output_length += strlen(stri_escape_sequence[ch]);
          } /* if */
        } else {
          output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
        } /* if */
      } /* if */
    } /* for */
    logFunction(printf("calculate_output_length(\"%s\") --> " FMT_U_MEM "\n",
                       striAsUnquotedCStri(stri), output_length););
    return output_length;
  } /* calculate_output_length */



/**
 *  Convert 'length' UTF-8 encoded bytes in 'stri8' to a string.
 *  Bytes that are not UTF-8 encoded are left as is.
 *  @return the converted string or NULL, if a memory error occurred.
 */
static striType stri8_buffer_to_stri (const strElemType *const stri8,
    const memSizeType length)

  {
    memSizeType stri_size = 0;
    memSizeType converted_size;
    memSizeType unconverted;
    charType ch;
    striType resized_stri;
    striType stri;

  /* stri8_buffer_to_stri */
    logFunction(printf("stri8_buffer_to_stri(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) stri8, length););
    if (stri8 == NULL) {
      stri = NULL;
    } else if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      unconverted = length;
      do {
        unconverted = stri8_to_stri(&stri->mem[stri_size], &converted_size,
                                    &stri8[length - unconverted], unconverted);
        stri_size += converted_size;
        if (unconverted != 0) {
          ch = stri8[length - unconverted];
          if (ch >= 0xC0) {
            /* ch range 192 to 255 (leading bits 11......) */
            do {
              stri->mem[stri_size] = (strElemType) ch;
              stri_size++;
              unconverted--;
              if (unconverted != 0) {
                ch = stri8[length - unconverted];
              } /* if */
            } while (unconverted != 0 && ch >= 0x80 && ch <= 0xBF);
            /* ch range outside 128 to 191 (leading bits not 10......) */
          } else {
            stri->mem[stri_size] = (strElemType) ch;
            stri_size++;
            unconverted--;
          } /* if */
        } /* if */
      } while (unconverted != 0);
      REALLOC_STRI_SIZE_SMALLER2(resized_stri, stri, length, stri_size);
      if (resized_stri == NULL) {
        FREE_STRI2(stri, length);
        stri = NULL;
      } else {
        stri = resized_stri;
        stri->size = stri_size;
      } /* if */
    } /* if */
    logFunction(printf("stri8_buffer_to_stri(" FMT_U_MEM ", " FMT_U_MEM ") --> \"%s\"\n",
                       (memSizeType) stri8, length, striAsUnquotedCStri(stri)););
    return stri;
  } /* stri8_buffer_to_stri */



static memSizeType computeColumnMarkerPos (const const_striType errorLine,
    memSizeType column)

  {
    memSizeType part1_len;
    striType part1;
    memSizeType columnMarkerPos;

  /* computeColumnMarkerPos */
    logFunction(printf("computeColumnMarkerPos(\"%s\", " FMT_U_MEM ")\n",
                       striAsUnquotedCStri(errorLine), column););
    part1_len = column - 1;
    if (part1_len > errorLine->size) {
      part1_len = errorLine->size;
    } /* if */
    part1 = stri8_buffer_to_stri(errorLine->mem, part1_len);
    if (part1 != NULL) {
      columnMarkerPos = calculate_output_length(part1) + 1;
      FREE_STRI(part1);
    } else {
      columnMarkerPos = part1_len + 1;
    } /* if */
    logFunction(printf("computeColumnMarkerPos(\"%s\", " FMT_U_MEM ") --> "
                       FMT_U_MEM "\n", striAsUnquotedCStri(errorLine),
                       column, columnMarkerPos););
    return columnMarkerPos;
  } /* computeColumnMarkerPos */



static void writeString (fileType errorFile, const const_striType stri)

  { /* writeString */
    if (errorFile->cFile == stdout) {
      conWrite(stri);
    } else {
      ut8Write(errorFile, stri);
    } /* if */
  } /* writeString */



static void writeChar (fileType errorFile, const charType aChar)

  {
    struct striStruct stri_buffer;
    striType stri;

  /* writeChar */
    stri = chrStrMacro(aChar, stri_buffer);
    writeString(errorFile, stri);
  } /* writeChar */



static void writeNewline (fileType errorFile)

  { /* writeNewline */
    writeChar(errorFile, '\n');
  } /* writeNewline */



static void writeError (fileType errorFile, parseErrorType error)

  {
    striType message;
    striType outputLine;
    striType output;
    memSizeType column;

  /* writeError */
    logFunction(printf("writeError()\n"););
    copyCStri(&message, "*** ");
    if (error->fileName != NULL) {
      appendString(&message, error->fileName);
      appendChar(&message, '(');
      appendInt(&message, error->lineNumber);
      appendCStri(&message, "):");
      appendInt(&message, error->err + 1);
      appendCStri(&message, ": ");
    } /* if */
    appendString(&message, error->msg);
    if (unlikely(message == NULL)) {
      logError(printf("writeError(): No more memory. (1)\n"););
      fatal_memory_error(SOURCE_POSITION(2123));
    } else {
      writeString(errorFile, message);
      FREE_STRI(message);
      writeNewline(errorFile);
      if (error->errorLine != NULL) {
        outputLine = stri8_buffer_to_stri(error->errorLine->mem,
                                          error->errorLine->size);
        if (unlikely(outputLine == NULL)) {
          logError(printf("writeError(): No more memory. (2)\n"););
          fatal_memory_error(SOURCE_POSITION(2124));
        } else {
          output = toOutputString(outputLine);
          FREE_STRI(outputLine);
          if (unlikely(output == NULL)) {
            logError(printf("writeError(): No more memory. (3)\n"););
            fatal_memory_error(SOURCE_POSITION(2125));
          } else {
            writeString(errorFile, output);
            FREE_STRI(output);
            writeNewline(errorFile);
            if (error->columnNumber != 0) {
              column = computeColumnMarkerPos(error->errorLine, error->columnNumber);
              for (; column > 1; column--) {
                writeChar(errorFile, '-');
              } /* for */
              writeChar(errorFile, '^');
            } /* if */
            writeNewline(errorFile);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("writeError() -->\n"););
  } /* writeError */



static void finalizeError (parseErrorType error)

  { /* finalizeError */
    logFunction(printf("finalizeError()\n"););
    appendErrorToProg(error);
    if (prog == NULL) {
      writeError(&stderrFileRecord, error);
      freeError(error);
    } else if (prog->writeErrors) {
      writeError(prog->errorFile, error);
    } /* if */
    logFunction(printf("finalizeError() -->\n"););
  } /* finalizeError */



static void storeColumnNumber (parseErrorType error,
    long line_start_position, long current_position)

  { /* storeColumnNumber */
    logFunction(printf("storeColumnNumber(%ld, %ld)\n",
                       line_start_position, current_position););
    if (line_start_position <= current_position) {
      if (line_start_position < current_position) {
        error->columnNumber = (memSizeType) (current_position - line_start_position);
      } else {
        error->columnNumber = 1;
      } /* if */
    } else {
      error->columnNumber = 0;
    } /* if */
    logFunction(printf("storeColumnNumber(%ld, %ld) -->\n",
                       line_start_position, current_position););
  } /* storeColumnNumber */



/**
 *  Read a line from infile.
 *  The reading stops, at the end of the line or when there is no more memory.
 *  @return the line without the terminating newline, or
 *          NULL if there is no more memory.
 */
static striType readLineFromCurrentFile (void)

  {
    memSizeType memlength;
    memSizeType newmemlength;
    strElemType *memory;
    memSizeType position;
    int ch;
    striType resizedLine;
    striType line;

  /* readLineFromCurrentFile */
    logFunction(printf("readLineFromCurrentFile()\n"););
    memlength = LINE_SIZE_INCREMENT;
    if (unlikely(ALLOC_STRI_SIZE_OK(line, memlength))) {
      memory = line->mem;
      position = 0;
      while ((ch = next_character()) != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + LINE_SIZE_INCREMENT;
          REALLOC_STRI_CHECK_SIZE2(resizedLine, line, memlength, newmemlength);
          if (unlikely(resizedLine == NULL)) {
            FREE_STRI2(line, memlength);
            return NULL;
          } /* if */
          line = resizedLine;
          memory = line->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strElemType) ch;
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_SMALLER2(resizedLine, line, memlength, position);
      if (unlikely(resizedLine == NULL)) {
        FREE_STRI2(line, memlength);
        line = NULL;
      } else {
        line = resizedLine;
        line->size = position;
      } /* if */
    } /* if */
    logFunction(printf("readLineFromCurrentFile() --> \"%s\"\n",
                       striAsUnquotedCStri(line)););
    return line;
  } /* readLineFromCurrentFile */



static void storeLineOfCurrentFile (parseErrorType error, lineNumType lineNumber)

  {
    long currentPosition;
    long bufferStartPosition;
    unsigned int tableSize;
    long *nlTable;
    unsigned int tableStart;
    unsigned int tablePos;
    boolType searching;
    int areaSize;
    int areaPos;
    int ch;

  /* storeLineOfCurrentFile */
    logFunction(printf("storeLineOfCurrentFile(%u) "
                       "in_file.line=%u in_file.name=\"%s\"\n",
                       lineNumber, in_file.line, in_file.name_ustri););
    if (in_file.curr_infile != NULL &&
        (currentPosition = IN_FILE_TELL()) >= 0L) {
      /* printf("currentPosition=%lu in_file.character=%d\n",
         currentPosition, in_file.character); */
      if (in_file.line < lineNumber) {
        do {
          SKIP_TO_NL(ch);
          lineNumber--;
        } while (in_file.line < lineNumber);
        error->errorLine = readLineFromCurrentFile();
      } else {
        tableSize = in_file.line - lineNumber + 1;
        if (ALLOC_TABLE(nlTable, long, tableSize)) {
          if (in_file.character == EOF) {
            bufferStartPosition = currentPosition;
          } else {
            bufferStartPosition = currentPosition - 1;
          } /* if */
          tablePos = 0;
          searching = TRUE;
          do {
            if (bufferStartPosition >= MAX_AREA_SIZE) {
              areaSize = MAX_AREA_SIZE;
            } else {
              areaSize = (int) bufferStartPosition;
            } /* if */
            bufferStartPosition -= areaSize;
            /* printf("bufferStartPosition=%ld\n", bufferStartPosition); */
            tableStart = tablePos;
            IN_FILE_SEEK(bufferStartPosition);
            areaPos = 0;
            while (areaPos < areaSize) {
              if (next_character() == '\n') {
                nlTable[tablePos] = IN_FILE_TELL();
                tablePos++;
                if (tablePos >= tableSize) {
                  tablePos = tableStart;
                  searching = FALSE;
                } /* if */
              } /* if */
              areaPos++;
            } /* while */
          } while (searching && bufferStartPosition > 0);
          if (!searching) {
            IN_FILE_SEEK(nlTable[tablePos]);
          } else {
            IN_FILE_SEEK(0);
          } /* if */
          error->errorLine = readLineFromCurrentFile();
          FREE_TABLE(nlTable, long, tableSize);
        } /* if */
      } /* if */
      IN_FILE_SEEK(currentPosition);
    } /* if */
    logFunction(printf("storeLineOfCurrentFile -->\n"););
  } /* storeLineOfCurrentFile */



static void storeLineFromBuffer (parseErrorType error,
    const const_ustriType lineStartPos, const_ustriType nlPos)

  { /* storeLineFromBuffer */
    if (nlPos != lineStartPos && nlPos[-1] == '\r') {
      /* Remove a '\r' which preceeds the '\n'. */
      nlPos--;
    } /* if */
    error->errorLine = ustri_buf_to_stri(lineStartPos,
                                         (memSizeType) (nlPos - lineStartPos));
  } /* storeLineFromBuffer */



static void storeLineFromFile (parseErrorType error, FILE *sourceFile,
    long lineStartPosInFile, long nlPosInFile)

  {
    memSizeType lineBufferSize;
    ucharType *lineBuffer;
    memSizeType numberOfCharsRead;

  /* storeLineFromFile */
    if (fseek(sourceFile, lineStartPosInFile, SEEK_SET) == 0) {
      lineBufferSize = (memSizeType) (nlPosInFile - lineStartPosInFile);
      if (ALLOC_USTRI(lineBuffer, lineBufferSize)) {
        numberOfCharsRead = fread(lineBuffer, 1, lineBufferSize, sourceFile);
        if (numberOfCharsRead != 0 && lineBuffer[numberOfCharsRead - 1] == '\r') {
          numberOfCharsRead--;
        } /* if */
        error->errorLine = ustri_buf_to_stri(lineBuffer, numberOfCharsRead);
        UNALLOC_USTRI(lineBuffer, lineBufferSize);
      } /* if */
    } /* if */
  } /* storeLineFromFile */



static void findLineEndAndStoreLine (parseErrorType error, FILE *sourceFile,
    const const_ustriType lineStartPos, const const_ustriType beyondDataInBuffer)

  {
    memSizeType numberOfCharsRead;
    long lineStartPosInFile;
    long nlPosInFile;
    ucharType *nlPos;
    ucharType buffer[BUFFER_SIZE];

  /* findLineEndAndStoreLine */
    lineStartPosInFile = ftell(sourceFile);
    if (lineStartPosInFile != -1) {
      lineStartPosInFile -= (long) (beyondDataInBuffer - lineStartPos);
      do {
        numberOfCharsRead = fread(buffer, 1, BUFFER_SIZE, sourceFile);
        nlPos = (ucharType *) memchr(buffer, '\n', numberOfCharsRead);
      } while (numberOfCharsRead != 0 && nlPos == NULL);
      nlPosInFile = ftell(sourceFile);
      if (nlPosInFile != -1) {
        if (nlPos != NULL) {
          nlPosInFile -= (long) (&buffer[numberOfCharsRead] - nlPos);
        } /* if */
        storeLineFromFile(error, sourceFile, lineStartPosInFile, nlPosInFile);
      } /* if */
    } /* if */
  } /* findLineEndAndStoreLine */



static void storeLineOfOtherFile (parseErrorType error,
    const const_striType sourceFileName, lineNumType lineNumber)

  {
    os_striType os_path;
    FILE *sourceFile;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    ucharType buffer[BUFFER_SIZE];
    memSizeType numberOfCharsRead;
    ucharType *lineStartPos;
    ucharType *nlPos;
    lineNumType line = 1;

  /* storeLineOfOtherFile */
    logFunction(printf("storeLineOfOtherFile(\"%s\", %d)\n",
                       striAsUnquotedCStri(sourceFileName), lineNumber););
    os_path = cp_to_os_path(sourceFileName, &path_info, &err_info);
    if (likely(os_path != NULL)) {
      sourceFile = os_fopen(os_path, os_mode_rb);
      /* printf("fopen(\"" FMT_S_OS "\") --> " FMT_U_MEM "\n",
             os_path, (memSizeType) sourceFile); */
      if (sourceFile != NULL) {
        do {
          numberOfCharsRead = fread(buffer, 1, BUFFER_SIZE, sourceFile);
          lineStartPos = buffer;
          nlPos = (ucharType *) memchr(buffer, '\n', numberOfCharsRead);
          while (line < lineNumber && nlPos != NULL) {
            line++;
            lineStartPos = &nlPos[1];
            nlPos = (ucharType *) memchr(lineStartPos, '\n',
                (memSizeType) (&buffer[numberOfCharsRead] - lineStartPos));
          } /* while */
        } while (line < lineNumber && numberOfCharsRead != 0);
        if (line == lineNumber) {
          if (nlPos != NULL) {
            storeLineFromBuffer(error, lineStartPos, nlPos);
          } else {
            findLineEndAndStoreLine(error, sourceFile, lineStartPos,
                                    &buffer[numberOfCharsRead]);
          } /* if */
        } /* if */
        fclose(sourceFile);
      } /* if */
    } /* if */
  } /* storeLineOfOtherFile */



static void storePositionedErrorLine (parseErrorType error,
    fileNumType fileNumber)

  { /* storePositionedErrorLine */
    logFunction(printf("storePositionedErrorLine(%u) "
                       "in_file.file_number=%u in_file.line=%u\n",
                       fileNumber, in_file.file_number, in_file.line););
    if (in_file.name != NULL) {
      error->columnNumber = 0;
      if (fileNumber == 0) {
        copyCStri(&error->errorLine, "");
      } else if (fileNumber == in_file.file_number) {
        storeLineOfCurrentFile(error, error->lineNumber);
      } else {
        storeLineOfOtherFile(error, error->fileName, error->lineNumber);
      } /* if */
    } /* if */
    logFunction(printf("storePositionedErrorLine -->\n"););
  } /* storePositionedErrorLine */



static void storeErrorLine (parseErrorType error)

  {
    long currentPosition;
    long lineStartPosition;
    long bufferStartPosition;
    boolType searchNewLine = TRUE;
    int charsToRead;
    int readCounter;
    int nlPos;
    int ch;

  /* storeErrorLine */
    logFunction(printf("storeErrorLine\n"););
    if (in_file.name != NULL && in_file.curr_infile != NULL &&
        (currentPosition = IN_FILE_TELL()) >= 0L) {
      if (currentPosition >= BUFFER_SIZE + 1) {
        charsToRead = BUFFER_SIZE;
        bufferStartPosition = currentPosition - BUFFER_SIZE - 1;
      } else {
        charsToRead = (int) currentPosition - 1;
        bufferStartPosition = 0;
      } /* if */
      do {
        /* printf("bufferStartPosition: %ld\n", bufferStartPosition);
           printf("charsToRead: %d\n", charsToRead); */
        IN_FILE_SEEK(bufferStartPosition);
        readCounter = 0;
        nlPos = -1;
        while (readCounter < charsToRead && (ch = next_character()) != EOF) {
          if (ch == '\n') {
            nlPos = readCounter;
          } /* if */
          readCounter++;
        } /* if */
        if (nlPos < 0 && bufferStartPosition != 0) {
          if (bufferStartPosition >= BUFFER_SIZE) {
            charsToRead = BUFFER_SIZE;
            bufferStartPosition -= BUFFER_SIZE;
          } else {
            charsToRead = (int) bufferStartPosition;
            bufferStartPosition = 0;
          } /* if */
        } else {
          searchNewLine = FALSE;
        } /* if */
      } while (searchNewLine);
      lineStartPosition = bufferStartPosition + nlPos + 1;
      storeColumnNumber(error, lineStartPosition, currentPosition);
      IN_FILE_SEEK(lineStartPosition);
      error->errorLine = readLineFromCurrentFile();
      IN_FILE_SEEK(currentPosition);
    } /* if */
    logFunction(printf("storeErrorLine -->\n"););
  } /* storeErrorLine */



static void setPlace (parseErrorType error, const const_striType name,
    const lineNumType lineNumber)

  { /* setPlace */
    logFunction(printf("setPlace(*, \"%s\", %d) in_file.line=%u\n",
                       striAsUnquotedCStri(name), lineNumber, in_file.line););
    error->fileName = name;
    error->lineNumber = lineNumber;
  } /* setPlace */



static void setPlaceForFileNumber (parseErrorType error,
    fileNumType fileNumber, lineNumType lineNumber)

  { /* setPlaceForFileNumber */
    logFunction(printf("setPlaceForFileNumber(*, %u, %u)\n",
                       fileNumber, lineNumber););
    setPlace(error, get_file_name(prog, fileNumber), lineNumber);
  } /* setPlaceForFileNumber */



static void setPlaceOfError (parseErrorType error)

  { /* setPlaceOfError */
    if (in_file.name != NULL) {
      setPlace(error, in_file.name, in_file.line);
    } /* if */
  } /* setPlaceOfError */



static void setPlaceAndLine (parseErrorType error, const_objectType expr_object)

  {
    fileNumType fileNumber;

  /* setPlaceAndLine */
    if (HAS_POSINFO(expr_object)) {
      fileNumber = POSINFO_FILE_NUM(expr_object);
      setPlace(error, get_file_name(prog, fileNumber),
               POSINFO_LINE_NUM(expr_object));
      storePositionedErrorLine(error, fileNumber);
    } else {
      if (in_file.name != NULL) {
        setPlace(error, in_file.name, in_file.line);
      } /* if */
      storeErrorLine(error);
    } /* if */
  } /* setPlaceAndLine */



static void undefErr (striType *const msg)

  { /* undefErr */
    copyCStri(msg, "Undefined error");
  } /* undefErr */



void err_warning (errorType err)

  {
    parseErrorType error;

  /* err_warning */
    logFunction(printf("err_warning(%d)\n", err););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case OUT_OF_HEAP_SPACE:
        copyCStri(&error->msg, "No more memory");
        break;
      case EOF_ENCOUNTERED:
        copyCStri(&error->msg, "\"END OF FILE\" encountered");
        break;
      case ILLEGALPRAGMA:
        copyCStri(&error->msg, "Illegal pragma ");
        appendSymbol(&error->msg, &symbol);
        break;
      case WRONGSYSTEM:
        copyCStri(&error->msg, "Illegal system declaration ");
        appendSymbol(&error->msg, &symbol);
        break;
      case NEGATIVE_EXPONENT:
        copyCStri(&error->msg, "Negative exponent in integer literal");
        break;
      case CHAR_EXCEEDS:
        copyCStri(&error->msg, "Character literal exceeds source line");
        break;
      case WRONG_NUMERICAL_ESCAPE:
        copyCStri(&error->msg, "Numerical escape sequences should end with \";\" not EOF");
        break;
      case WRONG_STRING_CONTINUATION:
        copyCStri(&error->msg, "String continuations should end with \"\\\" not EOF");
        break;
      case STRING_EXCEEDS:
        copyCStri(&error->msg, "String literal exceeds source line");
        break;
      case WRONG_QUOTATION_REPRESENTATION:
        copyCStri(&error->msg, "Use \\\" instead of \"\" to represent \" in a string");
        break;
      case WRONG_PATH_DELIMITER:
        copyCStri(&error->msg, "Use / instead of \\ as path delimiter");
        break;
      case NAME_EXPECTED:
        copyCStri(&error->msg, "Name expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case INTEGER_EXPECTED:
        copyCStri(&error->msg, "Integer literal expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case STRING_EXPECTED:
        copyCStri(&error->msg, "String literal expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case ILLEGAL_ASSOCIATIVITY:
        copyCStri(&error->msg, "Associativity expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case EXPR_EXPECTED:
        copyCStri(&error->msg, "Expression expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case TWO_PARAMETER_SYNTAX:
        copyCStri(&error->msg, "Syntax with two parameters before operator is illegal");
        break;
      case EMPTY_SYNTAX:
        copyCStri(&error->msg, "Empty syntax declaration");
        break;
      case TYPE_EXPECTED:
        copyCStri(&error->msg, "Type expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case LITERAL_TYPE_UNDEFINED:
        copyCStri(&error->msg, "Undefined type for literal ");
        appendSymbol(&error->msg, &symbol);
        break;
      case DOLLAR_VALUE_WRONG:
        copyCStri(&error->msg, "\"newtype\", \"subtype\", \"func\", \"enumlit\" or \"action\" expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case DOLLAR_TYPE_WRONG:
        copyCStri(&error->msg, "\"func\" or \"type\" expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case ESSENTIAL_INCLUDE_FAILED:
        copyCStri(&error->msg, "Failed to include essential file. Parsing terminated.");
        break;
      case SYSTEM_MAIN_MISSING:
        copyCStri(&error->msg, "System declaration for main missing");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_warning */



void err_num_stri (errorType err, int num_found, int num_expected,
    const_ustriType stri)

  {
    parseErrorType error;

  /* err_num_stri */
    logFunction(printf("err_num_stri(%d, %d, %d \"%s\")\n",
                       err, num_found, num_expected, stri););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case REDECLARED_INFIX_PRIORITY:
        copyCStri(&error->msg, "\"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" redeclared with infix priority ");
        appendInt(&error->msg, (intType) num_found);
        appendCStri(&error->msg, " not ");
        appendInt(&error->msg, (intType) num_expected);
        break;
      case REDECLARED_PREFIX_PRIORITY:
        copyCStri(&error->msg, "\"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" redeclared with prefix priority ");
        appendInt(&error->msg, (intType) num_found);
        appendCStri(&error->msg, " not ");
        appendInt(&error->msg, (intType) num_expected);
        break;
      case WRONG_EXPR_PARAM_PRIORITY:
        copyCStri(&error->msg, "Priority ");
        appendInt(&error->msg, (intType) num_expected);
        appendCStri(&error->msg, " required for parameter after \"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" not ");
        appendInt(&error->msg, (intType) num_found);
        break;
      case WRONG_PREFIX_PRIORITY:
        copyCStri(&error->msg, "Priority <= ");
        appendInt(&error->msg, (intType) num_expected);
        appendCStri(&error->msg, " expected found \"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" with priority ");
        appendInt(&error->msg, (intType) num_found);
        break;
      case DOT_EXPR_ILLEGAL:
        copyCStri(&error->msg, "\"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" must have priority ");
        appendInt(&error->msg, (intType) num_expected);
        appendCStri(&error->msg, " not ");
        appendInt(&error->msg, (intType) num_found);
        appendCStri(&error->msg, " for dot expression");
        break;
      case ILLEGAL_BASED_DIGIT:
        copyCStri(&error->msg, "Illegal digit \"");
        appendChar(&error->msg, (charType) num_found);
        appendCStri(&error->msg, "\" in based integer \"");
        appendInt(&error->msg, (intType) num_expected);
        appendChar(&error->msg, '#');
        appendUStri(&error->msg, stri);
        appendChar(&error->msg, '"');
        break;
      case BASED_INTEGER_TOO_BIG:
        copyCStri(&error->msg, "Based integer \"");
        appendInt(&error->msg, (intType) num_expected);
        appendChar(&error->msg, '#');
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" too big");
        break;
      case INTEGER_WITH_EXPONENT_TOO_BIG:
        copyCStri(&error->msg, "Integer \"");
        appendInt(&error->msg, (intType) num_expected);
        appendChar(&error->msg, 'E');
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" too big");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_num_stri */



void err_ident (errorType err, const_identType ident)

  {
    parseErrorType error;

  /* err_ident */
    logFunction(printf("err_ident(%d, " FMT_U_MEM ")\n",
                       err, (memSizeType) ident););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case PARAM_SPECIFIER_EXPECTED:
        copyCStri(&error->msg, "Parameter specifier \"ref\" expected found \"");
        appendUStri(&error->msg, ident->name);
        appendChar(&error->msg, '"');
        break;
      case EXPECTED_SYMBOL:
        copyCStri(&error->msg, "\"");
        appendUStri(&error->msg, ident->name);
        appendCStri(&error->msg, "\" expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_ident */



void err_object (errorType err, const_objectType obj_found)

  {
    parseErrorType error;

  /* err_object */
    logFunction(printf("err_object(%d, ", err);
                trace1(obj_found);
                printf(")\n"););
    error = newError(err);
    if (obj_found != NULL) {
      setPlaceAndLine(error, obj_found);
    } else {
      if (in_file.name != NULL) {
        setPlace(error, in_file.name, in_file.line);
      } /* if */
      storeErrorLine(error);
    } /* if */
    switch (err) {
      case PARAM_DECL_FAILED:
        copyCStri(&error->msg, "Declaration of parameter ");
        appendList(&error->msg, obj_found->value.listValue->next);
        appendCStri(&error->msg, " failed");
        break;
      case DECL_FAILED:
        copyCStri(&error->msg, "Declaration of \"");
        appendObjectWithParameters(&error->msg, obj_found);
        appendCStri(&error->msg, "\" failed");
        break;
      case EXCEPTION_RAISED:
        if (obj_found != NULL && HAS_ENTITY(obj_found)) {
          copyCStri(&error->msg, "Exception \"");
          appendUStri(&error->msg, GET_ENTITY(obj_found)->ident->name);
          appendCStri(&error->msg, "\" raised");
        } else {
          copyCStri(&error->msg, "Exception raised");
        } /* if */
        break;
      case SYNTAX_DECLARED_TWICE:
        copyCStri(&error->msg, "Syntax ");
        if (CATEGORY_OF_OBJ(obj_found) == LISTOBJECT) {
          appendDotExpr(&error->msg, obj_found->value.listValue);
        } else {
          appendObject(&error->msg, obj_found);
        } /* if */
        appendCStri(&error->msg, " declared twice");
        break;
      case EXPR_EXPECTED:
        copyCStri(&error->msg, "Expression expected found ");
        appendObject(&error->msg, obj_found);
        break;
      case DOT_EXPR_EXPECTED:
        copyCStri(&error->msg, "Dot expression expected as syntax description, found ");
        appendObject(&error->msg, obj_found);
        break;
      case IDENT_EXPECTED:
        copyCStri(&error->msg, "Identifier expected found ");
        switch (CATEGORY_OF_OBJ(obj_found)) {
          case INTOBJECT:
            appendChar(&error->msg, '"');
            appendInt(&error->msg, obj_found->value.intValue);
            appendChar(&error->msg, '"');
            break;
          case CHAROBJECT:
            appendCharLiteral(&error->msg, obj_found->value.charValue);
            break;
          case STRIOBJECT:
            appendStriLiteral(&error->msg, obj_found->value.striValue);
            break;
#if WITH_FLOAT
          case FLOATOBJECT:
            appendChar(&error->msg, '"');
            appendFloat(&error->msg, obj_found->value.floatValue);
            appendChar(&error->msg, '"');
            break;
#endif
          default:
            appendObject(&error->msg, obj_found);
            break;
        } /* switch */
        break;
      case NO_MATCH:
        copyCStri(&error->msg, "Match for ");
        appendListLimited(&error->msg, obj_found->value.listValue, MAX_DEPTH_SHOWN);
        appendCStri(&error->msg, " failed");
        break;
      case PROC_EXPECTED:
        copyCStri(&error->msg, "Procedure expected found \"");
        appendObject(&error->msg, obj_found);
        appendCStri(&error->msg, "\" expression");
        break;
      case TYPE_EXPECTED:
        copyCStri(&error->msg, "Type expected found ");
        appendObject(&error->msg, obj_found);
        break;
      case EXCEPTION_EXPECTED:
        copyCStri(&error->msg, "Exception expected found ");
        appendObject(&error->msg, obj_found);
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_object */



void err_type (errorType err, const_objectType expr_object)

  {
    parseErrorType error;

  /* err_type */
    logFunction(printf("err_type(%d, ", err);
                trace1(expr_object);
                printf(")\n"););
    error = newError(err);
    setPlaceAndLine(error, expr_object);
    switch (err) {
      case PROC_EXPECTED:
        copyCStri(&error->msg, "Procedure expected found \"");
        appendType(&error->msg, expr_object->type_of);
        appendCStri(&error->msg, "\" expression");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_type */



void err_expr_type (errorType err, const_objectType expr_object,
    const_typeType type_found)

  {
    parseErrorType error;

  /* err_expr_type */
    logFunction(printf("err_expr_type(%d, ", err);
                trace1(expr_object);
                printf(", ");
                printtype(type_found);
                printf(")\n"););
    error = newError(err);
    setPlaceAndLine(error, expr_object);
    switch (err) {
      case KIND_OF_IN_PARAM_UNDEFINED:
        copyCStri(&error->msg, "Kind of in-parameter (val or ref) unspecified for type \"");
        appendType(&error->msg, type_found);
        appendChar(&error->msg, '"');
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_expr_type */



void err_expr_obj (errorType err, const_objectType expr_object,
    objectType obj_found)

  {
    parseErrorType error;

  /* err_expr_obj */
    logFunction(printf("err_expr_obj(%d, ", err);
                trace1(expr_object);
                printf(", ");
                trace1(obj_found);
                printf(")\n"););
    error = newError(err);
    setPlaceAndLine(error, expr_object);
    switch (err) {
      case WRONGACCESSRIGHT:
        copyCStri(&error->msg, "Variable expected in ");
        appendListLimited(&error->msg, expr_object->value.listValue->next, MAX_DEPTH_SHOWN);
        appendCStri(&error->msg, " found ");
        appendObject(&error->msg, obj_found);
        break;
      case DECL_FAILED:
        copyCStri(&error->msg, "Declaration of \"");
        appendObjectWithParameters(&error->msg, obj_found);
        appendCStri(&error->msg, "\" failed");
        break;
      case INTEGER_EXPECTED:
        if (obj_found == NULL) {
          copyCStri(&error->msg, "Integer literal expected");
        } else {
          copyCStri(&error->msg, "Integer literal expected found ");
          if (CATEGORY_OF_OBJ(obj_found) == STRIOBJECT) {
            appendObject(&error->msg, obj_found);
          } else {
            appendChar(&error->msg, '"');
            appendObject(&error->msg, obj_found);
            appendChar(&error->msg, '"');
          } /* if */
        } /* if */
        break;
      case EXCEPTION_RAISED:
        if (obj_found != NULL && HAS_ENTITY(obj_found)) {
          copyCStri(&error->msg, "Exception \"");
          appendUStri(&error->msg, GET_ENTITY(obj_found)->ident->name);
          appendCStri(&error->msg, "\" raised");
        } else {
          copyCStri(&error->msg, "Exception raised");
        } /* if */
        break;
      case ILLEGAL_ASSOCIATIVITY:
        if (obj_found == NULL) {
          copyCStri(&error->msg, "Associativity expected");
        } else {
          copyCStri(&error->msg, "Associativity expected found ");
          if (CATEGORY_OF_OBJ(obj_found) == STRIOBJECT) {
            appendObject(&error->msg, obj_found);
          } else {
            appendChar(&error->msg, '"');
            appendObject(&error->msg, obj_found);
            appendChar(&error->msg, '"');
          } /* if */
        } /* if */
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_expr_obj */



void err_expr_obj_stri (errorType err, const_objectType expr_object,
    objectType obj_found, const_cstriType stri)

  {
    parseErrorType error;

  /* err_expr_obj_stri */
    logFunction(printf("err_expr_obj_stri(%d, ", err);
                trace1(expr_object);
                printf(", ");
                trace1(obj_found);
                printf(", \"%s\")\n", stri););
    error = newError(err);
    setPlaceAndLine(error, expr_object);
    switch (err) {
      case EXPECTED_SYMBOL:
        copyCStri(&error->msg, "\"");
        appendCStri(&error->msg, stri);
        appendCStri(&error->msg, "\" expected found ");
        if (CATEGORY_OF_OBJ(obj_found) == STRIOBJECT) {
          appendObject(&error->msg, obj_found);
        } else {
          appendChar(&error->msg, '"');
          appendObject(&error->msg, obj_found);
          appendChar(&error->msg, '"');
        } /* if */
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_expr_obj_stri */



static boolType contains_match_err_flag (objectType curr_obj)

  {
    const_listType list_elem;
    boolType result;

  /* contains_match_err_flag */
    result = FALSE;
    if (HAS_MATCH_ERR(curr_obj)) {
      result = TRUE;
    } else if (CATEGORY_OF_OBJ(curr_obj) == EXPROBJECT) {
      list_elem = curr_obj->value.listValue;
      while (list_elem != NULL) {
        if (list_elem->obj != NULL && contains_match_err_flag(list_elem->obj)) {
          result = TRUE;
        } /* if */
        list_elem = list_elem->next;
      } /* while */
    } /* if */
    return result;
  } /* contains_match_err_flag */



void err_match (errorType err, objectType obj_found)

  {
    parseErrorType error;

  /* err_match */
    logFunction(printf("err_match(%d, ", err);
                trace1(obj_found);
                printf(")\n"););
    if (!contains_match_err_flag(obj_found)) {
      error = newError(err);
      setPlaceAndLine(error, obj_found);
      switch (err) {
        case NO_MATCH:
          copyCStri(&error->msg, "Match for ");
          appendListLimited(&error->msg, obj_found->value.listValue, MAX_DEPTH_SHOWN);
          appendCStri(&error->msg, " failed");
          break;
        case EXPR_EXPECTED:
          copyCStri(&error->msg, "Expression expected found \"");
          appendUStri(&error->msg, GET_ENTITY(obj_found)->ident->name);
          appendChar(&error->msg, '"');
          break;
        default:
          undefErr(&error->msg);
          break;
      } /* switch */
      finalizeError(error);
      display_compilation_info();
    } /* if */
    SET_MATCH_ERR_FLAG(obj_found);
  } /* err_match */



void err_ustri (errorType err, const const_ustriType stri)

  {
    parseErrorType error;

  /* err_ustri */
    logFunction(printf("err_ustri(%d, \"%s\")\n", err, stri););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case ILLEGALPRAGMA:
        copyCStri(&error->msg, "Illegal parameter ");
        appendSymbol(&error->msg, &symbol);
        appendCStri(&error->msg, " in pragma \"");
        appendUStri(&error->msg, stri);
        appendChar(&error->msg, '"');
        break;
      case EXPECTED_SYMBOL:
        copyCStri(&error->msg, "\"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" expected found ");
        appendSymbol(&error->msg, &symbol);
        break;
      case INTEGER_TOO_BIG:
        copyCStri(&error->msg, "Integer \"");
        appendUStri(&error->msg, stri);
        appendCStri(&error->msg, "\" too big");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_ustri */



void err_stri (errorType err, const const_striType stri)

  {
    parseErrorType error;

  /* err_stri */
    logFunction(printf("err_stri(%d, \"%s\")\n", err,
                       striAsUnquotedCStri(stri)););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case INCLUDE_FILE_NOT_FOUND:
        copyCStri(&error->msg, "Include file ");
        appendStriLiteral(&error->msg, stri);
        appendCStri(&error->msg, " not found");
        break;
      case UNDEFINED_ACTION:
        copyCStri(&error->msg, "Undefined action ");
        appendStriLiteral(&error->msg, stri);
        break;
      case WRONG_PATH_DELIMITER:
        copyCStri(&error->msg, "Use / instead of \\ as path delimiter in ");
        appendStriLiteral(&error->msg, stri);
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_stri */



void err_integer (errorType err, intType number)

  {
    parseErrorType error;

  /* err_integer */
    logFunction(printf("err_integer(%d, " FMT_D ")\n",
                       err, number););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case BASE2TO36ALLOWED:
        copyCStri(&error->msg, "Integer base \"");
        appendInt(&error->msg, number);
        appendCStri(&error->msg, "\" not between 2 and 36");
        break;
      case ILLEGAL_PRIORITY:
        copyCStri(&error->msg, "Statement priority \"");
        appendInt(&error->msg, number);
        if (number < 0) {
          appendCStri(&error->msg, "\" too small");
        } else {
          appendCStri(&error->msg, "\" too big");
        } /* if */
        break;
      case NUMERICAL_ESCAPE_TOO_BIG:
        copyCStri(&error->msg, "The numerical escape sequence \"\\");
        appendInt(&error->msg, number);
        appendCStri(&error->msg, ";\" is too big");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_integer */



void err_cchar (errorType err, int character)

  {
    char buffer[100];
    parseErrorType error;

  /* err_cchar */
    logFunction(printf("err_cchar(%d, %d)\n",
                       err, character););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case CHAR_ILLEGAL:
        copyCStri(&error->msg, "Illegal character in text \"");
        break;
      case DIGIT_EXPECTED:
        copyCStri(&error->msg, "Digit expected found \"");
        break;
      case EXTENDED_DIGIT_EXPECTED:
        copyCStri(&error->msg, "Extended digit expected found \"");
        break;
      case APOSTROPHE_EXPECTED:
        copyCStri(&error->msg, "\"'\" expected found \"");
        break;
      case WRONG_STRING_CONTINUATION:
        copyCStri(&error->msg, "String continuations should end with \"\\\" not \"");
        break;
      case WRONG_NUMERICAL_ESCAPE:
        copyCStri(&error->msg, "Numerical escape sequences should end with \";\" not \"");
        break;
      case ILLEGAL_STRING_ESCAPE:
        copyCStri(&error->msg, "Illegal string escape \"\\");
        break;
      case UTF8_CONTINUATION_BYTE_EXPECTED:
        copyCStri(&error->msg, "UTF-8 continuation byte expected found \"");
        break;
      case UNEXPECTED_UTF8_CONTINUATION_BYTE:
        copyCStri(&error->msg, "Unexpected UTF-8 continuation byte found \"");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      sprintf(buffer, "%c\"", character);
    } else {
      sprintf(buffer, "\\%u;\" (U+%04x)", character, character);
    } /* if */
    appendCStri(&error->msg, buffer);
    finalizeError(error);
    display_compilation_info();
  } /* err_cchar */



void err_char (errorType err, charType character)

  {
    char buffer[100];
    parseErrorType error;

  /* err_char */
    logFunction(printf("err_char(%d, " FMT_U32 ")\n",
                       err, character););
    error = newError(err);
    setPlaceOfError(error);
    storeErrorLine(error);
    switch (err) {
      case CHAR_ILLEGAL:
        copyCStri(&error->msg, "Illegal character in text");
        break;
      case OVERLONG_UTF8_ENCODING:
        copyCStri(&error->msg, "Overlong UTF-8 encoding used for character");
        break;
      case UTF16_SURROGATE_CHAR_FOUND:
        copyCStri(&error->msg, "UTF-16 surrogate character found in UTF-8 encoding");
        break;
      case CHAR_NOT_UNICODE:
        copyCStri(&error->msg, "Non Unicode character found");
        break;
      case SOLITARY_UTF8_START_BYTE:
        copyCStri(&error->msg, "Solitary UTF-8 start byte found");
        break;
      case UTF16_BYTE_ORDER_MARK_FOUND:
        copyCStri(&error->msg, "UTF-16 byte order mark found");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      sprintf(buffer, " \"%c\"", (char) character);
    } else {
      sprintf(buffer, " \"\\%lu;\" (U+%04lx)",
              (unsigned long) character, (unsigned long) character);
    } /* if */
    appendCStri(&error->msg, buffer);
    finalizeError(error);
    display_compilation_info();
  } /* err_char */



void err_at_line (errorType err, lineNumType lineNumber)

  {
    parseErrorType error;

  /* err_at_line */
    logFunction(printf("err_at_line(%d, %u)\n",
                       err, lineNumber););
    error = newError(err);
    setPlace(error, in_file.name, lineNumber);
    storePositionedErrorLine(error, in_file.file_number);
    switch (err) {
      case UNCLOSED_COMMENT:
        copyCStri(&error->msg, "Unclosed comment");
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_at_line */



void err_at_file_in_line (errorType err, const_objectType obj_found,
    fileNumType fileNumber, lineNumType lineNumber)

  {
    parseErrorType error;

  /* err_at_file_in_line */
    logFunction(printf("err_at_file_in_line(%d, ", err);
                trace1(obj_found);
                printf(", %u, %u)\n", fileNumber, lineNumber););
    error = newError(err);
    if (fileNumber != 0 && lineNumber != 0) {
      setPlaceForFileNumber(error, fileNumber, lineNumber);
      if (fileNumber != in_file.file_number || lineNumber != in_file.line) {
        storePositionedErrorLine(error, fileNumber);
      } else {
        storeErrorLine(error);
      } /* if */
    } else {
      setPlace(error, in_file.name, in_file.line);
      storeErrorLine(error);
    } /* if */
    switch (err) {
      case PARAM_DECL_OR_SYMBOL_EXPECTED:
        copyCStri(&error->msg, "Parameter declaration or symbol expected found ");
        appendObject(&error->msg, obj_found);
        break;
      case EXCEPTION_RAISED:
        if (obj_found != NULL && HAS_ENTITY(obj_found)) {
          copyCStri(&error->msg, "Exception \"");
          appendUStri(&error->msg, GET_ENTITY(obj_found)->ident->name);
          appendCStri(&error->msg, "\" raised");
        } else {
          copyCStri(&error->msg, "Exception raised");
        } /* if */
        break;
      case REDECLARATION:
        copyCStri(&error->msg, "Redeclaration of \"");
        appendObjectWithParameters(&error->msg, obj_found);
        appendChar(&error->msg, '"');
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_at_file_in_line */



void err_existing_obj (errorType err, const_objectType obj_found)

  {
    fileNumType fileNumber;
    lineNumType lineNumber;
    parseErrorType error;

  /* err_existing_obj */
    logFunction(printf("err_existing_obj(%d, ", err);
                trace1(obj_found);
                printf(")\n"););
    error = newError(err);
    if (HAS_PROPERTY(obj_found)) {
      fileNumber = obj_found->descriptor.property->file_number;
      lineNumber = obj_found->descriptor.property->line;
    } else {
      fileNumber = 0;
      lineNumber = 0;
    } /* if */
    setPlaceForFileNumber(error, fileNumber, lineNumber);
    storePositionedErrorLine(error, fileNumber);
    switch (err) {
      case DECL_FAILED:
        copyCStri(&error->msg, "Declaration of \"");
        appendObjectWithParameters(&error->msg, obj_found);
        appendCStri(&error->msg, "\" failed");
        break;
      case PREVIOUS_DECLARATION:
        copyCStri(&error->msg, "Previous declaration of \"");
        appendObjectWithParameters(&error->msg, obj_found);
        appendChar(&error->msg, '"');
        break;
      default:
        undefErr(&error->msg);
        break;
    } /* switch */
    finalizeError(error);
    display_compilation_info();
  } /* err_existing_obj */
