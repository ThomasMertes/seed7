/********************************************************************/
/*                                                                  */
/*  setwpath.c   Set the search path (PATH variable) under Windows. */
/*  Copyright (C) 2014, 2017 - 2019  Thomas Mertes                  */
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
/*  Module: Setwpath                                                */
/*  File: seed7/src/setwpath.c                                      */
/*  Changes: 2014, 2017 - 2019  Thomas Mertes                       */
/*  Content: Set the search path (PATH variable) under Windows.     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wchar.h"
#if DIR_LIB == DIRENT_DIRECTORY
#include "dirent.h"
#elif DIR_LIB == DIRECT_DIRECTORY || DIR_LIB == DIRDOS_DIRECTORY || DIR_LIB == DIRWIN_DIRECTORY
#include "direct.h"
#endif
#include "windows.h"

#include "os_decls.h"

#ifdef RENAMED_POSIX_FUNCTIONS
#define chdir _chdir
#endif

#define BUFFER_LEN 4096

#define ENVIRONMENT L"System\\CurrentControlSet\\Control\\Session Manager\\Environment"
#define DESTINATION L"Path"

#define SET_PATH_FAILED  0
#define ALREADY_OKAY     1
#define CHANGE_DONE      2

/* The include files of some C compilers do not define DWORD_PTR. */
/* Instead of checking, if DWORD_PTR has been defined, we used memSizeType instead. */
#if POINTER_SIZE == 32
typedef UINT32TYPE memSizeType;
#elif POINTER_SIZE == 64
typedef UINT64TYPE memSizeType;
#endif


static void notifySettingChange (void)

  {
    memSizeType /* DWORD_PTR */ result;

  /* notifySettingChange */
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) L"Environment",
                        SMTO_NORMAL, 10000, (void *) &result);
  } /* notifySettingChange */


static int set_path (int add_to_path, wchar_t *directory, HKEY key, int read_only)

  {
    size_t directory_len;
    size_t value_len;
    DWORD value_type;
    DWORD size_of_value = 0;
    wchar_t *value_data;
    wchar_t *position;
    int save_value = 0;
    int success = SET_PATH_FAILED;

  /* set_path */
    directory_len = wcslen(directory);
    /* printf("directory: %ls\n", directory);
       printf("directory_len: %lu\n", directory_len); */
    if (RegQueryValueExW(key, DESTINATION, NULL, NULL, NULL,
                         &size_of_value) != ERROR_SUCCESS) {
      if (add_to_path) {
        if (!read_only) {
          size_of_value = (directory_len + 1) * sizeof(wchar_t);
          if (RegSetValueExW(key, DESTINATION, 0, REG_SZ, (BYTE *) directory,
                             size_of_value) != ERROR_SUCCESS) {
            printf(" *** Unable to set registry value.\n");
          } else {
            notifySettingChange();
            success = CHANGE_DONE;
          } /* if */
        } /* if */
      } /* if */
    } else {
      /* printf("size_of_value: %lu\n", size_of_value); */
      if ((value_data = (wchar_t *)
           malloc(size_of_value + (1 + directory_len) * sizeof(wchar_t))) == NULL) {
        printf(" *** Unable to request memory for the registry value.\n");
      } else {
        if (RegQueryValueExW(key, DESTINATION, NULL, &value_type, (LPBYTE) value_data,
                             &size_of_value) != ERROR_SUCCESS) {
          printf(" *** Unable to query the registry value.\n");
        } else {
          value_len = wcslen(value_data);
          /* printf("value_type: %lu\n", value_type);
             printf("size_of_value: %lu\n", size_of_value);
             printf("strlen(value_data): %lu\n", value_len);
             printf("value_data:\n%ls\n", value_data); */
          position = wcsstr(value_data, directory);
          while (position != NULL &&
              ((position != value_data && position[-1] != ';') ||
              (position[directory_len] != ';' && position[directory_len] != '\0'))) {
            position = wcsstr(&position[1], directory);
          } /* while */
          if (position != NULL) {
            if (add_to_path) {
              success = ALREADY_OKAY;
            } else {
              if (position != value_data) {
                if (position[-1] == ';' &&
                    (value_data[directory_len] == ';' || value_data[directory_len] == '\0')) {
                  memmove(&position[-1], &position[directory_len],
                          (&value_data[value_len + 1] - &position[directory_len]) *
                          sizeof(wchar_t));
                  size_of_value -= (1 + directory_len) * sizeof(wchar_t);
                  save_value = 1;
                } /* if */
              } else {
                if (value_data[directory_len] == ';') {
                  memmove(value_data, &value_data[directory_len + 1],
                          (&value_data[value_len + 1] - &value_data[directory_len + 1]) *
                          sizeof(wchar_t));
                  size_of_value -= (directory_len + 1) * sizeof(wchar_t);
                  save_value = 1;
                } else if (value_data[directory_len] == '\0') {
                  value_data[0] = '\0';
                  size_of_value -= (directory_len) * sizeof(wchar_t);
                  save_value = 1;
                } /* if */
              } /* if */
            } /* if */
          } else {
            if (add_to_path) {
              if (value_len == 0) {
                memcpy(value_data, directory,
                       (directory_len + 1) * sizeof(wchar_t));
                size_of_value += (directory_len) * sizeof(wchar_t);
              } else {
                value_data[value_len] = ';';  /* delimiter */
                memcpy(&value_data[value_len + 1], directory,
                       (directory_len + 1) * sizeof(wchar_t));
                size_of_value += (1 + directory_len) * sizeof(wchar_t);
              } /* if */
              save_value = 1;
            } else {
              success = ALREADY_OKAY;
            } /* if */
          } /* if */
          if (save_value) {
            /* printf("size_of_value: %lu\n", size_of_value);
               printf("strlen(value_data): %lu\n", wcslen(value_data));
               printf("value_data:\n%ls\n", value_data); */
            if (!read_only) {
              if (RegSetValueExW(key, DESTINATION, 0, value_type, (BYTE *) value_data,
                                 size_of_value) != ERROR_SUCCESS) {
                printf(" *** Unable to set registry value.\n");
              } else {
                notifySettingChange();
                success = CHANGE_DONE;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
        free(value_data);
      } /* if */
    } /* if */
    return success;
  } /* set_path */


static int set_win_path (int add_to_path, wchar_t *directory)

  {
    HKEY key;
    int key_open = 1;
    int read_only = 0;
    int success = SET_PATH_FAILED;

  /* set_win_path */
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, ENVIRONMENT, 0,
                      KEY_QUERY_VALUE | KEY_SET_VALUE, &key) != ERROR_SUCCESS) {
      if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, ENVIRONMENT, 0,
                        KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        key_open = 0;
      } else {
        read_only = 1;
      } /* if */
    } /* if */
    if (key_open) {
      success = set_path(add_to_path, directory, key, read_only);
      RegCloseKey(key);
    } /* if */
    return success;
  } /* set_win_path */


static int set_usr_path (int add_to_path, wchar_t *directory)

  {
    HKEY key;
    int key_open = 1;
    int read_only = 0;
    int success = SET_PATH_FAILED;

  /* set_usr_path */
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Environment", 0,
                      KEY_QUERY_VALUE | KEY_SET_VALUE, &key) != ERROR_SUCCESS) {
      if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Environment", 0,
                        KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        key_open = 0;
      } else {
        read_only = 1;
      } /* if */
    } /* if */
    if (key_open) {
      success = set_path(add_to_path, directory, key, read_only);
      RegCloseKey(key);
    } /* if */
    return success;
  } /* set_usr_path */


int main (int argc, char *argv[])

  {
    wchar_t directory[BUFFER_LEN];
    int win_success;
    int usr_success;

  /* main */
    if (argc != 3) {
      printf(" *** Wrong number of parameters.\n");
      printf("Usage: setwpath [add | remove] directory\n");
    } else {
      /* printf("dir: %s\n", argv[2]); */
      chdir(argv[2]);
      os_getcwd(directory, BUFFER_LEN);
      if (strcmp(argv[1], "add") == 0) {
        win_success = set_win_path(1, directory);
        if (win_success == ALREADY_OKAY) {
          printf("The directory  %ls\n", directory);
          printf("is already in the search path (PATH variable).\n");
        } else if (win_success == CHANGE_DONE) {
          printf("The directory  %ls\n", directory);
          printf("has been added to the search path (PATH variable).\n");
          printf("You need to start a new console "
                 "to see the effect of this change.\n");
        } else {
          usr_success = set_usr_path(1, directory);
          if (usr_success == ALREADY_OKAY) {
            printf("The directory  %ls\n", directory);
            printf("is already in the search path (PATH variable).\n");
          } else if (usr_success == CHANGE_DONE) {
            printf("The directory  %ls\n", directory);
            printf("has been added to the search path (PATH variable).\n");
            printf("You need to start a new console "
                   "to see the effect of this change.\n");
          } else {
            printf(" *** You need administrator rights to change the search path "
                   "(PATH variable).\n");
          } /* if */
        } /* if */
      } else if (strcmp(argv[1], "remove") == 0) {
        win_success = set_win_path(0, directory);
        usr_success = set_usr_path(0, directory);
        if (win_success == ALREADY_OKAY && usr_success == ALREADY_OKAY) {
          printf("The directory  %ls\n", directory);
          printf("is not part of the search path (PATH variable).\n");
        } else if (win_success == CHANGE_DONE || usr_success == CHANGE_DONE) {
          printf("The directory  %ls\n", directory);
          printf("has been removed from the search path (PATH variable).\n");
          printf("You need to start a new console "
                 "to see the effect of this change.\n");
        } else {
          printf(" *** You need administrator rights to change the search path "
                 "(PATH variable).\n");
        } /* if */
      } else {
        printf(" *** Neither 'add' nor 'remove' was specified.\n");
      } /* if */
    } /* if */
    return 0;
  } /* main */
