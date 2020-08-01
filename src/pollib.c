/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: All primitive actions to support poll.                 */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "pol_drv.h"



#ifdef ANSI_C

objecttype pol_addReadCheck (listtype arguments)
#else

objecttype pol_addReadCheck (arguments)
listtype arguments;
#endif

  { /* pol_addReadCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_interface(arg_3(arguments));
    polAddReadCheck(take_poll(arg_1(arguments)),
                    take_socket(arg_2(arguments)),
                    (rtlGenerictype) take_interface(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_addReadCheck */



#ifdef ANSI_C

objecttype pol_addWriteCheck (listtype arguments)
#else

objecttype pol_addWriteCheck (arguments)
listtype arguments;
#endif

  { /* pol_addWriteCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_interface(arg_3(arguments));
    polAddWriteCheck(take_poll(arg_1(arguments)),
                     take_socket(arg_2(arguments)),
                     (rtlGenerictype) take_interface(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_addWriteCheck */



#ifdef ANSI_C

objecttype pol_clear (listtype arguments)
#else

objecttype pol_clear (arguments)
listtype arguments;
#endif

  { /* pol_clear */
    isit_poll(arg_1(arguments));
    polClear(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_clear */



#ifdef ANSI_C

objecttype pol_cpy (listtype arguments)
#else

objecttype pol_cpy (arguments)
listtype arguments;
#endif

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



#ifdef ANSI_C

objecttype pol_create (listtype arguments)
#else

objecttype pol_create (arguments)
listtype arguments;
#endif

  {
    objecttype poll_to;
    objecttype poll_from;

  /* pol_create */
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



#ifdef ANSI_C

objecttype pol_destr (listtype arguments)
#else

objecttype pol_destr (arguments)
listtype arguments;
#endif

  { /* pol_destr */
    isit_poll(arg_1(arguments));
    polDestr(take_poll(arg_1(arguments)));
    arg_1(arguments)->value.pollvalue = NULL;
    return SYS_EMPTY_OBJECT;
  } /* pol_destr */



#ifdef ANSI_C

objecttype pol_empty (listtype arguments)
#else

objecttype pol_empty (arguments)
listtype arguments;
#endif

  { /* pol_empty */
    return bld_poll_temp(polEmpty());
  } /* pol_empty */



#ifdef ANSI_C

objecttype pol_files (listtype arguments)
#else

objecttype pol_files (arguments)
listtype arguments;
#endif

  {
    rtlArraytype fileArray;
    memsizetype arraySize;
    arraytype resultFileArray;
    memsizetype pos;

  /* pol_files */
    isit_poll(arg_1(arguments));
    fileArray = polFiles(take_poll(arg_1(arguments)));
    if (fileArray == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      arraySize = (uinttype) (fileArray->max_position - fileArray->min_position + 1);
      if (!ALLOC_ARRAY(resultFileArray, arraySize)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        resultFileArray->min_position = fileArray->min_position;
        resultFileArray->max_position = fileArray->max_position;
        for (pos = 0; pos < arraySize; pos++) {
          resultFileArray->arr[pos].value.objvalue = bld_interface_temp(
	      (objecttype) fileArray->arr[pos].value.genericvalue);
          /* printf("pol_file[%u]: %08lx ", pos, resultFileArray->arr[pos].value.objvalue);
             trace1(resultFileArray->arr[pos].value.objvalue);
             printf("\n"); */
        } /* for */
      } /* if */
    } /* if */
    return bld_array_temp(resultFileArray);
  } /* pol_files */



#ifdef ANSI_C

objecttype pol_hasNextReadFile (listtype arguments)
#else

objecttype pol_hasNextReadFile (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* pol_hasNextReadFile */
    isit_poll(arg_1(arguments));
    if (polHasNextReadFile(take_poll(arg_1(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_hasNextReadFile */



#ifdef ANSI_C

objecttype pol_hasNextWriteFile (listtype arguments)
#else

objecttype pol_hasNextWriteFile (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* pol_hasNextWriteFile */
    isit_poll(arg_1(arguments));
    if (polHasNextWriteFile(take_poll(arg_1(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_hasNextWriteFile */



#ifdef ANSI_C

objecttype pol_nextReadFile (listtype arguments)
#else

objecttype pol_nextReadFile (arguments)
listtype arguments;
#endif

  { /* pol_nextReadFile */
    isit_poll(arg_1(arguments));
    return (objecttype) polNextReadFile(take_poll(arg_1(arguments)),
                                        (rtlGenerictype) arg_2(arguments));
  } /* pol_nextReadFile */



#ifdef ANSI_C

objecttype pol_nextWriteFile (listtype arguments)
#else

objecttype pol_nextWriteFile (arguments)
listtype arguments;
#endif

  { /* pol_nextWriteFile */
    isit_poll(arg_1(arguments));
    return (objecttype) polNextWriteFile(take_poll(arg_1(arguments)),
                                         (rtlGenerictype) arg_2(arguments));
  } /* pol_nextWriteFile */



#ifdef ANSI_C

objecttype pol_poll (listtype arguments)
#else

objecttype pol_poll (arguments)
listtype arguments;
#endif

  { /* pol_poll */
    isit_poll(arg_1(arguments));
    polPoll(take_poll(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_poll */



#ifdef ANSI_C

objecttype pol_readyForRead (listtype arguments)
#else

objecttype pol_readyForRead (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* pol_readyForRead */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    if (polReadyForRead(take_poll(arg_1(arguments)),
                        take_socket(arg_2(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_readyForRead */



#ifdef ANSI_C

objecttype pol_readyForWrite (listtype arguments)
#else

objecttype pol_readyForWrite (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* pol_readyForWrite */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    if (polReadyForWrite(take_poll(arg_1(arguments)),
                         take_socket(arg_2(arguments)))) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* pol_readyForWrite */



#ifdef ANSI_C

objecttype pol_removeReadCheck (listtype arguments)
#else

objecttype pol_removeReadCheck (arguments)
listtype arguments;
#endif

  { /* pol_removeReadCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    polRemoveReadCheck(take_poll(arg_1(arguments)),
                       take_socket(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_removeReadCheck */



#ifdef ANSI_C

objecttype pol_removeWriteCheck (listtype arguments)
#else

objecttype pol_removeWriteCheck (arguments)
listtype arguments;
#endif

  { /* pol_removeWriteCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    polRemoveWriteCheck(take_poll(arg_1(arguments)),
                        take_socket(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_removeWriteCheck */



#ifdef ANSI_C

objecttype pol_value (listtype arguments)
#else

objecttype pol_value (arguments)
listtype arguments;
#endif

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
