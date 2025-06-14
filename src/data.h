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
    FWDREFOBJECT,        /* objValue -    Reference to Object which */
                         /*               was declared forward      */
    BLOCKOBJECT,         /* blockValue - Function possibly with     */
                         /*              parameters, declared       */
                         /*              result or local variables  */
    CALLOBJECT,          /* listValue - Subroutine call:            */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    MATCHOBJECT,         /* listValue - Don't exec subroutine call: */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    TYPEOBJECT,          /* typeValue -   type                      */
    FORMPARAMOBJECT,     /* objValue -    Reference to formal param */
    INTOBJECT,           /* intValue -    integer                   */
    BIGINTOBJECT,        /* bigIntValue - bigInteger                */
    CHAROBJECT,          /* charValue -   char                      */
    STRIOBJECT,          /* striValue -   string                    */
    BSTRIOBJECT,         /* bstriValue -  byte string               */
    ARRAYOBJECT,         /* arrayValue -  array                     */
    HASHOBJECT,          /* hashValue -   hash                      */
    HASHELEMOBJECT,      /* hashElemValue - hash table element      */
    STRUCTOBJECT,        /* structValue - struct                    */
    STRUCTELEMOBJECT,    /* objValue -    reference                 */
    CLASSOBJECT,         /* structValue - struct                    */
    INTERFACEOBJECT,     /* objValue -    Dynamic Object            */
    SETOBJECT,           /* setValue -    set                       */
    FILEOBJECT,          /* fileValue -   file                      */
    FILEDESOBJECT,       /* fileDesValue - file descriptor          */
    SOCKETOBJECT,        /* socketValue - socket                    */
    POLLOBJECT,          /* pollValue -   poll list                 */
    LISTOBJECT,          /* listValue -   list                      */
    FLOATOBJECT,         /* floatValue -  float                     */
    WINOBJECT,           /* winValue -    Window                    */
    POINTLISTOBJECT,     /* bstriValue -  Points of a polyline      */
    PROCESSOBJECT,       /* processValue - Process                  */
    ENUMLITERALOBJECT,   /* objValue -    Enumeration literal       */
    CONSTENUMOBJECT,     /* objValue -    Constant enumeration obj  */
    VARENUMOBJECT,       /* objValue -    Variable enumeration obj  */
    REFOBJECT,           /* objValue -    reference                 */
    REFLISTOBJECT,       /* listValue -   ref_list                  */
    EXPROBJECT,          /* listValue -   expression                */
    ACTOBJECT,           /* actValue -    Action                    */
    VALUEPARAMOBJECT,    /* objValue -    Formal value parameter    */
    REFPARAMOBJECT,      /* objValue -    Formal ref parameter      */
    RESULTOBJECT,        /* objValue -    Result of function        */
    LOCALVOBJECT,        /* objValue -    Local variable            */
    DATABASEOBJECT,      /* databaseValue - Database                */
    SQLSTMTOBJECT,       /* sqlStmtValue -  SQL statement           */
    PROGOBJECT           /* progValue -   Program                   */
  } objectCategory;

typedef enum {
    SY_TOKEN,
    EXPR_TOKEN,
    UNDEF_SYNTAX,
    LIST_WITH_TYPEOF_SYNTAX,
    SELECT_ELEMENT_FROM_LIST_SYNTAX
  } tokenCategory;

typedef uint32Type         posType;
typedef unsigned int       lineNumType;
typedef unsigned int       fileNumType;
typedef uint16Type         categoryType;

typedef struct identStruct      *identType;
typedef struct tokenStruct      *tokenType;
typedef struct nodeStruct       *nodeType;
typedef struct entityStruct     *entityType;
typedef struct ownerStruct      *ownerType;
typedef struct objectStruct     *objectType;
typedef struct propertyStruct   *propertyType;
typedef struct stackStruct      *stackType;
typedef struct typeListStruct   *typeListType;
typedef struct typeStruct       *typeType;
typedef struct listStruct       *listType;
typedef struct locObjStruct     *locObjType;
typedef struct locListStruct    *locListType;
typedef struct blockStruct      *blockType;
typedef struct arrayStruct      *arrayType;
typedef struct hashElemStruct   *hashElemType;
typedef struct hashStruct       *hashType;
typedef struct structStruct     *structType;
typedef struct parseErrorStruct *parseErrorType;
typedef struct progStruct       *progType;
typedef struct inFileStruct     *inFileType;

typedef const struct identStruct      *const_identType;
typedef const struct tokenStruct      *const_tokenType;
typedef const struct nodeStruct       *const_nodeType;
typedef const struct entityStruct     *const_entityType;
typedef const struct ownerStruct      *const_ownerType;
typedef const struct objectStruct     *const_objectType;
typedef const struct propertyStruct   *const_propertyType;
typedef const struct stackStruct      *const_stackType;
typedef const struct typeListStruct   *const_typeListType;
typedef const struct typeStruct       *const_typeType;
typedef const struct listStruct       *const_listType;
typedef const struct locObjStruct     *const_locObjType;
typedef const struct locListStruct    *const_locListType;
typedef const struct blockStruct      *const_blockType;
typedef const struct arrayStruct      *const_arrayType;
typedef const struct hashElemStruct   *const_hashElemType;
typedef const struct hashStruct       *const_hashType;
typedef const struct structStruct     *const_structType;
typedef const struct parseErrorStruct *const_parseErrorType;
typedef const struct progStruct       *const_progType;
typedef const struct inFileStruct     *const_inFileType;

typedef objectType (*actType) (listType);

typedef struct identStruct {
    ustriType name;
    sySizeType length;
    identType next1;
    identType next2;
    entityType entity;
    tokenType prefix_token;
    tokenType infix_token;
    priorityType prefix_priority;
    priorityType infix_priority;
    priorityType left_token_priority;
  } identRecord;

typedef struct tokenStruct {
    tokenType next;
    tokenType alternative;
    union {
      identType ident;
      struct {
        priorityType priority;
        typeType type_of;
      } expr_par;
      typeType type_of;
      intType select;
    } token_value;
    tokenCategory token_category;
  } tokenRecord;

typedef struct nodeStruct {
    uintType usage_count;
    objectType match_obj;
    nodeType next1;
    nodeType next2;
    entityType entity;
    nodeType symbol;
    nodeType inout_param;
    nodeType other_param;
    nodeType attr;
  } nodeRecord;

typedef struct entityStruct {
    identType ident;
    objectType syobject;
    listType fparam_list;
    union {
      ownerType owner;
      entityType next;
    } data;
  } entityRecord;

typedef struct ownerStruct {
    objectType obj;
    stackType decl_level;
    ownerType next;
  } ownerRecord;

typedef union {
    posType      pos;           /* SYMBOLOBJECT */
    typeType     typeValue;     /* TYPEOBJECT */
    intType      intValue;      /* INTOBJECT */
    bigIntType   bigIntValue;   /* BIGINTOBJECT */
    charType     charValue;     /* CHAROBJECT */
    striType     striValue;     /* STRIOBJECT */
    bstriType    bstriValue;    /* BSTRIOBJECT, POINTLISTOBJECT */
    arrayType    arrayValue;    /* ARRAYOBJECT */
    hashType     hashValue;     /* HASHOBJECT */
    hashElemType hashElemValue; /* HASHELEMOBJECT */
    setType      setValue;      /* SETOBJECT */
    structType   structValue;   /* STRUCTOBJECT */
    uintType     binaryValue;   /* INTOBJECT */
    fileType     fileValue;     /* FILEOBJECT */
    fileDesType  fileDesValue;  /* FILEDESOBJECT */
    socketType   socketValue;   /* SOCKETOBJECT */
    pollType     pollValue;     /* POLLOBJECT */
    listType     listValue;     /* LISTOBJECT, EXPROBJECT */
    winType      winValue;      /* WINOBJECT */
    processType  processValue;  /* PROCESSOBJECT */
    objectType   objValue;      /* ENUMLITERALOBJECT, CONSTENUMOBJECT */
                                /* VARENUMOBJECT, VALUEPARAMOBJECT */
                                /* REFPARAMOBJECT, RESULTOBJECT */
                                /* LOCALVOBJECT, FORMPARAMOBJECT */
                                /* INTERFACEOBJECT */
    blockType    blockValue;    /* BLOCKOBJECT */
    actType      actValue;      /* ACTOBJECT */
    databaseType databaseValue; /* DATABASEOBJECT */
    sqlStmtType  sqlStmtValue;  /* SQLSTMTOBJECT */
    progType     progValue;     /* PROGOBJECT */
#if WITH_FLOAT
    floatType    floatValue;    /* FLOATOBJECT */
#endif
  } valueUnion;

typedef struct propertyStruct {
    entityType entity;
    listType params;
    fileNumType file_number;
    lineNumType line;
    unsigned int syNumberInLine;
  } propertyRecord;

typedef union {
    propertyType property;
    posType posinfo;
  } descriptorUnion;

typedef struct objectStruct {
    typeType type_of;
    descriptorUnion descriptor;
    valueUnion value;
    categoryType objcategory;
  } objectRecord;

typedef struct stackStruct {
    listType local_object_list;
    listType *object_list_insert_place;
    stackType upward;
    stackType downward;
  } stackRecord;

typedef struct typeListStruct {
    typeListType next;
    typeType type_elem;
  } typeListRecord;

typedef enum {
    PARAM_UNDEFINED,
    PARAM_VALUE,
    PARAM_REF
  } parameterType;

typedef struct typeStruct {
    objectType match_obj;
    typeType meta;
    typeType func_type;
    typeType varfunc_type;
    typeType result_type;
    boolType is_varfunc_type;
    boolType is_type_type;
    parameterType in_param_type;
    typeListType interfaces;
    identType name;
    progType owningProg;
    objectType inout_f_param_prototype;
    objectType other_f_param_prototype;
    objectType create_call_obj;
    objectType destroy_call_obj;
    objectType copy_call_obj;
    objectType ord_call_obj;
    objectType in_call_obj;
    objectType value_obj;
  } typeRecord;

typedef struct listStruct {
    listType next;
    objectType obj;
  } listRecord;

typedef struct locObjStruct {
    objectType object;
    objectType init_value;
    objectType create_call_obj;
    objectType destroy_call_obj;
  } locObjRecord;

typedef struct locListStruct {
    locObjRecord local;
    locListType next;
  } locListRecord;

typedef struct blockStruct {
    memSizeType usage_count;
    locListType params;
    locObjRecord result;
    locListType local_vars;
    listType local_consts;
    objectType body;
  } blockRecord;

typedef struct arrayStruct {
    intType min_position;
    intType max_position;
#if WITH_ARRAY_CAPACITY
    memSizeType capacity;
#endif
    objectRecord arr[1];
  } arrayRecord;

typedef struct hashElemStruct {
    hashElemType next_less;
    hashElemType next_greater;
    objectRecord key;
    objectRecord data;
  } hashElemRecord;

typedef struct hashStruct {
    unsigned int bits;
    unsigned int mask;
    unsigned int table_size;
    memSizeType size;
    hashElemType table[1];
  } hashRecord;

typedef struct structStruct {
    memSizeType usage_count;
    memSizeType size;
    objectRecord stru[1];
  } structRecord;

#define NUMBER_OF_SYSVARS 32

typedef objectType sysType[NUMBER_OF_SYSVARS];

#define ID_TABLE_SIZE     1024

typedef struct {
    identType table[ID_TABLE_SIZE];
    identType table1[127];
    identType literal;
    identType end_of_file;
  } idRootType;

typedef struct {
    identType lparen;
    identType lbrack;
    identType lbrace;
    identType rparen;
    identType rbrack;
    identType rbrace;
    identType dot;
    identType colon;
    identType comma;
    identType semicolon;
    identType dollar;
    identType r_arrow;
    identType l_arrow;
    identType out_arrow;
    identType in_arrow;
    identType type;
    identType constant;
    identType ref;
    identType syntax;
    identType system;
    identType include;
    identType is;
    identType func;
    identType param;
    identType enumlit;
    identType subtype;
    identType newtype;
    identType action;
  } findIdType;

typedef struct {
    entityType literal;
    entityType inactive_list;
  } entityRootType;

typedef struct {
    propertyType literal;
  } propertyRootType;

typedef struct parseErrorStruct {
    int err;
    const_striType fileName;
    lineNumType lineNumber;
    memSizeType columnNumber;
    striType msg;
    striType errorLine;
    parseErrorType next;
  } parseErrorRecord;

typedef struct progStruct {
    uintType usage_count;
    objectType main_object;
    const_striType arg0;
    const_striType program_name;
    const_striType program_path;
    objectType arg_v;
    uintType option_flags;
    fileNumType fileCounter;
    inFileType fileList;
    boolType writeErrors;
    fileType errorFile;
    unsigned int error_count;
    parseErrorType errorList;
    parseErrorType errorListEnd;
    void *includeFileHash;
    idRootType ident;
    findIdType id_for;
    entityRootType entity;
    propertyRootType property;
    sysType sys_var;
    listType types;
    listType literals;
    nodeType declaration_root;
    stackType stack_global;
    stackType stack_data;
    stackType stack_current;
    void *objectNumberMap;
    intType nextFreeObjectNumber;
    void *typeNumberMap;
    intType nextFreeTypeNumber;
    listType exec_expr_temp_results;
    listType substituted_objects;
    listType struct_objects;
    void *structSymbolsMap;
    listType struct_symbols;
    listType when_value_objects;
    listType when_set_objects;
    listType match_expr_objects;
  } progRecord;

typedef struct inFileStruct {
    FILE *fil;
    const_ustriType name_ustri;
    striType name;
    striType path;
#if USE_ALTERNATE_NEXT_CHARACTER
    ustriType start;
    const_ustriType nextch;
    const_ustriType beyond;
    memSizeType buffer_size;
#else
#if USE_INFILE_BUFFER
    ustriType buffer;
#endif
#endif
    int character;
    inFileType curr_infile;
    inFileType up_infile;
    inFileType next;
#if WITH_COMPILATION_INFO
    boolType write_library_names;
    boolType write_line_numbers;
#endif
    lineNumType line;
    lineNumType incr_message_line;
    lineNumType next_msg_line;
    fileNumType file_number;
    boolType end_of_file;
  } inFileRecord;


extern progType prog;


#define INIT_POS(O,L,F)     (O)->value.pos = ((posType) (L)) | (((posType) (F)) << 20)
#define GET_POS_LINE_NUM(O) (lineNumType) ((O)->value.pos & 1048575L)
#define GET_POS_FILE_NUM(O) (fileNumType) (((O)->value.pos & 2146435072L) >> 20)

#define CREATE_POSINFO(L,F) (((posType) (L)) | (((posType) (F)) << 20))
#define GET_LINE_NUM(O)     (lineNumType) (((long)(O)->descriptor.posinfo) & 1048575L)
#define GET_FILE_NUM(O)     (fileNumType) ((((long)(O)->descriptor.posinfo) & 2146435072L) >> 20)

#define CATEGORY_MASK  ((categoryType)   63)
#define VAR_MASK       ((categoryType)   64)
#define TEMP_MASK      ((categoryType)  128)
#define TEMP2_MASK     ((categoryType)  256)
#define POSINFO_MASK   ((categoryType)  512)
#define MATCH_ERR_MASK ((categoryType) 1024)
#define UNUSED_MASK    ((categoryType) 2048)

#define MAX_CATEGORY_FIELD_VALUE ((categoryType) 4095)
#define LEGAL_CATEGORY_FIELD(O)  ((objectCategory) ((O)->objcategory <= MAX_CATEGORY_FIELD_VALUE))

#define SET_ANY_FLAG(O,FLAG)            (O)->objcategory = (categoryType) ((O)->objcategory | (FLAG))

#define INIT_CATEGORY_OF_OBJ(O,CAT)     (O)->objcategory = (categoryType) (CAT)
#define SET_CATEGORY_OF_OBJ(O,CAT)      (O)->objcategory = (categoryType) (((O)->objcategory & ~CATEGORY_MASK) | (categoryType) (CAT))
#define CATEGORY_OF_OBJ(O)              ((objectCategory) ((O)->objcategory & CATEGORY_MASK))

#define INIT_CATEGORY_OF_VAR(O,CAT)     (O)->objcategory = (categoryType) ((CAT) | VAR_MASK)
#define SET_VAR_FLAG(O)                 (O)->objcategory = (categoryType) ((O)->objcategory | VAR_MASK)
#define CLEAR_VAR_FLAG(O)               (O)->objcategory = (categoryType) ((O)->objcategory & ~VAR_MASK)
#define VAR_OBJECT(O)                   ((O)->objcategory & VAR_MASK)
#define COPY_VAR_FLAG(O1,O2)            (O1)->objcategory = (categoryType) (((O1)->objcategory & ~VAR_MASK) | ((O2)->objcategory & VAR_MASK))

#define INIT_CATEGORY_OF_TEMP(O,CAT)    (O)->objcategory = (categoryType) ((CAT) | TEMP_MASK)
#define SET_TEMP_FLAG(O)                (O)->objcategory = (categoryType) ((O)->objcategory | TEMP_MASK)
#define CLEAR_TEMP_FLAG(O)              (O)->objcategory = (categoryType) ((O)->objcategory & ~TEMP_MASK)
#define TEMP_OBJECT(O)                  ((O)->objcategory & TEMP_MASK)

#define INIT_CATEGORY_OF_TEMP2(O,CAT)   (O)->objcategory = (categoryType) ((CAT) | TEMP2_MASK)
#define SET_TEMP2_FLAG(O)               (O)->objcategory = (categoryType) ((O)->objcategory | TEMP2_MASK)
#define CLEAR_TEMP2_FLAG(O)             (O)->objcategory = (categoryType) ((O)->objcategory & ~TEMP2_MASK)
#define TEMP2_OBJECT(O)                 ((O)->objcategory & TEMP2_MASK)

#define INIT_CATEGORY_OF_POSINFO(O,CAT) (O)->objcategory = (categoryType) ((CAT) | POSINFO_MASK)
#define SET_POSINFO_FLAG(O)             (O)->objcategory = (categoryType) ((O)->objcategory | POSINFO_MASK)
#define CLEAR_POSINFO_FLAG(O)           (O)->objcategory = (categoryType) ((O)->objcategory & ~POSINFO_MASK)
#define HAS_POSINFO(O)                  ((O)->objcategory & POSINFO_MASK)
#define COPY_POSINFO_FLAG(O1,O2)        (O1)->objcategory = (categoryType) (((O1)->objcategory & ~POSINFO_MASK) | ((O2)->objcategory & POSINFO_MASK))

#define SET_MATCH_ERR_FLAG(O)           (O)->objcategory = (categoryType) ((O)->objcategory | MATCH_ERR_MASK)
#define CLEAR_MATCH_ERR_FLAG(O)         (O)->objcategory = (categoryType) ((O)->objcategory & ~MATCH_ERR_MASK)
#define HAS_MATCH_ERR(O)                ((O)->objcategory & MATCH_ERR_MASK)

#define SET_UNUSED_FLAG(O)              (O)->objcategory = (categoryType) ((O)->objcategory | UNUSED_MASK)
#define CLEAR_UNUSED_FLAG(O)            (O)->objcategory = (categoryType) ((O)->objcategory & ~UNUSED_MASK)
#define IS_UNUSED(O)                    ((O)->objcategory & UNUSED_MASK)

#define HAS_PROPERTY(O)                 (!HAS_POSINFO(O) && (O)->descriptor.property != NULL)
#define HAS_ENTITY(O)                   (HAS_PROPERTY(O) && (O)->descriptor.property->entity != NULL)
#define GET_ENTITY(O)                   ((O)->descriptor.property->entity)
