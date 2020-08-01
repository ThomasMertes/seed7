/********************************************************************/
/*                                                                  */
/*  heaputl.h     Procedures for heap allocation and maintenance.   */
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
/*  File: seed7/src/heaputl.h                                       */
/*  Changes: 1992 - 1994, 2008, 2010, 2011  Thomas Mertes           */
/*  Content: Procedures for heap allocation and maintenance.        */
/*                                                                  */
/********************************************************************/


#ifdef DO_HEAP_STATISTIC
typedef struct {
    unsigned long stri;
    memSizeType stri_elems;
    unsigned long bstri;
    memSizeType bstri_elems;
    unsigned long array;
    memSizeType arr_elems;
    memSizeType rtl_arr_elems;
    unsigned long hash;
    memSizeType hsh_elems;
    unsigned long helem;
    unsigned long rtl_helem;
    unsigned long set;
    memSizeType set_elems;
    unsigned long stru;
    memSizeType sct_elems;
    unsigned long big;
    memSizeType big_elems;
    unsigned long ident;
    unsigned long idt;
    memSizeType idt_bytes;
    unsigned long entity;
    unsigned long property;
    unsigned long object;
    unsigned long node;
    unsigned long token;
    unsigned long owner;
    unsigned long stack;
    unsigned long typelist_elems;
    unsigned long type;
    unsigned long list_elem;
    unsigned long block;
    unsigned long loclist;
    unsigned long infil;
    unsigned long prog;
    unsigned long polldata;
    unsigned long prepared_stmt;
    unsigned long win;
    unsigned long process;
    unsigned long fnam;
    memSizeType fnam_bytes;
    unsigned long symb;
    memSizeType symb_bytes;
    memSizeType byte;
  } countType;

#ifdef DO_INIT
countType count = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0};
#else
EXTERN countType count;
#endif

#ifdef USE_BIG_RTL_LIBRARY
extern size_t sizeof_bigDigitType;
extern size_t sizeof_bigIntRecord;
#endif
extern size_t sizeof_pollRecord;
extern size_t sizeof_winRecord;

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
#define MALLOC(size) ((size) <= 65500 ? malloc((size_t) (size)) : NULL)
#define REALLOC(ptr, size) ((size) <= 65500 ? realloc(ptr, (size_t) (size)) : NULL)
#else
#ifdef OUT_OF_ORDER
#ifndef DO_INIT
EXTERN int successful_mallocs;
#endif
#define MALLOC(size) (printf("%s(%d) %d\n", __FILE__, __LINE__, successful_mallocs), (successful_mallocs-- != 0 ? malloc((size_t) (size)) : NULL))
#endif
#define MALLOC(size) malloc((size_t) (size))
#define REALLOC(ptr, size) realloc(ptr, (size_t) (size))
#endif


#ifdef USE_CHUNK_ALLOCS
typedef struct {
    char *freemem;
    char *start;
    char *beyond;
    memSizeType size;
    memSizeType total_size;
    memSizeType lost_bytes;
    unsigned int number_of_chunks;
  } chunkType;

#ifdef DO_INIT
chunkType chunk = {NULL, NULL, NULL, 0, 0, 0, 0};
#else
EXTERN chunkType chunk;
#endif
#endif


#ifdef DO_HEAPSIZE_COMPUTATION
#ifdef DO_INIT
memSizeType hs = 0;
#else
EXTERN memSizeType hs;
#endif
#define HS_ADD(size)    hs += (memSizeType) (size)
#define HS_SUB(size)    hs -= (memSizeType) (size)
#else
#define HS_ADD(size)
#define HS_SUB(size)
#endif


#ifdef DO_HEAP_STATISTIC
#define USTRI_ADD(len,cnt,byt) cnt++, byt += (memSizeType) (len)
#define USTRI_SUB(len,cnt,byt) cnt--, byt -= (memSizeType) (len)
#define STRI_ADD(len)          count.stri++,  count.stri_elems += (memSizeType) (len)
#define STRI_SUB(len)          count.stri--,  count.stri_elems -= (memSizeType) (len)
#define BSTRI_ADD(len)         count.bstri++, count.bstri_elems += (memSizeType) (len)
#define BSTRI_SUB(len)         count.bstri--, count.bstri_elems -= (memSizeType) (len)
#define ARR_ADD(len)           count.array++, count.arr_elems += (memSizeType) (len)
#define ARR_SUB(len)           count.array--, count.arr_elems -= (memSizeType) (len)
#define HSH_ADD(len)           count.hash++,  count.hsh_elems += (memSizeType) (len)
#define HSH_SUB(len)           count.hash--,  count.hsh_elems -= (memSizeType) (len)
#define SET_ADD(len)           count.set++,   count.set_elems += (memSizeType) (len)
#define SET_SUB(len)           count.set--,   count.set_elems -= (memSizeType) (len)
#define SCT_ADD(len)           count.stru++,  count.sct_elems += (memSizeType) (len)
#define SCT_SUB(len)           count.stru--,  count.sct_elems -= (memSizeType) (len)
#define BIG_ADD(len)           count.big++,   count.big_elems += (memSizeType) (len)
#define BIG_SUB(len)           count.big--,   count.big_elems -= (memSizeType) (len)
#define REC_ADD(cnt)           cnt++
#define REC_SUB(cnt)           cnt--
#define BYT_ADD(size)          count.byte += (memSizeType) (size)
#define BYT_SUB(size)          count.byte -= (memSizeType) (size)
#define RTL_L_ELEM_ADD
#define RTL_L_ELEM_SUB
#define RTL_ARR_ADD(len)       count.array++, count.rtl_arr_elems += (memSizeType) (len)
#define RTL_ARR_SUB(len)       count.array--, count.rtl_arr_elems -= (memSizeType) (len)
#define RTL_HSH_ADD(len)       count.hash++,  count.hsh_elems += (memSizeType) (len)
#define RTL_HSH_SUB(len)       count.hash--,  count.hsh_elems -= (memSizeType) (len)
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
#define RTL_L_ELEM_ADD
#define RTL_L_ELEM_SUB
#define RTL_ARR_ADD(len)
#define RTL_ARR_SUB(len)
#define RTL_HSH_ADD(len)
#define RTL_HSH_SUB(len)
#endif


#ifdef DO_HEAP_CHECK
#define H_CHECK1(len)   , check_heap(  (long) (len), __FILE__, __LINE__)
#define H_CHECK2(len)   , check_heap(- (long) (len), __FILE__, __LINE__)
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


#define SIZ_USTRI(len)   ((len) + 1)
#define SIZ_CSTRI(len)   ((len) + 1)
#define SIZ_WSTRI(len)   (sizeof(wcharType) * ((len) + 1))
#define SIZ_STRI(len)    ((sizeof(striRecord)     - sizeof(strElemType))  + (len) * sizeof(strElemType))
#define SIZ_BSTRI(len)   ((sizeof(bstriRecord)    - sizeof(ucharType))    + (len) * sizeof(ucharType))
#define SIZ_ARR(len)     ((sizeof(arrayRecord)    - sizeof(objectRecord)) + (len) * sizeof(objectRecord))
#define SIZ_HSH(len)     ((sizeof(hashRecord)     - sizeof(hashElemType)) + (len) * sizeof(hashElemType))
#define SIZ_SET(len)     ((sizeof(setRecord)      - sizeof(bitSetType))   + (len) * sizeof(bitSetType))
#define SIZ_SCT(len)     ((sizeof(structRecord)   - sizeof(objectRecord)) + (len) * sizeof(objectRecord))
#define SIZ_BIG(len)     ((sizeof_bigIntRecord    - sizeof_bigDigitType)  + (len) * sizeof_bigDigitType)
#define SIZ_REC(rec)     (sizeof(rec))
#define SIZ_TAB(tp, nr)  (sizeof(tp) * (nr))
#define SIZ_RTL_L_ELEM   (sizeof(rtlListRecord))
#define SIZ_RTL_ARR(len) ((sizeof(rtlArrayRecord) - sizeof(rtlObjectType))   + (len) * sizeof(rtlObjectType))
#define SIZ_RTL_HSH(len) ((sizeof(rtlHashRecord)  - sizeof(rtlHashElemType)) + (len) * sizeof(rtlHashElemType))

#define MAX_USTRI_LEN   (MAX_MEMSIZETYPE - 1)
#define MAX_CSTRI_LEN   (MAX_MEMSIZETYPE - 1)
#define MAX_WSTRI_LEN   (MAX_MEMSIZETYPE / sizeof(wcharType) - 1)
#define MAX_STRI_LEN    ((MAX_MEMSIZETYPE - sizeof(striRecord)     + sizeof(strElemType))   / sizeof(strElemType))
#define MAX_BSTRI_LEN   ((MAX_MEMSIZETYPE - sizeof(bstriRecord)    + sizeof(ucharType))     / sizeof(ucharType))
#define MAX_ARR_LEN     ((MAX_MEMSIZETYPE - sizeof(arrayRecord)    + sizeof(objectRecord))  / sizeof(objectRecord))
#define MAX_SET_LEN     ((MAX_MEMSIZETYPE - sizeof(setRecord)      + sizeof(bitSetType))    / sizeof(bitSetType))
#define MAX_RTL_ARR_LEN ((MAX_MEMSIZETYPE - sizeof(rtlArrayRecord) + sizeof(rtlObjectType)) / sizeof(rtlObjectType))

#ifdef DO_HEAPSIZE_COMPUTATION
#ifdef DO_HEAP_STATISTIC
#define CALC_HS(cnt_hs, cnt)   (cnt_hs, cnt)
#define CNT4_STRI(var,len)     HS_ADD(((len)-(var)->size) * sizeof(strElemType)); count.stri_elems += (memSizeType) ((len)-(var)->size);
#else
#define CALC_HS(cnt_hs, cnt)   (cnt_hs)
#define CNT4_STRI(var,len)     HS_ADD(((len)-(var)->size) * sizeof(strElemType));
#endif
#define CNT(cnt)               cnt,
#define CNT3(cnt2, cnt1)       (cnt2, cnt1)
#else
#define CALC_HS(cnt_hs, cnt)
#define CNT(cnt)
#define CNT3(cnt2, cnt1)
#define CNT4_STRI(var,len)
#endif

#define CNT1_USTRI(L,S,C,B)    CALC_HS(HS_ADD(S),    USTRI_ADD(L,C,B) H_LOG1(S))
#define CNT2_USTRI(L,S,C,B)    CALC_HS(HS_SUB(S),    USTRI_SUB(L,C,B) H_LOG2(S))
#define CNT1_STRI(len,size)    CALC_HS(HS_ADD(size), STRI_ADD(len)    H_LOG1(size))
#define CNT2_STRI(len,size)    CALC_HS(HS_SUB(size), STRI_SUB(len)    H_LOG2(size))
#define CNT1_BSTRI(len,size)   CALC_HS(HS_ADD(size), BSTRI_ADD(len)   H_LOG1(size))
#define CNT2_BSTRI(len,size)   CALC_HS(HS_SUB(size), BSTRI_SUB(len)   H_LOG2(size))
#define CNT1_ARR(len,size)     CALC_HS(HS_ADD(size), ARR_ADD(len)     H_LOG1(size))
#define CNT2_ARR(len,size)     CALC_HS(HS_SUB(size), ARR_SUB(len)     H_LOG2(size))
#define CNT1_HSH(len,size)     CALC_HS(HS_ADD(size), HSH_ADD(len)     H_LOG1(size))
#define CNT2_HSH(len,size)     CALC_HS(HS_SUB(size), HSH_SUB(len)     H_LOG2(size))
#define CNT1_SET(len,size)     CALC_HS(HS_ADD(size), SET_ADD(len)     H_LOG1(size))
#define CNT2_SET(len,size)     CALC_HS(HS_SUB(size), SET_SUB(len)     H_LOG2(size))
#define CNT1_SCT(len,size)     CALC_HS(HS_ADD(size), SCT_ADD(len)     H_LOG1(size))
#define CNT2_SCT(len,size)     CALC_HS(HS_SUB(size), SCT_SUB(len)     H_LOG2(size))
#define CNT1_BIG(len,size)     CALC_HS(HS_ADD(size), BIG_ADD(len)     H_LOG1(size))
#define CNT2_BIG(len,size)     CALC_HS(HS_SUB(size), BIG_SUB(len)     H_LOG2(size))
#define CNT1_REC(size,cnt)     CALC_HS(HS_ADD(size), REC_ADD(cnt)     H_LOG1(size))
#define CNT2_REC(size,cnt)     CALC_HS(HS_SUB(size), REC_SUB(cnt)     H_LOG2(size))
#define CNT1_BYT(size)         CALC_HS(HS_ADD(size), BYT_ADD(size)    H_LOG1(size))
#define CNT2_BYT(size)         CALC_HS(HS_SUB(size), BYT_SUB(size)    H_LOG2(size))
#define CNT1_RTL_L_ELEM(size)  CALC_HS(HS_ADD(size), RTL_L_ELEM_ADD   H_LOG1(size))
#define CNT2_RTL_L_ELEM(size)  CALC_HS(HS_SUB(size), RTL_L_ELEM_SUB   H_LOG2(size))
#define CNT1_RTL_ARR(len,size) CALC_HS(HS_ADD(size), RTL_ARR_ADD(len) H_LOG1(size))
#define CNT2_RTL_ARR(len,size) CALC_HS(HS_SUB(size), RTL_ARR_SUB(len) H_LOG2(size))
#define CNT1_RTL_HSH(len,size) CALC_HS(HS_ADD(size), RTL_HSH_ADD(len) H_LOG1(size))
#define CNT2_RTL_HSH(len,size) CALC_HS(HS_SUB(size), RTL_HSH_SUB(len) H_LOG2(size))


#define ALLOC_HEAP(var,tp,byt)     ((var = (tp) MALLOC(byt)) != NULL)
#define REALLOC_HEAP(var,tp,byt)   ((tp) REALLOC(var, byt))
#define FREE_HEAP(var,byt)         (free((void *) var))


#define ALLOC_USTRI(var,len)       ALLOC_HEAP(var, ustriType, SIZ_USTRI(len))
#define UNALLOC_USTRI(var,len)     FREE_HEAP(var, SIZ_USTRI(len))
#define FREE_USTRI(var,L,cnt,byt)  (CNT(CNT2_USTRI(L, SIZ_USTRI(L), cnt, byt)) FREE_HEAP(var, SIZ_USTRI(L)))
#define REALLOC_USTRI(var,L1,L2)   REALLOC_HEAP(var, ustriType, SIZ_USTRI(L2))
#define COUNT_USTRI(len,cnt,byt)   CNT1_USTRI(len, SIZ_USTRI(len), cnt, byt)
#define COUNT3_USTRI(L1,L2,CT,byt) CNT3(CNT2_USTRI(L1, SIZ_USTRI(L1), CT, byt), CNT1_USTRI(L2, SIZ_USTRI(L2), CT, byt))


#define ALLOC_CSTRI(var,len)       ALLOC_HEAP(var, cstriType, SIZ_CSTRI(len))
#define REALLOC_CSTRI(var,len)     ((cstriType) REALLOC_HEAP(var, ustriType, SIZ_CSTRI(len)))
#define UNALLOC_CSTRI(var,len)     FREE_HEAP(var, SIZ_CSTRI(len))


#define ALLOC_WSTRI(var,len)       ALLOC_HEAP(var, wstriType, SIZ_WSTRI(len))
#define REALLOC_WSTRI(var,len)     ((wstriType) REALLOC_HEAP(var, ustriType, SIZ_WSTRI(len)))
#define UNALLOC_WSTRI(var,len)     FREE_HEAP(var, SIZ_WSTRI(len))


#ifdef WITH_STRI_CAPACITY
#ifdef ALLOW_STRITYPE_SLICES
#define HEAP_ALLOC_STRI(var,len)       (ALLOC_HEAP(var,striType,SIZ_STRI(len))?((var)->mem=(var)->mem1,(var)->capacity=(len),CNT(CNT1_STRI(len,SIZ_STRI(len))) TRUE):FALSE)
#define HEAP_REALLOC_STRI(v1,v2,l1,l2) if((v1=REALLOC_HEAP(v2,striType,SIZ_STRI(l2)))!=NULL){(v1)->mem=(v1)->mem1,(v1)->capacity=l2;}
#else
#define HEAP_ALLOC_STRI(var,len)       (ALLOC_HEAP(var,striType,SIZ_STRI(len))?((var)->capacity=(len),CNT(CNT1_STRI(len,SIZ_STRI(len))) TRUE):FALSE)
#define HEAP_REALLOC_STRI(v1,v2,l1,l2) if((v1=REALLOC_HEAP(v2,striType,SIZ_STRI(l2)))!=NULL)(v1)->capacity=l2;
#endif
#else
#ifdef ALLOW_STRITYPE_SLICES
#define HEAP_ALLOC_STRI(var,len)       (ALLOC_HEAP(var,striType,SIZ_STRI(len))?((var)->mem=(var)->mem1,CNT(CNT1_STRI(len,SIZ_STRI(len))) TRUE):FALSE)
#define HEAP_REALLOC_STRI(v1,v2,l1,l2) if((v1=REALLOC_HEAP(v2,striType,SIZ_STRI(l2)))!=NULL)(v1)->mem=(v1)->mem1;
#else
#define HEAP_ALLOC_STRI(var,len)       (ALLOC_HEAP(var,striType,SIZ_STRI(len))?(CNT(CNT1_STRI(len,SIZ_STRI(len))) TRUE):FALSE)
#define HEAP_REALLOC_STRI(v1,v2,l1,l2) v1=REALLOC_HEAP(v2,striType,SIZ_STRI(l2));
#endif
#endif
#define HEAP_FREE_STRI(var,len)        (CNT(CNT2_STRI(len,SIZ_STRI(len))) FREE_HEAP(var,SIZ_STRI(len)))
#define COUNT3_STRI(len1,len2)         CNT3(CNT2_STRI(len1, SIZ_STRI(len1)), CNT1_STRI(len2, SIZ_STRI(len2)))


#ifdef WITH_STRI_FREELIST
#ifdef WITH_STRI_CAPACITY

#define STRI_FREELIST_ARRAY_SIZE 20

#ifdef DO_INIT
freeListElemType sflist[STRI_FREELIST_ARRAY_SIZE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
unsigned int sflist_allowed[STRI_FREELIST_ARRAY_SIZE] = {
    160, 40, 40, 40, 40, 40, 40, 40, 40, 40,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40};
#else
EXTERN freeListElemType sflist[STRI_FREELIST_ARRAY_SIZE];
EXTERN unsigned int sflist_allowed[STRI_FREELIST_ARRAY_SIZE];
#endif

#define POP_STRI_OK(len)    (len) < STRI_FREELIST_ARRAY_SIZE && sflist[len] != NULL
#define PUSH_STRI_OK(var)   (var)->capacity < STRI_FREELIST_ARRAY_SIZE && sflist_allowed[(var)->capacity] > 0

#define POP_STRI(var,len)   (var = (striType) sflist[len], sflist[len] = sflist[len]->next, sflist_allowed[len]++, TRUE)
#define PUSH_STRI(var,len)  {CNT4_STRI(var,len) ((freeListElemType) var)->next = sflist[len]; sflist[len] = (freeListElemType) var; sflist_allowed[len]--; }

#define ALLOC_STRI_SIZE_OK(var,len)    (POP_STRI_OK(len) ? POP_STRI(var, len) : HEAP_ALLOC_STRI(var, len))
#define ALLOC_STRI_CHECK_SIZE(var,len) (POP_STRI_OK(len) ? POP_STRI(var, len) : ((len)<=MAX_STRI_LEN?HEAP_ALLOC_STRI(var, len):(var=NULL, FALSE)))
#define FREE_STRI(var,len)  if (PUSH_STRI_OK(var)) PUSH_STRI(var, (var)->capacity) else HEAP_FREE_STRI(var, len);

#else

#ifdef DO_INIT
freeListElemType sflist = NULL;
unsigned int sflist_allowed = 40;
#else
EXTERN freeListElemType sflist;
EXTERN unsigned int sflist_allowed;
#endif

#define POP_STRI_OK(len)    (len) == 1 && sflist != NULL
#define PUSH_STRI_OK(var)   (var)->size == 1 && sflist_allowed > 0

#define POP_STRI(var)       (var = (striType) sflist, sflist = sflist->next, sflist_allowed++, TRUE)
#define PUSH_STRI(var)      {((freeListElemType) var)->next = sflist; sflist = (freeListElemType) var; sflist_allowed--; }

#define ALLOC_STRI_SIZE_OK(var,len)    (POP_STRI_OK(len) ? POP_STRI(var) : HEAP_ALLOC_STRI(var, len))
#define ALLOC_STRI_CHECK_SIZE(var,len) (POP_STRI_OK(len) ? POP_STRI(var) : ((len)<=MAX_STRI_LEN?HEAP_ALLOC_STRI(var, len):(var=NULL, FALSE)))
#define FREE_STRI(var,len)  if (PUSH_STRI_OK(var)) PUSH_STRI(var) else HEAP_FREE_STRI(var, len);

#endif
#else

#define ALLOC_STRI_SIZE_OK(var,len)       HEAP_ALLOC_STRI(var, len)
#define ALLOC_STRI_CHECK_SIZE(var,len)    ((len)<=MAX_STRI_LEN?HEAP_ALLOC_STRI(var, len):(var=NULL, FALSE))
#define FREE_STRI(var,len)                HEAP_FREE_STRI(var,len)

#endif

#ifdef WITH_STRI_CAPACITY
#define GROW_STRI(v1,v2,l1,l2)            ((l2)>(v2)->capacity?(v1=growStri(v2,l2)):(v1=(v2)))
#define SHRINK_STRI(v1,v2,l1,l2)          ((l2)<(v2)->capacity>>2?(v1=shrinkStri(v2,l2)):(v1=(v2)))
#define SHRINK_REASON(v2,l2)              ((l2)<(v2)->capacity>>2)
#else
#define GROW_STRI(v1,v2,l1,l2)            if((l2) <= MAX_STRI_LEN){HEAP_REALLOC_STRI(v1,v2,l1,l2)}else v1=NULL;
#define SHRINK_STRI(v1,v2,l1,l2)          HEAP_REALLOC_STRI(v1,v2,l1,l2)
#endif

#define REALLOC_STRI_SIZE_OK(v1,v2,l1,l2)      HEAP_REALLOC_STRI(v1,v2,l1,l2)
#define REALLOC_STRI_CHECK_SIZE(v1,v2,l1,l2)   if((l2) <= MAX_STRI_LEN){HEAP_REALLOC_STRI(v1,v2,l1,l2)}else v1=NULL;
/* The following macro is used when the new size is smaller or equal to the current one. */
#define REALLOC_STRI_SIZE_SMALLER(v1,v2,l1,l2) HEAP_REALLOC_STRI(v1,v2,l1,l2)

#ifdef ALLOW_STRITYPE_SLICES
#ifdef WITH_STRI_CAPACITY
#define SLICE_OVERLAPPING(var,dest)       ((var)->mem>=(dest)->mem1&&(var)->mem<&(dest)->mem1[(dest)->capacity])
#define SET_SLICE_CAPACITY(var,cap)       (var)->capacity = (cap)
#else
#define SLICE_OVERLAPPING(var,dest)       ((var)->mem>=(dest)->mem1&&(var)->mem<&(dest)->mem1[(dest)->size])
#define SET_SLICE_CAPACITY(var,cap)
#endif
#endif


#ifdef ALLOW_BSTRITYPE_SLICES
#define ALLOC_BSTRI_SIZE_OK(var,len)       (ALLOC_HEAP(var, bstriType, SIZ_BSTRI(len))?(var->mem = var->mem1, CNT(CNT1_BSTRI(len, SIZ_BSTRI(len))) TRUE):FALSE)
#define REALLOC_BSTRI_SIZE_OK(v1,v2,l1,l2) ((v1=REALLOC_HEAP(v2, bstriType, SIZ_BSTRI(l2)))?((v1)->mem=(v1)->mem1,0):0)
#else
#define ALLOC_BSTRI_SIZE_OK(var,len)       (ALLOC_HEAP(var, bstriType, SIZ_BSTRI(len))?CNT(CNT1_BSTRI(len, SIZ_BSTRI(len))) TRUE:FALSE)
#define REALLOC_BSTRI_SIZE_OK(v1,v2,l1,l2) (v1=REALLOC_HEAP(v2, bstriType, SIZ_BSTRI(l2)),0)
#endif

#define ALLOC_BSTRI_CHECK_SIZE(var,len)       ((len) <= MAX_BSTRI_LEN?ALLOC_BSTRI_SIZE_OK(var, len):(var=NULL,FALSE))
#define FREE_BSTRI(var,len)                   (CNT(CNT2_BSTRI(len, SIZ_BSTRI(len))) FREE_HEAP(var, SIZ_BSTRI(len)))
#define REALLOC_BSTRI_CHECK_SIZE(v1,v2,l1,l2) ((l2)  <= MAX_BSTRI_LEN?REALLOC_BSTRI_SIZE_OK(v1,v2,l1,l2):(v1=NULL,0))
#define COUNT3_BSTRI(len1,len2)               CNT3(CNT2_BSTRI(len1, SIZ_BSTRI(len1)), CNT1_BSTRI(len2, SIZ_BSTRI(len2)))


#define ALLOC_RTL_L_ELEM(var)      (ALLOC_HEAP(var, rtlListType, SIZ_RTL_L_ELEM)?CNT(CNT1_RTL_L_ELEM(SIZ_RTL_L_ELEM)) TRUE:FALSE)
#define FREE_RTL_L_ELEM(var)       (CNT(CNT2_RTL_L_ELEM(SIZ_RTL_L_ELEM)) FREE_HEAP(var, SIZ_RTL_L_ELEM))


#define ALLOC_ARRAY(var,len)       (ALLOC_HEAP(var, arrayType, SIZ_ARR(len))?CNT(CNT1_ARR(len, SIZ_ARR(len))) TRUE:FALSE)
#define FREE_ARRAY(var,len)        (CNT(CNT2_ARR(len, SIZ_ARR(len))) FREE_HEAP(var, SIZ_ARR(len)))
#define REALLOC_ARRAY(var,ln1,ln2) REALLOC_HEAP(var, arrayType, SIZ_ARR(ln2))
#define COUNT3_ARRAY(len1,len2)    CNT3(CNT2_ARR(len1, SIZ_ARR(len1)), CNT1_ARR(len2, SIZ_ARR(len2)))


#define ALLOC_RTL_ARRAY(var,len)       (ALLOC_HEAP(var, rtlArrayType, SIZ_RTL_ARR(len))?CNT(CNT1_RTL_ARR(len, SIZ_RTL_ARR(len))) TRUE:FALSE)
#define FREE_RTL_ARRAY(var,len)        (CNT(CNT2_RTL_ARR(len, SIZ_RTL_ARR(len))) FREE_HEAP(var, SIZ_RTL_ARR(len)))
#define REALLOC_RTL_ARRAY(var,ln1,ln2) REALLOC_HEAP(var, rtlArrayType, SIZ_RTL_ARR(ln2))
#define COUNT3_RTL_ARRAY(len1,len2)    CNT3(CNT2_RTL_ARR(len1, SIZ_RTL_ARR(len1)), CNT1_RTL_ARR(len2, SIZ_RTL_ARR(len2)))


#define ALLOC_HASH(var,len)        (ALLOC_HEAP(var, hashType, SIZ_HSH(len))?CNT(CNT1_HSH(len, SIZ_HSH(len))) TRUE:FALSE)
#define FREE_HASH(var,len)         (CNT(CNT2_HSH(len, SIZ_HSH(len))) FREE_HEAP(var, SIZ_HSH(len)))
#define REALLOC_HASH(var,ln1,ln2)  REALLOC_HEAP(var, hashType, SIZ_HSH(ln2))
#define COUNT3_HASH(len1,len2)     CNT3(CNT2_HSH(len1, SIZ_HSH(len1)), CNT1_HSH(len2, SIZ_HSH(len2)))


#define ALLOC_RTL_HASH(var,len)       (ALLOC_HEAP(var, rtlHashType, SIZ_RTL_HSH(len))?CNT(CNT1_RTL_HSH(len, SIZ_RTL_HSH(len))) TRUE:FALSE)
#define FREE_RTL_HASH(var,len)        (CNT(CNT2_RTL_HSH(len, SIZ_RTL_HSH(len))) FREE_HEAP(var, SIZ_RTL_HSH(len)))
#define REALLOC_RTL_HASH(var,ln1,ln2) REALLOC_HEAP(var, rtlHashType, SIZ_RTL_HSH(ln2))
#define COUNT3_RTL_HASH(len1,len2)    CNT3(CNT2_RTL_HSH(len1, SIZ_RTL_HSH(len1)), CNT1_RTL_HSH(len2, SIZ_RTL_HSH(len2)))


#define ALLOC_SET(var,len)         (ALLOC_HEAP(var, setType, SIZ_SET(len))?CNT(CNT1_SET(len, SIZ_SET(len))) TRUE:FALSE)
#define FREE_SET(var,len)          (CNT(CNT2_SET(len, SIZ_SET(len))) FREE_HEAP(var, SIZ_SET(len)))
#define REALLOC_SET(var,len1,len2) REALLOC_HEAP(var, setType, SIZ_SET(len2))
#define COUNT3_SET(len1,len2)      CNT3(CNT2_SET(len1, SIZ_SET(len1)), CNT1_SET(len2, SIZ_SET(len2)))


#define ALLOC_STRUCT(var,len)       (ALLOC_HEAP(var, structType, SIZ_SCT(len))?CNT(CNT1_SCT(len, SIZ_SCT(len))) TRUE:FALSE)
#define FREE_STRUCT(var,len)        (CNT(CNT2_SCT(len, SIZ_SCT(len))) FREE_HEAP(var, SIZ_SCT(len)))
#define REALLOC_STRUCT(var,ln1,ln2) REALLOC_HEAP(var, structType, SIZ_SCT(ln2))
#define COUNT3_STRUCT(len1,len2)    CNT3(CNT2_SCT(len1, SIZ_SCT(len1)), CNT1_SCT(len2, SIZ_SCT(len2)))


#define ALLOC_RECORD(var,rec,cnt)  (ALLOC_HEAP(var, rec*, SIZ_REC(rec))?CNT(CNT1_REC(SIZ_REC(rec), cnt)) TRUE:FALSE)
#define FREE_RECORD(var,rec,cnt)   (CNT(CNT2_REC(SIZ_REC(rec), cnt)) FREE_HEAP(var, SIZ_REC(rec)))


#define ALLOC_BYTES(var,byt)       (ALLOC_HEAP(var, char *, byt)?CNT(CNT1_BYT(byt)) TRUE:FALSE)
#define ALLOC_UBYTES(var,byt)      (ALLOC_HEAP(var, unsigned char *, byt)?CNT(CNT1_BYT(byt)) TRUE:FALSE)
#define FREE_BYTES(var,byt)        (CNT(CNT2_BYT(byt)) FREE_HEAP(var, byt))


#define ALLOC_TABLE(var,tp,nr)      (ALLOC_HEAP(var, tp *, SIZ_TAB(tp,nr))?CNT(CNT1_BYT(SIZ_TAB(tp,nr))) TRUE:FALSE)
#define FREE_TABLE(var,tp,nr)       (CNT(CNT2_BYT(SIZ_TAB(tp, nr))) FREE_HEAP(var, SIZ_TAB(tp, nr)))
#define REALLOC_TABLE(var,tp,n1,n2) REALLOC_HEAP(var, tp *, SIZ_TAB(tp, n2))
#define COUNT3_TABLE(tp,nr1,nr2)    CNT3(CNT2_BYT(SIZ_TAB(tp, nr1)), CNT1_BYT(SIZ_TAB(tp, nr2)))


void setupStack (void);
#ifdef WITH_STRI_CAPACITY
striType growStri (striType stri, memSizeType len);
striType shrinkStri (striType stri, memSizeType len);
#endif
#ifdef DO_HEAP_CHECK
void check_heap (long, const char *, unsigned int);
#endif
#ifdef DO_HEAP_STATISTIC
void rtlHeapStatistic (void);
#endif
