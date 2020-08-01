/********************************************************************/
/*                                                                  */
/*  soc_dos.c     Dummy functions for the socket type.              */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/soc_dos.c                                       */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Dummy functions for the socket type.                   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "rtl_err.h"



#ifdef ANSI_C

sockettype socAccept (sockettype sock, bstritype *address)
#else

sockettype socAccept (sock, address)
sockettype sock;
bstritype *address;
#endif

  { /* socAccept */
    return 0;
  } /* socAccept */



#ifdef ANSI_C

inttype socAddrFamily (const const_bstritype address)
#else

inttype socAddrFamily (address)
bstritype address;
#endif

  { /* socAddrFamily */
    return 0;
  } /* socAddrFamily */



#ifdef ANSI_C

stritype socAddrNumeric (const const_bstritype address)
#else

stritype socAddrNumeric (address)
bstritype address;
#endif

  { /* socAddrNumeric */
    return NULL;
  } /* socAddrNumeric */



#ifdef ANSI_C

stritype socAddrService (const const_bstritype address)
#else

stritype socAddrService (address)
bstritype address;
#endif

  { /* socAddrService */
    return NULL;
  } /* socAddrService */



#ifdef ANSI_C

void socBind (sockettype sock, const_bstritype address)
#else

void socBind (sock, address)
sockettype sock;
bstritype address;
#endif

  { /* socBind */
  } /* socBind */



#ifdef ANSI_C

void socClose (sockettype sock)
#else

void socClose (sock)
sockettype sock;
#endif

  { /* socClose */
  } /* socClose */



#ifdef ANSI_C

void socConnect (sockettype sock, const_bstritype address)
#else

void socConnect (sock, address)
sockettype sock;
bstritype address;
#endif

  { /* socConnect */
  } /* socConnect */



#ifdef ANSI_C

chartype socGetc (sockettype sock)
#else

chartype socGetc (sock)
sockettype sock;
#endif

  { /* socGetc */
    return (chartype) EOF;
  } /* socGetc */



#ifdef ANSI_C

stritype socGets (sockettype sock, inttype length)
#else

stritype socGets (sock, length)
sockettype sock;
inttype length;
#endif

  { /* socGets */
    return NULL;
  } /* socGets */



#ifdef ANSI_C

bstritype socGetAddr (sockettype sock)
#else

bstritype socGetAddr (sock)
sockettype sock;
#endif

  { /* socGetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socGetAddr */



#ifdef ANSI_C

stritype socGetHostname (void)
#else

stritype socGetHostname ()
#endif

  { /* socGetHostname */
    return NULL;
  } /* socGetHostname */



#ifdef ANSI_C

booltype socHasNext (sockettype sock)
#else

booltype socHasNext (sock)
sockettype sock;
#endif

  { /* socHasNext */
    return TRUE;
  } /* socHasNext */



#ifdef ANSI_C

bstritype socInetAddr (const const_stritype host_name, inttype port)
#else

bstritype socInetAddr (host_name, port)
stritype host_name;
inttype port;
#endif

  { /* socInetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetAddr */



#ifdef ANSI_C

bstritype socInetLocalAddr (inttype port)
#else

bstritype socInetLocalAddr (port)
inttype port;
#endif

  { /* socInetLocalAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetLocalAddr */



#ifdef ANSI_C

bstritype socInetServAddr (inttype port)
#else

bstritype socInetServAddr (port)
inttype port;
#endif

  { /* socInetServAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetServAddr */



#ifdef ANSI_C

booltype socInputReady (sockettype sock, inttype seconds, inttype micro_seconds)
#else

booltype socInputReady (sock, seconds, micro_seconds)
sockettype sock;
inttype seconds;
inttype micro_seconds;
#endif

  { /* socInputReady */
    return FALSE;
  } /* socInputReady */



#ifdef ANSI_C

stritype socLineRead (sockettype sock, chartype *termination_char)
#else

stritype socLineRead (sock, termination_char)
sockettype sock;
chartype *termination_char;
#endif

  { /* socLineRead */
    return NULL;
  } /* socLineRead */



#ifdef ANSI_C

void socListen (sockettype sock, inttype backlog)
#else

void socListen (sock, backlog)
sockettype sock;
inttype backlog;
#endif

  { /* socListen */
  } /* socListen */



#ifdef ANSI_C

inttype socRecv (sockettype sock, stritype *stri, inttype length, inttype flags)
#else

inttype socRecv (sock, stri, length, flags)
sockettype sock;
stritype *stri;
inttype length;
inttype flags;
#endif

  { /* socRecv */
    return 0;
  } /* socRecv */



#ifdef ANSI_C

inttype socRecvfrom (sockettype sock, stritype *stri, inttype length, inttype flags,
    bstritype *address)
#else

inttype socRecvfrom (sock, stri, length, flags, address)
sockettype sock;
stritype *stri;
inttype length;
inttype flags;
bstritype *address;
#endif

  { /* socRecvfrom */
    return 0;
  } /* socRecvfrom */



#ifdef ANSI_C

inttype socSend (sockettype sock, const const_stritype stri, inttype flags)
#else

inttype socSend (sock, stri, flags)
sockettype sock;
stritype stri;
inttype flags;
#endif

  { /* socSend */
    return 0;
  } /* socSend */



#ifdef ANSI_C

inttype socSendto (sockettype sock, const const_stritype stri, inttype flags,
    const_bstritype address)
#else

inttype socSendto (sock, stri, flags, address)
sockettype sock;
stritype stri;
inttype flags;
bstritype address;
#endif

  { /* socSendto */
    return 0;
  } /* socSendto */



#ifdef ANSI_C

sockettype socSocket (inttype domain, inttype type, inttype protocol)
#else

sockettype socSocket (domain, type, protocol)
inttype domain;
inttype type;
inttype protocol;
#endif

  { /* socSocket */
    return 0;
  } /* socSocket */



#ifdef ANSI_C

stritype socWordRead (sockettype sock, chartype *termination_char)
#else

stritype socWordRead (sock, termination_char)
sockettype sock;
chartype *termination_char;
#endif

  { /* socWordRead */
    return NULL;
  } /* socWordRead */



#ifdef ANSI_C

void socWrite (sockettype sock, const const_stritype stri)
#else

void socWrite (sock, stri)
sockettype sock;
stritype stri;
#endif

  { /* socWrite */
  } /* socWrite */
