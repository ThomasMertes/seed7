/********************************************************************/
/*                                                                  */
/*  setpaths   Write definitions for Seed7 specific paths.          */
/*  Copyright (C) 2011  Thomas Mertes                               */
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
/*  Module: Setpaths                                                */
/*  File: seed7/src/setpaths.c                                      */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Program to write definitions for Seed7 specific paths. */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "ctype.h"

#ifdef USE_DIRENT
#include "dirent.h"
#endif
#if defined USE_DIRECT || defined USE_DIRDOS || defined USE_DIRWIN
#include "direct.h"
#endif
#ifdef OS_STRI_USES_CODEPAGE
#include "dos.h"
#endif
#ifdef UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "os_decls.h"

/**
 *  Some definitions from version.h are used:
 *
 *  OS_STRI_WCHAR
 *      System calls (os_...) use wide characters.
 *  OS_STRI_USES_CODEPAGE
 *      System calls (os_...) use characters (type char)
 *      encoded with a codepage (only the codepages
 *      437 and 850 are supported).
 *  os_getcwd
 *      Copy the current working directory to a buffer.
 *      E.g.: #define os_getcwd _wgetcwd
 */

#ifdef OS_STRI_WCHAR
#define os_chartype wchar_t
#else
#define os_chartype char
#endif

#define BUFFER_LEN 4096

#ifdef OS_STRI_USES_CODEPAGE
int codepage;
#endif



void get_cwd_to_buffer (os_chartype *buffer)

  {
    int position;

  /* get_cwd_to_buffer */
    os_getcwd(buffer, BUFFER_LEN);
    for (position = 0; buffer[position] != '\0'; position++) {
      if (buffer[position] == '\\') {
        buffer[position] = '/';
      } /* if */
    } /* for */
    if (position >= 2 && buffer[position - 1] == '/') {
      buffer[position - 1] = '\0';
    } /* if */
    if (((buffer[0] >= 'a' && buffer[0] <= 'z') ||
         (buffer[0] >= 'A' && buffer[0] <= 'Z')) &&
        buffer[1] == ':') {
      buffer[1] = tolower(buffer[0]);
      buffer[0] = '/';
    } /* if */
  } /* get_cwd_to_buffer */



#ifdef OS_STRI_WCHAR

void write_as_utf8 (os_chartype *wstri)

  {
    unsigned long utf32;

  /* write_as_utf8 */
    for (; *wstri != '\0'; wstri++) {
      if (*wstri <= 0x7F) {
        putchar((unsigned char) *wstri);
      } else if (*wstri <= 0x7FF) {
        putchar((unsigned char) (0xC0 | (*wstri >>  6)));
        putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));
      } else if (*wstri >= 0xD800 && *wstri <= 0xDBFF &&
                 wstri[1] >= 0xDC00 && wstri[1] <= 0xDFFF) {
        utf32 = ((((unsigned long) *wstri   - 0xD800) << 10) +
                  ((unsigned long) wstri[1] - 0xDC00) + 0x10000);
        wstri++;
        putchar((unsigned char) (0xF0 | (utf32 >> 18)));
        putchar((unsigned char) (0x80 |((utf32 >> 12) & 0x3F)));
        putchar((unsigned char) (0x80 |((utf32 >>  6) & 0x3F)));
        putchar((unsigned char) (0x80 |( utf32        & 0x3F)));
      } else {
        putchar((unsigned char) (0xE0 | (*wstri >> 12)));
        putchar((unsigned char) (0x80 |((*wstri >>  6) & 0x3F)));
        putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));
      } /* if */
    } /* for */
  } /* write_as_utf8 */



#else
#ifdef OS_STRI_USES_CODEPAGE

char *conv_437[] = {
    "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\007",
    "\\b", "\\t", "\\n", "\\013", "\\f", "\\r", "\\016", "\\017",
    "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
    "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
    " ", "!", "\\\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\\177",
    "\\303\\207", "\\303\\274", "\\303\\251", "\\303\\242",
    "\\303\\244", "\\303\\240", "\\303\\245", "\\303\\247",
    "\\303\\252", "\\303\\253", "\\303\\250", "\\303\\257",
    "\\303\\256", "\\303\\254", "\\303\\204", "\\303\\205",
    "\\303\\211", "\\303\\246", "\\303\\206", "\\303\\264",
    "\\303\\266", "\\303\\262", "\\303\\273", "\\303\\271",
    "\\303\\277", "\\303\\226", "\\303\\234", "\\302\\242",
    "\\302\\243", "\\302\\245", "\\342\\202\\247", "\\306\\222",
    "\\303\\241", "\\303\\255", "\\303\\263", "\\303\\272",
    "\\303\\261", "\\303\\221", "\\302\\252", "\\302\\272",
    "\\302\\277", "\\342\\214\\220", "\\302\\254", "\\302\\275",
    "\\302\\274", "\\302\\241", "\\302\\253", "\\302\\273",
    "\\342\\226\\221", "\\342\\226\\222", "\\342\\226\\223", "\\342\\224\\202",
    "\\342\\224\\244", "\\342\\225\\241", "\\342\\225\\242", "\\342\\225\\226",
    "\\342\\225\\225", "\\342\\225\\243", "\\342\\225\\221", "\\342\\225\\227",
    "\\342\\225\\235", "\\342\\225\\234", "\\342\\225\\233", "\\342\\224\\220",
    "\\342\\224\\224", "\\342\\224\\264", "\\342\\224\\254", "\\342\\224\\234",
    "\\342\\224\\200", "\\342\\224\\274", "\\342\\225\\236", "\\342\\225\\237",
    "\\342\\225\\232", "\\342\\225\\224", "\\342\\225\\251", "\\342\\225\\246",
    "\\342\\225\\240", "\\342\\225\\220", "\\342\\225\\254", "\\342\\225\\247",
    "\\342\\225\\250", "\\342\\225\\244", "\\342\\225\\245", "\\342\\225\\231",
    "\\342\\225\\230", "\\342\\225\\222", "\\342\\225\\223", "\\342\\225\\253",
    "\\342\\225\\252", "\\342\\224\\230", "\\342\\224\\214", "\\342\\226\\210",
    "\\342\\226\\204", "\\342\\226\\214", "\\342\\226\\220", "\\342\\226\\200",
    "\\316\\261", "\\303\\237", "\\316\\223", "\\317\\200",
    "\\316\\243", "\\317\\203", "\\302\\265", "\\317\\204",
    "\\316\\246", "\\316\\230", "\\316\\251", "\\316\\264",
    "\\342\\210\\236", "\\317\\206", "\\316\\265", "\\342\\210\\251",
    "\\342\\211\\241", "\\302\\261", "\\342\\211\\245", "\\342\\211\\244",
    "\\342\\214\\240", "\\342\\214\\241", "\\303\\267", "\\342\\211\\210",
    "\\302\\260", "\\342\\210\\231", "\\302\\267", "\\342\\210\\232",
    "\\342\\201\\277", "\\302\\262", "\\342\\226\\240", "\\302\\240"};

char *conv_850[] = {
    "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\007",
    "\\b", "\\t", "\\n", "\\013", "\\f", "\\r", "\\016", "\\017",
    "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
    "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
    " ", "!", "\\\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\\177",
    "\\303\\207", "\\303\\274", "\\303\\251", "\\303\\242",
    "\\303\\244", "\\303\\240", "\\303\\245", "\\303\\247",
    "\\303\\252", "\\303\\253", "\\303\\250", "\\303\\257",
    "\\303\\256", "\\303\\254", "\\303\\204", "\\303\\205",
    "\\303\\211", "\\303\\246", "\\303\\206", "\\303\\264",
    "\\303\\266", "\\303\\262", "\\303\\273", "\\303\\271",
    "\\303\\277", "\\303\\226", "\\303\\234", "\\303\\270",
    "\\302\\243", "\\303\\230", "\\303\\227", "\\306\\222",
    "\\303\\241", "\\303\\255", "\\303\\263", "\\303\\272",
    "\\303\\261", "\\303\\221", "\\302\\252", "\\302\\272",
    "\\302\\277", "\\302\\256", "\\302\\254", "\\302\\275",
    "\\302\\274", "\\302\\241", "\\302\\253", "\\302\\273",
    "\\342\\226\\221", "\\342\\226\\222", "\\342\\226\\223", "\\342\\224\\202",
    "\\342\\224\\244", "\\303\\201", "\\303\\202", "\\303\\200",
    "\\302\\251", "\\342\\225\\243", "\\342\\225\\221", "\\342\\225\\227",
    "\\342\\225\\235", "\\302\\242", "\\302\\245", "\\342\\224\\220",
    "\\342\\224\\224", "\\342\\224\\264", "\\342\\224\\254", "\\342\\224\\234",
    "\\342\\224\\200", "\\342\\224\\274", "\\303\\243", "\\303\\203",
    "\\342\\225\\232", "\\342\\225\\224", "\\342\\225\\251", "\\342\\225\\246",
    "\\342\\225\\240", "\\342\\225\\220", "\\342\\225\\254", "\\302\\244",
    "\\303\\260", "\\303\\220", "\\303\\212", "\\303\\213",
    "\\303\\210", "\\304\\261", "\\303\\215", "\\303\\216",
    "\\303\\217", "\\342\\224\\230", "\\342\\224\\214", "\\342\\226\\210",
    "\\342\\226\\204", "\\302\\246", "\\303\\214", "\\342\\226\\200",
    "\\303\\223", "\\303\\237", "\\303\\224", "\\303\\222",
    "\\303\\265", "\\303\\225", "\\302\\265", "\\303\\276",
    "\\303\\236", "\\303\\232", "\\303\\233", "\\303\\231",
    "\\303\\275", "\\303\\235", "\\302\\257", "\\302\\264",
    "\\302\\255", "\\302\\261", "\\342\\200\\227", "\\302\\276",
    "\\302\\266", "\\302\\247", "\\303\\267", "\\302\\270",
    "\\302\\260", "\\302\\250", "\\302\\267", "\\302\\271",
    "\\302\\263", "\\302\\262", "\\342\\226\\240", "\\302\\240"};



void write_as_utf8 (os_chartype *cstri)

  { /* write_as_utf8 */
    if (codepage == 437) {
      for (; *cstri != '\0'; cstri++) {
        printf("%s", conv_437[*cstri]);
      } /* for */
    } else if (codepage == 850) {
      for (; *cstri != '\0'; cstri++) {
        printf("%s", conv_850[*cstri]);
      } /* for */
    } else {
      printf("%s", cstri);
    } /* if */
  } /* write_as_utf8 */

#else



void write_as_utf8 (os_chartype *cstri)

  { /* write_as_utf8 */
    printf("%s", cstri);
  } /* write_as_utf8 */

#endif
#endif



#ifdef OS_STRI_USES_CODEPAGE
int get_codepage (void)

  {
    union REGS r;
    int codepage;

  /* get_codepage */
    r.h.ah = (unsigned char) 0x66;
    r.h.al = (unsigned char) 0x01;
    int86(0x21, &r, &r);
    codepage = r.w.bx;
    return codepage;
  } /* get_codepage */
#endif



int main (int argc, char **argv)

  {
    char **curr_arg;
    int found;
    os_chartype buffer[BUFFER_LEN];

  /* main */
#ifdef OS_STRI_USES_CODEPAGE
    codepage = get_codepage();
    printf("#define DEFAULT_CODEPAGE %d\n", codepage);
#endif
    chdir("../bin");
    printf("#define S7_LIB_DIR \"");
    found = 0;
    for (curr_arg = argv; *curr_arg != NULL; curr_arg++) {
      if (memcmp(*curr_arg, "S7_LIB_DIR=", 11 * sizeof(char)) == 0 &&
          (*curr_arg)[11] != '\0') {
        found = 1;
        printf("%s", &(*curr_arg)[11]);
      } /* if */
    } /* for */
    if (!found) {
      get_cwd_to_buffer(buffer);
      write_as_utf8(buffer);
    } /* if */
    printf("\"\n");
    chdir("../prg"); /* Use ../prg when ../lib does not exist */
    chdir("../lib");
    printf("#define SEED7_LIBRARY \"");
    found = 0;
    for (curr_arg = argv; *curr_arg != NULL; curr_arg++) {
      if (memcmp(*curr_arg, "SEED7_LIBRARY=", 14 * sizeof(char)) == 0 &&
          (*curr_arg)[14] != '\0') {
        found = 1;
        printf("%s", &(*curr_arg)[14]);
      } /* if */
    } /* for */
    if (!found) {
      get_cwd_to_buffer(buffer);
      write_as_utf8(buffer);
    } /* if */
    printf("\"\n");
    /* A DOS subprocess has not its own current working directory.  */
    /* A DOS chdir() changes also the current working directory of  */
    /* the calling process. Therefore it is necessary to change     */
    /* back to the original dir (Make calls setpaths from src).     */
    chdir("../src");
    return 0;
  } /* main */
