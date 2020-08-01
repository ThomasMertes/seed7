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
/* #include "Ws2tcpip.h" ** for getaddrinfo() */
#else
#include "netdb.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#endif
#include "errno.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "soc_rtl.h"


#ifdef USE_WINSOCK

typedef int socklen_type;
#define cast_send_recv_data(data_ptr) ((char *) (data_ptr))
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

#else

typedef socklen_t socklen_type;
#define cast_send_recv_data(data_ptr) ((void *) (data_ptr))

#endif

#define MAX_ADDRESS_SIZE 1024



#ifdef ANSI_C

sockettype socAccept (sockettype sock, bstritype *address)
#else

sockettype socAccept (sock, address)
sockettype sock;
bstritype *address;
#endif

  {
    bstritype resized_address;
    socklen_type addrlen;
    sockettype result;

  /* socAccept */
    REALLOC_BSTRI(resized_address, *address, (*address)->size, MAX_ADDRESS_SIZE);
    if (resized_address == NULL) {
      raise_error(MEMORY_ERROR);
      result = -1;
    } else {
      *address = resized_address;
      COUNT3_BSTRI((*address)->size, MAX_ADDRESS_SIZE);
      addrlen = MAX_ADDRESS_SIZE;
      result = accept(sock, (struct sockaddr *) (*address)->mem, &addrlen);
      if (result == -1 || addrlen < 0) {
        REALLOC_BSTRI(resized_address, *address, MAX_ADDRESS_SIZE, (*address)->size);
        if (resized_address == NULL) {
          (*address)->size = MAX_ADDRESS_SIZE;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (*address)->size);
        } /* if */
        /* printf("socAccept errno=%d\n", errno); */
        raise_error(RANGE_ERROR);
      } else {
        REALLOC_BSTRI(resized_address, *address, MAX_ADDRESS_SIZE, (memsizetype) addrlen);
        if (resized_address == NULL) {
          (*address)->size = MAX_ADDRESS_SIZE;
          raise_error(MEMORY_ERROR);
          result = -1;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memsizetype) addrlen);
          (*address)->size = (memsizetype) addrlen;
        } /* if */
      } /* if */
    } /* if */
    return(result);
  } /* socAccept */



#ifdef ANSI_C

void socBind (sockettype sock, const_bstritype address)
#else

void socBind (sock, address)
sockettype sock;
bstritype address;
#endif

  { /* socBind */
    if (bind(sock, (const struct sockaddr *) address->mem,
        (socklen_type) address->size) != 0) {
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

  { /* socClose */
    shutdown(sock, SHUT_RDWR);
#ifdef USE_WINSOCK
    closesocket(sock);
#else
    close(sock);
#endif
  } /* socClose */



#ifdef ANSI_C

void socConnect (sockettype sock, const_bstritype address)
#else

void socConnect (sock, address)
sockettype sock;
bstritype address;
#endif

  { /* socConnect */
    if (connect(sock, (const struct sockaddr *) address->mem,
        (socklen_type) address->size) != 0) {
      /* printf("socConnect(%d) errno=%d\n", sock, errno); */
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
    unsigned char ch;
    memsizetype bytes_received;

  /* socGetc */
    bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
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
    stritype resized_result;
    stritype result;

  /* socGets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      bytes_requested = (memsizetype) length;
      if (!ALLOC_STRI(result, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result_size = (memsizetype) recv(sock, cast_send_recv_data(result->mem),
          (size_t) bytes_requested, 0);
#ifdef UTF32_STRINGS
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
        REALLOC_STRI(resized_result, result, bytes_requested, result_size);
        if (resized_result == NULL) {
          FREE_STRI(result, bytes_requested);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(bytes_requested, result_size);
        } /* if */
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
        result = NULL;
        raise_error(MEMORY_ERROR);
      } else {
#ifdef OUT_OF_ORDER
        {
          struct addrinfo *res;
          int getaddrinfo_result;
          getaddrinfo_result = getaddrinfo(name, NULL, NULL, &res);
          if (getaddrinfo_result == 0) {
            printf("ai_flags=%d\n",    res->ai_flags);
            printf("ai_family=%d  (AF_INET=%d, AF_INET6=%d)\n",   res->ai_family, AF_INET, AF_INET6);
            printf("ai_socktype=%d\n", res->ai_socktype);
            printf("ai_protocol=%d\n", res->ai_protocol);
          } else {
            printf("getaddrinfo(\"%s\") -> %d\n", name, getaddrinfo_result);
            printf("EAI_AGAIN=%d  EAI_BADFLAGS=%d  EAI_FAIL=%d  EAI_FAMILY=%d  EAI_MEMORY=%d\n",
                EAI_AGAIN, EAI_BADFLAGS, EAI_FAIL, EAI_FAMILY, EAI_MEMORY);
            printf("EAI_NONAME=%d  EAI_SERVICE=%d  EAI_SOCKTYPE=%d\n",
                EAI_NONAME, EAI_SERVICE, EAI_SOCKTYPE);
            printf("EAI_SYSTEM=%d  EAI_OVERFLOW=%d\n",
		EAI_SYSTEM, EAI_OVERFLOW);
            /* printf("EAI_ADDRFAMILY=%d  EAI_NODATA=%d\n",
	        EAI_ADDRFAMILY, EAI_NODATA); */
          }
        }
#endif
        host_ent = gethostbyname(name);
        if (host_ent == NULL && h_errno == TRY_AGAIN) {
          /*
          printf("***** h_errno=%d\n", h_errno);
          printf("***** name=\"%s\"\n", name);
          printf("***** port=%d\n", port);
          printf("***** host_name=");
          prot_stri(host_name);
          printf("\n");
          */
          host_ent = gethostbyname(name);
        } /* if */
        if (host_ent == NULL) {
          result = NULL;
/*        printf("***** gethostbyname(\"%s\"): h_errno=%d\n", name, h_errno);
          printf("HOST_NOT_FOUND=%d  NO_DATA=%d  NO_RECOVERY=%d  TRY_AGAIN=%d\n",
              HOST_NOT_FOUND, NO_DATA, NO_RECOVERY, TRY_AGAIN); */
          raise_error(RANGE_ERROR);
        } else {
          /*
          printf("Host name:      %s\n", host_ent->h_name);
          printf("Port:           %d\n", port);
          printf("Address type:   %d\n", host_ent->h_addrtype);
          printf("Address type:   %d\n", AF_INET);
          printf("Address length: %d\n", host_ent->h_length);
          printf("Address length: %d\n", sizeof(struct sockaddr_in));
          printf("IP Address:     %s\n", inet_ntoa(*((struct in_addr *)host_ent->h_addr)));
          */
          if (host_ent->h_addrtype == AF_INET &&
              host_ent->h_length == sizeof(inet_address->sin_addr.s_addr)) {
            if (!ALLOC_BSTRI(result, sizeof(struct sockaddr_in))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = sizeof(struct sockaddr_in);
              inet_address = (struct sockaddr_in *) result->mem;
              inet_address->sin_family = host_ent->h_addrtype;
              inet_address->sin_port = htons((uint16type) port);       /* short, network byte order */
              memcpy(&inet_address->sin_addr.s_addr, host_ent->h_addr, (size_t) host_ent->h_length);
              memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
            } /* if */
          } else {
            result = NULL;
            raise_error(FILE_ERROR);
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
      inet_address->sin_port = htons((uint16type) port);      /* short, network byte order */
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
      inet_address->sin_port = htons((uint16type) port); /* short, network byte order */
      inet_address->sin_addr.s_addr = INADDR_ANY;        /* auto-fill with local IP */
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
    unsigned char ch;
    register memsizetype position;
    register memsizetype bytes_received;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* socLineRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
      while (bytes_received == 1 && ch != '\n') {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_STRI(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
      } /* while */
      if (bytes_received == 1 && ch == '\n' &&
          position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        if (bytes_received != 1) {
          *termination_char = (chartype) EOF;
        } else {
          *termination_char = (chartype) ch;
        } /* if */
      } /* if */
    } /* if */
    return(result);
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
    stritype resized_stri;
    memsizetype bytes_requested;
    memsizetype old_stri_size;
    memsizetype new_stri_size;

  /* socRecv */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      bytes_requested = (memsizetype) length;
      old_stri_size = (*stri)->size;
      if (old_stri_size < bytes_requested) {
        REALLOC_STRI(resized_stri, *stri, old_stri_size, bytes_requested);
        if (resized_stri == NULL) {
          raise_error(MEMORY_ERROR);
          return(0);
        } /* if */
        *stri = resized_stri;
        COUNT3_STRI(old_stri_size, bytes_requested);
        old_stri_size = bytes_requested;
      } /* if */
      new_stri_size = (memsizetype) recv(sock, cast_send_recv_data((*stri)->mem),
          (size_t) bytes_requested, flags);
#ifdef UTF32_STRINGS
      if (new_stri_size > 0) {
        uchartype *from = &((uchartype *) (*stri)->mem)[new_stri_size - 1];
        strelemtype *to = &(*stri)->mem[new_stri_size - 1];
        memsizetype number = new_stri_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
#endif
      (*stri)->size = new_stri_size;
      if (new_stri_size < old_stri_size) {
        REALLOC_STRI(resized_stri, *stri, old_stri_size, new_stri_size);
        if (resized_stri == NULL) {
          raise_error(MEMORY_ERROR);
          return(0);
        } /* if */
        *stri = resized_stri;
        COUNT3_STRI(old_stri_size, new_stri_size);
      } /* if */
    } /* if */
    return((inttype) new_stri_size);
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
    stritype resized_stri;
    bstritype resized_address;
    memsizetype bytes_requested;
    socklen_type fromlen;
    memsizetype stri_size;

  /* socRecvfrom */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      bytes_requested = (memsizetype) length;
      REALLOC_STRI(resized_stri, *stri, (*stri)->size, bytes_requested);
      if (resized_stri == NULL) {
        raise_error(MEMORY_ERROR);
        return(0);
      } /* if */
      *stri = resized_stri;
      COUNT3_STRI((*stri)->size, bytes_requested);
      REALLOC_BSTRI(resized_address, *address, (*address)->size, MAX_ADDRESS_SIZE);
      if (resized_address == NULL) {
        stri_size = (*stri)->size;
        REALLOC_STRI(resized_stri, *stri, bytes_requested, stri_size);
        if (resized_stri == NULL) {
          (*stri)->size = bytes_requested;
        } else {
          *stri = resized_stri;
          COUNT3_STRI(bytes_requested, stri_size);
        } /* if */
        raise_error(MEMORY_ERROR);
        return(0);
      } else {
        *address = resized_address;
        COUNT3_BSTRI((*address)->size, MAX_ADDRESS_SIZE);
        fromlen = MAX_ADDRESS_SIZE;
        stri_size = (memsizetype) recvfrom(sock, cast_send_recv_data((*stri)->mem),
            (size_t) bytes_requested, flags,
            (struct sockaddr *) (*address)->mem, &fromlen);
        if (stri_size == (memsizetype) -1 || fromlen < 0) {
          REALLOC_BSTRI(resized_address, *address, MAX_ADDRESS_SIZE, (*address)->size);
          if (resized_address == NULL) {
            (*address)->size = MAX_ADDRESS_SIZE;
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, (*address)->size);
          } /* if */
          /* printf("socRecvfrom errno=%d\n", errno); */
          raise_error(RANGE_ERROR);
        } else {
          REALLOC_BSTRI(resized_address, *address, MAX_ADDRESS_SIZE, (memsizetype) fromlen);
          if (resized_address == NULL) {
            (*address)->size = MAX_ADDRESS_SIZE;
            raise_error(MEMORY_ERROR);
            return(0);
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memsizetype) fromlen);
            (*address)->size = (memsizetype) fromlen;
          } /* if */
        } /* if */
      } /* if */
#ifdef UTF32_STRINGS
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
        REALLOC_STRI(resized_stri, *stri, bytes_requested, stri_size);
        if (resized_stri == NULL) {
          raise_error(MEMORY_ERROR);
          return(0);
        } /* if */
        *stri = resized_stri;
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
      FREE_BSTRI(buf, buf->size);
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = send(sock, cast_send_recv_data(buf->mem), buf->size, flags);
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
      FREE_BSTRI(buf, buf->size);
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = sendto(sock, cast_send_recv_data(buf->mem), buf->size, flags,
          (const struct sockaddr *) address->mem, (socklen_type) address->size);
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
        raise_error(FILE_ERROR);
        return(-1);
      } /* if */

      if (LOBYTE(wsaData.wVersion) != 2 ||
          HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        raise_error(FILE_ERROR);
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
    unsigned char ch;
    register memsizetype position;
    register memsizetype bytes_received;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* socWordRead */
    memlength = 256;
    if (!ALLOC_STRI(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      do {
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
      } while (bytes_received == 1 && (ch == ' ' || ch == '\t'));
      while (bytes_received == 1 &&
          ch != ' ' && ch != '\t' && ch != '\n') {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_STRI(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
      } /* while */
      if (bytes_received == 1 && ch == '\n' &&
          position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        if (bytes_received != 1) {
          *termination_char = (chartype) EOF;
        } else {
          *termination_char = (chartype) ch;
        } /* if */
      } /* if */
    } /* if */
    return(result);
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
    memsizetype bytes_sent;

  /* socWrite */
    buf = stri_to_bstri(stri);
    if (buf == NULL) {
      raise_error(MEMORY_ERROR);
    } else if (buf->size != stri->size) {
      FREE_BSTRI(buf, buf->size);
      raise_error(RANGE_ERROR);
    } else {
      bytes_sent = (memsizetype) send(sock, cast_send_recv_data(buf->mem), buf->size, 0);
      if (bytes_sent != buf->size) {
        /*
        printf("WSAGetLastError=%d\n", WSAGetLastError());
        printf("WSAECONNABORTED=%d\n", WSAECONNABORTED);
        printf("socWrite errno=%d\n", errno);
        printf("bytes_sent=%ld\n", bytes_sent);
        printf("buf->mem=%x\n", buf->mem);
        printf("buf->size=%lu\n", buf->size);
        printf("stri->size=%lu\n", stri->size);
        prot_stri(stri);
        printf("\n");
        printf("buf->mem[0]=%u\n", buf->mem[0]);
        */
        FREE_BSTRI(buf, buf->size);
        raise_error(FILE_ERROR);
      } else {
        FREE_BSTRI(buf, buf->size);
      } /* if */
    } /* if */
  } /* socWrite */
