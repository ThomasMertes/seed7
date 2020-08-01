/********************************************************************/
/*                                                                  */
/*  heaputl.h     Procedures for heap allocation and maintainance.  */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/heaputl.h                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Procedures for heap allocation and maintainance.       */
/*                                                                  */
/********************************************************************/

#ifdef DO_HEAP_STATISTIK
typedef struct {
    unsigned long stri;
    memsizetype stri_elems;
    unsigned long bstri;
    memsizetype bstri_elems;
    unsigned long array;
    memsizetype arr_elems;
    unsigned long hash;
    memsizetype hsh_elems;
    unsigned long helem;
    unsigned long set;
    memsizetype set_elems;
    unsigned long stru;
    memsizetype sct_elems;
    unsigned long big;
    memsizetype big_elems;
    unsigned long ident;
    unsigned long idt;
    memsizetype idt_bytes;
    unsigned long entity;
    unsigned long object;
    unsigned long node;
    unsigned long token;
    unsigned long owner;
    unsigned long stack;
    unsigned long type;
    unsigned long list_elem;
    unsigned long block;
    unsigned long loclist;
    unsigned long infil;
    unsigned long prog;
    unsigned long fnam;
    memsizetype fnam_bytes;
    unsigned long symb;
    memsizetype symb_bytes;
    memsizetype byte;
  } counttype;

#ifdef DO_INIT
counttype count = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
EXTERN counttype count;
#endif
#endif


/* Some stupid operating systems have under some compilers      */
/* limitations on how much memory can be requested with malloc. */
/* This is the case when the malloc function has an 16 bit      */
/* unsigned argument. If this is the case the only solution     */
/* is to check if the SIZE is under 2**16 - 1 . Here we use     */
/* a little bit lower value to be on the save side. Note        */
/* that if such an operating system has another function to     */
/* request more bytes it can be substituted here.               */

#ifdef USE_MAXIMUM_MALLOC_CHECK
#define MALLOC(size) ((size) <= 65500 ? malloc((SIZE_TYPE) (size)) : NULL)
#define REALLOC(ptr, size) ((size) <= 65500 ? realloc(ptr, (SIZE_TYPE) (size)) : NULL)
#else
#ifdef OUT_OF_ORDER
#ifndef DO_INIT
EXTERN int successful_mallocs;
#endif
#define MALLOC(size) (printf("%s(%d) %d\n", __FILE__, __LINE__, successful_mallocs), (successful_mallocs-- != 0 ? malloc((SIZE_TYPE) (size)) : NULL))
#endif
#define MALLOC(size) malloc((SIZE_TYPE) (size))
#define REALLOC(ptr, size) realloc(ptr, (SIZE_TYPE) (size))
#endif


#ifdef USE_CHUNK_ALLOCS
typedef struct {
    char *freemem;
    char *start;
    char *beyond;
    unsigned int size;
    memsizetype total_size;
    memsizetype lost_bytes;
    unsigned int number_of_chunks;
  } chunktype;

#ifdef DO_INIT
chunktype chunk = {NULL, NULL, NULL, 0, 0, 0, 0};
#else
EXTERN chunktype chunk;
#endif
#endif


#ifdef DO_HEAPSIZE_COMPUTATION
#ifdef DO_INIT
memsizetype hs = 0;
#else
EXTERN memsizetype hs;
#endif
#define HS_ADD(size)    hs += (long) (size)
#define HS_SUB(size)    hs -= (long) (size)
#else
#define HS_ADD(size)
#define HS_SUB(size)
#endif


#ifdef DO_HEAP_STATISTIK
#define USTRI_ADD(len,cnt,byt) , cnt++, byt += (memsizetype) (len)
#define USTRI_SUB(len,cnt,byt) , cnt--, byt -= (memsizetype) (len)
#define STRI_ADD(len)          , count.stri++,  count.stri_elems += (memsizetype) (len)
#define STRI_SUB(len)          , count.stri--,  count.stri_elems -= (memsizetype) (len)
#define BSTRI_ADD(len)         , count.bstri++, count.bstri_elems += (memsizetype) (len)
#define BSTRI_SUB(len)         , count.bstri--, count.bstri_elems -= (memsizetype) (len)
#define ARR_ADD(len)           , count.array++, count.arr_elems += (memsizetype) (len)
#define ARR_SUB(len)           , count.array--, count.arr_elems -= (memsizetype) (len)
#define HSH_ADD(len)           , count.hash++,  count.hsh_elems += (memsizetype) (len)
#define HSH_SUB(len)           , count.hash--,  count.hsh_elems -= (memsizetype) (len)
#define SET_ADD(len)           , count.set++,   count.set_elems += (memsizetype) (len)
#define SET_SUB(len)           , count.set--,   count.set_elems -= (memsizetype) (len)
#define SCT_ADD(len)           , count.stru++,  count.sct_elems += (memsizetype) (len)
#define SCT_SUB(len)           , count.stru--,  count.sct_elems -= (memsizetype) (len)
#define BIG_ADD(len)           , count.big++,   count.big_elems += (memsizetype) (len)
#define BIG_SUB(len)           , count.big--,   count.big_elems -= (memsizetype) (len)
#define REC_ADD(cnt)           , cnt++
#define REC_SUB(cnt)           , cnt--
#define BYT_ADD(size)          , count.byte += (memsizetype) (size)
#define BYT_SUB(size)          , count.byte -= (memsizetype) (size)
#else
#define USTRI_ADD(len,cnt,byt)
#define USTRI_SUB(len,cnt,byt)
#define STRI_ADD(len)
#define STRI_SUB(len)
#define BSTRI_ADD(len)
#define BSTRI_SUB(len)
#define ARR_ADD(len)
#define ARR_SUB(len)
#define HSH_ADD(len)
#define HSH_SUB(len)
#define SET_ADD(len)
#define SET_SUB(len)
#define SCT_ADD(len)
#define SCT_SUB(len)
#define BIG_ADD(len)
#define BIG_SUB(len)
#define REC_ADD(cnt)
#define REC_SUB(cnt)
#define BYT_ADD(size)
#define BYT_SUB(size)
#endif


#ifdef DO_HEAP_CHECK
#define H_CHECK1(len)   , check_heap(  (long) (len))
#define H_CHECK2(len)   , check_heap(- (long) (len))
#else
#define H_CHECK1(len)
#define H_CHECK2(len)
#endif


#ifdef DO_HEAP_LOG
#define H_LOG1(len)     , printf("\nmalloc(%ld)\n", (long) (len)) H_CHECK1(len)
#define H_LOG2(len)     , printf("\nmfree(%ld)\n", (long) (len))  H_CHECK2(len)
#else
#define H_LOG1(len)     H_CHECK1(len)
#define H_LOG2(len)     H_CHECK2(len)
#endif


#define SIZ_USTRI(len)  ((len) + 1)
#define SIZ_CSTRI(len)  ((len) + 1)
#define SIZ_ID(len)     ((((len) >> 3) + 1) << 3)
#define SIZ_STRI(len)   (sizeof(strirecord)   - sizeof(strelemtype)  + (len) * sizeof(strelemtype))
#define SIZ_BSTRI(len)  (sizeof(bstrirecord)  - sizeof(uchartype)    + (len) * sizeof(uchartype))
#define SIZ_ARR(len)    (sizeof(arrayrecord)  - sizeof(objectrecord) + (len) * sizeof(objectrecord))
#define SIZ_HSH(len)    (sizeof(hashrecord)   - sizeof(helemtype)    + (len) * sizeof(helemtype))
#define SIZ_SET(len)    (sizeof(setrecord)    - sizeof(bitsettype)   + (len) * sizeof(bitsettype))
#define SIZ_SCT(len)    (sizeof(structrecord) - sizeof(objectrecord) + (len) * sizeof(objectrecord))
#define SIZ_BIG(len)    (sizeof(bigintrecord) - sizeof(bigdigittype) + (len) * sizeof(bigdigittype))
#define SIZ_REC(rec)    (sizeof(rec))
#define SIZ_TAB(tp, nr) (sizeof(tp) * (nr))


#ifdef DO_HEAPSIZE_COMPUTATION
#define CNT1_USTRI(L,S,C,B)  (HS_ADD(S)    USTRI_ADD(L,C,B) H_LOG1(S))
#define CNT2_USTRI(L,S,C,B)  (HS_SUB(S)    USTRI_SUB(L,C,B) H_LOG2(S))
#define CNT1_STRI(len,size)  (HS_ADD(size) STRI_ADD(len)    H_LOG1(size))
#define CNT2_STRI(len,size)  (HS_SUB(size) STRI_SUB(len)    H_LOG2(size))
#define CNT1_BSTRI(len,size) (HS_ADD(size) BSTRI_ADD(len)   H_LOG1(size))
#define CNT2_BSTRI(len,size) (HS_SUB(size) BSTRI_SUB(len)   H_LOG2(size))
#define CNT1_ARR(len,size)   (HS_ADD(size) ARR_ADD(len)     H_LOG1(size))
#define CNT2_ARR(len,size)   (HS_SUB(size) ARR_SUB(len)     H_LOG2(size))
#define CNT1_HSH(len,size)   (HS_ADD(size) HSH_ADD(len)     H_LOG1(size))
#define CNT2_HSH(len,size)   (HS_SUB(size) HSH_SUB(len)     H_LOG2(size))
#define CNT1_SET(len,size)   (HS_ADD(size) SET_ADD(len)     H_LOG1(size))
#define CNT2_SET(len,size)   (HS_SUB(size) SET_SUB(len)     H_LOG2(size))
#define CNT1_SCT(len,size)   (HS_ADD(size) SCT_ADD(len)     H_LOG1(size))
#define CNT2_SCT(len,size)   (HS_SUB(size) SCT_SUB(len)     H_LOG2(size))
#define CNT1_BIG(len,size)   (HS_ADD(size) BIG_ADD(len)     H_LOG1(size))
#define CNT2_BIG(len,size)   (HS_SUB(size) BIG_SUB(len)     H_LOG2(size))
#define CNT1_REC(size,cnt)   (HS_ADD(size) REC_ADD(cnt)     H_LOG1(size))
#define CNT2_REC(size,cnt)   (HS_SUB(size) REC_SUB(cnt)     H_LOG2(size))
#define CNT1_BYT(size)       (HS_ADD(size) BYT_ADD(size)    H_LOG1(size))
#define CNT2_BYT(size)       (HS_SUB(size) BYT_SUB(size)    H_LOG2(size))
#else
#define CNT1_USTRI(L,S,C,B)  0
#define CNT2_USTRI(L,S,C,B)  0
#define CNT1_STRI(len,size)  0
#define CNT2_STRI(len,size)  0
#define CNT1_BSTRI(len,size) 0
#define CNT2_BSTRI(len,size) 0
#define CNT1_ARR(len,size)   0
#define CNT2_ARR(len,size)   0
#define CNT1_HSH(len,size)   0
#define CNT2_HSH(len,size)   0
#define CNT1_SET(len,size)   0
#define CNT2_SET(len,size)   0
#define CNT1_SCT(len,size)   0
#define CNT2_SCT(len,size)   0
#define CNT1_BIG(len,size)   0
#define CNT2_BIG(len,size)   0
#define CNT1_REC(size,cnt)   0
#define CNT2_REC(size,cnt)   0
#define CNT1_BYT(size)       0
#define CNT2_BYT(size)       0
#endif


#define ALLOC_HEAP(var,tp,byt)     ((var = (tp) MALLOC(byt)) != NULL)
#define REALLOC_HEAP(var,tp,byt)   ((var = (tp) REALLOC(var, byt)) != NULL)
#define FREE_HEAP(var,byt)         (free((void *) var))


#define ALLOC_USTRI(var,len)       ALLOC_HEAP(var, ustritype, SIZ_USTRI(len))
#define UNALLOC_USTRI(var,len)     FREE_HEAP(var, SIZ_USTRI(len))
#define FREE_USTRI(var,L,cnt,byt)  (CNT2_USTRI(L, SIZ_USTRI(L), cnt, byt), FREE_HEAP(var, byt))
#define RESIZE_USTRI(var,L1,L2)    REALLOC_HEAP(var, ustritype, SIZ_USTRI(L2))
#define COUNT_USTRI(len,cnt,byt)   CNT1_USTRI(len, SIZ_USTRI(len), cnt, byt)
#define COUNT3_USTRI(L1,L2,CT,byt) (CNT2_USTRI(L1, SIZ_USTRI(L1), CT, byt), CNT1_USTRI(L2, SIZ_USTRI(L2), CT, byt))


#define ALLOC_CSTRI(var,len)       ALLOC_HEAP(var, cstritype, SIZ_CSTRI(len))
#define UNALLOC_CSTRI(var,len)     FREE_HEAP(var, SIZ_CSTRI(len))


#ifndef WITH_STRI_FLIST
#ifdef MMAP_ABLE_STRI
#define ALLOC_STRI(var,len)        (ALLOC_HEAP(var, stritype, SIZ_STRI(len)) ? (var->mem = var->mem1, TRUE) : FALSE)
#define FREE_STRI(var,len)         (var->mem == var->mem1 ? (CNT2_STRI(len, SIZ_STRI(len)), FREE_HEAP(var, byt)) : void)
#define RESIZE_STRI(var,len1,len2) (REALLOC_HEAP(var, stritype, SIZ_STRI(len2)) ? (var->mem = var->mem1, TRUE) : FALSE)
#else
#define ALLOC_STRI(var,len)        ALLOC_HEAP(var, stritype, SIZ_STRI(len))
#define FREE_STRI(var,len)         (CNT2_STRI(len, SIZ_STRI(len)), FREE_HEAP(var, byt))
#define RESIZE_STRI(var,len1,len2) REALLOC_HEAP(var, stritype, SIZ_STRI(len2))
#endif
#endif
#define COUNT_STRI(len)            CNT1_STRI(len, SIZ_STRI(len))
#define COUNT3_STRI(len1,len2)     (CNT2_STRI(len1, SIZ_STRI(len1)), CNT1_STRI(len2, SIZ_STRI(len2)))


#ifdef MMAP_ABLE_BSTRI
#define ALLOC_BSTRI(var,len)       (ALLOC_HEAP(var, bstritype, SIZ_BSTRI(len)) ? (var->mem = var->mem1, TRUE) : FALSE)
#define FREE_BSTRI(var,len)        (var->mem == var->mem1 ? (CNT2_BSTRI(len, SIZ_BSTRI(len)), FREE_HEAP(var, byt)) : void)
#define RESIZE_BSTRI(var,ln1,ln2)  (REALLOC_HEAP(var, bstritype, SIZ_BSTRI(ln2)) ? (var->mem = var->mem1, TRUE) : FALSE)
#else
#define ALLOC_BSTRI(var,len)       ALLOC_HEAP(var, bstritype, SIZ_BSTRI(len))
#define FREE_BSTRI(var,len)        (CNT2_BSTRI(len, SIZ_BSTRI(len)), FREE_HEAP(var, byt))
#define RESIZE_BSTRI(var,ln1,ln2)  REALLOC_HEAP(var, bstritype, SIZ_BSTRI(ln2))
#endif
#define COUNT_BSTRI(len)           CNT1_BSTRI(len, SIZ_BSTRI(len))
#define COUNT3_BSTRI(len1,len2)    (CNT2_BSTRI(len1, SIZ_BSTRI(len1)), CNT1_BSTRI(len2, SIZ_BSTRI(len2)))


#define ALLOC_ARRAY(var,len)       ALLOC_HEAP(var, arraytype, SIZ_ARR(len))
#define FREE_ARRAY(var,len)        (CNT2_ARR(len, SIZ_ARR(len)), FREE_HEAP(var, byt))
#define RESIZE_ARRAY(var,ln1,ln2)  REALLOC_HEAP(var, arraytype, SIZ_ARR(ln2))
#define COUNT_ARRAY(len)           CNT1_ARR(len, SIZ_ARR(len))
#define COUNT3_ARRAY(len1,len2)    (CNT2_ARR(len1, SIZ_ARR(len1)), CNT1_ARR(len2, SIZ_ARR(len2)))


#define ALLOC_HASH(var,len)        ALLOC_HEAP(var, hashtype, SIZ_HSH(len))
#define FREE_HASH(var,len)         (CNT2_HSH(len, SIZ_HSH(len)), FREE_HEAP(var, byt))
#define RESIZE_HASH(var,ln1,ln2)   REALLOC_HEAP(var, hashtype, SIZ_HSH(ln2))
#define COUNT_HASH(len)            CNT1_HSH(len, SIZ_HSH(len))
#define COUNT3_HASH(len1,len2)     (CNT2_HSH(len1, SIZ_HSH(len1)), CNT1_HSH(len2, SIZ_HSH(len2)))


#define ALLOC_SET(var,len)         ALLOC_HEAP(var, settype, SIZ_SET(len))
#define FREE_SET(var,len)          (CNT2_SET(len, SIZ_SET(len)), FREE_HEAP(var, byt))
#define RESIZE_SET(var,len1,len2)  REALLOC_HEAP(var, settype, SIZ_SET(len2))
#define COUNT_SET(len)             CNT1_SET(len, SIZ_SET(len))
#define COUNT3_SET(len1,len2)      (CNT2_SET(len1, SIZ_SET(len1)), CNT1_SET(len2, SIZ_SET(len2)))


#define ALLOC_STRUCT(var,len)      ALLOC_HEAP(var, structtype, SIZ_SCT(len))
#define FREE_STRUCT(var,len)       (CNT2_SCT(len, SIZ_SCT(len)), FREE_HEAP(var, byt))
#define RESIZE_STRUCT(var,ln1,ln2) REALLOC_HEAP(var, structtype, SIZ_SCT(ln2))
#define COUNT_STRUCT(len)          CNT1_SCT(len, SIZ_SCT(len))
#define COUNT3_STRUCT(len1,len2)   (CNT2_SCT(len1, SIZ_SCT(len1)), CNT1_SCT(len2, SIZ_SCT(len2)))


#define ALLOC_BIG(var,len)         ALLOC_HEAP(var, biginttype, SIZ_BIG(len))
#define FREE_BIG(var,len)          (CNT2_BIG(len, SIZ_BIG(len)), FREE_HEAP(var, byt))
#define RESIZE_BIG(var,ln1,ln2)    REALLOC_HEAP(var, biginttype, SIZ_BIG(ln2))
#define COUNT_BIG(len)             CNT1_BIG(len, SIZ_BIG(len))
#define COUNT3_BIG(len1,len2)      (CNT2_BIG(len1, SIZ_BIG(len1)), CNT1_BIG(len2, SIZ_BIG(len2)))


#define ALLOC_RECORD(var,rec)      ALLOC_HEAP(var, rec *, SIZ_REC(rec))
#define FREE_RECORD(var,rec,cnt)   (CNT2_REC(SIZ_REC(rec), cnt), FREE_HEAP(var, SIZ_REC(rec)))
#define COUNT_RECORD(rec,cnt)      CNT1_REC(SIZ_REC(rec), cnt)


#define ALLOC_BYTES(var,byt)       ALLOC_HEAP(var, char *, byt)
#define FREE_BYTES(var,byt)        (CNT2_BYT(byt), FREE_HEAP(var, byt))
#define COUNT_BYTES(byt)           CNT1_BYT(byt)


#define ALLOC_TABLE(var,tp,nr)     ALLOC_HEAP(var, tp *, SIZ_TAB(tp, nr))
#define RESIZE_TABLE(var,tp,N1,N2) REALLOC_HEAP(var, tp *, SIZ_TAB(tp, N2))
#define COUNT_TABLE(tp,nr)         CNT1_BYT(SIZ_TAB(tp, nr))
#define COUNT3_TABLE(tp,nr1,nr2)   (CNT2_BYT(SIZ_TAB(tp, nr1)), CNT1_BYT(SIZ_TAB(tp, nr2)))


#ifdef ANSI_C

#ifdef DO_HEAP_CHECK
void check_heap (long);
#endif

#else

#ifdef DO_HEAP_CHECK
void check_heap ();
#endif

#endif
