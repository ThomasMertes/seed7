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
# SYSTEM_BIGINT_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SYSTEM_CONSOLE_LIBS =
# SYSTEM_DATABASE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SYSTEM_DRAW_LIBS =
SYSTEM_MATH_LIBS =
ALL_SYSTEM_LIBS = $(SYSTEM_LIBS) $(SYSTEM_BIGINT_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_MATH_LIBS)
SEED7_LIB = seed7_05.a
DRAW_LIB = s7_draw.a
CONSOLE_LIB = s7_con.a
DATABASE_LIB = s7_db.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(DRAW_LIB) ..\bin\$(CONSOLE_LIB) ..\bin\$(DATABASE_LIB) ..\bin\$(SEED7_LIB)
# CC = g++
CC = gcc
ECHO = djecho

MOBJ = s7.o
POBJ = runerr.o option.o primitiv.o
LOBJ = actlib.o arrlib.o biglib.o binlib.o blnlib.o bstlib.o chrlib.o cmdlib.o conlib.o dcllib.o \
       drwlib.o enulib.o fillib.o fltlib.o hshlib.o intlib.o itflib.o kbdlib.o lstlib.o pcslib.o \
       pollib.o prclib.o prglib.o reflib.o rfllib.o sctlib.o sellib.o setlib.o soclib.o sqllib.o \
       strlib.o timlib.o typlib.o ut8lib.o
EOBJ = exec.o doany.o objutl.o
AOBJ = act_comp.o prg_comp.o analyze.o syntax.o token.o parser.o name.o type.o \
       expr.o atom.o object.o scanner.o literal.o numlit.o findid.o msg_stri.o \
       error.o infile.o libpath.o symbol.o info.o stat.o fatal.o match.o
GOBJ = syvarutl.o traceutl.o actutl.o executl.o blockutl.o \
       entutl.o identutl.o chclsutl.o arrutl.o
ROBJ = arr_rtl.o bln_rtl.o bst_rtl.o chr_rtl.o cmd_rtl.o con_rtl.o dir_rtl.o drw_rtl.o fil_rtl.o \
       flt_rtl.o hsh_rtl.o int_rtl.o itf_rtl.o pcs_rtl.o set_rtl.o soc_rtl.o sql_rtl.o str_rtl.o \
       tim_rtl.o ut8_rtl.o heaputl.o numutl.o sigutl.o striutl.o
DOBJ = big_rtl.o big_gmp.o cmd_unx.o dir_win.o dll_dos.o fil_dos.o pcs_dos.o pol_dos.o soc_none.o \
       tim_dos.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_dos.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_wat.o
DATABASE_LIB_OBJ = sql_base.o sql_db2.o sql_fire.o sql_ifx.o sql_lite.o sql_my.o sql_oci.o \
                   sql_odbc.o sql_post.o sql_srv.o sql_tds.o
COMP_DATA_LIB_OBJ = typ_data.o rfl_data.o ref_data.o listutl.o flistutl.o typeutl.o datautl.o
COMPILER_LIB_OBJ = $(POBJ) $(LOBJ) $(EOBJ) $(AOBJ) $(GOBJ)

MSRC = s7.c
PSRC = runerr.c option.c primitiv.c
LSRC = actlib.c arrlib.c biglib.c binlib.c blnlib.c bstlib.c chrlib.c cmdlib.c conlib.c dcllib.c \
       drwlib.c enulib.c fillib.c fltlib.c hshlib.c intlib.c itflib.c kbdlib.c lstlib.c pcslib.c \
       pollib.c prclib.c prglib.c reflib.c rfllib.c sctlib.c sellib.c setlib.c soclib.c sqllib.c \
       strlib.c timlib.c typlib.c ut8lib.c
ESRC = exec.c doany.c objutl.c
ASRC = act_comp.c prg_comp.c analyze.c syntax.c token.c parser.c name.c type.c \
       expr.c atom.c object.c scanner.c literal.c numlit.c findid.c msg_stri.c \
       error.c infile.c libpath.c symbol.c info.c stat.c fatal.c match.c
GSRC = syvarutl.c traceutl.c actutl.c executl.c blockutl.c \
       entutl.c identutl.c chclsutl.c arrutl.c
RSRC = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c con_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c \
       flt_rtl.c hsh_rtl.c int_rtl.c itf_rtl.c pcs_rtl.c set_rtl.c soc_rtl.c sql_rtl.c str_rtl.c \
       tim_rtl.c ut8_rtl.c heaputl.c numutl.c sigutl.c striutl.c
DSRC = big_rtl.c big_gmp.c cmd_unx.c dir_win.c dll_unx.c fil_dos.c pcs_dos.c pol_dos.c soc_none.c \
       tim_dos.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_dos.c
CONSOLE_LIB_SRC = kbd_rtl.c con_wat.c
DATABASE_LIB_SRC = sql_base.c sql_db2.c sql_fire.c sql_ifx.c sql_lite.c sql_my.c sql_oci.c \
                   sql_odbc.c sql_post.c sql_srv.c sql_tds.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ..\bin\s7.exe ..\prg\s7.exe
	..\bin\s7 -l ..\lib level
	@$(ECHO)
	@$(ECHO) "  Use 'make s7c' (with your make command) to create the compiler."
	@$(ECHO)

s7c: ..\bin\s7c.exe ..\prg\s7c.exe
	@$(ECHO)
	@$(ECHO) "  Use 'make test' (with your make command) to check Seed7."
	@$(ECHO)

..\bin\s7.exe: levelup.exe next_lvl $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(ALL_SYSTEM_LIBS) -o ..\bin\s7.exe
	del next_lvl

..\prg\s7.exe: ..\bin\s7.exe
	copy ..\bin\s7.exe ..\prg

..\bin\s7c.exe: ..\prg\s7c.exe
	copy ..\prg\s7c.exe ..\bin /Y

..\prg\s7c.exe: ..\prg\s7c.sd7 $(ALL_S7_LIBS)
	..\bin\s7 -l ..\lib ..\prg\s7c -l ..\lib -b ..\bin -O2 ..\prg\s7c

levelup.exe: levelup.c
	$(CC) levelup.c -o levelup

next_lvl: levelup.exe
	.\levelup.exe
	echo X > next_lvl

sql_db2.o: sql_db2.c
	$(CC) -c $(CPPFLAGS) $(DB2_INCLUDE_OPTION) $(CFLAGS) $< -o $@

sql_ifx.o: sql_ifx.c
	$(CC) -c $(CPPFLAGS) $(INFORMIX_INCLUDE_OPTION) $(CFLAGS) $< -o $@

sql_srv.o: sql_srv.c
	$(CC) -c $(CPPFLAGS) $(SQL_SERVER_INCLUDE_OPTION) $(CFLAGS) $< -o $@

clean:
	del *.o
	del ..\bin\$(COMPILER_LIB)
	del ..\bin\$(COMP_DATA_LIB)
	del ..\bin\$(DRAW_LIB)
	del ..\bin\$(CONSOLE_LIB)
	del ..\bin\$(DATABASE_LIB)
	del ..\bin\$(SEED7_LIB)
	del ..\bin\s7.exe
	del ..\bin\s7c.exe
	del ..\prg\s7.exe
	del ..\prg\s7c.exe
	del depend
	del chkccomp.h
	del base.h
	del settings.h
	del version.h
	del chkccomp.exe
	del levelup.exe
	del next_lvl
	@$(ECHO)
	@$(ECHO) "  Use 'make depend' (with your make command) to create the dependencies."
	@$(ECHO)

clean_utils:
	del ..\bin\bas7.exe
	del ..\bin\bigfiles.exe
	del ..\bin\calc7.exe
	del ..\bin\cat.exe
	del ..\bin\comanche.exe
	del ..\bin\db7.exe
	del ..\bin\diff7.exe
	del ..\bin\find7.exe
	del ..\bin\findchar.exe
	del ..\bin\ftp7.exe
	del ..\bin\ftpserv.exe
	del ..\bin\hd.exe
	del ..\bin\ide7.exe
	del ..\bin\make7.exe
	del ..\bin\portfwd7.exe
	del ..\bin\pv7.exe
	del ..\bin\s7check.exe
	del ..\bin\sql7.exe
	del ..\bin\sydir7.exe
	del ..\bin\tar7.exe
	del ..\bin\toutf8.exe
	del ..\bin\which.exe

distclean: clean clean_utils
	del level_bk.h
	del level.h
	del vers_djg.h

test:
	..\bin\s7 -l ..\lib ..\prg\chk_all build

check: test

strip:
	strip ..\bin\s7.exe

chkccomp.h:
	$(ECHO) "#define LIST_DIRECTORY_CONTENTS \"dir\"" > chkccomp.h
	$(ECHO) "#define UNIX_DO_SLEEP" >> chkccomp.h
	$(ECHO) "#define ERROR_REDIRECTING_FAILS" >> chkccomp.h
	$(ECHO) "#define LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION 512" >> chkccomp.h
	$(ECHO) "#define USE_GMP 0" >> chkccomp.h
	$(ECHO) "#define SYSTEM_CONSOLE_LIBS \"$(SYSTEM_CONSOLE_LIBS)\"" >> chkccomp.h
	$(ECHO) "#define SYSTEM_DRAW_LIBS \"$(SYSTEM_DRAW_LIBS)\"" >> chkccomp.h

base.h:
	$(ECHO) "#define PATH_DELIMITER 92 /* backslash (ASCII) */" > base.h
	$(ECHO) "#define OBJECT_FILE_EXTENSION \".o\"" >> base.h
	$(ECHO) "#define EXECUTABLE_FILE_EXTENSION \".exe\"" >> base.h
	$(ECHO) "#define C_COMPILER \"$(CC)\"" >> base.h
	$(ECHO) "#define CC_OPT_VERSION_INFO \"--version\"" >> base.h
	$(ECHO) "#define CC_FLAGS \"\"" >> base.h
	$(ECHO) "#define CC_ERROR_FILEDES 0" >> base.h
	$(ECHO) "#define CC_VERSION_INFO_FILEDES 1" >> base.h
	$(ECHO) "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> base.h
	$(ECHO) "#define OS_STRI_USES_CODE_PAGE" >> base.h
	$(ECHO) "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> base.h
	$(ECHO) "#define SYSTEM_MATH_LIBS \"$(SYSTEM_MATH_LIBS)\"" >> base.h

settings.h:
	$(ECHO) "#define MAKE_UTILITY_NAME \"$(MAKE)\"" > settings.h
	$(ECHO) "#define MAKEFILE_NAME \"mk_djgpp.mak\"" >> settings.h
	$(ECHO) "#define SEARCH_PATH_DELIMITER ';'" >> settings.h
	$(ECHO) "#define AWAIT_WITH_SELECT" >> settings.h
	$(ECHO) "#define IMPLEMENT_PTY_WITH_PIPE2" >> settings.h
	$(ECHO) "#define MAP_LONG_FILE_NAMES_TO_SHORT" >> settings.h
	$(ECHO) "#define USE_CONSOLE_FOR_PROT_CSTRI" >> settings.h
	$(ECHO) "#define LIBRARY_FILE_EXTENSION \".a\"" >> settings.h
	$(ECHO) "#define CALL_C_COMPILER_FROM_SHELL 1" >> settings.h
	$(ECHO) "#define CC_OPT_DEBUG_INFO \"-g\"" >> settings.h
	$(ECHO) "#define CC_OPT_NO_WARNINGS \"-w\"" >> settings.h
	$(ECHO) "#define LINKER_FLAGS \"$(LDFLAGS)\"" >> settings.h
	$(ECHO) "#define SEED7_LIB \"$(SEED7_LIB)\"" >> settings.h
	$(ECHO) "#define DRAW_LIB \"$(DRAW_LIB)\"" >> settings.h
	$(ECHO) "#define CONSOLE_LIB \"$(CONSOLE_LIB)\"" >> settings.h
	$(ECHO) "#define DATABASE_LIB \"$(DATABASE_LIB)\"" >> settings.h
	$(ECHO) "#define COMP_DATA_LIB \"$(COMP_DATA_LIB)\"" >> settings.h
	$(ECHO) "#define COMPILER_LIB \"$(COMPILER_LIB)\"" >> settings.h
	$(ECHO) "#define STACK_SIZE_DEFINITION unsigned _stklen = 4194304" >> settings.h

version.h: chkccomp.exe base.h settings.h
	@$(ECHO)
	@$(ECHO) "The following C compiler errors can be safely ignored"
	.\chkccomp.exe version.h S7_LIB_DIR=$(S7_LIB_DIR) SEED7_LIBRARY=$(SEED7_LIBRARY)
	copy version.h vers_djg.h /Y

chkccomp.exe: chkccomp.c chkccomp.h base.h settings.h
	$(CC) chkccomp.c -o chkccomp.exe

depend: version.h
	@$(ECHO) "Working without C header dependency checks."
	@$(ECHO)
	@$(ECHO) "  Use 'make' (with your make command) to create the interpreter."
	@$(ECHO)

..\bin\$(SEED7_LIB): $(SEED7_LIB_OBJ)
	ar r ..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(DRAW_LIB): $(DRAW_LIB_OBJ)
	ar r ..\bin\$(DRAW_LIB) $(DRAW_LIB_OBJ)

..\bin\$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	ar r ..\bin\$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

..\bin\$(DATABASE_LIB): $(DATABASE_LIB_OBJ)
	ar r ..\bin\$(DATABASE_LIB) $(DATABASE_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r ..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r ..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

..\bin\bas7.exe: ..\prg\bas7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\bas7
	copy ..\prg\bas7.exe ..\bin /Y
	del ..\prg\bas7.exe

..\bin\bigfiles.exe: ..\prg\bigfiles.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\bigfiles
	copy ..\prg\bigfiles.exe ..\bin /Y
	del ..\prg\bigfiles.exe

..\bin\calc7.exe: ..\prg\calc7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\calc7
	copy ..\prg\calc7.exe ..\bin /Y
	del ..\prg\calc7.exe

..\bin\cat.exe: ..\prg\cat.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\cat
	copy ..\prg\cat.exe ..\bin /Y
	del ..\prg\cat.exe

..\bin\comanche.exe: ..\prg\comanche.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\comanche
	copy ..\prg\comanche.exe ..\bin /Y
	del ..\prg\comanche.exe

..\bin\db7.exe: ..\prg\db7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\db7
	copy ..\prg\db7.exe ..\bin /Y
	del ..\prg\db7.exe

..\bin\diff7.exe: ..\prg\diff7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\diff7
	copy ..\prg\diff7.exe ..\bin /Y
	del ..\prg\diff7.exe

..\bin\find7.exe: ..\prg\find7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\find7
	copy ..\prg\find7.exe ..\bin /Y
	del ..\prg\find7.exe

..\bin\findchar.exe: ..\prg\findchar.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\findchar
	copy ..\prg\findchar.exe ..\bin /Y
	del ..\prg\findchar.exe

..\bin\ftp7.exe: ..\prg\ftp7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\ftp7
	copy ..\prg\ftp7.exe ..\bin /Y
	del ..\prg\ftp7.exe

..\bin\ftpserv.exe: ..\prg\ftpserv.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\ftpserv
	copy ..\prg\ftpserv.exe ..\bin /Y
	del ..\prg\ftpserv.exe

..\bin\hd.exe: ..\prg\hd.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\hd
	copy ..\prg\hd.exe ..\bin /Y
	del ..\prg\hd.exe

..\bin\ide7.exe: ..\prg\ide7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\ide7
	copy ..\prg\ide7.exe ..\bin /Y
	del ..\prg\ide7.exe

..\bin\make7.exe: ..\prg\make7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\make7
	copy ..\prg\make7.exe ..\bin /Y
	del ..\prg\make7.exe

..\bin\portfwd7.exe: ..\prg\portfwd7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\portfwd7
	copy ..\prg\portfwd7.exe ..\bin /Y
	del ..\prg\portfwd7.exe

..\bin\pv7.exe: ..\prg\pv7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\pv7
	copy ..\prg\pv7.exe ..\bin /Y
	del ..\prg\pv7.exe

..\bin\s7check.exe: ..\prg\s7check.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\s7check
	copy ..\prg\s7check.exe ..\bin /Y
	del ..\prg\s7check.exe

..\bin\sql7.exe: ..\prg\sql7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\sql7
	copy ..\prg\sql7.exe ..\bin /Y
	del ..\prg\sql7.exe

..\bin\sydir7.exe: ..\prg\sydir7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\sydir7
	copy ..\prg\sydir7.exe ..\bin /Y
	del ..\prg\sydir7.exe

..\bin\tar7.exe: ..\prg\tar7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\tar7
	copy ..\prg\tar7.exe ..\bin /Y
	del ..\prg\tar7.exe

..\bin\toutf8.exe: ..\prg\toutf8.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\toutf8
	copy ..\prg\toutf8.exe ..\bin /Y
	del ..\prg\toutf8.exe

..\bin\which.exe: ..\prg\which.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O3 -oc3 ..\prg\which
	copy ..\prg\which.exe ..\bin /Y
	del ..\prg\which.exe

bas7: ..\bin\bas7.exe
bigfiles: ..\bin\bigfiles.exe
calc7: ..\bin\calc7.exe
cat: ..\bin\cat.exe
comanche: ..\bin\comanche.exe
db7: ..\bin\db7.exe
diff7: ..\bin\diff7.exe
find7: ..\bin\find7.exe
findchar: ..\bin\findchar.exe
ftp7: ..\bin\ftp7.exe
ftpserv: ..\bin\ftpserv.exe
hd: ..\bin\hd.exe
ide7: ..\bin\ide7.exe
make7: ..\bin\make7.exe
portfwd7: ..\bin\portfwd7.exe
pv7: ..\bin\pv7.exe
s7check: ..\bin\s7check.exe
sql7: ..\bin\sql7.exe
sydir7: ..\bin\sydir7.exe
tar7: ..\bin\tar7.exe
toutf8: ..\bin\toutf8.exe
which: ..\bin\which.exe

utils: ..\bin\bas7.exe ..\bin\bigfiles.exe ..\bin\calc7.exe ..\bin\cat.exe ..\bin\comanche.exe ..\bin\db7.exe \
       ..\bin\diff7.exe ..\bin\find7.exe ..\bin\findchar.exe ..\bin\ftp7.exe ..\bin\ftpserv.exe ..\bin\hd.exe \
       ..\bin\ide7.exe ..\bin\make7.exe ..\bin\portfwd7.exe ..\bin\pv7.exe ..\bin\s7check.exe ..\bin\sql7.exe \
       ..\bin\sydir7.exe ..\bin\tar7.exe ..\bin\toutf8.exe ..\bin\which.exe

wc: $(SRC)
	@$(ECHO) SRC:
	wc $(SRC)
	@$(ECHO) SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	@$(ECHO) DRAW_LIB_SRC:
	wc $(DRAW_LIB_SRC)
	@$(ECHO) CONSOLE_LIB_SRC:
	wc $(CONSOLE_LIB_SRC)
	@$(ECHO) DATABASE_LIB_SRC:
	wc $(DATABASE_LIB_SRC)
	@$(ECHO) COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	@$(ECHO) COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(ALL_SYSTEM_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(ALL_SYSTEM_LIBS)
