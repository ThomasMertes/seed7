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

objecttype soc_accept (listtype);
objecttype soc_bind (listtype);
objecttype soc_close (listtype);
objecttype soc_connect (listtype);
objecttype soc_cpy (listtype);
objecttype soc_create (listtype);
objecttype soc_empty (listtype);
objecttype soc_eq (listtype);
objecttype soc_getc (listtype);
objecttype soc_gets (listtype);
objecttype soc_inet_addr (listtype);
objecttype soc_inet_local_addr (listtype);
objecttype soc_inet_serv_addr (listtype);
objecttype soc_line_read (listtype);
objecttype soc_listen (listtype);
objecttype soc_ne (listtype);
objecttype soc_recv (listtype);
objecttype soc_recvfrom (listtype);
objecttype soc_send (listtype);
objecttype soc_sendto (listtype);
objecttype soc_socket (listtype);
objecttype soc_word_read (listtype);
objecttype soc_write (listtype);

#else

objecttype soc_accept ();
objecttype soc_bind ();
objecttype soc_close ();
objecttype soc_connect ();
objecttype soc_cpy ();
objecttype soc_create ();
objecttype soc_empty ();
objecttype soc_eq ();
objecttype soc_getc ();
objecttype soc_gets ();
objecttype soc_inet_addr ();
objecttype soc_inet_local_addr ();
objecttype soc_inet_serv_addr ();
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
