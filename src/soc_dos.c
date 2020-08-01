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



sockettype socAccept (sockettype sock, bstritype *address)

  { /* socAccept */
    return 0;
  } /* socAccept */



inttype socAddrFamily (const const_bstritype address)

  { /* socAddrFamily */
    return 0;
  } /* socAddrFamily */



stritype socAddrNumeric (const const_bstritype address)

  { /* socAddrNumeric */
    return NULL;
  } /* socAddrNumeric */



stritype socAddrService (const const_bstritype address)

  { /* socAddrService */
    return NULL;
  } /* socAddrService */



void socBind (sockettype sock, const_bstritype address)

  { /* socBind */
  } /* socBind */



void socClose (sockettype sock)

  { /* socClose */
  } /* socClose */



void socConnect (sockettype sock, const_bstritype address)

  { /* socConnect */
  } /* socConnect */



chartype socGetc (sockettype sock)

  { /* socGetc */
    return (chartype) EOF;
  } /* socGetc */



stritype socGets (sockettype sock, inttype length)

  { /* socGets */
    return NULL;
  } /* socGets */



bstritype socGetAddr (sockettype sock)

  { /* socGetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socGetAddr */



stritype socGetHostname (void)

  { /* socGetHostname */
    return NULL;
  } /* socGetHostname */



booltype socHasNext (sockettype sock)

  { /* socHasNext */
    return TRUE;
  } /* socHasNext */



bstritype socInetAddr (const const_stritype host_name, inttype port)

  { /* socInetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetAddr */



bstritype socInetLocalAddr (inttype port)

  { /* socInetLocalAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetLocalAddr */



bstritype socInetServAddr (inttype port)

  { /* socInetServAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetServAddr */



booltype socInputReady (sockettype sock, inttype seconds, inttype micro_seconds)

  { /* socInputReady */
    return FALSE;
  } /* socInputReady */



stritype socLineRead (sockettype sock, chartype *terminationChar)

  { /* socLineRead */
    return NULL;
  } /* socLineRead */



void socListen (sockettype sock, inttype backlog)

  { /* socListen */
  } /* socListen */



inttype socRecv (sockettype sock, stritype *stri, inttype length, inttype flags)

  { /* socRecv */
    return 0;
  } /* socRecv */



inttype socRecvfrom (sockettype sock, stritype *stri, inttype length, inttype flags,
    bstritype *address)

  { /* socRecvfrom */
    return 0;
  } /* socRecvfrom */



inttype socSend (sockettype sock, const const_stritype stri, inttype flags)

  { /* socSend */
    return 0;
  } /* socSend */



inttype socSendto (sockettype sock, const const_stritype stri, inttype flags,
    const_bstritype address)

  { /* socSendto */
    return 0;
  } /* socSendto */



sockettype socSocket (inttype domain, inttype type, inttype protocol)

  { /* socSocket */
    return 0;
  } /* socSocket */



stritype socWordRead (sockettype sock, chartype *terminationChar)

  { /* socWordRead */
    return NULL;
  } /* socWordRead */



void socWrite (sockettype sock, const const_stritype stri)

  { /* socWrite */
  } /* socWrite */
