# Makefile for make and gcc from DJGPP. Commands executed by: cmd.exe
# To compile use a windows console and call:
#   make -f mk_djgpp.mak depend
#   make -f mk_djgpp.mak
# If your get errors you can try mk_djgp2.mak instead.

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
# Since there is no linker option to determine the stack size
# it is determined with STACK_SIZE_DEFINITION (see below).
LDFLAGS =
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -lm
# SYSTEM_LIBS = -lm -lgmp
SYSTEM_CONSOLE_LIBS =
SYSTEM_DRAW_LIBS =
SEED7_LIB = seed7_05.a
CONSOLE_LIB = s7_con.a
DRAW_LIB = s7_draw.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(DRAW_LIB) ..\bin\$(CONSOLE_LIB) ..\bin\$(SEED7_LIB)
# CC = g++
CC = gcc
GET_CC_VERSION_INFO = $(CC) --version >
ECHO = djecho

BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
BIGINT_LIB = big_rtl
# BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
# BIGINT_LIB = big_gmp

MOBJ = s7.o
POBJ = runerr.o option.o primitiv.o
LOBJ = actlib.o arrlib.o biglib.o binlib.o blnlib.o bstlib.o chrlib.o cmdlib.o conlib.o dcllib.o \
       drwlib.o enulib.o fillib.o fltlib.o hshlib.o intlib.o itflib.o kbdlib.o lstlib.o pollib.o \
       prclib.o prglib.o reflib.o rfllib.o sctlib.o setlib.o soclib.o strlib.o timlib.o typlib.o \
       ut8lib.o
EOBJ = exec.o doany.o objutl.o
AOBJ = act_comp.o prg_comp.o analyze.o syntax.o token.o parser.o name.o type.o \
       expr.o atom.o object.o scanner.o literal.o numlit.o findid.o \
       error.o infile.o libpath.o symbol.o info.o stat.o fatal.o match.o
GOBJ = syvarutl.o traceutl.o actutl.o executl.o blockutl.o \
       entutl.o identutl.o chclsutl.o sigutl.o arrutl.o
ROBJ = arr_rtl.o bln_rtl.o bst_rtl.o chr_rtl.o cmd_rtl.o con_rtl.o dir_rtl.o drw_rtl.o fil_rtl.o \
       flt_rtl.o hsh_rtl.o int_rtl.o itf_rtl.o set_rtl.o soc_dos.o str_rtl.o tim_rtl.o ut8_rtl.o \
       heaputl.o striutl.o
DOBJ = $(BIGINT_LIB).o cmd_unx.o fil_dos.o pol_dos.o tim_dos.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_dos.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_wat.o
COMP_DATA_LIB_OBJ = typ_data.o rfl_data.o ref_data.o listutl.o flistutl.o typeutl.o datautl.o
COMPILER_LIB_OBJ = $(POBJ) $(LOBJ) $(EOBJ) $(AOBJ) $(GOBJ)

MSRC = s7.c
PSRC = runerr.c option.c primitiv.c
LSRC = actlib.c arrlib.c biglib.c binlib.c blnlib.c bstlib.c chrlib.c cmdlib.c conlib.c dcllib.c \
       drwlib.c enulib.c fillib.c fltlib.c hshlib.c intlib.c itflib.c kbdlib.c lstlib.c pollib.c \
       prclib.c prglib.c reflib.c rfllib.c sctlib.c setlib.c soclib.c strlib.c timlib.c typlib.c \
       ut8lib.c
ESRC = exec.c doany.c objutl.c
ASRC = act_comp.c prg_comp.c analyze.c syntax.c token.c parser.c name.c type.c \
       expr.c atom.c object.c scanner.c literal.c numlit.c findid.c \
       error.c infile.c libpath.c symbol.c info.c stat.c fatal.c match.c
GSRC = syvarutl.c traceutl.c actutl.c executl.c blockutl.c \
       entutl.c identutl.c chclsutl.c sigutl.c arrutl.c
RSRC = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c con_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c \
       flt_rtl.c hsh_rtl.c int_rtl.c itf_rtl.c set_rtl.c soc_dos.c str_rtl.c tim_rtl.c ut8_rtl.c \
       heaputl.c striutl.c
DSRC = $(BIGINT_LIB).c cmd_unx.c fil_dos.c pol_dos.c tim_dos.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_dos.c
CONSOLE_LIB_SRC = kbd_rtl.c con_wat.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ..\bin\s7.exe ..\prg\s7.exe
	..\bin\s7 -l ..\lib level
	$(ECHO) "  Use 'make s7c' (with your make command) to create the compiler."

s7c: ..\bin\s7c.exe ..\prg\s7c.exe
	$(ECHO) "  Use 'make test' (with your make command) to check Seed7."

..\bin\s7.exe: $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) -o ..\bin\s7.exe

..\prg\s7.exe: ..\bin\s7.exe
	copy ..\bin\s7.exe ..\prg

..\bin\s7c.exe: ..\prg\s7c.exe
	copy ..\prg\s7c.exe ..\bin /Y

..\prg\s7c.exe: ..\prg\s7c.sd7 $(ALL_S7_LIBS)
	..\bin\s7 -l ..\lib ..\prg\s7c -l ..\lib -b ..\bin -O2 ..\prg\s7c

clear: clean

clean:
	del *.o
	del ..\bin\*.a
	del ..\bin\s7.exe
	del ..\bin\s7c.exe
	del ..\prg\s7.exe
	del ..\prg\s7c.exe
	del depend
	del chkccomp.h
	del version.h
	$(ECHO) "  Use 'make depend' (with your make command) to create the dependencies."

distclean: clean
	copy level_bk.h level.h /Y

test:
	..\bin\s7 -l ..\lib ..\prg\chk_all build

dep: depend

strip:
	strip ..\bin\s7.exe

chkccomp.h:
	$(ECHO) "#include \"direct.h\"" > chkccomp.h
	$(ECHO) "#include \"unistd.h\"" >> chkccomp.h
	$(ECHO) "#define WRITE_CC_VERSION_INFO system(\"$(GET_CC_VERSION_INFO) cc_vers.txt\");" >> chkccomp.h
	$(ECHO) "#define LIST_DIRECTORY_CONTENTS \"dir\"" >> chkccomp.h

version.h: chkccomp.h
	$(ECHO) "#define PATH_DELIMITER 92 /* backslash (ASCII) */" > version.h
	$(ECHO) "#define USE_DIRENT" >> version.h
	$(ECHO) "#define SEARCH_PATH_DELIMITER ';'" >> version.h
	$(ECHO) "#define OS_PATH_HAS_DRIVE_LETTERS" >> version.h
	$(ECHO) "#define CATCH_SIGNALS" >> version.h
	$(ECHO) "#define AWAIT_WITH_SELECT" >> version.h
	$(ECHO) "#define IMPLEMENT_PTY_WITH_PIPE2" >> version.h
	$(ECHO) "#define OS_STRI_USES_CODE_PAGE" >> version.h
	$(ECHO) "#define os_lstat stat" >> version.h
	$(ECHO) "#define os_fseek fseek" >> version.h
	$(ECHO) "#define os_ftell ftell" >> version.h
	$(ECHO) "#define OS_FSEEK_OFFSET_BITS 32" >> version.h
	$(ECHO) "#define os_off_t off_t" >> version.h
	$(ECHO) "#define os_environ environ" >> version.h
	$(ECHO) "#define os_putenv putenv" >> version.h
	$(ECHO) "#define $(BIGINT_LIB_DEFINE)" >> version.h
	$(ECHO) "#define OBJECT_FILE_EXTENSION \".o\"" >> version.h
	$(ECHO) "#define LIBRARY_FILE_EXTENSION \".a\"" >> version.h
	$(ECHO) "#define EXECUTABLE_FILE_EXTENSION \".exe\"" >> version.h
	$(ECHO) "#define C_COMPILER \"$(CC)\"" >> version.h
	$(ECHO) "#define GET_CC_VERSION_INFO \"$(GET_CC_VERSION_INFO)\"" >> version.h
	$(ECHO) "#define CC_OPT_DEBUG_INFO \"-g\"" >> version.h
	$(ECHO) "#define CC_OPT_NO_WARNINGS \"-w\"" >> version.h
	$(ECHO) "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> version.h
	$(ECHO) "#define LINKER_FLAGS \"$(LDFLAGS)\"" >> version.h
	$(CC) chkccomp.c -lm -o chkccomp.exe
	$(ECHO) "The following C compiler errors can be safely ignored"
	.\chkccomp.exe version.h
	del chkccomp.exe
	del cc_vers.txt
	$(ECHO) "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> version.h
	$(ECHO) "#define SYSTEM_CONSOLE_LIBS \"$(SYSTEM_CONSOLE_LIBS)\"" >> version.h
	$(ECHO) "#define SYSTEM_DRAW_LIBS \"$(SYSTEM_DRAW_LIBS)\"" >> version.h
	$(ECHO) "#define SEED7_LIB \"$(SEED7_LIB)\"" >> version.h
	$(ECHO) "#define CONSOLE_LIB \"$(CONSOLE_LIB)\"" >> version.h
	$(ECHO) "#define DRAW_LIB \"$(DRAW_LIB)\"" >> version.h
	$(ECHO) "#define COMP_DATA_LIB \"$(COMP_DATA_LIB)\"" >> version.h
	$(ECHO) "#define COMPILER_LIB \"$(COMPILER_LIB)\"" >> version.h
	$(ECHO) "#define STACK_SIZE_DEFINITION unsigned _stklen = 4194304" >> version.h
	$(CC) setpaths.c -o setpaths.exe
	.\setpaths.exe S7_LIB_DIR=$(S7_LIB_DIR) SEED7_LIBRARY=$(SEED7_LIBRARY) >> version.h
	del setpaths.exe

depend: version.h
	$(ECHO) "Working without C header dependency checks."
	$(ECHO) "  Use 'make' (with your make command) to create the interpreter."

level.h:
	..\bin\s7 -l ..\lib level

..\bin\$(SEED7_LIB): $(SEED7_LIB_OBJ)
	ar r ..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	ar r ..\bin\$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

..\bin\$(DRAW_LIB): $(DRAW_LIB_OBJ)
	ar r ..\bin\$(DRAW_LIB) $(DRAW_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r ..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r ..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

make7: ..\bin\make7.exe

..\bin\make7.exe: ..\prg\make7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\make7
	copy ..\prg\make7.exe ..\bin /Y
	del ..\prg\make7.exe

wc: $(SRC)
	$(ECHO) SRC:
	wc $(SRC)
	$(ECHO) SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	$(ECHO) CONSOLE_LIB_SRC:
	wc $(CONSOLE_LIB_SRC)
	$(ECHO) DRAW_LIB_SRC:
	wc $(DRAW_LIB_SRC)
	$(ECHO) COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	$(ECHO) COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS)
