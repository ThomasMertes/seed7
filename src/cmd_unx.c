/********************************************************************/
/*                                                                  */
/*  cmd_unx.c     Command functions which call the Unix API.        */
/*  Copyright (C) 1989 - 2016, 2018 - 2021  Thomas Mertes           */
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
/*  File: seed7/src/cmd_unx.c                                       */
/*  Changes: 2010 - 2016, 2018 - 2021  Thomas Mertes                */
/*  Content: Command functions which call the Unix API.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#define _XOPEN_SOURCE 500
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "signal.h"
#ifdef EMULATE_NODE_ENVIRONMENT
#include "emscripten.h"
#endif
#include "errno.h"

#if UNISTD_H_PRESENT
#include "unistd.h"
#endif

#if HAS_GETGRGID_R || HAS_GETGRGID || HAS_GETGRNAM_R || HAS_GETGRNAM
#include "grp.h"
#endif

#if HAS_GETPWUID_R || HAS_GETPWUID || HAS_GETPWNAM_R || HAS_GETPWNAM
#include "pwd.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "arr_rtl.h"
#include "cmd_rtl.h"
#include "str_rtl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"


#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#ifdef EMULATE_ENVIRONMENT
char **environ7 = NULL;

#ifndef DETERMINE_OS_PROPERTIES_AT_RUNTIME
#define environmentStrncmp strncmp
#endif
#endif



/**
 *  Get the absolute path of the executable of the current process.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the absolute path of the current process.
 */
striType getExecutablePath (const const_striType arg_0)

  {
#if HAS_SYMBOLIC_LINKS
    striType procSelfExe;
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
    striType exeExtension;
#endif
#endif
    striType cwd;
    errInfoType err_info = OKAY_NO_ERROR;
    striType executablePath = NULL;

  /* getExecutablePath */
    logFunction(printf("getExecutablePath(\"%s\")\n", striAsUnquotedCStri(arg_0)););
#if HAS_SYMBOLIC_LINKS
    procSelfExe = CSTRI_LITERAL_TO_STRI("/proc/self/exe");
    if (likely(procSelfExe != NULL)) {
      executablePath = doReadLink(procSelfExe, &err_info);
      FREE_STRI(procSelfExe, procSelfExe->size);
      if (executablePath != NULL) {
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
        exeExtension = CSTRI_LITERAL_TO_STRI(EXECUTABLE_FILE_EXTENSION);
        if (likely(exeExtension != NULL)) {
          strAppendTemp(&executablePath, exeExtension);
        } else {
          FREE_STRI(executablePath, executablePath->size);
          executablePath = NULL;
        } /* if */
#endif
      } /* if */
    } /* if */
    if (executablePath == NULL) {
      err_info = OKAY_NO_ERROR;
#endif
      if (strChPos(arg_0, (charType) '/') == 0) {
        executablePath = examineSearchPath(arg_0);
      } else if (arg_0->size >= 1 && arg_0->mem[0] == (charType) '/') {
        executablePath = straightenAbsolutePath(arg_0);
      } else {
        cwd = doGetCwd(&err_info);
        if (unlikely(cwd == NULL)) {
          raise_error(err_info);
        } else {
          executablePath = concatPath(cwd, arg_0);
          FREE_STRI(cwd, cwd->size);
        } /* if */
      } /* if */
      if (unlikely(executablePath == NULL)) {
        raise_error(MEMORY_ERROR);
#if HAS_SYMBOLIC_LINKS
      } else {
        executablePath = followLink(executablePath, &err_info);
        if (unlikely(executablePath == NULL)) {
          raise_error(err_info);
        } /* if */
#endif
      } /* if */
#if HAS_SYMBOLIC_LINKS
    } /* if */
#endif
    logFunction(printf("getExecutablePath --> \"%s\"\n",
                       striAsUnquotedCStri(executablePath)););
    return executablePath;
  } /* getExecutablePath */



#ifdef EMULATE_ENVIRONMENT
char *getenv7 (const char *name)

  {
    size_t nameLen;
    char **p, *c;

  /* getenv7 */
    if (name == NULL || environ7 == NULL || strchr(name, '=') != NULL) {
      return NULL;
    } else {
      nameLen = strlen(name);
      for (p = environ7; (c = *p) != NULL; ++p) {
        if (environmentStrncmp(c, name, nameLen) == 0 && c[nameLen] == '=') {
          /* printf("getenv7(\"%s\") --> \"%s\"\n", name, &c[nameLen + 1]); */
          return &c[nameLen + 1];
        } /* if */
      } /* for */
      return NULL;
    } /* if */
  } /* getenv7 */



int setenv7 (const char *name, const char *value, int overwrite)

  {
    size_t nameLen;
    char **p, *c;
    size_t nameCount = 0;

  /* setenv7 */
    logFunction(printf("setenv7(\"%s\", \"%s\", %d)\n", name, value, overwrite););
    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
      errno = EINVAL;
      return -1;
    } else {
      nameLen = strlen(name);
      if (environ7 != NULL) {
        for (p = environ7; (c = *p) != NULL; ++p) {
          nameCount++;
          if (environmentStrncmp(c, name, nameLen) == 0 && c[nameLen] == '=') {
            if (overwrite) {
              if ((c = realloc(*p, nameLen + strlen(value) + 2)) == NULL) {
                errno = ENOMEM;
                return -1;
              } else {
                *p = c;
                strcpy(&c[nameLen + 1], value);
              } /* if */
            } /* if **/
            return 0;
          } /* if */
        } /* for */
      } /* if */
      if ((environ7 = realloc(environ7, sizeof(char *) * (nameCount + 2))) == NULL ||
          (c = malloc(nameLen + strlen(value) + 2)) == NULL) {
        errno = ENOMEM;
        return -1;
      } else {
        memcpy(c, name, nameLen);
        c[nameLen] = '=';
        strcpy(&c[nameLen + 1], value);
        environ7[nameCount] = c;
        environ7[nameCount + 1] = NULL;
        return 0;
      } /* if */
    } /* if */
  } /* setenv7 */



int unsetenv7 (const char *name)

  {
    size_t nameLen;
    char **p, *c;
    char **found = NULL;
    size_t nameCount = 0;

  /* unsetenv7 */
    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
      errno = EINVAL;
      return -1;
    } else {
      nameLen = strlen(name);
      if (environ7 != NULL) {
        for (p = environ7; (c = *p) != NULL; ++p) {
          nameCount++;
          if (found == NULL && environmentStrncmp(c, name, nameLen) == 0 && c[nameLen] == '=') {
            found = p;
          } /* if */
        } /* for */
        if (found != NULL) {
          if ((environ7 = realloc(environ7, sizeof(char *) * (nameCount))) == NULL) {
            errno = ENOMEM;
            return -1;
          } else {
            free(*found);
            *found = environ7[nameCount - 1];
            environ7[nameCount - 1] = NULL;
          } /* if */
        } /* if */
      } /* if */
      return 0;
    } /* if */
  } /* unsetenv7 */



#ifdef EMULATE_NODE_ENVIRONMENT
int setenvForNodeJs (const char *name, const char *value, int overwrite)

  {
    int result;

  /* setenvForNodeJs */
    result = setenv7(name, value, overwrite);
    if (result == 0) {
      EM_ASM({
        let key = Module.UTF8ToString($0);
        let value = Module.UTF8ToString($1);
        let overwrite = $2;
        if (typeof process !== 'undefined') {
          if (key in process.env) {
            if (overwrite) {
              process.env[key] = value;
            }
          } else {
            process.env[key] = value;
          }
        }
      }, name, value, overwrite);
    } /* if */
    return result;
  } /* setenvForNodeJs */



int unsetenvForNodeJs (const char *name)

  {
    int result;

  /* unsetenvForNodeJs */
    result = unsetenv7(name);
    if (result == 0) {
      EM_ASM({
        let key = Module.UTF8ToString($0);
        if (typeof process !== 'undefined') {
          delete process.env[key];
        }
      }, name);
    } /* if */
    return result;
  } /* unsetenvForNodeJs */
#endif
#endif



static striType getGroupFromGid (gid_t gid)

  {
#if HAS_GETGRGID_R || HAS_GETGRGID
#if HAS_GETGRGID_R
    int getgrgid_result;
    struct group grp;
    char buffer[2048];
#endif
    struct group *grpResult;
#endif
    striType group;

  /* getGroupFromGid */
#if HAS_GETGRGID_R || HAS_GETGRGID
#if HAS_GETGRGID_R
    getgrgid_result = getgrgid_r(gid, &grp, buffer,
                                 sizeof(buffer), &grpResult);
    if (unlikely(getgrgid_result != 0)) {
      logError(printf("getGroupFromGid: getgrgid_r(" FMT_U32 ", ...) failed:\n"
                      "errno=%d\nerror: %s\n",
                      gid, errno, strerror(errno)););
      raise_error(FILE_ERROR);
      group = NULL;
    } else
#else
    grpResult = getgrgid(gid);
#endif
    if (grpResult != NULL) {
      group = cstri8_or_cstri_to_stri(grpResult->gr_name);
      if (unlikely(group == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } else {
      group = intStr((intType) gid);
    } /* if */
#else
    group = intStr((intType) gid);
#endif
    return group;
  } /* getGroupFromGid */



static gid_t getGidFromGroup (const const_striType group, errInfoType *err_info)

  {
    os_striType os_group;
#if HAS_GETGRNAM_R || HAS_GETGRNAM
#if HAS_GETGRNAM_R
    int getgrnam_result;
    struct group grp;
    char buffer[2048];
#endif
    struct group *grpResult;
#endif
    unsigned long gidNum;
    char *endptr;
    gid_t gid;

  /* getGidFromGroup */
    logFunction(printf("getGidFromGroup(\"%s\", %d)\n",
                       striAsUnquotedCStri(group), *err_info););
    os_group = stri_to_os_stri(group, err_info);
    if (unlikely(os_group == NULL)) {
      logError(printf("getGidFromGroup: stri_to_os_stri(\"%s\", *, *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(group), *err_info););
      gid = (gid_t) -1;
    } else {
#if HAS_GETGRNAM_R || HAS_GETGRNAM
#if HAS_GETGRNAM_R
      getgrnam_result = getgrnam_r(os_group, &grp, buffer,
                                   sizeof(buffer), &grpResult);
      if (unlikely(getgrnam_result != 0)) {
        logError(printf("getGidFromGroup: getgrnam_r(\"%s\", ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_group, errno, strerror(errno)););
        os_stri_free(os_group);
        *err_info = FILE_ERROR;
        gid = (gid_t) -1;
      } else
#elif HAS_GETGRNAM
      grpResult = getgrgid(gid);
#endif
      if (grpResult != NULL) {
        gid = grpResult->gr_gid;
      } else
#endif
      if (os_group[0] >= 0 && os_group[0] <= '9') {
        gidNum = strtoul(os_group, &endptr, 10);
        if (gidNum != ULONG_MAX && endptr[0] == '\0' &&
            (long) (gid_t) gidNum == gidNum) {
          gid = (gid_t) gidNum;
        } else {
          *err_info = RANGE_ERROR;
          gid = (gid_t) -1;
        } /* if */
      } else {
        *err_info = RANGE_ERROR;
        gid = (gid_t) -1;
      } /* if */
      os_stri_free(os_group);
    } /* if */
    logFunction(printf("getGidFromGroup --> %ld\n", (long) gid););
    return gid;
  } /* getGidFromGroup */



static striType getUserFromUid (uid_t uid)

  {
#if HAS_GETPWUID_R || HAS_GETPWUID
#if HAS_GETPWUID_R
    int getpwuid_result;
    struct passwd pwd;
    char buffer[2048];
#endif
    struct passwd *pwdResult;
#endif
    striType user;

  /* getUserFromUid */
#if HAS_GETPWUID_R || HAS_GETPWUID
#if HAS_GETPWUID_R
    getpwuid_result = getpwuid_r(uid, &pwd, buffer,
                                 sizeof(buffer), &pwdResult);
    if (unlikely(getpwuid_result != 0)) {
      logError(printf("getUserFromUid: getpwuid_r(" FMT_U32 ", ...) failed:\n"
                      "errno=%d\nerror: %s\n",
                      uid, errno, strerror(errno)););
      raise_error(FILE_ERROR);
      user = NULL;
    } else
#else
    pwdResult = getpwuid(uid);
#endif
    if (pwdResult != NULL) {
      user = cstri8_or_cstri_to_stri(pwdResult->pw_name);
      if (unlikely(user == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } else {
      user = intStr((intType) uid);
    } /* if */
#else
    user = intStr((intType) uid);
#endif
    return user;
  } /* getUserFromUid */



static uid_t getUidFromUser (const const_striType user, errInfoType *err_info)

  {
    os_striType os_user;
#if HAS_GETPWNAM_R || HAS_GETPWNAM
#if HAS_GETPWNAM_R
    int getpwnam_result;
    struct passwd pwd;
    char buffer[2048];
#endif
    struct passwd *pwdResult;
#endif
    unsigned long uidNum;
    char *endptr;
    uid_t uid;

  /* getUidFromUser */
    logFunction(printf("getUidFromUser(\"%s\", %d)\n",
                       striAsUnquotedCStri(user), *err_info););
    os_user = stri_to_os_stri(user, err_info);
    if (unlikely(os_user == NULL)) {
      logError(printf("getUidFromUser: stri_to_os_stri(\"%s\", *, *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(user), *err_info););
      uid = (uid_t) -1;
    } else {
#if HAS_GETPWNAM_R || HAS_GETPWNAM
#if HAS_GETPWNAM_R
      getpwnam_result = getpwnam_r(os_user, &pwd, buffer,
                                   sizeof(buffer), &pwdResult);
      if (unlikely(getpwnam_result != 0)) {
        logError(printf("getUidFromUser: getpwnam_r(\"%s\", ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_user, errno, strerror(errno)););
        os_stri_free(os_user);
        *err_info = FILE_ERROR;
        uid = (uid_t) -1;
      } else
#elif HAS_GETPWNAM
      pwdResult = getpwuid(uid);
#endif
      if (pwdResult != NULL) {
        uid = pwdResult->pw_uid;
      } else
#endif
      if (os_user[0] >= 0 && os_user[0] <= '9') {
        uidNum = strtoul(os_user, &endptr, 10);
        if (uidNum != ULONG_MAX && endptr[0] == '\0' &&
            (long) (uid_t) uidNum == uidNum) {
          uid = (uid_t) uidNum;
        } else {
          *err_info = RANGE_ERROR;
          uid = (uid_t) -1;
        } /* if */
      } else {
        *err_info = RANGE_ERROR;
        uid = (uid_t) -1;
      } /* if */
      os_stri_free(os_user);
    } /* if */
    logFunction(printf("getUidFromUser --> %ld\n", (long) uid););
    return uid;
  } /* getUidFromUser */



striType cmdGetGroup (const const_striType filePath)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    os_stat_struct stat_buf;
    int stat_result;
    striType group;

  /* cmdGetGroup */
    logFunction(printf("cmdGetGroup(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdGetGroup: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
      raise_error(err_info);
      group = NULL;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetGroup: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        os_stri_free(os_path);
        raise_error(FILE_ERROR);
        group = NULL;
      } else {
        os_stri_free(os_path);
        /* printf("cmdGetGroup: st_gid=" FMT_U32 "\n", stat_buf.st_gid); */
        group = getGroupFromGid(stat_buf.st_gid);
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(group)););
    return group;
  } /* cmdGetGroup */



striType cmdGetOwner (const const_striType filePath)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    os_stat_struct stat_buf;
    int stat_result;
    striType owner;

  /* cmdGetOwner */
    logFunction(printf("cmdGetOwner(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdGetOwner: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
      raise_error(err_info);
      owner = NULL;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetOwner: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        os_stri_free(os_path);
        raise_error(FILE_ERROR);
        owner = NULL;
      } else {
        os_stri_free(os_path);
        /* printf("cmdGetOwner: st_uid=" FMT_U32 "\n", stat_buf.st_uid); */
        owner = getUserFromUid(stat_buf.st_uid);
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(owner)););
    return owner;
  } /* cmdGetOwner */



void cmdSetGroup (const const_striType filePath, const const_striType group)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    gid_t gid;

  /* cmdSetGroup */
    logFunction(printf("cmdSetGroup(\"%s\", ", striAsUnquotedCStri(filePath));
                printf("\"%s\")\n", striAsUnquotedCStri(group)));
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdSetGroup: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
      raise_error(err_info);
    } else {
      gid = getGidFromGroup(group, &err_info);
      if (unlikely(gid == (gid_t) -1)) {
        os_stri_free(os_path);
        raise_error(err_info);
      } else {
        if (unlikely(os_chown(os_path, (gid_t) -1, gid) != 0)) {
          logError(printf("cmdSetGroup: chown(\"" FMT_S_OS "\", -1, %ld) failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, (long) gid, errno, strerror(errno)););
          os_stri_free(os_path);
          raise_error(FILE_ERROR);
        } else {
          os_stri_free(os_path);
        } /* if */
      } /* if */
    } /* if */
  } /* cmdSetGroup */



void cmdSetOwner (const const_striType filePath, const const_striType owner)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    uid_t uid;

  /* cmdSetOwner */
    logFunction(printf("cmdSetOwner(\"%s\", ", striAsUnquotedCStri(filePath));
                printf("\"%s\")\n", striAsUnquotedCStri(owner)));
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdSetOwner: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
      raise_error(err_info);
    } else {
      uid = getUidFromUser(owner, &err_info);
      if (unlikely(uid == (uid_t) -1)) {
        os_stri_free(os_path);
        raise_error(err_info);
      } else {
        if (unlikely(os_chown(os_path, uid, (gid_t) -1) != 0)) {
          logError(printf("cmdSetOwner: chown(\"" FMT_S_OS "\", %ld, -1) failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, (long) uid, errno, strerror(errno)););
          os_stri_free(os_path);
          raise_error(FILE_ERROR);
        } else {
          os_stri_free(os_path);
        } /* if */
      } /* if */
    } /* if */
  } /* cmdSetOwner */



striType cmdUser (void)

  {
    striType user;

  /* cmdUser */
    user = getUserFromUid(getuid());
    logFunction(printf("cmdUser() --> \"%s\"", striAsUnquotedCStri(user)););
    return user;
  } /* cmdUser */
