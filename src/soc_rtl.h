/********************************************************************/
/*                                                                  */
/*  soc_rtl.h     Primitive actions for the socket type.            */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/soc_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for the socket type.                 */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

sockettype socAccept (sockettype sock, bstritype *address);
inttype socAddrFamily (const const_bstritype address);
stritype socAddrNumeric (const const_bstritype address);
stritype socAddrService (const const_bstritype address);
void socBind (sockettype sock, const_bstritype address);
void socClose (sockettype sock);
void socConnect (sockettype sock, const_bstritype address);
chartype socGetc (sockettype sock, chartype *const eof_indicator);
stritype socGets (sockettype sock, inttype length, chartype *const eof_indicator);
bstritype socGetAddr (sockettype sock);
stritype socGetHostname (void);
booltype socHasNext (sockettype sock);
bstritype socInetAddr (const const_stritype host_name, inttype port);
bstritype socInetLocalAddr (inttype port);
bstritype socInetServAddr (inttype port);
booltype socInputReady (sockettype sock, inttype seconds, inttype micro_seconds);
stritype socLineRead (sockettype sock, chartype *const termination_char);
void socListen (sockettype sock, inttype backlog);
inttype socRecv (sockettype sock, stritype *stri, inttype length, inttype flags);
inttype socRecvfrom (sockettype sock, stritype *stri, inttype length, inttype flags,
    bstritype *address);
inttype socSend (sockettype sock, const const_stritype stri, inttype flags);
inttype socSendto (sockettype sock, const const_stritype stri, inttype flags,
    const_bstritype address);
sockettype socSocket (inttype domain, inttype type, inttype protocol);
stritype socWordRead (sockettype sock, chartype *const termination_char);
void socWrite (sockettype sock, const const_stritype stri);

#else

sockettype socAccept ();
inttype socAddrFamily ();
stritype socAddrNumeric ();
stritype socAddrService ();
void socBind ();
void socClose ();
void socConnect ();
chartype socGetc ();
stritype socGets ();
bstritype socGetAddr ();
stritype socGetHostname ();
booltype socHasNext ();
bstritype socInetAddr ();
bstritype socInetLocalAddr ();
bstritype socInetServAddr ();
booltype socInputReady ();
stritype socLineRead ();
void socListen ();
inttype socRecv ();
inttype socRecvfrom ();
inttype socSend ();
inttype socSendto ();
sockettype socSocket ();
stritype socWordRead ();
void socWrite ();

#endif
