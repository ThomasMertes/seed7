/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Main                                                    */
/*  File: seed7/src/data.h                                          */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2000  Thomas Mertes      */
/*  Content: Type definitions for general data structures.          */
/*                                                                  */
/********************************************************************/

typedef enum {
    SYMBOLOBJECT,        /* pos (file, line) - Symbol object        */
                         /*                    created by read_atom */
                         /*                    and read_name        */
    DECLAREDOBJECT,      /* NO VALUE -         Object declared and  */
                         /*                    not initialized      */
    FORWARDOBJECT,       /* NO VALUE -         Object declared      */
                         /*                    forward              */
    FWDREFOBJECT,        /* objvalue -    Reference to Object which */
                         /*               was declared forward      */
    BLOCKOBJECT,         /* blockvalue - Procedure possibly with    */
                         /*              parameters, declared       */
                         /*              result or local variables  */
    CALLOBJECT,          /* listvalue - Subroutine call:            */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    MATCHOBJECT,         /* listvalue - Don't exec subroutine call: */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    TYPEOBJECT,          /* typevalue -   type                      */
    FORMPARAMOBJECT,     /* objvalue -    Reference to formal param */
    INTOBJECT,           /* intvalue -    integer                   */
    BIGINTOBJECT,        /* bigintvalue - bigInteger                */
    CHAROBJECT,          /* charvalue -   char                      */
    STRIOBJECT,          /* strivalue -   string                    */
    ARRAYOBJECT,         /* arrayvalue -  array                     */
    HASHOBJECT,          /* hashvalue -   hash                      */
    STRUCTOBJECT,        /* structvalue - struct                    */
    CLASSOBJECT,         /* structvalue - struct                    */
    INTERFACEOBJECT,     /* objvalue -    Dynamic Object            */
    SETOBJECT,           /* setvalue -    set                       */
    FILEOBJECT,          /* filevalue -   file                      */
    LISTOBJECT,          /* listvalue -   list                      */
    FLOATOBJECT,         /* floatvalue -  float                     */
    WINOBJECT,           /* winvalue -    Window                    */
    ENUMLITERALOBJECT,   /* objvalue -    Enumeration literal       */
    CONSTENUMOBJECT,     /* objvalue -    Constant enumeration obj  */
    VARENUMOBJECT,       /* objvalue -    Variable enumeration obj  */
    REFOBJECT,           /* objvalue -    reference                 */
    REFLISTOBJECT,       /* listvalue -   ref_list                  */
    EXPROBJECT,          /* listvalue -   expression                */
    ACTOBJECT,           /* actvalue -    Action                    */
    VALUEPARAMOBJECT,    /* objvalue -    Formal value parameter    */
    REFPARAMOBJECT,      /* objvalue -    Formal ref parameter      */
    RESULTOBJECT,        /* objvalue -    Result of procedure       */
    LOCALVOBJECT,        /* objvalue -    Local variable            */
    PROGOBJECT           /* progvalue -   Program                   */
  } objectclass;

typedef enum {
    SY_TOKEN,
    EXPR_TOKEN,
    UNDEF_SYNTAX,
    LIST_WITH_TYPEOF_SYNTAX,
    SELECT_ELEMENT_FROM_LIST_SYNTAX
  } token_class;

typedef unsigned long      postype;
typedef unsigned int       linenumtype;
typedef unsigned int       filenumtype;
typedef unsigned int       classtype;

typedef struct identstruct    *identtype;
typedef struct tokenstruct    *tokentype;
typedef struct nodestruct     *nodetype;
typedef struct entitystruct   *entitytype;
typedef struct ownerstruct    *ownertype;
typedef struct objectstruct   *objecttype;
typedef struct stackstruct    *stacktype;
typedef struct typeliststruct *typelisttype;
typedef struct typestruct     *typetype;
typedef struct liststruct     *listtype;
typedef struct locobjstruct   *locobjtype;
typedef struct locliststruct  *loclisttype;
typedef struct blockstruct    *blocktype;
typedef struct arraystruct    *arraytype;
typedef struct helemstruct    *helemtype;
typedef struct hashstruct     *hashtype;
typedef struct structstruct   *structtype;
typedef struct progstruct     *progtype;
typedef struct infilstruct    *infiltype;

#ifdef ANSI_C
typedef objecttype (*acttype) (listtype);
#else
typedef objecttype (*acttype) ();
#endif

typedef struct identstruct {
    ustritype name;
    identtype next1;
    identtype next2;
    entitytype entity;
    tokentype prefix_token;
    tokentype infix_token;
    smallpriortype prefix_priority;
    smallpriortype infix_priority;
    smallpriortype left_token_priority;
  } identrecord;

typedef struct tokenstruct {
    tokentype next;
    tokentype alternative;
    union {
      identtype ident;
      struct {
        smallpriortype priority;
        typetype type_of;
      } expr_par;
      typetype type_of;
      inttype select;
    } token_value;
    token_class token_class;
  } tokenrecord;

typedef struct nodestruct {
    unsigned long usage_count;
    objecttype match_obj;
    nodetype next1;
    nodetype next2;
    entitytype entity;
    nodetype symbol;
    nodetype inout_param;
    nodetype other_param;
    nodetype attr;
  } noderecord;

typedef struct entitystruct {
    identtype ident;
    objecttype syobject;
    listtype name_list;
    ownertype owner;
  } entityrecord;

typedef struct ownerstruct {
    objecttype obj;
    listtype params;
    stacktype decl_level;
    ownertype next;
  } ownerrecord;

typedef union {
    postype    pos;          /* SYMBOLOBJECT */
    nodetype   nodevalue;    /* MDULEOBJECT */
    typetype   typevalue;    /* TYPEOBJECT */
    inttype    intvalue;     /* INTOBJECT */
    biginttype bigintvalue;  /* BIGINTOBJECT */
    chartype   charvalue;    /* CHAROBJECT */
    stritype   strivalue;    /* STRIOBJECT */
    arraytype  arrayvalue;   /* ARRAYOBJECT */
    hashtype   hashvalue;    /* HASHOBJECT */
    settype    setvalue;     /* SETOBJECT */
    structtype structvalue;  /* STRUCTOBJECT */
    filetype   filevalue;    /* FILEOBJECT */
    listtype   listvalue;    /* LISTOBJECT, EXPROBJECT */
    wintype    winvalue;     /* WINOBJECT */
    objecttype objvalue;     /* ENUMLITERALOBJECT, CONSTENUMOBJECT */
                             /* VARENUMOBJECT, VALUEPARAMOBJECT */
                             /* REFPARAMOBJECT, RESULTOBJECT */
                             /* LOCALVOBJECT, FORMPARAMOBJECT */
                             /* CLASSOBJECT */
    blocktype  blockvalue;   /* BLOCKOBJECT */
    acttype    actvalue;     /* ACTOBJECT */
    progtype   progvalue;    /* PROGOBJECT */
#ifdef WITH_FLOAT
    floattype  floatvalue;   /* FLOATOBJECT */
#endif
  } valueunion;

typedef union {
    entitytype entity;
    postype posinfo;
  } descriptorunion;

typedef struct objectstruct {
    typetype type_of;
    descriptorunion descriptor;
    valueunion value;
    classtype objclass;
  } objectrecord;

typedef struct stackstruct {
    listtype local_object_list;
    listtype *object_list_insert_place;
    stacktype upward;
    stacktype downward;
  } stackrecord;

typedef struct typeliststruct {
    typelisttype next;
    typetype type_elem;
  } typelistrecord;

typedef struct typestruct {
    objecttype match_obj;
    typetype meta;
    typetype func_type;
    typetype varfunc_type;
    typetype result_type;
    booltype is_varfunc_type;
    typelisttype interfaces;
    identtype name;
    objecttype create_call_obj;
    objecttype destroy_call_obj;
    objecttype copy_call_obj;
    objecttype ord_call_obj;
    objecttype in_call_obj;
  } typerecord;

typedef struct liststruct {
    listtype next;
    objecttype obj;
  } listrecord;

typedef struct locobjstruct {
    objecttype object;
    objecttype init_value;
    objecttype create_call_obj;
    objecttype destroy_call_obj;
  } locobjrecord;

typedef struct locliststruct {
    locobjrecord local;
    loclisttype next;
  } loclistrecord;

typedef struct blockstruct {
    loclisttype params;
    locobjrecord result;
    loclisttype local_vars;
    listtype local_consts;
    objecttype body;
  } blockrecord;

typedef struct arraystruct {
    inttype min_position;
    inttype max_position;
    objectrecord arr[1];
  } arrayrecord;

typedef struct helemstruct {
    helemtype next_less;
    helemtype next_greater;
    objectrecord key;
    objectrecord data;
  } helemrecord;

typedef struct hashstruct {
    int bits;
    int mask;
    int table_size;
    memsizetype size;
    helemtype table[1];
  } hashrecord;

typedef struct structstruct {
    memsizetype size;
    objectrecord stru[1];
  } structrecord;

#define NUMBER_OF_SYSVARS 26

typedef objecttype systype[NUMBER_OF_SYSVARS];

#define ID_TABLE_SIZE     1024

typedef struct idrootstruct {
    identtype table[ID_TABLE_SIZE];
    identtype table1[127];
    identtype literal;
    identtype end_of_file;
  } idroottype;

typedef struct {
    identtype lparen;
    identtype lbrack;
    identtype lbrace;
    identtype rparen;
    identtype rbrack;
    identtype rbrace;
    identtype dot;
    identtype colon;
    identtype comma;
    identtype semicolon;
    identtype dollar;
    identtype r_arrow;
    identtype l_arrow;
    identtype out_arrow;
    identtype in_arrow;
    identtype type;
    identtype constant;
    identtype ref;
    identtype syntax;
    identtype system;
    identtype include;
    identtype is;
    identtype func;
    identtype param;
    identtype enumlit;
    identtype subtype;
    identtype newtype;
    identtype action;
  } findidtype;

typedef struct progstruct {
    unsigned long usage_count;
    objecttype main_object;
    ustritype source_file_name;
    unsigned int error_count;
    idroottype ident;
    findidtype id_for;
    systype sys_var;
    nodetype declaration_root;
    stacktype stack_global;
    stacktype stack_data;
    stacktype stack_current;
  } progrecord;

typedef struct infilstruct {
    FILE *fil;
    ustritype name;
#ifdef USE_ALTERNATE_NEXT_CHARACTER
    ustritype start;
    ustritype nextch;
    ustritype beyond;
    memsizetype buffer_size;
#else
#ifdef USE_INFILE_BUFFER
    ustritype buffer;
#endif
#endif
    int character;
    infiltype curr_infile;
    infiltype up_infile;
    infiltype next;
    linenumtype line;
    linenumtype next_msg_line;
    filenumtype file_number;
    booltype end_of_file;
  } infilrecord;


extern progrecord prog;


#define INIT_POS(O,L,F)     (O)->value.pos = ((postype) (L)) | (((postype) (F)) << 20)
#define CREATE_POSINFO(L,F) ((postype) (L)) | (((postype) (F)) << 20)
#define GET_LINE_NUM(O)     (linenumtype) (((long)(O)->descriptor.posinfo) & 1048575L)
#define GET_FILE_NUM(O)     (filenumtype) ((((long)(O)->descriptor.posinfo) & 2146435072L) >> 20)

#define CLASS_MASK   ((classtype)  63)
#define VAR_MASK     ((classtype)  64)
#define TEMP_MASK    ((classtype) 128)
#define TEMP2_MASK   ((classtype) 256)
#define POSINFO_MASK ((classtype) 512)

#define SET_ANY_FLAG(O,FLAG)           (O)->objclass = (O)->objclass | (FLAG)

#define INIT_CLASS_OF_OBJ(O,CLASS)     (O)->objclass = (classtype) (CLASS)
#define SET_CLASS_OF_OBJ(O,CLASS)      (O)->objclass = ((O)->objclass & ~CLASS_MASK) | (classtype) (CLASS)
#define CLASS_OF_OBJ(O)                ((objectclass) ((O)->objclass & CLASS_MASK))

#define INIT_CLASS_OF_VAR(O,CLASS)     (O)->objclass = (classtype) (CLASS) | VAR_MASK
#define SET_VAR_FLAG(O)                (O)->objclass = (O)->objclass | VAR_MASK
#define CLEAR_VAR_FLAG(O)              (O)->objclass = (O)->objclass & ~VAR_MASK
#define VAR_OBJECT(O)                  ((O)->objclass & VAR_MASK)
#define COPY_VAR_FLAG(O1,O2)           (O1)->objclass = ((O1)->objclass & ~VAR_MASK) | ((O2)->objclass & VAR_MASK)

#define INIT_CLASS_OF_TEMP(O,CLASS)    (O)->objclass = (classtype) (CLASS) | TEMP_MASK
#define SET_TEMP_FLAG(O)               (O)->objclass = (O)->objclass | TEMP_MASK
#define CLEAR_TEMP_FLAG(O)             (O)->objclass = (O)->objclass & ~TEMP_MASK
#define TEMP_OBJECT(O)                 ((O)->objclass & TEMP_MASK)

#define INIT_CLASS_OF_TEMP2(O,CLASS)   (O)->objclass = (classtype) (CLASS) | TEMP2_MASK
#define SET_TEMP2_FLAG(O)              (O)->objclass = (O)->objclass | TEMP2_MASK
#define CLEAR_TEMP2_FLAG(O)            (O)->objclass = (O)->objclass & ~TEMP2_MASK
#define TEMP2_OBJECT(O)                ((O)->objclass & TEMP2_MASK)

#define INIT_CLASS_OF_POSINFO(O,CLASS) (O)->objclass = (classtype) (CLASS) | POSINFO_MASK
#define SET_POSINFO_FLAG(O)            (O)->objclass = (O)->objclass | POSINFO_MASK
#define CLEAR_POSINFO_FLAG(O)          (O)->objclass = (O)->objclass & ~POSINFO_MASK
#define HAS_POSINFO(O)                 ((O)->objclass & POSINFO_MASK)

#define HAS_DESCRIPTOR_ENTITY(O)       (!HAS_POSINFO(O) && (O)->descriptor.entity != NULL) 
