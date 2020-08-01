/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/pollib.c                                        */
/*  Changes: 2011, 2013  Thomas Mertes                              */
/*  Content: All primitive actions to support poll.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "executl.h"
#include "runerr.h"
#include "pol_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "pollib.h"



static genericType incrUsageCount (const genericType pollFile)

  {
    objectType fileObject;
    structType fileStruct;
    objectType newFileOject;

  /* incrUsageCount */
    fileObject = (objectType) (memSizeType) pollFile;
    if (CATEGORY_OF_OBJ(fileObject) != STRUCTOBJECT) {
      run_error(STRUCTOBJECT, fileObject);
      return 0;
    } else {
      fileStruct = take_struct(fileObject);
      if (fileStruct == NULL) {
        empty_value(fileObject);
        return 0;
      } else {
        if (fileStruct->usage_count != 0) {
          fileStruct->usage_count++;
        } /* if */
        if (TEMP_OBJECT(fileObject) || TEMP2_OBJECT(fileObject)) {
          if (!ALLOC_OBJECT(newFileOject)) {
            CLEAR_TEMP_FLAG(fileObject);
            CLEAR_TEMP2_FLAG(fileObject);
            raise_error(MEMORY_ERROR);
          } else {
            memcpy(newFileOject, fileObject, sizeof(objectRecord));
            CLEAR_TEMP_FLAG(newFileOject);
            CLEAR_TEMP2_FLAG(newFileOject);
            fileObject = newFileOject;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return (genericType) (memSizeType) fileObject;
  } /* incrUsageCount */



static void decrUsageCount (const genericType pollFile)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* decrUsageCount */
    do_destroy((objectType) (memSizeType) pollFile, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* decrUsageCount */



static void initPollOps (void)

  { /* initPollOps */
    fileObjectOps.incrUsageCount = incrUsageCount;
    fileObjectOps.decrUsageCount = decrUsageCount;
  } /* initPollOps */



/**
 *  Add eventsToCheck/arg_3 for aSocket/arg_2 to pollData/arg_1.
 *  EventsToCheck/arg_3 can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData/arg_1 Poll data to which the event checks are added.
 *  @param aSocket/arg_2 Socket for which the events should be checked.
 *  @param eventsToCheck/arg_3 Events to be added to the checkedEvents
 *         field of pollData/arg_1.
 *  @param fileObj/arg_4 File to be returned, when the iterator returns
 *         files in pollData/arg_1.
 *  @exception RANGE_ERROR Illegal value for eventsToCheck/arg_3.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 *  @exception FILE_ERROR A limit of the operating system was reached.
 */
objectType pol_addCheck (listType arguments)

  { /* pol_addCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_interface(arg_4(arguments));
    isit_struct(take_interface(arg_4(arguments)));
    polAddCheck(take_poll(arg_1(arguments)),
                take_socket(arg_2(arguments)),
                take_int(arg_3(arguments)),
                (genericType) (memSizeType) take_interface(arg_4(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_addCheck */



/**
 *  Clears pollData/arg_1.
 *  All sockets and all events are removed from pollData/arg_1 and
 *  the iterator is reset, such that pol_hasNext() returns FALSE.
 */
objectType pol_clear (listType arguments)

  { /* pol_clear */
    isit_poll(arg_1(arguments));
    polClear(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_clear */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType pol_cpy (listType arguments)

  {
    objectType dest;
    objectType source;

  /* pol_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_poll(dest);
    isit_poll(source);
    is_variable(dest);
    if (TEMP_OBJECT(source)) {
      polDestr(take_poll(dest));
      dest->value.pollValue = take_poll(source);
      source->value.pollValue = NULL;
    } else {
      polCpy(dest->value.pollValue, take_poll(source));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pol_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType pol_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* pol_create */
    if (fileObjectOps.incrUsageCount == NULL) {
      initPollOps();
    } /* if */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_poll(source);
    SET_CATEGORY_OF_OBJ(dest, POLLOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.pollValue = take_poll(source);
      source->value.pollValue = NULL;
    } else {
      dest->value.pollValue = polCreate(take_poll(source));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pol_create */



/**
 *  Free the memory referred by 'pollData/arg_1'.
 *  After pol_destr is left 'pollData/arg_1' is NULL.
 *  The memory where 'pollData/arg_1' is stored can be freed afterwards.
 */
objectType pol_destr (listType arguments)

  { /* pol_destr */
    isit_poll(arg_1(arguments));
    polDestr(take_poll(arg_1(arguments)));
    arg_1(arguments)->value.pollValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* pol_destr */



/**
 *  Create an empty poll data value.
 *  @return an empty poll data value.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType pol_empty (listType arguments)

  { /* pol_empty */
    return bld_poll_temp(polEmpty());
  } /* pol_empty */



objectType pol_getCheck (listType arguments)

  {
    intType result;

  /* pol_getCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetCheck(take_poll(arg_1(arguments)),
                          take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getCheck */



objectType pol_getFinding (listType arguments)

  {
    intType result;

  /* pol_getFinding */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetFinding(take_poll(arg_1(arguments)),
                           take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getFinding */



objectType pol_hasNext (listType arguments)

  {
    objectType result;

  /* pol_hasNext */
    isit_poll(arg_1(arguments));
    if (polHasNext(take_poll(arg_1(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_hasNext */



objectType pol_iterChecks (listType arguments)

  { /* pol_iterChecks */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterChecks(take_poll(arg_1(arguments)),
                  take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterChecks */



objectType pol_iterFindings (listType arguments)

  { /* pol_iterFindings */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterFindings(take_poll(arg_1(arguments)),
                    take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterFindings */



objectType pol_nextFile (listType arguments)

  {
    objectType nextFile;

  /* pol_nextFile */
    /* printf("pol_nextFile ");
    trace1(arg_2(arguments));
    printf("\n"); */
    isit_poll(arg_1(arguments));
    isit_interface(arg_2(arguments));
    isit_struct(take_interface(arg_2(arguments)));
    nextFile = (objectType) (memSizeType)
               polNextFile(take_poll(arg_1(arguments)),
                           (genericType) (memSizeType) take_interface(arg_2(arguments)));
    isit_struct(nextFile);
    if (nextFile->value.structValue->usage_count != 0) {
      nextFile->value.structValue->usage_count++;
    } /* if */
    /* printf("pol_nextFile ->");
    trace1(nextFile);
    printf("\n"); */
    return bld_interface_temp(nextFile);
  } /* pol_nextFile */



objectType pol_poll (listType arguments)

  { /* pol_poll */
    isit_poll(arg_1(arguments));
    polPoll(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_poll */



/**
 *  Remove eventsToCheck/arg_3 for aSocket/arg_2 from pollData/arg_1.
 *  EventsToCheck/arg_3 can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData/arg_1 Poll data from which the event checks are removed.
 *  @param aSocket/arg_2 Socket for which the events should not be checked.
 *  @param eventsToCheck/arg_3 Events to be removed from the checkedEvents
 *         field of pollData/arg_1.
 *  @exception RANGE_ERROR Illegal value for eventsToCheck/arg_3.
 */
objectType pol_removeCheck (listType arguments)

  { /* pol_removeCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    polRemoveCheck(take_poll(arg_1(arguments)),
                   take_socket(arg_2(arguments)),
                   take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_removeCheck */



objectType pol_value (listType arguments)

  {
    objectType obj_arg;

  /* pol_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 CATEGORY_OF_OBJ(obj_arg) != POLLOBJECT)) {
      logError(printf("pol_value(");
               trace1(obj_arg);
               printf("): Category is not POLLOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_poll_temp(polCreate(take_poll(obj_arg)));
    } /* if */
  } /* pol_value */
