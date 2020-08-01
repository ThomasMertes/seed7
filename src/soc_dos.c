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



socketType socAccept (socketType sock, bstriType *address)

  { /* socAccept */
    return 0;
  } /* socAccept */



intType socAddrFamily (const const_bstriType address)

  { /* socAddrFamily */
    return 0;
  } /* socAddrFamily */



striType socAddrNumeric (const const_bstriType address)

  { /* socAddrNumeric */
    return NULL;
  } /* socAddrNumeric */



striType socAddrService (const const_bstriType address)

  { /* socAddrService */
    return NULL;
  } /* socAddrService */



void socBind (socketType sock, const_bstriType address)

  { /* socBind */
  } /* socBind */



void socClose (socketType sock)

  { /* socClose */
  } /* socClose */



void socConnect (socketType sock, const_bstriType address)

  { /* socConnect */
  } /* socConnect */



charType socGetc (socketType sock)

  { /* socGetc */
    return (charType) EOF;
  } /* socGetc */



striType socGets (socketType sock, intType length)

  { /* socGets */
    return NULL;
  } /* socGets */



bstriType socGetAddr (socketType sock)

  { /* socGetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socGetAddr */



striType socGetHostname (void)

  { /* socGetHostname */
    return NULL;
  } /* socGetHostname */



bstriType socGetLocalAddr (socketType sock)

  { /* socGetLocalAddr */
    return NULL;
  } /* socGetLocalAddr */



bstriType socGetPeerAddr (socketType sock)

  { /* socGetPeerAddr */
    return NULL;
  } /* socGetPeerAddr */



boolType socHasNext (socketType sock)

  { /* socHasNext */
    return TRUE;
  } /* socHasNext */



bstriType socInetAddr (const const_striType host_name, intType port)

  { /* socInetAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetAddr */



bstriType socInetLocalAddr (intType port)

  { /* socInetLocalAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetLocalAddr */



bstriType socInetServAddr (intType port)

  { /* socInetServAddr */
    raise_error(FILE_ERROR);
    return NULL;
  } /* socInetServAddr */



boolType socInputReady (socketType sock, intType seconds, intType micro_seconds)

  { /* socInputReady */
    return FALSE;
  } /* socInputReady */



striType socLineRead (socketType sock, charType *terminationChar)

  { /* socLineRead */
    return NULL;
  } /* socLineRead */



void socListen (socketType sock, intType backlog)

  { /* socListen */
  } /* socListen */



intType socRecv (socketType sock, striType *stri, intType length, intType flags)

  { /* socRecv */
    return 0;
  } /* socRecv */



intType socRecvfrom (socketType sock, striType *stri, intType length, intType flags,
    bstriType *address)

  { /* socRecvfrom */
    return 0;
  } /* socRecvfrom */



intType socSend (socketType sock, const const_striType stri, intType flags)

  { /* socSend */
    return 0;
  } /* socSend */



intType socSendto (socketType sock, const const_striType stri, intType flags,
    const_bstriType address)

  { /* socSendto */
    return 0;
  } /* socSendto */



void socSetOptBool (socketType sock, intType optname, boolType optval)

  { /* socSetOptBool */
  } /* socSetOptBool */



socketType socSocket (intType domain, intType type, intType protocol)

  { /* socSocket */
    return 0;
  } /* socSocket */



striType socWordRead (socketType sock, charType *terminationChar)

  { /* socWordRead */
    return NULL;
  } /* socWordRead */



void socWrite (socketType sock, const const_striType stri)

  { /* socWrite */
  } /* socWrite */
