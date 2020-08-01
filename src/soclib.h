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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/soclib.h                                        */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: All primitive actions for sockets.                     */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype soc_accept          (listtype arguments);
objecttype soc_addr_family     (listtype arguments);
objecttype soc_addr_numeric    (listtype arguments);
objecttype soc_addr_service    (listtype arguments);
objecttype soc_bind            (listtype arguments);
objecttype soc_close           (listtype arguments);
objecttype soc_connect         (listtype arguments);
objecttype soc_cpy             (listtype arguments);
objecttype soc_create          (listtype arguments);
objecttype soc_empty           (listtype arguments);
objecttype soc_eq              (listtype arguments);
objecttype soc_getc            (listtype arguments);
objecttype soc_gets            (listtype arguments);
objecttype soc_get_addr        (listtype arguments);
objecttype soc_get_hostname    (listtype arguments);
objecttype soc_inet_addr       (listtype arguments);
objecttype soc_inet_local_addr (listtype arguments);
objecttype soc_inet_serv_addr  (listtype arguments);
objecttype soc_input_ready     (listtype arguments);
objecttype soc_line_read       (listtype arguments);
objecttype soc_listen          (listtype arguments);
objecttype soc_ne              (listtype arguments);
objecttype soc_recv            (listtype arguments);
objecttype soc_recvfrom        (listtype arguments);
objecttype soc_send            (listtype arguments);
objecttype soc_sendto          (listtype arguments);
objecttype soc_socket          (listtype arguments);
objecttype soc_word_read       (listtype arguments);
objecttype soc_write           (listtype arguments);

#else

objecttype soc_accept ();
objecttype soc_addr_family ();
objecttype soc_addr_numeric ();
objecttype soc_addr_service ();
objecttype soc_bind ();
objecttype soc_close ();
objecttype soc_connect ();
objecttype soc_cpy ();
objecttype soc_create ();
objecttype soc_empty ();
objecttype soc_eq ();
objecttype soc_getc ();
objecttype soc_gets ();
objecttype soc_get_addr ();
objecttype soc_get_hostname ();
objecttype soc_inet_addr ();
objecttype soc_inet_local_addr ();
objecttype soc_inet_serv_addr ();
objecttype soc_input_ready ();
objecttype soc_line_read ();
objecttype soc_listen ();
objecttype soc_ne ();
objecttype soc_recv ();
objecttype soc_recvfrom ();
objecttype soc_send ();
objecttype soc_sendto ();
objecttype soc_socket ();
objecttype soc_word_read ();
objecttype soc_write ();

#endif
