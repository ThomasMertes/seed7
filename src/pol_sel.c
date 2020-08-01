/********************************************************************/
/*                                                                  */
/*  pol_sel.c     Poll type and function based on select function.  */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 2011, 2013  Thomas Mertes                              */
/*  Content: Poll type and function based on select function.       */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
#include "soc_rtl.h"
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
    os_socketType fd;
    genericType file;
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
    memSizeType size;
    memSizeType capacity;
    memSizeType iterPos;
    fdAndFileType *files;
    rtlHashType indexHash;
  } testType;

typedef struct {
    testType readTest;
    testType writeTest;
    iteratorType iteratorMode;
    memSizeType iterEvents;
    memSizeType numOfEvents;
  } select_based_pollRecord, *select_based_pollType;

typedef const select_based_pollRecord *const_select_based_pollType;

#if DO_HEAP_STATISTIC
size_t sizeof_pollRecord = sizeof(select_based_pollRecord);
#endif

#define conv(genericPollData) ((const_select_based_pollType) genericPollData)
#define var_conv(genericPollData) ((select_based_pollType) genericPollData)


#ifdef DYNAMIC_FD_SET

#define to_inFdset(test)            ((test)->inFdset)
#define to_outFdset(test)           ((test)->outFdset)
#define to_read_inFdset(data)       (((const_select_based_pollType) data)->readTest.inFdset)
#define to_write_inFdset(data)      (((const_select_based_pollType) data)->writeTest.inFdset)
#define to_read_outFdset(data)      (((const_select_based_pollType) data)->readTest.outFdset)
#define to_write_outFdset(data)     (((const_select_based_pollType) data)->writeTest.outFdset)
#define to_var_read_inFdset(data)   (((select_based_pollType) data)->readTest.inFdset)
#define to_var_write_inFdset(data)  (((select_based_pollType) data)->writeTest.inFdset)
#define to_var_read_outFdset(data)  (((select_based_pollType) data)->readTest.outFdset)
#define to_var_write_outFdset(data) (((select_based_pollType) data)->writeTest.outFdset)

#define ALLOC_FDSET(var,nr)      (ALLOC_HEAP(var, fd_set *, SIZEOF_FD_SET(nr))?CNT(CNT1_BYT(SIZEOF_FD_SET(nr))) TRUE:FALSE)
#define FREE_FDSET(var,nr)       (CNT(CNT2_BYT(SIZEOF_FD_SET(nr))) FREE_HEAP(var, SIZEOF_FD_SET(nr)))
#define REALLOC_FDSET(var,nr)    REALLOC_HEAP(var, fd_set *, SIZEOF_FD_SET(nr))
#define COUNT3_FDSET(nr1,nr2)    CNT3(CNT2_BYT(SIZEOF_FD_SET(nr1)), CNT1_BYT(SIZEOF_FD_SET(nr2)))

#else

#define to_inFdset(test)            (&(test)->inFdset)
#define to_outFdset(test)           (&(test)->outFdset)
#define to_read_inFdset(data)       (&((const_select_based_pollType) data)->readTest.inFdset)
#define to_write_inFdset(data)      (&((const_select_based_pollType) data)->writeTest.inFdset)
#define to_read_outFdset(data)      (&((const_select_based_pollType) data)->readTest.outFdset)
#define to_write_outFdset(data)     (&((const_select_based_pollType) data)->writeTest.outFdset)
#define to_var_read_inFdset(data)   (&((select_based_pollType) data)->readTest.inFdset)
#define to_var_write_inFdset(data)  (&((select_based_pollType) data)->writeTest.inFdset)
#define to_var_read_outFdset(data)  (&((select_based_pollType) data)->readTest.outFdset)
#define to_var_write_outFdset(data) (&((select_based_pollType) data)->writeTest.outFdset)

#endif


#define TABLE_START_SIZE  256
#define TABLE_INCREMENT  1024



#ifdef OUT_OF_ORDER
static void dumpPoll (const const_pollType pollData)
    {
      memSizeType pos;
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



void initPollOperations (const createFuncType incrUsageCount,
    const destrFuncType decrUsageCount)

  { /* initPollOperations */
    fileObjectOps.incrUsageCount = incrUsageCount;
    fileObjectOps.decrUsageCount = decrUsageCount;
  } /* initPollOperations */



#ifdef MEMCPY_FD_SET
#define copyFdSet(dest, source, unused) memcpy(dest, source, USED_FD_SET_SIZE(source))
#else

static void copyFdSet (fd_set *dest, const fd_set *source, testType *test)

  {
    memSizeType pos;

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

static boolType allocFdSet (testType *test, memSizeType capacity)

  {
    boolType result = TRUE;

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

static void freeFdSet (testType *test, memSizeType capacity)

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

static boolType reallocFdSet (testType *test, memSizeType increment)

  {
    fd_set *newFdset;
    boolType result = TRUE;

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

static boolType replaceFdSet (testType *test, memSizeType capacity)

  {
#ifdef USE_PREPARED_FD_SET
    fd_set *newInFdset;
#endif
    fd_set *newOutFdset;
    boolType result = TRUE;

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



static void addCheck (testType *test, const socketType aSocket,
    const genericType fileObj)

  {
    memSizeType pos;

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
    pos = (memSizeType) hshIdxEnterDefault(test->indexHash,
        (genericType) (usocketType) aSocket, (genericType) test->size,
        (intType) aSocket, (compareType) &genericCmp,
        (createFuncType) &genericCreate, (createFuncType) &genericCreate);
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
      test->files[pos].fd = (os_socketType) aSocket;
      test->files[pos].file = fileObjectOps.incrUsageCount(fileObj);
#ifdef USE_PREPARED_FD_SET
      FD_SET((os_socketType) aSocket, to_inFdset(test));
#ifdef SELECT_WITH_NFDS
      if (aSocket >= test->preparedNfds) {
        test->preparedNfds = (int) aSocket + 1;
      } /* if */
#endif
#endif
    } /* if */
  } /* addCheck */



static void removeCheck (testType *test, const socketType aSocket)

  {
    memSizeType pos;

  /* removeCheck */
    /* printf("removeCheck(..., %u)\n", aSocket); */
    pos = (memSizeType) hshIdxWithDefault(test->indexHash,
        (genericType) (usocketType) aSocket, (genericType) test->size,
        (intType) aSocket, (compareType) &genericCmp);
    if (pos != test->size) {
      fileObjectOps.decrUsageCount(test->files[pos].file);
      if (pos + 1 <= test->iterPos) {
        test->iterPos--;
        if (pos < test->iterPos) {
          memcpy(&test->files[pos],
                 &test->files[test->iterPos], sizeof(fdAndFileType));
          hshIdxAddr(test->indexHash,
                     (genericType) (usocketType) test->files[pos].fd,
                     (intType) (socketType) test->files[pos].fd,
                     (compareType) &genericCmp)->value.genericValue = (genericType) pos;
          pos = test->iterPos;
        } /* if */
      } /* if */
      test->size--;
      if (pos < test->size) {
        memcpy(&test->files[pos],
               &test->files[test->size], sizeof(fdAndFileType));
        hshIdxAddr(test->indexHash,
                   (genericType) (usocketType) test->files[pos].fd,
                   (intType) (socketType) test->files[pos].fd,
                   (compareType) &genericCmp)->value.genericValue = (genericType) pos;
      } /* if */
      hshExcl(test->indexHash, (genericType) (usocketType) aSocket,
              (intType) aSocket, (compareType) &genericCmp,
              (destrFuncType) &genericDestr, (destrFuncType) &genericDestr);
#ifdef USE_PREPARED_FD_SET
      FD_CLR((os_socketType) aSocket, to_inFdset(test));
#endif
    } /* if */
  } /* removeCheck */



static void doPoll (const pollType pollData, struct timeval *timeout)

  {
    int nfds = 0;
    fd_set *readFds;
    fd_set *writeFds;
#ifndef USE_PREPARED_FD_SET
    memSizeType pos;
    os_socketType sock;
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
    do {
      select_result = select(nfds, readFds, writeFds, NULL, timeout);
    } while (unlikely(select_result == -1 && errno == EINTR));
    if (unlikely(select_result < 0)) {
      logError(printf("doPoll: select(%d, *, *, NULL, " FMT_U_MEM ") failed:\n"
                      "%s=%d\nerror: %s\n",
                      nfds, timeout, ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } else {
      var_conv(pollData)->readTest.iterPos = 0;
      var_conv(pollData)->writeTest.iterPos = 0;
      var_conv(pollData)->numOfEvents = (memSizeType) select_result;
    } /* if */
  } /* doPoll */



static boolType isChecked (const testType *test, const socketType aSocket)

  {
    memSizeType pos;
    boolType result;

  /* isChecked */
    pos = (memSizeType) hshIdxWithDefault(test->indexHash,
        (genericType) (usocketType) aSocket, (genericType) test->size,
        (intType) aSocket, (compareType) &genericCmp);
    result = pos != test->size;
    /* printf("isChecked: sock=%d, pos=%d, fd=%d\n",
        aSocket, pos, test->files[pos].fd); */
    return result;
  } /* isChecked */



static boolType isReady (const testType *test, const socketType aSocket)

  {
    memSizeType pos;
    boolType result;

  /* isReady */
    pos = (memSizeType) hshIdxWithDefault(test->indexHash,
        (genericType) (usocketType) aSocket, (genericType) test->size,
        (intType) aSocket, (compareType) &genericCmp);
    if (pos == test->size) {
      result = FALSE;
    } else {
      result = FD_ISSET(test->files[pos].fd, to_outFdset(test)) != 0;
      /* printf("isReady: sock=%d, pos=%d, fd=%d\n",
          aSocket, pos, test->files[pos].fd); */
    } /* if */
    return result;
  } /* isReady */



static boolType hasNextCheck (const testType *test)

  { /* hasNextCheck */
    return test->iterPos < test->size;
  } /* hasNextCheck */



static boolType hasNextFinding (testType *test, memSizeType iterEvents)

  {
    memSizeType pos;
    fd_set *fdSet;
    boolType hasNext;

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



static genericType nextCheck (testType *test,
    const genericType nullFile)

  {
    memSizeType pos;
    genericType checkFile;

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



static genericType nextFinding (testType *test,
    memSizeType *iterEvents, const genericType nullFile)

  {
    memSizeType pos;
    fd_set *fdSet;
    genericType resultFile;

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
 *  Add 'eventsToCheck' for 'aSocket' to 'pollData'.
 *  'EventsToCheck' can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData Poll data to which the event checks are added.
 *  @param aSocket Socket for which the events should be checked.
 *  @param eventsToCheck Events to be added to the checkedEvents
 *         field of 'pollData'.
 *  @param fileObj File to be returned, when the iterator returns
 *         files in 'pollData'.
 *  @exception RANGE_ERROR Illegal value for 'eventsToCheck'.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 *  @exception FILE_ERROR A limit of the operating system was reached.
 */
void polAddCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck, const genericType fileObj)

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



/**
 *  Clears 'pollData'.
 *  All sockets and all events are removed from 'pollData' and
 *  the iterator is reset, such that polHasNext() returns FALSE.
 */
void polClear (const pollType pollData)

  {
    memSizeType pos;

  /* polClear */
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
    /* Clear readTest */
    for (pos = 0; pos < conv(pollData)->readTest.size; pos++) {
      fileObjectOps.decrUsageCount(conv(pollData)->readTest.files[pos].file);
    } /* for */
    var_conv(pollData)->readTest.size = 0;
    var_conv(pollData)->readTest.iterPos = 0;
    hshDestr(conv(pollData)->readTest.indexHash, (destrFuncType) &genericDestr,
             (destrFuncType) &genericDestr);
    var_conv(pollData)->readTest.indexHash = hshEmpty();
    /* Clear writeTest */
    for (pos = 0; pos < conv(pollData)->readTest.size; pos++) {
      fileObjectOps.decrUsageCount(conv(pollData)->readTest.files[pos].file);
    } /* for */
    var_conv(pollData)->writeTest.size = 0;
    var_conv(pollData)->writeTest.iterPos = 0;
    hshDestr(conv(pollData)->writeTest.indexHash, (destrFuncType) &genericDestr,
             (destrFuncType) &genericDestr);
    var_conv(pollData)->writeTest.indexHash = hshEmpty();
    var_conv(pollData)->iteratorMode = ITER_EMPTY;
    var_conv(pollData)->iterEvents = 0;
    var_conv(pollData)->numOfEvents = 0;
  } /* polClear */



/**
 *  Assign source to dest.
 *  A copy function assumes that dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void polCpy (const pollType dest, const const_pollType source)

  {
    select_based_pollType pollData;
    rtlHashType newReadIndexHash;
    rtlHashType newWriteIndexHash;
    memSizeType newReadFilesCapacity;
    fdAndFileType *newReadFiles;
    fdAndFileType *oldReadFiles;
    memSizeType oldReadFilesSize;
    memSizeType oldReadFilesCapacity;
    memSizeType newWriteFilesCapacity;
    fdAndFileType *newWriteFiles;
    fdAndFileType *oldWriteFiles;
    memSizeType oldWriteFilesSize;
    memSizeType oldWriteFilesCapacity;
    memSizeType pos;

  /* polCpy */
    logFunction(printf("polCpy(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) dest, (memSizeType) source););
    if (dest != source) {
      newReadIndexHash = hshCreate(conv(source)->readTest.indexHash,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
      newWriteIndexHash = hshCreate(conv(source)->writeTest.indexHash,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
      if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        pollData = var_conv(dest);
        oldReadFiles = pollData->readTest.files;
        oldReadFilesSize = pollData->readTest.size;
        oldReadFilesCapacity = pollData->readTest.capacity;
        if (pollData->readTest.capacity < conv(source)->readTest.size) {
          newReadFilesCapacity = conv(source)->readTest.capacity;
        } else {
          newReadFilesCapacity = pollData->readTest.capacity;
        } /* if */
        if (unlikely(!ALLOC_TABLE(newReadFiles, fdAndFileType, newReadFilesCapacity))) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (pollData->readTest.capacity < conv(source)->readTest.size) {
            if (unlikely(!replaceFdSet(&pollData->readTest, conv(source)->readTest.capacity))) {
              FREE_TABLE(newReadFiles, fdAndFileType, newReadFilesCapacity);
              raise_error(MEMORY_ERROR);
              return;
            } else {
              pollData->readTest.capacity = conv(source)->readTest.capacity;
            } /* if */
          } /* if */
        } /* if */
        oldWriteFiles = pollData->writeTest.files;
        oldWriteFilesSize = pollData->writeTest.size;
        oldWriteFilesCapacity = pollData->writeTest.capacity;
        if (pollData->writeTest.capacity < conv(source)->writeTest.size) {
          newWriteFilesCapacity = conv(source)->writeTest.capacity;
        } else {
          newWriteFilesCapacity = pollData->writeTest.capacity;
        } /* if */
        if (unlikely(!ALLOC_TABLE(newWriteFiles, fdAndFileType, newWriteFilesCapacity))) {
          FREE_TABLE(newReadFiles, fdAndFileType, newReadFilesCapacity);
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (pollData->writeTest.capacity < conv(source)->writeTest.size) {
            if (unlikely(!replaceFdSet(&pollData->writeTest, conv(source)->writeTest.capacity))) {
              FREE_TABLE(newReadFiles, fdAndFileType, newReadFilesCapacity);
              FREE_TABLE(newWriteFiles, fdAndFileType, newWriteFilesCapacity);
              raise_error(MEMORY_ERROR);
              return;
            } else {
              pollData->writeTest.capacity = conv(source)->writeTest.capacity;
            } /* if */
          } /* if */
        } /* if */
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_read_inFdset(pollData), to_read_inFdset(source),
                  &conv(source)->readTest);
#ifdef SELECT_WITH_NFDS
        pollData->readTest.preparedNfds = conv(source)->readTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_read_outFdset(pollData), to_read_outFdset(source),
                  &conv(source)->readTest);
        pollData->readTest.size = conv(source)->readTest.size;
        pollData->readTest.files = newReadFiles;
        pollData->readTest.iterPos = conv(source)->readTest.iterPos;
        hshDestr(pollData->readTest.indexHash, (destrFuncType) &genericDestr,
                 (destrFuncType) &genericDestr);
        pollData->readTest.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_write_inFdset(pollData), to_write_inFdset(source),
                  &conv(source)->writeTest);
#ifdef SELECT_WITH_NFDS
        pollData->writeTest.preparedNfds = conv(source)->writeTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_write_outFdset(pollData), to_write_outFdset(source),
                  &conv(source)->writeTest);
        pollData->writeTest.size = conv(source)->writeTest.size;
        pollData->writeTest.files = newWriteFiles;
        pollData->writeTest.iterPos = conv(source)->writeTest.iterPos;
        hshDestr(pollData->writeTest.indexHash, (destrFuncType) &genericDestr,
                 (destrFuncType) &genericDestr);
        pollData->writeTest.indexHash = newWriteIndexHash;
        pollData->iteratorMode = conv(source)->iteratorMode;
        pollData->iterEvents = conv(source)->iterEvents;
        pollData->numOfEvents = conv(source)->numOfEvents;
        for (pos = 0; pos < conv(source)->readTest.size; pos++) {
          newReadFiles[pos].fd = conv(source)->readTest.files[pos].fd;
          newReadFiles[pos].file =
              fileObjectOps.incrUsageCount(conv(source)->readTest.files[pos].file);
        } /* for */
        for (pos = 0; pos < conv(source)->writeTest.size; pos++) {
          newWriteFiles[pos].fd = conv(source)->writeTest.files[pos].fd;
          newWriteFiles[pos].file =
              fileObjectOps.incrUsageCount(conv(source)->writeTest.files[pos].file);
        } /* for */
        for (pos = 0; pos < oldReadFilesSize; pos++) {
          fileObjectOps.decrUsageCount(oldReadFiles[pos].file);
        } /* for */
        for (pos = 0; pos < oldWriteFilesSize; pos++) {
          fileObjectOps.decrUsageCount(oldWriteFiles[pos].file);
        } /* for */
        FREE_TABLE(oldReadFiles, fdAndFileType, oldReadFilesCapacity);
        FREE_TABLE(oldWriteFiles, fdAndFileType, oldWriteFilesCapacity);
      } /* if */
    } /* if */
    logFunction(printf("polCpy -->\n"););
    /* dumpPoll(dest); */
  } /* polCpy */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
pollType polCreate (const const_pollType source)

  {
    rtlHashType newReadIndexHash;
    rtlHashType newWriteIndexHash;
    memSizeType pos;
    select_based_pollType result;

  /* polCreate */
    logFunction(printf("polCreate(" FMT_U_MEM ")\n",
                       (memSizeType) source););
    newReadIndexHash = hshCreate(conv(source)->readTest.indexHash,
        (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
        (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
    newWriteIndexHash = hshCreate(conv(source)->writeTest.indexHash,
        (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
        (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
    if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL ||
                 !ALLOC_RECORD(result, select_based_pollRecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_TABLE(result->readTest.files, fdAndFileType,
                                conv(source)->readTest.capacity) ||
                   !ALLOC_TABLE(result->writeTest.files, fdAndFileType,
                                conv(source)->writeTest.capacity) ||
                   !allocFdSet(&result->readTest, conv(source)->readTest.capacity) ||
                   !allocFdSet(&result->writeTest, conv(source)->writeTest.capacity))) {
        if (result->readTest.files != NULL) {
          FREE_TABLE(result->readTest.files, fdAndFileType, conv(source)->readTest.capacity);
          if (result->writeTest.files != NULL) {
            FREE_TABLE(result->writeTest.files, fdAndFileType, conv(source)->readTest.capacity);
            freeFdSet(&result->readTest, conv(source)->readTest.capacity);
          } /* if */
        } /* if */
        FREE_RECORD(result, select_based_pollRecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_read_inFdset(result), to_read_inFdset(source),
                  &conv(source)->readTest);
#ifdef SELECT_WITH_NFDS
        result->readTest.preparedNfds = conv(source)->readTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_read_outFdset(result), to_read_outFdset(source),
                  &conv(source)->readTest);
        result->readTest.size = conv(source)->readTest.size;
        result->readTest.capacity = conv(source)->readTest.capacity;
        result->readTest.iterPos = conv(source)->readTest.iterPos;
        for (pos = 0; pos < conv(source)->readTest.size; pos++) {
          result->readTest.files[pos].fd = conv(source)->readTest.files[pos].fd;
          result->readTest.files[pos].file =
              fileObjectOps.incrUsageCount(conv(source)->readTest.files[pos].file);
        } /* for */
        result->readTest.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_var_write_inFdset(result), to_write_inFdset(source),
                  &conv(source)->writeTest);
#ifdef SELECT_WITH_NFDS
        result->writeTest.preparedNfds = conv(source)->writeTest.preparedNfds;
#endif
#endif
        copyFdSet(to_var_write_outFdset(result), to_write_outFdset(source),
                  &conv(source)->writeTest);
        result->writeTest.size = conv(source)->writeTest.size;
        result->writeTest.capacity = conv(source)->writeTest.capacity;
        result->writeTest.iterPos = conv(source)->writeTest.iterPos;
        for (pos = 0; pos < conv(source)->writeTest.size; pos++) {
          result->writeTest.files[pos].fd = conv(source)->writeTest.files[pos].fd;
          result->writeTest.files[pos].file =
              fileObjectOps.incrUsageCount(conv(source)->writeTest.files[pos].file);
        } /* for */
        result->writeTest.indexHash = newWriteIndexHash;
        result->iteratorMode = conv(source)->iteratorMode;
        result->iterEvents = conv(source)->iterEvents;
        result->numOfEvents = conv(source)->numOfEvents;
      } /* if */
    } /* if */
    logFunction(printf("polCreate -->\n"););
    /* dumpPoll((pollType) result); */
    return (pollType) result;
  } /* polCreate */



/**
 *  Free the memory referred by 'oldPollData'.
 *  After polDestr is left 'oldPollData' refers to not existing memory.
 *  The memory where 'oldPollData' is stored can be freed afterwards.
 */
void polDestr (const pollType oldPollData)

  {
    memSizeType capacity;
    memSizeType pos;

  /* polDestr */
    if (oldPollData != NULL) {
      /* Free readTest */
      for (pos = 0; pos < conv(oldPollData)->readTest.size; pos++) {
        fileObjectOps.decrUsageCount(conv(oldPollData)->readTest.files[pos].file);
      } /* for */
      capacity = conv(oldPollData)->readTest.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->readTest.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->readTest.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->readTest.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->readTest.indexHash, (destrFuncType) &genericDestr,
               (destrFuncType) &genericDestr);
      /* Free writeTest */
      for (pos = 0; pos < conv(oldPollData)->writeTest.size; pos++) {
        fileObjectOps.decrUsageCount(conv(oldPollData)->writeTest.files[pos].file);
      } /* for */
      capacity = conv(oldPollData)->writeTest.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->writeTest.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->writeTest.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->writeTest.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->writeTest.indexHash, (destrFuncType) &genericDestr,
               (destrFuncType) &genericDestr);
      FREE_RECORD(var_conv(oldPollData), select_based_pollRecord, count.polldata);
    } /* if */
  } /* polDestr */



/**
 *  Create an empty poll data value.
 *  @return an empty poll data value.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
pollType polEmpty (void)

  {
    rtlHashType newReadIndexHash;
    rtlHashType newWriteIndexHash;
    select_based_pollType result;

  /* polEmpty */
    /* printf("polEmpty()\n"); */
    newReadIndexHash = hshEmpty();
    newWriteIndexHash = hshEmpty();
    if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL ||
                 !ALLOC_RECORD(result, select_based_pollRecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
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
        FREE_RECORD(result, select_based_pollRecord, count.polldata);
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
       dumpPoll((pollType) result); */
    return (pollType) result;
  } /* polEmpty */



/**
 *  Return the checkedEvents field from 'pollData' for 'aSocket'.
 *  The polPoll function uses the checkedEvents as input.
 *  The following checkedEvents can be returned:
 *  - POLLNOTHING no data can be read or written.
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @param pollData Poll data from which the checkedEvents are
 *         retrieved.
 *  @param aSocket Socket for which the checkedEvents are retrived.
 *  @return POLLNOTHING, POLLIN, POLLOUT or POLLINOUT, depending on
 *          the events added and removed for 'aSocket' with
 *          'addCheck' and 'removeCheck'.
 */
intType polGetCheck (const const_pollType pollData, const socketType aSocket)

  {
    intType result;

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



/**
 *  Return the eventFindings field from 'pollData' for 'aSocket'.
 *  The polPoll function assigns the
 *  eventFindings for 'aSocket' to 'pollData'. The following
 *  eventFindings can be returned:
 *  - POLLNOTHING no data can be read or written.
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @return POLLNOTHING, POLLIN, POLLOUT or POLLINOUT, depending on
 *          the findings of polPoll concerning 'aSocket'.
 */
intType polGetFinding (const const_pollType pollData, const socketType aSocket)

  {
    intType result;

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



/**
 *  Determine if the 'pollData' iterator can deliver another file.
 *  @return TRUE if 'nextFile' would return another file from the
 *          'pollData' iterator, FALSE otherwise.
 */
boolType polHasNext (const pollType pollData)

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



/**
 *  Reset the 'pollData' iterator to process checkedEvents.
 *  The following calls of 'hasNext' and 'nextFile' refer to
 *  the checkedEvents of the given 'pollMode'. 'PollMode'
 *  can have one of the following values:
 *  - POLLNOTHING don't iterate ('hasNext' returns FALSE).
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read or written (POLLIN and POLLOUT).
 *
 *  @exception RANGE_ERROR Illegal value for 'pollMode'.
 */
void polIterChecks (const pollType pollData, intType pollMode)

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



/**
 *  Reset the 'pollData' iterator to process eventFindings.
 *  The following calls of 'hasNext' and 'nextFile' refer to
 *  the eventFindings of the given 'pollMode'. 'PollMode'
 *  can have one of the following values:
 *  - POLLNOTHING don't iterate ('hasNext' returns FALSE).
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read or written (POLLIN and POLLOUT).
 *
 *  @exception RANGE_ERROR Illegal value for 'pollMode'.
 */
void polIterFindings (const pollType pollData, intType pollMode)

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



/**
 *  Get the next file from the 'pollData' iterator.
 *  Successive calls of 'nextFile' return all files from the 'pollData'
 *  iterator. The file returned by 'nextFile' is determined with the
 *  function 'addCheck'. The files covered by the 'pollData' iterator
 *  are determined with 'iterChecks' or 'iterFindings'.
 *  @return the next file from the 'pollData' iterator, or STD_NULL,
 *          when no file from the 'pollData' iterator is available.
 */
genericType polNextFile (const pollType pollData, const genericType nullFile)

  {
    genericType nextFile;

  /* polNextFile */
    switch (conv(pollData)->iteratorMode) {
      case ITER_CHECKS_IN:
        nextFile = nextCheck(&var_conv(pollData)->readTest, nullFile);
        break;
      case ITER_CHECKS_OUT:
        nextFile = nextCheck(&var_conv(pollData)->writeTest, nullFile);
        break;
      case ITER_CHECKS_INOUT:
        if (hasNextCheck(&conv(pollData)->readTest)) {
          nextFile = nextCheck(&var_conv(pollData)->readTest, nullFile);
        } else {
          nextFile = nextCheck(&var_conv(pollData)->writeTest, nullFile);
        } /* if */
        break;
      case ITER_FINDINGS_IN:
        nextFile = nextFinding(&var_conv(pollData)->readTest,
                           &var_conv(pollData)->iterEvents, nullFile);
        break;
      case ITER_FINDINGS_OUT:
        nextFile = nextFinding(&var_conv(pollData)->writeTest,
                           &var_conv(pollData)->iterEvents, nullFile);
        break;
      case ITER_FINDINGS_INOUT:
        if (hasNextFinding(&var_conv(pollData)->readTest,
                           conv(pollData)->iterEvents)) {
          nextFile = nextFinding(&var_conv(pollData)->readTest,
                             &var_conv(pollData)->iterEvents, nullFile);
        } else {
          nextFile = nextFinding(&var_conv(pollData)->writeTest,
                             &var_conv(pollData)->iterEvents, nullFile);
        } /* if */
        break;
      case ITER_EMPTY:
      default:
        nextFile = nullFile;
        break;
    } /* switch */
    return nextFile;
  } /* polNextFile */



/**
 *  Waits for one or more of the checkedEvents from 'pollData'.
 *  polPoll waits until one of the checkedEvents for a
 *  corresponding socket occurs. When a checkedEvents occurs
 *  the eventFindings field is assigned a value. The following
 *  eventFindings values are assigned:
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @exception FILE_ERROR The system function returns an error.
 */
void polPoll (const pollType pollData)

  { /* polPoll */
    doPoll(pollData, NULL);
  } /* polPoll */



/**
 *  Remove 'eventsToCheck' for 'aSocket' from 'pollData'.
 *  'EventsToCheck' can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pData Poll data from which the event checks are removed.
 *  @param aSocket Socket for which the events should not be checked.
 *  @param eventsToCheck Events to be removed from the checkedEvents
 *         field of 'pData'.
 *  @exception RANGE_ERROR Illegal value for 'eventsToCheck'.
 */
 void polRemoveCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck)

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
