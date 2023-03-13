/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015, 2019, 2021  Thomas Mertes            */
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
#include "stat.h"
#include "chr_rtl.h"
#include "con_drv.h"

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



/**
 *  Read a line from infile.
 *  The reading stops, at the end of the line or when there is no more memory.
 *  @param line_length Place to return the length of the line (in bytes).
 *  @return a line as sequence of bytes (not null terminated), or
 *          NULL if there is no more memory.
 */
static ustriType read_ustri8_line (memSizeType *line_len)

  {
    int ch;
    memSizeType chars_read = 0;
    memSizeType buffer_len = 0;
    ustriType resized_line;
    ustriType line = NULL;

  /* read_ustri8_line */
    logFunction(printf("read_ustri8_line(*)\n"););
    if ((ch = next_character()) == '\r') {
      ch = next_character();
    } /* if */
    while (ch != '\r' && ch != '\n' && ch != EOF) {
      if (chars_read >= buffer_len &&
          (resized_line = REALLOC_USTRI(line, buffer_len,
                                        buffer_len + LINE_SIZE_INCREMENT)) != NULL) {
        line = resized_line;
        buffer_len += LINE_SIZE_INCREMENT;
      } /* if */
      if (chars_read >= buffer_len) {
        /* The realloc failed: Assume that the line ends. */
        ch = EOF;
      } else {
        line[chars_read] = (ucharType) ch;
        chars_read++;
        ch = next_character();
      } /* if */
    } /* while */
    resized_line = REALLOC_USTRI(line, buffer_len, chars_read);
    if (resized_line != NULL) {
      line = resized_line;
    } /* if */
    *line_len = chars_read;
    logFunction(printf("read_ustri8_line(" FMT_U_MEM ") --> " FMT_U_MEM "\n",
                       *line_len, (memSizeType) line););
    return line;
  } /* read_ustri8_line */



/**
 *  Convert 'length' UTF-8 encoded bytes in 'ustri' to a string.
 *  Bytes that are not UTF-8 encoded are left as is.
 *  @return the converted string or NULL, if a memory error occurred.
 */
static striType ustri8_buffer_to_stri (const const_ustriType ustri,
    const memSizeType length)

  {
    memSizeType stri_size = 0;
    memSizeType converted_size;
    memSizeType unconverted;
    ucharType ch;
    striType resized_stri;
    striType stri;

  /* ustri8_buffer_to_stri */
    logFunction(printf("ustri8_buffer_to_stri(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) ustri, length););
    if (ustri == NULL) {
      stri = NULL;
    } else if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      unconverted = length;
      do {
        unconverted = utf8_to_stri(&stri->mem[stri_size], &converted_size,
                                   &ustri[length - unconverted], unconverted);
        stri_size += converted_size;
        if (unconverted != 0) {
          ch = ustri[length - unconverted];
          if (ch >= 0xC0) {
            /* ch range 192 to 255 (leading bits 11......) */
            do {
              stri->mem[stri_size] = (strElemType) ch;
              stri_size++;
              unconverted--;
              if (unconverted != 0) {
                ch = ustri[length - unconverted];
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
      REALLOC_STRI_SIZE_SMALLER(resized_stri, stri, length, stri_size);
      if (resized_stri == NULL) {
        FREE_STRI(stri, length);
        stri = NULL;
      } else {
        stri = resized_stri;
        COUNT3_STRI(length, stri_size);
        stri->size = stri_size;
      } /* if */
    } /* if */
    logFunction(printf("ustri8_buffer_to_stri(" FMT_U_MEM ", " FMT_U_MEM ") --> \"%s\"\n",
                       (memSizeType) ustri, length, striAsUnquotedCStri(stri)););
    return stri;
  } /* ustri8_buffer_to_stri */



static memSizeType calculate_output_length (striType stri)

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



static void print_stri (striType stri)

  {
    memSizeType pos;
    charType ch;
    memSizeType width;
    memSizeType output_length = 0;
    struct striStruct stri1_buffer;
    striType stri1;
    char escapeBuffer[ESC_SEQUENCE_MAX_LEN + NULL_TERMINATION_LEN];
    char tabBuffer[TAB_POSITION + NULL_TERMINATION_LEN];

  /* print_stri */
    logFunction(printf("print_stri(\"%s\")\n", striAsUnquotedCStri(stri)););
    for (pos = 0; pos < stri->size; pos++) {
      ch = stri->mem[pos];
      if ((ch >= 0xd800 && ch <= 0xdfff) || ch > 0x10ffff) {
        /* UTF-16 surrogate character or non Unicode character. */
        output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
        prot_cstri(escapeBuffer);
      } else {
        width = (memSizeType) chrWidth(ch);
        if (width >= 1) {
          stri1 = chrStrMacro(ch, stri1_buffer);
          prot_string(stri1);
          output_length += width;
        } else if (ch < ' ') {
          if (ch == '\t') {
            width = TAB_POSITION - output_length % TAB_POSITION;
            memset(tabBuffer, ' ', width);
            tabBuffer[width] = '\0';
            prot_cstri(tabBuffer);
            output_length += width;
          } else {
            prot_cstri(stri_escape_sequence[ch]);
            output_length += strlen(stri_escape_sequence[ch]);
          } /* if */
        } else {
          output_length += (memSizeType) sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
          prot_cstri(escapeBuffer);
        } /* if */
      } /* if */
    } /* for */
    logFunction(printf("print_stri -->\n"););
  } /* print_stri */



static void read_and_print_line (long line_start_position, long current_position)

  {
    ustriType line;
    memSizeType line_len;
    memSizeType part1_len;
    striType part1;
    memSizeType error_column;
    striType lineStri;

  /* read_and_print_line */
    logFunction(printf("read_and_print_line(%ld, %ld)\n",
                       line_start_position, current_position););
    line = read_ustri8_line(&line_len);
    if (line_start_position <= current_position) {
      if (line_start_position + 1 < current_position) {
        part1_len = (memSizeType) (current_position - line_start_position - 1);
        if (part1_len > line_len) {
          part1_len = line_len;
        } /* if */
        part1 = ustri8_buffer_to_stri(line, part1_len);
        if (part1 != NULL) {
          error_column = calculate_output_length(part1);
          FREE_STRI(part1, part1->size);
        } else {
          error_column = part1_len;
        } /* if */
      } else {
        error_column = 0;
      } /* if */
      lineStri = ustri8_buffer_to_stri(line, line_len);
      if (lineStri != NULL) {
        print_stri(lineStri);
        FREE_STRI(lineStri, lineStri->size);
      } /* if */
      prot_nl();
      for (; error_column > 0; error_column--) {
        prot_cstri("-");
      } /* for */
      prot_cstri("^");
    } else {
      lineStri = ustri8_buffer_to_stri(line, line_len);
      if (lineStri != NULL) {
        print_stri(lineStri);
        FREE_STRI(lineStri, lineStri->size);
      } /* if */
      prot_nl();
    } /* if */
    prot_nl();
    UNALLOC_USTRI(line, line_len);
    logFunction(printf("read_and_print_line(%ld, %ld) -->\n",
                       line_start_position, current_position););
  } /* read_and_print_line */



static void printLineOfCurrentFile (lineNumType errorLine)

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

  /* printLineOfCurrentFile */
    logFunction(printf("printLineOfCurrentFile(%u) in_file.line=%u\n",
                       errorLine, in_file.line););
    if (in_file.curr_infile != NULL &&
        (currentPosition = IN_FILE_TELL()) >= 0L) {
      /* printf("currentPosition=%lu in_file.character=%d\n",
         currentPosition, in_file.character); */
      tableSize = in_file.line - errorLine + 1;
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
          read_and_print_line(currentPosition + 1, currentPosition);
          FREE_TABLE(nlTable, long, tableSize);
        } /* if */
      } /* if */
      IN_FILE_SEEK(currentPosition);
    } /* if */
    logFunction(printf("printLineOfCurrentFile -->\n"););
  } /* printLineOfCurrentFile */



static void printLineFromBuffer (const const_ustriType lineStartPos, const_ustriType nlPos)

  {
    striType lineStri;

  /* printLineFromBuffer */
    if (nlPos != lineStartPos && nlPos[-1] == '\r') {
      nlPos--;
    } /* if */
    lineStri = ustri8_buffer_to_stri(lineStartPos, (memSizeType) (nlPos - lineStartPos));
    if (lineStri != NULL) {
      print_stri(lineStri);
      FREE_STRI(lineStri, lineStri->size);
    } /* if */
    prot_nl();
    prot_nl();
  } /* printLineFromBuffer */



static void printLineFromFile (FILE *sourceFile, long lineStartPosInFile, long nlPosInFile)

  {
    memSizeType lineBufferSize;
    ucharType *lineBuffer;
    memSizeType numberOfCharsRead;
    striType lineStri;

  /* printLineFromFile */
    if (fseek(sourceFile, lineStartPosInFile, SEEK_SET) == 0) {
      lineBufferSize = (memSizeType) (nlPosInFile - lineStartPosInFile);
      if (ALLOC_USTRI(lineBuffer, lineBufferSize)) {
        numberOfCharsRead = fread(lineBuffer, 1, lineBufferSize, sourceFile);
        if (numberOfCharsRead != 0 && lineBuffer[numberOfCharsRead - 1] == '\r') {
          numberOfCharsRead--;
        } /* if */
        lineStri = ustri8_buffer_to_stri(lineBuffer, numberOfCharsRead);
        if (lineStri != NULL) {
          print_stri(lineStri);
          FREE_STRI(lineStri, lineStri->size);
        } /* if */
        prot_nl();
        prot_nl();
        UNALLOC_USTRI(lineBuffer, lineBufferSize);
      } /* if */
    } /* if */
  } /* printLineFromFile */



static void findLineEndAndPrintLine (FILE *sourceFile, const const_ustriType lineStartPos,
    const const_ustriType beyondDataInBuffer)

  {
    memSizeType numberOfCharsRead;
    long lineStartPosInFile;
    long nlPosInFile;
    ucharType *nlPos;
    ucharType buffer[BUFFER_SIZE];

  /* findLineEndAndPrintLine */
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
        printLineFromFile(sourceFile, lineStartPosInFile, nlPosInFile);
      } /* if */
    } /* if */
  } /* findLineEndAndPrintLine */



static void printLineOfOtherFile (const_striType sourceFileName, lineNumType errorLine)

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

  /* printLineOfOtherFile */
    logFunction(printf("printLineOfOtherFile(\"%s\", %d)\n",
                       striAsUnquotedCStri(sourceFileName), errorLine););
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
          while (line < errorLine && nlPos != NULL) {
            line++;
            lineStartPos = &nlPos[1];
            nlPos = (ucharType *) memchr(lineStartPos, '\n',
                (memSizeType) (&buffer[numberOfCharsRead] - lineStartPos));
          } /* while */
        } while (line < errorLine && numberOfCharsRead != 0);
        if (line == errorLine) {
          if (nlPos != NULL) {
            printLineFromBuffer(lineStartPos, nlPos);
          } else {
            findLineEndAndPrintLine(sourceFile, lineStartPos, &buffer[numberOfCharsRead]);
          } /* if */
        } /* if */
        fclose(sourceFile);
      } /* if */
    } /* if */
  } /* printLineOfOtherFile */



static void print_line (fileNumType fileNumber, lineNumType errorLine)

  { /* print_line */
    logFunction(printf("print_line(%u, %u) in_file.file_number=%u in_file.line=%u\n",
                       fileNumber, errorLine, in_file.file_number, in_file.line););
    if (in_file.name != NULL) {
      if (fileNumber == in_file.file_number) {
        printLineOfCurrentFile(errorLine);
      } else {
        printLineOfOtherFile(get_file_name(fileNumber), errorLine);
      } /* if */
    } /* if */
    logFunction(printf("print_line -->\n"););
  } /* print_line */



static void print_error_line (void)

  {
    long current_position;
    long line_start_position;
    long buffer_start_position;
    ucharType buffer[BUFFER_SIZE];
    boolType searchNewLine = TRUE;
    int chars_to_read;
    int chars_in_buffer;
    int start;
    int ch;

  /* print_error_line */
    logFunction(printf("print_error_line\n"););
    if (in_file.name != NULL && in_file.curr_infile != NULL &&
        (current_position = IN_FILE_TELL()) >= 0L) {
      if (current_position >= BUFFER_SIZE + 1) {
        chars_to_read = BUFFER_SIZE;
        buffer_start_position = current_position - BUFFER_SIZE - 1;
      } else {
        chars_to_read = (int) current_position - 1;
        buffer_start_position = 0;
      } /* if */
      do {
        /* printf("buffer_start_position: %ld\n", buffer_start_position);
           printf("chars_to_read: %d\n", chars_to_read); */
        IN_FILE_SEEK(buffer_start_position);
        chars_in_buffer = 0;
        while (chars_in_buffer < chars_to_read && (ch = next_character()) != EOF) {
          buffer[chars_in_buffer] = (ucharType) ch;
          chars_in_buffer++;
        } /* if */
        /* prot_cstri("buf: ");
        fwrite(buffer, 1, (size_t) chars_in_buffer, stdout);
        prot_nl(); */
        start = chars_in_buffer - 1;
        while (start >= 0 && buffer[start] != '\n' &&
            buffer[start] != '\r') {
          /* printf("buffer[%d]=%d\n", start, buffer[start]); */
          start--;
        } /* while */
        if (start < 0 && buffer_start_position != 0) {
          if (buffer_start_position >= BUFFER_SIZE) {
            chars_to_read = BUFFER_SIZE;
            buffer_start_position -= BUFFER_SIZE;
          } else {
            chars_to_read = (int) buffer_start_position;
            buffer_start_position = 0;
          } /* if */
        } else {
          searchNewLine = FALSE;
        } /* if */
      } while (searchNewLine);
      line_start_position = buffer_start_position + start + 1;
      IN_FILE_SEEK(line_start_position);
      read_and_print_line(line_start_position, current_position);
      IN_FILE_SEEK(current_position);
    } /* if */
    logFunction(printf("print_error_line -->\n"););
  } /* print_error_line */



static void write_place (errorType err, const striType name, const lineNumType line)

  { /* write_place */
    prot_cstri("*** ");
    prot_string(name);
    prot_cstri("(");
    prot_int((intType) line);
    prot_cstri("):");
    prot_int((intType) err + 1);
    prot_cstri(": ");
  } /* write_place */



void place_of_error (errorType err)

  { /* place_of_error */
/*  print_error_line(); */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    if (in_file.name != NULL) {
      write_place(err, in_file.name, in_file.line);
    } else {
      prot_cstri("*** ");
    } /* if */
  } /* place_of_error */



static void undef_err (void)

  { /* undef_err */
    prot_cstri("Undefined error");
    prot_nl();
  } /* undef_err */



static void write_symbol (void)

  { /* write_symbol */
    if (symbol.sycategory == PARENSYMBOL) {
      prot_cstri(" \"");
      prot_uchar(symbol.name[0]);
      prot_cstri("\"");
      prot_nl();
    } else if (symbol.sycategory == INTLITERAL) {
      prot_cstri(" \"");
      prot_int(symbol.intValue);
      prot_cstri("\"");
      prot_nl();
    } else if (symbol.sycategory == BIGINTLITERAL) {
      prot_cstri(" \"");
      prot_ustri(symbol.name);
      prot_cstri("_\"");
      prot_nl();
    } else if (symbol.sycategory == CHARLITERAL) {
      prot_cstri(" ");
      prot_char(symbol.charValue);
      prot_nl();
    } else if (symbol.sycategory == STRILITERAL) {
      prot_cstri(" ");
      prot_stri(symbol.striValue);
      prot_nl();
#if WITH_FLOAT
    } else if (symbol.sycategory == FLOATLITERAL) {
      prot_cstri(" \"");
      prot_float(symbol.floatValue);
      prot_cstri("\"");
      prot_nl();
#endif
    } else {
      prot_cstri(" \"");
      prot_ustri(symbol.name);
      prot_cstri("\"");
      prot_nl();
    } /* if */
  } /* write_symbol */



static void write_type (const_typeType anytype)

  { /* write_type */
    if (anytype != NULL) {
      if (anytype->name != NULL) {
        prot_ustri(anytype->name->entity->ident->name);
      } else if (anytype->result_type != NULL &&
          anytype->result_type->name != NULL) {
        prot_cstri("func ");
        prot_ustri(anytype->result_type->name->entity->ident->name);
      } else {
        prot_cstri(" *ANONYM_TYPE* ");
      } /* if */
    } else {
      prot_cstri(" *NULL_TYPE* ");
    } /* if */
  } /* write_type */



static void write_name_list (const_listType params)

  {
    const_listType param_list_end;
    objectType formal_param;
    int in_formal_param_list = 0;

  /* write_name_list */
    if (params != NULL) {
      param_list_end = params;
      while (param_list_end->next != NULL) {
        param_list_end = param_list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(params->obj) != SYMBOLOBJECT &&
          CATEGORY_OF_OBJ(param_list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(param_list_end->obj)) {
        prot_cstri(" ");
        prot_ustri(GET_ENTITY(param_list_end->obj)->ident->name);
      } else {
        param_list_end = NULL;
      } /* if */
    } else {
      param_list_end = NULL;
    } /* if */
    while (params != param_list_end) {
      if (CATEGORY_OF_OBJ(params->obj) == FORMPARAMOBJECT) {
        if (in_formal_param_list) {
          prot_cstri(", ");
        } else {
          prot_cstri(" (");
          in_formal_param_list = 1;
        } /* if */
        formal_param = params->obj->value.objValue;
        switch (CATEGORY_OF_OBJ(formal_param)) {
          case VALUEPARAMOBJECT:
            if (VAR_OBJECT(formal_param)) {
              prot_cstri("in var ");
            } else {
              prot_cstri("val ");
            } /* if */
            write_type(formal_param->type_of);
            if (HAS_ENTITY(formal_param)) {
              prot_cstri(": ");
              prot_ustri(GET_ENTITY(formal_param)->ident->name);
            } else {
              prot_cstri(" param");
            } /* if */
            break;
          case REFPARAMOBJECT:
            if (VAR_OBJECT(formal_param)) {
              prot_cstri("inout ");
            } else {
              prot_cstri("ref ");
            } /* if */
            write_type(formal_param->type_of);
            if (HAS_ENTITY(formal_param)) {
              prot_cstri(": ");
              prot_ustri(GET_ENTITY(formal_param)->ident->name);
            } else {
              prot_cstri(" param");
            } /* if */
            break;
          case TYPEOBJECT:
            if (HAS_ENTITY(formal_param)) {
              prot_cstri("attr ");
              prot_ustri(GET_ENTITY(formal_param)->ident->name);
            } else {
              prot_cstri("attr ");
              write_type(formal_param->value.typeValue);
            } /* if */
            break;
          default:
            prot_cstri("unknown formal ");
            trace1(formal_param);
            break;
        } /* switch */
      } else {
        if (in_formal_param_list) {
          prot_cstri(") ");
          in_formal_param_list = 0;
        } else {
          prot_cstri(" ");
        } /* if */
        switch (CATEGORY_OF_OBJ(params->obj)) {
          case SYMBOLOBJECT:
            if (HAS_ENTITY(params->obj)) {
              prot_ustri(GET_ENTITY(params->obj)->ident->name);
            } else {
              prot_cstri("*symbol*");
            } /* if */
            break;
          default:
            prot_cstri("unknown param ");
            trace1(params->obj);
            break;
        } /* switch */
      } /* if */
      params = params->next;
    } /* while */
    if (in_formal_param_list) {
      prot_cstri(") ");
    } /* if */
  } /* write_name_list */



static void write_object (objectType anyobject)

  { /* write_object */
    if (anyobject == NULL) {
      prot_cstri("(NULL)");
    } else {
      switch (CATEGORY_OF_OBJ(anyobject)) {
        case TYPEOBJECT:
          prot_cstri("type ");
          write_type(anyobject->value.typeValue);
          break;
        case VALUEPARAMOBJECT:
          prot_cstri("parameter (");
          if (VAR_OBJECT(anyobject)) {
            prot_cstri("in var ");
          } else {
            prot_cstri("val ");
          } /* if */
          write_type(anyobject->type_of);
          if (HAS_ENTITY(anyobject)) {
            prot_cstri(": ");
            prot_ustri(GET_ENTITY(anyobject)->ident->name);
          } else {
            prot_cstri(" param");
          } /* if */
          prot_cstri(")");
          break;
        case REFPARAMOBJECT:
          prot_cstri("parameter (");
          if (VAR_OBJECT(anyobject)) {
            prot_cstri("inout ");
          } else {
            prot_cstri("ref ");
          } /* if */
          write_type(anyobject->type_of);
          if (HAS_ENTITY(anyobject)) {
            prot_cstri(": ");
            prot_ustri(GET_ENTITY(anyobject)->ident->name);
          } else {
            prot_cstri(" param");
          } /* if */
          prot_cstri(")");
          break;
        case INTOBJECT:
        case CHAROBJECT:
        case STRIOBJECT:
        case FILEOBJECT:
        case FLOATOBJECT:
        case ARRAYOBJECT:
        case STRUCTOBJECT:
        case BLOCKOBJECT:
          if (VAR_OBJECT(anyobject)) {
            prot_cstri("variable ");
          } else {
            prot_cstri("constant ");
          } /* if */
          write_type(anyobject->type_of);
          prot_cstri(": ");
          printvalue(anyobject);
          break;
        case CALLOBJECT:
        case MATCHOBJECT:
          if (anyobject->value.listValue != NULL &&
              anyobject->value.listValue->obj != NULL) {
            prot_cstri("expression (");
            prot_list(anyobject->value.listValue->next);
            prot_cstri(") of type ");
            write_type(anyobject->value.listValue->obj->type_of);
          } /* if */
          break;
        default:
          if (HAS_ENTITY(anyobject)) {
            prot_cstri8(id_string(GET_ENTITY(anyobject)->ident));
          } else {
            printcategory(CATEGORY_OF_OBJ(anyobject));
            prot_cstri(" *NULL_ENTITY_OBJECT*");
          } /* if */
          break;
      } /* switch */
    } /* if */
  } /* write_object */



void err_warning (errorType err)

  { /* err_warning */
    place_of_error(err);
    switch (err) {
      case OUT_OF_HEAP_SPACE:
        prot_cstri("No more memory");
        prot_nl();
        break;
      case EOF_ENCOUNTERED:
        prot_cstri("\"END OF FILE\" encountered");
        prot_nl();
        break;
      case ILLEGALPRAGMA:
        prot_cstri("Illegal pragma");
        write_symbol();
        break;
      case WRONGSYSTEM:
        prot_cstri("Illegal system declaration");
        write_symbol();
        break;
      case NEGATIVEEXPONENT:
        prot_cstri("Negative exponent in integer literal");
        prot_nl();
        break;
      case CHAREXCEEDS:
        prot_cstri("Character literal exceeds source line");
        prot_nl();
        break;
      case STRINGEXCEEDS:
        prot_cstri("String literal exceeds source line");
        prot_nl();
        break;
      case WRONG_QUOTATION_REPRESENTATION:
        prot_cstri("Use \\\" instead of \"\" to represent \" in a string");
        prot_nl();
        break;
      case WRONG_PATH_DELIMITER:
        prot_cstri("Use / instead of \\ as path delimiter");
        prot_nl();
        break;
      case NAMEEXPECTED:
        prot_cstri("Name expected found");
        write_symbol();
        break;
      case STRI_EXPECTED:
        prot_cstri("String literal expected found");
        write_symbol();
        break;
      case ILLEGAL_ASSOCIATIVITY:
        prot_cstri("Associativity expected found");
        write_symbol();
        break;
      case EXPR_EXPECTED:
        prot_cstri("Expression expected found");
        write_symbol();
        break;
      case TWO_PARAMETER_SYNTAX:
        prot_cstri("Syntax with two parameters before operator is illegal");
        prot_nl();
        break;
      case EMPTY_SYNTAX:
        prot_cstri("Empty syntax declaration");
        prot_nl();
        break;
      case SYNTAX_DECLARED_TWICE:
        prot_cstri("Syntax declared twice");
        prot_nl();
        break;
      case DOT_EXPR_REQUESTED:
        prot_cstri("Dot expression requested as syntax description");
        prot_nl();
        break;
      case TYPE_EXPECTED:
        prot_cstri("Type expected found");
        write_symbol();
        break;
      case LITERAL_TYPE_UNDEFINED:
        prot_cstri("Undefined type for literal");
        write_symbol();
        break;
      case DOLLAR_VALUE_WRONG:
        prot_cstri("\"newtype\", \"subtype\", \"func\", \"enumlit\" or \"action\" expected found");
        write_symbol();
        break;
      case DOLLAR_TYPE_WRONG:
        prot_cstri("\"func\" or \"type\" expected found");
        write_symbol();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_warning */



void err_num_stri (errorType err, int num_found, int num_expected,
    const_ustriType stri)

  { /* err_num_stri */
    place_of_error(err);
    switch (err) {
      case FALSE_INFIX_PRIORITY:
        prot_cstri("\"");
        prot_ustri(stri);
        prot_cstri("\" redeclared with infix priority ");
        prot_int((intType) num_found);
        prot_cstri(" not ");
        prot_int((intType) num_expected);
        prot_nl();
        break;
      case FALSE_PREFIX_PRIORITY:
        prot_cstri("\"");
        prot_ustri(stri);
        prot_cstri("\" redeclared with prefix priority ");
        prot_int((intType) num_found);
        prot_cstri(" not ");
        prot_int((intType) num_expected);
        prot_nl();
        break;
      case WRONG_EXPR_PARAM_PRIORITY:
        prot_cstri("Priority ");
        prot_int((intType) num_expected);
        prot_cstri(" required for parameter after \"");
        prot_ustri(stri);
        prot_cstri("\" not ");
        prot_int((intType) num_found);
        prot_nl();
        break;
      case WRONG_PREFIX_PRIORITY:
        prot_cstri("Priority <= ");
        prot_int((intType) num_expected);
        prot_cstri(" expected found \"");
        prot_ustri(stri);
        prot_cstri("\" with priority ");
        prot_int((intType) num_found);
        prot_nl();
        break;
      case DOT_EXPR_ILLEGAL:
        prot_cstri("\"");
        prot_ustri(stri);
        prot_cstri("\" must have priority ");
        prot_int((intType) num_expected);
        prot_cstri(" not ");
        prot_int((intType) num_found);
        prot_cstri(" for dot expression");
        prot_nl();
        break;
      case ILLEGALBASEDDIGIT:
        prot_cstri("Illegal digit \"");
        prot_cchar((char) num_found);
        prot_cstri("\" in based integer \"");
        prot_int((intType) num_expected);
        prot_cstri("#");
        prot_ustri(stri);
        prot_cstri("\"");
        prot_nl();
        break;
      case CARD_BASED_TOO_BIG:
        prot_cstri("Based integer \"");
        prot_int((intType) num_expected);
        prot_cstri("#");
        prot_ustri(stri);
        prot_cstri("\" too big");
        prot_nl();
        break;
      case CARD_WITH_EXPONENT_TOO_BIG:
        prot_cstri("Integer \"");
        prot_int((intType) num_expected);
        prot_cstri("E");
        prot_ustri(stri);
        prot_cstri("\" too big");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_num_stri */



void err_ident (errorType err, const_identType ident)

  { /* err_ident */
    place_of_error(err);
    switch (err) {
      case PARAM_SPECIFIER_EXPECTED:
        prot_cstri("Parameter specifier expected found \"");
        prot_ustri(ident->name);
        prot_cstri("\"");
        prot_nl();
        break;
      case EXPECTED_SYMBOL:
        prot_cstri("\"");
        prot_ustri(ident->name);
        prot_cstri("\" expected found");
        write_symbol();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_ident */



void err_object (errorType err, const_objectType obj_found)

  { /* err_object */
    /* place_of_error(err); */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    if (obj_found != NULL && HAS_POSINFO(obj_found)){
      write_place(err, get_file_name(GET_FILE_NUM(obj_found)),
          GET_LINE_NUM(obj_found));
    } else if (in_file.name != NULL) {
      write_place(err, in_file.name, in_file.line);
    } else {
      prot_cstri("*** ");
    } /* if */
    switch (err) {
      case OBJTWICEDECLARED:
        if (GET_ENTITY(obj_found)->fparam_list == NULL) {
          prot_cstri("\"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\" declared twice");
        } else {
          write_name_list(GET_ENTITY(obj_found)->fparam_list);
          prot_cstri(" declared twice");
        } /* if */
        prot_nl();
        break;
      case OBJUNDECLARED:
        prot_cstri("\"");
        prot_ustri(GET_ENTITY(obj_found)->ident->name);
        prot_cstri("\" not declared");
        prot_nl();
        break;
      case PARAM_DECL_FAILED:
        prot_cstri("Declaration of parameter ");
        prot_list(obj_found->value.listValue->next);
        prot_cstri(" failed");
        prot_nl();
        break;
      case DECL_FAILED:
        if (GET_ENTITY(obj_found)->fparam_list == NULL) {
          prot_cstri("Declaration of \"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\" failed");
        } else {
          prot_cstri("Declaration of ");
          write_name_list(GET_ENTITY(obj_found)->fparam_list);
          prot_cstri(" failed");
        } /* if */
        prot_nl();
        break;
      case EXCEPTION_RAISED:
        if (obj_found != NULL && HAS_ENTITY(obj_found)) {
          prot_cstri("Exception \"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\" raised");
        } else {
          prot_cstri("Exception raised");
        } /* if */
        prot_nl();
        break;
      case IDENT_EXPECTED:
        prot_cstri("Identifier expected found ");
        switch (CATEGORY_OF_OBJ(obj_found)) {
          case INTOBJECT:
            prot_cstri("\"");
            prot_int(obj_found->value.intValue);
            prot_cstri("\"");
            break;
          case CHAROBJECT:
            prot_char(obj_found->value.charValue);
            break;
          case STRIOBJECT:
            prot_stri(obj_found->value.striValue);
            break;
#if WITH_FLOAT
          case FLOATOBJECT:
            prot_cstri("\"");
            prot_float(obj_found->value.floatValue);
            prot_cstri("\"");
            break;
#endif
          default:
            printcategory(CATEGORY_OF_OBJ(obj_found));
            prot_cstri(" ");
            trace1(obj_found);
            break;
        } /* switch */
        prot_nl();
        break;
      case NO_MATCH:
        prot_cstri("Match for ");
        prot_list_limited(obj_found->value.listValue, MAX_DEPTH_SHOWN);
        prot_cstri(" failed");
        prot_nl();
        break;
      case PROC_EXPECTED:
        prot_cstri("Procedure expected found ");
        printobject(obj_found);
        prot_cstri(" expression");
        prot_nl();
        break;
      case TYPE_EXPECTED:
        prot_cstri("Type expected found ");
        printobject(obj_found);
        prot_cstri(" ");
        prot_nl();
        break;
      case EVAL_TYPE_FAILED:
        prot_cstri("Evaluate type expression ");
        trace1(obj_found);
        prot_cstri(" failed");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (obj_found != NULL && HAS_POSINFO(obj_found)){
      print_line(GET_FILE_NUM(obj_found), GET_LINE_NUM(obj_found));
    } else {
      print_error_line();
    } /* if */
    display_compilation_info();
  } /* err_object */



void err_type (errorType err, const_typeType type_found)

  { /* err_type */
    place_of_error(err);
    switch (err) {
      case PROC_EXPECTED:
        prot_cstri("Procedure expected found ");
        printobject(type_found->match_obj);
        prot_cstri(" expression");
        prot_nl();
        break;
      case KIND_OF_IN_PARAM_UNDEFINED:
        prot_cstri("Kind of in-parameter (val or ref) unspecified for type \"");
        write_type(type_found);
        prot_cstri("\"");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_type */



void err_expr_type (errorType err, const_objectType expr_object,
    const_typeType type_found)

  {
    boolType hasPosInfo;
    fileNumType fileNumber;
    lineNumType errorLine;

  /* err_expr_type */
    /* place_of_error(err); */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    hasPosInfo = HAS_POSINFO(expr_object);
    if (hasPosInfo){
      fileNumber = GET_FILE_NUM(expr_object);
      errorLine = GET_LINE_NUM(expr_object);
      write_place(err, get_file_name(fileNumber), errorLine);
    } else if (in_file.name != NULL) {
      write_place(err, in_file.name, in_file.line);
    } else {
      prot_cstri("*** ");
    } /* if */
    switch (err) {
      case KIND_OF_IN_PARAM_UNDEFINED:
        prot_cstri("Kind of in-parameter (val or ref) unspecified for type \"");
        write_type(type_found);
        prot_cstri("\"");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (hasPosInfo){
      print_line(fileNumber, errorLine);
    } else {
      print_error_line();
    } /* if */
    display_compilation_info();
  } /* err_expr_type */



void err_expr_obj (errorType err, const_objectType expr_object,
    objectType obj_found)

  {
    boolType hasPosInfo;
    fileNumType fileNumber;
    lineNumType errorLine;

  /* err_expr_obj */
    /* place_of_error(err); */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    hasPosInfo = HAS_POSINFO(expr_object);
    if (hasPosInfo){
      fileNumber = GET_FILE_NUM(expr_object);
      errorLine = GET_LINE_NUM(expr_object);
      write_place(err, get_file_name(fileNumber), errorLine);
    } else if (in_file.name != NULL) {
      write_place(err, in_file.name, in_file.line);
    } else {
      prot_cstri("*** ");
    } /* if */
    switch (err) {
      case WRONGACCESSRIGHT:
        prot_cstri("Variable expected in ");
        prot_list_limited(expr_object->value.listValue->next, MAX_DEPTH_SHOWN);
        prot_cstri(" found ");
        write_object(obj_found);
        prot_nl();
        break;
      case DECL_FAILED:
        if (GET_ENTITY(obj_found)->fparam_list == NULL) {
          prot_cstri("Declaration of \"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\" failed");
        } else {
          prot_cstri("Declaration of ");
          write_name_list(GET_ENTITY(obj_found)->fparam_list);
          prot_cstri(" failed");
        } /* if */
        prot_nl();
        break;
      case EXCEPTION_RAISED:
        if (obj_found != NULL && HAS_ENTITY(obj_found)) {
          prot_cstri("Exception \"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\" raised");
        } else {
          prot_cstri("Exception raised");
        } /* if */
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (hasPosInfo){
      print_line(fileNumber, errorLine);
    } else {
      print_error_line();
    } /* if */
    display_compilation_info();
  } /* err_expr_obj */



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

  { /* err_match */
    if (!contains_match_err_flag(obj_found)) {
      /* place_of_error(err); */
      if (prog != NULL) {
        prog->error_count++;
      } /* if */
      if (HAS_POSINFO(obj_found)){
        write_place(err, get_file_name(GET_FILE_NUM(obj_found)),
            GET_LINE_NUM(obj_found));
      } else if (in_file.name != NULL) {
        write_place(err, in_file.name, in_file.line);
      } else {
        prot_cstri("*** ");
      } /* if */
      switch (err) {
        case NO_MATCH:
          prot_cstri("Match for ");
          prot_list_limited(obj_found->value.listValue, MAX_DEPTH_SHOWN);
          prot_cstri(" failed");
          prot_nl();
          break;
        case EXPR_EXPECTED:
          prot_cstri("Expression expected found \"");
          prot_ustri(GET_ENTITY(obj_found)->ident->name);
          prot_cstri("\"");
          prot_nl();
          break;
        default:
          undef_err();
          break;
      } /* switch */
      if (HAS_POSINFO(obj_found)){
        print_line(GET_FILE_NUM(obj_found), GET_LINE_NUM(obj_found));
      } else {
        print_error_line();
      } /* if */
      display_compilation_info();
    } /* if */
    SET_MATCH_ERR_FLAG(obj_found);
  } /* err_match */



void err_string (errorType err, const_ustriType stri)

  { /* err_string */
    place_of_error(err);
    switch (err) {
      case EXPECTED_SYMBOL:
        prot_cstri("\"");
        prot_ustri(stri);
        prot_cstri("\" expected found");
        write_symbol();
        break;
      case CARD_DECIMAL_TOO_BIG:
        prot_cstri("Integer \"");
        prot_ustri(stri);
        prot_cstri("\" too big");
        prot_nl();
        break;
      case CARD_EXPECTED:
        prot_cstri("Integer literal expected found \"");
        prot_ustri(stri);
        prot_cstri("\"");
        prot_nl();
        break;
      case FILENOTFOUND:
        prot_cstri("Include file \"");
        prot_ustri(stri);
        prot_cstri("\" not found");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_string */



void err_stri (errorType err, const_striType stri)

  { /* err_stri */
    place_of_error(err);
    switch (err) {
      case FILENOTFOUND:
        prot_cstri("Include file ");
        prot_stri(stri);
        prot_cstri(" not found");
        prot_nl();
        break;
      case WRONGACTION:
        prot_cstri("Illegal action ");
        prot_stri(stri);
        prot_nl();
        break;
      case WRONG_PATH_DELIMITER:
        prot_cstri("Use / instead of \\ as path delimiter in ");
        prot_stri(stri);
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_stri */



void err_integer (errorType err, intType number)

  { /* err_integer */
    place_of_error(err);
    switch (err) {
      case BASE2TO36ALLOWED:
        prot_cstri("Integer base \"");
        prot_int(number);
        prot_cstri("\" not between 2 and 36");
        prot_nl();
        break;
      case ILLEGAL_PRIORITY:
        prot_cstri("Statement priority \"");
        prot_int(number);
        prot_cstri("\" too big");
        prot_nl();
        break;
      case NUMERICAL_ESCAPE_TOO_BIG:
        prot_cstri("The numerical escape sequence \"\\");
        prot_int(number);
        prot_cstri(";\" is too big");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_integer */



void err_cchar (errorType err, int character)

  {
    char buffer[100];

  /* err_cchar */
    place_of_error(err);
    switch (err) {
      case CHAR_ILLEGAL:
        prot_cstri("Illegal character in text \"");
        break;
      case DIGITEXPECTED:
        prot_cstri("Digit expected found \"");
        break;
      case EXTDIGITEXPECTED:
        prot_cstri("Extended digit expected found \"");
        break;
      case APOSTROPHEXPECTED:
        prot_cstri("\"'\" expected found \"");
        break;
      case BACKSLASHEXPECTED:
        prot_cstri("String continuations should end with \"\\\" not \"");
        break;
      case WRONGNUMERICALESCAPE:
        prot_cstri("Numerical escape sequences should end with \";\" not \"");
        break;
      case STRINGESCAPE:
        prot_cstri("Illegal string escape \"\\");
        break;
      case UTF8_CONTINUATION_BYTE_EXPECTED:
        prot_cstri("UTF-8 continuation byte expected found \"");
        break;
      case UNEXPECTED_UTF8_CONTINUATION_BYTE:
        prot_cstri("Unexpected UTF-8 continuation byte found \"");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      sprintf(buffer, "%c\"\n", character);
    } else {
      sprintf(buffer, "\\%u;\" (U+%04x)\n", character, character);
    } /* if */
    prot_cstri(buffer);
    print_error_line();
    display_compilation_info();
  } /* err_cchar */



void err_char (errorType err, charType character)

  {
    char buffer[100];

  /* err_char */
    place_of_error(err);
    switch (err) {
      case CHAR_ILLEGAL:
        prot_cstri("Illegal character in text");
        break;
      case OVERLONG_UTF8_ENCODING:
        prot_cstri("Overlong UTF-8 encoding used for character");
        break;
      case UTF16_SURROGATE_CHAR_FOUND:
        prot_cstri("UTF-16 surrogate character found in UTF-8 encoding");
        break;
      case CHAR_NOT_UNICODE:
        prot_cstri("Non Unicode character found");
        break;
      case SOLITARY_UTF8_START_BYTE:
        prot_cstri("Solitary UTF-8 start byte found");
        break;
      case UTF16_BYTE_ORDER_MARK_FOUND:
        prot_cstri("UTF-16 byte order mark found");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      sprintf(buffer, " \"%c\"\n", (char) character);
    } else {
      sprintf(buffer, " \"\\%lu;\" (U+%04lx)\n",
              (unsigned long) character, (unsigned long) character);
    } /* if */
    prot_cstri(buffer);
    print_error_line();
    display_compilation_info();
  } /* err_char */



void err_at_line (errorType err, lineNumType line)

  { /* err_at_line */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    write_place(err, in_file.name, line);
    switch (err) {
      case COMMENTOPEN:
        prot_cstri("Unclosed comment");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_line(in_file.file_number, line);
    display_compilation_info();
  } /* err_at_line */



void err_undeclared (errorType err, fileNumType file_num,
    lineNumType line, const_ustriType stri)

  { /* err_undeclared */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    write_place(err, get_file_name(file_num), line);
    switch (err) {
      case OBJUNDECLARED:
        prot_cstri("\"");
        prot_ustri(stri);
        prot_cstri("\" not declared");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_undeclared */



void err_message (errorType err, const_striType stri)

  { /* err_message */
    if (prog != NULL) {
      prog->error_count++;
    } /* if */
    prot_cstri("*** ");
    switch (err) {
      case NO_SOURCEFILE:
        prot_cstri("File ");
        prot_stri(stri);
        prot_cstri(" not found");
        prot_nl();
        break;
      default:
        undef_err();
        break;
    } /* switch */
  } /* err_message */
