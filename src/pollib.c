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



static generictype incrUsageCount (const generictype pollFile)

  {
    objecttype fileObject;
    structtype fileStruct;
    objecttype newFileOject;

  /* incrUsageCount */
    fileObject = (objecttype) (memsizetype) pollFile;
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
            memcpy(newFileOject, fileObject, sizeof(objectrecord));
            CLEAR_TEMP_FLAG(newFileOject);
            CLEAR_TEMP2_FLAG(newFileOject);
            fileObject = newFileOject;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return (generictype) (memsizetype) fileObject;
  } /* incrUsageCount */



static void decrUsageCount (const generictype pollFile)

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* decrUsageCount */
    do_destroy((objecttype) (memsizetype) pollFile, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* decrUsageCount */



static void initPollOps (void)

  { /* initPollOps */
    fileObjectOps.incrUsageCount = incrUsageCount;
    fileObjectOps.decrUsageCount = decrUsageCount;
  } /* initPollOps */



objecttype pol_addCheck (listtype arguments)

  { /* pol_addCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_interface(arg_4(arguments));
    isit_struct(take_interface(arg_4(arguments)));
    polAddCheck(take_poll(arg_1(arguments)),
                take_socket(arg_2(arguments)),
                take_int(arg_3(arguments)),
                (generictype) (memsizetype) take_interface(arg_4(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_addCheck */



objecttype pol_clear (listtype arguments)

  { /* pol_clear */
    isit_poll(arg_1(arguments));
    polClear(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_clear */



objecttype pol_cpy (listtype arguments)

  {
    objecttype poll_to;
    objecttype poll_from;

  /* pol_cpy */
    poll_to = arg_1(arguments);
    poll_from = arg_3(arguments);
    isit_poll(poll_to);
    isit_poll(poll_from);
    is_variable(poll_to);
    if (TEMP_OBJECT(poll_from)) {
      polDestr(take_poll(poll_to));
      poll_to->value.pollvalue = take_poll(poll_from);
      poll_from->value.pollvalue = NULL;
    } else {
      polCpy(poll_to->value.pollvalue, take_poll(poll_from));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pol_cpy */



objecttype pol_create (listtype arguments)

  {
    objecttype poll_to;
    objecttype poll_from;

  /* pol_create */
    if (fileObjectOps.incrUsageCount == NULL) {
      initPollOps();
    } /* if */
    poll_to = arg_1(arguments);
    poll_from = arg_3(arguments);
    isit_poll(poll_from);
    SET_CATEGORY_OF_OBJ(poll_to, POLLOBJECT);
    if (TEMP_OBJECT(poll_from)) {
      poll_to->value.pollvalue = take_poll(poll_from);
      poll_from->value.pollvalue = NULL;
    } else {
      poll_to->value.pollvalue = polCreate(take_poll(poll_from));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pol_create */



objecttype pol_destr (listtype arguments)

  { /* pol_destr */
    isit_poll(arg_1(arguments));
    polDestr(take_poll(arg_1(arguments)));
    arg_1(arguments)->value.pollvalue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* pol_destr */



objecttype pol_empty (listtype arguments)

  { /* pol_empty */
    return bld_poll_temp(polEmpty());
  } /* pol_empty */



objecttype pol_getCheck (listtype arguments)

  {
    inttype result;

  /* pol_getCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetCheck(take_poll(arg_1(arguments)),
                          take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getCheck */



objecttype pol_getFinding (listtype arguments)

  {
    inttype result;

  /* pol_getFinding */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetFinding(take_poll(arg_1(arguments)),
                           take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getFinding */



objecttype pol_hasNext (listtype arguments)

  {
    objecttype result;

  /* pol_hasNext */
    isit_poll(arg_1(arguments));
    if (polHasNext(take_poll(arg_1(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_hasNext */



objecttype pol_iterChecks (listtype arguments)

  { /* pol_iterChecks */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterChecks(take_poll(arg_1(arguments)),
                  take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterChecks */



objecttype pol_iterFindings (listtype arguments)

  { /* pol_iterFindings */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterFindings(take_poll(arg_1(arguments)),
                    take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterFindings */



objecttype pol_nextFile (listtype arguments)

  {
    objecttype nextFile;

  /* pol_nextFile */
    /* printf("pol_nextFile ");
    trace1(arg_2(arguments));
    printf("\n"); */
    isit_poll(arg_1(arguments));
    isit_interface(arg_2(arguments));
    isit_struct(take_interface(arg_2(arguments)));
    nextFile = (objecttype) (memsizetype)
               polNextFile(take_poll(arg_1(arguments)),
                           (generictype) (memsizetype) take_interface(arg_2(arguments)));
    isit_struct(nextFile);
    if (nextFile->value.structvalue->usage_count != 0) {
      nextFile->value.structvalue->usage_count++;
    } /* if */
    /* printf("pol_nextFile ->");
    trace1(nextFile);
    printf("\n"); */
    return bld_interface_temp(nextFile);
  } /* pol_nextFile */



objecttype pol_poll (listtype arguments)

  { /* pol_poll */
    isit_poll(arg_1(arguments));
    polPoll(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_poll */



objecttype pol_removeCheck (listtype arguments)

  { /* pol_removeCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    polRemoveCheck(take_poll(arg_1(arguments)),
                   take_socket(arg_2(arguments)),
                   take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_removeCheck */



objecttype pol_value (listtype arguments)

  {
    objecttype obj_arg;

  /* pol_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != POLLOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_poll_temp(polCreate(take_poll(obj_arg)));
    } /* if */
  } /* pol_value */
