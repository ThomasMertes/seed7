/********************************************************************/
/*                                                                  */
/*  soc_rtl.c     Primitive actions for the socket type.            */
/*  Copyright (C) 1989 - 2015  Thomas Mertes                        */
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
/*  Changes: 2007, 2011, 2013 - 2015  Thomas Mertes                 */
/*  Content: Primitive actions for the socket type.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
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

typedef int sockLenType;
#define cast_send_recv_data(data_ptr) ((char *) (data_ptr))
#define cast_buffer_len(len)          ((int) (len))
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

#else

typedef socklen_t sockLenType;
#define cast_send_recv_data(data_ptr) ((void *) (data_ptr))
#define cast_buffer_len(len)          len
#define INVALID_SOCKET (-1)

#endif

#define BUFFER_SIZE             4096
#define GETS_DEFAULT_SIZE    1048576
#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048
#define MAX_ADDRESS_SIZE        1024

#ifdef USE_WINSOCK
static boolType initialized = FALSE;
#define check_initialization(err_result) if (unlikely(!initialized)) {if (init_winsock()) {return err_result;}}
#else
#define check_initialization(err_result)
#endif



#ifdef USE_WINSOCK
static int init_winsock (void)

  {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    int result;

  /* init_winsock */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
      logError(printf("init_winsock: WSAStartup() failed:\n"
                      "errno=%d\nerror: %s\n",
                      ERROR_INFORMATION););
      raise_error(FILE_ERROR);
      result = -1;
    } else {
      if (LOBYTE(wsaData.wVersion) != 2 ||
          HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        logError(printf("init_winsock: WSAStartup() "
                        "did not return the requested version:\n"););
        raise_error(FILE_ERROR);
        result = -1;
      } else {
        initialized = TRUE;
        result = 0;
      } /* if */
    } /* if */
    return result;
  } /* init_winsock */



cstriType wsaErrorMessage (void)

  {
    static char buffer[1024];

  /* wsaErrorMessage */
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                   NULL, WSAGetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer, 1024, NULL);
    return buffer;
  } /* wsaErrorMessage */
#endif



#ifdef USE_GETADDRINFO
static struct addrinfo *select_addrinfo (struct addrinfo *addrinfo_list,
    int addr_family1, int addr_family2)

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
          uint32Type ip4_address = ntohl(inet_address->sin_addr.s_addr);
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
static void dump_addrinfo (struct addrinfo *addrinfo_list)

  {
    struct addrinfo *addr;

  /* dump_addrinfo */
    addr = addrinfo_list;
    do {
      printf("ai_flags=%d\n",    addr->ai_flags);
      printf("ai_family=%d (%s)\n", addr->ai_family,
          addr->ai_family == AF_INET ? "AF_INET" :
          (addr->ai_family == AF_INET6 ? "AF_INET6" :
          (addr->ai_family == AF_UNSPEC ? "AF_UNSPEC" : "UNKNOWN")));
      printf("ai_socktype=%d\n", addr->ai_socktype);
      printf("ai_protocol=%d\n", addr->ai_protocol);
      printf("ai_addrlen=%d\n", addr->ai_addrlen);
      printf("sa_family=%d (%s)\n", addr->ai_addr->sa_family,
          addr->ai_addr->sa_family == AF_INET ? "AF_INET" :
          (addr->ai_addr->sa_family == AF_INET6 ? "AF_INET6" :
          (addr->ai_addr->sa_family == AF_UNSPEC ? "AF_UNSPEC" : "UNKNOWN")));
      if (addr->ai_addr->sa_family == AF_INET) {
        struct sockaddr_in *inet_address = (struct sockaddr_in *) addr->ai_addr;
        uint32Type ip4_address = ntohl(inet_address->sin_addr.s_addr);
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
              (unsigned int) inet6_address->sin6_addr.s6_addr[pos << 1] << 8 |
              (unsigned int) inet6_address->sin6_addr.s6_addr[(pos << 1) + 1];
        } /* for */
        printf("sin_addr.s_addr=%x:%x:%x:%x:%x:%x:%x:%x\n",
            digitGroup[0], digitGroup[1], digitGroup[2], digitGroup[3],
            digitGroup[4], digitGroup[5], digitGroup[6], digitGroup[7]);
      }
      printf("ai_canonname=%s\n", addr->ai_canonname);
      printf("ai_next=%lx\n", (memSizeType) addr->ai_next);
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



/**
 *  Read a string, when we do not know how many bytes are avaliable.
 *  This function reads the data is read into a list of buffers,
 *  until enough characters are read or EOF has been reached.
 *  Afterwards the string is allocated, the data is copied from the
 *  buffers and the list of buffers is freed.
 */
static striType read_and_alloc_stri (socketType inSocket, memSizeType chars_missing,
    errInfoType *err_info)

  {
    struct bufferStruct buffer;
    bufferList currBuffer = &buffer;
    bufferList oldBuffer;
    memSizeType bytes_in_buffer = LIST_BUFFER_SIZE;
    boolType input_ready = TRUE;
    memSizeType result_pos;
    memSizeType result_size = 0;
    striType result;

  /* read_and_alloc_stri */
    logFunction(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *)\n",
                       inSocket, chars_missing););
    buffer.next = NULL;
    while (chars_missing - result_size >= LIST_BUFFER_SIZE &&
           bytes_in_buffer == LIST_BUFFER_SIZE &&
           input_ready &&
           *err_info == OKAY_NO_ERROR) {
      bytes_in_buffer = (memSizeType) recv((os_socketType) inSocket,
                                           cast_send_recv_data(currBuffer->buffer),
                                           cast_buffer_len(LIST_BUFFER_SIZE), 0);
      /* printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n", bytes_in_buffer); */
      if (unlikely(bytes_in_buffer == (memSizeType) (-1) && result_size == 0)) {
        logError(printf("read_and_alloc_stri: "
                        "recv(%d, *, " FMT_U_MEM ", 0) failed.\n",
                        inSocket, (memSizeType) LIST_BUFFER_SIZE););
        *err_info = FILE_ERROR;
        result = NULL;
      } else {
        result_size += bytes_in_buffer;
        if (chars_missing > result_size && bytes_in_buffer == LIST_BUFFER_SIZE) {
          currBuffer->next = (bufferList) malloc(sizeof(struct bufferStruct));
          if (unlikely(currBuffer->next == NULL)) {
            logError(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *): "
                            "Out of memory when allocating buffer.\n",
                            inSocket, chars_missing););
            *err_info = MEMORY_ERROR;
            result = NULL;
          } else {
            currBuffer = currBuffer->next;
            currBuffer->next = NULL;
            input_ready = socInputReady(inSocket, 0, 0);
          } /* if */
        } /* if */
      } /* if */
    } /* while */
    if (chars_missing > result_size &&
        bytes_in_buffer == LIST_BUFFER_SIZE &&
        input_ready &&
        *err_info == OKAY_NO_ERROR) {
      bytes_in_buffer = (memSizeType) recv((os_socketType) inSocket,
                                           cast_send_recv_data(currBuffer->buffer),
                                           cast_buffer_len(chars_missing - result_size), 0);
      /* printf("read_and_alloc_stri: bytes_in_buffer=" FMT_U_MEM "\n", bytes_in_buffer); */
      if (unlikely(bytes_in_buffer == (memSizeType) (-1) && result_size == 0)) {
        logError(printf("read_and_alloc_stri: "
                        "recv(%d, *, " FMT_U_MEM ", 0) failed.\n",
                        inSocket, chars_missing - result_size););
        *err_info = FILE_ERROR;
        result = NULL;
      } else {
        result_size += bytes_in_buffer;
      } /* if */
    } /* if */
    if (likely(*err_info == OKAY_NO_ERROR)) {
      if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
        logError(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", *): "
                        "Out of memory when allocating result.\n",
                        inSocket, chars_missing););
        *err_info = MEMORY_ERROR;
      } else {
        result->size = result_size;
        currBuffer = &buffer;
        result_pos = 0;
        while (result_size - result_pos >= LIST_BUFFER_SIZE) {
          memcpy_to_strelem(&result->mem[result_pos], currBuffer->buffer, LIST_BUFFER_SIZE);
          currBuffer = currBuffer->next;
          result_pos += LIST_BUFFER_SIZE;
        } /* while */
        memcpy_to_strelem(&result->mem[result_pos], currBuffer->buffer, result_size - result_pos);
      } /* if */
    } /* if */
    currBuffer = buffer.next;
    while (currBuffer != NULL) {
      oldBuffer = currBuffer;
      currBuffer = currBuffer->next;
      free(oldBuffer);
    } /* while */
    logFunction(printf("read_and_alloc_stri(%d, " FMT_U_MEM ", %d) --> \"%s\"\n",
                       inSocket, chars_missing, *err_info, striAsUnquotedCStri(result)););
    return result;
  } /* read_and_alloc_stri */



/**
 *  Create a new accepted connection socket for 'listenerSocket'.
 *  The function waits until at least one connection request is
 *  in the sockets queue of pending connections. Then it extracts
 *  the first connection request from the sockets queue. This
 *  request is accepted and a connection socket is created for it.
 *  @return the accepted connection socket.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
socketType socAccept (socketType listenerSocket, bstriType *address)

  {
    memSizeType old_address_size;
    bstriType resized_address;
    sockLenType addrlen;
    os_socketType result;

  /* socAccept */
    logFunction(printf("socAccept(%d, \"%s\")\n",
                       listenerSocket, bstriAsUnquotedCStri(*address)););
    old_address_size = (*address)->size;
    REALLOC_BSTRI_SIZE_OK(resized_address, *address, old_address_size, MAX_ADDRESS_SIZE);
    if (unlikely(resized_address == NULL)) {
      raise_error(MEMORY_ERROR);
      result = (os_socketType) -1;
    } else {
      *address = resized_address;
      COUNT3_BSTRI(old_address_size, MAX_ADDRESS_SIZE);
      addrlen = MAX_ADDRESS_SIZE;
      result = (os_socketType) accept((os_socketType) listenerSocket,
                                      (struct sockaddr *) (*address)->mem, &addrlen);
      if (unlikely(result == INVALID_SOCKET || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
        /* printf("socAccept(%d) errno=%d %s\n", listenerSocket, ERROR_INFORMATION); */
        REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, old_address_size);
        if (resized_address == NULL) {
          (*address)->size = MAX_ADDRESS_SIZE;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, old_address_size);
        } /* if */
        logError(printf("socAccept: accept(%d, \"%s\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        listenerSocket, bstriAsUnquotedCStri(*address),
                        ERROR_INFORMATION););
        raise_error(FILE_ERROR);
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, (memSizeType) addrlen);
        if (unlikely(resized_address == NULL)) {
          (*address)->size = MAX_ADDRESS_SIZE;
          raise_error(MEMORY_ERROR);
          result = (os_socketType) -1;
        } else {
          *address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memSizeType) addrlen);
          (*address)->size = (memSizeType) addrlen;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socAccept(%d, \"%s\") --> %d\n",
                       listenerSocket, bstriAsUnquotedCStri(*address), result););
    return (socketType) result;
  } /* socAccept */



intType socAddrFamily (const const_bstriType address)

  {
    const struct sockaddr *addr;
    intType result;

  /* socAddrFamily */
    logFunction(printf("socAddrFamily(\"%s\")\n", bstriAsUnquotedCStri(address)););
    if (unlikely(address->size < sizeof(struct sockaddr))) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      addr = (const struct sockaddr *) address->mem;
      result = addr->sa_family;
      /* printf("socAddrFamily --> %d\n", result); */
    } /* if */
    logFunction(printf("socAddrFamily(\"%s\") --> " FMT_D "\n",
                       bstriAsUnquotedCStri(address), result););
    return result;
  } /* socAddrFamily */



/**
 *  Get the numeric (IP) address of the host at 'address'.
 *  IPv4 addresses return the socketAddress in dot notation (e.g.:
 *  "192.0.2.235") and IPv6 addresses return the socketAddress in
 *  colon notation (e.g.: "fe80:0:0:0:202:b3ff:fe1e:8329").
 *  @return the IP address of the specified host.
 */
striType socAddrNumeric (const const_bstriType address)

  {
    const struct sockaddr *addr;
    char buffer[40];
    striType result;

  /* socAddrNumeric */
    logFunction(printf("socAddrNumeric(\"%s\")\n", bstriAsUnquotedCStri(address)););
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
            uint32Type ip4_address = ntohl(inet_address->sin_addr.s_addr);

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
    logFunction(printf("socAddrNumeric(\"%s\") --> \"%s\"\n",
                       bstriAsUnquotedCStri(address), striAsUnquotedCStri(result)););
    return result;
  } /* socAddrNumeric */



striType socAddrService (const const_bstriType address)

  {
    const struct sockaddr *addr;
    striType result;

  /* socAddrService */
    logFunction(printf("socAddrService(\"%s\")\n", bstriAsUnquotedCStri(address)););
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
            intType port;
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
            intType port;
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
    logFunction(printf("socAddrService(\"%s\") --> \"%s\"\n",
                       bstriAsUnquotedCStri(address), striAsUnquotedCStri(result)););
    return result;
  } /* socAddrService */



/**
 *  Assign the specified 'address' to the 'listenerSocket'.
 *  @param address An internet listener socket address.
 *  @exception FILE_ERROR A system function returns an error.
 */
void socBind (socketType listenerSocket, const_bstriType address)

  { /* socBind */
    logFunction(printf("socBind(%d, \"%s\")\n", 
                       listenerSocket, bstriAsUnquotedCStri(address)););
    if (unlikely(bind((os_socketType) listenerSocket,
                      (const struct sockaddr *) address->mem,
                      (sockLenType) address->size) != 0)) {
      logError(printf("socBind: bind(%d, \"%s\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      listenerSocket, bstriAsUnquotedCStri(address),
                      ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } /* if */
  } /* socBind */


/**
 *  Close the socket 'aSocket'.
 *  @exception FILE_ERROR A system function returns an error.
 */
void socClose (socketType aSocket)

  {
    int close_result;

  /* socClose */
    logFunction(printf("socClose(%d)\n", aSocket););
    shutdown((os_socketType) aSocket, SHUT_RDWR);
#ifdef USE_WINSOCK
    close_result = closesocket((os_socketType) aSocket);
#else
    close_result = close((os_socketType) aSocket);
#endif
    if (unlikely(close_result != 0)) {
      logError(printf("socClose: close(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      aSocket, ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } /* if */
  } /* socClose */



/**
 *  Connect 'aSocket' to the given 'address'.
 *  @exception FILE_ERROR A system function returns an error.
 */
void socConnect (socketType aSocket, const_bstriType address)

  { /* socConnect */
    logFunction(printf("socConnect(%d, \"%s\")\n",
                       aSocket, bstriAsUnquotedCStri(address)););
    if (unlikely(connect((os_socketType) aSocket,
                         (const struct sockaddr *) address->mem,
                         (sockLenType) address->size) != 0)) {
      logError(printf("socConnect: connect(%d, \"%s\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      aSocket, bstriAsUnquotedCStri(address),
                      ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } /* if */
  } /* socConnect */



/**
 *  Read a character from 'inSocket'.
 *  @return the character read.
 */
charType socGetc (socketType inSocket, charType *const eofIndicator)

  {
    unsigned char ch;
    memSizeType bytes_received;

  /* socGetc */
    bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                        cast_send_recv_data(&ch), 1, 0);
    if (bytes_received != 1) {
      *eofIndicator = (charType) EOF;
      return (charType) EOF;
    } else {
      return (charType) ch;
    } /* if */
  } /* socGetc */



/**
 *  Read a string with a maximum length from 'inSocket'.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType socGets (socketType inSocket, intType length, charType *const eofIndicator)

  {
    memSizeType chars_requested;
    memSizeType result_size;
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* socGets */
    logFunction(printf("socGets(%d, " FMT_D ", '\\" FMT_U32 ";')\n",
                       inSocket, length, *eofIndicator););
    if (unlikely(length < 0)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        chars_requested = MAX_MEMSIZETYPE;
      } else {
        chars_requested = (memSizeType) length;
      } /* if */
      if (chars_requested <= BUFFER_SIZE) {
        ucharType buffer[BUFFER_SIZE];

        result_size = (memSizeType) recv((os_socketType) inSocket,
                                         cast_send_recv_data(buffer),
                                         cast_buffer_len(chars_requested), 0);
        /* printf("socGets: result_size=" FMT_U_MEM "\n", result_size); */
        if (result_size == (memSizeType) (-1)) {
          result_size = 0;
        } /* if */
        if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, result_size))) {
          logError(printf("socGets(%d, " FMT_D ", *): "
                          "Out of memory when allocating result.\n",
                          inSocket, length););
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          memcpy_to_strelem(result->mem, buffer, result_size);
          result->size = result_size;
          if (result_size == 0 && result_size < chars_requested) {
            *eofIndicator = (charType) EOF;
          } /* if */
        } /* if */
      } else {
        if (chars_requested > GETS_DEFAULT_SIZE) {
          /* Read a string, when we do not know how many bytes are avaliable. */
          result = read_and_alloc_stri(inSocket, chars_requested, &err_info);
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            raise_error(err_info);
          } /* if */
        } else {
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, chars_requested))) {
            logError(printf("socGets(%d, " FMT_D ", *): "
                            "Out of memory when allocating result.\n",
                            inSocket, length););
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result_size = (memSizeType) recv((os_socketType) inSocket,
                                             cast_send_recv_data(result->mem),
                                             cast_buffer_len(chars_requested), 0);
            /* printf("socGets: result_size=" FMT_U_MEM "\n", result_size); */
            if (result_size == (memSizeType) (-1)) {
              result_size = 0;
            } /* if */
            memcpy_to_strelem(result->mem, (ucharType *) result->mem, result_size);
            result->size = result_size;
            if (result_size < chars_requested) {
              if (result_size == 0) {
                *eofIndicator = (charType) EOF;
              } /* if */
              REALLOC_STRI_SIZE_SMALLER(resized_result, result, chars_requested, result_size);
              if (unlikely(resized_result == NULL)) {
                FREE_STRI(result, chars_requested);
                logError(printf("socGets(%d, " FMT_D ", *): "
                                "Out of memory when allocating result.\n",
                                inSocket, length););
                raise_error(MEMORY_ERROR);
                result = NULL;
              } else {
                result = resized_result;
                COUNT3_STRI(chars_requested, result_size);
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socGets(%d, " FMT_D ", '\\" FMT_U32 ";') --> \"%s\"\n",
                       inSocket, length, *eofIndicator, striAsUnquotedCStri(result)););
    return result;
  } /* socGets */



/**
 *  Determine the hostname.
 *  @return the hostname.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType socGetHostname (void)

  {
    char name[1024];
    striType result;

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



/**
 *  Get the local address of the socket 'sock'.
 *  @return the address to which the socket 'sock' is bound.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType socGetLocalAddr (socketType sock)

  {
    sockLenType addrlen;
    int getsockname_result;
    bstriType resized_address;
    bstriType address;

  /* socGetLocalAddr */
    logFunction(printf("socGetLocalAddr(%d)\n", sock););
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(address, MAX_ADDRESS_SIZE))) {
      raise_error(MEMORY_ERROR);
    } else {
      addrlen = MAX_ADDRESS_SIZE;
      getsockname_result = getsockname((os_socketType) sock,
                                       (struct sockaddr *) address->mem, &addrlen);
      if (unlikely(getsockname_result != 0 || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
        FREE_BSTRI(address, MAX_ADDRESS_SIZE);
        logError(printf("socGetLocalAddr: getsockname(%d, ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        sock, ERROR_INFORMATION););
        raise_error(FILE_ERROR);
        address = NULL;
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_address, address, MAX_ADDRESS_SIZE, (memSizeType) addrlen);
        if (unlikely(resized_address == NULL)) {
          FREE_BSTRI(address, MAX_ADDRESS_SIZE);
          raise_error(MEMORY_ERROR);
          address = NULL;
        } else {
          address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memSizeType) addrlen);
          address->size = (memSizeType) addrlen;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socGetLocalAddr(%d) --> \"%s\"\n",
                       sock, bstriAsUnquotedCStri(address)););
    return address;
  } /* socGetLocalAddr */



/**
 *  Get the address of the peer to which the socket 'sock' is connected.
 *  @return the address of the peer connected to the socket 'sock'.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType socGetPeerAddr (socketType sock)

  {
    sockLenType addrlen;
    int getpeername_result;
    bstriType resized_address;
    bstriType address;

  /* socGetPeerAddr */
    logFunction(printf("socGetPeerAddr(%d)\n", sock););
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(address, MAX_ADDRESS_SIZE))) {
      raise_error(MEMORY_ERROR);
    } else {
      addrlen = MAX_ADDRESS_SIZE;
      getpeername_result = getpeername((os_socketType) sock,
                                       (struct sockaddr *) address->mem, &addrlen);
      if (unlikely(getpeername_result != 0 || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
        FREE_BSTRI(address, MAX_ADDRESS_SIZE);
        logError(printf("socGetPeerAddr: getpeername(%d, ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        sock, ERROR_INFORMATION););
        raise_error(FILE_ERROR);
        address = NULL;
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_address, address, MAX_ADDRESS_SIZE, (memSizeType) addrlen);
        if (unlikely(resized_address == NULL)) {
          FREE_BSTRI(address, MAX_ADDRESS_SIZE);
          raise_error(MEMORY_ERROR);
          address = NULL;
        } else {
          address = resized_address;
          COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memSizeType) addrlen);
          address->size = (memSizeType) addrlen;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socGetPeerAddr(%d) --> \"%s\"\n",
                       sock, bstriAsUnquotedCStri(address)););
    return address;
  } /* socGetPeerAddr */



/**
 *  Determine if at least one character can be read successfully.
 *  This function allows a socket to be handled like an iterator.
 *  Since socHasNext peeks the next character from the socket
 *  it may block.
 *  @return FALSE if socGetc would return EOF, TRUE otherwise.
 */
boolType socHasNext (socketType inSocket)

  {
    unsigned char next_char;
    memSizeType bytes_received;
    boolType result;

  /* socHasNext */
    bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                        cast_send_recv_data(&next_char), 1, MSG_PEEK);
    if (bytes_received != 1) {
      /* printf("socHasNext: bytes_received=%ld\n", (long int) bytes_received); */
      result = FALSE;
    } else {
      /* printf("socHasNext: next_char=%d\n", next_char); */
      result = TRUE;
    } /* if */
    return result;
  } /* socHasNext */



/**
 *  Create an internet socket address of a port at a host.
 *  The 'hostName' is either a host name (e.g.: "www.example.org"),
 *  or an IPv4 address in standard dot notation (e.g.: "192.0.2.235").
 *  Operating systems supporting IPv6 may also accept an IPv6 address
 *  in colon notation.
 *  @return the internet socket address or an empty bstring when
 *          the host cannot be found.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception RANGE_ERROR The port is not in the range 0 to 65535 or
 *             hostName cannot be converted to the system string type.
 *  @exception MEMORY_ERROR Not enough memory to convert 'hostName'.
 *             to the system representation or not enough memory to
 *             represent the result.
 */
bstriType socInetAddr (const const_striType hostName, intType port)

  {
    cstriType name;
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
    errInfoType err_info = OKAY_NO_ERROR;
    bstriType result;

  /* socInetAddr */
    logFunction(printf("socInetAddr(\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(hostName), port););
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      name = stri_to_cstri8(hostName, &err_info);
      if (unlikely(name == NULL)) {
        raise_error(err_info);
        result = NULL;
      } else {
#ifdef USE_GETADDRINFO
        sprintf(servicename, "%u", (unsigned int) port);
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo_result = getaddrinfo(name, servicename, &hints, &addrinfo_list);
        if (unlikely(getaddrinfo_result != 0)) {
          /* printf("getaddrinfo(\"%s\") -> %d\n", name, getaddrinfo_result); */
          free_cstri8(name, hostName);
          if (getaddrinfo_result == EAI_NONAME) {
            /* Return empty address */
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = 0;
            } /* if */
          } else {
            logError(printf("socInetAddr: getaddrinfo(...) failed:\n"
                            "errno=%d\nerror: %s\n",
                            ERROR_INFORMATION););
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
            free_cstri8(name, hostName);
            freeaddrinfo(addrinfo_list);
            raise_error(MEMORY_ERROR);
          } else {
            result->size = result_addrinfo->ai_addrlen;
            memcpy(result->mem, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen);
            free_cstri8(name, hostName);
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
          printf("***** hostName=");
          prot_stri(hostName);
          printf("\n");
          */
          host_ent = gethostbyname(name);
        } /* if */
        if (unlikely(host_ent == NULL)) {
          /* printf("***** gethostbyname(\"%s\"): h_errno=%d\n", name, h_errno);
             printf("HOST_NOT_FOUND=%d  NO_DATA=%d  NO_RECOVERY=%d  TRY_AGAIN=%d\n",
                 HOST_NOT_FOUND, NO_DATA, NO_RECOVERY, TRY_AGAIN); */
          free_cstri8(name, hostName);
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
          free_cstri8(name, hostName);
          if (host_ent->h_addrtype == AF_INET &&
              host_ent->h_length == sizeof(inet_address->sin_addr.s_addr)) {
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, sizeof(struct sockaddr_in)))) {
              raise_error(MEMORY_ERROR);
            } else {
              result->size = sizeof(struct sockaddr_in);
              inet_address = (struct sockaddr_in *) result->mem;
              inet_address->sin_family = host_ent->h_addrtype;
              inet_address->sin_port = htons((uint16Type) port);       /* short, network byte order */
              memcpy(&inet_address->sin_addr.s_addr, host_ent->h_addr, (size_t) host_ent->h_length);
              memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
              /* {
                uint32Type ip4_address = ntohl(inet_address->sin_addr.s_addr);
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
    logFunction(printf("socInetAddr(\"%s\", " FMT_D ") --> \"%s\"\n",
                       striAsUnquotedCStri(hostName), port,
                       bstriAsUnquotedCStri(result)););
    return result;
  } /* socInetAddr */



/**
 *  Create an internet socket address of a port at localhost.
 *  @return the internet socket address.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception RANGE_ERROR The port is not in the range 0 to 65535.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType socInetLocalAddr (intType port)

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
    bstriType result;

  /* socInetLocalAddr */
    logFunction(printf("socInetLocalAddr(" FMT_D ")\n", port););
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
#ifdef USE_GETADDRINFO
      sprintf(servicename, "%u", (unsigned int) port);
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      getaddrinfo_result = getaddrinfo(NULL, servicename, &hints, &addrinfo_list);
      if (unlikely(getaddrinfo_result != 0)) {
        logError(printf("socInetLocalAddr: getaddrinfo(NULL, %s, ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        servicename, ERROR_INFORMATION););
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
        inet_address->sin_port = htons((uint16Type) port);      /* short, network byte order */
        inet_address->sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* local host */
        memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
      } /* if */
#endif
    } /* if */
    logFunction(printf("socInetLocalAddr(" FMT_D ") --> \"%s\"\n",
                       port, bstriAsUnquotedCStri(result)););
    return result;
  } /* socInetLocalAddr */



/**
 *  Create an internet listener socket address of a port at localhost.
 *  @return the internet listener socket address.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception RANGE_ERROR The port is not in the range 0 to 65535.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType socInetServAddr (intType port)

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
    bstriType result;

  /* socInetServAddr */
    logFunction(printf("socInetServAddr(" FMT_D ")\n", port););
    check_initialization(NULL);
    if (unlikely(port < 0 || port > 65535)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
#ifdef USE_GETADDRINFO
      sprintf(servicename, "%u", (unsigned int) port);
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      getaddrinfo_result = getaddrinfo(NULL, servicename, &hints, &addrinfo_list);
      if (unlikely(getaddrinfo_result != 0)) {
        logError(printf("socInetServAddr: getaddrinfo(NULL, %s, ...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        servicename, ERROR_INFORMATION););
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
        inet6_address->sin6_port = htons((uint16Type) port); /* short, network byte order */
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
        inet_address->sin_port = htons((uint16Type) port); /* short, network byte order */
        inet_address->sin_addr.s_addr = INADDR_ANY;        /* auto-fill with local IP */
        memset(inet_address->sin_zero, '\0', sizeof(inet_address->sin_zero));
      } /* if */
#endif
#endif
    } /* if */
    logFunction(printf("socInetServAddr(" FMT_D ") --> \"%s\"\n",
                       port, bstriAsUnquotedCStri(result)););
    return result;
  } /* socInetServAddr */



#ifdef HAS_POLL
boolType socInputReady (socketType sock, intType seconds, intType micro_seconds)

  {
    struct pollfd pollFd[1];
    int poll_result;
    unsigned char next_char;
    memSizeType bytes_received;
    boolType result;

  /* socInputReady */
    logFunction(printf("socInputReady(%d, " FMT_D ", " FMT_D ")\n",
                       sock, seconds, micro_seconds););
    if (unlikely(seconds < 0 || seconds >= INT_MAX / 1000 ||
                 micro_seconds < 0 || micro_seconds >= 1000000)) {
      raise_error(RANGE_ERROR);
      result = FALSE;
    } else {
      pollFd[0].fd = (int) sock;
      pollFd[0].events = POLLIN;
      poll_result = os_poll(pollFd, 1, (int) seconds * 1000 + (int) (micro_seconds / 1000));
      if (unlikely(poll_result < 0)) {
        logError(printf("socInputReady(%d): os_poll(...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        sock, ERROR_INFORMATION););
        raise_error(FILE_ERROR);
        result = FALSE;
      } else {
        result = poll_result == 1 && (pollFd[0].revents & POLLIN);
        if (result) {
          /* Verify that it is really possible to read at least one character */
          bytes_received = (memSizeType) recv((os_socketType) sock,
                                              cast_send_recv_data(&next_char), 1, MSG_PEEK);
          if (bytes_received != 1) {
            /* printf("socInputReady: bytes_received=%ld\n", (long int) bytes_received); */
            result = FALSE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socInputReady(%d, " FMT_D ", " FMT_D ") --> %d\n",
                       sock, seconds, micro_seconds, result););
    return result;
  } /* socInputReady */

#else



boolType socInputReady (socketType sock, intType seconds, intType micro_seconds)

  {
    int nfds;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    unsigned char next_char;
    memSizeType bytes_received;
    boolType result;

  /* socInputReady */
    logFunction(printf("socInputReady(%d, " FMT_D ", " FMT_D ")\n",
                       sock, seconds, micro_seconds););
    if (unlikely(seconds < 0 || seconds >= LONG_MAX ||
                 micro_seconds < 0 || micro_seconds >= 1000000)) {
      raise_error(RANGE_ERROR);
      result = FALSE;
    } else {
      FD_ZERO(&readfds);
      FD_SET((os_socketType) sock, &readfds);
      nfds = (int) sock + 1;
      timeout.tv_sec = (long int) seconds;
      timeout.tv_usec = (long int) micro_seconds;
      /* printf("select(%d, %d)\n", nfds, sock); */
      select_result = select(nfds, &readfds, NULL, NULL, &timeout);
      /* printf("select_result: %d\n", select_result); */
      if (unlikely(select_result < 0)) {
        logError(printf("socInputReady(%d): select(...) failed:\n"
                        "errno=%d\nerror: %s\n",
                        sock, ERROR_INFORMATION););
        raise_error(FILE_ERROR);
        result = FALSE;
      } else {
        result = FD_ISSET((os_socketType) sock, &readfds);
        if (result) {
          /* Verify that it is really possible to read at least one character */
          bytes_received = (memSizeType) recv((os_socketType) sock,
                                              cast_send_recv_data(&next_char), 1, MSG_PEEK);
          if (bytes_received != 1) {
            /* printf("socInputReady: bytes_received=%ld\n", (long int) bytes_received); */
            result = FALSE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socInputReady(%d, " FMT_D ", " FMT_D ") --> %d\n",
                       sock, seconds, micro_seconds, result););
    return result;
  } /* socInputReady */

#endif



#define BUFFER_START_SIZE 256
#define BUFFER_DELTA_SIZE 256

/**
 *  Read a line from 'inSocket'.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains '\n' or EOF.
 *  @return the line read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType socLineRead (socketType inSocket, charType *const terminationChar)

  {
    memSizeType bytes_received;
    memSizeType bytes_requested;
    memSizeType result_size;
    memSizeType old_result_size;
    memSizeType result_pos;
    ucharType *nlPos;
    ucharType buffer[BUFFER_SIZE];
    striType resized_result;
    striType result;

  /* socLineRead */
    logFunction(printf("socLineRead(%d, '\\" FMT_U32 ";')\n",
                       inSocket, *terminationChar););
    bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                        cast_send_recv_data(buffer),
                                        BUFFER_START_SIZE, MSG_PEEK);
    if (bytes_received == (memSizeType) (-1)) {
      bytes_received = 0;
    } /* if */
    if (bytes_received == 0) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = 0;
        *terminationChar = (charType) EOF;
      } /* if */
    } else {
      nlPos = (ucharType *) memchr(buffer, '\n', bytes_received);
      if (nlPos != NULL) {
        bytes_requested = (memSizeType) (nlPos - buffer) + 1;
        /* This should overwrite the buffer with identical data up to '\n'. */
        bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                            cast_send_recv_data(buffer),
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
          memcpy_to_strelem(result->mem, buffer, result_size);
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
          bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                              cast_send_recv_data(buffer),
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
            memcpy_to_strelem(&result->mem[result_pos], buffer, bytes_requested);
            result_pos += bytes_requested;
            bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                                cast_send_recv_data(buffer),
                                                BUFFER_DELTA_SIZE, MSG_PEEK);
            if (bytes_received == (memSizeType) (-1)) {
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
                *terminationChar = (charType) EOF;
              } /* if */
            } else {
              nlPos = (ucharType *) memchr(buffer, '\n', bytes_received);
            } /* if */
          } /* if */
        } while (result != NULL && bytes_received != 0 && nlPos == NULL);
        if (result != NULL && nlPos != NULL) {
          bytes_requested = (memSizeType) (nlPos - buffer) + 1;
          /* This should overwrite the buffer with identical data up to '\n'. */
          bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                              cast_send_recv_data(buffer),
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
            memcpy_to_strelem(&result->mem[result_pos], buffer, bytes_requested);
            result->size = result_size;
            *terminationChar = '\n';
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socLineRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       inSocket, *terminationChar, striAsUnquotedCStri(result)););
    return result;
  } /* socLineRead */



#ifdef OUT_OF_ORDER
striType socLineRead (socketType inSocket, charType *const terminationChar)

  {
    unsigned char ch;
    register memSizeType position;
    register memSizeType bytes_received;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* socLineRead */
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                          cast_send_recv_data(&ch), 1, 0);
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
        memory[position++] = (strElemType) ch;
        bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                            cast_send_recv_data(&ch), 1, 0);
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
          *terminationChar = (charType) EOF;
        } else {
          *terminationChar = (charType) ch;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socLineRead */
#endif



/**
 *  Listen for socket connections and limit the incoming queue.
 *  The 'backlog' argument defines the maximum length to which
 *  the queue of pending connections for 'listenerSocket' may grow.
 *  @exception FILE_ERROR A system function returns an error.
 */
void socListen (socketType listenerSocket, intType backlog)

  { /* socListen */
    logFunction(printf("socListen(%d, " FMT_D ")\n", listenerSocket, backlog););
    if (!inIntRange(backlog)) {
      raise_error(RANGE_ERROR);
    } else if (unlikely(listen((os_socketType) listenerSocket,
                               (int) backlog) != 0)) {
      logError(printf("socListen: listen(%d, " FMT_D ") failed:\n"
                      "errno=%d\nerror: %s\n",
                      listenerSocket, backlog, ERROR_INFORMATION););
      raise_error(FILE_ERROR);
    } /* if */
  } /* socListen */



intType socRecv (socketType sock, striType *stri, intType length, intType flags)

  {
    striType resized_stri;
    memSizeType bytes_requested;
    memSizeType old_stri_size;
    memSizeType new_stri_size;

  /* socRecv */
    if (unlikely(length < 0 || !inIntRange(flags))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memSizeType) length;
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
      new_stri_size = (memSizeType) recv((os_socketType) sock,
                                         cast_send_recv_data((*stri)->mem),
                                         cast_buffer_len(bytes_requested), (int) flags);
      memcpy_to_strelem((*stri)->mem, (ucharType *) (*stri)->mem, new_stri_size);
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
    return (intType) new_stri_size;
  } /* socRecv */



intType socRecvfrom (socketType sock, striType *stri, intType length, intType flags,
    bstriType *address)

  {
    striType resized_stri;
    memSizeType old_address_size;
    bstriType resized_address;
    memSizeType bytes_requested;
    sockLenType addrlen;
    memSizeType stri_size;

  /* socRecvfrom */
    if (unlikely(length < 0 || !inIntRange(flags))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memSizeType) length;
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
        stri_size = (memSizeType) recvfrom((os_socketType) sock,
                                           cast_send_recv_data((*stri)->mem),
                                           cast_buffer_len(bytes_requested), (int) flags,
                                           (struct sockaddr *) (*address)->mem, &addrlen);
        if (unlikely(stri_size == (memSizeType) -1 || addrlen < 0 || addrlen > MAX_ADDRESS_SIZE)) {
          REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, old_address_size);
          if (resized_address == NULL) {
            (*address)->size = MAX_ADDRESS_SIZE;
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, old_address_size);
          } /* if */
          logError(printf("socRecvfrom: recvfrom(%d, ...) failed:\n"
                          "errno=%d\nerror: %s\n",
                          sock, ERROR_INFORMATION););
          raise_error(FILE_ERROR);
        } else {
          REALLOC_BSTRI_SIZE_OK(resized_address, *address, MAX_ADDRESS_SIZE, (memSizeType) addrlen);
          if (unlikely(resized_address == NULL)) {
            (*address)->size = MAX_ADDRESS_SIZE;
            raise_error(MEMORY_ERROR);
            return 0;
          } else {
            *address = resized_address;
            COUNT3_BSTRI(MAX_ADDRESS_SIZE, (memSizeType) addrlen);
            (*address)->size = (memSizeType) addrlen;
          } /* if */
        } /* if */
      } /* if */
      memcpy_to_strelem((*stri)->mem, (ucharType *) (*stri)->mem, stri_size);
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
    return (intType) stri_size;
  } /* socRecvfrom */



intType socSend (socketType sock, const const_striType stri, intType flags)

  {
    bstriType buf;
    memSizeType bytes_sent;
    errInfoType err_info = OKAY_NO_ERROR;
    intType result;

  /* socSend */
    if (unlikely(!inIntRange(flags))) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      buf = stri_to_bstri(stri, &err_info);
      if (unlikely(buf == NULL)) {
        raise_error(err_info);
        result = 0;
      } else {
        bytes_sent = (memSizeType) send((os_socketType) sock,
                                        cast_send_recv_data(buf->mem),
                                        cast_buffer_len(buf->size), (int) flags);
        FREE_BSTRI(buf, buf->size);
        if (unlikely(bytes_sent == (memSizeType) -1)) {
          result = -1;
        } else if (unlikely(bytes_sent > MAX_MEM_INDEX)) {
          result = MAX_MEM_INDEX;
        } else {
          result = (intType) bytes_sent;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socSend */



intType socSendto (socketType sock, const const_striType stri, intType flags,
    const_bstriType address)

  {
    bstriType buf;
    memSizeType bytes_sent;
    errInfoType err_info = OKAY_NO_ERROR;
    intType result;

  /* socSendto */
    if (unlikely(!inIntRange(flags))) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      buf = stri_to_bstri(stri, &err_info);
      if (unlikely(buf == NULL)) {
        raise_error(err_info);
        result = 0;
      } else {
        bytes_sent = (memSizeType) sendto((os_socketType) sock,
                                          cast_send_recv_data(buf->mem),
                                          cast_buffer_len(buf->size), (int) flags,
                                          (const struct sockaddr *) address->mem,
                                          (sockLenType) address->size);
        FREE_BSTRI(buf, buf->size);
        if (unlikely(bytes_sent == (memSizeType) -1)) {
          result = -1;
        } else if (unlikely(bytes_sent > MAX_MEM_INDEX)) {
          result = MAX_MEM_INDEX;
        } else {
          result = (intType) bytes_sent;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* socSendto */



void socSetOptBool (socketType sock, intType optname, boolType optval)

  { /* socSetOptBool */
    logFunction(printf("socSetOptBool(%d, " FMT_D ", %s)\n",
                       sock, optname, optval ? "TRUE" : "FALSE"););
    switch (optname) {
      case SOC_OPT_NONE:
        break;
      case SOC_OPT_REUSEADDR: {
          int so_reuseaddr = optval;
          if (setsockopt((os_socketType) sock,
                         SOL_SOCKET, SO_REUSEADDR,
                         (const char *) &so_reuseaddr, sizeof(so_reuseaddr)) != 0) {
            logError(printf("socSetOptBool: setsockopt(%d, ...) failed:\n"
                            "errno=%d\nerror: %s\n",
                            sock, ERROR_INFORMATION););
            raise_error(FILE_ERROR);
          } /* if */
        }
        break;
      default:
        raise_error(RANGE_ERROR);
        break;
    } /* switch */
  } /* socSetOptBool */



socketType socSocket (intType domain, intType type, intType protocol)

  {
    os_socketType result;

  /* socSocket */
    logFunction(printf("socSocket(" FMT_D ", " FMT_D ", " FMT_D ")\n",
                       domain, type, protocol););
    if (!inIntRange(domain) || !inIntRange(type) || !inIntRange(protocol)) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      /* printf("socSocket(%d, %d, %d)\n", domain, type, protocol); */
      check_initialization((socketType) -1);
      result = (os_socketType) socket((int) domain, (int) type, (int) protocol);
#if defined USE_WINSOCK && !defined TWOS_COMPLEMENT_INTTYPE
      /* In this case INVALID_SOCKET != (socketType) -1 holds and    */
      /* (socketType) -1 must be returned instead of INVALID_SOCKET. */
      /* Probably a computer, which needs this, does not exist.      */
      if (unlikely(result == INVALID_SOCKET)) {
        result = (os_socketType) -1;
      } /* if */
#endif
    } /* if */
    logFunction(printf("socSocket(" FMT_D ", " FMT_D ", " FMT_D ") --> %d\n",
                       domain, type, protocol, result););
    return (socketType) result;
  } /* socSocket */



/**
 *  Read a word from 'inSocket'.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains ' ', '\t', '\n' or
 *  EOF.
 *  @return the word read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType socWordRead (socketType inSocket, charType *const terminationChar)

  {
    unsigned char ch;
    register memSizeType position;
    register memSizeType bytes_received;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* socWordRead */
    logFunction(printf("socWordRead(%d, '\\" FMT_U32 ";')\n",
                       inSocket, *terminationChar););
    memlength = READ_STRI_INIT_SIZE;
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, memlength))) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      do {
        bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                            cast_send_recv_data(&ch), 1, 0);
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
        memory[position++] = (strElemType) ch;
        bytes_received = (memSizeType) recv((os_socketType) inSocket,
                                            cast_send_recv_data(&ch), 1, 0);
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
          *terminationChar = (charType) EOF;
        } else {
          *terminationChar = (charType) ch;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("socWordRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       inSocket, *terminationChar, striAsUnquotedCStri(result)););
    return result;
  } /* socWordRead */



/**
 *  Write a string to 'outSocket'.
 *  @exception FILE_ERROR The system function is not able to write
 *             all characters of the string.
 *  @exception RANGE_ERROR The string contains a character that does
 *             not fit into a byte.
 */
void socWrite (socketType outSocket, const const_striType stri)

  {
    ucharType buffer[BUFFER_SIZE];
    ustriType buf;
    memSizeType bytes_to_send;
    memSizeType bytes_sent;
    memSizeType totally_sent = 0;
    errInfoType err_info = OKAY_NO_ERROR;
    bstriType bstri = NULL;

  /* socWrite */
    logFunction(printf("socWrite(%d, \"%s\")\n",
                       outSocket, striAsUnquotedCStri(stri)););
    if (stri->size <= BUFFER_SIZE) {
      if (unlikely(memcpy_from_strelem(buffer, stri->mem, stri->size))) {
        logError(printf("socWrite(%d, \"%s\"): "
                        "One of the characters does not fit into a byte.\n",
                        outSocket, striAsUnquotedCStri(stri)););
        err_info = RANGE_ERROR;
        buf = NULL;
        bytes_to_send = 0;
      } else {
        buf = buffer;
        bytes_to_send = stri->size;
      } /* if */
    } else {
      bstri = stri_to_bstri(stri, &err_info);
      if (unlikely(bstri == NULL)) {
        logError(printf("socWrite(%d, \"%s\"): "
                        "Failed to create a temporary bstring.\n",
                        outSocket, striAsUnquotedCStri(stri)););
        buf = NULL;
        bytes_to_send = 0;
      } else {
        buf = bstri->mem;
        bytes_to_send = bstri->size;
      } /* if */
    } /* if */
    while (bytes_to_send != 0 && err_info == OKAY_NO_ERROR) {
      bytes_sent = (memSizeType) send((os_socketType) outSocket,
                                      cast_send_recv_data(&buf[totally_sent]),
                                      cast_buffer_len(bytes_to_send), 0);
      if (bytes_sent == (memSizeType) -1) {
        logError(printf("socWrite: send(%d, data, " FMT_U_MEM ") failed:\n"
                        "errno=%d\nerror: %s\n",
                        outSocket, bytes_to_send, ERROR_INFORMATION););
        err_info = FILE_ERROR;
      } else {
        totally_sent += bytes_sent;
        bytes_to_send -= bytes_sent;
      } /* if */
    } /* while */
    if (bstri != NULL) {
      FREE_BSTRI(bstri, bstri->size);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* socWrite */
