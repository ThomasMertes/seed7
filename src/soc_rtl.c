/********************************************************************/
/*                                                                  */
/*  soc_rtl.c     Primitive actions for the socket type.            */
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
/*  File: seed7/src/soc_rtl.c                                       */
/*  Changes: 2007, 2011  Thomas Mertes                              */
/*  Content: Primitive actions for the socket type.                 */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#ifdef USE_WINSOCK
#define FD_SETSIZE 16384
#include "winsock2.h"
#ifdef USE_GETADDRINFO
#include "ws2tcpip.h"
#endif
#else
#include "sys/types.h"
#include "sys/socket.h"
#include "netdb.h"
#include "netinet/in.h"
#include "sys/select.h"
#ifdef HAS_POLL
#include "poll.h"
#endif
#endif
#include "errno.h"

#ifdef UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "os_decls.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "soc_rtl.h"


#ifdef USE_WINSOCK

typedef int socklen_type;
#define cast_send_recv_data(data_ptr) ((char *) (data_ptr))
#define cast_buffer_len(len)          ((int) (len))
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

#else

typedef socklen_t socklen_type;
#define cast_send_recv_data(data_ptr) ((void *) (data_ptr))
#define cast_buffer_len(len)          len
#define INVALID_SOCKET (-1)

#endif

#define MAX_ADDRESS_SIZE        1024
#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048

#ifdef USE_WINSOCK
static booltype initialized = FALSE;
#define check_initialization(err_result) if (unlikely(!initialized)) {if (init_winsock()) {return err_result;}}
#else
#define check_initialization(err_result)
#endif

#define BUFFER_SIZE 4096



#ifdef USE_WINSOCK
#ifdef ANSI_C

static int init_winsock (void)
#else

static int init_winsock ()
#endif

  {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    int result;

  /* init_winsock */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
      raise_error(FILE_ERROR);
      result = -1;
    } else {
      if (LOBYTE(wsaData.wVersion) != 2 ||
          HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        raise_error(FILE_ERROR);
        result = -1;
      } else {
        initialized = TRUE;
        result = 0;
      } /* if */
    } /* if */
    return result;
  } /* init_winsock */
#endif



#ifdef USE_GETADDRINFO
#ifdef ANSI_C

static struct addrinfo *select_addrinfo (struct addrinfo *addrinfo_list,
    int addr_family1, int addr_family2)
#else

static struct addrinfo *select_addrinfo (addrinfo_list, addr_family1, addr_family2)
struct addrinfo *addrinfo_list;
int addr_family1;
int addr_family2;
#endif

  {
    struct addrinfo *current_addrinfo;
    struct addrinfo *inet_addrinfo = NULL;
    struct addrinfo *inet6_addrinfo = NULL;
    struct addrinfo *alternate_inet_addrinfo = NULL;
    struct addrinfo *result_addrinfo = NULL;

  /* select_addrinfo */
    current_addrinfo = addrinfo_list;
    while (current_addrinfo != NULL) {
      if (current_addrinfo->ai_addr->sa_family == AF_INET) {
        if (inet_addrinfo == NULL) {
          struct sockaddr_in *inet_address = (struct sockaddr_in *) current_addrinfo->ai_addr;
          uint32type ip4_address = ntohl(inet_address->sin_addr.s_addr);
          /* printf("ip4=%d.%d.%d.%d\n",
              ip4_address >> 24, (ip4_address >> 16) & 255,
              (ip4_address >> 8) & 255, ip4_address & 255); */
          if ((ip4_address & (127 << 24)) == (127 << 24)) {
            alternate_inet_addrinfo = current_addrinfo;
          } else {
            inet_addrinfo = current_addrinfo;
          } /* if */
        } /* if */
      } else if (current_addrinfo->ai_addr->sa_family == AF_INET6) {
        if (inet6_addrinfo == NULL) {
          inet6_addrinfo = current_addrinfo;
        } /* if */
      } /* if */
      current_addrinfo = current_addrinfo->ai_next;
    } /* while */
    if (inet_addrinfo == NULL) {
      inet_addrinfo = alternate_inet_addrinfo;
    } /* if */
    if (addr_family1 == AF_INET && inet_addrinfo != NULL) {
      result_addrinfo = inet_addrinfo;
    } else if (addr_family1 == AF_INET6 && inet6_addrinfo != NULL) {
      result_addrinfo = inet6_addrinfo;
    } /* if */
    if (result_addrinfo == NULL) {
      if (addr_family2 == AF_INET && inet_addrinfo != NULL) {
        result_addrinfo = inet_addrinfo;
      } else if (addr_family2 == AF_INET6 && inet6_addrinfo != NULL) {
        result_addrinfo = inet6_addrinfo;
      } /* if */
      if (result_addrinfo == NULL) {
        /* No addr_family1 and addr_family2 present: Take first address */
        result_addrinfo = addrinfo_list;
      } /* if */
    } /* if */
    return result_addrinfo;
  } /* select_addrinfo */
#endif



#ifdef DUMP_ADDRINFO
#ifdef ANSI_C

static void dump_addrinfo (struct addrinfo *addrinfo_list)
#else

static void dump_addrinfo (addrinfo_list)
struct addrinfo *addrinfo_list;
#endif

  {
    struct addrinfo *addr;

  /* dump_addrinfo */
    addr = addrinfo_list;
    do {
      printf("ai_flags=%d\n",    addr->ai_flags);
      printf("ai_family=%d  (AF_INET=%d, AF_INET6=%d)\n", addr->ai_family, AF_INET, AF_INET6);
      printf("ai_socktype=%d\n", addr->ai_socktype);
      printf("ai_protocol=%d\n", addr->ai_protocol);
      printf("ai_addrlen=%d\n", addr->ai_addrlen);
      printf("sa_family=%d  (AF_INET=%d, AF_INET6=%d)\n", addr->ai_addr->sa_family, AF_INET, AF_INET6);
      if (addr->ai_addr->sa_family == AF_INET) {
        struct sockaddr_in *inet_address = (struct sockaddr_in *) addr->ai_addr;
        uint32type ip4_address = ntohl(inet_address->sin_addr.s_addr);
        printf("sin_port=%d\n", ntohs(inet_address->sin_port));
        printf("sin_addr.s_addr=%d.%d.%d.%d\n",
            (ip4_address >> 24) & 255,
            (ip4_address >> 16) & 255,
            (ip4_address >>  8) & 255,
             ip4_address        & 255);
      } else if (addr->ai_addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *inet6_address = (struct sockaddr_in6 *) addr->ai_addr;
        unsigned int digitGroup[8];
        int pos;
        printf("sin_port=%d\n", ntohs(inet6_address->sin6_port));
        for (pos = 0; pos <= 7; pos++) {
          digitGroup[pos] =
              inet6_address->sin6_addr.s6_addr[pos << 1] << 8 |
              inet6_address->sin6_addr.s6_addr[(pos << 1) + 1];
        } /* for */
        printf("sin_addr.s_addr=%x:%x:%x:%x:%x:%x:%x:%x\n",
            digitGroup[0], digitGroup[1], digitGroup[2], digitGroup[3],
            digitGroup[4], digitGroup[5], digitGroup[6], digitGroup[7]);
      }
      printf("ai_canonname=%s\n", addr->ai_canonname);
      printf("ai_next=%x\n", (memsizetype) addr->ai_next);
      /* {
        char name[1024];
        char serv[1024];
        getnameinfo(addr->ai_addr, 100, name, 1024, serv, 1024, 0);
        printf("name=%s\n", name);
        printf("serv=%s\n", serv);
      } */
      addr = addr->ai_next;
    } while (addr != NULL);
  } /* dump_addrinfo */
#endif



#ifdef ANSI_C

sockettype socAccept (sockettype sock, bstritype *address)
#else

sockettype socAccept (sock, address)
sockettype sock;
bstritype *address;
#endif

  {
    memsizetype old_address_size;
    bstritype resized_address;
    socklen_type addrlen;
    sockettype result;

  /* socAccept */
    /* printf("begin socAccept(%u, *)\n", sock); */
    old_address_size = (*address)->size;
    REALLOC_BSTRI_SIZE_OK(resized_address, *address, old_address_size, MAX_ADDRESS_SIZE);
    if (unlikely(resized_address == NULL)) {
      raise_error(MEMORY_ERROR);
      result = (sockettype) -1;
    } else {
      *address = resized_address;
      COUNT3_BSTRI(old_address_size, MAX_ADDRESS_SIZE);
      addrlen = MAX_ADDRESS_SIZE;
      result = accept(sock, (struct sockaddr *) (*address)->mem, &addrlen);
      if (unlikely(result == INVALID_SOCKET || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
        /* printf("socAccept(%d) errno=%d %s\n", sock, errno, strerror(errno)); */
        REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, old_address_size);
        if (resized_address == NULL) {
          (*address)->size = MAX_ADDRESS_SIZE;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, old_address_size);
        } /* if */
        raise_error(FILE_ERROR);
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, (memsizetype) addrlen);
        if (unlikely(resized_address == NULL)) {
          (*address)->size = MAX_ADDRESS_SIZE;
          raise_error(MEMORY_ERROR);
          result = (sockettype) -1;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memsizetype) addrlen);
          (*address)->size = (memsizetype) addrlen;
        } /* if */
      } /* if */
    } /* if */
    /* printf("end socAccept(%u, ", sock);
       prot_bstri(*address);
       printf(")\n"); */
    return result;
  } /* socAccept */



#ifdef ANSI_C

inttype socAddrFamily (const const_bstritype address)
#else

inttype socAddrFamily (address)
bstritype address;
#endif

  {
    const struct sockaddr *addr;
    inttype result;

  /* socAddrFamily */
    if (unlikely(address->size < sizeof(struct sockaddr))) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      addr = (const struct sockaddr *) address->mem;
      result = addr->sa_family;
      /* printf("socAddrFamily --> %d\n", result); */
    } /* if */
    return result;
  } /* socAddrFamily */



#ifdef ANSI_C

stritype socAddrNumeric (const const_bstritype address)
#else

stritype socAddrNumeric (address)
bstritype address;
#endif

  {
    const struct sockaddr *addr;
    char buffer[40];
    stritype result;

  /* socAddrNumeric */
    if (unlikely(address->size < sizeof(struct sockaddr))) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      addr = (const struct sockaddr *) address->mem;
      switch (addr->sa_family) {
        case AF_INET:
          if (unlikely(address->size != sizeof(struct sockaddr_in))) {
            raise_error(RANGE_ERROR);
            result = NULL;
          } else {
            const struct sockaddr_in *inet_address = (const struct sockaddr_in *) address->mem;
            uint32type ip4_address = ntohl(inet_address->sin_addr.s_addr);

            sprintf(buffer, "%d.%d.%d.%d",
                (ip4_address >> 24) & 255,
                (ip4_address >> 16) & 255,
                (ip4_address >>  8) & 255,
                 ip4_address        & 255);
            result = cstri_to_stri(buffer);
          } /* if */
          break;
#if defined USE_GETADDRINFO || defined INET6_SERVER_ADDRESS
        case AF_INET6:
          if (unlikely(address->size != sizeof(struct sockaddr_in6))) {
            raise_error(RANGE_ERROR);
            result = NULL;
          } else {
            const struct sockaddr_in6 *inet6_address = (const struct sockaddr_in6 *) address->mem;
            unsigned int digitGroup[8];
            int pos;

            for (pos = 0; pos <= 7; pos++) {
              digitGroup[pos] =
                  (unsigned int) (inet6_address->sin6_addr.s6_addr[pos << 1]) << 8 |
                  (unsigned int) (inet6_address->sin6_addr.s6_addr[(pos << 1) + 1]);
            } /* for */
            sprintf(buffer, "%x:%x:%x:%x:%x:%x:%x:%x",
                digitGroup[0], digitGroup[1], digitGroup[2], digitGroup[3],
                digitGroup[4], digitGroup[5], digitGroup[6], digitGroup[7]);
            result = cstri_to_stri(buffer);
          } /* if */
          break;
#endif
        default:
          raise_error(RANGE_ERROR);
          result = NULL;
          break;
      } /* switch */
    } /* if */
    return result;
  } /* socAddrNumeric */



#ifdef ANSI_C

stritype socAddrService (const const_bstritype address)
#else

stritype socAddrService (address)
bstritype address;
#endif

  {
    const struct sockaddr *addr;
    stritype result;

  /* socAddrService */
    if (unlikely(address->size < sizeof(struct sockaddr))) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      addr = (const struct sockaddr *) address->mem;
      switch (addr->sa_family) {
        case AF_INET:
          if (unlikely(address->size != sizeof(struct sockaddr_in))) {
            raise_error(RANGE_ERROR);
            result = NULL;
          } else {
            inttype port;
            const struct sockaddr_in *inet_address;
            inet_address = (const struct sockaddr_in *) address->mem;
            port = ntohs(inet_address->sin_port);       /* short, network byte order */
            result = intStr(port);
          } /* if */
          break;
#if defined USE_GETADDRINFO || defined INET6_SERVER_ADDRESS
        case AF_INET6:
          if (unlikely(address->size != sizeof(struct sockaddr_in6))) {
            raise_error(RANGE_ERROR);
            result = NULL;
          } else {
            inttype port;
            const struct sockaddr_in6 *inet6_address;
            inet6_address = (const struct sockaddr_in6 *) address->mem;
            port = ntohs(inet6_address->sin6_port);     /* short, network byte order */
            result = intStr(port);
          } /* if */
          break;
#endif
        default:
          raise_error(RANGE_ERROR);
          result = NULL;
          break;
      } /* switch */
    } /* if */
    return result;
  } /* socAddrService */



#ifdef ANSI_C

void socBind (sockettype sock, const_bstritype address)
#else

void socBind (sock, address)
sockettype sock;
bstritype address;
#endif

  { /* socBind */
    /* printf("socBind(%u, ", sock);
       prot_bstri(address);
       printf(")\n"); */
    if (unlikely(bind(sock, (const struct sockaddr *) address->mem,
        (socklen_type) address->size) != 0)) {
      /* printf("socBind errno=%d\n", errno);
         printf("EADDRINUSE=%d\n", EADDRINUSE); */
      raise_error(FILE_ERROR);
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
    /* printf("socConnect(%u, ", sock);
       prot_bstri(address);
       printf(")\n"); */
    if (unlikely(connect(sock, (const struct sockaddr *) address->mem,
        (socklen_type) address->size) != 0)) {
      /* printf("socConnect(%d) errno=%d %s\n", sock, errno, strerror(errno));
      printf("WSAGetLastError=%d\n", WSAGetLastError());
      printf("WSANOTINITIALISED=%ld, WSAENETDOWN=%ld, WSAEADDRINUSE=%ld, WSAEINTR=%ld, WSAEALREADY=%ld\n",
             WSANOTINITIALISED, WSAENETDOWN, WSAEADDRINUSE, WSAEINTR, WSAEALREADY);
      printf("WSAEINPROGRESS=%ld, WSAEADDRNOTAVAIL=%ld, WSAEAFNOSUPPORT=%ld, WSAECONNREFUSED=%ld\n",
             WSAEINPROGRESS, WSAEADDRNOTAVAIL, WSAEAFNOSUPPORT, WSAECONNREFUSED);
      printf("WSAEFAULT=%ld, WSAEINVAL=%ld, WSAEISCONN=%ld, WSAENETUNREACH=%ld, WSAEHOSTUNREACH=%ld\n",
             WSAEFAULT, WSAEINVAL, WSAEISCONN, WSAENETUNREACH, WSAEHOSTUNREACH);
      printf("WSAENOBUFS=%ld, WSAENOTSOCK=%ld, WSAETIMEDOUT=%ld, WSAEWOULDBLOCK=%ld, WSAEACCES=%ld\n",
             WSAENOBUFS, WSAENOTSOCK, WSAETIMEDOUT, WSAEWOULDBLOCK, WSAEACCES); */
      raise_error(FILE_ERROR);
    } /* if */
  } /* socConnect */



#ifdef ANSI_C

chartype socGetc (sockettype sock, chartype *const eofIndicator)
#else

chartype socGetc (sock, eofIndicator)
sockettype sock;
chartype *eofIndicator;
#endif

  {
    unsigned char ch;
    memsizetype bytes_received;

  /* socGetc */
    bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
    if (bytes_received != 1) {
      *eofIndicator = (chartype) EOF;
      return (chartype) EOF;
    } else {
      return (chartype) ch;
    } /* if */
  } /* socGetc */



#ifdef ANSI_C

stritype socGets (sockettype sock, inttype length, chartype *const eofIndicator)
#else

stritype socGets (sock, length, eofIndicator)
sockettype sock;
inttype length;
chartype *eofIndicator;
#endif

  {
    memsizetype bytes_requested;
    memsizetype result_size;
    stritype resized_result;
    stritype result;

  /* socGets */
    /* printf("socGets(%u, %d)\n", sock, length); */
    if (unlikely(length < 0)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if ((uinttype) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memsizetype) length;
      } /* if */
      if (bytes_requested <= BUFFER_SIZE) {
        uchartype buffer[BUFFER_SIZE];

        result_size = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                         cast_buffer_len(bytes_requested), 0);
        /* printf("socGets: result_size=%ld\n", (long int) result_size); */
        if (result_size == (memsizetype) (-1)) {
          result_size = 0;
        } /* if */
        if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, result_size))) {
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          if (result_size > 0) {
            register const uchartype *from = buffer;
            register strelemtype *to = result->mem;
            register memsizetype number = result_size;

            for (; number > 0; from++, to++, number--) {
              *to = *from;
            } /* for */
          } /* if */
          result->size = result_size;
          if (result_size == 0 && result_size < bytes_requested) {
            *eofIndicator = (chartype) EOF;
          } /* if */
        } /* if */
      } else {
        if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, bytes_requested))) {
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result_size = (memsizetype) recv(sock, cast_send_recv_data(result->mem),
                                           cast_buffer_len(bytes_requested), 0);
          /* printf("socGets: result_size=%ld\n", (long int) result_size); */
          if (result_size == (memsizetype) (-1)) {
            result_size = 0;
          } /* if */
          if (result_size > 0) {
            register const uchartype *from = &((uchartype *) result->mem)[result_size - 1];
            register strelemtype *to = &result->mem[result_size - 1];
            register memsizetype number = result_size;

            for (; number > 0; from--, to--, number--) {
              *to = *from;
            } /* for */
          } /* if */
          result->size = result_size;
          if (result_size < bytes_requested) {
            if (result_size == 0) {
              *eofIndicator = (chartype) EOF;
            } /* if */
            REALLOC_STRI_SIZE_OK(resized_result, result, bytes_requested, result_size);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI(result, bytes_requested);
              raise_error(MEMORY_ERROR);
              result = NULL;
            } else {
              result = resized_result;
              COUNT3_STRI(bytes_requested, result_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socGets */



#ifdef ANSI_C

bstritype socGetAddr (sockettype sock)
#else

bstritype socGetAddr (sock)
sockettype sock;
#endif

  {
    socklen_type addrlen;
    int getsockname_result;
    bstritype result;

  /* socGetAddr */
    /* printf("socGetAddr(%u)\n", sock); */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, MAX_ADDRESS_SIZE))) {
      raise_error(MEMORY_ERROR);
    } else {
      addrlen = MAX_ADDRESS_SIZE;
      getsockname_result = getsockname(sock, (struct sockaddr *) result->mem, &addrlen);
      if (unlikely(getsockname_result != 0 || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
        FREE_BSTRI(result, MAX_ADDRESS_SIZE);
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        result->size = (memsizetype) addrlen;
        /* {
          struct sockaddr_in *inet_address = (struct sockaddr_in *) result->mem;
          printf("sin_addr.s_addr=%d.%d.%d.%d\n",
              address        & 255,
             (address >>  8) & 255,
             (address >> 16) & 255,
             (address >> 24) & 255);
        } */
      } /* if */
    } /* if */
    return result;
  } /* socGetAddr */



#ifdef ANSI_C

stritype socGetHostname (void)
#else

stritype socGetHostname ()
#endif

  {
    char name[1024];
    stritype result;

  /* socGetHostname */
    check_initialization(NULL);
    if (gethostname(name, 1024) != 0) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result = cstri8_or_cstri_to_stri(name);
    } /* if */
    return result;
  } /* socGetHostname */



#ifdef ANSI_C

booltype socHasNext (sockettype sock)
#else

booltype socHasNext (sock)
sockettype sock;
#endif

  {
    unsigned char next_char;
    memsizetype bytes_received;
    booltype result;

  /* socHasNext */
    bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&next_char), 1, MSG_PEEK);
    if (bytes_received != 1) {
      /* printf("socHasNext: bytes_received=%ld\n", (long int) bytes_received); */
      result = FALSE;
    } else {
      /* printf("socHasNext: next_char=%d\n", next_char); */
      result = TRUE;
    } /* if */
    return result;
  } /* socHasNext */



#ifdef ANSI_C

bstritype socInetAddr (const const_stritype host_name, inttype port)
#else

bstritype socInetAddr (host_name, port)
stritype host_name;
inttype port;
#endif

  {
    cstritype name;
#ifdef USE_GETADDRINFO
    char servicename[10];
    struct addrinfo *addrinfo_list;
    struct addrinfo *result_addrinfo;
    struct addrinfo hints;
    int getaddrinfo_result;
#else
    struct hostent *host_ent;
    struct sockaddr_in *inet_address;
#endif
    bstritype result;

  /* socInetAddr */
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      name = cp_to_cstri8(host_name);
      if (unlikely(name == NULL)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
#ifdef USE_GETADDRINFO
        sprintf(servicename, "%u", port);
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo_result = getaddrinfo(name, servicename, &hints, &addrinfo_list);
        if (unlikely(getaddrinfo_result != 0)) {
          /* printf("getaddrinfo(\"%s\") -> %d\n", name, getaddrinfo_result); */
          free_cstri(name, host_name);
          if (getaddrinfo_result == EAI_NONAME) {
            /* Return empty address */
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = 0;
            } /* if */
          } else {
            /*
            printf("EAI_AGAIN=%d  EAI_BADFLAGS=%d  EAI_FAIL=%d  EAI_FAMILY=%d  EAI_MEMORY=%d\n",
                EAI_AGAIN, EAI_BADFLAGS, EAI_FAIL, EAI_FAMILY, EAI_MEMORY);
            printf("EAI_NONAME=%d  EAI_SERVICE=%d  EAI_SOCKTYPE=%d\n",
                EAI_NONAME, EAI_SERVICE, EAI_SOCKTYPE);
            */
            /* printf("EAI_SYSTEM=%d  EAI_OVERFLOW=%d\n",
                EAI_SYSTEM, EAI_OVERFLOW); */
            /* printf("EAI_ADDRFAMILY=%d  EAI_NODATA=%d\n",
                EAI_ADDRFAMILY, EAI_NODATA); */
            raise_error(FILE_ERROR);
            result = NULL;
          } /* if */
        } else {
          /* dump_addrinfo(addrinfo_list); */
          result_addrinfo = select_addrinfo(addrinfo_list, AF_INET, AF_INET6);
          if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_addrinfo->ai_addrlen))) {
            free_cstri(name, host_name);
            freeaddrinfo(addrinfo_list);
            raise_error(MEMORY_ERROR);
          } else {
            result->size = result_addrinfo->ai_addrlen;
            memcpy(result->mem, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen);
            free_cstri(name, host_name);
            freeaddrinfo(addrinfo_list);
          } /* if */
        } /* if */
#else
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
        if (unlikely(host_ent == NULL)) {
          /* printf("***** gethostbyname(\"%s\"): h_errno=%d\n", name, h_errno);
             printf("HOST_NOT_FOUND=%d  NO_DATA=%d  NO_RECOVERY=%d  TRY_AGAIN=%d\n",
                 HOST_NOT_FOUND, NO_DATA, NO_RECOVERY, TRY_AGAIN); */
          free_cstri(name, host_name);
          /* Return empty address */
          if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
            raise_error(MEMORY_ERROR);
          } else {
            result->size = 0;
          } /* if */
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
          free_cstri(name, host_name);
          if (host_ent->h_addrtype == AF_INET &&
              host_ent->h_length == sizeof(inet_address->sin_addr.s_addr)) {
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, sizeof(struct sockaddr_in)))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = sizeof(struct sockaddr_in);
              inet_address = (struct sockaddr_in *) result->mem;
              inet_address->sin_family = host_ent->h_addrtype;
              inet_address->sin_port = htons((uint16type) port);       /* short, network byte order */
              memcpy(&inet_address->sin_addr.s_addr, host_ent->h_addr, (size_t) host_ent->h_length);
              memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
              /* {
                uint32type ip4_address = ntohl(inet_address->sin_addr.s_addr);
                printf("ip4_address=%d.%d.%d.%d\n",
                    (ip4_address >> 24  & 255,
                    (ip4_address >> 16) & 255,
                    (ip4_address >>  8) & 255,
                     ip4_address        & 255);
              } */
            } /* if */
          } else {
            /* printf("socInetAddr: addrtype=%d\n", host_ent->h_addrtype); */
            /* raise_error(FILE_ERROR);
               result = NULL; */
            /* Return empty address */
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = 0;
            } /* if */
          } /* if */
        } /* if */
#endif
      } /* if */
    } /* if */
    /* printf("socInetAddr --> ");
       prot_bstri(result);
       printf("\n"); */
    return result;
  } /* socInetAddr */



#ifdef ANSI_C

bstritype socInetLocalAddr (inttype port)
#else

bstritype socInetLocalAddr (port)
inttype port;
#endif

  {
#ifdef USE_GETADDRINFO
    char servicename[10];
    struct addrinfo *addrinfo_list;
    struct addrinfo *result_addrinfo;
    struct addrinfo hints;
    int getaddrinfo_result;
#else
    struct sockaddr_in *inet_address;
#endif
    bstritype result;

  /* socInetLocalAddr */
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
#ifdef USE_GETADDRINFO
      sprintf(servicename, "%u", port);
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      getaddrinfo_result = getaddrinfo(NULL, servicename, &hints, &addrinfo_list);
      if (unlikely(getaddrinfo_result != 0)) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        /* dump_addrinfo(addrinfo_list); */
        result_addrinfo = select_addrinfo(addrinfo_list, AF_INET, AF_INET6);
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_addrinfo->ai_addrlen))) {
          freeaddrinfo(addrinfo_list);
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_addrinfo->ai_addrlen;
          memcpy(result->mem, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen);
          freeaddrinfo(addrinfo_list);
        } /* if */
      } /* if */
#else
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, sizeof(struct sockaddr_in)))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = sizeof(struct sockaddr_in);
        inet_address = (struct sockaddr_in *) result->mem;
        inet_address->sin_family = AF_INET;
        inet_address->sin_port = htons((uint16type) port);      /* short, network byte order */
        inet_address->sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* local host */
        memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
      } /* if */
#endif
    } /* if */
    /* printf("socInetLocalAddr --> ");
       prot_bstri(result);
       printf("\n"); */
    return result;
  } /* socInetLocalAddr */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

bstritype socInetOwnAddr (inttype port)
#else

bstritype socInetOwnAddr (port)
inttype port;
#endif

  {
    bstritype result;

  /* socInetOwnAddr */
    return result;
  } /* socInetOwnAddr */
#endif



#ifdef ANSI_C

bstritype socInetServAddr (inttype port)
#else

bstritype socInetServAddr (port)
inttype port;
#endif

  {
#ifdef USE_GETADDRINFO
    char servicename[10];
    struct addrinfo *addrinfo_list;
    struct addrinfo *result_addrinfo;
    struct addrinfo hints;
    int getaddrinfo_result;
#else
#ifdef INET6_SERVER_ADDRESS
    struct sockaddr_in6 *inet6_address;
#else
    struct sockaddr_in *inet_address;
#endif
#endif
    bstritype result;

  /* socInetServAddr */
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
#ifdef USE_GETADDRINFO
      sprintf(servicename, "%u", port);
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      getaddrinfo_result = getaddrinfo(NULL, servicename, &hints, &addrinfo_list);
      if (unlikely(getaddrinfo_result != 0)) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        /* dump_addrinfo(addrinfo_list); */
        result_addrinfo = select_addrinfo(addrinfo_list, AF_INET, AF_INET6);
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_addrinfo->ai_addrlen))) {
          freeaddrinfo(addrinfo_list);
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_addrinfo->ai_addrlen;
          memcpy(result->mem, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen);
          freeaddrinfo(addrinfo_list);
        } /* if */
      } /* if */
#else
#ifdef INET6_SERVER_ADDRESS
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, sizeof(struct sockaddr_in6)))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = sizeof(struct sockaddr_in6);
        inet6_address = (struct sockaddr_in6 *) result->mem;
        inet6_address->sin6_family = AF_INET6;
        inet6_address->sin6_port = htons((uint16type) port); /* short, network byte order */
        inet6_address->sin6_flowinfo = 0;
        memcpy(&inet6_address->sin6_addr, &in6addr_any, sizeof(struct in6_addr)); /* auto-fill with local IP */
        inet6_address->sin6_scope_id = 0;
      } /* if */
#else
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, sizeof(struct sockaddr_in)))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = sizeof(struct sockaddr_in);
        inet_address = (struct sockaddr_in *) result->mem;
        inet_address->sin_family = AF_INET;
        inet_address->sin_port = htons((uint16type) port); /* short, network byte order */
        inet_address->sin_addr.s_addr = INADDR_ANY;        /* auto-fill with local IP */
        memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
      } /* if */
#endif
#endif
    } /* if */
    /* printf("socInetServAddr --> ");
       prot_bstri(result);
       printf("\n"); */
    return result;
  } /* socInetServAddr */



#ifdef HAS_POLL
#ifdef ANSI_C

booltype socInputReady (sockettype sock, inttype seconds, inttype micro_seconds)
#else

booltype socInputReady (sock, seconds, micro_seconds)
sockettype sock;
inttype seconds;
inttype micro_seconds;
#endif

  {
    struct pollfd pollFd[1];
    int poll_result;
    unsigned char next_char;
    memsizetype bytes_received;
    booltype result;

  /* socInputReady */
    pollFd[0].fd = (int) sock;
    pollFd[0].events = POLLIN;
    poll_result = os_poll(pollFd, 1, 0);
    if (unlikely(poll_result < 0)) {
      raise_error(FILE_ERROR);
      result = FALSE;
    } else {
      result = poll_result == 1 && (pollFd[0].revents & POLLIN);
      if (result) {
        /* Verify that it is really possible to read at least one character */
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&next_char), 1, MSG_PEEK);
        if (bytes_received != 1) {
          /* printf("socInputReady: bytes_received=%ld\n", (long int) bytes_received); */
          result = FALSE;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socInputReady */

#else



#ifdef ANSI_C

booltype socInputReady (sockettype sock, inttype seconds, inttype micro_seconds)
#else

booltype socInputReady (sock, seconds, micro_seconds)
sockettype sock;
inttype seconds;
inttype micro_seconds;
#endif

  {
    int nfds;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    unsigned char next_char;
    memsizetype bytes_received;
    booltype result;

  /* socInputReady */
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    nfds = (int) sock + 1;
    timeout.tv_sec = seconds;
    timeout.tv_usec = micro_seconds;
    /* printf("select(%d, %d)\n", nfds, sock); */
    select_result = select(nfds, &readfds, NULL, NULL, &timeout);
    /* printf("select_result: %d\n", select_result); */
    if (unlikely(select_result < 0)) {
      raise_error(FILE_ERROR);
      result = FALSE;
    } else {
      result = FD_ISSET(sock, &readfds);
      if (result) {
        /* Verify that it is really possible to read at least one character */
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&next_char), 1, MSG_PEEK);
        if (bytes_received != 1) {
          /* printf("socInputReady: bytes_received=%ld\n", (long int) bytes_received); */
          result = FALSE;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socInputReady */

#endif



#define BUFFER_START_SIZE 256
#define BUFFER_DELTA_SIZE 256

#ifdef ANSI_C

stritype socLineRead (sockettype sock, chartype *const terminationChar)
#else

stritype socLineRead (sock, terminationChar)
sockettype sock;
chartype *terminationChar;
#endif

  {
    memsizetype bytes_received;
    memsizetype bytes_requested;
    memsizetype result_size;
    memsizetype old_result_size;
    memsizetype result_pos;
    uchartype *nlPos;
    uchartype buffer[BUFFER_SIZE];
    stritype resized_result;
    stritype result;

  /* socLineRead */
    bytes_received = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                        BUFFER_START_SIZE, MSG_PEEK);
    if (bytes_received == (memsizetype) (-1)) {
      bytes_received = 0;
    } /* if */
    if (bytes_received == 0) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = 0;
        *terminationChar = (chartype) EOF;
      } /* if */
    } else {
      nlPos = (uchartype *) memchr(buffer, '\n', bytes_received);
      if (nlPos != NULL) {
        bytes_requested = (memsizetype) (nlPos - buffer) + 1;
        /* This should overwrite the buffer with identical data up to '\n'. */
        bytes_received = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                            cast_buffer_len(bytes_requested), 0);
        /* bytes_received should always be identical to bytes_requested. */
        result_size = bytes_requested - 1;
        if (nlPos != buffer && nlPos[-1] == '\r') {
          result_size--;
        } /* if */
        if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, result_size))) {
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          if (result_size > 0) {
            register const uchartype *from = buffer;
            register strelemtype *to = result->mem;
            register memsizetype number = result_size;

            for (; number > 0; from++, to++, number--) {
              *to = *from;
            } /* for */
          } /* if */
          result->size = result_size;
          *terminationChar = '\n';
        } /* if */
      } else {
        result_size = bytes_received;
        old_result_size = 0;
        result_pos = 0;
        result = NULL;
        do {
          bytes_requested = bytes_received;
          /* This should overwrite the buffer with identical data. */
          bytes_received = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                              cast_buffer_len(bytes_requested), 0);
          /* bytes_received should always be identical to bytes_requested. */
          result_size += BUFFER_DELTA_SIZE;
          /* printf("A result=%08lx, old_result_size=%d, result_size=%d\n", (unsigned long) result, old_result_size, result_size); */
          REALLOC_STRI_CHECK_SIZE(resized_result, result, old_result_size, result_size);
          /* printf("B result=%08lx, resized_result=%08lx\n", (unsigned long) result, (unsigned long) resized_result); */
          if (unlikely(resized_result == NULL)) {
            if (result != NULL) {
              FREE_STRI(result, old_result_size);
            } /* if */
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            old_result_size = result_size;
            /* printf("a result[%d], size=%d\n", result_pos, bytes_requested); */
            {
              register const uchartype *from = buffer;
              register strelemtype *to = &result->mem[result_pos];
              register memsizetype number = bytes_requested;

              for (; number > 0; from++, to++, number--) {
                *to = *from;
              } /* for */
            }
            result_pos += bytes_requested;
            bytes_received = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                                BUFFER_DELTA_SIZE, MSG_PEEK);
            if (bytes_received == (memsizetype) (-1)) {
              bytes_received = 0;
            } /* if */
            if (bytes_received == 0) {
              REALLOC_STRI_CHECK_SIZE(resized_result, result, result_size, result_pos);
              if (unlikely(resized_result == NULL)) {
                FREE_STRI(result, result_size);
                raise_error(MEMORY_ERROR);
                result = NULL;
              } else {
                result = resized_result;
                result->size = result_pos;
                *terminationChar = (chartype) EOF;
              } /* if */
            } else {
              nlPos = (uchartype *) memchr(buffer, '\n', bytes_received);
            } /* if */
          } /* if */
        } while (result != NULL && bytes_received != 0 && nlPos == NULL);
        if (result != NULL && nlPos != NULL) {
          bytes_requested = (memsizetype) (nlPos - buffer) + 1;
          /* This should overwrite the buffer with identical data up to '\n'. */
          bytes_received = (memsizetype) recv(sock, cast_send_recv_data(buffer),
                                              cast_buffer_len(bytes_requested), 0);
          /* bytes_received should always be identical to bytes_requested. */
          bytes_requested--;
          if (nlPos == buffer) {
            if (result->mem[result_pos - 1] == '\r') {
              result_pos--;
            } /* if */
          } else if (nlPos[-1] == '\r') {
            bytes_requested--;
          } /* if */
          old_result_size = result_size;
          result_size = result_pos + bytes_requested;
          /* printf("C result=%08lx, old_result_size=%d, result_size=%d\n", (unsigned long) result, old_result_size, result_size); */
          REALLOC_STRI_CHECK_SIZE(resized_result, result, old_result_size, result_size);
          /* printf("D result=%08lx, resized_result=%08lx\n", (unsigned long) result, (unsigned long) resized_result); */
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, result_size);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            /* printf("e result[%d], size=%d\n", result_pos, bytes_requested); */
            if (result_size > 0) {
              register const uchartype *from = buffer;
              register strelemtype *to = &result->mem[result_pos];
              register memsizetype number = bytes_requested;

              for (; number > 0; from++, to++, number--) {
                *to = *from;
              } /* for */
            } /* if */
            result->size = result_size;
            *terminationChar = '\n';
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socLineRead */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

stritype socLineRead (sockettype sock, chartype *const terminationChar)
#else

stritype socLineRead (sock, terminationChar)
sockettype sock;
chartype *terminationChar;
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
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      bytes_received = (memsizetype) recv(sock, cast_send_recv_data(&ch), 1, 0);
      while (bytes_received == 1 && ch != '\n') {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
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
      REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        if (bytes_received != 1) {
          *terminationChar = (chartype) EOF;
        } else {
          *terminationChar = (chartype) ch;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socLineRead */
#endif



#ifdef ANSI_C

void socListen (sockettype sock, inttype backlog)
#else

void socListen (sock, backlog)
sockettype sock;
inttype backlog;
#endif

  { /* socListen */
    if (unlikely(listen(sock, backlog) != 0)) {
      /* printf("socListen(%d) errno=%d %s\n", sock, errno, strerror(errno));
      printf("WSAGetLastError=%d\n", WSAGetLastError());
      printf("WSANOTINITIALISED=%ld, WSAENETDOWN=%ld, WSAEFAULT=%ld, WSAENOTCONN=%ld\n",
             WSANOTINITIALISED, WSAENETDOWN, WSAEFAULT, WSAENOTCONN);
      printf("WSAEINTR=%ld, WSAEINPROGRESS=%ld, WSAENETRESET=%ld, WSAENOTSOCK=%ld\n",
             WSAEINTR, WSAEINPROGRESS, WSAENETRESET, WSAENOTSOCK);
      printf("WSAEOPNOTSUPP=%ld, WSAESHUTDOWN=%ld, WSAEWOULDBLOCK=%ld, WSAEMSGSIZE=%ld\n",
             WSAEOPNOTSUPP, WSAESHUTDOWN, WSAEWOULDBLOCK, WSAEMSGSIZE);
      printf("WSAEINVAL=%ld, WSAECONNABORTED=%ld, WSAETIMEDOUT=%ld, WSAECONNRESET=%ld\n",
             WSAEINVAL, WSAECONNABORTED, WSAETIMEDOUT, WSAECONNRESET);
      printf("WSAENOBUFS=%ld\n", WSAENOBUFS); */
      raise_error(FILE_ERROR);
    } /* if */
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
    if (unlikely(length < 0)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      if ((uinttype) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memsizetype) length;
      } /* if */
      old_stri_size = (*stri)->size;
      if (old_stri_size < bytes_requested) {
        REALLOC_STRI_CHECK_SIZE(resized_stri, *stri, old_stri_size, bytes_requested);
        if (unlikely(resized_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return 0;
        } /* if */
        *stri = resized_stri;
        COUNT3_STRI(old_stri_size, bytes_requested);
        old_stri_size = bytes_requested;
      } /* if */
      new_stri_size = (memsizetype) recv(sock, cast_send_recv_data((*stri)->mem),
                                         cast_buffer_len(bytes_requested), flags);
      if (new_stri_size > 0) {
        uchartype *from = &((uchartype *) (*stri)->mem)[new_stri_size - 1];
        strelemtype *to = &(*stri)->mem[new_stri_size - 1];
        memsizetype number = new_stri_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
      (*stri)->size = new_stri_size;
      if (new_stri_size < old_stri_size) {
        REALLOC_STRI_SIZE_OK(resized_stri, *stri, old_stri_size, new_stri_size);
        if (unlikely(resized_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return 0;
        } /* if */
        *stri = resized_stri;
        COUNT3_STRI(old_stri_size, new_stri_size);
      } /* if */
    } /* if */
    return (inttype) new_stri_size;
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
    memsizetype old_address_size;
    bstritype resized_address;
    memsizetype bytes_requested;
    socklen_type addrlen;
    memsizetype stri_size;

  /* socRecvfrom */
    if (unlikely(length < 0)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      if ((uinttype) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memsizetype) length;
      } /* if */
      REALLOC_STRI_CHECK_SIZE(resized_stri, *stri, (*stri)->size, bytes_requested);
      if (unlikely(resized_stri == NULL)) {
        raise_error(MEMORY_ERROR);
        return 0;
      } /* if */
      *stri = resized_stri;
      COUNT3_STRI((*stri)->size, bytes_requested);
      old_address_size = (*address)->size;
      REALLOC_BSTRI_SIZE_OK(resized_address, *address, old_address_size, MAX_ADDRESS_SIZE);
      if (unlikely(resized_address == NULL)) {
        stri_size = (*stri)->size;
        REALLOC_STRI_SIZE_OK(resized_stri, *stri, bytes_requested, stri_size);
        if (resized_stri == NULL) {
          (*stri)->size = bytes_requested;
        } else {
          *stri = resized_stri;
          COUNT3_STRI(bytes_requested, stri_size);
        } /* if */
        raise_error(MEMORY_ERROR);
        return 0;
      } else {
        *address = resized_address;
        COUNT3_BSTRI(old_address_size, MAX_ADDRESS_SIZE);
        addrlen = MAX_ADDRESS_SIZE;
        stri_size = (memsizetype) recvfrom(sock, cast_send_recv_data((*stri)->mem),
                                           cast_buffer_len(bytes_requested), flags,
                                           (struct sockaddr *) (*address)->mem, &addrlen);
        if (unlikely(stri_size == (memsizetype) -1 || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
          REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, old_address_size);
          if (resized_address == NULL) {
            (*address)->size = MAX_ADDRESS_SIZE;
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, old_address_size);
          } /* if */
          /* printf("socRecvfrom errno=%d\n", errno); */
          raise_error(FILE_ERROR);
        } else {
          REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, (memsizetype) addrlen);
          if (unlikely(resized_address == NULL)) {
            (*address)->size = MAX_ADDRESS_SIZE;
            raise_error(MEMORY_ERROR);
            return 0;
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memsizetype) addrlen);
            (*address)->size = (memsizetype) addrlen;
          } /* if */
        } /* if */
      } /* if */
      if (stri_size > 0) {
        uchartype *from = &((uchartype *) (*stri)->mem)[stri_size - 1];
        strelemtype *to = &(*stri)->mem[stri_size - 1];
        memsizetype number = stri_size;

        for (; number > 0; from--, to--, number--) {
          *to = *from;
        } /* for */
      } /* if */
      (*stri)->size = stri_size;
      if (stri_size < bytes_requested) {
        REALLOC_STRI_SIZE_OK(resized_stri, *stri, bytes_requested, stri_size);
        if (unlikely(resized_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return 0;
        } /* if */
        *stri = resized_stri;
        COUNT3_STRI(bytes_requested, stri_size);
      } /* if */
    } /* if */
    return (inttype) stri_size;
  } /* socRecvfrom */



#ifdef ANSI_C

inttype socSend (sockettype sock, const const_stritype stri, inttype flags)
#else

inttype socSend (sock, stri, flags)
sockettype sock;
stritype stri;
inttype flags;
#endif

  {
    bstritype buf;
    memsizetype bytes_sent;
    inttype result;

  /* socSend */
    buf = stri_to_bstri(stri);
    if (unlikely(buf == NULL)) {
      raise_error(MEMORY_ERROR);
      result = 0;
    } else if (unlikely(buf->size != stri->size)) {
      FREE_BSTRI(buf, buf->size);
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      bytes_sent = (memsizetype) send(sock, cast_send_recv_data(buf->mem),
                                      cast_buffer_len(buf->size), flags);
      FREE_BSTRI(buf, buf->size);
      if (unlikely(bytes_sent == (memsizetype) -1)) {
        result = -1;
      } else if (unlikely(bytes_sent > MAX_MEM_INDEX)) {
        result = MAX_MEM_INDEX;
      } else {
        result = (inttype) bytes_sent;
      } /* if */
    } /* if */
    return result;
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

  {
    bstritype buf;
    memsizetype bytes_sent;
    inttype result;

  /* socSendto */
    buf = stri_to_bstri(stri);
    if (unlikely(buf == NULL)) {
      raise_error(MEMORY_ERROR);
      result = 0;
    } else if (unlikely(buf->size != stri->size)) {
      FREE_BSTRI(buf, buf->size);
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      bytes_sent = (memsizetype) sendto(sock, cast_send_recv_data(buf->mem),
                                        cast_buffer_len(buf->size), flags,
                                        (const struct sockaddr *) address->mem,
                                        (socklen_type) address->size);
      FREE_BSTRI(buf, buf->size);
      if (unlikely(bytes_sent == (memsizetype) -1)) {
        result = -1;
      } else if (unlikely(bytes_sent > MAX_MEM_INDEX)) {
        result = MAX_MEM_INDEX;
      } else {
        result = (inttype) bytes_sent;
      } /* if */
    } /* if */
    return result;
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
    sockettype result;

  /* socSocket */
    /* printf("socSocket(%d, %d, %d)\n", domain, type, protocol); */
    check_initialization((sockettype) -1);
    result = socket(domain, type, protocol);
#if defined USE_WINSOCK && !defined TWOS_COMPLEMENT_INTTYPE
    /* In this case INVALID_SOCKET != (sockettype) -1 holds and    */
    /* (sockettype) -1 must be returned instead of INVALID_SOCKET. */
    /* Probably a computer, which needs this, does not exist.      */
    if (unlikely(result == INVALID_SOCKET)) {
      result = (sockettype) -1;
    } /* if */
#endif
    return result;
  } /* socSocket */



#ifdef ANSI_C

stritype socWordRead (sockettype sock, chartype *const terminationChar)
#else

stritype socWordRead (sock, terminationChar)
sockettype sock;
chartype *terminationChar;
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
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
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
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (unlikely(resized_result == NULL)) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
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
      REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        if (bytes_received != 1) {
          *terminationChar = (chartype) EOF;
        } else {
          *terminationChar = (chartype) ch;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socWordRead */



#ifdef ANSI_C

void socWrite (sockettype sock, const const_stritype stri)
#else

void socWrite (sock, stri)
sockettype sock;
stritype stri;
#endif

  {
    memsizetype bytes_sent;

  /* socWrite */
    if (stri->size <= BUFFER_SIZE) {
      register strelemtype *str = stri->mem;
      register uchartype *ustri;
      register uint16type buf_len = (uint16type) stri->size;
      uchartype buffer[BUFFER_SIZE];

      for (ustri = buffer; buf_len > 0; buf_len--) {
        if (unlikely(*str >= 256)) {
          raise_error(RANGE_ERROR);
          return;
        } /* if */
        *ustri++ = (uchartype) *str++;
      } /* for */
      bytes_sent = (memsizetype) send(sock, cast_send_recv_data(buffer),
                                      cast_buffer_len(stri->size), 0);
    } else {
      bstritype buf;

      buf = stri_to_bstri(stri);
      if (unlikely(buf == NULL)) {
        raise_error(MEMORY_ERROR);
        return;
      } else if (unlikely(buf->size != stri->size)) {
        FREE_BSTRI(buf, buf->size);
        raise_error(RANGE_ERROR);
        return;
      } /* if */
      bytes_sent = (memsizetype) send(sock, cast_send_recv_data(buf->mem),
                                      cast_buffer_len(buf->size), 0);
      FREE_BSTRI(buf, buf->size);
    } /* if */
    if (unlikely(bytes_sent != stri->size)) {
      /* printf("socWrite(%d) errno=%d %s\n", sock, errno, strerror(errno));
      printf("WSAGetLastError=%d\n", WSAGetLastError());
      printf("WSANOTINITIALISED=%ld, WSAENETDOWN=%ld, WSAEFAULT=%ld, WSAENOTCONN=%ld\n",
             WSANOTINITIALISED, WSAENETDOWN, WSAEFAULT, WSAENOTCONN);
      printf("WSAEINTR=%ld, WSAEINPROGRESS=%ld, WSAENETRESET=%ld, WSAENOTSOCK=%ld\n",
             WSAEINTR, WSAEINPROGRESS, WSAENETRESET, WSAENOTSOCK);
      printf("WSAEOPNOTSUPP=%ld, WSAESHUTDOWN=%ld, WSAEWOULDBLOCK=%ld, WSAEMSGSIZE=%ld\n",
             WSAEOPNOTSUPP, WSAESHUTDOWN, WSAEWOULDBLOCK, WSAEMSGSIZE);
      printf("WSAEINVAL=%ld, WSAECONNABORTED=%ld, WSAETIMEDOUT=%ld, WSAECONNRESET=%ld\n",
             WSAEINVAL, WSAECONNABORTED, WSAETIMEDOUT, WSAECONNRESET);
      printf("WSAENOBUFS=%ld\n", WSAENOBUFS);
      printf("bytes_sent=%ld, stri->size=%lu\n", (long) bytes_sent, stri->size); */
      raise_error(FILE_ERROR);
    } /* if */
  } /* socWrite */
