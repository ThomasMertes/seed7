/********************************************************************/
/*                                                                  */
/*  fil_rtl.h     Primitive actions for the C library file type.    */
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
/*  File: seed7/src/fil_rtl.h                                       */
/*  Changes: 1992, 1993, 1994, 2009  Thomas Mertes                  */
/*  Content: Primitive actions for the C library file type.         */
/*                                                                  */
/********************************************************************/

#ifdef DO_INIT
fileRecord nullFileRecord = {NULL, 0, TRUE, TRUE, FALSE};
fileRecord stdinFileRecord = {NULL, 0, TRUE, FALSE, FALSE};
fileRecord stdoutFileRecord = {NULL, 0, FALSE, TRUE, FALSE};
fileRecord stderrFileRecord = {NULL, 0, FALSE, TRUE, FALSE};
#else
EXTERN fileRecord nullFileRecord;
EXTERN fileRecord stdinFileRecord;
EXTERN fileRecord stdoutFileRecord;
EXTERN fileRecord stderrFileRecord;
#endif

#define initFileType(aFile, readingOkay, writingOkay) \
    (aFile)->usage_count = 1; \
    (aFile)->readingAllowed = (readingOkay); \
    (aFile)->writingAllowed = (writingOkay); \
    (aFile)->isPipe = FALSE;

#define initPipeType(aFile, readingOkay, writingOkay) \
    (aFile)->usage_count = 1; \
    (aFile)->readingAllowed = (readingOkay); \
    (aFile)->writingAllowed = (writingOkay); \
    (aFile)->isPipe = TRUE;

int offsetSeek (cFileType aFile, const os_off_t anOffset,
                const int origin);
memSizeType remainingBytesInFile (cFileType aFile);
intType getFileLengthUsingSeek (cFileType aFile);
bigIntType getBigFileLengthUsingSeek (cFileType aFile);
bigIntType filBigLng (const const_fileType aFile);
void filBigSeek (const const_fileType aFile,
                 const const_bigIntType big_position);
bigIntType filBigTell (const const_fileType aFile);
void filClose (const fileType aFile);
void filCpy (fileType *const dest, const fileType source);
fileType filCreate (const fileType source);
void filDestr (const fileType oldFile);
boolType filEof (const const_fileType inFile);
void filFlush (const const_fileType outFile);
void filFree (const fileType oldFile);
charType filGetcChkCtrlC (const const_fileType inFile);
striType filGets (const const_fileType inFile, intType length);
striType filGetsChkCtrlC (const const_fileType inFile, intType length);
boolType filHasNext (const const_fileType inFile);
boolType filHasNextChkCtrlC (const const_fileType inFile);
striType filLineRead (const const_fileType inFile,
                      charType *const terminationChar);
striType filLineReadChkCtrlC (const const_fileType inFile,
    charType *const terminationChar);
striType filLiteral (const const_fileType aFile);
intType filLng (const const_fileType aFile);
fileType filOpen (const const_striType path, const const_striType mode);
fileType filOpenNullDevice (void);
fileType filPopen (const const_striType command,
    const const_rtlArrayType parameters, const const_striType mode);
void filPrint (const const_striType stri);
void filSeek (const const_fileType aFile, intType file_position);
boolType filSeekable (const const_fileType aFile);
intType filTell (const const_fileType aFile);
striType filTerminatedRead (const const_fileType inFile,
                            charType terminator,
                            charType *const terminationChar);
void filTruncate (const const_fileType aFile, intType length);
striType filWordRead (const const_fileType inFile,
                      charType *const terminationChar);
striType filWordReadChkCtrlC (const const_fileType inFile,
    charType *const terminationChar);
void filWrite (const const_fileType outFile, const const_striType stri);
