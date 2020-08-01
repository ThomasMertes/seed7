# Makefile for mingw32-make and clang. Commands executed by: cmd.exe
# To compile use a Windows console and call:
#   mingw32-make -f mk_clangw.mak depend
#   mingw32-make -f mk_clangw.mak
# When your make utility uses Unix commands, you should use mk_clang.mak instead.

# CFLAGS =
# CFLAGS = -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar -Wc++-compat
# CFLAGS = -O2 -g -x c++ -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wextra -Wswitch-default -Wcast-qual -Waggregate-return -Wwrite-strings -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv -fsanitize=address,integer,undefined,dataflow,alignment,bool,bounds,enum,shift,integer-divide-by-zero -fno-sanitize=unsigned-integer-overflow
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv -fsanitize=address,integer,undefined
CFLAGS = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv
# CFLAGS = -O2 -g -x c++ -Wall -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv
# CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,/STACK:8388608
# LDFLAGS = -Wl,--gc-sections,--stack,8388608
# LDFLAGS = -Wl,--gc-sections,--stack,8388608 -fsanitize=address,integer,undefined
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -luser32 -lshell32 -lws2_32
# SYSTEM_LIBS = -lm -lws2_32 -lgmp
SYSTEM_CONSOLE_LIBS =
SYSTEM_DRAW_LIBS = -lgdi32
LIB_EXT = .lib
SEED7_LIB = seed7_05$(LIB_EXT)
CONSOLE_LIB = s7_con$(LIB_EXT)
DRAW_LIB = s7_draw$(LIB_EXT)
COMP_DATA_LIB = s7_data$(LIB_EXT)
COMPILER_LIB = s7_comp$(LIB_EXT)
ALL_S7_LIBS = ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(DRAW_LIB) ..\bin\$(CONSOLE_LIB) ..\bin\$(SEED7_LIB)
# CC = g++
CC = clang
GET_CC_VERSION_INFO = $(CC) --version >

BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
BIGINT_LIB = big_rtl
# BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
# BIGINT_LIB = big_gmp

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
       tim_rtl.o ut8_rtl.o heaputl.o numutl.o sigutl.o striutl.o \
       sql_base.o sql_lite.o sql_my.o sql_oci.o sql_odbc.o sql_post.o
DOBJ = $(BIGINT_LIB).o cmd_win.o dir_win.o dll_win.o fil_win.o pcs_win.o pol_sel.o stat_win.o tim_win.o
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
       tim_rtl.c ut8_rtl.c heaputl.c numutl.c sigutl.c striutl.c \
       sql_base.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c sql_post.c
DSRC = $(BIGINT_LIB).c cmd_win.c dir_win.c dll_win.c fil_win.c pcs_win.c pol_sel.c stat_win.c tim_win.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_win.c gkb_win.c
CONSOLE_LIB_SRC = kbd_rtl.c con_win.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ..\bin\s7.exe ..\prg\s7.exe
	..\bin\s7 -l ..\lib level
	@echo.
	@echo Use 'make s7c' (with your make command) to create the compiler.
	@echo.

s7c: ..\bin\s7c.exe ..\prg\s7c.exe
	@echo.
	@echo Use 'make test' (with your make command) to check Seed7.
	@echo.

..\bin\s7.exe: $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(SYSTEM_DB_LIBS) -o ..\bin\s7.exe

..\prg\s7.exe: ..\bin\s7.exe
	copy ..\bin\s7.exe ..\prg /Y

..\bin\s7c.exe: ..\prg\s7c.exe
	copy ..\prg\s7c.exe ..\bin /Y

..\prg\s7c.exe: ..\prg\s7c.sd7 $(ALL_S7_LIBS)
	..\bin\s7 -l ..\lib ..\prg\s7c -l ..\lib -b ..\bin -O2 ..\prg\s7c

clear: clean

clean:
	del *.o
	del ..\bin\*$(LIB_EXT)
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
	@echo.
	@echo Use 'make depend' (with your make command) to create the dependencies.
	@echo.

distclean: clean
	copy level_bk.h level.h /Y

test:
	..\bin\s7 -l ..\lib ..\prg\chk_all build
	@echo.
	@echo Use 'sudo make install' (with your make command) to install Seed7.
	@echo Or open a console as administrator, go to the directory seed7/src
	@echo and use 'make install' (with your make command) to install Seed7.
	@echo.

install: setwpath.exe
	.\setwpath.exe add ..\bin

uninstall: setwpath.exe
	.\setwpath.exe remove ..\bin

dep: depend

strip:
	strip ..\bin\s7.exe

chkccomp.h:
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

version.h: chkccomp.h
	echo #define PATH_DELIMITER '\\' > version.h
	echo #define SEARCH_PATH_DELIMITER ';' >> version.h
	echo #define NULL_DEVICE "NUL:" >> version.h
	echo #define INT_DIV_BY_ZERO_POPUP >> version.h
	echo #define FILENO_WORKS_FOR_NULL 0 >> version.h
	echo #define WITH_SQL >> version.h
	echo #define CONSOLE_WCHAR >> version.h
	echo #define OS_STRI_WCHAR >> version.h
	echo #define RENAMED_POSIX_FUNCTIONS >> version.h
	echo #define os_fstat _fstati64 >> version.h
	echo #define DEFINE_WSTATI64_EXT >> version.h
	echo #define os_lstat wstati64Ext >> version.h
	echo #define os_stat wstati64Ext >> version.h
	echo #define os_stat_orig _wstati64 >> version.h
	echo #define os_stat_struct struct _stati64 >> version.h
	echo #define os_fseek _fseeki64 >> version.h
	echo #define os_ftell _ftelli64 >> version.h
	echo #define os_off_t __int64 >> version.h
	echo #define os_environ _wenviron >> version.h
	echo #define os_putenv _wputenv >> version.h
	echo #define os_getch _getwch >> version.h
	echo #define QUOTE_WHOLE_SHELL_COMMAND >> version.h
	echo #define USE_WINSOCK >> version.h
	echo #define $(BIGINT_LIB_DEFINE) >> version.h
	echo #define OBJECT_FILE_EXTENSION ".o" >> version.h
	echo #define LIBRARY_FILE_EXTENSION "$(LIB_EXT)" >> version.h
	echo #define EXECUTABLE_FILE_EXTENSION ".exe" >> version.h
	echo #define C_COMPILER "$(CC)" >> version.h
	echo #define GET_CC_VERSION_INFO "$(GET_CC_VERSION_INFO)" >> version.h
	echo #define CC_OPT_DEBUG_INFO "-g" >> version.h
	echo #define CC_OPT_NO_WARNINGS "-w" >> version.h
	echo #define CC_FLAGS "-ffunction-sections -fdata-sections -ftrapv" >> version.h
	echo #define CC_ERROR_FILDES 2 >> version.h
	echo #define LINKER_OPT_DEBUG_INFO "-g" >> version.h
	echo #define LINKER_OPT_NO_DEBUG_INFO "-Wl,--strip-debug" >> version.h
	echo #define LINKER_OPT_OUTPUT_FILE "-o " >> version.h
	echo #define LINKER_FLAGS "$(LDFLAGS)" >> version.h
	echo #define SYSTEM_LIBS "$(SYSTEM_LIBS)" >> version.h
	echo #define SYSTEM_CONSOLE_LIBS "$(SYSTEM_CONSOLE_LIBS)" >> version.h
	echo #define SYSTEM_DRAW_LIBS "$(SYSTEM_DRAW_LIBS)" >> version.h
	$(GET_CC_VERSION_INFO) cc_vers.txt
	$(CC) -ftrapv -Wno-deprecated-declarations chkccomp.c -o chkccomp.exe
	.\chkccomp.exe version.h
	del chkccomp.exe
	del cc_vers.txt
	echo #define SEED7_LIB "$(SEED7_LIB)" >> version.h
	echo #define CONSOLE_LIB "$(CONSOLE_LIB)" >> version.h
	echo #define DRAW_LIB "$(DRAW_LIB)" >> version.h
	echo #define COMP_DATA_LIB "$(COMP_DATA_LIB)" >> version.h
	echo #define COMPILER_LIB "$(COMPILER_LIB)" >> version.h
	$(CC) -o setpaths.exe setpaths.c
	.\setpaths.exe "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" >> version.h
	del setpaths.exe
	$(CC) -luser32 -ladvapi32 setwpath.c -o setwpath.exe
	$(CC) wrdepend.c -o wrdepend.exe
	$(CC) -lshell32 sudo.c -w -o sudo.exe

depend: version.h
	.\wrdepend.exe $(CFLAGS) -M $(SRC) "> depend"
	.\wrdepend.exe $(CFLAGS) -M $(SEED7_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(CONSOLE_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(DRAW_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(COMP_DATA_LIB_SRC) ">> depend"
	.\wrdepend.exe $(CFLAGS) -M $(COMPILER_LIB_SRC) ">> depend"
	@echo.
	@echo Use 'make' (with your make command) to create the interpreter.
	@echo.

level.h:
	..\bin\s7 -l ..\lib level

..\bin\$(SEED7_LIB): $(SEED7_LIB_OBJ)
	..\bin\call_lib /out:..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	..\bin\call_lib /out:..\bin\$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

..\bin\$(DRAW_LIB): $(DRAW_LIB_OBJ)
	..\bin\call_lib /out:..\bin\$(DRAW_LIB) $(DRAW_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	..\bin\call_lib /out:..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	..\bin\call_lib /out:..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

..\bin\bas7.exe: ..\prg\bas7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\bas7
	copy ..\prg\bas7.exe ..\bin /Y
	del ..\prg\bas7.exe

..\bin\calc7.exe: ..\prg\calc7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\calc7
	copy ..\prg\calc7.exe ..\bin /Y
	del ..\prg\calc7.exe

..\bin\cat.exe: ..\prg\cat.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\cat
	copy ..\prg\cat.exe ..\bin /Y
	del ..\prg\cat.exe

..\bin\comanche.exe: ..\prg\comanche.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\comanche
	copy ..\prg\comanche.exe ..\bin /Y
	del ..\prg\comanche.exe

..\bin\diff7.exe: ..\prg\diff7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\diff7
	copy ..\prg\diff7.exe ..\bin /Y
	del ..\prg\diff7.exe

..\bin\find7.exe: ..\prg\find7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\find7
	copy ..\prg\find7.exe ..\bin /Y
	del ..\prg\find7.exe

..\bin\ftp7.exe: ..\prg\ftp7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\ftp7
	copy ..\prg\ftp7.exe ..\bin /Y
	del ..\prg\ftp7.exe

..\bin\ftpserv.exe: ..\prg\ftpserv.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\ftpserv
	copy ..\prg\ftpserv.exe ..\bin /Y
	del ..\prg\ftpserv.exe

..\bin\hd.exe: ..\prg\hd.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\hd
	copy ..\prg\hd.exe ..\bin /Y
	del ..\prg\hd.exe

..\bin\make7.exe: ..\prg\make7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\make7
	copy ..\prg\make7.exe ..\bin /Y
	del ..\prg\make7.exe

..\bin\sql7.exe: ..\prg\sql7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\sql7
	copy ..\prg\sql7.exe ..\bin /Y
	del ..\prg\sql7.exe

..\bin\sydir7.exe: ..\prg\sydir7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\sydir7
	copy ..\prg\sydir7.exe ..\bin /Y
	del ..\prg\sydir7.exe

..\bin\tar7.exe: ..\prg\tar7.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\tar7
	copy ..\prg\tar7.exe ..\bin /Y
	del ..\prg\tar7.exe

..\bin\toutf8.exe: ..\prg\toutf8.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\toutf8
	copy ..\prg\toutf8.exe ..\bin /Y
	del ..\prg\toutf8.exe

..\bin\which.exe: ..\prg\which.sd7 ..\bin\s7c.exe
	..\bin\s7c.exe -l ..\lib -b ..\bin -O2 ..\prg\which
	copy ..\prg\which.exe ..\bin /Y
	del ..\prg\which.exe

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
	echo SRC:
	wc $(SRC)
	echo SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	echo CONSOLE_LIB_SRC:
	wc $(CONSOLE_LIB_SRC)
	echo DRAW_LIB_SRC:
	wc $(DRAW_LIB_SRC)
	echo COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	echo COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(SYSTEM_DB_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(SYSTEM_DB_LIBS)

ifeq (depend,$(wildcard depend))
include depend
endif

ifeq (macros,$(wildcard macros))
include macros
endif