/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/flistutl.h                                      */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Procedures for free memory list maintainance.          */
/*                                                                  */
/********************************************************************/

typedef struct {
    objecttype objects;
    listtype list_elems;
    nodetype nodes;
    infiltype infiles;
  } freelisttype;

#ifdef DO_INIT
freelisttype flist = {NULL, NULL, NULL, NULL};
#else
EXTERN freelisttype flist;
#endif


#ifdef DO_HEAP_LOG
#define F_LOG1(X)     printf("\nlalloc(%u) -> %ld\n", sizeof(*X), (long) X),
#define F_LOG2(X)     printf("\nlfree(%u) <- %ld\n", sizeof(*X), (long) X),
#else
#define F_LOG1(X)
#define F_LOG2(X)
#endif

#ifdef USE_CHUNK_ALLOCS
#ifdef USE_ALTERNATE_CHUNK_ALLOCS
#define OLD_CHUNK(var,tp,byt)    (var = (tp) chunk.freemem, chunk.freemem += (byt), TRUE)
#define NEW_CHUNK(var,tp,byt)    ((var = (tp) heap_chunk(byt)) != NULL)
#define ALLOC_CHUNK(var,tp,byt)  ((byt) > chunk.beyond - chunk.freemem ? NEW_CHUNK(var, tp, byt) : OLD_CHUNK(var, tp, byt))
#define FREE_CHUNK(var,byt)      (chunk.lost_bytes += (byt))
#else
#define OLD_CHUNK(var,tp,byt)    (var = (tp) chunk.freemem, chunk.freemem += (byt), chunk.size -= (byt), TRUE)
#define NEW_CHUNK(var,tp,byt)    ((var = (tp) heap_chunk(byt)) != NULL)
#define ALLOC_CHUNK(var,tp,byt)  ((byt) > chunk.size ? NEW_CHUNK(var, tp, byt) : OLD_CHUNK(var, tp, byt))
#define FREE_CHUNK(var,byt)      (chunk.lost_bytes += (byt))
#endif
#else
#endif


#ifdef USE_CHUNK_ALLOCS
#define ALLOC_ID_NAME(var,len)     ALLOC_CHUNK(var, ustritype, SIZ_ID(len))
#define FREE_ID_NAME(var,len)      (CNT(CNT2_USTRI(len, SIZ_USTRI(len), count.idt, count.idt_bytes)) FREE_CHUNK(var, SIZ_ID(len)))
#else
#define ALLOC_ID_NAME(var,len)     ALLOC_HEAP(var, ustritype, SIZ_USTRI(len))
#define FREE_ID_NAME(var,len)      (CNT(CNT2_USTRI(len, SIZ_USTRI(len), count.idt, count.idt_bytes)) FREE_HEAP(var, SIZ_USTRI(len)))
#endif
#define COUNT_ID_NAME(len)         CNT1_USTRI((len), SIZ_USTRI(len), count.idt, count.idt_bytes)

#ifdef USE_CHUNK_ALLOCS
#define ALLOC_FLISTELEM(var,rec)   ALLOC_CHUNK(var, rec *, SIZ_REC(rec))
#else
#define ALLOC_FLISTELEM(var,rec)   ALLOC_HEAP(var, rec *, SIZ_REC(rec))
#endif

#define COUNT_FLISTELEM(rec,cnt)   CNT1_REC(SIZ_REC(rec), cnt)


#define HEAP_OBJ(O,T)   (!ALLOC_FLISTELEM(O, T) ? FALSE : CNT(COUNT_FLISTELEM(T, count.object))    TRUE)
#define HEAP_L_E(L,T)   (!ALLOC_FLISTELEM(L, T) ? FALSE : CNT(COUNT_FLISTELEM(T, count.list_elem)) TRUE)
#define HEAP_NODE(N,T)  (!ALLOC_FLISTELEM(N, T) ? FALSE : CNT(COUNT_FLISTELEM(T, count.node))      TRUE)
#define HEAP_FILE(F,T)  (!ALLOC_FLISTELEM(F, T) ? FALSE : CNT(COUNT_FLISTELEM(T, count.infil))     TRUE)

#define POP_OBJ(O)      (O = flist.objects,    flist.objects = flist.objects->value.objvalue, F_LOG1(O) TRUE)
#define POP_L_E(L)      (L = flist.list_elems, flist.list_elems = flist.list_elems->next,     F_LOG1(L) TRUE)
#define POP_NODE(N)     (N = flist.nodes,      flist.nodes = flist.nodes->next1,              F_LOG1(N) TRUE)
#define POP_FILE(F)     (F = flist.infiles,    flist.infiles = flist.infiles->next,           F_LOG1(F) TRUE)


#ifdef WITH_OBJECT_FREELIST
#define ALLOC_OBJECT(O) (flist.objects != NULL ? POP_OBJ(O) : HEAP_OBJ(O, objectrecord))
#define FREE_OBJECT(O)  (F_LOG2(O) (O)->value.objvalue = flist.objects, (O)->objcategory = 0, flist.objects = (O))
/* #define FREE_OBJECT(O)  (printf("FREE_OBJECT(%lu)\n", O), F_LOG2(O) (O)->value.objvalue = flist.objects, (O)->objcategory = 0, flist.objects = (O)) */
/* #define FREE_OBJECT(O)  (F_LOG2(O) check_obj_flist(O) ? ((O)->value.objvalue=flist.objects,(O)->objcategory=0,flist.objects=(O)) : NULL) */
#else
#define ALLOC_OBJECT(O) HEAP_OBJ(O, objectrecord)
#define FREE_OBJECT(O)  FREE_RECORD(O, objectrecord, count.object)
#ifdef USE_CHUNK_ALLOCS
#error Configuration error: USE_CHUNK_ALLOCS needs WITH_OBJECT_FREELIST
#endif
#endif

#ifdef WITH_LIST_FREELIST
#define ALLOC_L_ELEM(L) (flist.list_elems != NULL ? POP_L_E(L) : HEAP_L_E(L, listrecord))
#define FREE_L_ELEM(L)  (F_LOG2(L) (L)->next = flist.list_elems, flist.list_elems = (L))
#else
#define ALLOC_L_ELEM(L) HEAP_L_E(L, listrecord)
#define FREE_L_ELEM(L)  FREE_RECORD(L, listrecord, count.list_elem)
#ifdef USE_CHUNK_ALLOCS
#error Configuration error: USE_CHUNK_ALLOCS needs WITH_LIST_FREELIST
#endif
#endif

#ifdef WITH_NODE_FREELIST
#define ALLOC_NODE(N)   (flist.nodes != NULL ? POP_NODE(N) : HEAP_NODE(N, noderecord))
#define FREE_NODE(N)    (F_LOG2(N) (N)->next1 = flist.nodes, flist.nodes = (N))
#else
#define ALLOC_NODE(N)   HEAP_NODE(N, noderecord)
#define FREE_NODE(N)    FREE_RECORD(N, noderecord, count.node)
#ifdef USE_CHUNK_ALLOCS
#error Configuration error: USE_CHUNK_ALLOCS needs WITH_NODE_FREELIST
#endif
#endif

#ifdef WITH_FILE_FREELIST
#define ALLOC_FILE(F)   (flist.infiles != NULL ? POP_FILE(F) : HEAP_FILE(F, infilrecord))
#define FREE_FILE(F)    (F_LOG2(F) (F)->next = flist.infiles, flist.infiles = (F))
#else
#define ALLOC_FILE(F)   HEAP_FILE(F, infilrecord)
#define FREE_FILE(F)    FREE_RECORD(F, infilrecord, count.infil)
#ifdef USE_CHUNK_ALLOCS
#error Configuration error: USE_CHUNK_ALLOCS needs WITH_FILE_FREELIST
#endif
#endif


#ifdef DO_HEAP_STATISTIC
void heap_statistic (void);
#endif
memsizetype heapsize (void);
#ifdef USE_CHUNK_ALLOCS
#ifdef USE_FLIST_ALLOC
void *flist_alloc (size_t);
#endif
#else
void reuse_free_lists (void);
#endif
#ifdef USE_CHUNK_ALLOCS
void *heap_chunk (size_t);
#endif
