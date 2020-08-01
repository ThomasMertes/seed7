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
/*  File: seed7/src/soclib.c                                        */
/*  Changes: 2007, 2011  Thomas Mertes                              */
/*  Content: All primitive actions for sockets.                     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "soc_rtl.h"



#ifdef ANSI_C

objecttype soc_accept (listtype arguments)
#else

objecttype soc_accept (arguments)
listtype arguments;
#endif

  { /* soc_accept */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    is_variable(arg_2(arguments));
    return bld_socket_temp(
        socAccept(take_socket(arg_1(arguments)),
                 &take_bstri(arg_2(arguments))));
  } /* soc_accept */



#ifdef ANSI_C

objecttype soc_addr_family (listtype arguments)
#else

objecttype soc_addr_family (arguments)
listtype arguments;
#endif

  { /* soc_addr_family */
    isit_bstri(arg_1(arguments));
    return bld_int_temp(
        socAddrFamily(take_bstri(arg_1(arguments))));
  } /* soc_addr_family */



#ifdef ANSI_C

objecttype soc_addr_numeric (listtype arguments)
#else

objecttype soc_addr_numeric (arguments)
listtype arguments;
#endif

  { /* soc_addr_numeric */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        socAddrNumeric(take_bstri(arg_1(arguments))));
  } /* soc_addr_numeric */



#ifdef ANSI_C

objecttype soc_addr_service (listtype arguments)
#else

objecttype soc_addr_service (arguments)
listtype arguments;
#endif

  { /* soc_addr_service */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        socAddrService(take_bstri(arg_1(arguments))));
  } /* soc_addr_service */



#ifdef ANSI_C

objecttype soc_bind (listtype arguments)
#else

objecttype soc_bind (arguments)
listtype arguments;
#endif

  { /* soc_bind */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    socBind(take_socket(arg_1(arguments)), take_bstri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_bind */



#ifdef ANSI_C

objecttype soc_close (listtype arguments)
#else

objecttype soc_close (arguments)
listtype arguments;
#endif

  { /* soc_close */
    isit_socket(arg_1(arguments));
    socClose(take_socket(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_close */



#ifdef ANSI_C

objecttype soc_connect (listtype arguments)
#else

objecttype soc_connect (arguments)
listtype arguments;
#endif

  { /* soc_connect */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    socConnect(take_socket(arg_1(arguments)), take_bstri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_connect */



#ifdef ANSI_C

objecttype soc_cpy (listtype arguments)
#else

objecttype soc_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype socket_variable;

  /* soc_cpy */
    socket_variable = arg_1(arguments);
    isit_socket(socket_variable);
    is_variable(socket_variable);
    isit_socket(arg_3(arguments));
    socket_variable->value.socketvalue = take_socket(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* soc_cpy */



#ifdef ANSI_C

objecttype soc_create (listtype arguments)
#else

objecttype soc_create (arguments)
listtype arguments;
#endif

  { /* soc_create */
    isit_socket(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), SOCKETOBJECT);
    arg_1(arguments)->value.socketvalue = take_socket(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* soc_create */



#ifdef ANSI_C

objecttype soc_empty (listtype arguments)
#else

objecttype soc_empty (arguments)
listtype arguments;
#endif

  { /* soc_empty */
    return bld_socket_temp((sockettype) -1);
  } /* soc_empty */



#ifdef ANSI_C

objecttype soc_eq (listtype arguments)
#else

objecttype soc_eq (arguments)
listtype arguments;
#endif

  { /* soc_eq */
    isit_socket(arg_1(arguments));
    isit_socket(arg_3(arguments));
    if (take_socket(arg_1(arguments)) ==
        take_socket(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* soc_eq */



#ifdef ANSI_C

objecttype soc_getc (listtype arguments)
#else

objecttype soc_getc (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* soc_getc */
    isit_socket(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_char_temp((chartype)
        socGetc(take_socket(arg_1(arguments)),
                &ch_variable->value.charvalue));
  } /* soc_getc */



#ifdef ANSI_C

objecttype soc_gets (listtype arguments)
#else

objecttype soc_gets (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* soc_gets */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    ch_variable = arg_3(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        socGets(take_socket(arg_1(arguments)),
                take_int(arg_2(arguments)),
                &ch_variable->value.charvalue));
  } /* soc_gets */



#ifdef ANSI_C

objecttype soc_get_addr (listtype arguments)
#else

objecttype soc_get_addr (arguments)
listtype arguments;
#endif

  { /* soc_get_addr */
    isit_socket(arg_1(arguments));
    return bld_bstri_temp(
        socGetAddr(take_socket(arg_1(arguments))));
  } /* soc_get_addr */



#ifdef ANSI_C

objecttype soc_get_hostname (listtype arguments)
#else

objecttype soc_get_hostname (arguments)
listtype arguments;
#endif

  { /* soc_get_hostname */
    return bld_stri_temp(socGetHostname());
  } /* soc_get_hostname */



#ifdef ANSI_C

objecttype soc_has_next (listtype arguments)
#else

objecttype soc_has_next (arguments)
listtype arguments;
#endif

  { /* soc_has_next */
    isit_socket(arg_1(arguments));
    if (socHasNext(take_socket(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* soc_has_next */



#ifdef ANSI_C

objecttype soc_inet_addr (listtype arguments)
#else

objecttype soc_inet_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_addr */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_bstri_temp(
        socInetAddr(take_stri(arg_1(arguments)),
                    take_int(arg_2(arguments))));
  } /* soc_inet_addr */



#ifdef ANSI_C

objecttype soc_inet_local_addr (listtype arguments)
#else

objecttype soc_inet_local_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_local_addr */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        socInetLocalAddr(take_int(arg_1(arguments))));
  } /* soc_inet_local_addr */



#ifdef ANSI_C

objecttype soc_inet_serv_addr (listtype arguments)
#else

objecttype soc_inet_serv_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_serv_addr */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        socInetServAddr(take_int(arg_1(arguments))));
  } /* soc_inet_serv_addr */



#ifdef ANSI_C

objecttype soc_input_ready (listtype arguments)
#else

objecttype soc_input_ready (arguments)
listtype arguments;
#endif

  { /* soc_input_ready */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    if (socInputReady(take_socket(arg_1(arguments)),
                      take_int(arg_2(arguments)),
                      take_int(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* soc_input_ready */



#ifdef ANSI_C

objecttype soc_line_read (listtype arguments)
#else

objecttype soc_line_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* soc_line_read */
    isit_socket(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        socLineRead(take_socket(arg_1(arguments)), &ch_variable->value.charvalue));
  } /* soc_line_read */



#ifdef ANSI_C

objecttype soc_listen (listtype arguments)
#else

objecttype soc_listen (arguments)
listtype arguments;
#endif

  { /* soc_listen */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    socListen(take_socket(arg_1(arguments)), take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_listen */



#ifdef ANSI_C

objecttype soc_ne (listtype arguments)
#else

objecttype soc_ne (arguments)
listtype arguments;
#endif

  { /* soc_ne */
    isit_socket(arg_1(arguments));
    isit_socket(arg_3(arguments));
    if (take_socket(arg_1(arguments)) !=
        take_socket(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* soc_ne */



#ifdef ANSI_C

objecttype soc_recv (listtype arguments)
#else

objecttype soc_recv (arguments)
listtype arguments;
#endif

  { /* soc_recv */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    is_variable(arg_2(arguments));
    return bld_int_temp(
        socRecv(take_socket(arg_1(arguments)),
               &take_stri(arg_2(arguments)),
                take_int(arg_3(arguments)),
                take_int(arg_4(arguments))));
  } /* soc_recv */



#ifdef ANSI_C

objecttype soc_recvfrom (listtype arguments)
#else

objecttype soc_recvfrom (arguments)
listtype arguments;
#endif

  { /* soc_recvfrom */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_bstri(arg_5(arguments));
    is_variable(arg_2(arguments));
    is_variable(arg_5(arguments));
    return bld_int_temp(
        socRecvfrom(take_socket(arg_1(arguments)),
                   &take_stri(arg_2(arguments)),
                    take_int(arg_3(arguments)),
                    take_int(arg_4(arguments)),
                   &take_bstri(arg_5(arguments))));
  } /* soc_recvfrom */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype soc_select (listtype arguments)
#else

objecttype soc_select (arguments)
listtype arguments;
#endif

  {
    arraytype sockArray;
    memsizetype array_size;
    memsizetype pos;
    int nfds = 0;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    int fd;
    arraytype result_array;
    arraytype result;

  /* soc_select */
    isit_array(arg_1(arguments));
    sockArray = take_array(arg_1(arguments));
    FD_ZERO(&readfds);
    if (sockArray->max_position >= sockArray->min_position) {
      array_size = (uinttype) (sockArray->max_position - sockArray->min_position) + 1;
      for (pos = 0; pos < array_size; pos++) {
        FD_SET(sockArray->arr[pos].value.socketvalue, &readfds);
        if (sockArray->arr[pos].value.socketvalue >= nfds) {
          nfds = sockArray->arr[pos].value.socketvalue + 1;
        } /* if */
      } /* for */
    } /* if */
    select_result = select(nfds, &readfds, NULL, NULL, &timeout);
    if (unlikely(select_result < 0)) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_ARRAY(result_array, select_result))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result_array->min_position = 1;
        result_array->max_position = select_result;
        pos = 0;
        for (fd = 0; pos < nfds; fd++) {
          if (FD_ISSET(fd, &readfds)) {
            result_array->arr[pos].value.socketvalue = fd;
            pos++;
          } /* if */
        } /* for */
        result = bld_array_temp(result_array);
      } /* if */
    } /* if */
    return result;
  } /* soc_select */
#endif



#ifdef ANSI_C

objecttype soc_send (listtype arguments)
#else

objecttype soc_send (arguments)
listtype arguments;
#endif

  { /* soc_send */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        socSend(take_socket(arg_1(arguments)),
                take_stri(arg_2(arguments)),
                take_int(arg_3(arguments))));
  } /* soc_send */



#ifdef ANSI_C

objecttype soc_sendto (listtype arguments)
#else

objecttype soc_sendto (arguments)
listtype arguments;
#endif

  { /* soc_sendto */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_bstri(arg_4(arguments));
    return bld_int_temp(
        socSendto(take_socket(arg_1(arguments)),
                  take_stri(arg_2(arguments)),
                  take_int(arg_3(arguments)),
                  take_bstri(arg_4(arguments))));
  } /* soc_sendto */



#ifdef ANSI_C

objecttype soc_socket (listtype arguments)
#else

objecttype soc_socket (arguments)
listtype arguments;
#endif

  { /* soc_socket */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_socket_temp(
        socSocket(take_int(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_int(arg_3(arguments))));
  } /* soc_socket */



#ifdef ANSI_C

objecttype soc_word_read (listtype arguments)
#else

objecttype soc_word_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* soc_word_read */
    isit_socket(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        socWordRead(take_socket(arg_1(arguments)), &ch_variable->value.charvalue));
  } /* soc_word_read */



#ifdef ANSI_C

objecttype soc_write (listtype arguments)
#else

objecttype soc_write (arguments)
listtype arguments;
#endif

  { /* soc_write */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    socWrite(take_socket(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_write */
