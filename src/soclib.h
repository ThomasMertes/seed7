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
/*  File: seed7/src/soclib.h                                        */
/*  Changes: 2007, 2011  Thomas Mertes                              */
/*  Content: All primitive actions for sockets.                     */
/*                                                                  */
/********************************************************************/

objectType soc_accept          (listType arguments);
objectType soc_addr_family     (listType arguments);
objectType soc_addr_numeric    (listType arguments);
objectType soc_addr_service    (listType arguments);
objectType soc_bind            (listType arguments);
objectType soc_close           (listType arguments);
objectType soc_connect         (listType arguments);
objectType soc_cpy             (listType arguments);
objectType soc_create          (listType arguments);
objectType soc_empty           (listType arguments);
objectType soc_eq              (listType arguments);
objectType soc_getc            (listType arguments);
objectType soc_gets            (listType arguments);
objectType soc_get_addr        (listType arguments);
objectType soc_get_hostname    (listType arguments);
objectType soc_has_next        (listType arguments);
objectType soc_inet_addr       (listType arguments);
objectType soc_inet_local_addr (listType arguments);
objectType soc_inet_serv_addr  (listType arguments);
objectType soc_input_ready     (listType arguments);
objectType soc_line_read       (listType arguments);
objectType soc_listen          (listType arguments);
objectType soc_ne              (listType arguments);
objectType soc_recv            (listType arguments);
objectType soc_recvfrom        (listType arguments);
objectType soc_send            (listType arguments);
objectType soc_sendto          (listType arguments);
objectType soc_setOptBool      (listType arguments);
objectType soc_socket          (listType arguments);
objectType soc_word_read       (listType arguments);
objectType soc_write           (listType arguments);
