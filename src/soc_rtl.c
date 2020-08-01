/********************************************************************/
/*                                                                  */
/*  soc_rtl.c     Primitive actions for the socket type.            */
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
/*  File: seed7/src/soc_rtl.c                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for the socket type.                 */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_WINSOCK
#include "winsock2.h"
#else
#include "netdb.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#endif
/* #include "errno.h" */

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"


#ifdef USE_WINSOCK
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#endif

#define MAX_ADDRESS_SIZE 1024



#ifdef ANSI_C

sockettype socAccept (sockettype sock, bstritype *address)
#else

sockettype socAccept (sock)
sockettype sock;
bstritype *address;
#endif

  {
    memsizetype address_size;
    sockettype result;

  /* socAccept */
    if (!RESIZE_BSTRI(*address, (*address)->size, MAX_ADDRESS_SIZE)) {
      raise_error(MEMORY_ERROR);
      result = -1;
    } else {
      COUNT3_BSTRI((*address)->size, MAX_ADDRESS_SIZE);
      address_size = MAX_ADDRESS_SIZE;
      result = accept(sock, (struct sockaddr *) (*address)->mem, &address_size);
      if (result == -1) {
        RESIZE_BSTRI(*address, MAX_ADDRESS_SIZE, (*address)->size);
        COUNT3_BSTRI(MAX_ADDRESS_SIZE, (*address)->size);
        /* printf("socAccept errno=%d\n", errno); */
        raise_error(RANGE_ERROR);
      } else if (!RESIZE_BSTRI(*address, MAX_ADDRESS_SIZE, address_size)) {
        raise_error(MEMORY_ERROR);
        result = -1;
      } else {
        COUNT3_BSTRI(MAX_ADDRESS_SIZE, address_size);
        (*address)->size = address_size;
      } /* if */
    } /* if */
    return(result);
  } /* socAccept */



#ifdef ANSI_C

void socBind (sockettype sock, const_bstritype address)
#else

void socBind (sock)
sockettype sock;
bstritype address;
#endif

  { /* socBind */
    if (bind(sock, (const struct sockaddr *) address->mem,
        address->size) != 0) {
      /* printf("socBind errno=%d\n", errno); */
      raise_error(RANGE_ERROR);
    } /* if */
  } /* socBind */



#ifdef ANSI_C

void socClose (sockettype sock)
#else

void socClose (sock)
sockettype sock;
#endif

  { /* socGetc */
    shutdown(sock, SHUT_RDWR);
  } /* socGetc */



#ifdef ANSI_C

void socConnect (sockettype sock, const_bstritype address)
#else

void socConnect (sock)
sockettype sock;
bstritype address;
#endif

  { /* socConnect */
    if (connect(sock, (const struct sockaddr *) address->mem,
        address->size) != 0) {
      /* printf("socConnect errno=%d\n", errno); */
      raise_error(RANGE_ERROR);
    } /* if */
  } /* socConnect */



#ifdef ANSI_C

chartype socGetc (sockettype sock)
#else

chartype socGetc (sock)
sockettype sock;
#endif

  {
    char ch;
    memsizetype bytes_received;

  /* socGetc */
    bytes_received = (memsizetype) recv(sock, &ch, 1, 0);
    if (bytes_received != 1) {
      return((chartype) EOF);
    } else {
      return((chartype) ch);
    } /* if */
  } /* socGetc */



#ifdef ANSI_C

stritype socGets (sockettype sock, inttype length)
#else

stritype socGets (sock, length)
sockettype sock;
inttype length;
#endif

  {
    memsizetype bytes_requested;
    memsizetype result_size;
    stritype result;

  /* socGets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
      bytes_requested = (memsizetype) length;
      if (!ALLOC_STRI(result, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT_STRI(bytes_requested);
      result_size = (memsizetype) recv(sock, result->mem,
          (SIZE_TYPE) bytes_requested, 0);
#ifdef WIDE_CHAR_STRINGS
      if (result_size > 0) {
        uchartype *from = &((uchartype *) result->mem)[result_size - 1];
        strelemtype *to = &result->mem[result_size - 1];
        memsizetype number = result_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
#endif
      result->size = result_size;
      if (result_size < bytes_requested) {
        if (!RESIZE_STRI(result, bytes_requested, result_size)) {
          raise_error(MEMORY_ERROR);
          return(NULL);
        } /* if */
        COUNT3_STRI(bytes_requested, result_size);
      } /* if */
    } /* if */
    return(result);
  } /* socGets */



#ifdef ANSI_C

bstritype socInetAddr (stritype host_name, inttype port)
#else

bstritype socInetAddr (host_name, port)
stritype host_name;
inttype port;
#endif

  {
    cstritype name;
    struct hostent *host_ent;
    struct sockaddr_in *inet_address;
    bstritype result;

  /* socInetAddr */
    if (port < 0 || port > 65535) {
      result = NULL;
      raise_error(RANGE_ERROR);
    } else {
      name = cp_to_cstri(host_name);
      if (name == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        host_ent = gethostbyname(name);
        if (host_ent == NULL) {
          raise_error(IO_ERROR);
        } else {
          /*
          printf("Host name:      %s\n", host_ent->h_name);
          printf("Address type:   %d\n", host_ent->h_addrtype);
          printf("Address type:   %d\n", AF_INET);
          printf("Address length: %d\n", host_ent->h_length);
          printf("Address length: %d\n", sizeof(struct sockaddr_in));
          printf("IP Address:     %s\n", inet_ntoa(*((struct in_addr *)host_ent->h_addr)));
          */
          if (host_ent->h_addrtype != AF_INET ||
              host_ent->h_length != sizeof(inet_address->sin_addr.s_addr)) {
            raise_error(IO_ERROR);
          } else {
            if (!ALLOC_BSTRI(result, sizeof(struct sockaddr_in))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = sizeof(struct sockaddr_in);
              inet_address = (struct sockaddr_in *) result->mem;
              inet_address->sin_family = host_ent->h_addrtype;
              inet_address->sin_port = htons(port);                    /* short, network byte order */
              memcpy(&inet_address->sin_addr.s_addr, host_ent->h_addr, host_ent->h_length);
              memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#if 0
              struct hostent {
                      char    *h_name;        /* official name of host */
                      char    **h_aliases;    /* alias list */
                      int     h_addrtype;     /* host address type */
                      int     h_length;       /* length of address */
                      char    **h_addr_list;  /* list of addresses */
              }
#endif
    return(result);
  } /* socInetAddr */



#ifdef ANSI_C

bstritype socInetLocalAddr (inttype port)
#else

bstritype socInetLocalAddr (port)
inttype port;
#endif

  {
    struct sockaddr_in *inet_address;
    bstritype result;

  /* socInetLocalAddr */
    if (port < 0 || port > 65535) {
      result = NULL;
      raise_error(RANGE_ERROR);
    } else if (!ALLOC_BSTRI(result, sizeof(struct sockaddr_in))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = sizeof(struct sockaddr_in);
      inet_address = (struct sockaddr_in *) result->mem;
      inet_address->sin_family = AF_INET;
      inet_address->sin_port = htons(port);                   /* short, network byte order */
      inet_address->sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* local host */
      memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
    } /* if */
    return(result);
  } /* socInetLocalAddr */



#ifdef ANSI_C

bstritype socInetServAddr (inttype port)
#else

bstritype socInetServAddr (port)
inttype port;
#endif

  {
    struct sockaddr_in *inet_address;
    bstritype result;

  /* socInetServAddr */
    if (port < 0 || port > 65535) {
      result = NULL;
      raise_error(RANGE_ERROR);
    } else if (!ALLOC_BSTRI(result, sizeof(struct sockaddr_in))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = sizeof(struct sockaddr_in);
      inet_address = (struct sockaddr_in *) result->mem;
      inet_address->sin_family = AF_INET;
      inet_address->sin_port = htons(port);       /* short, network byte order */
      inet_address->sin_addr.s_addr = INADDR_ANY; /* auto-fill with local IP */
      memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
    } /* if */
    return(result);
  } /* socInetServAddr */



#ifdef ANSI_C

stritype socLineRead (sockettype sock, chartype *termination_char)
#else

stritype socLineRead (sock, termination_char)
sockettype sock;
chartype *termination_char;
#endif

  {
    char ch;
    register memsizetype position;
    register memsizetype bytes_received;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype result;

  /* socLineRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_STRI(memlength);
      memory = result->mem;
      position = 0;
      bytes_received = (memsizetype) recv(sock, &ch, 1, 0);
      while (bytes_received == 1 && ch != '\n') {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          if (!RESIZE_STRI(result, memlength, newmemlength)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        bytes_received = (memsizetype) recv(sock, &ch, 1, 0);
      } /* while */
      if (bytes_received == 1 && ch == '\n' &&
          position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!RESIZE_STRI(result, memlength, position)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT3_STRI(memlength, position);
      result->size = position;
      if (bytes_received != 1) {
        *termination_char = (chartype) EOF;
      } else {
        *termination_char = (chartype) ch;
      } /* if */
      return(result);
    } /* if */
  } /* socLineRead */



#ifdef ANSI_C

void socListen (sockettype sock, inttype backlog)
#else

void socListen (sock, backlog)
sockettype sock;
inttype backlog;
#endif

  { /* socListen */
    listen(sock, backlog);
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

  {
    memsizetype bytes_requested;
    memsizetype stri_size;

  /* socRecv */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      bytes_requested = (memsizetype) length;
      if (!RESIZE_STRI(*stri, (*stri)->size, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return(0);
      } /* if */
      COUNT3_STRI((*stri)->size, bytes_requested);
      stri_size = (memsizetype) recv(sock, (*stri)->mem,
          (SIZE_TYPE) bytes_requested, flags);
#ifdef WIDE_CHAR_STRINGS
      if (stri_size > 0) {
        uchartype *from = &((uchartype *) (*stri)->mem)[stri_size - 1];
        strelemtype *to = &(*stri)->mem[stri_size - 1];
        memsizetype number = stri_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
#endif
      (*stri)->size = stri_size;
      if (stri_size < bytes_requested) {
        if (!RESIZE_STRI(*stri, bytes_requested, stri_size)) {
          raise_error(MEMORY_ERROR);
          return(0);
        } /* if */
        COUNT3_STRI(bytes_requested, stri_size);
      } /* if */
    } /* if */
    return((inttype) stri_size);
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

  {
    memsizetype bytes_requested;
    memsizetype address_size;
    memsizetype stri_size;

  /* socRecvfrom */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      bytes_requested = (memsizetype) length;
      if (!RESIZE_STRI(*stri, (*stri)->size, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return(0);
      } /* if */
      COUNT3_STRI((*stri)->size, bytes_requested);
      if (!RESIZE_BSTRI(*address, (*address)->size, MAX_ADDRESS_SIZE)) {
        if (RESIZE_STRI(*stri, bytes_requested, (*stri)->size)) {
          COUNT3_STRI(bytes_requested, (*stri)->size);
        } /* if */
        raise_error(MEMORY_ERROR);
        return(0);
      } else {
        COUNT3_BSTRI((*address)->size, MAX_ADDRESS_SIZE);
        address_size = MAX_ADDRESS_SIZE;
        stri_size = (memsizetype) recvfrom(sock, (*stri)->mem,
            (SIZE_TYPE) bytes_requested, flags,
			(struct sockaddr *) (*address)->mem, &address_size);
        if (stri_size == -1) {
          RESIZE_BSTRI(*address, MAX_ADDRESS_SIZE, (*address)->size);
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (*address)->size);
          /* printf("socRecvfrom errno=%d\n", errno); */
          raise_error(RANGE_ERROR);
        } else if (!RESIZE_BSTRI(*address, MAX_ADDRESS_SIZE, address_size)) {
          raise_error(MEMORY_ERROR);
          return(0);
        } else {
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, address_size);
          (*address)->size = address_size;
	    } /* if */
      } /* if */
#ifdef WIDE_CHAR_STRINGS
      if (stri_size > 0) {
        uchartype *from = &((uchartype *) (*stri)->mem)[stri_size - 1];
        strelemtype *to = &(*stri)->mem[stri_size - 1];
        memsizetype number = stri_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
#endif
      (*stri)->size = stri_size;
      if (stri_size < bytes_requested) {
        if (!RESIZE_STRI(*stri, bytes_requested, stri_size)) {
          raise_error(MEMORY_ERROR);
          return(0);
        } /* if */
        COUNT3_STRI(bytes_requested, stri_size);
      } /* if */
    } /* if */
    return((inttype) stri_size);
  } /* socRecvfrom */



#ifdef ANSI_C

inttype socSend (sockettype sock, stritype stri, inttype flags)
#else

inttype socSend (sock, stri, flags)
sockettype sock;
stritype stri;
inttype flags;
#endif

  {
    bstritype buf;
    inttype result;

  /* socSend */
    buf = stri_to_bstri(stri);
    if (buf == NULL) {
      raise_error(MEMORY_ERROR);
      result = 0;
    } else if (buf->size != stri->size) {
      raise_error(RANGE_ERROR);
      FREE_BSTRI(buf, buf->size);
      result = 0;
    } else {
      result = send(sock, buf->mem, buf->size, flags);
      FREE_BSTRI(buf, buf->size);
    } /* if */
    return(result);
  } /* socSend */



#ifdef ANSI_C

inttype socSendto (sockettype sock, stritype stri, inttype flags,
    const_bstritype address)
#else

inttype socSendto (sock, stri, flags, address)
sockettype sock;
stritype stri;
inttype flags;
bstritype address;
#endif

  {
    bstritype buf;
    inttype result;

  /* socSendto */
    buf = stri_to_bstri(stri);
    if (buf == NULL) {
      raise_error(MEMORY_ERROR);
      result = 0;
    } else if (buf->size != stri->size) {
      raise_error(RANGE_ERROR);
      FREE_BSTRI(buf, buf->size);
      result = 0;
    } else {
      result = sendto(sock, buf->mem, buf->size, flags,
          (const struct sockaddr *) address->mem, address->size);
      FREE_BSTRI(buf, buf->size);
    } /* if */
    return(result);
  } /* socSendto */



#ifdef ANSI_C

sockettype socSocket (inttype domain, inttype type, inttype protocol)
#else

sockettype socSocket (domain, type, protocol)
inttype domain;
inttype type;
inttype protocol;
#endif

  {
#ifdef USE_WINSOCK
    static booltype initialized = FALSE;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
#endif
    sockettype result;

  /* socSocket */
#ifdef USE_WINSOCK
    if (!initialized) {
      wVersionRequested = MAKEWORD(2, 2);

      err = WSAStartup(wVersionRequested, &wsaData);
      if (err != 0) {
        raise_error(IO_ERROR);
        return(-1);
      } /* if */

      if (LOBYTE(wsaData.wVersion) != 2 ||
          HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        raise_error(IO_ERROR);
        return(-1);
      } /* if */
      initialized = TRUE;
    } /* if */
#endif
    result = socket(domain, type, protocol);
    return(result);
  } /* socSocket */



#ifdef ANSI_C

stritype socWordRead (sockettype sock, chartype *termination_char)
#else

stritype socWordRead (sock, termination_char)
sockettype sock;
chartype *termination_char;
#endif

  {
    char ch;
    register memsizetype position;
    register memsizetype bytes_received;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype result;

  /* socWordRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_STRI(memlength);
      memory = result->mem;
      position = 0;
      do {
        bytes_received = (memsizetype) recv(sock, &ch, 1, 0);
      } while (bytes_received == 1 && (ch == ' ' || ch == '\t'));
      while (bytes_received == 1 &&
          ch != ' ' && ch != '\t' && ch != '\n') {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          if (!RESIZE_STRI(result, memlength, newmemlength)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        bytes_received = (memsizetype) recv(sock, &ch, 1, 0);
      } /* while */
      if (bytes_received == 1 && ch == '\n' &&
          position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!RESIZE_STRI(result, memlength, position)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT3_STRI(memlength, position);
      result->size = position;
      if (bytes_received != 1) {
        *termination_char = (chartype) EOF;
      } else {
        *termination_char = (chartype) ch;
      } /* if */
      return(result);
    } /* if */
  } /* socWordRead */



#ifdef ANSI_C

void socWrite (sockettype sock, stritype stri)
#else

void socWrite (sock, stri)
sockettype sock;
stritype stri;
#endif

  {
    bstritype buf;

  /* socWrite */
    buf = stri_to_bstri(stri);
    if (buf == NULL) {
      raise_error(MEMORY_ERROR);
    } else if (buf->size != stri->size) {
      raise_error(RANGE_ERROR);
      FREE_BSTRI(buf, buf->size);
    } else {
      if (send(sock, buf->mem, buf->size, 0) != buf->size) {
        FREE_BSTRI(buf, buf->size);
        raise_error(IO_ERROR);
      } else {
        FREE_BSTRI(buf, buf->size);
      } /* if */
    } /* if */
  } /* socWrite */
