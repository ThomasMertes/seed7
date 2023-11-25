/********************************************************************/
/*                                                                  */
/*  cmd_win.c     Command functions which call the Windows API.     */
/*  Copyright (C) 1989 - 2016, 2020 Thomas Mertes                   */
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
/*  File: seed7/src/cmd_win.c                                       */
/*  Changes: 2010 - 2016, 2020  Thomas Mertes                       */
/*  Content: Command functions which call the Windows API.          */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#if ACLAPI_H_PRESENT
#include "aclapi.h"
#endif
#include "io.h"
#include "fcntl.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"


#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#if !ACLAPI_H_PRESENT
#define SE_FILE_OBJECT 1
DWORD GetNamedSecurityInfoW (LPCWSTR pObjectName,
    int objectType, SECURITY_INFORMATION securityInfo,
    PSID *ppsidOwner, PSID *ppsidGroup,
    PACL *ppDacl, PACL *ppSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor);
DWORD SetNamedSecurityInfoW (LPWSTR pObjectName,
    int objectType, SECURITY_INFORMATION securityInfo,
    PSID psidOwner, PSID psidGroup,
    PACL pDacl, PACL pSacl);
#endif



#if defined OS_STRI_WCHAR && !defined USE_WMAIN
#ifdef DEFINE_COMMAND_LINE_TO_ARGV_W
#define CommandLineToArgvW MyCommandLineToArgvW
/**
 *  Special handling of backslash characters for CommandLineToArgvW.
 *  CommandLineToArgvW reads arguments in two modes. Inside and
 *  outside quotation mode. The following rules apply if a
 *  backslash is encountered:
 *  - 2n backslashes followed by a quotation mark produce
 *    n backslashes and a switch from inside to outside quotation
 *    mode and vice versa. In this case the quotation mark is not
 *    added to the argument.
 *  - (2n) + 1 backslashes followed by a quotation mark produce
 *    n backslashes followed by a quotation mark. In this case the
 *    quotation mark is added to the argument and the quotation mode
 +    is not changed.
 *  - n backslashes not followed by a quotation mark simply produce
 *    n backslashes.
 */
static void processBackslash (const_os_striType *sourcePos, os_striType *destPos)

  {
    memSizeType backslashCount;
    memSizeType count;

  /* processBackslash */
    backslashCount = 1;
    (*sourcePos)++;
    while (**sourcePos == '\\') {
      backslashCount++;
      (*sourcePos)++;
    } /* while */
    if (**sourcePos == '"') {
      /* Backslashes in the result: backslashCount / 2   */
      for (count = backslashCount >> 1; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
      if (backslashCount & 1) {
        /* Add a quotation mark (") to the result.       */
        **destPos = '"';
        (*destPos)++;
        /* Stay in current quotation mode                */
        (*sourcePos)++;
      } else {
        /* Ignore the quotation mark (").                */
        /* Switch from inside to outside quotation mode  */
        /* and vice versa.                               */
      } /* if */
    } else {
      /* N backslashes not followed by a quotation mark  */
      /* simply produce n backslashes.                   */
      for (count = backslashCount; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
    } /* if */
  } /* processBackslash */



/**
 *  Parse commandLine and generate an array of pointers to the arguments.
 *  The parameter w_argc and the returned array of pointers (w_argv)
 *  correspond to the parameters argc and argv of main().
 *  The rules to recognize the first argument (the command) are
 *  different from the rules to recognize the other (normal) arguments.
 *  Arguments can be quoted or unquoted. Normal arguments (all except
 *  the first argument) can consist of quoted and unquoted parts. The
 *  quoted and unquoted parts that are concatenated to form one argument.
 *  To handle quoted and unquoted parts the function works with two
 *  modes: Inside and outside quotation mode.
 *  @param w_argc Address to which the argument count is copied.
 *  @return an array of pointers to the arguments of commandLine.
 */
static os_striType *CommandLineToArgvW (const_os_striType commandLine, int *w_argc)

  {
    size_t command_line_size;
    const_os_striType sourcePos;
    os_striType destPos;
    os_striType destBuffer;
    memSizeType argumentCount;
    os_striType *w_argv;

  /* CommandLineToArgvW */
    command_line_size = os_stri_strlen(commandLine);
    argumentCount = 0;
    w_argv = (os_striType *) malloc(command_line_size * sizeof(os_striType *));
    if (w_argv != NULL) {
      sourcePos = commandLine;
      while (*sourcePos == ' ') {
        sourcePos++;
      } /* while */
      if (*sourcePos == 0) {
        w_argv[0] = NULL;
      } else {
        if (unlikely(!os_stri_alloc(destBuffer, command_line_size))) {
          free(w_argv);
          w_argv = NULL;
        } else {
          /* Set pointer to first char of first argument */
          w_argv[0] = destBuffer;
          argumentCount = 1;
          destPos = destBuffer;
          if (*sourcePos == '"') {
            sourcePos++;
            while (*sourcePos != '"' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } else {
            while (*sourcePos != ' ' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } /* if */
          if (*sourcePos != 0) {
            do {
              sourcePos++;
            } while (*sourcePos == ' ');
            if (*sourcePos != 0) {
              /* Terminate the current argument */
              *destPos = 0;
              destPos++;
              /* Set pointer to first char of next argument */
              w_argv[argumentCount] = destPos;
              argumentCount++;
            } /* if */
          } /* if */
          while (*sourcePos != 0) {
            /* printf("source char: %d\n", *sourcePos); */
            if (*sourcePos == '"') {
              /* Inside quotation mode */
              sourcePos++;
              while (*sourcePos != '"' && *sourcePos != 0) {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } /* while */
              if (*sourcePos == '"') {
                /* Consume the terminating quotation mark */
                sourcePos++;
              } /* if */
            } /* if */
            if (*sourcePos != ' ' && *sourcePos != 0) {
              /* Outside quotation mode */
              do {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } while (*sourcePos != ' ' && *sourcePos != '"' && *sourcePos != 0);
            } /* if */
            if (*sourcePos == ' ') {
              do {
                sourcePos++;
              } while (*sourcePos == ' ');
              if (*sourcePos != 0) {
                /* Terminate the current argument */
                *destPos = 0;
                destPos++;
                /* Set pointer to first char of next argument */
                w_argv[argumentCount] = destPos;
                argumentCount++;
              } /* if */
            } /* if */
          } /* while */
          /* Terminate the last argument */
          *destPos = 0;
          w_argv[argumentCount] = NULL;
        } /* if */
      } /* if */
    } /* if */
    *w_argc = argumentCount;
    return w_argv;
  } /* CommandLineToArgvW */



void freeUtf16Argv (os_striType *w_argv)

  { /* freeUtf16Argv */
    if (w_argv != NULL) {
      os_stri_free(w_argv[0]);
      free(w_argv);
    } /* if */
  } /* freeUtf16Argv */

#else



void freeUtf16Argv (os_striType *w_argv)

  { /* freeUtf16Argv */
    LocalFree(w_argv);
  } /* freeUtf16Argv */
#endif



os_striType *getUtf16Argv (int *w_argc)

  {
    os_striType commandLine;
    os_striType *w_argv;

  /* getUtf16Argv */
    commandLine = GetCommandLineW();
    w_argv = CommandLineToArgvW(commandLine, w_argc);
    return w_argv;
  } /* getUtf16Argv */
#endif



/**
 *  Get the absolute path of the executable of the current process.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the absolute path of the current process.
 */
striType getExecutablePath (const const_striType arg_0)

  {
    os_charType buffer[PATH_MAX];
    errInfoType err_info = OKAY_NO_ERROR;
    striType executablePath;

  /* getExecutablePath */
    if (unlikely(GetModuleFileNameW(NULL, buffer, PATH_MAX) == 0)) {
      raise_error(FILE_ERROR);
      executablePath = NULL;
    } else {
      executablePath = cp_from_os_path(buffer, &err_info);
      if (unlikely(executablePath == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return executablePath;
  } /* getExecutablePath */



#if USE_GET_ENVIRONMENT
os_striType *getEnvironment (void)

  {
    os_striType envBuffer;
    os_striType currPos;
    memSizeType length;
    memSizeType numElems = 0;
    memSizeType currIdx = 0;
    os_striType *env;

  /* getEnvironment */
    logFunction(printf("getEnvironment()"););
    envBuffer = GetEnvironmentStringsW();
    if (envBuffer == NULL) {
      env = NULL;
    } else {
      /* printf("envBuffer: \"" FMT_S_OS "\"\n", envBuffer); */
      currPos = envBuffer;
      do {
        length = os_stri_strlen(currPos);
        currPos = &currPos[length + 1];
        numElems++;
      } while (length != 0);
      /* printf("numElems: " FMT_U_MEM "\n", numElems); */
      env = (os_striType *) malloc(numElems * sizeof(os_striType));
      if (env != NULL) {
        currPos = envBuffer;
        do {
          env[currIdx] = currPos;
          length = os_stri_strlen(currPos);
          currPos = &currPos[length + 1];
          currIdx++;
        } while (length != 0);
        env[currIdx - 1] = NULL;
        /* for (currIdx = 0; env[currIdx] != NULL; currIdx++) {
          printf("env[" FMT_U_MEM "]: \"" FMT_S_OS "\"\n", currIdx, env[currIdx]);
        } */
      } /* if */
      if (env == NULL || env[0] == NULL) {
        if (FreeEnvironmentStringsW(envBuffer) == 0) {
          logError(printf("getEnvironment: FreeEnvironmentStrings() failed.\n"););
        } /* if */
      } /* if */
    } /* if */
    return env;
  }  /* getEnvironment */



void freeEnvironment (os_striType *environment)

  { /* freeEnvironment */
    if (environment != NULL) {
      if (environment[0] != NULL) {
        if (FreeEnvironmentStringsW(environment[0]) == 0) {
          logError(printf("getEnvironment: FreeEnvironmentStrings() failed.\n"););
        } /* if */
      } /* if */
      free(environment);
    } /* if */
  } /* freeEnvironment */
#endif



#ifdef DEFINE_WGETENV
os_striType wgetenv (const const_os_striType name)

  {
    memSizeType value_size;
    os_striType value;

  /* wgetenv */
    value_size = GetEnvironmentVariableW(name, NULL, 0);
    if (value_size == 0) {
      value = NULL;
    } else {
      if (ALLOC_UTF16(value, value_size - 1)) {
        if (GetEnvironmentVariableW(name, value, value_size) != value_size - 1) {
          FREE_OS_STRI(value);
          value = NULL;
        } /* if */
      } /* if */
    } /* if */
    return value;
  } /* wgetenv */
#endif



#ifdef DEFINE_WSETENV
int wsetenv (const const_os_striType name, const const_os_striType value,
    int overwrite)

  {
    int result;

  /* wsetenv */
    logFunction(printf("wsetenv(\"" FMT_S_OS "\", \"" FMT_S_OS "\", &d)\n",
                       name, value, overwrite););
    result = !SetEnvironmentVariableW(name, value);
    return result;
  } /* wsetenv */
#endif



#ifdef DEFINE_WUNSETENV
int wunsetenv (const const_os_striType name)

  {
    int result;

  /* wunsetenv */
    logFunction(printf("wunsetenv(\"" FMT_S_OS "\")\n", name););
    result = !SetEnvironmentVariableW(name, NULL);
    return result;
  } /* wunsetenv */
#endif



#ifdef HAS_DEVICE_IO_CONTROL

typedef struct {
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
      struct {
        USHORT SubstituteNameOffset;
        USHORT SubstituteNameLength;
        USHORT PrintNameOffset;
        USHORT PrintNameLength;
        ULONG Flags;
        WCHAR PathBuffer[1];
      } SymbolicLinkReparseBuffer;
    };
  } REPARSE_DATA_BUFFER;



/**
 *  Reads the destination of a symbolic link.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  RANGE_ERROR if the conversion to the system path failed, and
 *                  FILE_ERROR if the file does not exist or is not a symbolic link.
 *  @return The destination referred by the symbolic link, or
 *          NULL if an error occurred.
 */
striType winReadLink (const const_striType filePath, errInfoType *err_info)

  {
    os_striType os_filePath;
    int path_info;
    HANDLE fileHandle;
    union info_t {
      char buffer[100]; /* Arbitrary buffer size (must be >= 28) */
      REPARSE_DATA_BUFFER reparseDataBuffer;
    } info;
    memSizeType dataBufferHeadLength;
    memSizeType dataBufferLength;
    REPARSE_DATA_BUFFER *reparseDataBuffer;
    DWORD bytesReturned;
    DWORD lastError;
    striType destination = NULL;

  /* winReadLink */
    logFunction(printf("winReadLink(\"%s\", %d)\n",
                       striAsUnquotedCStri(filePath), *err_info););
    os_filePath = cp_to_os_path(filePath, &path_info, err_info);
    if (unlikely(os_filePath == NULL)) {
      logError(printf("winReadLink: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, *err_info););
    } else {
      fileHandle = CreateFileW(os_filePath, GENERIC_READ, 0, NULL,
                               OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT |
                               FILE_FLAG_BACKUP_SEMANTICS, NULL);
      if (unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
        logError(printf("winReadLink(\"%s\", *): "
                        "CreateFileW(\"" FMT_S_OS "\", *) failed:\n"
                        "lastError=" FMT_U32 "\n",
                        striAsUnquotedCStri(filePath), os_filePath,
                        (uint32Type) GetLastError()););
        *err_info = FILE_ERROR;
      } else {
        if (unlikely(DeviceIoControl(fileHandle,
                                     FSCTL_GET_REPARSE_POINT,
                                     NULL, 0, info.buffer, sizeof(info),
                                     &bytesReturned, NULL) == 0)) {
          lastError = GetLastError();
          if (lastError != ERROR_MORE_DATA) {
            logError(printf("winReadLink(\"%s\", *): "
                            "DeviceIoControl() failed:\n"
                            "lastError=" FMT_U32 "%s\n",
                            striAsUnquotedCStri(filePath),
                            (uint32Type) lastError,
                            lastError == ERROR_NOT_A_REPARSE_POINT ?
                                " (ERROR_NOT_A_REPARSE_POINT)" : ""););
            *err_info = FILE_ERROR;
          } else {
            dataBufferHeadLength = (memSizeType)
                ((char *) &info.reparseDataBuffer.SymbolicLinkReparseBuffer -
                 (char *) &info.reparseDataBuffer);
            dataBufferLength = dataBufferHeadLength +
                info.reparseDataBuffer.ReparseDataLength;
            reparseDataBuffer = (REPARSE_DATA_BUFFER *) malloc(dataBufferLength);
            if (unlikely(reparseDataBuffer == NULL)) {
              *err_info = MEMORY_ERROR;
            } else {
              if (unlikely(DeviceIoControl(fileHandle,
                                           FSCTL_GET_REPARSE_POINT, NULL, 0,
                                           reparseDataBuffer, dataBufferLength,
                                           &bytesReturned, NULL) == 0)) {
                logError(printf("winReadLink(\"%s\", *): "
                                "DeviceIoControl() failed:\n"
                                "lastError=" FMT_U32 "%\n",
                                striAsUnquotedCStri(filePath),
                                (uint32Type) GetLastError()););
                *err_info = FILE_ERROR;
              } else {
                destination = cp_from_os_path_buffer(
                    &((wchar_t *) reparseDataBuffer->SymbolicLinkReparseBuffer.PathBuffer)[
                        reparseDataBuffer->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t)],
                    reparseDataBuffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t),
                    err_info);
              } /* if */
              free(reparseDataBuffer);
            } /* if */
          } /* if */
        } else {
          destination = cp_from_os_path_buffer(
              &((wchar_t *) info.reparseDataBuffer.SymbolicLinkReparseBuffer.PathBuffer)[
                  info.reparseDataBuffer.SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t)],
              info.reparseDataBuffer.SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t),
              err_info);
        } /* if */
        CloseHandle(fileHandle);
      } /* if */
      os_stri_free(os_filePath);
    } /* if */
    logFunction(printf("winReadLink(\"%s\", %d) --> ",
                       striAsUnquotedCStri(filePath), *err_info);
                printf("\"%s\"\n", striAsUnquotedCStri(destination)););
    return destination;
  } /* winReadLink */
#endif



static striType getNameFromSid (PSID sid)

  {
    LPWSTR AcctName;
    DWORD dwAcctName = 0;
    LPWSTR DomainName;
    DWORD dwDomainName = 0;
    SID_NAME_USE eUse = SidTypeUnknown;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* getNameFromSid */
    LookupAccountSidW(NULL, sid,
                      NULL, (LPDWORD) &dwAcctName,
                      NULL, (LPDWORD) &dwDomainName, &eUse);
    AcctName = (LPWSTR) GlobalAlloc(GMEM_FIXED, dwAcctName * sizeof(os_charType));
    DomainName = (LPWSTR) GlobalAlloc(GMEM_FIXED, dwDomainName * sizeof(os_charType));
    if (unlikely(AcctName == NULL || DomainName == NULL)) {
      logError(printf("getNameFromSid: GlobalAlloc() failed:\n"
                      "lastError=" FMT_U32 "\n",
                      (uint32Type) GetLastError()););
      if (AcctName != NULL) {
        GlobalFree(AcctName);
      } /* if */
      raise_error(MEMORY_ERROR);
      name = NULL;
    } else if (unlikely(
      LookupAccountSidW(NULL, sid,
                        AcctName, (LPDWORD) &dwAcctName,
                        DomainName, (LPDWORD) &dwDomainName,
                        &eUse) == FALSE)) {
      logError(printf("getNameFromSid: LookupAccountSidW() failed:\n"
                      "lastError=" FMT_U32 "\n",
                      (uint32Type) GetLastError()););
      GlobalFree(AcctName);
      GlobalFree(DomainName);
      raise_error(FILE_ERROR);
      name = NULL;
    } else {
      name = os_stri_to_stri(AcctName, &err_info);
      GlobalFree(AcctName);
      GlobalFree(DomainName);
      if (unlikely(name == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return name;
  } /* getNameFromSid */



static PSID getSidFromName (const const_striType name, errInfoType *err_info)

  {
    os_striType accountName;
    DWORD numberOfBytesForSid = 0;
    DWORD numberOfCharsForDomainName = 0;
    os_striType domainName;
    SID_NAME_USE sidNameUse = SidTypeInvalid;
    PSID sid = NULL;

  /* getSidFromName */
    logFunction(printf("getSidFromName(\"%s\", %d)\n",
                       striAsUnquotedCStri(name), *err_info););
    accountName = stri_to_os_stri(name, err_info);
    if (likely(accountName != NULL)) {
      LookupAccountNameW(NULL, accountName, NULL, &numberOfBytesForSid,
                         NULL, &numberOfCharsForDomainName, &sidNameUse);
      sid = (PSID) malloc(numberOfBytesForSid);
      if (unlikely(sid == NULL)) {
        *err_info = MEMORY_ERROR;
      } else if (unlikely(!ALLOC_OS_STRI(domainName, numberOfCharsForDomainName))) {
        free(sid);
        *err_info = MEMORY_ERROR;
        sid = NULL;
      } else {
        if (unlikely(LookupAccountNameW(NULL, accountName, sid, &numberOfBytesForSid,
                                        domainName, &numberOfCharsForDomainName, &sidNameUse) == 0)) {
          logError(printf("getSidFromName: LookupAccountNameW failed:\n"
                          "lastError=" FMT_U32 "\n",
                          (uint32Type) GetLastError()););
          free(sid);
          *err_info = FILE_ERROR;
          sid = NULL;
        } /* if */
        FREE_OS_STRI(domainName);
      } /* if */
      os_stri_free(accountName);
    } /* if */
    return sid;
  } /* getSidFromName */



striType cmdGetGroup (const const_striType filePath)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PSID pSidGroup = NULL;
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
    } else if (unlikely(
      GetNamedSecurityInfoW(os_path, SE_FILE_OBJECT,
                            GROUP_SECURITY_INFORMATION, NULL,
                            &pSidGroup, NULL, NULL, &pSD) != ERROR_SUCCESS)) {
      logError(printf("cmdGetGroup: "
                      "GetNamedSecurityInfoW(\"" FMT_S_OS "\", ...) failed:\n"
                      "lastError=" FMT_U32 "\n",
                      os_path, (uint32Type) GetLastError()););
      os_stri_free(os_path);
      raise_error(FILE_ERROR);
      group = NULL;
    } else {
      os_stri_free(os_path);
      group = getNameFromSid(pSidGroup);
      /* The SID referenced by pSidOwner is located */
      /* inside of the PSECURITY_DESCRIPTOR pSD.    */
      /* Therefore it is freed together with pSD.   */
      LocalFree(pSD);
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(group)););
    return group;
  } /* cmdGetGoup */



striType cmdGetOwner (const const_striType filePath)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PSID pSidOwner = NULL;
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
    } else if (unlikely(
      GetNamedSecurityInfoW(os_path, SE_FILE_OBJECT,
                            OWNER_SECURITY_INFORMATION, &pSidOwner,
                            NULL, NULL, NULL, &pSD) != ERROR_SUCCESS)) {
      logError(printf("cmdGetOwner: "
                      "GetNamedSecurityInfoW(\"" FMT_S_OS "\", ...) failed:\n"
                      "lastError=" FMT_U32 "\n",
                      os_path, (uint32Type) GetLastError()););
      os_stri_free(os_path);
      raise_error(FILE_ERROR);
      owner = NULL;
    } else {
      os_stri_free(os_path);
      owner = getNameFromSid(pSidOwner);
      /* The SID referenced by pSidOwner is located */
      /* inside of the PSECURITY_DESCRIPTOR pSD.    */
      /* Therefore it is freed together with pSD.   */
      LocalFree(pSD);
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(owner)););
    return owner;
  } /* cmdGetOwner */



void cmdSetGroup (const const_striType filePath, const const_striType group)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    PSID pSidGroup;

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
      pSidGroup = getSidFromName(group, &err_info);
      if (unlikely(pSidGroup == NULL)) {
        os_stri_free(os_path);
        raise_error(err_info);
      } else {
        SetNamedSecurityInfoW(os_path, SE_FILE_OBJECT,
                              GROUP_SECURITY_INFORMATION, NULL,
                              pSidGroup, NULL, NULL);
        free(pSidGroup);
        os_stri_free(os_path);
      } /* if */
    } /* if */
  } /* cmdSetGroup */



void cmdSetOwner (const const_striType filePath, const const_striType owner)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    PSID pSidOwner;

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
      pSidOwner = getSidFromName(owner, &err_info);
      if (unlikely(pSidOwner == NULL)) {
        os_stri_free(os_path);
        raise_error(err_info);
      } else {
        SetNamedSecurityInfoW(os_path, SE_FILE_OBJECT,
                              OWNER_SECURITY_INFORMATION, pSidOwner,
                              NULL, NULL, NULL);
        free(pSidOwner);
        os_stri_free(os_path);
      } /* if */
    } /* if */
  } /* cmdSetOwner */



striType cmdUser (void)

  {
    HANDLE hToken = NULL;
    TOKEN_USER *ptu;
    DWORD dwSize = 0;
    striType user;

  /* cmdUser */
    if (unlikely(OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken) == 0 &&
                 OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0)) {
      logError(printf("cmdUser: OpenThreadToken() and OpenProcessToken() failed."););
      raise_error(FILE_ERROR);
      user = NULL;
    } else {
      if (unlikely(GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize) != 0 ||
                   ERROR_INSUFFICIENT_BUFFER != GetLastError())) {
        logError(printf("cmdUser: GetTokenInformation() "
                        "should fail with ERROR_INSUFFICIENT_BUFFER.\n"
                        "lastError=" FMT_U32 "\n",
                        (uint32Type) GetLastError()););
        raise_error(FILE_ERROR);
        user = NULL;
      } else {
        ptu = (TOKEN_USER *) LocalAlloc(LPTR, dwSize);
        if (unlikely(ptu == NULL)) {
          raise_error(MEMORY_ERROR);
          user = NULL;
        } else {
          if (GetTokenInformation(hToken, TokenUser, ptu, dwSize, &dwSize) == 0) {
            logError(printf("cmdUser: GetNamedSecurityInfoW() failed:\n"
                            "lastError=" FMT_U32 "\n",
                            (uint32Type) GetLastError()););
            LocalFree((HLOCAL) ptu);
            raise_error(FILE_ERROR);
            user = NULL;
          } else {
            user = getNameFromSid(ptu->User.Sid);
            LocalFree((HLOCAL) ptu);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return user;
  } /* cmdUser */
