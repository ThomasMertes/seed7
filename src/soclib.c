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



/**
 *  Create a new accepted connection socket for a listener socket.
 *  The function waits until at least one connection request is
 *  in the sockets queue of pending connections. Then it extracts
 *  the first connection request from the sockets queue. This
 *  request is accepted and a connection socket is created for it.
 *  @return the accepted connection socket.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType soc_accept (listType arguments)

  { /* soc_accept */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    is_variable(arg_2(arguments));
    return bld_socket_temp(
        socAccept(take_socket(arg_1(arguments)),
                 &take_bstri(arg_2(arguments))));
  } /* soc_accept */



objectType soc_addr_family (listType arguments)

  { /* soc_addr_family */
    isit_bstri(arg_1(arguments));
    return bld_int_temp(
        socAddrFamily(take_bstri(arg_1(arguments))));
  } /* soc_addr_family */



/**
 *  Get the numeric (IP) address of the host at a socket address.
 *  IPv4 addresses return the socketAddress in dot notation (e.g.:
 *  "192.0.2.235") and IPv6 addresses return the socketAddress in
 *  colon notation (e.g.: "fe80:0:0:0:202:b3ff:fe1e:8329").
 *  @return the IP address of the specified host.
 */
objectType soc_addr_numeric (listType arguments)

  { /* soc_addr_numeric */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        socAddrNumeric(take_bstri(arg_1(arguments))));
  } /* soc_addr_numeric */



objectType soc_addr_service (listType arguments)

  { /* soc_addr_service */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        socAddrService(take_bstri(arg_1(arguments))));
  } /* soc_addr_service */



/**
 *  Assign an internet listener socket address to a listener socket.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType soc_bind (listType arguments)

  { /* soc_bind */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    socBind(take_socket(arg_1(arguments)), take_bstri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_bind */



/**
 *  Close a socket.
 */
objectType soc_close (listType arguments)

  { /* soc_close */
    isit_socket(arg_1(arguments));
    socClose(take_socket(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_close */



/**
 *  Connect a socket to an address.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType soc_connect (listType arguments)

  { /* soc_connect */
    isit_socket(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    socConnect(take_socket(arg_1(arguments)), take_bstri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_connect */



objectType soc_cpy (listType arguments)

  {
    objectType socket_variable;

  /* soc_cpy */
    socket_variable = arg_1(arguments);
    isit_socket(socket_variable);
    is_variable(socket_variable);
    isit_socket(arg_3(arguments));
    socket_variable->value.socketValue = take_socket(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* soc_cpy */



objectType soc_create (listType arguments)

  { /* soc_create */
    isit_socket(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), SOCKETOBJECT);
    arg_1(arguments)->value.socketValue = take_socket(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* soc_create */



objectType soc_empty (listType arguments)

  { /* soc_empty */
    return bld_socket_temp((socketType) -1);
  } /* soc_empty */



/**
 *  Check if two sockets are equal.
 *  @return TRUE if the two sockets are equal,
 *          FALSE otherwise.
 */
objectType soc_eq (listType arguments)

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



/**
 *  Read a character from a socket.
 *  @return the character read.
 */
objectType soc_getc (listType arguments)

  {
    objectType eofIndicator;

  /* soc_getc */
    isit_socket(arg_1(arguments));
    eofIndicator = arg_2(arguments);
    isit_char(eofIndicator);
    is_variable(eofIndicator);
    return bld_char_temp((charType)
        socGetc(take_socket(arg_1(arguments)),
                &eofIndicator->value.charValue));
  } /* soc_getc */



/**
 *  Read a string with a maximum length from a socket.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType soc_gets (listType arguments)

  {
    objectType eofIndicator;

  /* soc_gets */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    eofIndicator = arg_3(arguments);
    isit_char(eofIndicator);
    is_variable(eofIndicator);
    return bld_stri_temp(
        socGets(take_socket(arg_1(arguments)),
                take_int(arg_2(arguments)),
                &eofIndicator->value.charValue));
  } /* soc_gets */



objectType soc_get_addr (listType arguments)

  { /* soc_get_addr */
    isit_socket(arg_1(arguments));
    return bld_bstri_temp(
        socGetAddr(take_socket(arg_1(arguments))));
  } /* soc_get_addr */



objectType soc_get_hostname (listType arguments)

  { /* soc_get_hostname */
    return bld_stri_temp(socGetHostname());
  } /* soc_get_hostname */



/**
 *  Determine if at least one character can be read successfully.
 *  This function allows a socket to be handled like an iterator.
 *  Since socHasNext peeks the next character from the socket
 *  it may block.
 *  @return FALSE if socGetc would return EOF, TRUE otherwise.
 */
objectType soc_has_next (listType arguments)

  { /* soc_has_next */
    isit_socket(arg_1(arguments));
    if (socHasNext(take_socket(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* soc_has_next */



objectType soc_inet_addr (listType arguments)

  { /* soc_inet_addr */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_bstri_temp(
        socInetAddr(take_stri(arg_1(arguments)),
                    take_int(arg_2(arguments))));
  } /* soc_inet_addr */



objectType soc_inet_local_addr (listType arguments)

  { /* soc_inet_local_addr */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        socInetLocalAddr(take_int(arg_1(arguments))));
  } /* soc_inet_local_addr */



objectType soc_inet_serv_addr (listType arguments)

  { /* soc_inet_serv_addr */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        socInetServAddr(take_int(arg_1(arguments))));
  } /* soc_inet_serv_addr */



objectType soc_input_ready (listType arguments)

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



/**
 *  Read a line from a socket.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains '\n' or EOF.
 *  @return the line read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType soc_line_read (listType arguments)

  {
    objectType terminationChar;

  /* soc_line_read */
    isit_socket(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        socLineRead(take_socket(arg_1(arguments)),
                    &terminationChar->value.charValue));
  } /* soc_line_read */



/**
 *  Listen for socket connections and limit the incoming queue.
 *  The function also sets the maximum length to which the queue
 *  of pending connections for a listener socket may grow.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType soc_listen (listType arguments)

  { /* soc_listen */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    socListen(take_socket(arg_1(arguments)), take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_listen */



/**
 *  Check if two sockets are not equal.
 *  @return FALSE if both sockets are equal,
 *          TRUE otherwise.
 */
objectType soc_ne (listType arguments)

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



objectType soc_recv (listType arguments)

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



objectType soc_recvfrom (listType arguments)

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
objectType soc_select (listType arguments)

  {
    arrayType sockArray;
    memSizeType array_size;
    memSizeType pos;
    int nfds = 0;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    int fd;
    arrayType result_array;
    arrayType result;

  /* soc_select */
    isit_array(arg_1(arguments));
    sockArray = take_array(arg_1(arguments));
    FD_ZERO(&readfds);
    if (sockArray->max_position >= sockArray->min_position) {
      array_size = (uintType) (sockArray->max_position - sockArray->min_position) + 1;
      for (pos = 0; pos < array_size; pos++) {
        FD_SET(sockArray->arr[pos].value.socketValue, &readfds);
        if (sockArray->arr[pos].value.socketValue >= nfds) {
          nfds = sockArray->arr[pos].value.socketValue + 1;
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
            result_array->arr[pos].value.socketValue = fd;
            pos++;
          } /* if */
        } /* for */
        result = bld_array_temp(result_array);
      } /* if */
    } /* if */
    return result;
  } /* soc_select */
#endif



objectType soc_send (listType arguments)

  { /* soc_send */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        socSend(take_socket(arg_1(arguments)),
                take_stri(arg_2(arguments)),
                take_int(arg_3(arguments))));
  } /* soc_send */



objectType soc_sendto (listType arguments)

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



objectType soc_setOptBool (listType arguments)

  { /* soc_setOptBool */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bool(arg_3(arguments));
    socSetOptBool(take_socket(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT);
    return SYS_EMPTY_OBJECT;
  } /* soc_setOptBool */



objectType soc_socket (listType arguments)

  { /* soc_socket */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_socket_temp(
        socSocket(take_int(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_int(arg_3(arguments))));
  } /* soc_socket */



/**
 *  Read a word from a socket.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains ' ', '\t', '\n' or EOF.
 *  @return the word read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType soc_word_read (listType arguments)

  {
    objectType terminationChar;

  /* soc_word_read */
    isit_socket(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        socWordRead(take_socket(arg_1(arguments)),
                    &terminationChar->value.charValue));
  } /* soc_word_read */



/**
 *  Write a string to a socket.
 *  @exception FILE_ERROR The system function is not able to write
 *             all characters of the string.
 *  @exception RANGE_ERROR The string contains a character that does
 *             not fit into a byte.
 */
objectType soc_write (listType arguments)

  { /* soc_write */
    isit_socket(arg_1(arguments));
    isit_stri(arg_2(arguments));
    socWrite(take_socket(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* soc_write */
