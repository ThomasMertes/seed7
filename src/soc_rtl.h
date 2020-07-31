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

#if SOCKET_LIB == UNIX_SOCKETS
#define ERROR_INFORMATION "errno", errno, strerror(errno)
#elif SOCKET_LIB == WINSOCK_SOCKETS
#define ERROR_INFORMATION "wsa_error_code", WSAGetLastError(), wsaErrorMessage()
#endif


#if SOCKET_LIB == WINSOCK_SOCKETS
const_cstriType wsaErrorMessage (void);
#endif
socketType socAccept (socketType listenerSocket, bstriType *address);
intType socAddrFamily (const const_bstriType address);
striType socAddrNumeric (const const_bstriType address);
striType socAddrService (const const_bstriType address);
void socBind (socketType listenerSocket, const_bstriType address);
void socClose (socketType aSocket);
void socConnect (socketType aSocket, const_bstriType address);
charType socGetc (socketType inSocket, charType *const eofIndicator);
striType socGets (socketType inSocket, intType length, charType *const eofIndicator);
striType socGetHostname (void);
bstriType socGetLocalAddr (socketType sock);
bstriType socGetPeerAddr (socketType sock);
boolType socHasNext (socketType inSocket);
bstriType socInetAddr (const const_striType host_name, intType port);
bstriType socInetLocalAddr (intType port);
bstriType socInetServAddr (intType port);
boolType socInputReady (socketType sock, intType seconds, intType micro_seconds);
striType socLineRead (socketType inSocket, charType *const terminationChar);
void socListen (socketType listenerSocket, intType backlog);
intType socRecv (socketType sock, striType *stri, intType length, intType flags);
intType socRecvfrom (socketType sock, striType *stri, intType length, intType flags,
    bstriType *address);
intType socSend (socketType sock, const const_striType stri, intType flags);
intType socSendto (socketType sock, const const_striType stri, intType flags,
    const_bstriType address);
void socSetOptBool (socketType sock, intType optname, boolType optval);
socketType socSocket (intType domain, intType type, intType protocol);
striType socWordRead (socketType inSocket, charType *const terminationChar);
void socWrite (socketType outSocket, const const_striType stri);
