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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#if SOCKET_LIB == WINSOCK_SOCKETS
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
#include "soc_rtl.h"
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

#if DO_HEAP_STATISTIC
size_t sizeof_pollRecord = sizeof(poll_based_pollRecord);
#endif

#define conv(genericPollData) ((const_poll_based_pollType) (genericPollData))
#define var_conv(genericPollData) ((poll_based_pollType) (genericPollData))


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
    struct pollfd *resizedPollFds;
    genericType *resizedPollFiles;
    struct pollfd *aPollFd;

  /* addCheck */
    pos = (memSizeType) hshIdxEnterDefault(pollData->indexHash,
        (genericType) (usocketType) aSocket, (genericType) pollData->size,
        (intType) aSocket);
    if (pos == pollData->size) {
      if (pollData->size + NUM_OF_EXTRA_ELEMS >= pollData->capacity) {
        resizedPollFds = REALLOC_TABLE(pollData->pollFds, struct pollfd,
            pollData->capacity, pollData->capacity + TABLE_INCREMENT);
        if (resizedPollFds == NULL) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          pollData->pollFds = resizedPollFds;
          resizedPollFiles = REALLOC_TABLE(pollData->pollFiles, genericType,
              pollData->capacity, pollData->capacity + TABLE_INCREMENT);
          if (resizedPollFiles == NULL) {
            raise_error(MEMORY_ERROR);
            return;
          } else {
            pollData->pollFiles = resizedPollFiles;
            COUNT3_TABLE(struct pollfd, pollData->capacity, pollData->capacity + TABLE_INCREMENT);
            COUNT3_TABLE(genericType, pollData->capacity, pollData->capacity + TABLE_INCREMENT);
            pollData->capacity += TABLE_INCREMENT;
          } /* if */
        } /* if */
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
 *  'EventsToCheck' can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData Poll data to which the event checks are added.
 *  @param aSocket Socket for which the events should be checked.
 *  @param eventsToCheck Events to be added to the checkedEvents
 *         field of 'pollData'.
 *  @param fileObj File to be returned, if the iterator returns
 *         files in 'pollData'.
 *  @exception RANGE_ERROR Illegal value for 'eventsToCheck'.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
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



/**
 *  Clears 'pollData'.
 *  All sockets and all events are removed from 'pollData' and
 *  the iterator is reset, such that polHasNext() returns FALSE.
 */
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



/**
 *  Assign source to dest.
 *  A copy function assumes that dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void polCpy (const pollType dest, const const_pollType source)

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
    logFunction(printf("polCpy(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) dest, (memSizeType) source););
    if (dest != source) {
      newIndexHash = hshCreate(conv(source)->indexHash,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
          (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
      pollData = var_conv(dest);
      hshDestr(pollData->indexHash, (destrFuncType) &genericDestr,
               (destrFuncType) &genericDestr);
      oldPollFiles = pollData->pollFiles;
      oldPollFilesSize = pollData->size;
      oldPollFilesCapacity = pollData->capacity;
      if (conv(source)->size + NUM_OF_EXTRA_ELEMS > pollData->capacity) {
        newPollFilesCapacity = conv(source)->capacity;
      } else {
        newPollFilesCapacity = pollData->capacity;
      } /* if */
      if (unlikely(!ALLOC_TABLE(newPollFiles, genericType, newPollFilesCapacity))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        if (conv(source)->size + NUM_OF_EXTRA_ELEMS > pollData->capacity) {
          if (unlikely(!ALLOC_TABLE(newPollFds, struct pollfd, conv(source)->capacity))) {
            FREE_TABLE(newPollFiles, genericType, newPollFilesCapacity);
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->pollFds, struct pollfd, pollData->capacity);
            pollData->capacity = conv(source)->capacity;
            pollData->pollFds = newPollFds;
          } /* if */
        } /* if */
      } /* if */
      pollData->size = conv(source)->size;
      pollData->iteratorMode = conv(source)->iteratorMode;
      pollData->iterPos = conv(source)->iterPos;
      pollData->iterEvents = conv(source)->iterEvents;
      pollData->numOfEvents = conv(source)->numOfEvents;
      pollData->pollFiles = newPollFiles;
      memcpy(pollData->pollFds, conv(source)->pollFds,
             pollData->size * sizeof(struct pollfd));
      memset(&pollData->pollFds[pollData->size], 0, sizeof(struct pollfd));
      pollData->pollFds[pollData->size].revents = TERMINATING_REVENT;
      pollData->indexHash = newIndexHash;
      for (pos = 0; pos < pollData->size; pos++) {
        newPollFiles[pos] = fileObjectOps.incrUsageCount(conv(source)->pollFiles[pos]);
      } /* for */
      for (pos = 0; pos < oldPollFilesSize; pos++) {
        fileObjectOps.decrUsageCount(oldPollFiles[pos]);
      } /* for */
      FREE_TABLE(oldPollFiles, genericType, oldPollFilesCapacity);
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
    memSizeType pos;
    poll_based_pollType result;

  /* polCreate */
    logFunction(printf("polCreate(" FMT_U_MEM ")\n",
                       (memSizeType) source););
    if (unlikely(!ALLOC_RECORD(result, poll_based_pollRecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (unlikely(!ALLOC_TABLE(result->pollFds, struct pollfd, conv(source)->capacity))) {
        FREE_RECORD(result, poll_based_pollRecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_TABLE(result->pollFiles, genericType, conv(source)->capacity))) {
          FREE_TABLE(result->pollFds, struct pollfd, conv(source)->capacity);
          FREE_RECORD(result, poll_based_pollRecord, count.polldata);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result->indexHash = hshCreate(conv(source)->indexHash,
              (createFuncType) &genericCreate, (destrFuncType) &genericDestr,
              (createFuncType) &genericCreate, (destrFuncType) &genericDestr);
          result->size = conv(source)->size;
          result->capacity = conv(source)->capacity;
          result->iteratorMode = conv(source)->iteratorMode;
          result->iterPos = conv(source)->iterPos;
          result->iterEvents = conv(source)->iterEvents;
          result->numOfEvents = conv(source)->numOfEvents;
          memcpy(result->pollFds, conv(source)->pollFds,
                 conv(source)->size * sizeof(struct pollfd));
          for (pos = 0; pos < conv(source)->size; pos++) {
            result->pollFiles[pos] = fileObjectOps.incrUsageCount(conv(source)->pollFiles[pos]);
          } /* for */
          memset(&result->pollFds[result->size], 0, sizeof(struct pollfd));
          result->pollFds[result->size].revents = TERMINATING_REVENT;
        } /* if */
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



/**
 *  Create an empty poll data value.
 *  @return an empty poll data value.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
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
 *  @param aSocket Socket for which the checkedEvents are retrieved.
 *  @return POLLNOTHING, POLLIN, POLLOUT or POLLINOUT, depending on
 *          the events added and removed for 'aSocket' with
 *          'addCheck' and 'removeCheck'.
 */
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



/**
 *  Determine if the 'pollData' iterator can deliver another file.
 *  @return TRUE if 'nextFile' would return another file from the
 *          'pollData' iterator, FALSE otherwise.
 */
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



/**
 *  Get the next file from the 'pollData' iterator.
 *  Successive calls of 'nextFile' return all files from the 'pollData'
 *  iterator. The file returned by 'nextFile' is determined with the
 *  function 'addCheck'. The files covered by the 'pollData' iterator
 *  are determined with 'iterChecks' or 'iterFindings'.
 *  @return the next file from the 'pollData' iterator, or
 *          STD_NULL if no file from the 'pollData' iterator is available.
 */
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



/**
 *  Waits for one or more of the checkedEvents from 'pollData'.
 *  polPoll waits until one of the checkedEvents for a
 *  corresponding socket occurs. If a checked event occurs
 *  the eventFindings field is assigned a value. The following
 *  eventFindings values are assigned:
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @exception FILE_ERROR The system function returns an error.
 */
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
      logError(printf("polPoll: poll(*, " FMT_U_MEM ", -1) failed:\n"
                      "%s=%d\nerror: %s\n",
                      conv(pollData)->size, ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } else {
      var_conv(pollData)->iteratorMode = ITER_EMPTY;
      var_conv(pollData)->iterPos = 0;
      var_conv(pollData)->numOfEvents = (memSizeType) poll_result;
    } /* if */
  } /* polPoll */



/**
 *  Remove 'eventsToCheck' for 'aSocket' from 'pollData'.
 *  'EventsToCheck' can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData Poll data from which the event checks are removed.
 *  @param aSocket Socket for which the events should not be checked.
 *  @param eventsToCheck Events to be removed from the checkedEvents
 *         field of 'pollData'.
 *  @exception RANGE_ERROR Illegal value for 'eventsToCheck'.
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
