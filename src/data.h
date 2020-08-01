/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
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
    BSTRIOBJECT,         /* bstrivalue -  byte string               */
    ARRAYOBJECT,         /* arrayvalue -  array                     */
    HASHOBJECT,          /* hashvalue -   hash                      */
    STRUCTOBJECT,        /* structvalue - struct                    */
    CLASSOBJECT,         /* structvalue - struct                    */
    INTERFACEOBJECT,     /* objvalue -    Dynamic Object            */
    SETOBJECT,           /* setvalue -    set                       */
    FILEOBJECT,          /* filevalue -   file                      */
    FILEDESOBJECT,       /* filedesvalue - file descriptor          */
    SOCKETOBJECT,        /* socketvalue - socket                    */
    POLLOBJECT,          /* pollvalue -   poll list                 */
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
    PROGOBJECT,          /* progvalue -   Program                   */
    DATABASEOBJECT,      /* databasevalue - Database                */
    SQLSTMTOBJECT        /* sqlstmtvalue -  SQL statement           */
  } objectcategory;

typedef enum {
    SY_TOKEN,
    EXPR_TOKEN,
    UNDEF_SYNTAX,
    LIST_WITH_TYPEOF_SYNTAX,
    SELECT_ELEMENT_FROM_LIST_SYNTAX
  } tokencategory;

typedef uint32type         postype;
typedef unsigned int       linenumtype;
typedef unsigned int       filenumtype;
typedef uint16type         categorytype;

typedef struct identstruct    *identtype;
typedef struct tokenstruct    *tokentype;
typedef struct nodestruct     *nodetype;
typedef struct entitystruct   *entitytype;
typedef struct ownerstruct    *ownertype;
typedef struct objectstruct   *objecttype;
typedef struct propertystruct *propertytype;
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

typedef const struct identstruct    *const_identtype;
typedef const struct tokenstruct    *const_tokentype;
typedef const struct nodestruct     *const_nodetype;
typedef const struct entitystruct   *const_entitytype;
typedef const struct ownerstruct    *const_ownertype;
typedef const struct objectstruct   *const_objecttype;
typedef const struct propertystruct *const_propertytype;
typedef const struct stackstruct    *const_stacktype;
typedef const struct typeliststruct *const_typelisttype;
typedef const struct typestruct     *const_typetype;
typedef const struct liststruct     *const_listtype;
typedef const struct locobjstruct   *const_locobjtype;
typedef const struct locliststruct  *const_loclisttype;
typedef const struct blockstruct    *const_blocktype;
typedef const struct arraystruct    *const_arraytype;
typedef const struct helemstruct    *const_helemtype;
typedef const struct hashstruct     *const_hashtype;
typedef const struct structstruct   *const_structtype;
typedef const struct progstruct     *const_progtype;
typedef const struct infilstruct    *const_infiltype;

typedef objecttype (*acttype) (listtype);

typedef struct identstruct {
    ustritype name;
    identtype next1;
    identtype next2;
    entitytype entity;
    tokentype prefix_token;
    tokentype infix_token;
    prioritytype prefix_priority;
    prioritytype infix_priority;
    prioritytype left_token_priority;
  } identrecord;

typedef struct tokenstruct {
    tokentype next;
    tokentype alternative;
    union {
      identtype ident;
      struct {
        prioritytype priority;
        typetype type_of;
      } expr_par;
      typetype type_of;
      inttype select;
    } token_value;
    tokencategory token_category;
  } tokenrecord;

typedef struct nodestruct {
    uinttype usage_count;
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
    listtype fparam_list;
    union {
      ownertype owner;
      entitytype next;
    } data;
  } entityrecord;

typedef struct ownerstruct {
    objecttype obj;
    stacktype decl_level;
    ownertype next;
  } ownerrecord;

typedef union {
    postype      pos;           /* SYMBOLOBJECT */
    nodetype     nodevalue;     /* MDULEOBJECT */
    typetype     typevalue;     /* TYPEOBJECT */
    inttype      intvalue;      /* INTOBJECT */
    biginttype   bigintvalue;   /* BIGINTOBJECT */
    chartype     charvalue;     /* CHAROBJECT */
    stritype     strivalue;     /* STRIOBJECT */
    bstritype    bstrivalue;    /* BSTRIOBJECT */
    arraytype    arrayvalue;    /* ARRAYOBJECT */
    hashtype     hashvalue;     /* HASHOBJECT */
    settype      setvalue;      /* SETOBJECT */
    structtype   structvalue;   /* STRUCTOBJECT */
    filetype     filevalue;     /* FILEOBJECT */
    filedestype  filedesvalue;  /* FILEDESOBJECT */
    sockettype   socketvalue;   /* SOCKETOBJECT */
    polltype     pollvalue;     /* POLLOBJECT */
    listtype     listvalue;     /* LISTOBJECT, EXPROBJECT */
    wintype      winvalue;      /* WINOBJECT */
    objecttype   objvalue;      /* ENUMLITERALOBJECT, CONSTENUMOBJECT */
                                /* VARENUMOBJECT, VALUEPARAMOBJECT */
                                /* REFPARAMOBJECT, RESULTOBJECT */
                                /* LOCALVOBJECT, FORMPARAMOBJECT */
                                /* INTERFACEOBJECT */
    blocktype    blockvalue;    /* BLOCKOBJECT */
    acttype      actvalue;      /* ACTOBJECT */
    progtype     progvalue;     /* PROGOBJECT */
    databasetype databasevalue; /* DATABASEOBJECT */
    sqlstmttype  sqlstmtvalue;  /* SQLSTMTOBJECT */
#ifdef WITH_FLOAT
    floattype    floatvalue;    /* FLOATOBJECT */
#endif
  } valueunion;

typedef struct propertystruct {
    entitytype entity;
    listtype params;
    filenumtype file_number;
    linenumtype line;
    unsigned int syNumberInLine;
  } propertyrecord;

typedef union {
    propertytype property;
    postype posinfo;
  } descriptorunion;

typedef struct objectstruct {
    typetype type_of;
    descriptorunion descriptor;
    valueunion value;
    categorytype objcategory;
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
    progtype owningProg;
    objecttype inout_f_param_prototype;
    objecttype other_f_param_prototype;
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
    unsigned int bits;
    unsigned int mask;
    unsigned int table_size;
    memsizetype size;
    helemtype table[1];
  } hashrecord;

typedef struct structstruct {
    memsizetype usage_count;
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

typedef struct entrootstruct {
    entitytype literal;
    entitytype inactive_list;
  } entroottype;

typedef struct propertyrootstruct {
    propertytype literal;
  } propertyroottype;

typedef struct progstruct {
    uinttype usage_count;
    progtype owningProg;
    objecttype main_object;
    const_stritype arg0;
    const_stritype program_name;
    const_stritype program_path;
    objecttype arg_v;
    uinttype option_flags;
    unsigned int error_count;
    idroottype ident;
    findidtype id_for;
    entroottype entity;
    propertyroottype property;
    systype sys_var;
    listtype types;
    listtype literals;
    nodetype declaration_root;
    stacktype stack_global;
    stacktype stack_data;
    stacktype stack_current;
  } progrecord;

typedef struct infilstruct {
    FILE *fil;
    const_ustritype name_ustri;
    stritype name;
#ifdef USE_ALTERNATE_NEXT_CHARACTER
    ustritype start;
    const_ustritype nextch;
    const_ustritype beyond;
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
#ifdef WITH_COMPILATION_INFO
    booltype write_library_names;
    booltype write_line_numbers;
#endif
    linenumtype line;
    linenumtype incr_message_line;
    linenumtype next_msg_line;
    filenumtype file_number;
    progtype owningProg;
    booltype end_of_file;
  } infilrecord;


extern progrecord prog;


#define INIT_POS(O,L,F)     (O)->value.pos = ((postype) (L)) | (((postype) (F)) << 20)
#define GET_POS_LINE_NUM(O) (linenumtype) ((O)->value.pos & 1048575L)
#define GET_POS_FILE_NUM(O) (filenumtype) (((O)->value.pos & 2146435072L) >> 20)

#define CREATE_POSINFO(L,F) (((postype) (L)) | (((postype) (F)) << 20))
#define GET_LINE_NUM(O)     (linenumtype) (((long)(O)->descriptor.posinfo) & 1048575L)
#define GET_FILE_NUM(O)     (filenumtype) ((((long)(O)->descriptor.posinfo) & 2146435072L) >> 20)

#define CATEGORY_MASK  ((categorytype)   63)
#define VAR_MASK       ((categorytype)   64)
#define TEMP_MASK      ((categorytype)  128)
#define TEMP2_MASK     ((categorytype)  256)
#define POSINFO_MASK   ((categorytype)  512)
#define MATCH_ERR_MASK ((categorytype) 1024)
#define UNUSED_MASK    ((categorytype) 2048)

#define SET_ANY_FLAG(O,FLAG)            (O)->objcategory = (categorytype) ((O)->objcategory | (FLAG))

#define INIT_CATEGORY_OF_OBJ(O,CAT)     (O)->objcategory = (categorytype) (CAT)
#define SET_CATEGORY_OF_OBJ(O,CAT)      (O)->objcategory = (categorytype) (((O)->objcategory & ~CATEGORY_MASK) | (categorytype) (CAT))
#define CATEGORY_OF_OBJ(O)              ((objectcategory) ((O)->objcategory & CATEGORY_MASK))

#define INIT_CATEGORY_OF_VAR(O,CAT)     (O)->objcategory = (categorytype) ((CAT) | VAR_MASK)
#define SET_VAR_FLAG(O)                 (O)->objcategory = (categorytype) ((O)->objcategory | VAR_MASK)
#define CLEAR_VAR_FLAG(O)               (O)->objcategory = (categorytype) ((O)->objcategory & ~VAR_MASK)
#define VAR_OBJECT(O)                   ((O)->objcategory & VAR_MASK)
#define COPY_VAR_FLAG(O1,O2)            (O1)->objcategory = (categorytype) (((O1)->objcategory & ~VAR_MASK) | ((O2)->objcategory & VAR_MASK))

#define INIT_CATEGORY_OF_TEMP(O,CAT)    (O)->objcategory = (categorytype) ((CAT) | TEMP_MASK)
#define SET_TEMP_FLAG(O)                (O)->objcategory = (categorytype) ((O)->objcategory | TEMP_MASK)
#define CLEAR_TEMP_FLAG(O)              (O)->objcategory = (categorytype) ((O)->objcategory & ~TEMP_MASK)
#define TEMP_OBJECT(O)                  ((O)->objcategory & TEMP_MASK)

#define INIT_CATEGORY_OF_TEMP2(O,CAT)   (O)->objcategory = (categorytype) ((CAT) | TEMP2_MASK)
#define SET_TEMP2_FLAG(O)               (O)->objcategory = (categorytype) ((O)->objcategory | TEMP2_MASK)
#define CLEAR_TEMP2_FLAG(O)             (O)->objcategory = (categorytype) ((O)->objcategory & ~TEMP2_MASK)
#define TEMP2_OBJECT(O)                 ((O)->objcategory & TEMP2_MASK)

#define INIT_CATEGORY_OF_POSINFO(O,CAT) (O)->objcategory = (categorytype) ((CAT) | POSINFO_MASK)
#define SET_POSINFO_FLAG(O)             (O)->objcategory = (categorytype) ((O)->objcategory | POSINFO_MASK)
#define CLEAR_POSINFO_FLAG(O)           (O)->objcategory = (categorytype) ((O)->objcategory & ~POSINFO_MASK)
#define HAS_POSINFO(O)                  ((O)->objcategory & POSINFO_MASK)
#define COPY_POSINFO_FLAG(O1,O2)        (O1)->objcategory = (categorytype) (((O1)->objcategory & ~POSINFO_MASK) | ((O2)->objcategory & POSINFO_MASK))

#define SET_MATCH_ERR_FLAG(O)           (O)->objcategory = (categorytype) ((O)->objcategory | MATCH_ERR_MASK)
#define CLEAR_MATCH_ERR_FLAG(O)         (O)->objcategory = (categorytype) ((O)->objcategory & ~MATCH_ERR_MASK)
#define HAS_MATCH_ERR(O)                ((O)->objcategory & MATCH_ERR_MASK)

#define SET_UNUSED_FLAG(O)              (O)->objcategory = (categorytype) ((O)->objcategory | UNUSED_MASK)
#define CLEAR_UNUSED_FLAG(O)            (O)->objcategory = (categorytype) ((O)->objcategory & ~UNUSED_MASK)
#define IS_UNUSED(O)                    ((O)->objcategory & UNUSED_MASK)

#define HAS_PROPERTY(O)                 (!HAS_POSINFO(O) && (O)->descriptor.property != NULL)
#define HAS_ENTITY(O)                   (HAS_PROPERTY(O) && (O)->descriptor.property->entity != NULL)
#define GET_ENTITY(O)                   ((O)->descriptor.property->entity)
