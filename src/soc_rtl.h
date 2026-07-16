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

#ifdef DO_INIT
socketRecord emptySocketRecord = {(socketNumberType) -1, 0};
#else
EXTERN socketRecord emptySocketRecord;
#endif

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
socketType socAccept (const const_socketType listenerSocket,
                      bstriType *const address);
intType socAddrFamily (const const_bstriType address);
striType socAddrNumeric (const const_bstriType address);
striType socAddrService (const const_bstriType address);
void socBind (const const_socketType listenerSocket,
              const const_bstriType address);
void socClose (const socketType aSocket);
void socConnect (const const_socketType aSocket,
                 const const_bstriType address);
void socCpy (socketType *const dest, const socketType source);
socketType socCreate (const socketType source);
void socDestr (const socketType oldSocket);
void socFree (const socketType oldSocket);
charType socGetc (const const_socketType inSocket,
                  charType *const eofIndicator);
striType socGets (const const_socketType inSocket, intType length,
                  charType *const eofIndicator);
striType socGetHostname (void);
bstriType socGetLocalAddr (const const_socketType aSocket);
bstriType socGetPeerAddr (const const_socketType aSocket);
boolType socHasNext (const const_socketType inSocket);
bstriType socInetAddr (const const_striType hostName, intType port);
bstriType socInetLocalAddr (intType port);
bstriType socInetServAddr (intType port);
boolType socInputReady (const const_socketType inSocket, intType seconds,
                        intType micro_seconds);
striType socLineRead (const const_socketType inSocket,
                      charType *const terminationChar);
void socListen (const const_socketType listenerSocket,
                intType backlog);
intType socOrd (const const_socketType aSocket);
intType socRecv (const const_socketType inSocket, striType *const stri,
                 intType length, intType flags);
intType socRecvfrom (const const_socketType inSocket,
                     striType *const stri, intType length,
                     intType flags, bstriType *const address);
intType socSend (const const_socketType outSocket,
                 const const_striType stri, intType flags);
intType socSendto (const const_socketType outSocket,
                   const const_striType stri, intType flags,
                   const_bstriType address);
void socSetOptBool (const const_socketType aSocket, intType optname,
                    boolType optval);
socketType socSocket (intType domain, intType type,
                      intType protocol);
striType socWordRead (const const_socketType inSocket,
                      charType *const terminationChar);
void socWrite (const const_socketType outSocket, const const_striType stri);
