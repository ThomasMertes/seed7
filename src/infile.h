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
/*  File: seed7/src/infile.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures to open, close and read the source file.    */
/*                                                                  */
/*  The next_character macro is the key macro for all parsing       */
/*  operations. All read operations for source file(s) are based    */
/*  on next_character. All other macros and functions read from     */
/*  the source files(s) only via next_character. Note the two       */
/*  versions of the next_character macro for conventional and for   */
/*  mmap use. If sometimes wide characters shoud be used this is    */
/*  the right place to substitute a new wide-getc function here.    */
/*                                                                  */
/********************************************************************/

#ifdef USE_ALTERNATE_NEXT_CHARACTER
#define next_character()  (in_file.nextch >= in_file.byond ? fill_buf() : (int) *in_file.nextch++)
#define FILE_TELL()       (ftell(in_file.fil) + (in_file.nextch - in_file.byond))
#define FILE_SEEK(POS)    (fseek(in_file.fil, (POS), SEEK_SET), in_file.nextch = in_file.byond)
#define MEM_TELL()        (in_file.nextch - in_file.start)
#define MEM_SEEK(POS)     (in_file.nextch = in_file.start + (POS))
#ifdef USE_MMAP
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


#ifdef DO_INIT
infilrecord in_file = {
    NULL, NULL, NULL,
#ifdef USE_ALTERNATE_NEXT_CHARACTER
    NULL, NULL, NULL, FALSE,
#else
#ifdef USE_INFILE_BUFFER
    NULL,
#endif
#endif
    ' ', 0, 0, 0, TRUE};
#else
EXTERN infilrecord in_file;
#endif


EXTERN stritype file_path;


#ifdef ANSI_C

int fill_buf (void);
void open_infile (stritype, errinfotype *);
void close_infile (void);
void open_string (bstritype, errinfotype *);
void next_file (void);
int next_line (void);
ustritype file_name (filenumtype);
void find_include_file (stritype, errinfotype *);
void set_search_path (stritype);
void set_search_path2 (ustritype, memsizetype);

#else

int fill_buf ();
void open_infile ();
void close_infile ();
void open_string ();
void next_file ();
int next_line ();
ustritype file_name ();
void find_include_file ();
void set_search_path ();
void set_search_path2 ();

#endif
