# Makefile for linux/bsd/unix and gcc. Commands executed by: bash
# To compile use a command shell and call:
#   make -f mk_linux.mak depend
#   make -f mk_linux.mak
# If you are under windows you should use MinGW with mk_mingw.mak, mk_nmake.mak or mk_msys.mak instead.

# CFLAGS =
# CFLAGS = -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -x c++ -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wextra -Wswitch-default -Wcast-qual -Waggregate-return -Wwrite-strings -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
# CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv
# CFLAGS = -O2 -g -x c++ -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -ftrapv
# CFLAGS_NO_FLTO = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar -Wc++-compat
CFLAGS_NO_FLTO = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wstrict-aliasing -Wstrict-overflow -Wuninitialized
# CFLAGS_NO_FLTO = -O2 -g -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = $(CC_OPT_LINK_TIME_OPTIMIZATION) $(CFLAGS_NO_FLTO)
# CFLAGS = -O2 -g -std=c99 -D_POSIX_SOURCE -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -std=c1x -D_XOPEN_SOURCE -ffunction-sections -fdata-sections $(INCLUDE_OPTIONS) -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--gc-sections
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS =
# SYSTEM_LIBS = -lm -ldl -lgmp
# SYSTEM_LIBS = -lm_p -lc_p
# SYSTEM_BIGINT_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_CONSOLE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_DATABASE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_DRAW_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
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
DOBJ = big_rtl.o big_gmp.o cmd_unx.o dir_win.o dll_unx.o fil_unx.o pcs_unx.o pol_unx.o soc_none.o \
       tim_unx.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_x11.o gkb_x11.o fwd_x11.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_inf.o kbd_inf.o kbd_poll.o trm_inf.o trm_cap.o fwd_term.o
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
DSRC = big_rtl.c big_gmp.c cmd_unx.c dir_win.c dll_unx.c fil_unx.c pcs_unx.c pol_unx.c soc_none.c \
       tim_unx.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_x11.c gkb_x11.c fwd_x11.c
CONSOLE_LIB_SRC = kbd_rtl.c con_inf.c kbd_inf.c kbd_poll.c trm_inf.c trm_cap.c fwd_term.c
DATABASE_LIB_SRC_STD_INCL = sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c \
                            sql_post.c sql_tds.c
DATABASE_LIB_SRC = $(DATABASE_LIB_SRC_STD_INCL) sql_db2.c sql_ifx.c sql_srv.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ../bin/s7 ../prg/s7
	../bin/s7 -l ../lib level
	@echo
	@echo "  Use 'make s7c' (with your make command) to create the compiler."
	@echo

s7c: ../bin/s7c ../prg/s7c
	@echo
	@echo "  Use 'make test' (with your make command) to check Seed7."
	@echo

../bin/s7: levelup next_lvl $(OBJ) $(ALL_S7_LIBS)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(ALL_SYSTEM_LIBS) -o ../bin/s7
	rm next_lvl

../prg/s7:
	ln -s ../bin/s7 ../prg

../bin/s7c: ../prg/s7c
	cp -p ../prg/s7c ../bin

../prg/s7c: ../prg/s7c.sd7 $(ALL_S7_LIBS)
	../bin/s7 -l ../lib ../prg/s7c -l ../lib -b ../bin -O2 ../prg/s7c

levelup: levelup.c
	$(CC) levelup.c -o levelup

next_lvl: levelup
	./levelup
	echo "X" > next_lvl

OBJCOPY_PARAMS = \
       -L SQLAllocHandle -L SQLBindCol -L SQLBindParameter -L SQLBrowseConnectW -L SQLColAttributeW \
       -L SQLConnectW -L SQLDataSources -L SQLDescribeColW -L SQLDescribeParam -L SQLDisconnect \
       -L SQLDriverConnectW -L SQLDriversW -L SQLExecute -L SQLFetch -L SQLFreeHandle \
       -L SQLFreeStmt -L SQLGetData -L SQLGetDiagRecW -L SQLGetInfoW -L SQLGetStmtAttrW \
       -L SQLGetTypeInfoW -L SQLNumParams -L SQLNumResultCols -L SQLPrepareW -L SQLSetDescFieldW \
       -L SQLSetEnvAttr

sql_db2.o: sql_db2.c
	$(CC) $(CPPFLAGS) $(DB2_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $(DB2_LIBS) -c $(LINKER_OPT_PARTIAL_LINKING) -o $@ $<
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

sql_ifx.o: sql_ifx.c
	$(CC) $(CPPFLAGS) $(INFORMIX_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $(INFORMIX_LIBS) -c $(LINKER_OPT_PARTIAL_LINKING) -o $@ $<
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

sql_srv.o: sql_srv.c
	$(CC) $(CPPFLAGS) $(SQL_SERVER_INCLUDE_OPTION) $(CFLAGS_NO_FLTO) $(SQL_SERVER_LIBS) -c $(LINKER_OPT_PARTIAL_LINKING) -o $@ $<
	$(OBJCOPY) $(OBJCOPY_PARAMS) $@

all: depend
	$(MAKE) -f mk_linux.mak s7
	$(MAKE) -f mk_linux.mak s7c

.PHONY: clean s7 s7c test check install all next_lvl strip clean_utils distclean uninstall

clean:
	rm -f *.o $(ALL_S7_LIBS) ../bin/s7 ../bin/s7c ../prg/s7 ../prg/s7c depend macros chkccomp.h base.h settings.h version.h chkccomp wrdepend levelup next_lvl
	@echo
	@echo "  Use 'make depend' (with your make command) to create the dependencies."
	@echo

clean_utils:
	rm -f ../bin/bas7 ../bin/bigfiles ../bin/calc7 ../bin/cat ../bin/comanche ../bin/db7 ../bin/diff7 ../bin/find7 ../bin/findchar ../bin/ftp7
	rm -f ../bin/ftpserv ../bin/hd ../bin/ide7 ../bin/make7 ../bin/portfwd7 ../bin/pv7 ../bin/s7check ../bin/sql7 ../bin/sydir7 ../bin/tar7 ../bin/toutf8 ../bin/which

distclean: clean clean_utils
	rm -f level_bk.h level.h
	rm -f vers_linux.h

test:
	../bin/s7 -l ../lib ../prg/chk_all build
	@echo
	@echo "  Use 'sudo make install' (with your make command) to install Seed7."
	@echo

check: test

install:
	cd ../bin; ln -fs `pwd`/s7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/s7c /usr/local/bin
	gzip -c ../doc/s7.1 > /usr/share/man/man1/s7.1.gz
	gzip -c ../doc/s7c.1 > /usr/share/man/man1/s7c.1.gz
	cd ../bin; ln -fs `pwd`/bas7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/calc7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/db7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/diff7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/find7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/ftp7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/ide7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/make7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/portfwd7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/pv7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/s7check /usr/local/bin
	cd ../bin; ln -fs `pwd`/sql7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/sydir7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/tar7 /usr/local/bin
	cd ../bin; ln -fs `pwd`/toutf8 /usr/local/bin

uninstall:
	rm -f /usr/local/bin/s7
	rm -f /usr/local/bin/s7c
	rm -f /usr/share/man/man1/s7.1.gz
	rm -f /usr/share/man/man1/s7c.1.gz
	rm -f /usr/local/bin/bas7
	rm -f /usr/local/bin/calc7
	rm -f /usr/local/bin/db7
	rm -f /usr/local/bin/diff7
	rm -f /usr/local/bin/find7
	rm -f /usr/local/bin/ftp7
	rm -f /usr/local/bin/ide7
	rm -f /usr/local/bin/make7
	rm -f /usr/local/bin/portfwd7
	rm -f /usr/local/bin/pv7
	rm -f /usr/local/bin/s7check
	rm -f /usr/local/bin/sql7
	rm -f /usr/local/bin/sydir7
	rm -f /usr/local/bin/tar7
	rm -f /usr/local/bin/toutf8

strip:
	strip ../bin/s7
	strip ../bin/s7c
	strip ../bin/$(SEED7_LIB)
	strip ../bin/$(DRAW_LIB)
	strip ../bin/$(CONSOLE_LIB)
	strip ../bin/$(DATABASE_LIB)
	strip ../bin/$(COMP_DATA_LIB)
	strip ../bin/$(COMPILER_LIB)

chkccomp.h:
	echo "#define LIST_DIRECTORY_CONTENTS \"ls\"" > chkccomp.h
	echo "#define CC_OPT_LINK_TIME_OPTIMIZATION \"-flto=auto\"" >> chkccomp.h
	echo "#define CC_OPT_POSITION_INDEPENDENT_CODE \"-fPIC\"" >> chkccomp.h
	echo "#define LINKER_OPT_STATIC_LINKING \"-static\"" >> chkccomp.h
	echo "#define LINKER_OPT_NO_LTO \"-fno-lto\"" >> chkccomp.h
	echo "#define LINKER_OPT_DYN_LINK_LIBS \"-ldl\"" >> chkccomp.h
	echo "#define POTENTIAL_PARTIAL_LINKING_OPTIONS \"-r\", \"-r -nostdlib\"" >> chkccomp.h
	echo "#define USE_GMP 0" >> chkccomp.h
	echo "#define ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS" >> chkccomp.h

base.h:
	echo "#define PATH_DELIMITER '/'" > base.h
	echo "#define OBJECT_FILE_EXTENSION \".o\"" >> base.h
	echo "#define C_COMPILER \"$(CC)\"" >> base.h
	echo "#define CC_OPT_VERSION_INFO \"--version\"" >> base.h
	echo "#define CC_FLAGS \"-ffunction-sections -fdata-sections\"" >> base.h
	echo "#define CC_ERROR_FILEDES 2" >> base.h
	echo "#define CC_VERSION_INFO_FILEDES 1" >> base.h
	echo "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> base.h
	echo "#define ARCHIVER \"$(AR)\"" >> base.h
	echo "#define ARCHIVER_OPT_REPLACE \"r \"" >> base.h
	echo "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> base.h
	echo "#define SYSTEM_MATH_LIBS \"$(SYSTEM_MATH_LIBS)\"" >> base.h

settings.h:
	echo "#define MAKE_UTILITY_NAME \"$(MAKE)\"" > settings.h
	echo "#define MAKEFILE_NAME \"mk_linux.mak\"" >> settings.h
	echo "#define SEARCH_PATH_DELIMITER ':'" >> settings.h
	echo "#define AWAIT_WITH_SELECT" >> settings.h
	echo "#define SIGNAL_HANDLER_CAN_DO_IO" >> settings.h
	echo "#define CONSOLE_UTF8" >> settings.h
	echo "#define OS_STRI_UTF8" >> settings.h
	echo "#define ESCAPE_SHELL_COMMANDS" >> settings.h
	echo "#define LIBRARY_FILE_EXTENSION \".a\"" >> settings.h
	echo "#define CPLUSPLUS_COMPILER \"g++\"" >> settings.h
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

version.h: chkccomp base.h settings.h
	./chkccomp version.h "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" "LINK_TIME=$(LINK_TIME)"
	cp version.h vers_linux.h

chkccomp: chkccomp.c chkccomp.h base.h settings.h
	$(CC) chkccomp.c -o chkccomp

wrdepend: version.h wrdepend.c
	$(CC) wrdepend.c -o wrdepend

depend: version.h wrdepend
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SRC) "> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(SEED7_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DRAW_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(CONSOLE_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(DATABASE_LIB_SRC_STD_INCL) ">> depend"
	./wrdepend OPTION=DB2_INCLUDE_OPTION $(CFLAGS) -M sql_db2.c ">> depend"
	./wrdepend OPTION=INFORMIX_INCLUDE_OPTION $(CFLAGS) -M sql_ifx.c ">> depend"
	./wrdepend OPTION=SQL_SERVER_INCLUDE_OPTION $(CFLAGS) -M sql_srv.c ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMP_DATA_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M $(COMPILER_LIB_SRC) ">> depend"
	$(CC) warn.c -o warn
	@./warn
	@rm warn
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

../bin/%: ../prg/%.sd7 ../bin/s7c
	../bin/s7c -l ../lib -b ../bin -O3 -oc3 $<
	mv $(<:.sd7=) ../bin

bas7: ../bin/bas7
bigfiles: ../bin/bigfiles
calc7: ../bin/calc7
cat: ../bin/cat
comanche: ../bin/comanche
db7: ../bin/db7
diff7: ../bin/diff7
find7: ../bin/find7
findchar: ../bin/findchar
ftp7: ../bin/ftp7
ftpserv: ../bin/ftpserv
hd: ../bin/hd
ide7: ../bin/ide7
make7: ../bin/make7
portfwd7: ../bin/portfwd7
pv7: ../bin/pv7
s7check: ../bin/s7check
sql7: ../bin/sql7
sydir7: ../bin/sydir7
tar7: ../bin/tar7
toutf8: ../bin/toutf8
which: ../bin/which

utils: ../bin/bas7 ../bin/bigfiles ../bin/calc7 ../bin/cat ../bin/comanche ../bin/db7 \
       ../bin/diff7 ../bin/find7 ../bin/findchar ../bin/ftp7 ../bin/ftpserv ../bin/hd \
       ../bin/ide7 ../bin/make7 ../bin/portfwd7 ../bin/pv7 ../bin/s7check ../bin/sql7 \
       ../bin/sydir7 ../bin/tar7 ../bin/toutf8 ../bin/which

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

cppcheck: $(SRC) $(SEED7_LIB_SRC) $(DRAW_LIB_SRC) $(COMP_DATA_LIB_SRC) $(COMPILER_LIB_SRC)
	cppcheck --force --enable=all $(SRC) $(SEED7_LIB_SRC) $(DRAW_LIB_SRC) $(COMP_DATA_LIB_SRC) $(COMPILER_LIB_SRC)

ifeq (depend,$(wildcard depend))
include depend
endif

ifeq (macros,$(wildcard macros))
include macros
endif
