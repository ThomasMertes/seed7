/********************************************************************/
/*                                                                  */
/*  pol_unx.c     Poll type and function using UNIX capabilities.   */
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
/*  File: seed7/src/pol_unx.c                                       */
/*  Changes: 2011, 2013  Thomas Mertes                              */
/*  Content: Poll type and function using UNIX capabilities.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_WINSOCK
#include "winsock2.h"
/* #define os_poll WSAPoll  - available with Vista */
#else
#include "poll.h"
#endif
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "hsh_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "pol_drv.h"


#include "data.h"
typedef enum {
    ITER_EMPTY,
    ITER_CHECKS_IN, ITER_CHECKS_OUT, ITER_CHECKS_INOUT,
    ITER_FINDINGS_IN, ITER_FINDINGS_OUT, ITER_FINDINGS_INOUT
  } iteratorType;

typedef struct {
    memSizeType size;
    memSizeType capacity;
    iteratorType iteratorMode;
    memSizeType iterPos;
    memSizeType iterEvents;
    memSizeType numOfEvents;
    struct pollfd *pollFds;
    genericType *pollFiles;
    rtlHashType indexHash;
  } poll_based_pollRecord, *poll_based_pollType;

typedef const poll_based_pollRecord *const_poll_based_pollType;

#ifdef DO_HEAP_STATISTIC
size_t sizeof_pollRecord = sizeof(poll_based_pollRecord);
#endif

#define conv(genericPollData) ((const_poll_based_pollType) genericPollData)
#define var_conv(genericPollData) ((poll_based_pollType) genericPollData)


#define TABLE_START_SIZE    256
#define TABLE_INCREMENT    1024
#define NUM_OF_EXTRA_ELEMS    1
#define TERMINATING_REVENT   ~0



#ifdef OUT_OF_ORDER
void dumpPoll (const const_pollType pollData)
    {
      memSizeType pos;
      genericType pollFile;

      printf("size=%d\n", conv(pollData)->size);
      printf("capacity=%d\n", conv(pollData)->capacity);
      printf("iteratorMode=%d\n", conv(pollData)->iteratorMode);
      printf("iterPos=%d\n", conv(pollData)->iterPos);
      printf("iterEvents=%d\n", conv(pollData)->iterEvents);
      printf("numOfEvents=%d\n", conv(pollData)->numOfEvents);
      for (pos = 0; pos < conv(pollData)->size; pos++) {
        printf("pollfd[%d]: fd=%d, events=%o, revents=%o, pollFile=",
               pos,
               conv(pollData)->pollFds[pos].fd,
               conv(pollData)->pollFds[pos].events,
               conv(pollData)->pollFds[pos].revents);
        pollFile = conv(pollData)->pollFiles[pos];
        if (pollFile == NULL) {
          printf("NULL\n");
        } else {
          /* trace1((objectType) pollFile); */
          printf("struct[] usage_count=%lu", ((rtlStructType) pollFile)->usage_count);
          printf("\n");
        }
      }
    }
#endif



void initPollOperations (const createFuncType incrUsageCount,
    const destrFuncType decrUsageCount)

  { /* initPollOperations */
    fileObjectOps.incrUsageCount = incrUsageCount;
    fileObjectOps.decrUsageCount = decrUsageCount;
  } /* initPollOperations */



static void addCheck (const poll_based_pollType pollData, short eventsToCheck,
    const socketType aSocket, const genericType fileObj)

  {
    memSizeType pos;
    struct pollfd *aPollFd;

  /* addCheck */
    pos = (memSizeType) hshIdxEnterDefault(pollData->indexHash,
        (genericType) (usocketType) aSocket, (genericType) pollData->size,
        (intType) aSocket, (compareType) &genericCmp,
        (createFuncType) &genericCreate, (createFuncType) &genericCreate);
    if (pos == pollData->size) {
      if (pollData->size + NUM_OF_EXTRA_ELEMS >= pollData->capacity) {
        pollData->pollFds = REALLOC_TABLE(pollData->pollFds, struct pollfd,
            pollData->capacity, pollData->capacity + TABLE_INCREMENT);
        pollData->pollFiles = REALLOC_TABLE(pollData->pollFiles, genericType,
            pollData->capacity, pollData->capacity + TABLE_INCREMENT);
        if (pollData->pollFds == NULL || pollData->pollFiles == NULL) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_TABLE(struct pollfd, pollData->capacity, pollData->capacity + TABLE_INCREMENT);
          COUNT3_TABLE(genericType, pollData->capacity, pollData->capacity + TABLE_INCREMENT);
        } /* if */
        pollData->capacity += TABLE_INCREMENT;
      } /* if */
      pollData->size++;
      aPollFd = &pollData->pollFds[pos];
      memset(aPollFd, 0, sizeof(struct pollfd));
      aPollFd->fd = (int) aSocket;
      aPollFd->events = eventsToCheck;
      memset(&pollData->pollFds[pollData->size], 0, sizeof(struct pollfd));
      pollData->pollFds[pollData->size].revents = TERMINATING_REVENT;
      pollData->pollFiles[pos] = fileObjectOps.incrUsageCount(fileObj);
    } else {
      pollData->pollFds[pos].events |= eventsToCheck;
    } /* if */
    /* printf("end addCheck:\n");
       dumpPoll(pollData); */
  } /* addCheck */



static void removeCheck (const poll_based_pollType pollData, short eventsToCheck,
    const socketType aSocket)

  {
    memSizeType pos;
    struct pollfd *aPollFd;

  /* removeCheck */
    pos = (memSizeType) hshIdxWithDefault(pollData->indexHash,
        (genericType) (usocketType) aSocket, (genericType) pollData->size,
        (intType) aSocket, (compareType) &genericCmp);
    if (pos != pollData->size) {
      aPollFd = &pollData->pollFds[pos];
      aPollFd->events &= (short) ~eventsToCheck;
      if (aPollFd->events == 0) {
        fileObjectOps.decrUsageCount(pollData->pollFiles[pos]);
        if (pos + 1 <= pollData->iterPos) {
          pollData->iterPos--;
          if (pos < pollData->iterPos) {
            memcpy(&pollData->pollFds[pos],
                   &pollData->pollFds[pollData->iterPos], sizeof(struct pollfd));
            pollData->pollFiles[pos] = pollData->pollFiles[pollData->iterPos];
            hshIdxAddr(pollData->indexHash,
                       (genericType) (usocketType) pollData->pollFds[pos].fd,
                       (intType) (socketType) pollData->pollFds[pos].fd,
                       (compareType) &genericCmp)->value.genericValue = (genericType) pos;
            pos = pollData->iterPos;
          } /* if */
        } /* if */
        pollData->size--;
        if (pos < pollData->size) {
          memcpy(&pollData->pollFds[pos],
                 &pollData->pollFds[pollData->size], sizeof(struct pollfd));
          pollData->pollFiles[pos] = pollData->pollFiles[pollData->size];
          hshIdxAddr(pollData->indexHash,
                     (genericType) (usocketType) pollData->pollFds[pos].fd,
                     (intType) (socketType) pollData->pollFds[pos].fd,
                     (compareType) &genericCmp)->value.genericValue = (genericType) pos;
        } /* if */
        hshExcl(pollData->indexHash, (genericType) (usocketType) aSocket,
                (intType) aSocket, (compareType) &genericCmp,
                (destrFuncType) &genericDestr, (destrFuncType) &genericDestr);
      } /* if */
    } /* if */
  } /* removeCheck */



static boolType hasNextCheck (const poll_based_pollType pollData, short eventsToCheck)

  {
    register memSizeType pos;
    boolType hasNext;

  /* hasNextCheck */
    pos = pollData->iterPos;
    while ((pollData->pollFds[pos].events & eventsToCheck) == 0) {
      pos++;
    } /* while */
    hasNext = pos < pollData->size;
    pollData->iterPos = pos;
    return hasNext;
  } /* hasNextCheck */



static boolType hasNextFinding (const poll_based_pollType pollData, short eventsToCheck)

  {
    register memSizeType pos;
    boolType hasNext;

  /* hasNextFinding */
    if (pollData->iterEvents == 0) {
      hasNext = FALSE;
      pollData->iterPos = pollData->size;
    } else {
      pos = pollData->iterPos;
      while ((pollData->pollFds[pos].revents & eventsToCheck) == 0) {
        pos++;
      } /* while */
      hasNext = pos < pollData->size;
      pollData->iterPos = pos;
    } /* if */
    return hasNext;
  } /* hasNextFinding */



static genericType nextCheck (const poll_based_pollType pollData,
    short eventsToCheck, const genericType nullFile)

  {
    register memSizeType pos;
    genericType checkFile;

  /* nextCheck */
    pos = pollData->iterPos;
    while ((pollData->pollFds[pos].events & eventsToCheck) == 0) {
      pos++;
    } /* while */
    if (pos < pollData->size) {
      checkFile = pollData->pollFiles[pos];
      /* printf("nextCheck -> 0x%lx fd[%u]=%d\n",
          (unsigned long) checkFile, pos,
          pollData->pollFds[pos].fd); */
      pos++;
    } else {
      checkFile = nullFile;
    } /* if */
    pollData->iterPos = pos;
    /* printf("end nextCheck -> %d:\n", checkFile);
       dumpPoll(pollData); */
    return checkFile;
  } /* nextCheck */



static genericType nextFinding (const poll_based_pollType pollData,
    short eventsToCheck, const genericType nullFile)

  {
    register memSizeType pos;
    genericType resultFile;

  /* nextFinding */
    /* printf("nextFinding\n");
       dumpPoll(pollData); */
    if (pollData->iterEvents == 0) {
      resultFile = nullFile;
      pollData->iterPos = pollData->size;
    } else {
      pos = pollData->iterPos;
      while ((pollData->pollFds[pos].revents & eventsToCheck) == 0) {
        pos++;
      } /* while */
      if (pos < pollData->size) {
        resultFile = pollData->pollFiles[pos];
        /* printf("nextFinding -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, pos,
            pollData->pollFds[pos].fd); */
        if ((pollData->pollFds[pos].revents & ~eventsToCheck) == 0) {
          pollData->iterEvents--;
        } /* if */
        pos++;
      } else {
        resultFile = nullFile;
      } /* if */
      pollData->iterPos = pos;
    } /* if */
    /* printf("end nextFinding -> %lx %ld\n", resultFile, pos - 1);
       dumpPoll(pollData); */
    return resultFile;
  } /* nextFinding */



/**
 *  Add 'eventsToCheck' for 'aSocket' to 'pollData'.
 *  The parameter 'fileObj' determines, which file is returned,
 *  when the iterator returns files in 'pollData'.
 */
void polAddCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck, const genericType fileObj)

  { /* polAddCheck */
    switch (eventsToCheck) {
      case POLL_IN:
        addCheck(var_conv(pollData), POLLIN, aSocket, fileObj);
        break;
      case POLL_OUT:
        addCheck(var_conv(pollData), POLLOUT, aSocket, fileObj);
        break;
      case POLL_INOUT:
        addCheck(var_conv(pollData), POLLIN | POLLOUT, aSocket, fileObj);
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* polAddCheck */



void polClear (const pollType pollData)

  {
    memSizeType pos;

  /* polClear */
    for (pos = 0; pos < conv(pollData)->size; pos++) {
      fileObjectOps.decrUsageCount(conv(pollData)->pollFiles[pos]);
    } /* for */
    var_conv(pollData)->size = 0;
    var_conv(pollData)->iteratorMode = ITER_EMPTY;
    var_conv(pollData)->iterPos = 0;
    var_conv(pollData)->iterEvents = 0;
    var_conv(pollData)->numOfEvents = 0;
    hshDestr(conv(pollData)->indexHash, (destrFuncType) &genericDestr,
             (destrFuncType) &genericDestr);
    var_conv(pollData)->indexHash = hshEmpty();
  } /* polClear */



void polCpy (const pollType poll_to, const const_pollType pollDataFrom)

  {
    poll_based_pollType pollData;
    rtlHashType newIndexHash;
    struct pollfd *newPollFds;
    memSizeType newPollFilesCapacity;
    genericType *newPollFiles;
    genericType *oldPollFiles;
    memSizeType oldPollFilesSize;
    memSizeType oldPollFilesCapacity;
    memSizeType pos;

  /* polCpy */
    if (poll_to != pollDataFrom) {
      newIndexHash = hshCreate(conv(pollDataFrom)->indexHash,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
      pollData = var_conv(poll_to);
      hshDestr(pollData->indexHash, (destrFuncType) &genericDestr,
               (destrFuncType) &genericDestr);
      oldPollFiles = pollData->pollFiles;
      oldPollFilesSize = pollData->size;
      oldPollFilesCapacity = pollData->capacity;
      if (conv(pollDataFrom)->size + NUM_OF_EXTRA_ELEMS > pollData->capacity) {
        newPollFilesCapacity = conv(pollDataFrom)->capacity;
      } else {
        newPollFilesCapacity = pollData->capacity;
      } /* if */
      if (unlikely(!ALLOC_TABLE(newPollFiles, genericType, newPollFilesCapacity))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        if (conv(pollDataFrom)->size + NUM_OF_EXTRA_ELEMS > pollData->capacity) {
          if (unlikely(!ALLOC_TABLE(newPollFds, struct pollfd, conv(pollDataFrom)->capacity))) {
            FREE_TABLE(newPollFiles, genericType, newPollFilesCapacity);
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->pollFds, struct pollfd, pollData->capacity);
            pollData->capacity = conv(pollDataFrom)->capacity;
            pollData->pollFds = newPollFds;
          } /* if */
        } /* if */
      } /* if */
      pollData->size = conv(pollDataFrom)->size;
      pollData->iteratorMode = conv(pollDataFrom)->iteratorMode;
      pollData->iterPos = conv(pollDataFrom)->iterPos;
      pollData->iterEvents = conv(pollDataFrom)->iterEvents;
      pollData->numOfEvents = conv(pollDataFrom)->numOfEvents;
      pollData->pollFiles = newPollFiles;
      memcpy(pollData->pollFds, conv(pollDataFrom)->pollFds,
             pollData->size * sizeof(struct pollfd));
      memset(&pollData->pollFds[pollData->size], 0, sizeof(struct pollfd));
      pollData->pollFds[pollData->size].revents = TERMINATING_REVENT;
      pollData->indexHash = newIndexHash;
      for (pos = 0; pos < pollData->size; pos++) {
        newPollFiles[pos] = fileObjectOps.incrUsageCount(conv(pollDataFrom)->pollFiles[pos]);
      } /* for */
      for (pos = 0; pos < oldPollFilesSize; pos++) {
        fileObjectOps.decrUsageCount(oldPollFiles[pos]);
      } /* for */
      FREE_TABLE(oldPollFiles, genericType, oldPollFilesCapacity);
    } /* if */
    /* printf("end polCpy:\n");
       dumpPoll(poll_to); */
  } /* polCpy */



pollType polCreate (const const_pollType pollDataFrom)

  {
    memSizeType pos;
    poll_based_pollType result;

  /* polCreate */
    if (unlikely(!ALLOC_RECORD(result, poll_based_pollRecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (unlikely(!ALLOC_TABLE(result->pollFds, struct pollfd, conv(pollDataFrom)->capacity))) {
        FREE_RECORD(result, poll_based_pollRecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_TABLE(result->pollFiles, genericType, conv(pollDataFrom)->capacity))) {
          FREE_TABLE(result->pollFds, struct pollfd, conv(pollDataFrom)->capacity);
          FREE_RECORD(result, poll_based_pollRecord, count.polldata);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result->indexHash = hshCreate(conv(pollDataFrom)->indexHash,
              (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
              (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
          result->size = conv(pollDataFrom)->size;
          result->capacity = conv(pollDataFrom)->capacity;
          result->iteratorMode = conv(pollDataFrom)->iteratorMode;
          result->iterPos = conv(pollDataFrom)->iterPos;
          result->iterEvents = conv(pollDataFrom)->iterEvents;
          result->numOfEvents = conv(pollDataFrom)->numOfEvents;
          memcpy(result->pollFds, conv(pollDataFrom)->pollFds,
                 conv(pollDataFrom)->size * sizeof(struct pollfd));
          for (pos = 0; pos < conv(pollDataFrom)->size; pos++) {
            result->pollFiles[pos] = fileObjectOps.incrUsageCount(conv(pollDataFrom)->pollFiles[pos]);
          } /* for */
          memset(&result->pollFds[result->size], 0, sizeof(struct pollfd));
          result->pollFds[result->size].revents = TERMINATING_REVENT;
        } /* if */
      } /* if */
    } /* if */
    /* printf("end polCreate:\n");
       dumpPoll((pollType) result); */
    return (pollType) result;
  } /* polCreate */



void polDestr (const pollType oldPollData)

  {
    memSizeType pos;

  /* polDestr */
    if (oldPollData != NULL) {
      for (pos = 0; pos < conv(oldPollData)->size; pos++) {
        fileObjectOps.decrUsageCount(conv(oldPollData)->pollFiles[pos]);
      } /* for */
      FREE_TABLE(conv(oldPollData)->pollFds, struct pollfd, conv(oldPollData)->capacity);
      FREE_TABLE(conv(oldPollData)->pollFiles, genericType, conv(oldPollData)->capacity);
      hshDestr(conv(oldPollData)->indexHash, (destrFuncType) &genericDestr,
               (destrFuncType) &genericDestr);
      FREE_RECORD(var_conv(oldPollData), poll_based_pollRecord, count.polldata);
    } /* if */
  } /* polDestr */



pollType polEmpty (void)

  {
    poll_based_pollType result;

  /* polEmpty */
    if (unlikely(!ALLOC_RECORD(result, poll_based_pollRecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (unlikely(!ALLOC_TABLE(result->pollFds, struct pollfd, TABLE_START_SIZE))) {
        FREE_RECORD(result, poll_based_pollRecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_TABLE(result->pollFiles, genericType, TABLE_START_SIZE))) {
          FREE_TABLE(result->pollFds, struct pollfd, TABLE_START_SIZE);
          FREE_RECORD(result, poll_based_pollRecord, count.polldata);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result->indexHash = hshEmpty();
          result->size = 0;
          result->capacity = TABLE_START_SIZE;
          result->iteratorMode = ITER_EMPTY;
          result->iterPos = 0;
          result->iterEvents = 0;
          result->numOfEvents = 0;
          memset(&result->pollFds[0], 0, sizeof(struct pollfd));
          result->pollFds[0].revents = TERMINATING_REVENT;
        } /* if */
      } /* if */
    } /* if */
    /* printf("end polEmpty:\n");
       dumpPoll((pollType) result); */
    return (pollType) result;
  } /* polEmpty */



intType polGetCheck (const const_pollType pollData, const socketType aSocket)

  {
    memSizeType pos;
    short events;
    intType result;

  /* polGetCheck */
    pos = (memSizeType) hshIdxWithDefault(conv(pollData)->indexHash,
        (genericType) (usocketType) aSocket, (genericType) conv(pollData)->size,
        (intType) aSocket, (compareType) &genericCmp);
    if (pos == conv(pollData)->size) {
      result = POLL_NOTHING;
    } else {
      events = conv(pollData)->pollFds[pos].events;
      if ((events & POLLIN) != 0) {
        if ((events & POLLOUT) != 0) {
          result = POLL_INOUT;
        } else {
          result = POLL_IN;
        } /* if */
      } else if ((events & POLLOUT) != 0) {
        result = POLL_OUT;
      } else {
        result = POLL_NOTHING;
      } /* if */
    } /* if */
    return result;
  } /* polGetCheck */



intType polGetFinding (const const_pollType pollData, const socketType aSocket)

  {
    memSizeType pos;
    short revents;
    intType result;

  /* polGetFinding */
    pos = (memSizeType) hshIdxWithDefault(conv(pollData)->indexHash,
        (genericType) (usocketType) aSocket, (genericType) conv(pollData)->size,
        (intType) aSocket, (compareType) &genericCmp);
    if (pos == conv(pollData)->size) {
      result = POLL_NOTHING;
    } else {
      revents = conv(pollData)->pollFds[pos].revents;
      if ((revents & POLLIN) != 0) {
        if ((revents & POLLOUT) != 0) {
          result = POLL_INOUT;
        } else {
          result = POLL_IN;
        } /* if */
      } else if ((revents & POLLOUT) != 0) {
        result = POLL_OUT;
      } else {
        result = POLL_NOTHING;
      } /* if */
    } /* if */
    return result;
  } /* polGetFinding */



boolType polHasNext (const pollType pollData)

  { /* polHasNext */
    switch (conv(pollData)->iteratorMode) {
      case ITER_CHECKS_IN:
        return hasNextCheck(var_conv(pollData), POLLIN);
      case ITER_CHECKS_OUT:
        return hasNextCheck(var_conv(pollData), POLLOUT);
      case ITER_CHECKS_INOUT:
        return hasNextCheck(var_conv(pollData), POLLIN | POLLOUT);
      case ITER_FINDINGS_IN:
        return hasNextFinding(var_conv(pollData), POLLIN);
      case ITER_FINDINGS_OUT:
        return hasNextFinding(var_conv(pollData), POLLOUT);
      case ITER_FINDINGS_INOUT:
        return hasNextFinding(var_conv(pollData), POLLIN | POLLOUT);
      case ITER_EMPTY:
      default:
        return FALSE;
    } /* switch */
  } /* polHasNext */



void polIterChecks (const pollType pollData, intType pollMode)

  { /* polIterChecks */
    switch (pollMode) {
      case POLL_NOTHING:
        var_conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_IN;
        break;
      case POLL_OUT:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_OUT;
        break;
      case POLL_INOUT:
        var_conv(pollData)->iteratorMode = ITER_CHECKS_INOUT;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
    var_conv(pollData)->iterPos = 0;
  } /* polIterChecks */



void polIterFindings (const pollType pollData, intType pollMode)

  { /* polIterFindings */
    switch (pollMode) {
      case POLL_NOTHING:
        var_conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_IN;
        break;
      case POLL_OUT:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_OUT;
        break;
      case POLL_INOUT:
        var_conv(pollData)->iteratorMode = ITER_FINDINGS_INOUT;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
    var_conv(pollData)->iterPos = 0;
    var_conv(pollData)->iterEvents = conv(pollData)->numOfEvents;
  } /* polIterFindings */



genericType polNextFile (const pollType pollData, const genericType nullFile)

  {
    genericType nextFile;

  /* polNextFile */
    /* printf("polNextFile(%d)\n", conv(pollData)->iteratorMode); */
    switch (conv(pollData)->iteratorMode) {
      case ITER_CHECKS_IN:
        nextFile = nextCheck(var_conv(pollData), POLLIN, nullFile);
        break;
      case ITER_CHECKS_OUT:
        nextFile = nextCheck(var_conv(pollData), POLLOUT, nullFile);
        break;
      case ITER_CHECKS_INOUT:
        nextFile = nextCheck(var_conv(pollData), POLLIN | POLLOUT, nullFile);
        break;
      case ITER_FINDINGS_IN:
        nextFile = nextFinding(var_conv(pollData), POLLIN, nullFile);
        break;
      case ITER_FINDINGS_OUT:
        nextFile = nextFinding(var_conv(pollData), POLLOUT, nullFile);
        break;
      case ITER_FINDINGS_INOUT:
        nextFile = nextFinding(var_conv(pollData), POLLIN | POLLOUT, nullFile);
        break;
      case ITER_EMPTY:
      default:
        nextFile = nullFile;
        break;
    } /* switch */
    return nextFile;
  } /* polNextFile */



void polPoll (const pollType pollData)

  {
    int poll_result;

  /* polPoll */
    /* printf("begin polPoll:\n");
       dumpPoll(pollData); */
    do {
      poll_result = os_poll(conv(pollData)->pollFds, conv(pollData)->size, -1); /* &timeout); */
    } while (unlikely(poll_result == -1 && errno == EINTR));
    if (unlikely(poll_result < 0)) {
      /* printf("errno=%d\n", errno);
      printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
          EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
      printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d  EBADF=%d\n",
           EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL, EBADF); */
      raise_error(FILE_ERROR);
    } else {
      var_conv(pollData)->iteratorMode = ITER_EMPTY;
      var_conv(pollData)->iterPos = 0;
      var_conv(pollData)->numOfEvents = (memSizeType) poll_result;
    } /* if */
  } /* polPoll */



/**
 *  Remove 'eventsToCheck' for 'aSocket' from 'pollData'.
 */
void polRemoveCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck)

  { /* polRemoveCheck */
    switch (eventsToCheck) {
      case POLL_IN:
        removeCheck(var_conv(pollData), POLLIN, aSocket);
        break;
      case POLL_OUT:
        removeCheck(var_conv(pollData), POLLOUT, aSocket);
        break;
      case POLL_INOUT:
        removeCheck(var_conv(pollData), POLLIN | POLLOUT, aSocket);
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* polRemoveCheck */
