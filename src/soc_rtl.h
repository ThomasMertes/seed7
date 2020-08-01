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

#define SOC_OPT_NONE      0
#define SOC_OPT_REUSEADDR 1


socketType socAccept (socketType sock, bstriType *address);
intType socAddrFamily (const const_bstriType address);
striType socAddrNumeric (const const_bstriType address);
striType socAddrService (const const_bstriType address);
void socBind (socketType sock, const_bstriType address);
void socClose (socketType sock);
void socConnect (socketType sock, const_bstriType address);
charType socGetc (socketType sock, charType *const eofIndicator);
striType socGets (socketType sock, intType length, charType *const eofIndicator);
bstriType socGetAddr (socketType sock);
striType socGetHostname (void);
boolType socHasNext (socketType sock);
bstriType socInetAddr (const const_striType host_name, intType port);
bstriType socInetLocalAddr (intType port);
bstriType socInetServAddr (intType port);
boolType socInputReady (socketType sock, intType seconds, intType micro_seconds);
striType socLineRead (socketType sock, charType *const terminationChar);
void socListen (socketType sock, intType backlog);
intType socRecv (socketType sock, striType *stri, intType length, intType flags);
intType socRecvfrom (socketType sock, striType *stri, intType length, intType flags,
    bstriType *address);
intType socSend (socketType sock, const const_striType stri, intType flags);
intType socSendto (socketType sock, const const_striType stri, intType flags,
    const_bstriType address);
void socSetOptBool (socketType sock, intType optname, boolType optval);
socketType socSocket (intType domain, intType type, intType protocol);
striType socWordRead (socketType sock, charType *const terminationChar);
void socWrite (socketType sock, const const_striType stri);
