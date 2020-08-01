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
/*  File: seed7/src/infile.h                                        */
/*  Changes: 1990 - 1994, 2013  Thomas Mertes                       */
/*  Content: Procedures to open, close and read the source file.    */
/*                                                                  */
/*  The next_character macro is the key macro for all parsing       */
/*  operations. All read operations for source file(s) are based    */
/*  on next_character. All other macros and functions read from     */
/*  the source files(s) only via next_character. Note the two       */
/*  versions of the next_character macro for conventional and for   */
/*  mmap use. If wide characters should be used this is the right   */
/*  place to substitute a new wide-getc function.                   */
/*                                                                  */
/********************************************************************/

#if USE_ALTERNATE_NEXT_CHARACTER
#define next_character()  (in_file.nextch >= in_file.beyond ? fill_buf() : (int) *in_file.nextch++)
#define FILE_TELL()       (ftell(in_file.fil) + (long) (in_file.nextch - in_file.beyond))
#define FILE_SEEK(POS)    (fseek(in_file.fil, (POS), SEEK_SET), in_file.nextch = in_file.beyond)
#define MEM_TELL()        ((long) (in_file.nextch - in_file.start))
#define MEM_SEEK(POS)     (in_file.nextch = in_file.start + (POS))
#if HAS_MMAP
#define IN_FILE_TELL()    MEM_TELL()
#define IN_FILE_SEEK(POS) MEM_SEEK(POS)
#else
#define IN_FILE_TELL()    (in_file.fil ? FILE_TELL():MEM_TELL())
#define IN_FILE_SEEK(POS) (in_file.fil ? FILE_SEEK(POS):MEM_SEEK(POS))
#endif
#else
#define next_character()  getc(in_file.fil)
#define IN_FILE_TELL()    (ftell(in_file.fil))
#define IN_FILE_SEEK(POS) (fseek(in_file.fil, (POS), SEEK_SET))
#endif


#define SKIP_SPACE(CH) do CH = next_character(); while (CH == ' ' || CH == '\t')
#define SKIP_CR_SP(CH) do CH = next_character(); while (CH == ' ' || CH == '\t' || CH == '\r')
#define SKIP_TO_NL(CH) do { CH = next_character(); } while (CH != '\n' && CH != EOF);

#define LARGE_INCR 16383


#ifdef DO_INIT
inFileRecord in_file = {
    NULL,       /* fil */
    NULL,       /* name_ustri */
    NULL,       /* name */
#if USE_ALTERNATE_NEXT_CHARACTER
    NULL,       /* start */
    NULL,       /* nextch */
    NULL,       /* beyond */
    0,          /* buffer_size */
#else
#if USE_INFILE_BUFFER
    NULL,       /* buffer */
#endif
#endif
    ' ',        /* character */
    NULL,       /* curr_infile */
    NULL,       /* up_infile */
    NULL,       /* next */
#if WITH_COMPILATION_INFO
    FALSE,      /* write_library_names */
    FALSE,      /* write_line_numbers */
#endif
    0,          /* line */
    0,          /* incr_message_line */
    LARGE_INCR, /* next_msg_line */
    0,          /* file_number */
    NULL,       /* owningProg */
    TRUE        /* end_of_file */
  };
#else
EXTERN inFileRecord in_file;
#endif


int fill_buf (void);
void open_infile (const_striType sourceFileName, boolType write_library_names,
    boolType write_line_numbers, errInfoType *err_info);
void close_infile (void);
void open_string (bstriType input_string, boolType write_library_names,
    boolType write_line_numbers, errInfoType *err_info);
void remove_prog_files (progType currentProg);
void next_file (void);
int next_line (void);
striType get_file_name (fileNumType file_num);
const_ustriType get_file_name_ustri (fileNumType file_num);
