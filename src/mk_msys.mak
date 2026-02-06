# Makefile for gmake and gcc from MinGW. Commands executed by: bash
# To compile use a MSYS console and call:
#   mingw32-make -f mk_msys.mak depend
#   mingw32-make -f mk_msys.mak
# It might also be possible to use this makefile from a Windows console instead of the MSYS console.
# When your make utility is mingw32-make, you should use mk_mingw.mak instead.
# When the nmake utility from Windows is available, you can use mk_nmake.mak instead.

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS_NO_FLTO = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS_NO_FLTO = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = $(CC_OPT_LINK_TIME_OPTIMIZATION) $(CFLAGS_NO_FLTO)
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--gc-sections,--stack,8388608
# LDFLAGS = -Wl,--gc-sections,--stack,8388608,--subsystem,windows
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -lws2_32
# SYSTEM_LIBS = -lm -lws2_32 -lgmp
# SYSTEM_BIGINT_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SYSTEM_CONSOLE_LIBS =
# SYSTEM_DATABASE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SYSTEM_DRAW_LIBS = -lgdi32
SYSTEM_MATH_LIBS = -lm
ALL_SYSTEM_LIBS = $(SYSTEM_LIBS) $(SYSTEM_BIGINT_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_MATH_LIBS)
SEED7_LIB = seed7_05.a
DRAW_LIB = s7_draw.a
CONSOLE_LIB = s7_con.a
DATABASE_LIB = s7_db.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ../bin/$(COMPILER_LIB) ../bin/$(COMP_DATA_LIB) ../bin/$(DRAW_LIB) ../bin/$(CONSOLE_LIB) ../bin/$(DATABASE_LIB) ../bin/$(SEED7_LIB)
# CC = g++
CC = gcc
AR = ar

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
DOBJ = big_rtl.o big_gmp.o cmd_win.o dir_win.o dll_win.o fil_win.o pcs_win.o pol_sel.o segv_win.o \
       soc_none.o stat_win.o tim_win.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_win.o gkb_win.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_win.o
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
DSRC = big_rtl.c big_gmp.c cmd_win.c dir_win.c dll_win.c fil_win.c pcs_win.c pol_sel.c segv_win.c \
       soc_none.c stat_win.c tim_win.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_win.c gkb_win.c
CONSOLE_LIB_SRC = kbd_rtl.c con_win.c
DATABASE_LIB_SRC_STD_INCL = sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c \
                            sql_post.c sql_tds.c
DATABASE_LIB_SRC = $(DATABASE_LIB_SRC_STD_INCL) sql_db2.c sql_ifx.c sql_srv.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ../bin/s7.exe ../prg/s7.exe
	../bin/s7.exe -l ../lib level
	@echo
	@echo "  Use 'make s7c' (with your make command) to create the compiler."
	@echo

s7c: ../bin/s7c.exe ../prg/s7c.exe
	@echo
	@echo "  Use 'make test' (with your make command) to check Seed7."
	@echo

../bin/s7.exe: levelup.exe next_lvl $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(ALL_SYSTEM_LIBS) -o ../bin/s7
	rm next_lvl

../prg/s7.exe: ../bin/s7.exe
	cp ../bin/s7.exe ../prg

../bin/s7c.exe: ../prg/s7c.exe
	cp ../prg/s7c.exe ../bin

../prg/s7c.exe: ../prg/s7c.sd7 $(ALL_S7_LIBS)
	../bin/s7.exe -l ../lib ../prg/s7c -l ../lib -b ../bin -O2 ../prg/s7c

levelup.exe: levelup.c
	$(CC) levelup.c -o levelup

next_lvl: levelup.exe
	./levelup.exe
	echo "X" > next_lvl

OBJCOPY_PARAMS = \
       -L SQLAllocHandle -L SQLBindCol -L SQLBindParameter -L SQLBrowseConnectW -L SQLColAttributeW \
       -L SQLConnectW -L SQLDataSources -L SQLDescribeColW -L SQLDescribeParam -L SQLDisconnect \
       -L SQLDriverConnectW -L SQLDriversW -L SQLEndTran -L SQLExecute -L SQLFetch -L SQLFreeHandle \
       -L SQLFreeStmt -L SQLGetConnectAttrW -L SQLGetData -L SQLGetDiagRecW -L SQLGetInfoW \
       -L SQLGetStmtAttrW -L SQLGetTypeInfoW -L SQLNumParams -L SQLNumResultCols -L SQLPrepareW \
       -L SQLSetConnectAttrW -L SQLSetDescFieldW -L SQLSetEnvAttr

sql_db2.o: sql_db2.c sql_cli.c sql_log.c
	$(CC) $(CPPFLAGS) $(DB2_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $< $(DB2_LIBS) $(DB2_CC_OPTION) -o $@
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

sql_ifx.o: sql_ifx.c sql_cli.c sql_log.c
	$(CC) $(CPPFLAGS) $(INFORMIX_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $< $(INFORMIX_LIBS) $(INFORMIX_CC_OPTION) -o $@
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

sql_srv.o: sql_srv.c sql_cli.c sql_log.c
	$(CC) $(CPPFLAGS) $(SQL_SERVER_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $< $(SQL_SERVER_LIBS) $(SQL_SERVER_CC_OPTION) -o $@
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

all: depend
	$(MAKE) -f mk_msys.mak s7
	$(MAKE) -f mk_msys.mak s7c

.PHONY: clean s7 s7c test check install all next_lvl strip clean_utils distclean uninstall

clean:
	rm -f *.o $(ALL_S7_LIBS) ../bin/s7.exe ../bin/s7c.exe ../prg/s7.exe ../prg/s7c.exe depend macros chkccomp.h base.h settings.h version.h chkccomp.exe setwpath.exe wrdepend.exe sudo.exe levelup.exe next_lvl
	@echo
	@echo "  Use 'make depend' (with your make command) to create the dependencies."
	@echo

clean_utils:
	rm -f ../bin/bas7.exe ../bin/bigfiles.exe ../bin/calc7.exe ../bin/cat.exe ../bin/comanche.exe ../bin/db7.exe
	rm -f ../bin/diff7.exe ../bin/find7.exe ../bin/findchar.exe ../bin/ftp7.exe ../bin/ftpserv.exe ../bin/hd.exe
	rm -f ../bin/ide7.exe ../bin/make7.exe ../bin/portfwd7.exe ../bin/pv7.exe ../bin/s7check.exe ../bin/sql7.exe
	rm -f ../bin/sydir7.exe ../bin/tar7.exe ../bin/toutf8.exe ../bin/which.exe

distclean: clean clean_utils
	rm -f level_bk.h level.h
	rm -f vers_msys.h

test:
	../bin/s7.exe -l ../lib ../prg/chk_all build
	@echo
	@echo "  Use './sudo make install' (with your make command) to install Seed7."
	@echo "  Or open a console as administrator, go to the directory seed7/src"
	@echo "  and use 'make install' (with your make command) to install Seed7."
	@echo

minimal_test:
	../bin/s7.exe -l ../lib ../prg/chk_all build minimal_test

check: test

install: setwpath.exe
	./setwpath.exe add ../bin

uninstall: setwpath.exe
	./setwpath.exe remove ../bin

strip:
	strip ../bin/s7.exe

chkccomp.h:
	echo "#define LIST_DIRECTORY_CONTENTS \"dir\"" > chkccomp.h
	echo "#define LINKER_OPT_STATIC_LINKING \"-static\"" >> chkccomp.h
	echo "#define CC_FLAGS \"-ffunction-sections -fdata-sections\"" >> chkccomp.h
	echo "#define CC_FLAGS64 \"-Wa,-mbig-obj\"" >> chkccomp.h
	echo "#define CC_OPT_LINK_TIME_OPTIMIZATION \"-flto=auto\"" >> chkccomp.h
	echo "#define CC_OPT_POSITION_INDEPENDENT_CODE \"-fPIC\"" >> chkccomp.h
	echo "#define LINKER_OPT_NO_LTO \"-fno-lto\"" >> chkccomp.h
	echo "#define POTENTIAL_PARTIAL_LINKING_OPTIONS \"-r\"" >> chkccomp.h
	echo "#define USE_GMP 0" >> chkccomp.h
	echo "#define SYSTEM_CONSOLE_LIBS \"$(SYSTEM_CONSOLE_LIBS)\"" >> chkccomp.h
	echo "#define SYSTEM_DRAW_LIBS \"$(SYSTEM_DRAW_LIBS)\"" >> chkccomp.h
	echo "#define INT_DIV_OVERFLOW_INFINITE_LOOP 1" >> chkccomp.h
	echo "#define POSTGRESQL_USE_DLL" >> chkccomp.h

base.h:
	echo "#define PATH_DELIMITER 92 /* backslash (ASCII) */" > base.h
	echo "#define OS_STRI_WCHAR" >> base.h
	echo "#define QUOTE_WHOLE_SHELL_COMMAND" >> base.h
	echo "#define OBJECT_FILE_EXTENSION \".o\"" >> base.h
	echo "#define EXECUTABLE_FILE_EXTENSION \".exe\"" >> base.h
	echo "#define C_COMPILER \"$(CC)\"" >> base.h
	echo "#define CC_OPT_VERSION_INFO \"--version\"" >> base.h
	echo "#define CC_ERROR_FILEDES 2" >> base.h
	echo "#define CC_VERSION_INFO_FILEDES 1" >> base.h
	echo "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> base.h
	echo "#define DEFAULT_STACK_SIZE 8388608" >> base.h
	echo "#define ARCHIVER \"$(AR)\"" >> base.h
	echo "#define ARCHIVER_OPT_REPLACE \"r \"" >> base.h
	echo "#define FORMAT_LL_TRIGGERS_WARNINGS 1" >> base.h
	echo "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> base.h
	echo "#define SYSTEM_MATH_LIBS \"$(SYSTEM_MATH_LIBS)\"" >> base.h

settings.h:
	echo "#define MAKE_UTILITY_NAME \"$(MAKE)\"" > settings.h
	echo "#define MAKEFILE_NAME \"mk_msys.mak\"" >> settings.h
	echo "#define SEARCH_PATH_DELIMITER ';'" >> settings.h
	echo "#define CONSOLE_WCHAR" >> settings.h
	echo "#define os_getch _getwch" >> settings.h
	echo "#define LIBRARY_FILE_EXTENSION \".a\"" >> settings.h
	echo "#define CC_OPT_DEBUG_INFO \"-g\"" >> settings.h
	echo "#define CC_OPT_NO_WARNINGS \"-w\"" >> settings.h
	echo "#define LINKER_OPT_NO_DEBUG_INFO \"-Wl,--strip-debug\"" >> settings.h
	echo "#define LINKER_OPT_STACK_SIZE \"-Wl,--stack,\"" >> settings.h
	echo "#define LINKER_FLAGS \"-Wl,--gc-sections\"" >> settings.h
	echo "#define SEED7_LIB \"$(SEED7_LIB)\"" >> settings.h
	echo "#define DRAW_LIB \"$(DRAW_LIB)\"" >> settings.h
	echo "#define CONSOLE_LIB \"$(CONSOLE_LIB)\"" >> settings.h
	echo "#define DATABASE_LIB \"$(DATABASE_LIB)\"" >> settings.h
	echo "#define COMP_DATA_LIB \"$(COMP_DATA_LIB)\"" >> settings.h
	echo "#define COMPILER_LIB \"$(COMPILER_LIB)\"" >> settings.h

version.h: chkccomp.exe base.h settings.h
	./chkccomp.exe version.h "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)"
	cp version.h vers_msys.h

chkccomp.exe: chkccomp.c chkccomp.h base.h settings.h
	$(CC) chkccomp.c -o chkccomp

setwpath.exe: version.h setwpath.c
	$(CC) setwpath.c -o setwpath

wrdepend.exe: version.h wrdepend.c
	$(CC) wrdepend.c -o wrdepend

sudo.exe: sudo.c
	$(CC) sudo.c -w -o sudo

depend: version.h setwpath.exe wrdepend.exe sudo.exe
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SRC) "> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SEED7_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DRAW_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(CONSOLE_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DATABASE_LIB_SRC_STD_INCL) ">> depend"
	./wrdepend.exe OPTION=DB2_INCLUDE_OPTION $(CFLAGS) -M sql_db2.c ">> depend"
	./wrdepend.exe OPTION=INFORMIX_INCLUDE_OPTION $(CFLAGS) -M sql_ifx.c ">> depend"
	./wrdepend.exe OPTION=SQL_SERVER_INCLUDE_OPTION $(CFLAGS) -M sql_srv.c ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMP_DATA_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMPILER_LIB_SRC) ">> depend"
	@echo
	@echo "  Use 'make' (with your make command) to create the interpreter."
	@echo

../bin/$(SEED7_LIB): $(SEED7_LIB_OBJ)
	$(AR) r ../bin/$(SEED7_LIB) $(SEED7_LIB_OBJ)

../bin/$(DRAW_LIB): $(DRAW_LIB_OBJ)
	$(AR) r ../bin/$(DRAW_LIB) $(DRAW_LIB_OBJ)

../bin/$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	$(AR) r ../bin/$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

../bin/$(DATABASE_LIB): $(DATABASE_LIB_OBJ)
	$(AR) r ../bin/$(DATABASE_LIB) $(DATABASE_LIB_OBJ)

../bin/$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	$(AR) r ../bin/$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

../bin/$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	$(AR) r ../bin/$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

../bin/bas7.exe: ../prg/bas7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/bas7
	mv ../prg/bas7.exe ../bin

../bin/bigfiles.exe: ../prg/bigfiles.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/bigfiles
	mv ../prg/bigfiles.exe ../bin

../bin/calc7.exe: ../prg/calc7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/calc7
	mv ../prg/calc7.exe ../bin

../bin/cat.exe: ../prg/cat.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/cat
	mv ../prg/cat.exe ../bin

../bin/comanche.exe: ../prg/comanche.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/comanche
	mv ../prg/comanche.exe ../bin

../bin/db7.exe: ../prg/db7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/db7
	mv ../prg/db7.exe ../bin

../bin/diff7.exe: ../prg/diff7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/diff7
	mv ../prg/diff7.exe ../bin

../bin/find7.exe: ../prg/find7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/find7
	mv ../prg/find7.exe ../bin

../bin/findchar.exe: ../prg/findchar.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/findchar
	mv ../prg/findchar.exe ../bin

../bin/ftp7.exe: ../prg/ftp7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/ftp7
	mv ../prg/ftp7.exe ../bin

../bin/ftpserv.exe: ../prg/ftpserv.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/ftpserv
	mv ../prg/ftpserv.exe ../bin

../bin/hd.exe: ../prg/hd.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/hd
	mv ../prg/hd.exe ../bin

../bin/ide7.exe: ../prg/ide7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/ide7
	mv ../prg/ide7.exe ../bin

../bin/make7.exe: ../prg/make7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/make7
	mv ../prg/make7.exe ../bin

../bin/portfwd7.exe: ../prg/portfwd7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/portfwd7
	mv ../prg/portfwd7.exe ../bin

../bin/pv7.exe: ../prg/pv7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/pv7
	mv ../prg/pv7.exe ../bin

../bin/s7check.exe: ../prg/s7check.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/s7check
	mv ../prg/s7check.exe ../bin

../bin/sql7.exe: ../prg/sql7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/sql7
	mv ../prg/sql7.exe ../bin

../bin/sydir7.exe: ../prg/sydir7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/sydir7
	mv ../prg/sydir7.exe ../bin

../bin/tar7.exe: ../prg/tar7.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/tar7
	mv ../prg/tar7.exe ../bin

../bin/toutf8.exe: ../prg/toutf8.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/toutf8
	mv ../prg/toutf8.exe ../bin

../bin/which.exe: ../prg/which.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O3 -oc3 ../prg/which
	mv ../prg/which.exe ../bin

bas7: ../bin/bas7.exe
bigfiles: ../bin/bigfiles.exe
calc7: ../bin/calc7.exe
cat: ../bin/cat.exe
comanche: ../bin/comanche.exe
db7: ../bin/db7.exe
diff7: ../bin/diff7.exe
find7: ../bin/find7.exe
findchar: ../bin/findchar.exe
ftp7: ../bin/ftp7.exe
ftpserv: ../bin/ftpserv.exe
hd: ../bin/hd.exe
ide7: ../bin/ide7.exe
make7: ../bin/make7.exe
portfwd7: ../bin/portfwd7.exe
pv7: ../bin/pv7.exe
s7check: ../bin/s7check.exe
sql7: ../bin/sql7.exe
sydir7: ../bin/sydir7.exe
tar7: ../bin/tar7.exe
toutf8: ../bin/toutf8.exe
which: ../bin/which.exe

utils: ../bin/bas7.exe ../bin/bigfiles.exe ../bin/calc7.exe ../bin/cat.exe ../bin/comanche.exe ../bin/db7.exe \
       ../bin/diff7.exe ../bin/find7.exe ../bin/findchar.exe ../bin/ftp7.exe ../bin/ftpserv.exe ../bin/hd.exe \
       ../bin/ide7.exe ../bin/make7.exe ../bin/portfwd7.exe ../bin/pv7.exe ../bin/s7check.exe ../bin/sql7.exe \
       ../bin/sydir7.exe ../bin/tar7.exe ../bin/toutf8.exe ../bin/which.exe

wc: $(SRC)
	@echo SRC:
	wc $(SRC)
	@echo SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	@echo DRAW_LIB_SRC:
	wc $(DRAW_LIB_SRC)
	@echo CONSOLE_LIB_SRC:
	wc $(CONSOLE_LIB_SRC)
	@echo DATABASE_LIB_SRC:
	wc $(DATABASE_LIB_SRC)
	@echo COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	@echo COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(ALL_SYSTEM_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(ALL_SYSTEM_LIBS)

ifeq (depend,$(wildcard depend))
include depend
endif

ifeq (macros,$(wildcard macros))
include macros
endif
