/********************************************************************/
/*                                                                  */
/*  pol_unx.c     Poll type and function using UNIX capabilities.   */
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
/*  File: seed7/src/pol_unx.c                                       */
/*  Changes: 2011  Thomas Mertes                                    */
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
#include "pol_drv.h"


typedef enum {
    ITER_EMPTY,
    ITER_CHECKS_IN, ITER_CHECKS_OUT, ITER_CHECKS_INOUT,
    ITER_FINDINGS_IN, ITER_FINDINGS_OUT, ITER_FINDINGS_INOUT
  } iteratorType;

typedef struct poll_based_pollstruct {
    memsizetype size;
    memsizetype capacity;
    iteratorType iteratorMode;
    memsizetype iterPos;
    memsizetype iterEvents;
    memsizetype numOfEvents;
    struct pollfd *pollFds;
    rtlGenerictype *pollFiles;
    rtlHashtype indexHash;
  } poll_based_pollrecord, *poll_based_polltype;

typedef const struct poll_based_pollstruct *const_poll_based_polltype;

#define conv(genericPollData) ((poll_based_polltype) genericPollData)

#define TABLE_START_SIZE    256
#define TABLE_INCREMENT    1024
#define NUM_OF_EXTRA_ELEMS    1
#define TERMINATING_REVENT   ~0



#ifdef OUT_OF_ORDER
void dumpPoll (const const_polltype pollData)
    {
      memsizetype pos;

      printf("size=%d\n", conv(pollData)->size);
      printf("capacity=%d\n", conv(pollData)->capacity);
      printf("iteratorMode=%d\n", conv(pollData)->iteratorMode);
      printf("iterPos=%d\n", conv(pollData)->iterPos);
      printf("iterEvents=%d\n", conv(pollData)->iterEvents);
      printf("numOfEvents=%d\n", conv(pollData)->numOfEvents);
      for (pos = 0; pos < conv(pollData)->size; pos++) {
        printf("pollfd[%d]: fd=%d, events=%o, revents=%o\n",
               pos,
               conv(pollData)->pollFds[pos].fd,
               conv(pollData)->pollFds[pos].events,
               conv(pollData)->pollFds[pos].revents);
      }
    }
#endif



#ifdef ANSI_C

static void addCheck (polltype pollData, short eventsToCheck,
    const sockettype aSocket, const rtlGenerictype fileObj)
#else

static void addCheck (pollData, eventsToCheck, aSocket, fileObj)
polltype pollData;
short eventsToCheck;
sockettype aSocket;
rtlGenerictype fileObj;
#endif

  {
    memsizetype pos;
    struct pollfd *aPollFd;

  /* addCheck */
    pos = (memsizetype) hshIdxEnterDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
        (createfunctype) &intCreateGeneric, (createfunctype) &intCreateGeneric);
    if (pos == conv(pollData)->size) {
      if (conv(pollData)->size + NUM_OF_EXTRA_ELEMS >= conv(pollData)->capacity) {
        conv(pollData)->pollFds = REALLOC_TABLE(conv(pollData)->pollFds, struct pollfd,
            conv(pollData)->capacity, conv(pollData)->capacity + TABLE_INCREMENT);
        conv(pollData)->pollFiles = REALLOC_TABLE(conv(pollData)->pollFiles, rtlGenerictype,
            conv(pollData)->capacity, conv(pollData)->capacity + TABLE_INCREMENT);
        if (conv(pollData)->pollFds == NULL || conv(pollData)->pollFiles == NULL) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_TABLE(struct pollfd, conv(pollData)->capacity, conv(pollData)->capacity + TABLE_INCREMENT);
          COUNT3_TABLE(rtlGenerictype, conv(pollData)->capacity, conv(pollData)->capacity + TABLE_INCREMENT);
        } /* if */
        conv(pollData)->capacity += TABLE_INCREMENT;
      } /* if */
      conv(pollData)->size++;
      aPollFd = &conv(pollData)->pollFds[pos];
      memset(aPollFd, 0, sizeof(struct pollfd));
      aPollFd->fd = (int) aSocket;
      aPollFd->events = eventsToCheck;
      memset(&conv(pollData)->pollFds[conv(pollData)->size], 0, sizeof(struct pollfd));
      conv(pollData)->pollFds[conv(pollData)->size].revents = TERMINATING_REVENT;
      conv(pollData)->pollFiles[pos] = fileObj;
    } else {
      conv(pollData)->pollFds[pos].events |= eventsToCheck;
    } /* if */
    /* printf("end addCheck:\n");
       dumpPoll(pollData); */
  } /* addCheck */



#ifdef ANSI_C

static void removeCheck (polltype pollData, short eventsToCheck,
    const sockettype aSocket)
#else

static void removeCheck (pollData, eventsToCheck, aSocket)
polltype pollData;
short eventsToCheck;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    struct pollfd *aPollFd;

  /* removeCheck */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos != conv(pollData)->size) {
      aPollFd = &conv(pollData)->pollFds[pos];
      aPollFd->events &= ~eventsToCheck;
      if (aPollFd->events == 0) {
        if (pos + 1 <= conv(pollData)->iterPos) {
          conv(pollData)->iterPos--;
          if (pos < conv(pollData)->iterPos) {
            memcpy(&conv(pollData)->pollFds[pos],
                   &conv(pollData)->pollFds[conv(pollData)->iterPos], sizeof(struct pollfd));
            conv(pollData)->pollFiles[pos] = conv(pollData)->pollFiles[conv(pollData)->iterPos];
            hshIdxAddr(conv(pollData)->indexHash,
                       (rtlGenerictype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                       (inttype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                       (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
            pos = conv(pollData)->iterPos;
          } /* if */
        } /* if */
        conv(pollData)->size--;
        if (pos < conv(pollData)->size) {
          memcpy(&conv(pollData)->pollFds[pos],
                 &conv(pollData)->pollFds[conv(pollData)->size], sizeof(struct pollfd));
          conv(pollData)->pollFiles[pos] = conv(pollData)->pollFiles[conv(pollData)->size];
          hshIdxAddr(conv(pollData)->indexHash,
                     (rtlGenerictype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                     (inttype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                     (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
        } /* if */
        hshExcl(conv(pollData)->indexHash, (rtlGenerictype) (memsizetype) aSocket,
                (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
                (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
      } /* if */
    } /* if */
  } /* removeCheck */



#ifdef ANSI_C

static booltype hasNextCheck (polltype pollData, short eventsToCheck)
#else

static booltype hasNextCheck (pollData, eventsToCheck)
polltype pollData;
short eventsToCheck;
#endif

  {
    register memsizetype pos;
    booltype hasNext;

  /* hasNextCheck */
    pos = conv(pollData)->iterPos;
    while ((conv(pollData)->pollFds[pos].events & eventsToCheck) == 0) {
      pos++;
    } /* while */
    hasNext = pos < conv(pollData)->size;
    conv(pollData)->iterPos = pos;
    return hasNext;
  } /* hasNextCheck */



#ifdef ANSI_C

static booltype hasNextFinding (polltype pollData, short eventsToCheck)
#else

static booltype hasNextFinding (pollData, eventsToCheck)
polltype pollData;
short eventsToCheck;
#endif

  {
    register memsizetype pos;
    booltype hasNext;

  /* hasNextFinding */
    if (conv(pollData)->iterEvents == 0) {
      hasNext = FALSE;
      conv(pollData)->iterPos = conv(pollData)->size;
    } else {
      pos = conv(pollData)->iterPos;
      while ((conv(pollData)->pollFds[pos].revents & eventsToCheck) == 0) {
        pos++;
      } /* while */
      hasNext = pos < conv(pollData)->size;
      conv(pollData)->iterPos = pos;
    } /* if */
    return hasNext;
  } /* hasNextFinding */



#ifdef ANSI_C

static rtlGenerictype nextCheck (polltype pollData, short eventsToCheck,
    const rtlGenerictype nullFile)
#else

static rtlGenerictype nextCheck (pollData, eventsToCheck, nullFile)
polltype pollData;
short eventsToCheck;
rtlGenerictype nullFile;
#endif

  {
    register memsizetype pos;
    rtlGenerictype checkFile;

  /* nextCheck */
    pos = conv(pollData)->iterPos;
    while ((conv(pollData)->pollFds[pos].events & eventsToCheck) == 0) {
      pos++;
    } /* while */
    if (pos < conv(pollData)->size) {
      checkFile = conv(pollData)->pollFiles[pos];
      /* printf("nextCheck -> 0x%lx fd[%u]=%d\n",
          (unsigned long) checkFile, pos,
          conv(pollData)->pollFds[pos].fd); */
      pos++;
    } else {
      checkFile = nullFile;
    } /* if */
    conv(pollData)->iterPos = pos;
    /* printf("end nextCheck -> %d:\n", checkFile);
       dumpPoll(pollData); */
    return checkFile;
  } /* nextCheck */



#ifdef ANSI_C

static rtlGenerictype nextFinding (polltype pollData, short eventsToCheck,
    const rtlGenerictype nullFile)
#else

static rtlGenerictype nextFinding (pollData, eventsToCheck, nullFile)
polltype pollData;
short eventsToCheck;
rtlGenerictype nullFile;
#endif

  {
    register memsizetype pos;
    rtlGenerictype resultFile;

  /* nextFinding */
    if (conv(pollData)->iterEvents == 0) {
      resultFile = nullFile;
      conv(pollData)->iterPos = conv(pollData)->size;
    } else {
      pos = conv(pollData)->iterPos;
      while ((conv(pollData)->pollFds[pos].revents & eventsToCheck) == 0) {
        pos++;
      } /* while */
      if (pos < conv(pollData)->size) {
        resultFile = conv(pollData)->pollFiles[pos];
        /* printf("nextFinding -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, pos,
            conv(pollData)->pollFds[pos].fd); */
        if ((conv(pollData)->pollFds[pos].revents & ~eventsToCheck) == 0) {
          conv(pollData)->iterEvents--;
        } /* if */
        pos++;
      } else {
        resultFile = nullFile;
      } /* if */
      conv(pollData)->iterPos = pos;
    } /* if */
    /* printf("end nextFinding -> %d:\n", resultFile);
       dumpPoll(pollData); */
    return resultFile;
  } /* nextFinding */



/**
 *  Add ''eventsToCheck'' for ''aSocket'' to ''pollData''.
 *  The parameter ''fileObj'' determines, which file is returned,
 *  when the iterator returns files in ''pollData''.
 */
#ifdef ANSI_C

void polAddCheck (polltype pollData, const sockettype aSocket,
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
        addCheck(pollData, POLLIN, aSocket, fileObj);
        break;
      case POLL_OUT:
        addCheck(pollData, POLLOUT, aSocket, fileObj);
        break;
      case POLL_INOUT:
        addCheck(pollData, POLLIN | POLLOUT, aSocket, fileObj);
        break;
    } /* switch */
  } /* polAddCheck */



#ifdef ANSI_C

void polClear (polltype pollData)
#else

void polClear (pollData)
polltype pollData;
#endif

  { /* polClear */
    conv(pollData)->size = 0;
    conv(pollData)->iteratorMode = ITER_EMPTY;
    conv(pollData)->iterPos = 0;
    conv(pollData)->iterEvents = 0;
    conv(pollData)->numOfEvents = 0;
    hshDestr(conv(pollData)->indexHash, (destrfunctype) &intDestrGeneric,
             (destrfunctype) &intDestrGeneric);
    conv(pollData)->indexHash = hshEmpty();
  } /* polClear */



#ifdef ANSI_C

void polCpy (polltype poll_to, const const_polltype pollDataFrom)
#else

void polCpy (poll_to, pollDataFrom)
polltype poll_to;
polltype pollDataFrom;
#endif

  {
    poll_based_polltype pollData;
    rtlHashtype newIndexHash;
    struct pollfd *newPollFds;
    rtlGenerictype *newPollFiles;

  /* polCpy */
    if (poll_to != pollDataFrom) {
      newIndexHash = hshCreate(conv(pollDataFrom)->indexHash,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
      pollData = conv(poll_to);
      hshDestr(pollData->indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      if (conv(pollDataFrom)->size + NUM_OF_EXTRA_ELEMS > pollData->capacity) {
        if (unlikely(!ALLOC_TABLE(newPollFds, struct pollfd, conv(pollDataFrom)->capacity))) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (unlikely(!ALLOC_TABLE(newPollFiles, rtlGenerictype, conv(pollDataFrom)->capacity))) {
            FREE_TABLE(newPollFds, struct pollfd, conv(pollDataFrom)->capacity);
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->pollFds, struct pollfd, pollData->capacity);
            FREE_TABLE(pollData->pollFiles, rtlGenerictype, pollData->capacity);
            pollData->capacity = conv(pollDataFrom)->capacity;
            pollData->pollFds = newPollFds;
            pollData->pollFiles = newPollFiles;
          } /* if */
        } /* if */
      } /* if */
      pollData->size = conv(pollDataFrom)->size;
      pollData->iteratorMode = conv(pollDataFrom)->iteratorMode;
      pollData->iterPos = conv(pollDataFrom)->iterPos;
      pollData->iterEvents = conv(pollDataFrom)->iterEvents;
      pollData->numOfEvents = conv(pollDataFrom)->numOfEvents;
      memcpy(pollData->pollFds, conv(pollDataFrom)->pollFds,
             conv(pollDataFrom)->size * sizeof(struct pollfd));
      memcpy(pollData->pollFiles, conv(pollDataFrom)->pollFiles,
             conv(pollDataFrom)->size * sizeof(rtlGenerictype));
      memset(&pollData->pollFds[pollData->size], 0, sizeof(struct pollfd));
      pollData->pollFds[pollData->size].revents = TERMINATING_REVENT;
      pollData->indexHash = newIndexHash;
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
    poll_based_polltype result;

  /* polCreate */
    if (unlikely(!ALLOC_RECORD(result, poll_based_pollrecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (unlikely(!ALLOC_TABLE(result->pollFds, struct pollfd, conv(pollDataFrom)->capacity))) {
        FREE_RECORD(result, poll_based_pollrecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_TABLE(result->pollFiles, rtlGenerictype, conv(pollDataFrom)->capacity))) {
          FREE_TABLE(result->pollFds, struct pollfd, conv(pollDataFrom)->capacity);
          FREE_RECORD(result, poll_based_pollrecord, count.polldata);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result->indexHash = hshCreate(conv(pollDataFrom)->indexHash,
              (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
              (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
          result->size = conv(pollDataFrom)->size;
          result->capacity = conv(pollDataFrom)->capacity;
          result->iteratorMode = conv(pollDataFrom)->iteratorMode;
          result->iterPos = conv(pollDataFrom)->iterPos;
          result->iterEvents = conv(pollDataFrom)->iterEvents;
          result->numOfEvents = conv(pollDataFrom)->numOfEvents;
          memcpy(result->pollFds, conv(pollDataFrom)->pollFds,
                 conv(pollDataFrom)->size * sizeof(struct pollfd));
          memcpy(result->pollFiles, conv(pollDataFrom)->pollFiles,
                 conv(pollDataFrom)->size * sizeof(rtlGenerictype));
          memset(&result->pollFds[result->size], 0, sizeof(struct pollfd));
          result->pollFds[result->size].revents = TERMINATING_REVENT;
        } /* if */
      } /* if */
    } /* if */
    /* printf("end polCreate:\n");
       dumpPoll((polltype) result); */
    return (polltype) result;
  } /* polCreate */



#ifdef ANSI_C

void polDestr (const const_polltype oldPollData)
#else

void polDestr (oldPollData)
polltype oldPollData;
#endif

  { /* polDestr */
    if (oldPollData != NULL) {
      FREE_TABLE(conv(oldPollData)->pollFds, struct pollfd, conv(oldPollData)->capacity);
      FREE_TABLE(conv(oldPollData)->pollFiles, rtlGenerictype, conv(oldPollData)->capacity);
      hshDestr(conv(oldPollData)->indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      FREE_RECORD(conv(oldPollData), poll_based_pollrecord, count.pollData);
    } /* if */
  } /* polDestr */



#ifdef ANSI_C

polltype polEmpty (void)
#else

polltype polEmpty ()
#endif

  {
    poll_based_polltype result;

  /* polEmpty */
    if (unlikely(!ALLOC_RECORD(result, poll_based_pollrecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
    } else {
      if (unlikely(!ALLOC_TABLE(result->pollFds, struct pollfd, TABLE_START_SIZE))) {
        FREE_RECORD(result, poll_based_pollrecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_TABLE(result->pollFiles, rtlGenerictype, TABLE_START_SIZE))) {
          FREE_TABLE(result->pollFds, struct pollfd, TABLE_START_SIZE);
          FREE_RECORD(result, poll_based_pollrecord, count.polldata);
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
       dumpPoll((polltype) result); */
    return (polltype) result;
  } /* polEmpty */



#ifdef ANSI_C

inttype polGetCheck (polltype pollData, const sockettype aSocket)
#else

inttype polGetCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    short events;
    inttype result;

  /* polGetCheck */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
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



#ifdef ANSI_C

inttype polGetFinding (polltype pollData, const sockettype aSocket)
#else

inttype polGetFinding (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    short revents;
    inttype result;

  /* polGetFinding */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
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



#ifdef ANSI_C

booltype polHasNext (polltype pollData)
#else

booltype polHasNext (pollData)
polltype pollData;
#endif

  { /* polHasNext */
    switch (conv(pollData)->iteratorMode) {
      case ITER_EMPTY:
        return FALSE;
      case ITER_CHECKS_IN:
        return hasNextCheck(pollData, POLLIN);
      case ITER_CHECKS_OUT:
        return hasNextCheck(pollData, POLLOUT);
      case ITER_CHECKS_INOUT:
        return hasNextCheck(pollData, POLLIN | POLLOUT);
      case ITER_FINDINGS_IN:
        return hasNextFinding(pollData, POLLIN);
      case ITER_FINDINGS_OUT:
        return hasNextFinding(pollData, POLLOUT);
      case ITER_FINDINGS_INOUT:
        return hasNextFinding(pollData, POLLIN | POLLOUT);
      default:
        raise_error(RANGE_ERROR);
        return FALSE;
    } /* switch */
  } /* polHasNext */



#ifdef ANSI_C

void polIterChecks (polltype pollData, inttype pollMode)
#else

void polIterChecks (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterChecks */
    switch (pollMode) {
      case POLL_NOTHING:
        conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        conv(pollData)->iteratorMode = ITER_CHECKS_IN;
        break;
      case POLL_OUT:
        conv(pollData)->iteratorMode = ITER_CHECKS_OUT;
        break;
      case POLL_INOUT:
        conv(pollData)->iteratorMode = ITER_CHECKS_INOUT;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
    conv(pollData)->iterPos = 0;
  } /* polIterChecks */



#ifdef ANSI_C

void polIterFindings (polltype pollData, inttype pollMode)
#else

void polIterFindings (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterFindings */
    switch (pollMode) {
      case POLL_NOTHING:
        conv(pollData)->iteratorMode = ITER_EMPTY;
        break;
      case POLL_IN:
        conv(pollData)->iteratorMode = ITER_FINDINGS_IN;
        break;
      case POLL_OUT:
        conv(pollData)->iteratorMode = ITER_FINDINGS_OUT;
        break;
      case POLL_INOUT:
        conv(pollData)->iteratorMode = ITER_FINDINGS_INOUT;
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
    conv(pollData)->iterPos = 0;
    conv(pollData)->iterEvents = conv(pollData)->numOfEvents;
  } /* polIterFindings */



#ifdef ANSI_C

rtlGenerictype polNextFile (polltype pollData, const rtlGenerictype nullFile)
#else

rtlGenerictype polNextFile (pollData, nullFile)
polltype pollData;
rtlGenerictype nullFile;
#endif

  { /* polNextFile */
    switch (conv(pollData)->iteratorMode) {
      case ITER_EMPTY:
        return nullFile;
      case ITER_CHECKS_IN:
        return nextCheck(pollData, POLLIN, nullFile);
      case ITER_CHECKS_OUT:
        return nextCheck(pollData, POLLOUT, nullFile);
      case ITER_CHECKS_INOUT:
        return nextCheck(pollData, POLLIN | POLLOUT, nullFile);
      case ITER_FINDINGS_IN:
        return nextFinding(pollData, POLLIN, nullFile);
      case ITER_FINDINGS_OUT:
        return nextFinding(pollData, POLLOUT, nullFile);
      case ITER_FINDINGS_INOUT:
        return nextFinding(pollData, POLLIN | POLLOUT, nullFile);
      default:
        raise_error(RANGE_ERROR);
        return nullFile;
    } /* switch */
  } /* polNextFile */



#ifdef ANSI_C

void polPoll (polltype pollData)
#else

void polPoll (pollData)
polltype pollData;
#endif

  {
    int poll_result;

  /* polPoll */
    /* printf("begin polPoll:\n");
       dumpPoll(pollData); */
    poll_result = os_poll(conv(pollData)->pollFds, conv(pollData)->size, -1); /* &timeout); */
    if (unlikely(poll_result < 0)) {
      /* printf("errno=%d\n", errno);
      printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
          EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
      printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d  EBADF=%d\n",
           EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL, EBADF); */
      raise_error(FILE_ERROR);
    } else {
      conv(pollData)->iteratorMode = ITER_EMPTY;
      conv(pollData)->iterPos = 0;
      conv(pollData)->numOfEvents = (memsizetype) poll_result;
    } /* if */
  } /* polPoll */



/**
 *  Remove ''eventsToCheck'' for ''aSocket'' from ''pollData''.
 */
#ifdef ANSI_C

void polRemoveCheck (polltype pollData, const sockettype aSocket,
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
        removeCheck(pollData, POLLIN, aSocket);
        break;
      case POLL_OUT:
        removeCheck(pollData, POLLOUT, aSocket);
        break;
      case POLL_INOUT:
        removeCheck(pollData, POLLIN | POLLOUT, aSocket);
        break;
    } /* switch */
  } /* polRemoveCheck */
