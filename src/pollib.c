/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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

objecttype pol_addCheck (listtype arguments)
#else

objecttype pol_addCheck (arguments)
listtype arguments;
#endif

  { /* pol_addCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_interface(arg_4(arguments));
    polAddCheck(take_poll(arg_1(arguments)),
                take_socket(arg_2(arguments)),
                take_int(arg_3(arguments)),
                (rtlGenerictype) take_interface(arg_4(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_addCheck */



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

objecttype pol_getCheck (listtype arguments)
#else

objecttype pol_getCheck (arguments)
listtype arguments;
#endif

  {
    inttype result;

  /* pol_getCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetCheck(take_poll(arg_1(arguments)),
                          take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getCheck */



#ifdef ANSI_C

objecttype pol_getFinding (listtype arguments)
#else

objecttype pol_getFinding (arguments)
listtype arguments;
#endif

  {
    inttype result;

  /* pol_getFinding */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    result = polGetFinding(take_poll(arg_1(arguments)),
                           take_socket(arg_2(arguments)));
    return bld_int_temp(result);
  } /* pol_getFinding */



#ifdef ANSI_C

objecttype pol_hasNext (listtype arguments)
#else

objecttype pol_hasNext (arguments)
listtype arguments;
#endif

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



#ifdef ANSI_C

objecttype pol_iterChecks (listtype arguments)
#else

objecttype pol_iterChecks (arguments)
listtype arguments;
#endif

  { /* pol_iterChecks */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterChecks(take_poll(arg_1(arguments)),
                  take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterChecks */



#ifdef ANSI_C

objecttype pol_iterFindings (listtype arguments)
#else

objecttype pol_iterFindings (arguments)
listtype arguments;
#endif

  { /* pol_iterFindings */
    isit_poll(arg_1(arguments));
    isit_int(arg_2(arguments));
    polIterFindings(take_poll(arg_1(arguments)),
                    take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_iterFindings */



#ifdef ANSI_C

objecttype pol_nextFile (listtype arguments)
#else

objecttype pol_nextFile (arguments)
listtype arguments;
#endif

  { /* pol_nextFile */
    isit_poll(arg_1(arguments));
    return (objecttype) polNextFile(take_poll(arg_1(arguments)),
                                    (rtlGenerictype) arg_2(arguments));
  } /* pol_nextFile */



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

objecttype pol_removeCheck (listtype arguments)
#else

objecttype pol_removeCheck (arguments)
listtype arguments;
#endif

  { /* pol_removeCheck */
    isit_poll(arg_1(arguments));
    isit_socket(arg_2(arguments));
    isit_int(arg_3(arguments));
    polRemoveCheck(take_poll(arg_1(arguments)),
                   take_socket(arg_2(arguments)),
                   take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pol_removeCheck */



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
