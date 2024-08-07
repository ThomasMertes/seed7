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
fileRecord nullFileRecord = {NULL, 0, TRUE, TRUE};
fileRecord stdinFileRecord = {NULL, 0, TRUE, FALSE};
fileRecord stdoutFileRecord = {NULL, 0, FALSE, TRUE};
fileRecord stderrFileRecord = {NULL, 0, FALSE, TRUE};
#else
EXTERN fileRecord nullFileRecord;
EXTERN fileRecord stdinFileRecord;
EXTERN fileRecord stdoutFileRecord;
EXTERN fileRecord stderrFileRecord;
#endif

#define initFileType(aFile, readingOkay, writingOkay) \
    (aFile)->usage_count = 1; \
    (aFile)->readingAllowed = (readingOkay); \
    (aFile)->writingAllowed = (writingOkay);

int offsetSeek (cFileType aFile, const os_off_t anOffset, const int origin);
memSizeType remainingBytesInFile (cFileType aFile);
intType getFileLengthUsingSeek (cFileType aFile);
bigIntType getBigFileLengthUsingSeek (cFileType aFile);
bigIntType filBigLng (fileType aFile);
void filBigSeek (fileType aFile, const const_bigIntType big_position);
bigIntType filBigTell (fileType aFile);
void filClose (fileType aFile);
void filCpy (fileType *const dest, const fileType source);
fileType filCreate (const fileType source);
void filDestr (const fileType old_file);
boolType filEof (fileType inFile);
void filFlush (fileType outFile);
void filFree (fileType oldFile);
charType filGetcChkCtrlC (fileType inFile);
striType filGets (fileType inFile, intType length);
striType filGetsChkCtrlC (fileType inFile, intType length);
boolType filHasNext (fileType inFile);
boolType filHasNextChkCtrlC (fileType inFile);
striType filLineRead (fileType inFile, charType *terminationChar);
striType filLineReadChkCtrlC (fileType inFile, charType *terminationChar);
striType filLit (fileType aFile);
intType filLng (fileType aFile);
fileType filOpen (const const_striType path, const const_striType mode);
fileType filOpenNullDevice (void);
void filPclose (fileType aFile);
fileType filPopen (const const_striType command,
    const const_striType parameters, const const_striType mode);
void filPrint (const const_striType stri);
void filSeek (fileType aFile, intType file_position);
boolType filSeekable (fileType aFile);
void filSetbuf (fileType aFile, intType mode, intType size);
intType filTell (fileType aFile);
striType filTerminatedRead (fileType inFile, charType terminator,
    charType *terminationChar);
void filTruncate (fileType aFile, intType length);
striType filWordRead (fileType inFile, charType *terminationChar);
striType filWordReadChkCtrlC (fileType inFile, charType *terminationChar);
void filWrite (fileType outFile, const const_striType stri);
