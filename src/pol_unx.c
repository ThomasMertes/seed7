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


typedef struct poll_based_pollstruct {
    memsizetype size;
    memsizetype capacity;
    memsizetype readPos;
    memsizetype writePos;
    memsizetype numOfEvents;
    struct pollfd *pollFds;
    rtlGenerictype *pollFiles;
    rtlHashtype indexHash;
  } poll_based_pollrecord, *poll_based_polltype;

typedef const struct poll_based_pollstruct *const_poll_based_polltype;

#define conv(genericPollData) ((poll_based_polltype) genericPollData)

#define TABLE_START_SIZE  256
#define TABLE_INCREMENT  1024

void dumpPoll (const const_polltype pollData)
    {
      memsizetype pos;

      printf("size=%d\n", conv(pollData)->size);
      printf("capacity=%d\n", conv(pollData)->capacity);
      printf("readPos=%d\n", conv(pollData)->readPos);
      printf("writePos=%d\n", conv(pollData)->writePos);
      printf("numOfEvents=%d\n", conv(pollData)->numOfEvents);
      for (pos = 0; pos < conv(pollData)->size; pos++) {
        printf("pollfd[%d]: fd=%d, events=%o, revents=%o\n",
               pos,
               conv(pollData)->pollFds[pos].fd,
               conv(pollData)->pollFds[pos].events,
               conv(pollData)->pollFds[pos].revents);
      }
    }



#ifdef ANSI_C

static void polAddCheck (polltype pollData, const sockettype aSocket,
    const rtlGenerictype fileObj, short eventsToCheck)
#else

static void polAddCheck (pollData, aSocket, fileObj, eventsToCheck)
polltype pollData;
sockettype aSocket;
rtlGenerictype fileObj;
short eventsToCheck;
#endif

  {
    memsizetype pos;
    struct pollfd *aPollFd;

  /* polAddCheck */
    pos = (memsizetype) hshIdxEnterDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
        (createfunctype) &intCreateGeneric, (createfunctype) &intCreateGeneric);
    if (pos == conv(pollData)->size) {
      if (conv(pollData)->size >= conv(pollData)->capacity) {
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
      conv(pollData)->pollFiles[pos] = fileObj;
    } else {
      conv(pollData)->pollFds[pos].events |= eventsToCheck;
    } /* if */
    /* printf("end polAddCheck:\n");
       dumpPoll(pollData); */
  } /* polAddCheck */



#ifdef ANSI_C

static void polRemoveCheck (polltype pollData, const sockettype aSocket,
    short eventsToCheck)
#else

static void polRemoveCheck (pollData, aSocket, eventsToCheck)
polltype pollData;
sockettype aSocket;
short eventsToCheck;
#endif

  {
    memsizetype pos;
    struct pollfd *aPollFd;

  /* polRemoveCheck */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos != conv(pollData)->size) {
      aPollFd = &conv(pollData)->pollFds[pos];
      aPollFd->events &= !eventsToCheck;
      if (aPollFd->events == 0 &&
          conv(pollData)->readPos <= pos + 1 &&
          conv(pollData)->writePos <= pos + 1) {
        conv(pollData)->size--;
        if (pos != conv(pollData)->size) {
          conv(pollData)->pollFds[pos] = conv(pollData)->pollFds[conv(pollData)->size];
          conv(pollData)->pollFiles[pos] = conv(pollData)->pollFiles[conv(pollData)->size];
          hshIdxAddr(conv(pollData)->indexHash,
                     (rtlGenerictype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                     (inttype) (memsizetype) conv(pollData)->pollFds[pos].fd,
                     (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
        } /* if */
        hshExcl(conv(pollData)->indexHash, (rtlGenerictype) (memsizetype) aSocket,
                (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
                (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
        if (conv(pollData)->readPos == pos + 1) {
          conv(pollData)->readPos--;
        } /* if */
        if (conv(pollData)->writePos == pos + 1) {
          conv(pollData)->writePos--;
        } /* if */
      } /* if */
    } /* if */
  } /* polRemoveCheck */



#ifdef ANSI_C

static booltype polReady (polltype pollData, const sockettype aSocket, short eventsToCheck)
#else

static booltype polReady (pollData, aSocket, eventsToCheck)
polltype pollData;
sockettype aSocket;
short eventsToCheck;
#endif

  {
    memsizetype pos;
    struct pollfd *aPollFd;
    booltype result;

  /* polReady */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos == conv(pollData)->size) {
      result = FALSE;
    } else {
      aPollFd = &conv(pollData)->pollFds[pos];
      result = (eventsToCheck & aPollFd->revents) != 0;
      /* printf("polReady: sock=%d, pos=%d, fd=%d, revents=%o\n",
         aSocket, pos, aPollFd->fd, aPollFd->revents); */
    } /* if */
    return result;
  } /* polReady */



#ifdef ANSI_C

void polAddReadCheck (polltype pollData, const sockettype aSocket,
    const rtlGenerictype fileObj)
#else

void polAddReadCheck (pollData, aSocket, fileObj)
polltype pollData;
sockettype aSocket;
rtlGenerictype fileObj;
#endif

  { /* polAddReadCheck */
    polAddCheck(pollData, aSocket, fileObj, POLLIN);
  } /* polAddReadCheck */



#ifdef ANSI_C

void polAddWriteCheck (polltype pollData, const sockettype aSocket,
    const rtlGenerictype fileObj)
#else

void polAddWriteCheck (pollData, aSocket, fileObj)
polltype pollData;
sockettype aSocket;
rtlGenerictype fileObj;
#endif

  { /* polAddWriteCheck */
    polAddCheck(pollData, aSocket, fileObj, POLLOUT);
  } /* polAddWriteCheck */



#ifdef ANSI_C

void polClear (polltype pollData)
#else

void polClear (pollData)
polltype pollData;
#endif

  { /* polClear */
    conv(pollData)->size = 0;
    conv(pollData)->readPos = 0;
    conv(pollData)->writePos = 0;
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
      if (pollData->capacity < conv(pollDataFrom)->size) {
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
      pollData->readPos = conv(pollDataFrom)->readPos;
      pollData->writePos = conv(pollDataFrom)->writePos;
      pollData->numOfEvents = conv(pollDataFrom)->numOfEvents;
      memcpy(pollData->pollFds, conv(pollDataFrom)->pollFds, conv(pollDataFrom)->size * sizeof(struct pollfd));
      memcpy(pollData->pollFiles, conv(pollDataFrom)->pollFiles, conv(pollDataFrom)->size * sizeof(rtlGenerictype));
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
          result->readPos = conv(pollDataFrom)->readPos;
          result->writePos = conv(pollDataFrom)->writePos;
          result->numOfEvents = conv(pollDataFrom)->numOfEvents;
          memcpy(result->pollFds, conv(pollDataFrom)->pollFds, conv(pollDataFrom)->size * sizeof(struct pollfd));
          memcpy(result->pollFiles, conv(pollDataFrom)->pollFiles, conv(pollDataFrom)->size * sizeof(rtlGenerictype));
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
          result->readPos = 0;
          result->writePos = 0;
          result->numOfEvents = 0;
          memset(result->pollFds, 0, TABLE_START_SIZE * sizeof(struct pollfd));
          memset(result->pollFiles, 0, TABLE_START_SIZE * sizeof(rtlGenerictype));
        } /* if */
      } /* if */
    } /* if */
    /* printf("end polEmpty:\n");
       dumpPoll((polltype) result); */
    return (polltype) result;
  } /* polEmpty */



#ifdef ANSI_C

rtlArraytype polFiles (const const_polltype pollData)
#else

rtlArraytype polFiles (pollData)
polltype pollData;
#endif

  {
    rtlArraytype fileArray;

  /* polFiles */
    if (!ALLOC_RTL_ARRAY(fileArray, conv(pollData)->size)) {
      raise_error(MEMORY_ERROR);
    } else {
      fileArray->min_position = 1;
      fileArray->max_position = (inttype) conv(pollData)->size;
      memcpy(fileArray->arr, conv(pollData)->pollFiles,
             conv(pollData)->size * sizeof(rtlGenerictype));
    } /* if */
    return fileArray;
  } /* polFiles */



#ifdef ANSI_C

booltype polHasNextReadFile (polltype pollData)
#else

booltype polHasNextReadFile (pollData)
polltype pollData;
#endif

  {
    register memsizetype readPos;
    booltype hasNext;

  /* polHasNextReadFile */
    if (conv(pollData)->numOfEvents == 0) {
      hasNext = FALSE;
      conv(pollData)->readPos = conv(pollData)->size;
    } else {
      readPos = conv(pollData)->readPos;
      while (readPos < conv(pollData)->size &&
             !(conv(pollData)->pollFds[readPos].revents & POLLIN)) {
        readPos++;
      } /* while */
      hasNext = readPos < conv(pollData)->size;
      conv(pollData)->readPos = readPos;
    } /* if */
    return hasNext;
  } /* polHasNextReadFile */



#ifdef ANSI_C

booltype polHasNextWriteFile (polltype pollData)
#else

booltype polHasNextWriteFile (pollData)
polltype pollData;
#endif

  {
    register memsizetype writePos;
    booltype hasNext;

  /* polHasNextWriteFile */
    if (conv(pollData)->numOfEvents == 0) {
      hasNext = FALSE;
      conv(pollData)->writePos = conv(pollData)->size;
    } else {
      writePos = conv(pollData)->writePos;
      while (writePos < conv(pollData)->size &&
             !(conv(pollData)->pollFds[writePos].revents & POLLIN)) {
        writePos++;
      } /* while */
      hasNext = writePos < conv(pollData)->size;
      conv(pollData)->writePos = writePos;
    } /* if */
    return hasNext;
  } /* polHasNextWriteFile */



#ifdef ANSI_C

rtlGenerictype polNextReadFile (polltype pollData, const rtlGenerictype nullFile)
#else

rtlGenerictype polNextReadFile (pollData, nullFile)
polltype pollData;
rtlGenerictype nullFile;
#endif

  {
    register memsizetype readPos;
    rtlGenerictype resultFile;

  /* polNextReadFile */
    if (conv(pollData)->numOfEvents == 0) {
      resultFile = nullFile;
      conv(pollData)->readPos = conv(pollData)->size;
    } else {
      readPos = conv(pollData)->readPos;
      while (readPos < conv(pollData)->size &&
             (conv(pollData)->pollFds[readPos].revents & POLLIN) == 0) {
        readPos++;
      } /* while */
      if (readPos < conv(pollData)->size) {
        resultFile = conv(pollData)->pollFiles[readPos];
        /* printf("polNextReadFile -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, readPos,
            conv(pollData)->pollFds[readPos].fd); */
        if ((conv(pollData)->pollFds[readPos].revents & !POLLIN) == 0) {
          conv(pollData)->numOfEvents--;
        } /* if */
        readPos++;
      } else {
        resultFile = nullFile;
      } /* if */
      conv(pollData)->readPos = readPos;
    } /* if */
    return resultFile;
  } /* polNextReadFile */



#ifdef ANSI_C

rtlGenerictype polNextWriteFile (polltype pollData, const rtlGenerictype nullFile)
#else

rtlGenerictype polNextWriteFile (pollData, nullFile)
polltype pollData;
rtlGenerictype nullFile;
#endif

  {
    register memsizetype writePos;
    rtlGenerictype resultFile;

  /* polNextWriteFile */
    if (conv(pollData)->numOfEvents == 0) {
      resultFile = nullFile;
      conv(pollData)->writePos = conv(pollData)->size;
    } else {
      writePos = conv(pollData)->writePos;
      while (writePos < conv(pollData)->size &&
             (conv(pollData)->pollFds[writePos].revents & POLLOUT) == 0) {
        writePos++;
      } /* while */
      if (writePos < conv(pollData)->size) {
        resultFile = conv(pollData)->pollFiles[writePos];
        /* printf("polNextWriteFile -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, writePos,
            conv(pollData)->pollFds[writePos].fd); */
        if ((conv(pollData)->pollFds[writePos].revents & !POLLOUT) == 0) {
          conv(pollData)->numOfEvents--;
        } /* if */
        conv(pollData)->numOfEvents--;
        writePos++;
      } else {
        resultFile = nullFile;
      } /* if */
      conv(pollData)->writePos = writePos;
    } /* if */
    /* printf("end polNextWriteFile -> %d:\n", resultFile);
       dumpPoll(pollData); */
    return resultFile;
  } /* polNextWriteFile */



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
      raise_error(FILE_ERROR);
    } else {
      conv(pollData)->readPos = 0;
      conv(pollData)->writePos = 0;
      conv(pollData)->numOfEvents = (memsizetype) poll_result;
    } /* if */
  } /* polPoll */



#ifdef ANSI_C

booltype polReadyForRead (polltype pollData, const sockettype aSocket)
#else

booltype polReadyForRead (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polReadyForRead */
    return polReady(pollData, aSocket, POLLIN);
  } /* polReadyForRead */



#ifdef ANSI_C

booltype polReadyForWrite (polltype pollData, const sockettype aSocket)
#else

booltype polReadyForWrite (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polReadyForWrite */
    return polReady(pollData, aSocket, POLLOUT);
  } /* polReadyForWrite */



#ifdef ANSI_C

void polRemoveReadCheck (polltype pollData, const sockettype aSocket)
#else

void polRemoveReadCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polRemoveReadCheck */
    polRemoveCheck(pollData, aSocket, POLLIN);
  } /* polRemoveReadCheck */



#ifdef ANSI_C

void polRemoveWriteCheck (polltype pollData, const sockettype aSocket)
#else

void polRemoveWriteCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polRemoveWriteCheck */
    polRemoveCheck(pollData, aSocket, POLLOUT);
  } /* polRemoveWriteCheck */
