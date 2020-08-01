# Makefile for gmake and gcc from Cygwin. Commands executed by: bash
# To compile use a Cygwin console and call:
#   make -f mk_cygw.mak depend
#   make -f mk_cygw.mak
# If you do not have Cygwin you could use MinGW and MSYS together with mk_msys.mak.

# CFLAGS =
# CFLAGS = -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar -Wc++-compat
# CFLAGS = -O2 -fomit-frame-pointer -pedantic -Wall -Wextra -Wswitch-default -Wcast-qual -Wlogical-op -Waggregate-return -Wwrite-strings -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar -Wpadded
CFLAGS = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--gc-sections,--stack,16777216
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -lm
# SYSTEM_LIBS = -lm -lgmp
# SYSTEM_DRAW_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_CONSOLE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_DATABASE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SEED7_LIB = seed7_05.a
DRAW_LIB = s7_draw.a
CONSOLE_LIB = s7_con.a
DATABASE_LIB = s7_db.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
ALL_S7_LIBS = ../bin/$(COMPILER_LIB) ../bin/$(COMP_DATA_LIB) ../bin/$(DRAW_LIB) ../bin/$(CONSOLE_LIB) ../bin/$(DATABASE_LIB) ../bin/$(SEED7_LIB)
# CC = g++
CC = gcc

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
DOBJ = big_rtl.o big_gmp.o cmd_unx.o dir_win.o dll_unx.o fil_unx.o pcs_unx.o pol_sel.o soc_none.o \
       tim_unx.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_x11.o gkb_x11.o fwd_x11.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_inf.o kbd_inf.o kbd_poll.o trm_inf.o trm_cap.o fwd_term.o
DATABASE_LIB_OBJ = sql_base.o sql_db2.o sql_fire.o sql_lite.o sql_my.o sql_oci.o sql_odbc.o \
                   sql_post.o sql_srv.o sql_tds.o
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
DSRC = big_rtl.c big_gmp.c cmd_unx.c dir_win.c dll_unx.c fil_unx.c pcs_unx.c pol_sel.c soc_none.c \
       tim_unx.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_x11.c gkb_x11.c fwd_x11.c
CONSOLE_LIB_SRC = kbd_rtl.c con_inf.c kbd_inf.c kbd_poll.c trm_inf.c trm_cap.c fwd_term.c
DATABASE_LIB_SRC_STD_INCL = sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c \
                            sql_post.c sql_tds.c
DATABASE_LIB_SRC = $(DATABASE_LIB_SRC_STD_INCL) sql_db2.c sql_srv.c
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

../bin/s7.exe: $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS) -o ../bin/s7

../prg/s7.exe:
	ln -s ../bin/s7.exe ../prg

../bin/s7c.exe: ../prg/s7c.exe
	cp ../prg/s7c.exe ../bin

../prg/s7c.exe: ../prg/s7c.sd7 $(ALL_S7_LIBS)
	../bin/s7.exe -l ../lib ../prg/s7c -l ../lib -b ../bin -O2 ../prg/s7c

OBJCOPY_PARAMS = \
       -L SQLAllocHandle -L SQLBindCol -L SQLBindParameter -L SQLBrowseConnectW -L SQLColAttributeW \
       -L SQLConnectW -L SQLDataSources -L SQLDescribeColW -L SQLDescribeParam -L SQLDisconnect \
       -L SQLDriverConnectW -L SQLDriversW -L SQLExecute -L SQLFetch -L SQLFreeHandle \
       -L SQLFreeStmt -L SQLGetData -L SQLGetDiagRecW -L SQLGetInfoW -L SQLGetStmtAttrW \
       -L SQLGetTypeInfoW -L SQLNumParams -L SQLNumResultCols -L SQLPrepareW -L SQLSetDescFieldW \
       -L SQLSetEnvAttr

sql_db2.o: sql_db2.c
	$(CC) $(CPPFLAGS) $(DB2_INCLUDE_OPTION) $(CFLAGS) $(DB2_LIBS) -c -r -o $@ $<
	objcopy $(OBJCOPY_PARAMS) $@

sql_srv.o: sql_srv.c
	$(CC) $(CPPFLAGS) $(SQL_SERVER_INCLUDE_OPTION) $(CFLAGS) $(SQL_SERVER_LIBS) -c -r -o $@ $<
	objcopy $(OBJCOPY_PARAMS) $@

all: depend
	$(MAKE) -f mk_cygw.mak s7 s7c

clear: clean

clean:
	rm -f *.o ../bin/*.a ../bin/s7.exe ../bin/s7c.exe ../prg/s7.exe ../prg/s7c.exe depend macros chkccomp.h base.h settings.h version.h wrdepend.exe
	rm -f chkint chkovf chkflt chkstr chkprc chkbig chkbool chkset chkhsh chkexc
	rm -f ../bin/s7 ../bin/s7c ../prg/s7 ../prg/s7c
	@echo
	@echo "  Use 'make depend' (with your make command) to create the dependencies."
	@echo

distclean: clean
	cp level_bk.h level.h
	rm -f vers_cygw.h

test:
	../bin/s7.exe -l ../lib ../prg/chk_all build
	@echo
	@echo "  Use 'make install' (with your make command) to install Seed7."
	@echo

install:
	cd ../bin; ln -s `pwd`/s7 /usr/local/bin
	cd ../bin; ln -s `pwd`/s7c /usr/local/bin
	gzip -c ../doc/s7.1 > /usr/share/man/man1/s7.1.gz
	gzip -c ../doc/s7c.1 > /usr/share/man/man1/s7c.1.gz

uninstall:
	rm /usr/local/bin/s7
	rm /usr/local/bin/s7c
	rm /usr/share/man/man1/s7.1.gz
	rm /usr/share/man/man1/s7c.1.gz

dep: depend

strip:
	strip ../bin/s7.exe

chkccomp.h:
	echo "#define LIST_DIRECTORY_CONTENTS \"ls\"" > chkccomp.h
	echo "#define LINKER_OPT_STATIC_LINKING \"-static\"" >> chkccomp.h
	echo "#define SUPPORTS_PARTIAL_LINKING" >> chkccomp.h

base.h:
	echo "#define PATH_DELIMITER '/'" > base.h
	echo "#define QUOTE_WHOLE_SHELL_COMMAND" >> base.h
	echo "#define OBJECT_FILE_EXTENSION \".o\"" >> base.h
	echo "#define EXECUTABLE_FILE_EXTENSION \".exe\"" >> base.h
	echo "#define C_COMPILER \"$(CC)\"" >> base.h
	echo "#define CC_OPT_VERSION_INFO \"--version\"" >> base.h
	echo "#define CC_FLAGS \"-ffunction-sections -fdata-sections\"" >> base.h
	echo "#define CC_ERROR_FILEDES 2" >> base.h
	echo "#define CC_VERSION_INFO_FILEDES 1" >> base.h
	echo "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> base.h
	echo "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> base.h

settings.h:
	echo "#define SEARCH_PATH_DELIMITER ':'" > settings.h
	echo "#define AWAIT_WITH_SIGACTION" >> settings.h
	echo "#define INCL_NCURSES_TERM" >> settings.h
	echo "#define SIGNAL_HANDLER_CAN_DO_IO" >> settings.h
	echo "#define CONSOLE_UTF8" >> settings.h
	echo "#define OS_STRI_UTF8" >> settings.h
	echo "#define APPEND_EXTENSION_TO_EXECUTABLE_PATH" >> settings.h
	echo "#define LIBRARY_FILE_EXTENSION \".a\"" >> settings.h
	echo "#define CC_SOURCE_UTF8" >> settings.h
	echo "#define CC_OPT_DEBUG_INFO \"-g\"" >> settings.h
	echo "#define CC_OPT_NO_WARNINGS \"-w\"" >> settings.h
	echo "#define LINKER_OPT_NO_DEBUG_INFO \"-Wl,--strip-debug\"" >> settings.h
	echo "#define LINKER_FLAGS \"$(LDFLAGS)\"" >> settings.h
	echo "#define SEED7_LIB \"$(SEED7_LIB)\"" >> settings.h
	echo "#define DRAW_LIB \"$(DRAW_LIB)\"" >> settings.h
	echo "#define CONSOLE_LIB \"$(CONSOLE_LIB)\"" >> settings.h
	echo "#define DATABASE_LIB \"$(DATABASE_LIB)\"" >> settings.h
	echo "#define COMP_DATA_LIB \"$(COMP_DATA_LIB)\"" >> settings.h
	echo "#define COMPILER_LIB \"$(COMPILER_LIB)\"" >> settings.h

version.h: chkccomp.h base.h settings.h
	$(CC) chkccomp.c -o chkccomp
	./chkccomp.exe version.h
	rm chkccomp.exe
	$(CC) setpaths.c -o setpaths
	./setpaths.exe "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" >> version.h
	rm setpaths.exe
	$(CC) wrdepend.c -o wrdepend
	cp version.h vers_cygw.h

depend: version.h
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SRC) "> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SEED7_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DRAW_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(CONSOLE_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DATABASE_LIB_SRC_STD_INCL) ">> depend"
	./wrdepend.exe OPTION=DB2_INCLUDE_OPTION $(CFLAGS) -M sql_db2.c ">> depend"
	./wrdepend.exe OPTION=SQL_SERVER_INCLUDE_OPTION $(CFLAGS) -M sql_srv.c ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMP_DATA_LIB_SRC) ">> depend"
	./wrdepend.exe OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMPILER_LIB_SRC) ">> depend"
	@echo
	@echo "  Use 'make' (with your make command) to create the interpreter."
	@echo

level.h:
	../bin/s7.exe -l ../lib level

../bin/$(SEED7_LIB): $(SEED7_LIB_OBJ)
	ar r ../bin/$(SEED7_LIB) $(SEED7_LIB_OBJ)

../bin/$(DRAW_LIB): $(DRAW_LIB_OBJ)
	ar r ../bin/$(DRAW_LIB) $(DRAW_LIB_OBJ)

../bin/$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	ar r ../bin/$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

../bin/$(DATABASE_LIB): $(DATABASE_LIB_OBJ)
	ar r ../bin/$(DATABASE_LIB) $(DATABASE_LIB_OBJ)

../bin/$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r ../bin/$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

../bin/$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r ../bin/$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

../bin/%.exe: ../prg/%.sd7 ../bin/s7c.exe
	../bin/s7c.exe -l ../lib -b ../bin -O2 $<
	mv $(<:.sd7=.exe) ../bin

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
make7: ../bin/make7.exe
sql7: ../bin/sql7.exe
sydir7: ../bin/sydir7.exe
tar7: ../bin/tar7.exe
toutf8: ../bin/toutf8.exe
which: ../bin/which.exe

utils: ../bin/bas7.exe ../bin/bigfiles.exe ../bin/calc7.exe ../bin/cat.exe ../bin/comanche.exe ../bin/db7.exe \
       ../bin/diff7.exe ../bin/find7.exe ../bin/findchar.exe ../bin/ftp7.exe ../bin/ftpserv.exe ../bin/hd.exe \
       ../bin/make7.exe ../bin/sql7.exe ../bin/sydir7.exe ../bin/tar7.exe ../bin/toutf8.exe ../bin/which.exe

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
	lint -p $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(SYSTEM_DRAW_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_LIBS) $(ADDITIONAL_SYSTEM_LIBS)

ifeq (depend,$(wildcard depend))
include depend
endif

ifeq (macros,$(wildcard macros))
include macros
endif
