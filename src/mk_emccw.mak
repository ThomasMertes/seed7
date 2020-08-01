# Makefile for mingw32-make (or make7), emcc from Emscripten and gcc from MinGW. Commands executed by: cmd.exe
# To compile use a Windows console and call:
#   mingw32-make -f mk_emccw.mak depend
#   mingw32-make -f mk_emccw.mak

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--gc-sections -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=1
# LDFLAGS = -Wl,--gc-sections,--stack,8388608,--subsystem,windows
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS =
# SYSTEM_LIBS = -lm -lws2_32 -lgmp
SYSTEM_CONSOLE_LIBS =
SYSTEM_DRAW_LIBS =
SEED7_LIB = seed7_05.a
CONSOLE_LIB = s7_con.a
DRAW_LIB = s7_draw.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(DRAW_LIB) ..\bin\$(CONSOLE_LIB) ..\bin\$(SEED7_LIB)
# CC = em++
CC = emcc
GET_CC_VERSION_INFO = $(CC) --version >

TERMINFO_OR_TERMCAP = USE_TERMINFO

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
DOBJ = big_rtl.o big_gmp.o cmd_unx.o dir_win.o dll_unx.o fil_unx.o pcs_unx.o pol_unx.o soc_none.o \
       sql_base.o sql_fire.o sql_lite.o sql_my.o sql_oci.o sql_odbc.o sql_post.o tim_unx.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_dos.o gkb_x11.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_emc.o
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
DSRC = big_rtl.c big_gmp.c cmd_unx.c dir_win.c dll_unx.c fil_unx.c pcs_unx.c pol_unx.c soc_none.c \
       sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c sql_post.c tim_unx.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_dos.c gkb_x11.c
CONSOLE_LIB_SRC = kbd_rtl.c con_emc.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ..\bin\s7.js ..\prg\s7.js
	node ..\bin\s7.js -l ..\lib level
	@echo.
	@echo Use 'make s7c' (with your make command) to create the compiler.
	@echo.

s7c: ..\bin\s7c.js ..\prg\s7c.js
	@echo.
	@echo Use 'make test' (with your make command) to check Seed7.
	@echo.

..\bin\s7.js: $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS) -o ..\bin\s7.js

..\prg\s7.js: ..\bin\s7.js
	copy ..\bin\s7.js ..\prg /Y
	copy ..\bin\s7.wasm ..\prg /Y

..\bin\s7c.js: ..\prg\s7c.js
	copy ..\prg\s7c.js ..\bin /Y
	copy ..\prg\s7c.wasm ..\bin /Y

..\prg\s7c.js: ..\prg\s7c.sd7 $(ALL_S7_LIBS)
	node ..\bin\s7.js -l ..\lib ..\prg\s7c -l ..\lib -b ..\bin -O2 ..\prg\s7c

sql_%.o: sql_%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE_OPTIONS) -c $< -o $@

big_%.o: big_%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE_OPTIONS) -c $< -o $@

clear: clean

clean:
	del *.o
	del ..\bin\*.a
	del ..\bin\s7.js
	del ..\bin\s7.wasm
	del ..\bin\s7c.js
	del ..\bin\s7c.wasm
	del ..\prg\s7.js
	del ..\prg\s7.wasm
	del ..\prg\s7c.js
	del ..\prg\s7c.wasm
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
	del vers_emccw.h

test:
	node ..\bin\s7.js -l ..\lib ..\prg\chk_all build
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
	echo #define ODBC_DLL "odbc32.dll" >> chkccomp.h
	echo #define ODBC_USE_DLL >> chkccomp.h
	echo #define OCI_DLL "oci.dll" >> chkccomp.h
	echo #define OCI_USE_DLL >> chkccomp.h
	echo #define FIRE_LIBS "-lfbclient" >> chkccomp.h
	echo #define FIRE_DLL "fbclient.dll", "gds32.dll" >> chkccomp.h
	echo #define FIRE_USE_DLL >> chkccomp.h

version.h: chkccomp.h
	echo #define PATH_DELIMITER '\\' > version.h
	echo #define SEARCH_PATH_DELIMITER ';' >> version.h
	echo #define AWAIT_WITH_NANOSLEEP >> version.h
	echo #define IMPLEMENT_PTY_WITH_PIPE2 >> version.h
	echo #define USE_EGL >> version.h
	echo #define $(TERMINFO_OR_TERMCAP) >> version.h
	echo #define CONSOLE_UTF8 >> version.h
	echo #define OS_STRI_UTF8 >> version.h
	echo #define QUOTE_WHOLE_SHELL_COMMAND >> version.h
	echo #define OBJECT_FILE_EXTENSION ".o" >> version.h
	echo #define LIBRARY_FILE_EXTENSION ".a" >> version.h
	echo #define EXECUTABLE_FILE_EXTENSION ".js" >> version.h
	echo #define MOUNT_NODEFS >> version.h
	echo #define INTERPRETER_FOR_EXECUTABLE "node" >> version.h
	echo #define C_COMPILER "$(CC)" >> version.h
	echo #define CPLUSPLUS_COMPILER "em++" >> version.h
	echo #define GET_CC_VERSION_INFO "$(GET_CC_VERSION_INFO)" >> version.h
	echo #define CC_OPT_DEBUG_INFO "-g" >> version.h
	echo #define CC_OPT_NO_WARNINGS "-w" >> version.h
	echo #define CC_FLAGS "" >> version.h
	echo #define CC_ERROR_FILDES 2 >> version.h
	echo #define LINKER_OPT_NO_DEBUG_INFO "-Wl,--strip-debug" >> version.h
	echo #define LINKER_OPT_OUTPUT_FILE "-o " >> version.h
	echo #define LINKER_FLAGS "$(LDFLAGS)" >> version.h
	echo #define SYSTEM_LIBS "$(SYSTEM_LIBS)" >> version.h
	echo #define SYSTEM_CONSOLE_LIBS "$(SYSTEM_CONSOLE_LIBS)" >> version.h
	echo #define SYSTEM_DRAW_LIBS "$(SYSTEM_DRAW_LIBS)" >> version.h
	$(GET_CC_VERSION_INFO) cc_vers.txt
	gcc chkccomp.c -o chkccomp
	.\chkccomp.exe version.h
	del chkccomp.exe
	del cc_vers.txt
	del ctest*.wasm
	echo #define SEED7_LIB "$(SEED7_LIB)" >> version.h
	echo #define CONSOLE_LIB "$(CONSOLE_LIB)" >> version.h
	echo #define DRAW_LIB "$(DRAW_LIB)" >> version.h
	echo #define COMP_DATA_LIB "$(COMP_DATA_LIB)" >> version.h
	echo #define COMPILER_LIB "$(COMPILER_LIB)" >> version.h
	gcc -o setpaths setpaths.c
	.\setpaths.exe "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" >> version.h
	del setpaths.exe
	gcc setwpath.c -o setwpath
	gcc wrdepend.c -o wrdepend
	gcc sudo.c -w -o sudo
	copy version.h vers_emccw.h /Y

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
	ar r ..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	ar r ..\bin\$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

..\bin\$(DRAW_LIB): $(DRAW_LIB_OBJ)
	ar r ..\bin\$(DRAW_LIB) $(DRAW_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r ..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r ..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

..\bin\bas7.js: ..\prg\bas7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\bas7
	copy ..\prg\bas7.js ..\bin /Y
	del ..\prg\bas7.js

..\bin\calc7.js: ..\prg\calc7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\calc7
	copy ..\prg\calc7.js ..\bin /Y
	del ..\prg\calc7.js

..\bin\cat.js: ..\prg\cat.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\cat
	copy ..\prg\cat.js ..\bin /Y
	del ..\prg\cat.js

..\bin\comanche.js: ..\prg\comanche.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\comanche
	copy ..\prg\comanche.js ..\bin /Y
	del ..\prg\comanche.js

..\bin\diff7.js: ..\prg\diff7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\diff7
	copy ..\prg\diff7.js ..\bin /Y
	del ..\prg\diff7.js

..\bin\find7.js: ..\prg\find7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\find7
	copy ..\prg\find7.js ..\bin /Y
	del ..\prg\find7.js

..\bin\ftp7.js: ..\prg\ftp7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\ftp7
	copy ..\prg\ftp7.js ..\bin /Y
	del ..\prg\ftp7.js

..\bin\ftpserv.js: ..\prg\ftpserv.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\ftpserv
	copy ..\prg\ftpserv.js ..\bin /Y
	del ..\prg\ftpserv.js

..\bin\hd.js: ..\prg\hd.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\hd
	copy ..\prg\hd.js ..\bin /Y
	del ..\prg\hd.js

..\bin\make7.js: ..\prg\make7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\make7
	copy ..\prg\make7.js ..\bin /Y
	del ..\prg\make7.js

..\bin\sql7.js: ..\prg\sql7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\sql7
	copy ..\prg\sql7.js ..\bin /Y
	del ..\prg\sql7.js

..\bin\sydir7.js: ..\prg\sydir7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\sydir7
	copy ..\prg\sydir7.js ..\bin /Y
	del ..\prg\sydir7.js

..\bin\tar7.js: ..\prg\tar7.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\tar7
	copy ..\prg\tar7.js ..\bin /Y
	del ..\prg\tar7.js

..\bin\toutf8.js: ..\prg\toutf8.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\toutf8
	copy ..\prg\toutf8.js ..\bin /Y
	del ..\prg\toutf8.js

..\bin\which.js: ..\prg\which.sd7 ..\bin\s7c.js
	node ..\bin\s7c.js -l ..\lib -b ..\bin -O2 ..\prg\which
	copy ..\prg\which.js ..\bin /Y
	del ..\prg\which.js

bas7: ..\bin\bas7.js
calc7: ..\bin\calc7.js
cat: ..\bin\cat.js
comanche: ..\bin\comanche.js
diff7: ..\bin\diff7.js
find7: ..\bin\find7.js
ftp7: ..\bin\ftp7.js
ftpserv: ..\bin\ftpserv.js
hd: ..\bin\hd.js
make7: ..\bin\make7.js
sql7: ..\bin\sql7.js
sydir7: ..\bin\sydir7.js
tar7: ..\bin\tar7.js
toutf8: ..\bin\toutf8.js
which: ..\bin\which.js

utils: ..\bin\bas7.js ..\bin\calc7.js ..\bin\cat.js ..\bin\comanche.js ..\bin\diff7.js \
       ..\bin\find7.js ..\bin\ftp7.js ..\bin\ftpserv.js ..\bin\hd.js ..\bin\make7.js \
       ..\bin\sql7.js ..\bin\sydir7.js ..\bin\tar7.js ..\bin\toutf8.js ..\bin\which.js

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
