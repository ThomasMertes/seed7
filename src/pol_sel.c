/********************************************************************/
/*                                                                  */
/*  pol_sel.c     Poll type and function based on select function.  */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/pol_sel.c                                       */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Poll type and function based on select function.       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_WINSOCK
#define FD_SETSIZE 65536
#include "winsock2.h"
#else
#include "sys/select.h"
#endif
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "hsh_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "pol_drv.h"


#ifdef USE_WINSOCK
#define DYNAMIC_FD_SET
#define VERIFY_FD_SETSIZE
#define SIZEOF_FD_SET(size) \
    (sizeof(fd_set) - FD_SETSIZE * sizeof(SOCKET) + (size) * sizeof(SOCKET))
#define USED_FD_SET_SIZE(fdset) SIZEOF_FD_SET((fdset)->fd_count)
#else
#define SELECT_WITH_NFDS
#define VERIFY_MAXIMUM_FD_NUMBER
#define SIZEOF_FD_SET(size) sizeof(fd_set)
#define USED_FD_SET_SIZE(fdset) sizeof(fd_set)
#endif

#define USE_PREPARED_FD_SET
#define MEMCPY_FD_SET


typedef enum {
    ITER_EMPTY,
    ITER_CHECKS_IN, ITER_CHECKS_OUT, ITER_CHECKS_INOUT,
    ITER_FINDINGS_IN, ITER_FINDINGS_OUT, ITER_FINDINGS_INOUT
  } iteratorType;

typedef struct {
    sockettype fd;
    rtlGenerictype file;
  } fdAndFileType;

typedef struct {
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
    fd_set *inFdset;
#endif
    fd_set *outFdset;
#else
#ifdef USE_PREPARED_FD_SET
    fd_set inFdset;
#endif
    fd_set outFdset;
#endif
#if defined USE_PREPARED_FD_SET && defined SELECT_WITH_NFDS
    int preparedNfds;
#endif
    memsizetype size;
    memsizetype capacity;
    memsizetype iterPos;
    fdAndFileType *files;
    rtlHashtype indexHash;
  } testType;

typedef struct select_based_pollstruct {
    testType readTest;
    testType writeTest;
    iteratorType iteratorMode;
    memsizetype iterEvents;
    memsizetype numOfEvents;
  } select_based_pollrecord, *select_based_polltype;

typedef const struct select_based_pollstruct *const_select_based_polltype;

#define conv(genericPollData) ((const_select_based_polltype) genericPollData)
#define var_conv(genericPollData) ((select_based_polltype) genericPollData)


#ifdef DYNAMIC_FD_SET

#define to_inFdset(test)            ((test)->inFdset)
#define to_outFdset(test)           ((test)->outFdset)
#define to_read_inFdset(data)       (((const_select_based_polltype) data)->readTest.inFdset)
#define to_write_inFdset(data)      (((const_select_based_polltype) data)->writeTest.inFdset)
#define to_read_outFdset(data)      (((const_select_based_polltype) data)->readTest.outFdset)
#define to_write_outFdset(data)     (((const_select_based_polltype) data)->writeTest.outFdset)
#define to_var_read_inFdset(data)   (((select_based_polltype) data)->readTest.inFdset)
#define to_var_write_inFdset(data)  (((select_based_polltype) data)->writeTest.inFdset)
#define to_var_read_outFdset(data)  (((select_based_polltype) data)->readTest.outFdset)
#define to_var_write_outFdset(data) (((select_based_polltype) data)->writeTest.outFdset)

#define ALLOC_FDSET(var,nr)      (ALLOC_HEAP(var,fd_set *,SIZEOF_FD_SET(nr))?CNT1_BYT(SIZEOF_FD_SET(nr)),TRUE:FALSE)
#define FREE_FDSET(var,nr)       (CNT2_BYT(SIZEOF_FD_SET(nr)) FREE_HEAP(var, SIZEOF_FD_SET(nr)))
#define REALLOC_FDSET(var,nr)    REALLOC_HEAP(var, fd_set *, SIZEOF_FD_SET(nr))
#define COUNT3_FDSET(nr1,nr2)    CNT3(CNT2_BYT(SIZEOF_FD_SET(nr1)) CNT1_BYT(SIZEOF_FD_SET(nr2)))

#else

#define to_inFdset(test)            (&(test)->inFdset)
#define to_outFdset(test)           (&(test)->outFdset)
#define to_read_inFdset(data)       (&((const_select_based_polltype) data)->readTest.inFdset)
#define to_write_inFdset(data)      (&((const_select_based_polltype) data)->writeTest.inFdset)
#define to_read_outFdset(data)      (&((const_select_based_polltype) data)->readTest.outFdset)
#define to_write_outFdset(data)     (&((const_select_based_polltype) data)->writeTest.outFdset)
#define to_var_read_inFdset(data)   (&((select_based_polltype) data)->readTest.inFdset)
#define to_var_write_inFdset(data)  (&((select_based_polltype) data)->writeTest.inFdset)
#define to_var_read_outFdset(data)  (&((select_based_polltype) data)->readTest.outFdset)
#define to_var_write_outFdset(data) (&((select_based_polltype) data)->writeTest.outFdset)

#endif


#define TABLE_START_SIZE  256
#define TABLE_INCREMENT  1024



#ifdef OUT_OF_ORDER
static void dumpPoll (const const_polltype pollData)
    {
      memsizetype pos;
/*
      printf("readSize=%d\n", conv(pollData)->readTest.size);
      printf("readCapacity=%d\n", conv(pollData)->readTest.capacity);
      printf("readPos=%d\n", conv(pollData)->readTest.pos);
      printf("writeSize=%d\n", conv(pollData)->writeTest.size);
      printf("writeCapacity=%d\n", conv(pollData)->writeTest.capacity);
      printf("writePos=%d\n", conv(pollData)->writeTest.pos);
      printf("iteratorMode=%d\n", conv(pollData)->iteratorMode);
      printf("iterEvents=%d\n", conv(pollData)->iterEvents);
      printf("numOfEvents=%d\n", conv(pollData)->numOfEvents);
      */
#ifdef USE_PREPARED_FD_SET
      //if (conv(pollData)->readTest.size <= 5) {
      printf("readFds:");
      for (pos = 0; pos < conv(pollData)->readTest.size; pos++) {
        printf(" %d", conv(pollData)->readTest.files[pos].fd);
        if (!FD_ISSET(conv(pollData)->readTest.files[pos].fd, to_read_inFdset(pollData))) {
          printf("*");
        }
      }
      printf("\n");
      //}
      /*
      printf("writeFds:");
      for (pos = 0; pos < conv(pollData)->writeTest.size; pos++) {
        printf(" %d", conv(pollData)->writeTest.files[pos].fd);
        if (!FD_ISSET(conv(pollData)->writeTest.files[pos].fd, to_write_inFdset(pollData))) {
          printf("*");
        }
      }
      */
#endif
    }
#endif



#ifdef MEMCPY_FD_SET
#define copyFdSet(dest, source, unused) memcpy(dest, source, USED_FD_SET_SIZE(source))
#else

#ifdef ANSI_C

static void copyFdSet (fd_set *dest, const fd_set *source, testType *test)
#else

static void copyFdSet (dest, source, test)
fd_set *dest;
fd_set *source;
testType *test;
#endif

  {
    memsizetype pos;

  /* copyFdSet */
    FD_ZERO(dest);
    for (pos = 0; pos < test->size; pos++) {
      if (FD_ISSET(test->files[pos].fd, source)) {
        FD_SET(test->files[pos].fd, dest);
      } /* if */
    } /* for */
  } /* copyFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define allocFdSet(test, capacity) TRUE
#else

#ifdef ANSI_C

static booltype allocFdSet (testType *test, memsizetype capacity)
#else

static booltype allocFdSet (test, capacity)
testType *test;
memsizetype capacity;
#endif

  {
    booltype result = TRUE;

  /* allocFdSet */
#ifdef USE_PREPARED_FD_SET
    if (unlikely(!ALLOC_FDSET(test->inFdset, capacity))) {
      result = FALSE;
      test->outFdset = NULL;
    } else if (unlikely(!ALLOC_FDSET(test->outFdset, capacity))) {
      result = FALSE;
      FREE_FDSET(test->inFdset, capacity);
      test->inFdset = NULL;
    } /* if */
#else
    if (unlikely(!ALLOC_FDSET(test->outFdset, capacity))) {
      result = FALSE;
    } /* if */
#endif
    return result;
  } /* allocFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define freeFdSet(test, capacity) 0
#else

#ifdef ANSI_C

static void freeFdSet (testType *test, memsizetype capacity)
#else

static void freeFdSet (test, capacity)
testType *test;
memsizetype capacity;
#endif

  { /* freeFdSet */
    if (test->outFdset != NULL) {
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(test->inFdset, capacity);
#endif
      FREE_FDSET(test->outFdset, capacity);
    } /* if */
  } /* freeFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define reallocFdSet(pollData, increment) TRUE
#else

#ifdef ANSI_C

static booltype reallocFdSet (testType *test, memsizetype increment)
#else

static booltype reallocFdSet (test, increment)
testType *test;
memsizetype increment;
#endif

  {
    fd_set *newFdset;
    booltype result = TRUE;

  /* reallocFdSet */
#ifdef USE_PREPARED_FD_SET
    if (unlikely((newFdset = REALLOC_FDSET(test->inFdset,
        test->capacity + increment)) == NULL)) {
      result = FALSE;
    } else {
      COUNT3_FDSET(test->capacity,
                   test->capacity + increment);
      test->inFdset = newFdset;
    } /* if */
#endif
    if (unlikely((newFdset = REALLOC_FDSET(test->outFdset,
        test->capacity + increment)) == NULL)) {
      result = FALSE;
    } else {
      COUNT3_FDSET(test->capacity,
                   test->capacity + increment);
      test->outFdset = newFdset;
    } /* if */
    return result;
  } /* reallocFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define replaceFdSet(test, capacity) TRUE
#else

#ifdef ANSI_C

static booltype replaceFdSet (testType *test, memsizetype capacity)
#else

static booltype replaceFdSet (test, capacity)
testType *test;
memsizetype capacity;
#endif

  {
#ifdef USE_PREPARED_FD_SET
    fd_set *newInFdset;
#endif
    fd_set *newOutFdset;
    booltype result = TRUE;

  /* replaceFdSet */
#ifdef USE_PREPARED_FD_SET
    if (unlikely(!ALLOC_FDSET(newInFdset, capacity))) {
      result = FALSE;
    } /* if */
#endif
    if (unlikely(!ALLOC_FDSET(newOutFdset, capacity))) {
      result = FALSE;
#ifdef USE_PREPARED_FD_SET
      if (newInFdset != NULL) {
        FREE_FDSET(newInFdset, capacity);
      } /* if */
#endif
    } /* if */
    if (result) {
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(test->inFdset, capacity);
      test->inFdset = newInFdset;
#endif
      FREE_FDSET(test->outFdset, capacity);
      test->outFdset = newOutFdset;
    } /* if */
    return result;
  } /* replaceFdSet */
#endif



#ifdef ANSI_C

static void addCheck (testType *test, const sockettype aSocket,
    const rtlGenerictype fileObj)
#else

static void addCheck (test, aSocket, fileObj)
testType *test;
sockettype aSocket;
rtlGenerictype fileObj;
#endif

  {
    memsizetype pos;

  /* addCheck */
    /* printf("addCheck(..., %u, 0x%lx)\n", aSocket, (unsigned long) fileObj); */
#ifdef VERIFY_MAXIMUM_FD_NUMBER
    if (aSocket < 0 || aSocket >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
#ifdef VERIFY_FD_SETSIZE
    if (test->size >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    pos = (memsizetype) hshIdxEnterDefault(test->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) test->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
        (createfunctype) &intCreateGeneric, (createfunctype) &intCreateGeneric);
    if (pos == test->size) {
      if (test->size >= test->capacity) {
        test->files = REALLOC_TABLE(test->files,
            fdAndFileType, test->capacity,
            test->capacity + TABLE_INCREMENT);
        if (test->files == NULL ||
            !reallocFdSet(test, TABLE_INCREMENT)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_TABLE(fdAndFileType, test->capacity,
                       test->capacity + TABLE_INCREMENT);
          test->capacity += TABLE_INCREMENT;
          /* printf("increment table to %lu\n", test->capacity); */
        } /* if */
      } /* if */
      test->size++;
      test->files[pos].fd = aSocket;
      test->files[pos].file = fileObj;
#ifdef USE_PREPARED_FD_SET
      FD_SET(aSocket, to_inFdset(test));
#ifdef SELECT_WITH_NFDS
      if (aSocket >= test->preparedNfds) {
        test->preparedNfds = (int) aSocket + 1;
      } /* if */
#endif
#endif
    } /* if */
  } /* addCheck */



#ifdef ANSI_C

static void removeCheck (testType *test, const sockettype aSocket)
#else

static void removeCheck (test, aSocket)
testType *test;
sockettype aSocket;
#endif

  {
    memsizetype pos;

  /* removeCheck */
    /* printf("removeCheck(..., %u)\n", aSocket); */
    pos = (memsizetype) hshIdxWithDefault(test->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) test->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos != test->size) {
      if (pos + 1 <= test->iterPos) {
        test->iterPos--;
        if (pos < test->iterPos) {
          memcpy(&test->files[pos],
                 &test->files[test->iterPos], sizeof(fdAndFileType));
          hshIdxAddr(test->indexHash,
                     (rtlGenerictype) (memsizetype) test->files[pos].fd,
                     (inttype) (memsizetype) test->files[pos].fd,
                     (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
          pos = test->iterPos;
        } /* if */
      } /* if */
      test->size--;
      if (pos < test->size) {
        memcpy(&test->files[pos],
               &test->files[test->size], sizeof(fdAndFileType));
        hshIdxAddr(test->indexHash,
                   (rtlGenerictype) (memsizetype) test->files[pos].fd,
                   (inttype) (memsizetype) test->files[pos].fd,
                   (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
      } /* if */
      hshExcl(test->indexHash, (rtlGenerictype) (memsizetype) aSocket,
              (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
              (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
#ifdef USE_PREPARED_FD_SET
      FD_CLR(aSocket, to_inFdset(test));
#endif
    } /* if */
  } /* removeCheck */



#ifdef ANSI_C

static void doPoll (const polltype pollData, struct timeval *timeout)
#else

static void doPoll (pollData, timeout)
polltype pollData;
struct timeval *timeout;
#endif

  {
    int nfds = 0;
    fd_set *readFds;
    fd_set *writeFds;
#ifndef USE_PREPARED_FD_SET
    memsizetype pos;
    sockettype sock;
#endif
    int select_result;

  /* doPoll */
    /* printf("doPoll\n");
       dumpPoll(pollData); */
    readFds = to_var_read_outFdset(pollData);
#ifdef USE_PREPARED_FD_SET
    copyFdSet(readFds, to_read_inFdset(pollData),
              &conv(pollData)->readTest.files);
#ifdef SELECT_WITH_NFDS
    nfds = conv(pollData)->readTest.preparedNfds;
#endif
#else
    FD_ZERO(readFds);
    for (pos = 0; pos < conv(pollData)->readTest.size; pos++) {
      sock = conv(pollData)->readTest.files[pos].fd;
      FD_SET(sock, readFds);
#ifdef SELECT_WITH_NFDS
      if ((int) sock >= nfds) {
        nfds = (int) sock + 1;
      } /* if */
#endif
    } /* for */
#endif
    writeFds = to_var_write_outFdset(pollData);
#ifdef USE_PREPARED_FD_SET
    copyFdSet(writeFds, to_write_inFdset(pollData),
              &conv(pollData)->writeTest.files);
#ifdef SELECT_WITH_NFDS
    if (conv(pollData)->writeTest.preparedNfds > nfds) {
      nfds = conv(pollData)->writeTest.preparedNfds;
    } /* if */
#endif
#else
    FD_ZERO(writeFds);
    for (pos = 0; pos < conv(pollData)->writeTest.size; pos++) {
      sock = conv(pollData)->writeTest.files[pos].fd;
      FD_SET(sock, writeFds);
#ifdef SELECT_WITH_NFDS
      if ((int) sock >= nfds) {
        nfds = (int) sock + 1;
      } /* if */
#endif
    } /* for */
#endif
    /* printf("doPoll %lx: nfds=%d\n", pollData, nfds); */
    select_result = select(nfds, readFds, writeFds, NULL, timeout);
    if (unlikely(select_result < 0)) {
      /* printf("errno=%d\n", errno);
      printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
          EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
      printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d  EBADF=%d\n",
           EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL, EBADF); */
      /* printf("WSAGetLastError=%d\n", WSAGetLastError());
      printf("WSANOTINITIALISED=%ld  WSAEFAULT=%ld  WSAENETDOWN=%ld  WSAEINVAL=%ld\n",
          WSANOTINITIALISED, WSAEFAULT, WSAENETDOWN, WSAEINVAL);
      printf("WSAEINTR=%ld  WSAEINPROGRESS=%ld  WSAENOTSOCK =%ld\n",
          WSAEINTR, WSAEINPROGRESS, WSAENOTSOCK); */
      /* printf("nfds=%d\n", nfds); */
      raise_error(FILE_ERROR);
    } else {
      var_conv(pollData)->readTest.iterPos = 0;
      var_conv(pollData)->writeTest.iterPos = 0;
      var_conv(pollData)->numOfEvents = (memsizetype) select_result;
    } /* if */
  } /* doPoll */



#ifdef ANSI_C

static booltype isChecked (const testType *test, const sockettype aSocket)
#else

static booltype isChecked (test, aSocket)
testType *test;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    booltype result;

  /* isChecked */
    pos = (memsizetype) hshIdxWithDefault(test->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) test->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    result = pos != test->size;
    /* printf("isChecked: sock=%d, pos=%d, fd=%d\n",
        aSocket, pos, test->files[pos].fd); */
    return result;
  } /* isChecked */



#ifdef ANSI_C

static booltype isReady (const testType *test, const sockettype aSocket)
#else

static booltype isReady (test, aSocket)
testType *test;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    booltype result;

  /* isReady */
    pos = (memsizetype) hshIdxWithDefault(test->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) test->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos == test->size) {
      result = FALSE;
    } else {
      result = FD_ISSET(test->files[pos].fd, to_outFdset(test)) != 0;
      /* printf("isReady: sock=%d, pos=%d, fd=%d\n",
          aSocket, pos, test->files[pos].fd); */
    } /* if */
    return result;
  } /* isReady */



#ifdef ANSI_C

static booltype hasNextCheck (const testType *test)
#else

static booltype hasNextCheck (test)
testType *test;
#endif

  { /* hasNextCheck */
    return test->iterPos < test->size;
  } /* hasNextCheck */



#ifdef ANSI_C

static booltype hasNextFinding (testType *test, memsizetype iterEvents)
#else

static booltype hasNextFinding (test, iterEvents)
testType *test;
memsizetype iterEvents;
#endif

  {
    memsizetype pos;
    fd_set *fdSet;
    booltype hasNext;

  /* hasNextFinding */
    if (iterEvents == 0) {
      hasNext = FALSE;
      test->iterPos = test->size;
    } else {
      pos = test->iterPos;
      fdSet = to_outFdset(test);
      while (pos < test->size &&
          !FD_ISSET(test->files[pos].fd, fdSet)) {
        pos++;
      } /* while */
      hasNext = pos < test->size;
      test->iterPos = pos;
    } /* if */
    return hasNext;
  } /* hasNextFinding */



#ifdef ANSI_C

static rtlGenerictype nextCheck (testType *test,
    const rtlGenerictype nullFile)
#else

static rtlGenerictype nextCheck (test, nullFile)
testType *test;
rtlGenerictype nullFile;
#endif

  {
    memsizetype pos;
    rtlGenerictype checkFile;

  /* nextCheck */
    pos = test->iterPos;
    if (pos < test->size) {
      checkFile = test->files[pos].file;
      /* printf("nextCheck -> 0x%lx fd[%u]=%d\n",
          (unsigned long) checkFile, pos,
          test->files[pos].fd); */
      test->iterPos = pos + 1;
    } else {
      checkFile = nullFile;
    } /* if */
    return checkFile;
  } /* nextCheck */



#ifdef ANSI_C

static rtlGenerictype nextFinding (testType *test,
    memsizetype *iterEvents, const rtlGenerictype nullFile)
#else

static rtlGenerictype nextFinding (test, iterEvents, nullFile)
testType *test;
memsizetype *iterEvents;
rtlGenerictype nullFile;
#endif

  {
    memsizetype pos;
    fd_set *fdSet;
    rtlGenerictype resultFile;

  /* nextFinding */
    if (*iterEvents == 0) {
      resultFile = nullFile;
      test->iterPos = test->size;
    } else {
      pos = test->iterPos;
      fdSet = to_outFdset(test);
      while (pos < test->size &&
          !FD_ISSET(test->files[pos].fd, fdSet)) {
        pos++;
      } /* while */
      if (pos < test->size) {
        resultFile = test->files[pos].file;
        /* printf("nextFinding -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, pos,
            test->files[pos].fd); */
        (*iterEvents)--;
        pos++;
      } else {
        resultFile = nullFile;
      } /* if */
      test->iterPos = pos;
    } /* if */
    return resultFile;
  } /* nextFinding */



/**
 *  Add ''eventsToCheck'' for ''aSocket'' to ''pollData''.
 *  The parameter ''fileObj'' determines, which file is returned,
 *  when the iterator returns files in ''pollData''.
 */
#ifdef ANSI_C

void polAddCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck, const rtlGenerictype fileObj)
#else

void polAddCheck (pollData, aSocket, eventsToCheck, fileObj)
polltype pollData;
sockettype aSocket;
inttype eventsToCheck;
rtlGenerictype fileObj;
#endif

  { /* polAddCheck */
    switch (eventsToCheck) {
      case POLL_IN:
        addCheck(&var_conv(pollData)->readTest, aSocket, fileObj);
        break;
      case POLL_OUT:
        addCheck(&var_conv(pollData)->writeTest, aSocket, fileObj);
        break;
      case POLL_INOUT:
        addCheck(&var_conv(pollData)->readTest, aSocket, fileObj);
        addCheck(&var_conv(pollData)->writeTest, aSocket, fileObj);
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* polAddCheck */



#ifdef ANSI_C

void polClear (const polltype pollData)
#else

void polClear (pollData)
polltype pollData;
#endif

  { /* polClear */
    /* printf("polClear\n"); */
#ifdef USE_PREPARED_FD_SET
    FD_ZERO(to_read_inFdset(pollData));
    FD_ZERO(to_write_inFdset(pollData));
#ifdef SELECT_WITH_NFDS
    var_conv(pollData)->readTest.preparedNfds = 0;
    var_conv(pollData)->writeTest.preparedNfds = 0;
#endif
#endif
    FD_ZERO(to_read_outFdset(pollData));
    FD_ZERO(to_write_outFdset(pollData));
    var_conv(pollData)->readTest.size = 0;
    var_conv(pollData)->readTest.iterPos = 0;
    hshDestr(conv(pollData)->readTest.indexHash, (destrfunctype) &intDestrGeneric,
             (destrfunctype) &intDestrGeneric);
    var_conv(pollData)->readTest.indexHash = hshEmpty();
    var_conv(pollData)->writeTest.size = 0;
    var_conv(pollData)->writeTest.iterPos = 0;
    hshDestr(conv(pollData)->writeTest.indexHash, (destrfunctype) &intDestrGeneric,
             (destrfunctype) &intDestrGeneric);
    var_conv(pollData)->writeTest.indexHash = hshEmpty();
    var_conv(pollData)->iteratorMode = ITER_EMPTY;
    var_conv(pollData)->iterEvents = 0;
    var_conv(pollData)->numOfEvents = 0;
  } /* polClear */



#ifdef ANSI_C

void polCpy (const polltype poll_to, const const_polltype pollDataFrom)
#else

void polCpy (poll_to, pollDataFrom)
polltype poll_to;
polltype pollDataFrom;
#endif

  {
    select_based_polltype pollData;
    rtlHashtype newReadIndexHash;
    rtlHashtype newWriteIndexHash;
    fdAndFileType *newReadFiles;
    fdAndFileType *newWriteFiles;

  /* polCpy */
    /* printf("polCpy\n"); */
    if (poll_to != pollDataFrom) {
      newReadIndexHash = hshCreate(conv(pollDataFrom)->readTest.indexHash,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
      newWriteIndexHash = hshCreate(conv(pollDataFrom)->writeTest.indexHash,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
      if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        pollData = var_conv(poll_to);
        if (pollData->readTest.capacity < conv(pollDataFrom)->readTest.size) {
          if (unlikely(!ALLOC_TABLE(newReadFiles, fdAndFileType,
              conv(pollDataFrom)->readTest.capacity) ||
              !replaceFdSet(&pollData->readTest, conv(pollDataFrom)->readTest.capacity))) {
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->readTest.files, fdAndFileType, pollData->readTest.capacity);
            pollData->readTest.capacity = conv(pollDataFrom)->readTest.capacity;
            pollData->readTest.files = newReadFiles;
          } /* if */
        } /* if */
        if (pollData->writeTest.capacity < conv(pollDataFrom)->writeTest.size) {
          if (unlikely(!ALLOC_TABLE(newWriteFiles, fdAndFileType,
              conv(pollDataFrom)->writeTest.capacity) ||
              !replaceFdSet(&pollData->writeTest, conv(pollDataFrom)->writeTest.capacity))) {
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->writeTest.files, fdAndFileType, pollData->writeTest.capacity);
            pollData->writeTest.capacity = conv(pollDataFrom)->writeTest.capacity;
            pollData->writeTest.files = newWriteFiles;
          } /* if */
        } /* if */
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_read_inFdset(pollData), to_read_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->readTest);
#ifdef SELECT_WITH_NFDS
        pollData->readTest.preparedNfds = conv(pollDataFrom)->readTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_read_outFdset(pollData), to_read_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->readTest);
        pollData->readTest.size = conv(pollDataFrom)->readTest.size;
        pollData->readTest.iterPos = conv(pollDataFrom)->readTest.iterPos;
        memcpy(pollData->readTest.files, conv(pollDataFrom)->readTest.files,
            conv(pollDataFrom)->readTest.size * sizeof(fdAndFileType));
        hshDestr(pollData->readTest.indexHash, (destrfunctype) &intDestrGeneric,
                 (destrfunctype) &intDestrGeneric);
        pollData->readTest.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_write_inFdset(pollData), to_write_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeTest);
#ifdef SELECT_WITH_NFDS
        pollData->writeTest.preparedNfds = conv(pollDataFrom)->writeTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_write_outFdset(pollData), to_write_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeTest);
        pollData->writeTest.size = conv(pollDataFrom)->writeTest.size;
        pollData->writeTest.iterPos = conv(pollDataFrom)->writeTest.iterPos;
        memcpy(pollData->writeTest.files, conv(pollDataFrom)->writeTest.files,
               conv(pollDataFrom)->writeTest.size * sizeof(fdAndFileType));
        hshDestr(pollData->writeTest.indexHash, (destrfunctype) &intDestrGeneric,
                 (destrfunctype) &intDestrGeneric);
        pollData->writeTest.indexHash = newWriteIndexHash;
        pollData->iteratorMode = conv(pollDataFrom)->iteratorMode;
        pollData->iterEvents = conv(pollDataFrom)->iterEvents;
        pollData->numOfEvents = conv(pollDataFrom)->numOfEvents;
      } /* if */
    } /* if */
    /* printf("end polCpy:\n");
       dumpPoll(poll_to); */
  } /* polCpy */



#ifdef ANSI_C

polltype polCreate (const const_polltype pollDataFrom)
#else

polltype polCreate (pollDataFrom)
polltype pollDataFrom;
#endif

  {
    rtlHashtype newReadIndexHash;
    rtlHashtype newWriteIndexHash;
    select_based_polltype result;

  /* polCreate */
    /* printf("polCreate\n"); */
    newReadIndexHash = hshCreate(conv(pollDataFrom)->readTest.indexHash,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
    newWriteIndexHash = hshCreate(conv(pollDataFrom)->writeTest.indexHash,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
    if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL ||
                 !ALLOC_RECORD(result, select_based_pollrecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
#ifdef DO_HEAP_STATISTIC
      count.size_pollrecord = sizeof(select_based_pollrecord);
#endif
      if (unlikely(!ALLOC_TABLE(result->readTest.files, fdAndFileType,
                                conv(pollDataFrom)->readTest.capacity) ||
                   !ALLOC_TABLE(result->writeTest.files, fdAndFileType,
                                conv(pollDataFrom)->writeTest.capacity) ||
                   !allocFdSet(&result->readTest, conv(pollDataFrom)->readTest.capacity) ||
                   !allocFdSet(&result->writeTest, conv(pollDataFrom)->writeTest.capacity))) {
        if (result->readTest.files != NULL) {
          FREE_TABLE(result->readTest.files, fdAndFileType, conv(pollDataFrom)->readTest.capacity);
          if (result->writeTest.files != NULL) {
            FREE_TABLE(result->writeTest.files, fdAndFileType, conv(pollDataFrom)->readTest.capacity);
            freeFdSet(&result->readTest, conv(pollDataFrom)->readTest.capacity);
          } /* if */
        } /* if */
        FREE_RECORD(result, select_based_pollrecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_read_inFdset(result), to_read_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->readTest);
#ifdef SELECT_WITH_NFDS
        result->readTest.preparedNfds = conv(pollDataFrom)->readTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_read_outFdset(result), to_read_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->readTest);
        result->readTest.size = conv(pollDataFrom)->readTest.size;
        result->readTest.capacity = conv(pollDataFrom)->readTest.capacity;
        result->readTest.iterPos = conv(pollDataFrom)->readTest.iterPos;
        memcpy(result->readTest.files, conv(pollDataFrom)->readTest.files,
               conv(pollDataFrom)->readTest.size * sizeof(fdAndFileType));
        result->readTest.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_write_inFdset(result), to_write_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeTest);
#ifdef SELECT_WITH_NFDS
        result->writeTest.preparedNfds = conv(pollDataFrom)->writeTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_write_outFdset(result), to_write_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeTest);
        result->writeTest.size = conv(pollDataFrom)->writeTest.size;
        result->writeTest.capacity = conv(pollDataFrom)->writeTest.capacity;
        result->writeTest.iterPos = conv(pollDataFrom)->writeTest.iterPos;
        memcpy(result->writeTest.files, conv(pollDataFrom)->writeTest.files,
               conv(pollDataFrom)->writeTest.size * sizeof(fdAndFileType));
        result->writeTest.indexHash = newWriteIndexHash;
        result->iteratorMode = conv(pollDataFrom)->iteratorMode;
        result->iterEvents = conv(pollDataFrom)->iterEvents;
        result->numOfEvents = conv(pollDataFrom)->numOfEvents;
      } /* if */
    } /* if */
    /* printf("end polCreate:\n");
       dumpPoll((polltype) result); */
    return (polltype) result;
  } /* polCreate */



#ifdef ANSI_C

void polDestr (const polltype oldPollData)
#else

void polDestr (oldPollData)
polltype oldPollData;
#endif

  {
    memsizetype capacity;

  /* polDestr */
    if (oldPollData != NULL) {
      capacity = conv(oldPollData)->readTest.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->readTest.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->readTest.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->readTest.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->readTest.indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      capacity = conv(oldPollData)->writeTest.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->writeTest.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->writeTest.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->writeTest.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->writeTest.indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      FREE_RECORD(var_conv(oldPollData), select_based_pollrecord, count.polldata);
    } /* if */
  } /* polDestr */



#ifdef ANSI_C

polltype polEmpty (void)
#else

polltype polEmpty ()
#endif

  {
    rtlHashtype newReadIndexHash;
    rtlHashtype newWriteIndexHash;
    select_based_polltype result;

  /* polEmpty */
    /* printf("polEmpty()\n"); */
    newReadIndexHash = hshEmpty();
    newWriteIndexHash = hshEmpty();
    if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL ||
                 !ALLOC_RECORD(result, select_based_pollrecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
#ifdef DO_HEAP_STATISTIC
      count.size_pollrecord = sizeof(select_based_pollrecord);
#endif
      if (unlikely(!ALLOC_TABLE(result->readTest.files, fdAndFileType, TABLE_START_SIZE) ||
                   !ALLOC_TABLE(result->writeTest.files, fdAndFileType, TABLE_START_SIZE) ||
                   !allocFdSet(&result->readTest, TABLE_START_SIZE) ||
                   !allocFdSet(&result->writeTest, TABLE_START_SIZE))) {
        if (result->readTest.files != NULL) {
          FREE_TABLE(result->readTest.files, fdAndFileType, TABLE_START_SIZE);
          if (result->writeTest.files != NULL) {
            FREE_TABLE(result->writeTest.files, fdAndFileType, TABLE_START_SIZE);
            freeFdSet(&result->readTest, TABLE_START_SIZE);
          } /* if */
        } /* if */
        FREE_RECORD(result, select_based_pollrecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
#ifdef USE_PREPARED_FD_SET
        FD_ZERO(to_read_inFdset(result));
        FD_ZERO(to_write_inFdset(result));
#ifdef SELECT_WITH_NFDS
        result->readTest.preparedNfds = 0;
        result->writeTest.preparedNfds = 0;
#endif
#endif
        FD_ZERO(to_read_outFdset(result));
        FD_ZERO(to_write_outFdset(result));
        result->readTest.size = 0;
        result->readTest.capacity = TABLE_START_SIZE;
        result->readTest.iterPos = 0;
        result->readTest.indexHash = newReadIndexHash;
        result->writeTest.size = 0;
        result->writeTest.capacity = TABLE_START_SIZE;
        result->writeTest.iterPos = 0;
        result->writeTest.indexHash = newWriteIndexHash;
        result->iteratorMode = ITER_EMPTY;
        result->iterEvents = 0;
        result->numOfEvents = 0;
      } /* if */
    } /* if */
    /* printf("end polEmpty:\n");
       dumpPoll((polltype) result); */
    return (polltype) result;
  } /* polEmpty */



#ifdef ANSI_C

inttype polGetCheck (const const_polltype pollData, const sockettype aSocket)
#else

inttype polGetCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    inttype result;

  /* polGetCheck */
    if (isChecked(&conv(pollData)->readTest, aSocket)) {
      if (isChecked(&conv(pollData)->writeTest, aSocket)) {
        result = POLL_INOUT;
      } else {
        result = POLL_IN;
      } /* if */
    } else if (isChecked(&conv(pollData)->writeTest, aSocket)) {
      result = POLL_OUT;
    } else {
      result = POLL_NOTHING;
    } /* if */
    return result;
  } /* polGetCheck */



#ifdef ANSI_C

inttype polGetFinding (const const_polltype pollData, const sockettype aSocket)
#else

inttype polGetFinding (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    inttype result;

  /* polGetFinding */
    if (isReady(&conv(pollData)->readTest, aSocket)) {
      if (isReady(&conv(pollData)->writeTest, aSocket)) {
        result = POLL_INOUT;
      } else {
        result = POLL_IN;
      } /* if */
    } else if (isReady(&conv(pollData)->writeTest, aSocket)) {
      result = POLL_OUT;
    } else {
      result = POLL_NOTHING;
    } /* if */
    return result;
  } /* polGetFinding */



#ifdef ANSI_C

booltype polHasNext (const polltype pollData)
#else

booltype polHasNext (pollData)
polltype pollData;
#endif

  { /* polHasNext */
    switch (conv(pollData)->iteratorMode) {
      case ITER_CHECKS_IN:
        return hasNextCheck(&conv(pollData)->readTest);
      case ITER_CHECKS_OUT:
        return hasNextCheck(&conv(pollData)->writeTest);
      case ITER_CHECKS_INOUT:
        return hasNextCheck(&conv(pollData)->readTest) ||
               hasNextCheck(&conv(pollData)->writeTest);
      case ITER_FINDINGS_IN:
        return hasNextFinding(&var_conv(pollData)->readTest,
                              conv(pollData)->iterEvents);
      case ITER_FINDINGS_OUT:
        return hasNextFinding(&var_conv(pollData)->writeTest,
                              conv(pollData)->iterEvents);
      case ITER_FINDINGS_INOUT:
        return hasNextFinding(&var_conv(pollData)->readTest,
                              conv(pollData)->iterEvents) ||
               hasNextFinding(&var_conv(pollData)->writeTest,
                              conv(pollData)->iterEvents);
      case ITER_EMPTY:
      default:
        return FALSE;
    } /* switch */
  } /* polHasNext */



#ifdef ANSI_C

void polIterChecks (const polltype pollData, inttype pollMode)
#else

void polIterChecks (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterChecks */
    switch (pollMode) {
      case POLL_NOTHING:
        var_conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_IN;
        var_conv(pollData)->readTest.iterPos = 0;
        break;
      case POLL_OUT:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_OUT;
        var_conv(pollData)->writeTest.iterPos = 0;
        break;
      case POLL_INOUT:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_INOUT;
        var_conv(pollData)->readTest.iterPos = 0;
        var_conv(pollData)->writeTest.iterPos = 0;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* polIterChecks */



#ifdef ANSI_C

void polIterFindings (const polltype pollData, inttype pollMode)
#else

void polIterFindings (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterFindings */
    switch (pollMode) {
      case POLL_NOTHING:
        var_conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_IN;
        var_conv(pollData)->readTest.iterPos = 0;
        break;
      case POLL_OUT:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_OUT;
        var_conv(pollData)->writeTest.iterPos = 0;
        break;
      case POLL_INOUT:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_INOUT;
        var_conv(pollData)->readTest.iterPos = 0;
        var_conv(pollData)->writeTest.iterPos = 0;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
    var_conv(pollData)->iterEvents = conv(pollData)->numOfEvents;
  } /* polIterFindings */



#ifdef ANSI_C

rtlGenerictype polNextFile (const polltype pollData, const rtlGenerictype nullFile)
#else

rtlGenerictype polNextFile (pollData, nullFile)
polltype pollData;
rtlGenerictype nullFile;
#endif

  { /* polNextFile */
    switch (conv(pollData)->iteratorMode) {
      case ITER_CHECKS_IN:
        return nextCheck(&var_conv(pollData)->readTest, nullFile);
      case ITER_CHECKS_OUT:
        return nextCheck(&var_conv(pollData)->writeTest, nullFile);
      case ITER_CHECKS_INOUT:
        if (hasNextCheck(&conv(pollData)->readTest)) {
          return nextCheck(&var_conv(pollData)->readTest, nullFile);
        } else {
          return nextCheck(&var_conv(pollData)->writeTest, nullFile);
        } /* if */
      case ITER_FINDINGS_IN:
        return nextFinding(&var_conv(pollData)->readTest,
                           &var_conv(pollData)->iterEvents, nullFile);
      case ITER_FINDINGS_OUT:
        return nextFinding(&var_conv(pollData)->writeTest,
                           &var_conv(pollData)->iterEvents, nullFile);
      case ITER_FINDINGS_INOUT:
        if (hasNextFinding(&var_conv(pollData)->readTest,
                           conv(pollData)->iterEvents)) {
          return nextFinding(&var_conv(pollData)->readTest,
                             &var_conv(pollData)->iterEvents, nullFile);
        } else {
          return nextFinding(&var_conv(pollData)->writeTest,
                             &var_conv(pollData)->iterEvents, nullFile);
        } /* if */
      case ITER_EMPTY:
      default:
        return nullFile;
    } /* switch */
  } /* polNextFile */



#ifdef ANSI_C

void polPoll (const polltype pollData)
#else

void polPoll (pollData)
polltype pollData;
#endif

  { /* polPoll */
    doPoll(pollData, NULL);
  } /* polPoll */



/**
 *  Remove ''eventsToCheck'' for ''aSocket'' from ''pollData''.
 */
#ifdef ANSI_C

void polRemoveCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck)
#else

void polRemoveCheck (pollData, aSocket, eventsToCheck)
polltype pollData;
sockettype aSocket;
inttype eventsToCheck;
#endif

  { /* polRemoveCheck */
    switch (eventsToCheck) {
      case POLL_IN:
        removeCheck(&var_conv(pollData)->readTest, aSocket);
        break;
      case POLL_OUT:
        removeCheck(&var_conv(pollData)->writeTest, aSocket);
        break;
      case POLL_INOUT:
        removeCheck(&var_conv(pollData)->readTest, aSocket);
        removeCheck(&var_conv(pollData)->writeTest, aSocket);
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* polRemoveCheck */
