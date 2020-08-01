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
#define CHECK_FD_SETSIZE
#define SIZEOF_FD_SET(size) \
    (sizeof(fd_set) - FD_SETSIZE * sizeof(SOCKET) + (size) * sizeof(SOCKET))
#define USED_FD_SET_SIZE(fdset) SIZEOF_FD_SET((fdset)->fd_count)
#else
#define SELECT_WITH_NFDS
#define CHECK_MAXIMUM_FD_NUMBER
#define SIZEOF_FD_SET(size) sizeof(fd_set)
#define USED_FD_SET_SIZE(fdset) sizeof(fd_set)
#endif

#define USE_PREPARED_FD_SET
#define MEMCPY_FD_SET


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
    memsizetype pos;
    fdAndFileType *files;
    rtlHashtype indexHash;
  } checkType;

typedef struct select_based_pollstruct {
    checkType readCheck;
    checkType writeCheck;
    memsizetype numOfEvents;
  } select_based_pollrecord, *select_based_polltype;

#define conv(data)               ((select_based_polltype) data)


#ifdef DYNAMIC_FD_SET

#define to_read_inFdset(data)    (((select_based_polltype) data)->readCheck.inFdset)
#define to_write_inFdset(data)   (((select_based_polltype) data)->writeCheck.inFdset)
#define to_read_outFdset(data)   (((select_based_polltype) data)->readCheck.outFdset)
#define to_write_outFdset(data)  (((select_based_polltype) data)->writeCheck.outFdset)

#define ALLOC_FDSET(var,nr)      (ALLOC_HEAP(var,fd_set *,SIZEOF_FD_SET(nr))?CNT1_BYT(SIZEOF_FD_SET(nr)),TRUE:FALSE)
#define FREE_FDSET(var,nr)       (CNT2_BYT(SIZEOF_FD_SET(nr)) FREE_HEAP(var, SIZEOF_FD_SET(nr)))
#define REALLOC_FDSET(var,nr)    REALLOC_HEAP(var, fd_set *, SIZEOF_FD_SET(nr))
#define COUNT3_FDSET(nr1,nr2)    CNT3(CNT2_BYT(SIZEOF_FD_SET(nr1)) CNT1_BYT(SIZEOF_FD_SET(nr2)))

#else

#define to_read_inFdset(data)    (&((select_based_polltype) data)->readCheck.inFdset)
#define to_write_inFdset(data)   (&((select_based_polltype) data)->writeCheck.inFdset)
#define to_read_outFdset(data)   (&((select_based_polltype) data)->readCheck.outFdset)
#define to_write_outFdset(data)  (&((select_based_polltype) data)->writeCheck.outFdset)

#endif


#define TABLE_START_SIZE  256
#define TABLE_INCREMENT  1024


static void dumpPoll (const const_polltype pollData)
    {
      memsizetype pos;
/*
      printf("readSize=%d\n", conv(pollData)->readCheck.size);
      printf("readCapacity=%d\n", conv(pollData)->readCheck.capacity);
      printf("readPos=%d\n", conv(pollData)->readCheck.pos);
      printf("writeSize=%d\n", conv(pollData)->writeCheck.size);
      printf("writeCapacity=%d\n", conv(pollData)->writeCheck.capacity);
      printf("writePos=%d\n", conv(pollData)->writeCheck.pos);
      printf("numOfEvents=%d\n", conv(pollData)->numOfEvents);
      */
#ifdef USE_PREPARED_FD_SET
      //if (conv(pollData)->readCheck.size <= 5) {
      printf("readFds:");
      for (pos = 0; pos < conv(pollData)->readCheck.size; pos++) {
        printf(" %d", conv(pollData)->readCheck.files[pos].fd);
        if (!FD_ISSET(conv(pollData)->readCheck.files[pos].fd, to_read_inFdset(pollData))) {
          printf("*");
        }
      }
      printf("\n");
      //}
      /*
      printf("writeFds:");
      for (pos = 0; pos < conv(pollData)->writeCheck.size; pos++) {
        printf(" %d", conv(pollData)->writeCheck.files[pos].fd);
        if (!FD_ISSET(conv(pollData)->writeCheck.files[pos].fd, to_write_inFdset(pollData))) {
          printf("*");
        }
      }
      */
#endif
    }



#ifdef MEMCPY_FD_SET
#define copyFdSet(dest, source, unused) memcpy(dest, source, USED_FD_SET_SIZE(source))
#else

#ifdef ANSI_C

static void copyFdSet (fd_set *dest, fd_set *source, checkType *check)
#else

static void copyFdSet (dest, source, check)
fd_set *dest;
fd_set *source;
checkType *check;
#endif

  {
    memsizetype pos;

  /* copyFdSet */
    FD_ZERO(dest);
    for (pos = 0; pos < check->size; pos++) {
      if (FD_ISSET(check->files[pos].fd, source)) {
        FD_SET(check->files[pos].fd, dest);
      } /* if */
    } /* for */
  } /* copyFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define allocFdSet(check, capacity) TRUE
#else

#ifdef ANSI_C

static booltype allocFdSet (checkType *check, memsizetype capacity)
#else

static booltype allocFdSet (check, capacity)
checkType *check;
memsizetype capacity;
#endif

  {
    booltype result = TRUE;

  /* allocFdSet */
#ifdef USE_PREPARED_FD_SET
    if (unlikely(!ALLOC_FDSET(check->inFdset, capacity))) {
      result = FALSE;
      check->outFdset = NULL;
    } else if (unlikely(!ALLOC_FDSET(check->outFdset, capacity))) {
      result = FALSE;
      FREE_FDSET(check->inFdset, capacity);
      check->inFdset = NULL;
    } /* if */
#else
    if (unlikely(!ALLOC_FDSET(check->outFdset, capacity))) {
      result = FALSE;
    } /* if */
#endif
    return result;
  } /* allocFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define freeFdSet(check, capacity) 0
#else

#ifdef ANSI_C

static void freeFdSet (checkType *check, memsizetype capacity)
#else

static void freeFdSet (check, capacity)
checkType *check;
memsizetype capacity;
#endif

  { /* freeFdSet */
    if (check->outFdset != NULL) {
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(check->inFdset, capacity);
#endif
      FREE_FDSET(check->outFdset, capacity);
    } /* if */
  } /* freeFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define reallocFdSet(pollData, increment) TRUE
#else

#ifdef ANSI_C

static booltype reallocFdSet (checkType *check, memsizetype increment)
#else

static booltype reallocFdSet (check, increment)
checkType *check;
memsizetype increment;
#endif

  {
    fd_set *newFdset;
    booltype result = TRUE;

  /* reallocFdSet */
#ifdef USE_PREPARED_FD_SET
    if (unlikely((newFdset = REALLOC_FDSET(check->inFdset,
        check->capacity + increment)) == NULL)) {
      result = FALSE;
    } else {
      COUNT3_FDSET(check->capacity,
                   check->capacity + increment);
      check->inFdset = newFdset;
    } /* if */
#endif
    if (unlikely((newFdset = REALLOC_FDSET(check->outFdset,
        check->capacity + increment)) == NULL)) {
      result = FALSE;
    } else {
      COUNT3_FDSET(check->capacity,
                   check->capacity + increment);
      check->outFdset = newFdset;
    } /* if */
    return result;
  } /* reallocFdSet */
#endif



#ifndef DYNAMIC_FD_SET
#define replaceFdSet(check, capacity) TRUE
#else

#ifdef ANSI_C

static booltype replaceFdSet (checkType *check, memsizetype capacity)
#else

static booltype replaceFdSet (check, capacity)
checkType *check;
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
      FREE_FDSET(check->inFdset, capacity);
      check->inFdset = newInFdset;
#endif
      FREE_FDSET(check->outFdset, capacity);
      check->outFdset = newOutFdset;
    } /* if */
    return result;
  } /* replaceFdSet */
#endif



#ifdef ANSI_C

void polAddReadCheck (polltype pollData, const sockettype aSocket,
    const rtlGenerictype fileObj)
#else

void polAddReadCheck (pollData, aSocket, fileObj)
polltype pollData;
sockettype aSocket;
rtlGenerictype fileObj;
#endif

  {
    memsizetype pos;

  /* polAddReadCheck */
    /* printf("polAddReadCheck(..., %u, 0x%lx)\n", aSocket, (unsigned long) fileObj); */
#ifdef CHECK_MAXIMUM_FD_NUMBER
    if (aSocket < 0 || aSocket >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
#ifdef CHECK_FD_SETSIZE
    if (conv(pollData)->readCheck.size >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    pos = (memsizetype) hshIdxEnterDefault(conv(pollData)->readCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->readCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
        (createfunctype) &intCreateGeneric, (createfunctype) &intCreateGeneric);
    if (pos == conv(pollData)->readCheck.size) {
      if (conv(pollData)->readCheck.size >= conv(pollData)->readCheck.capacity) {
        conv(pollData)->readCheck.files = REALLOC_TABLE(conv(pollData)->readCheck.files,
            fdAndFileType, conv(pollData)->readCheck.capacity,
            conv(pollData)->readCheck.capacity + TABLE_INCREMENT);
        if (conv(pollData)->readCheck.files == NULL ||
            !reallocFdSet(&conv(pollData)->readCheck, TABLE_INCREMENT)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_TABLE(fdAndFileType, conv(pollData)->readCheck.capacity,
                       conv(pollData)->readCheck.capacity + TABLE_INCREMENT);
          conv(pollData)->readCheck.capacity += TABLE_INCREMENT;
          /* printf("increment table to %lu\n", conv(pollData)->readCheck.capacity); */
        } /* if */
      } /* if */
      conv(pollData)->readCheck.size++;
      conv(pollData)->readCheck.files[pos].fd = aSocket;
      conv(pollData)->readCheck.files[pos].file = fileObj;
#ifdef USE_PREPARED_FD_SET
      FD_SET(aSocket, to_read_inFdset(pollData));
#ifdef SELECT_WITH_NFDS
      if (aSocket >= conv(pollData)->readCheck.preparedNfds) {
        conv(pollData)->readCheck.preparedNfds = (int) aSocket + 1;
      } /* if */
#endif
#endif
    } /* if */
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

  {
    memsizetype pos;

  /* polAddWriteCheck */
#ifdef CHECK_MAXIMUM_FD_NUMBER
    if (aSocket < 0 || aSocket >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
#ifdef CHECK_FD_SETSIZE
    if (conv(pollData)->writeCheck.size >= FD_SETSIZE) {
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    pos = (memsizetype) hshIdxEnterDefault(conv(pollData)->writeCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->writeCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
        (createfunctype) &intCreateGeneric, (createfunctype) &intCreateGeneric);
    if (pos == conv(pollData)->writeCheck.size) {
      if (conv(pollData)->writeCheck.size >= conv(pollData)->writeCheck.capacity) {
        conv(pollData)->writeCheck.files = REALLOC_TABLE(conv(pollData)->writeCheck.files,
            fdAndFileType, conv(pollData)->writeCheck.capacity,
            conv(pollData)->writeCheck.capacity + TABLE_INCREMENT);
        if (conv(pollData)->writeCheck.files == NULL ||
            !reallocFdSet(&conv(pollData)->writeCheck, TABLE_INCREMENT)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_TABLE(fdAndFileType, conv(pollData)->writeCheck.capacity,
                       conv(pollData)->writeCheck.capacity + TABLE_INCREMENT);
          conv(pollData)->writeCheck.capacity += TABLE_INCREMENT;
        } /* if */
      } /* if */
      conv(pollData)->writeCheck.size++;
      conv(pollData)->writeCheck.files[pos].fd = aSocket;
      conv(pollData)->writeCheck.files[pos].file = fileObj;
#ifdef USE_PREPARED_FD_SET
      FD_SET(aSocket, to_write_inFdset(pollData));
#ifdef SELECT_WITH_NFDS
      if (aSocket >= conv(pollData)->writeCheck.preparedNfds) {
        conv(pollData)->writeCheck.preparedNfds = (int) aSocket + 1;
      } /* if */
#endif
#endif
    } /* if */
  } /* polAddWriteCheck */



#ifdef ANSI_C

void polClear (polltype pollData)
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
    conv(pollData)->readCheck.preparedNfds = 0;
    conv(pollData)->writeCheck.preparedNfds = 0;
#endif
#endif
    FD_ZERO(to_read_outFdset(pollData));
    FD_ZERO(to_write_outFdset(pollData));
    conv(pollData)->readCheck.size = 0;
    conv(pollData)->readCheck.pos = 0;
    hshDestr(conv(pollData)->readCheck.indexHash, (destrfunctype) &intDestrGeneric,
             (destrfunctype) &intDestrGeneric);
    conv(pollData)->readCheck.indexHash = hshEmpty();
    conv(pollData)->writeCheck.size = 0;
    conv(pollData)->writeCheck.pos = 0;
    hshDestr(conv(pollData)->writeCheck.indexHash, (destrfunctype) &intDestrGeneric,
             (destrfunctype) &intDestrGeneric);
    conv(pollData)->writeCheck.indexHash = hshEmpty();
    conv(pollData)->numOfEvents = 0;
  } /* polClear */



#ifdef ANSI_C

void polCpy (polltype poll_to, const const_polltype pollDataFrom)
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
      newReadIndexHash = hshCreate(conv(pollDataFrom)->readCheck.indexHash,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
      newWriteIndexHash = hshCreate(conv(pollDataFrom)->writeCheck.indexHash,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
          (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
      if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        pollData = conv(poll_to);
        if (pollData->readCheck.capacity < conv(pollDataFrom)->readCheck.size) {
          if (unlikely(!ALLOC_TABLE(newReadFiles, fdAndFileType,
              conv(pollDataFrom)->readCheck.capacity) ||
              !replaceFdSet(&pollData->readCheck, conv(pollDataFrom)->readCheck.capacity))) {
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->readCheck.files, fdAndFileType, pollData->readCheck.capacity);
            pollData->readCheck.capacity = conv(pollDataFrom)->readCheck.capacity;
            pollData->readCheck.files = newReadFiles;
          } /* if */
        } /* if */
        if (pollData->writeCheck.capacity < conv(pollDataFrom)->writeCheck.size) {
          if (unlikely(!ALLOC_TABLE(newWriteFiles, fdAndFileType,
              conv(pollDataFrom)->writeCheck.capacity) ||
              !replaceFdSet(&pollData->writeCheck, conv(pollDataFrom)->writeCheck.capacity))) {
            raise_error(MEMORY_ERROR);
            return;
          } else {
            FREE_TABLE(pollData->writeCheck.files, fdAndFileType, pollData->writeCheck.capacity);
            pollData->writeCheck.capacity = conv(pollDataFrom)->writeCheck.capacity;
            pollData->writeCheck.files = newWriteFiles;
          } /* if */
        } /* if */
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_read_inFdset(pollData), to_read_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->readCheck);
#ifdef SELECT_WITH_NFDS
        pollData->readCheck.preparedNfds = conv(pollDataFrom)->readCheck.preparedNfds;
#endif
#endif
        copyFdSet(to_read_outFdset(pollData), to_read_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->readCheck);
        pollData->readCheck.size = conv(pollDataFrom)->readCheck.size;
        pollData->readCheck.pos = conv(pollDataFrom)->readCheck.pos;
        memcpy(pollData->readCheck.files, conv(pollDataFrom)->readCheck.files,
            conv(pollDataFrom)->readCheck.size * sizeof(fdAndFileType));
        hshDestr(pollData->readCheck.indexHash, (destrfunctype) &intDestrGeneric,
                 (destrfunctype) &intDestrGeneric);
        pollData->readCheck.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_write_inFdset(pollData), to_write_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeCheck);
#ifdef SELECT_WITH_NFDS
        pollData->writeCheck.preparedNfds = conv(pollDataFrom)->writeCheck.preparedNfds;
#endif
#endif
        copyFdSet(to_write_outFdset(pollData), to_write_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeCheck);
        pollData->writeCheck.size = conv(pollDataFrom)->writeCheck.size;
        pollData->writeCheck.pos = conv(pollDataFrom)->writeCheck.pos;
        memcpy(pollData->writeCheck.files, conv(pollDataFrom)->writeCheck.files,
               conv(pollDataFrom)->writeCheck.size * sizeof(fdAndFileType));
        hshDestr(pollData->writeCheck.indexHash, (destrfunctype) &intDestrGeneric,
                 (destrfunctype) &intDestrGeneric);
        pollData->writeCheck.indexHash = newWriteIndexHash;
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
    newReadIndexHash = hshCreate(conv(pollDataFrom)->readCheck.indexHash,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
    newWriteIndexHash = hshCreate(conv(pollDataFrom)->writeCheck.indexHash,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric,
        (createfunctype) &intCreateGeneric, (destrfunctype) &intDestrGeneric);
    if (unlikely(newReadIndexHash == NULL || newWriteIndexHash == NULL ||
                 !ALLOC_RECORD(result, select_based_pollrecord, count.polldata))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_TABLE(result->readCheck.files, fdAndFileType,
                                conv(pollDataFrom)->readCheck.capacity) ||
                   !ALLOC_TABLE(result->writeCheck.files, fdAndFileType,
                                conv(pollDataFrom)->writeCheck.capacity) ||
                   !allocFdSet(&result->readCheck, conv(pollDataFrom)->readCheck.capacity) ||
                   !allocFdSet(&result->writeCheck, conv(pollDataFrom)->writeCheck.capacity))) {
        if (result->readCheck.files != NULL) {
          FREE_TABLE(result->readCheck.files, fdAndFileType, conv(pollDataFrom)->readCheck.capacity);
          if (result->writeCheck.files != NULL) {
            FREE_TABLE(result->writeCheck.files, fdAndFileType, conv(pollDataFrom)->readCheck.capacity);
            freeFdSet(&result->readCheck, conv(pollDataFrom)->readCheck.capacity);
          } /* if */
        } /* if */
        FREE_RECORD(result, select_based_pollrecord, count.polldata);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_read_inFdset(result), to_read_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->readCheck);
#ifdef SELECT_WITH_NFDS
        result->readCheck.preparedNfds = conv(pollDataFrom)->readCheck.preparedNfds;
#endif
#endif
        copyFdSet(to_read_outFdset(result), to_read_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->readCheck);
        result->readCheck.size = conv(pollDataFrom)->readCheck.size;
        result->readCheck.capacity = conv(pollDataFrom)->readCheck.capacity;
        result->readCheck.pos = conv(pollDataFrom)->readCheck.pos;
        memcpy(result->readCheck.files, conv(pollDataFrom)->readCheck.files,
               conv(pollDataFrom)->readCheck.size * sizeof(fdAndFileType));
        result->readCheck.indexHash = newReadIndexHash;
#ifdef USE_PREPARED_FD_SET
        copyFdSet(to_write_inFdset(result), to_write_inFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeCheck);
#ifdef SELECT_WITH_NFDS
        result->writeCheck.preparedNfds = conv(pollDataFrom)->writeCheck.preparedNfds;
#endif
#endif
        copyFdSet(to_write_outFdset(result), to_write_outFdset(pollDataFrom),
                  &conv(pollDataFrom)->writeCheck);
        result->writeCheck.size = conv(pollDataFrom)->writeCheck.size;
        result->writeCheck.capacity = conv(pollDataFrom)->writeCheck.capacity;
        result->writeCheck.pos = conv(pollDataFrom)->writeCheck.pos;
        memcpy(result->writeCheck.files, conv(pollDataFrom)->writeCheck.files,
               conv(pollDataFrom)->writeCheck.size * sizeof(fdAndFileType));
        result->writeCheck.indexHash = newWriteIndexHash;
        result->numOfEvents = conv(pollDataFrom)->numOfEvents;
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

  {
    memsizetype capacity;

  /* polDestr */
    if (oldPollData != NULL) {
      capacity = conv(oldPollData)->readCheck.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->readCheck.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->readCheck.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->readCheck.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->readCheck.indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      capacity = conv(oldPollData)->writeCheck.capacity;
#ifdef DYNAMIC_FD_SET
#ifdef USE_PREPARED_FD_SET
      FREE_FDSET(conv(oldPollData)->writeCheck.inFdset, capacity);
#endif
      FREE_FDSET(conv(oldPollData)->writeCheck.outFdset, capacity);
#endif
      FREE_TABLE(conv(oldPollData)->writeCheck.files, fdAndFileType, capacity);
      hshDestr(conv(oldPollData)->writeCheck.indexHash, (destrfunctype) &intDestrGeneric,
               (destrfunctype) &intDestrGeneric);
      FREE_RECORD(conv(oldPollData), select_based_pollrecord, count.pollData);
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
      if (unlikely(!ALLOC_TABLE(result->readCheck.files, fdAndFileType, TABLE_START_SIZE) ||
                   !ALLOC_TABLE(result->writeCheck.files, fdAndFileType, TABLE_START_SIZE) ||
                   !allocFdSet(&result->readCheck, TABLE_START_SIZE) ||
                   !allocFdSet(&result->writeCheck, TABLE_START_SIZE))) {
        if (result->readCheck.files != NULL) {
          FREE_TABLE(result->readCheck.files, fdAndFileType, TABLE_START_SIZE);
          if (result->writeCheck.files != NULL) {
            FREE_TABLE(result->writeCheck.files, fdAndFileType, TABLE_START_SIZE);
            freeFdSet(&result->readCheck, TABLE_START_SIZE);
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
        result->readCheck.preparedNfds = 0;
        result->writeCheck.preparedNfds = 0;
#endif
#endif
        FD_ZERO(to_read_outFdset(result));
        FD_ZERO(to_write_outFdset(result));
        result->readCheck.size = 0;
        result->readCheck.capacity = TABLE_START_SIZE;
        result->readCheck.pos = 0;
        result->readCheck.indexHash = newReadIndexHash;
        result->writeCheck.size = 0;
        result->writeCheck.capacity = TABLE_START_SIZE;
        result->writeCheck.pos = 0;
        result->writeCheck.indexHash = newWriteIndexHash;
        result->numOfEvents = 0;
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
    memsizetype size;
    memsizetype pos;
    rtlObjecttype *element;
    rtlArraytype fileArray;

  /* polFiles */
    size = conv(pollData)->readCheck.size + conv(pollData)->writeCheck.size;
    if (!ALLOC_RTL_ARRAY(fileArray, size)) {
      raise_error(MEMORY_ERROR);
    } else {
      fileArray->min_position = 1;
      fileArray->max_position = (inttype) size;
      element = fileArray->arr;
      for (pos = 0; pos < conv(pollData)->readCheck.size; pos++, element++) {
        element->value.genericvalue = conv(pollData)->readCheck.files[pos].file;
      } /* for */
      for (pos = 0; pos < conv(pollData)->writeCheck.size; pos++, element++) {
        element->value.genericvalue = conv(pollData)->writeCheck.files[pos].file;
      } /* for */
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
    memsizetype readPos;
    fd_set *readFds;
    booltype hasNext;

  /* polHasNextReadFile */
    if (conv(pollData)->numOfEvents == 0) {
      hasNext = FALSE;
      conv(pollData)->readCheck.pos = conv(pollData)->readCheck.size;
    } else {
      readPos = conv(pollData)->readCheck.pos;
      readFds = to_read_outFdset(pollData);
      while (readPos < conv(pollData)->readCheck.size &&
          !FD_ISSET(conv(pollData)->readCheck.files[readPos].fd, readFds)) {
        readPos++;
      } /* while */
      hasNext = readPos < conv(pollData)->readCheck.size;
      conv(pollData)->readCheck.pos = readPos;
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
    memsizetype writePos;
    fd_set *writeFds;
    booltype hasNext;

  /* polHasNextWriteFile */
    if (conv(pollData)->numOfEvents == 0) {
      hasNext = FALSE;
      conv(pollData)->writeCheck.pos = conv(pollData)->writeCheck.size;
    } else {
      writePos = conv(pollData)->writeCheck.pos;
      writeFds = to_write_outFdset(pollData);
      while (writePos < conv(pollData)->writeCheck.size &&
          !FD_ISSET(conv(pollData)->writeCheck.files[writePos].fd, writeFds)) {
        writePos++;
      } /* while */
      hasNext = writePos < conv(pollData)->writeCheck.size;
      conv(pollData)->writeCheck.pos = writePos;
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
    memsizetype readPos;
    fd_set *readFds;
    rtlGenerictype resultFile;

  /* polNextReadFile */
    if (conv(pollData)->numOfEvents == 0) {
      resultFile = nullFile;
      conv(pollData)->readCheck.pos = conv(pollData)->readCheck.size;
    } else {
      readPos = conv(pollData)->readCheck.pos;
      readFds = to_read_outFdset(pollData);
      while (readPos < conv(pollData)->readCheck.size &&
          !FD_ISSET(conv(pollData)->readCheck.files[readPos].fd, readFds)) {
        readPos++;
      } /* while */
      if (readPos < conv(pollData)->readCheck.size) {
        resultFile = conv(pollData)->readCheck.files[readPos].file;
        /* printf("polNextReadFile -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, readPos,
            conv(pollData)->readCheck.files[readPos].fd); */
        conv(pollData)->numOfEvents--;
        readPos++;
      } else {
        resultFile = nullFile;
      } /* if */
      conv(pollData)->readCheck.pos = readPos;
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
    memsizetype writePos;
    fd_set *writeFds;
    rtlGenerictype resultFile;

  /* polNextWriteFile */
    if (conv(pollData)->numOfEvents == 0) {
      resultFile = nullFile;
      conv(pollData)->writeCheck.pos = conv(pollData)->writeCheck.size;
    } else {
      writePos = conv(pollData)->writeCheck.pos;
      writeFds = to_write_outFdset(pollData);
      while (writePos < conv(pollData)->writeCheck.size &&
          !FD_ISSET(conv(pollData)->writeCheck.files[writePos].fd, writeFds)) {
        writePos++;
      } /* while */
      if (writePos < conv(pollData)->writeCheck.size) {
        resultFile = conv(pollData)->writeCheck.files[writePos].file;
        /* printf("polNextWriteFile -> 0x%lx fd[%u]=%d\n",
            (unsigned long) resultFile, writePos,
            conv(pollData)->writeCheck.files[writePos].fd); */
        conv(pollData)->numOfEvents--;
        writePos++;
      } else {
        resultFile = nullFile;
      } /* if */
      conv(pollData)->writeCheck.pos = writePos;
    } /* if */
    return resultFile;
  } /* polNextWriteFile */



#ifdef ANSI_C

void polPoll (polltype pollData)
#else

void polPoll (pollData)
polltype pollData;
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

  /* polPoll */
    /* printf("polPoll\n");
       dumpPoll(pollData); */
    readFds = to_read_outFdset(pollData);
#ifdef USE_PREPARED_FD_SET
    copyFdSet(readFds, to_read_inFdset(pollData),
              &conv(pollData)->readCheck.files);
#ifdef SELECT_WITH_NFDS
    nfds = conv(pollData)->readCheck.preparedNfds;
#endif
#else
    FD_ZERO(readFds);
    for (pos = 0; pos < conv(pollData)->readCheck.size; pos++) {
      sock = conv(pollData)->readCheck.files[pos].fd;
      FD_SET(sock, readFds);
#ifdef SELECT_WITH_NFDS
      if ((int) sock >= nfds) {
        nfds = (int) sock + 1;
      } /* if */
#endif
    } /* for */
#endif
    writeFds = to_write_outFdset(pollData);
#ifdef USE_PREPARED_FD_SET
    copyFdSet(writeFds, to_write_inFdset(pollData),
              &conv(pollData)->writeCheck.files);
#ifdef SELECT_WITH_NFDS
    if (conv(pollData)->writeCheck.preparedNfds > nfds) {
      nfds = conv(pollData)->writeCheck.preparedNfds;
    } /* if */
#endif
#else
    FD_ZERO(writeFds);
    for (pos = 0; pos < conv(pollData)->writeCheck.size; pos++) {
      sock = conv(pollData)->writeCheck.files[pos].fd;
      FD_SET(sock, writeFds);
#ifdef SELECT_WITH_NFDS
      if ((int) sock >= nfds) {
        nfds = (int) sock + 1;
      } /* if */
#endif
    } /* for */
#endif
    /* printf("polPoll %lx: nfds=%d\n", pollData, nfds); */
    select_result = select(nfds, readFds, writeFds, NULL, NULL); /* &timeout); */
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
      /* printf("nfds=%d", nfds); */
      raise_error(FILE_ERROR);
    } else {
      conv(pollData)->readCheck.pos = 0;
      conv(pollData)->writeCheck.pos = 0;
      conv(pollData)->numOfEvents = (memsizetype) select_result;
    } /* if */
  } /* polPoll */



#ifdef ANSI_C

booltype polReadyForRead (polltype pollData, const sockettype aSocket)
#else

booltype polReadyForRead (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    booltype result;

  /* polReadyForRead */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->readCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->readCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos == conv(pollData)->readCheck.size) {
      result = FALSE;
    } else {
      result = FD_ISSET(conv(pollData)->readCheck.files[pos].fd,
                        to_read_outFdset(pollData)) != 0;
      /* printf("polReadyForRead: sock=%d, pos=%d, fd=%d\n",
          aSocket, pos, conv(pollData)->readCheck.files[pos].fd); */
    } /* if */
    return result;
  } /* polReadyForRead */



#ifdef ANSI_C

booltype polReadyForWrite (polltype pollData, const sockettype aSocket)
#else

booltype polReadyForWrite (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;
    booltype result;

  /* polReadyForWrite */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->writeCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->writeCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos == conv(pollData)->writeCheck.size) {
      result = FALSE;
    } else {
      result = FD_ISSET(conv(pollData)->writeCheck.files[pos].fd,
                        to_write_outFdset(pollData)) != 0;
      /* printf("polReadyForWrite: sock=%d, pos=%d, fd=%d\n",
          aSocket, pos, conv(pollData)->writeCheck.files[pos].fd); */
    } /* if */
    return result;
  } /* polReadyForWrite */



#ifdef ANSI_C

void polRemoveReadCheck (polltype pollData, const sockettype aSocket)
#else

void polRemoveReadCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;

  /* polRemoveReadCheck */
    /* printf("polRemoveReadCheck(..., %u)\n", aSocket); */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->readCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->readCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos != conv(pollData)->readCheck.size) {
      if (conv(pollData)->readCheck.pos <= pos + 1 &&
          conv(pollData)->writeCheck.pos <= pos + 1) {
        conv(pollData)->readCheck.size--;
        if (pos != conv(pollData)->readCheck.size) {
          memcpy(&conv(pollData)->readCheck.files[pos],
                 &conv(pollData)->readCheck.files[conv(pollData)->readCheck.size],
                 sizeof(fdAndFileType));
          hshIdxAddr(conv(pollData)->readCheck.indexHash,
                     (rtlGenerictype) (memsizetype) conv(pollData)->readCheck.files[pos].fd,
                     (inttype) (memsizetype) conv(pollData)->readCheck.files[pos].fd,
                     (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
        } /* if */
        hshExcl(conv(pollData)->readCheck.indexHash, (rtlGenerictype) (memsizetype) aSocket,
                (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
                (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
        if (conv(pollData)->readCheck.pos == pos + 1) {
          conv(pollData)->readCheck.pos--;
        } /* if */
        if (conv(pollData)->writeCheck.pos == pos + 1) {
          conv(pollData)->writeCheck.pos--;
        } /* if */
      } /* if */
#ifdef USE_PREPARED_FD_SET
      FD_CLR(aSocket, to_read_inFdset(pollData));
#endif
    } /* if */
  } /* polRemoveReadCheck */



#ifdef ANSI_C

void polRemoveWriteCheck (polltype pollData, const sockettype aSocket)
#else

void polRemoveWriteCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  {
    memsizetype pos;

  /* polRemoveWriteCheck */
    pos = (memsizetype) hshIdxWithDefault(conv(pollData)->writeCheck.indexHash,
        (rtlGenerictype) (memsizetype) aSocket, (rtlGenerictype) conv(pollData)->writeCheck.size,
        (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric);
    if (pos != conv(pollData)->writeCheck.size) {
      if (conv(pollData)->readCheck.pos <= pos + 1 &&
          conv(pollData)->writeCheck.pos <= pos + 1) {
        conv(pollData)->writeCheck.size--;
        if (pos != conv(pollData)->writeCheck.size) {
          memcpy(&conv(pollData)->writeCheck.files[pos],
                 &conv(pollData)->writeCheck.files[conv(pollData)->writeCheck.size],
                 sizeof(fdAndFileType));
          hshIdxAddr(conv(pollData)->writeCheck.indexHash,
                     (rtlGenerictype) (memsizetype) conv(pollData)->writeCheck.files[pos].fd,
                     (inttype) (memsizetype) conv(pollData)->writeCheck.files[pos].fd,
                     (comparetype) &uintCmpGeneric)->value.genericvalue = (rtlGenerictype) pos;
        } /* if */
        hshExcl(conv(pollData)->writeCheck.indexHash, (rtlGenerictype) (memsizetype) aSocket,
                (inttype) (memsizetype) aSocket, (comparetype) &uintCmpGeneric,
                (destrfunctype) &intDestrGeneric, (destrfunctype) &intDestrGeneric);
        if (conv(pollData)->readCheck.pos == pos + 1) {
          conv(pollData)->readCheck.pos--;
        } /* if */
        if (conv(pollData)->writeCheck.pos == pos + 1) {
          conv(pollData)->writeCheck.pos--;
        } /* if */
      } /* if */
#ifdef USE_PREPARED_FD_SET
      FD_CLR(aSocket, to_write_inFdset(pollData));
#endif
    } /* if */
  } /* polRemoveWriteCheck */
