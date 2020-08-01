# Makefile for make7 and gcc from MinGW. Commands executed by: cmd.exe
# To compile use a Windows console and call:
#   ..\bin\make7 -f mk_mingc.mak depend
#   ..\bin\make7 -f mk_mingc.mak
# When your make utility uses Unix commands, you should use mk_msys.mak instead.
# When the nmake utility from Windows is available, you can use mk_nmake.mak instead.
# When you are using the MSYS console from MinGW you should use mk_msys.mak instead.

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS)
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--gc-sections,--stack,8388608
# LDFLAGS = -Wl,--gc-sections,--stack,8388608,--subsystem,windows
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -lm -lws2_32
# SYSTEM_LIBS = -lm -lws2_32 -lgmp
SYSTEM_CONSOLE_LIBS =
SYSTEM_DRAW_LIBS = -lgdi32
SEED7_LIB = seed7_05.a
CONSOLE_LIB = s7_con.a
DRAW_LIB = s7_draw.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(DRAW_LIB) ..\bin\$(CONSOLE_LIB) ..\bin\$(SEED7_LIB)
# CC = g++
CC = ..\bin\call_gcc
GET_CC_VERSION_INFO = $(CC) --version >

MOBJ = s7.o
POBJ = runerr.o option.o primitiv.o
LOBJ = actlib.o arrlib.o biglib.o binlib.o blnlib.o bstlib.o chrlib.o cmdlib.o conlib.o dcllib.o \
       drwlib.o enulib.o fillib.o fltlib.o hshlib.o intlib.o itflib.o kbdlib.o lstlib.o pcslib.o \
       pollib.o prclib.o prglib.o reflib.o rfllib.o sctlib.o setlib.o soclib.o sqllib.o strlib.o \
       timlib.o typlib.o ut8lib.o
EOBJ = exec.o doany.o objutl.o
AOBJ = act_comp.o prg_comp.o analyze.o syntax.o token.o parser.o name.o type.o \
       expr.o atom.o object.o scanner.o literal.o numlit.o findid.o \
       error.o infile.o libpath.o symbol.o info.o stat.o fatal.o match.o
GOBJ = syvarutl.o traceutl.o actutl.o executl.o blockutl.o \
       entutl.o identutl.o chclsutl.o arrutl.o
ROBJ = arr_rtl.o bln_rtl.o bst_rtl.o chr_rtl.o cmd_rtl.o con_rtl.o dir_rtl.o drw_rtl.o fil_rtl.o \
       flt_rtl.o hsh_rtl.o int_rtl.o itf_rtl.o pcs_rtl.o set_rtl.o soc_rtl.o sql_rtl.o str_rtl.o \
       tim_rtl.o ut8_rtl.o heaputl.o numutl.o sigutl.o striutl.o
DOBJ = big_rtl.o big_gmp.o cmd_win.o dir_win.o dll_win.o fil_win.o pcs_win.o pol_sel.o soc_none.o \
       sql_base.o sql_fire.o sql_lite.o sql_my.o sql_oci.o sql_odbc.o sql_post.o stat_win.o tim_win.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_win.o gkb_win.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_win.o
COMP_DATA_LIB_OBJ = typ_data.o rfl_data.o ref_data.o listutl.o flistutl.o typeutl.o datautl.o
COMPILER_LIB_OBJ = $(POBJ) $(LOBJ) $(EOBJ) $(AOBJ) $(GOBJ)

MSRC = s7.c
PSRC = runerr.c option.c primitiv.c
LSRC = actlib.c arrlib.c biglib.c binlib.c blnlib.c bstlib.c chrlib.c cmdlib.c conlib.c dcllib.c \
       drwlib.c enulib.c fillib.c fltlib.c hshlib.c intlib.c itflib.c kbdlib.c lstlib.c pcslib.c \
       pollib.c prclib.c prglib.c reflib.c rfllib.c sctlib.c setlib.c soclib.c sqllib.c strlib.c \
       timlib.c typlib.c ut8lib.c
ESRC = exec.c doany.c objutl.c
ASRC = act_comp.c prg_comp.c analyze.c syntax.c token.c parser.c name.c type.c \
       expr.c atom.c object.c scanner.c literal.c numlit.c findid.c \
       error.c infile.c libpath.c symbol.c info.c stat.c fatal.c match.c
GSRC = syvarutl.c traceutl.c actutl.c executl.c blockutl.c \
       entutl.c identutl.c chclsutl.c arrutl.c
RSRC = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c con_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c \
       flt_rtl.c hsh_rtl.c int_rtl.c itf_rtl.c pcs_rtl.c set_rtl.c soc_rtl.c sql_rtl.c str_rtl.c \
       tim_rtl.c ut8_rtl.c heaputl.c numutl.c sigutl.c striutl.c
DSRC = big_rtl.c big_gmp.c cmd_win.c dir_win.c dll_win.c fil_win.c pcs_win.c pol_sel.c soc_none.c \
       sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c sql_post.c stat_win.c tim_win.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_win.c gkb_win.c
CONSOLE_LIB_SRC = kbd_rtl.c con_win.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ..\bin\s7.exe ..\prg\s7.exe
	..\bin\s7 -l ..\lib level

s7c: ..\bin\s7c.exe ..\prg\s7c.exe

..\bin\s7.exe: $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS) -o ..\bin\s7

..\prg\s7.exe: ..\bin\s7.exe
	copy ..\bin\s7.exe ..\prg /Y

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
	del macros
	del chkccomp.h
	del version.h
	del setwpath.exe
	del wrdepend.exe
	del sudo.exe

distclean: clean
	copy level_bk.h level.h /Y

test:
	..\bin\s7 -l ..\lib ..\prg\chk_all build

install: setwpath.exe
	.\sudo .\setwpath.exe add ..\bin

uninstall: setwpath.exe
	.\sudo .\setwpath.exe remove ..\bin

dep: depend

strip:
	strip ..\bin\s7.exe

chkccomp.h:
	echo #define TEST_C_COMPILER "../bin/call_gcc" >> chkccomp.h
	echo #define LINKER_OPT_STATIC_LINKING "-static" >> chkccomp.h
	echo #define LIST_DIRECTORY_CONTENTS "dir" >> chkccomp.h
	echo #define MYSQL_DLL "libmariadb.dll", "libmysql.dll" >> chkccomp.h
	echo #define MYSQL_USE_DLL >> chkccomp.h
	echo #define SQLITE_DLL "sqlite3.dll" >> chkccomp.h
	echo #define SQLITE_USE_DLL >> chkccomp.h
	echo #define POSTGRESQL_DLL "libpq.dll" >> chkccomp.h
	echo #define POSTGRESQL_USE_DLL >> chkccomp.h
	echo #define ODBC_LIBS "-lodbc32" >> chkccomp.h
	echo #define ODBC_DLL "odbc32.dll" >> chkccomp.h
	echo #define ODBC_USE_LIB >> chkccomp.h
	echo #define OCI_DLL "oci.dll" >> chkccomp.h
	echo #define OCI_USE_DLL >> chkccomp.h
	echo #define FIRE_LIBS "-lfbclient" >> chkccomp.h
	echo #define FIRE_DLL "fbclient.dll", "gds32.dll" >> chkccomp.h
	echo #define FIRE_USE_DLL >> chkccomp.h

version.h: chkccomp.h
	echo #define PATH_DELIMITER '\\' > version.h
	echo #define SEARCH_PATH_DELIMITER ';' >> version.h
	echo #define CONSOLE_WCHAR >> version.h
	echo #define OS_STRI_WCHAR >> version.h
	echo #define os_getch _getwch >> version.h
	echo #define QUOTE_WHOLE_SHELL_COMMAND >> version.h
	echo #define OBJECT_FILE_EXTENSION ".o" >> version.h
	echo #define LIBRARY_FILE_EXTENSION ".a" >> version.h
	echo #define EXECUTABLE_FILE_EXTENSION ".exe" >> version.h
	echo #define C_COMPILER_SCRIPT "call_gcc" >> version.h
	echo #define GET_CC_VERSION_INFO_OPTIONS "--version >" >> version.h
	echo #define CC_OPT_DEBUG_INFO "-g" >> version.h
	echo #define CC_OPT_NO_WARNINGS "-w" >> version.h
	echo #define CC_FLAGS "-ffunction-sections -fdata-sections" >> version.h
	echo #define CC_ERROR_FILDES 2 >> version.h
	echo #define LINKER_OPT_NO_DEBUG_INFO "-Wl,--strip-debug" >> version.h
	echo #define LINKER_OPT_OUTPUT_FILE "-o " >> version.h
	echo #define LINKER_FLAGS "$(LDFLAGS)" >> version.h
	echo #define SYSTEM_LIBS "$(SYSTEM_LIBS)" >> version.h
	echo #define SYSTEM_CONSOLE_LIBS "$(SYSTEM_CONSOLE_LIBS)" >> version.h
	echo #define SYSTEM_DRAW_LIBS "$(SYSTEM_DRAW_LIBS)" >> version.h
	$(GET_CC_VERSION_INFO) cc_vers.txt
	$(CC) chkccomp.c -w -o chkccomp
	.\chkccomp.exe version.h
	del chkccomp.exe
	del cc_vers.txt
	echo #define SEED7_LIB "$(SEED7_LIB)" >> version.h
	echo #define CONSOLE_LIB "$(CONSOLE_LIB)" >> version.h
	echo #define DRAW_LIB "$(DRAW_LIB)" >> version.h
	echo #define COMP_DATA_LIB "$(COMP_DATA_LIB)" >> version.h
	echo #define COMPILER_LIB "$(COMPILER_LIB)" >> version.h
	$(CC) -o setpaths setpaths.c
	.\setpaths.exe "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" >> version.h
	del setpaths.exe
	$(CC) setwpath.c -w -o setwpath
	$(CC) wrdepend.c -w -o wrdepend
	$(CC) sudo.c -w -o sudo

depend: version.h
	.\wrdepend.exe $(CFLAGS) -M $(SRC) "> depend"
	.\wrdepend.exe $(CFLAGS) -M $(SEED7_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(CONSOLE_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(DRAW_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(COMP_DATA_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(COMPILER_LIB_SRC) ">> depend"

level.h:
	..\bin\s7 -l ..\lib level

..\bin\$(SEED7_LIB): $(SEED7_LIB_OBJ)
	..\bin\call_ar r ..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	..\bin\call_ar r ..\bin\$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

..\bin\$(DRAW_LIB): $(DRAW_LIB_OBJ)
	..\bin\call_ar r ..\bin\$(DRAW_LIB) $(DRAW_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	..\bin\call_ar r ..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	..\bin\call_ar r ..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

..\bin\%.exe: ..\prg\%.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 $<
	move /Y $(<:.sd7=.exe) ..\bin

bas7: ..\bin\bas7.exe
calc7: ..\bin\calc7.exe
cat: ..\bin\cat.exe
comanche: ..\bin\comanche.exe
diff7: ..\bin\diff7.exe
find7: ..\bin\find7.exe
ftp7: ..\bin\ftp7.exe
ftpserv: ..\bin\ftpserv.exe
hd: ..\bin\hd.exe
make7: ..\bin\make7.exe
sql7: ..\bin\sql7.exe
sydir7: ..\bin\sydir7.exe
tar7: ..\bin\tar7.exe
toutf8: ..\bin\toutf8.exe
which: ..\bin\which.exe

utils: ..\bin\bas7.exe ..\bin\calc7.exe ..\bin\cat.exe ..\bin\comanche.exe ..\bin\diff7.exe \
       ..\bin\find7.exe ..\bin\ftp7.exe ..\bin\ftpserv.exe ..\bin\hd.exe ..\bin\make7.exe \
       ..\bin\sql7.exe ..\bin\sydir7.exe ..\bin\tar7.exe ..\bin\toutf8.exe ..\bin\which.exe

wc: $(SRC)
	@echo SRC:
	wc $(SRC)
	@echo SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	@echo CONSOLE_LIB_SRC:
	wc $(CONSOLE_LIB_SRC)
	@echo DRAW_LIB_SRC:
	wc $(DRAW_LIB_SRC)
	@echo COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	@echo COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS)

ifeq (depend,$(wildcard depend))
include depend
endif

ifeq (macros,$(wildcard macros))
include macros
endif
