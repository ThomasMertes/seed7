/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/soclib.c                                        */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: All primitive actions for sockets.                     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
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
    return(bld_socket_temp(
        socAccept(take_socket(arg_1(arguments)),
                 &take_bstri(arg_2(arguments)))));
  } /* soc_accept */



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
    return(SYS_EMPTY_OBJECT);
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
    return(SYS_EMPTY_OBJECT);
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
    return(SYS_EMPTY_OBJECT);
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
    return(SYS_EMPTY_OBJECT);
  } /* soc_cpy */



#ifdef ANSI_C

objecttype soc_create (listtype arguments)
#else

objecttype soc_create (arguments)
listtype arguments;
#endif

  { /* soc_create */
    isit_socket(arg_3(arguments));
    SET_CLASS_OF_OBJ(arg_1(arguments), SOCKETOBJECT);
    arg_1(arguments)->value.socketvalue = take_socket(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* soc_create */



#ifdef ANSI_C

objecttype soc_empty (listtype arguments)
#else

objecttype soc_empty (arguments)
listtype arguments;
#endif

  { /* soc_empty */
    return(bld_socket_temp((sockettype) -1));
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
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* soc_eq */



#ifdef ANSI_C

objecttype soc_getc (listtype arguments)
#else

objecttype soc_getc (arguments)
listtype arguments;
#endif

  { /* soc_getc */
    isit_socket(arg_1(arguments));
    return(bld_char_temp((chartype)
        socGetc(take_socket(arg_1(arguments)))));
  } /* soc_getc */



#ifdef ANSI_C

objecttype soc_gets (listtype arguments)
#else

objecttype soc_gets (arguments)
listtype arguments;
#endif

  { /* soc_gets */
    isit_socket(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_stri_temp(
        socGets(take_socket(arg_1(arguments)), take_int(arg_2(arguments)))));
  } /* soc_gets */



#ifdef ANSI_C

objecttype soc_inet_addr (listtype arguments)
#else

objecttype soc_inet_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_addr */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_bstri_temp(
	socInetAddr(take_stri(arg_1(arguments)),
                    take_int(arg_2(arguments)))));
  } /* soc_inet_addr */



#ifdef ANSI_C

objecttype soc_inet_local_addr (listtype arguments)
#else

objecttype soc_inet_local_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_local_addr */
    isit_int(arg_1(arguments));
    return(bld_bstri_temp(
	socInetLocalAddr(take_int(arg_1(arguments)))));
  } /* soc_inet_local_addr */



#ifdef ANSI_C

objecttype soc_inet_serv_addr (listtype arguments)
#else

objecttype soc_inet_serv_addr (arguments)
listtype arguments;
#endif

  { /* soc_inet_serv_addr */
    isit_int(arg_1(arguments));
    return(bld_bstri_temp(
	socInetServAddr(take_int(arg_1(arguments)))));
  } /* soc_inet_serv_addr */



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
    return(bld_stri_temp(
        socLineRead(take_socket(arg_1(arguments)), &ch_variable->value.charvalue)));
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
    return(SYS_EMPTY_OBJECT);
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
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
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
    return(bld_int_temp(
        socRecv(take_socket(arg_1(arguments)),
		        &take_stri(arg_2(arguments)),
		        take_int(arg_3(arguments)),
		        take_int(arg_4(arguments)))));
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
    return(bld_int_temp(
        socRecvfrom(take_socket(arg_1(arguments)),
		            &take_stri(arg_2(arguments)),
		            take_int(arg_3(arguments)),
		            take_int(arg_4(arguments)),
                    &take_bstri(arg_5(arguments)))));
  } /* soc_recvfrom */



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
    return(bld_int_temp(
        socSend(take_socket(arg_1(arguments)),
		        take_stri(arg_2(arguments)),
		        take_int(arg_3(arguments)))));
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
    return(bld_int_temp(
        socSendto(take_socket(arg_1(arguments)),
		          take_stri(arg_2(arguments)),
		          take_int(arg_3(arguments)),
                  take_bstri(arg_4(arguments)))));
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
    return(bld_socket_temp(
        socSocket(take_int(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_int(arg_3(arguments)))));
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
    return(bld_stri_temp(
        socWordRead(take_socket(arg_1(arguments)), &ch_variable->value.charvalue)));
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
    return(SYS_EMPTY_OBJECT);
  } /* soc_write */
