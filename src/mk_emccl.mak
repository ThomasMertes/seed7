# Makefile for linux/bsd/unix, emcc from Emscripten and gcc. Commands executed by: bash
# To compile use a command shell and call:
#   make -f mk_emccl.mak depend
#   make -f mk_emccl.mak

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = -O3 -g -gsource-map -ffunction-sections -fdata-sections -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -ffunction-sections -fdata-sections -Wall -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -g -gsource-map -s TOTAL_STACK=1048576 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString'] -s ASYNCIFY -s ASYNCIFY_STACK_SIZE=16384
# LDFLAGS = -Wl,--gc-sections,--stack,8388608
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
SYSTEM_LIBS = -lnodefs.js
# SYSTEM_LIBS = -lm -lws2_32 -lgmp
# SYSTEM_BIGINT_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_CONSOLE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_DATABASE_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
# SYSTEM_DRAW_LIBS is defined in the file "macros". The program chkccomp.c writes it to "macros" when doing "make depend".
SYSTEM_MATH_LIBS = -lm
ALL_SYSTEM_LIBS = $(SYSTEM_LIBS) $(SYSTEM_BIGINT_LIBS) $(SYSTEM_CONSOLE_LIBS) $(SYSTEM_DATABASE_LIBS) $(SYSTEM_DRAW_LIBS) $(SYSTEM_MATH_LIBS)
SEED7_LIB = seed7_05_emc.a
DRAW_LIB = s7_draw_emc.a
CONSOLE_LIB = s7_con_emc.a
DATABASE_LIB = s7_db_emc.a
COMP_DATA_LIB = s7_data_emc.a
COMPILER_LIB = s7_comp_emc.a
SPECIAL_LIB = pre_js.js
ALL_S7_LIBS = ../bin/$(COMPILER_LIB) ../bin/$(COMP_DATA_LIB) ../bin/$(DRAW_LIB) ../bin/$(CONSOLE_LIB) ../bin/$(DATABASE_LIB) ../bin/$(SEED7_LIB)
# CC = em++
CC = emcc
CC_ENVIRONMENT_INI = emcc_env.ini
# Some make tools terminate with the error "make: node: Permission denied" if node is called directly.
NODE = `which node`

MOBJ = s7.o
POBJ = runerr.o option.o primitiv.o
LOBJ = actlib.o arrlib.o biglib.o binlib.o blnlib.o bstlib.o chrlib.o cmdlib.o conlib.o dcllib.o \
       drwlib.o enulib.o fillib.o fltlib.o hshlib.o intlib.o itflib.o kbdlib.o pcslib.o pollib.o \
       prclib.o prglib.o reflib.o rfllib.o sctlib.o sellib.o setlib.o soclib.o sqllib.o strlib.o \
       timlib.o typlib.o ut8lib.o
EOBJ = exec.o doany.o objutl.o
AOBJ = act_comp.o prg_comp.o analyze.o syntax.o token.o parser.o name.o type.o \
       expr.o atom.o object.o scanner.o literal.o numlit.o findid.o msg_stri.o \
       error.o infile.o libpath.o symbol.o info.o stat.o fatal.o match.o
GOBJ = syvarutl.o traceutl.o actutl.o executl.o blockutl.o \
       entutl.o identutl.o chclsutl.o arrutl.o
ROBJ = arr_rtl.o bln_rtl.o bst_rtl.o chr_rtl.o cmd_rtl.o con_rtl.o dir_rtl.o drw_rtl.o fil_rtl.o \
       flt_rtl.o hsh_rtl.o int_rtl.o itf_rtl.o pcs_rtl.o set_rtl.o soc_rtl.o sql_rtl.o str_rtl.o \
       tim_rtl.o ut8_rtl.o heaputl.o numutl.o sigutl.o striutl.o
DOBJ = big_rtl.o big_gmp.o cmd_unx.o dir_win.o dll_unx.o emc_utl.o fil_emc.o pcs_unx.o pol_unx.o \
       soc_none.o tim_emc.o
OBJ = $(MOBJ)
SEED7_LIB_OBJ = $(ROBJ) $(DOBJ)
DRAW_LIB_OBJ = gkb_rtl.o drw_emc.o gkb_emc.o
CONSOLE_LIB_OBJ = kbd_rtl.o con_emc.o
DATABASE_LIB_OBJ = sql_base.o sql_db2.o sql_fire.o sql_ifx.o sql_lite.o sql_my.o sql_oci.o \
                   sql_odbc.o sql_post.o sql_srv.o sql_tds.o
COMP_DATA_LIB_OBJ = typ_data.o rfl_data.o ref_data.o listutl.o flistutl.o typeutl.o datautl.o
COMPILER_LIB_OBJ = $(POBJ) $(LOBJ) $(EOBJ) $(AOBJ) $(GOBJ)

MSRC = s7.c
PSRC = runerr.c option.c primitiv.c
LSRC = actlib.c arrlib.c biglib.c binlib.c blnlib.c bstlib.c chrlib.c cmdlib.c conlib.c dcllib.c \
       drwlib.c enulib.c fillib.c fltlib.c hshlib.c intlib.c itflib.c kbdlib.c pcslib.c pollib.c \
       prclib.c prglib.c reflib.c rfllib.c sctlib.c sellib.c setlib.c soclib.c sqllib.c strlib.c \
       timlib.c typlib.c ut8lib.c
ESRC = exec.c doany.c objutl.c
ASRC = act_comp.c prg_comp.c analyze.c syntax.c token.c parser.c name.c type.c \
       expr.c atom.c object.c scanner.c literal.c numlit.c findid.c msg_stri.c \
       error.c infile.c libpath.c symbol.c info.c stat.c fatal.c match.c
GSRC = syvarutl.c traceutl.c actutl.c executl.c blockutl.c \
       entutl.c identutl.c chclsutl.c arrutl.c
RSRC = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c con_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c \
       flt_rtl.c hsh_rtl.c int_rtl.c itf_rtl.c pcs_rtl.c set_rtl.c soc_rtl.c sql_rtl.c str_rtl.c \
       tim_rtl.c ut8_rtl.c heaputl.c numutl.c sigutl.c striutl.c
DSRC = big_rtl.c big_gmp.c cmd_unx.c dir_win.c dll_unx.c emc_utl.c fil_emc.c pcs_unx.c pol_unx.c \
       soc_none.c tim_emc.c
SRC = $(MSRC)
SEED7_LIB_SRC = $(RSRC) $(DSRC)
DRAW_LIB_SRC = gkb_rtl.c drw_emc.c gkb_emc.c
CONSOLE_LIB_SRC = kbd_rtl.c con_emc.c
DATABASE_LIB_SRC_STD_INCL = sql_base.c sql_fire.c sql_lite.c sql_my.c sql_oci.c sql_odbc.c \
                            sql_post.c sql_tds.c
DATABASE_LIB_SRC = $(DATABASE_LIB_SRC_STD_INCL) sql_db2.c sql_ifx.c sql_srv.c
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC) $(LSRC) $(ESRC) $(ASRC) $(GSRC)

s7: ../bin/s7.js ../prg/s7.js
	$(NODE) ../bin/s7.js -l ../lib level
	$(NODE) --stack-size=8192 ../bin/s7.js -l ../lib -q ../prg/confval > ../bin/cc_conf_emcc.prop
	@echo
	@echo "  Use 'make s7c' (with your make command) to create the compiler."
	@echo

s7c: ../bin/s7c.js ../prg/s7c.js
	@echo
	@echo "  Use 'make test' (with your make command) to check Seed7."
	@echo

../bin/s7.js: levelup next_lvl $(OBJ) $(ALL_S7_LIBS) ../bin/$(SPECIAL_LIB)
	$(CC) $(LDFLAGS) $(OBJ) $(ALL_S7_LIBS) $(ALL_SYSTEM_LIBS) --pre-js ../bin/$(SPECIAL_LIB) -o ../bin/s7.js
	rm next_lvl

../prg/s7.js: ../bin/s7.js
	ln -s ../bin/s7.js ../prg
	ln -s ../bin/s7.wasm ../prg

../bin/s7c.js: ../prg/s7c.js
	cp -p ../prg/s7c.js ../bin
	cp -p ../prg/s7c.wasm ../bin

../prg/s7c.js: ../prg/s7c.sd7 $(ALL_S7_LIBS) ../bin/$(SPECIAL_LIB)
	$(NODE) --stack-size=8192 ../bin/s7.js -l ../lib ../prg/s7c -l ../lib -b ../bin -O2 ../prg/s7c

levelup: levelup.c
	gcc levelup.c -o levelup

next_lvl: levelup
	./levelup
	echo "X" > next_lvl

sql_%.o: sql_%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE_OPTIONS) -c $< -o $@

big_%.o: big_%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDE_OPTIONS) -c $< -o $@

sql_db2.o: sql_db2.c
	$(CC) $(CPPFLAGS) $(DB2_INCLUDE_OPTION) $(CFLAGS) $(INCLUDE_OPTIONS) -c $<

sql_ifx.o: sql_ifx.c
	$(CC) $(CPPFLAGS) $(INFORMIX_INCLUDE_OPTION) $(CFLAGS) $(INCLUDE_OPTIONS) -c $<

sql_srv.o: sql_srv.c
	$(CC) $(CPPFLAGS) $(SQL_SERVER_INCLUDE_OPTION) $(CFLAGS) $(INCLUDE_OPTIONS) -c $<

.PHONY: clean s7 s7c test check install all next_lvl strip clean_utils distclean uninstall

clean:
	rm -f *.o $(ALL_S7_LIBS) ../bin/s7.js ../bin/s7.wasm ../bin/$(CC_ENVIRONMENT_INI) ../bin/s7c.js ../bin/s7c.wasm ../bin/$(SPECIAL_LIB)
	rm -f ../prg/s7.js ../prg/s7.wasm ../prg/s7c.js ../prg/s7c.wasm
	rm -f depend macros chkccomp.h base.h settings.h version.h chkccomp wrdepend levelup next_lvl
	@echo
	@echo "  Use 'make depend' (with your make command) to create the dependencies."
	@echo

clean_utils:
	rm -f ../bin/bas7.js ../bin/bigfiles.js ../bin/calc7.js ../bin/cat.js ../bin/comanche.js ../bin/db7.js
	rm -f ../bin/diff7.js ../bin/find7.js ../bin/findchar.js ../bin/ftp7.js ../bin/ftpserv.js ../bin/hd.js
	rm -f ../bin/ide7.js ../bin/make7.js ../bin/portfwd7.js ../bin/pv7.js ../bin/s7check.js ../bin/sql7.js
	rm -f ../bin/sydir7.js ../bin/tar7.js ../bin/toutf8.js ../bin/which.js
	rm -f ../bin/bas7.wasm ../bin/bigfiles.wasm ../bin/calc7.wasm ../bin/cat.wasm ../bin/comanche.wasm ../bin/db7.wasm
	rm -f ../bin/diff7.wasm ../bin/find7.wasm ../bin/findchar.wasm ../bin/ftp7.wasm ../bin/ftpserv.wasm ../bin/hd.wasm
	rm -f ../bin/ide7.wasm ../bin/make7.wasm ../bin/portfwd7.wasm ../bin/pv7.wasm ../bin/s7check.wasm ../bin/sql7.wasm
	rm -f ../bin/sydir7.wasm ../bin/tar7.wasm ../bin/toutf8.wasm ../bin/which.wasm

distclean: clean clean_utils
	rm -f level_bk.h level.h
	rm -f vers_emccl.h

test:
	$(NODE) ../bin/s7.js -l ../lib ../prg/chk_all build
	@echo
	@echo "  Use 'sudo make install' (with your make command) to install Seed7."
	@echo

minimal_test:
	$(NODE) ../bin/s7.js -l ../lib ../prg/chk_all build minimal_test

check: test

install:
	@echo
	@echo "  Cannot install."
	@echo

uninstall:
	@echo
	@echo "  Cannot uninstall."
	@echo

dep: depend

strip:
	strip ../bin/s7

chkccomp.h:
	echo "#define LIST_DIRECTORY_CONTENTS \"ls\"" > chkccomp.h
	echo "#define UNIX_DO_SLEEP" >> chkccomp.h
	echo "#define REMOVE_WASM" >> chkccomp.h
	echo "#define USE_GMP 0" >> chkccomp.h

base.h:
	echo "#define PATH_DELIMITER '/'" > base.h
	echo "#define OBJECT_FILE_EXTENSION \".o\"" >> base.h
	echo "#define C_COMPILER \"$(CC)\"" >> base.h
	echo "#define CC_OPT_VERSION_INFO \"--version\"" >> base.h
	echo "#define CC_FLAGS \"\"" >> base.h
	echo "#define CC_ERROR_FILEDES 2" >> base.h
	echo "#define CC_VERSION_INFO_FILEDES 1" >> base.h
	echo "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> base.h
	echo "#define LINKED_PROGRAM_EXTENSION \".js\"" >> base.h
	echo "#define INTERPRETER_FOR_LINKED_PROGRAM \"node\"" >> base.h
	echo "#define MOUNT_NODEFS" >> base.h
	echo "#define EMULATE_ENVIRONMENT" >> base.h
	echo "#define EMULATE_NODE_ENVIRONMENT" >> base.h
	echo "#define DETERMINE_OS_PROPERTIES_AT_RUNTIME" >> base.h
	echo "#define SYSTEM_LIBS \"$(SYSTEM_LIBS)\"" >> base.h

settings.h:
	echo "#define MAKE_UTILITY_NAME \"$(MAKE)\"" > settings.h
	echo "#define MAKEFILE_NAME \"mk_emccl.mak\"" >> settings.h
	echo "#define SEARCH_PATH_DELIMITER 0" >> settings.h
	echo "#define AWAIT_WITH_NANOSLEEP" >> settings.h
	echo "#define IMPLEMENT_PTY_WITH_PIPE2" >> settings.h
	echo "#define USE_EGL" >> settings.h
	echo "#define CONSOLE_UTF8" >> settings.h
	echo "#define OS_STRI_UTF8" >> settings.h
	echo "#define ESCAPE_SHELL_COMMANDS" >> settings.h
	echo "#define LIBRARY_FILE_EXTENSION \".a\"" >> settings.h
	echo "#define USE_CONSOLE_FOR_PROT_CSTRI" >> settings.h
	echo "#define DEFINE_SYSTEM_FUNCTION" >> settings.h
	echo "#define USE_START_MAIN" >> settings.h
	echo "#define USE_DO_EXIT" >> settings.h
	echo "#define os_exit doExit" >> settings.h
	echo "#define os_atexit registerExitFunction" >> settings.h
	echo "#define CALL_C_COMPILER_FROM_SHELL 1" >> settings.h
	echo "#define CPLUSPLUS_COMPILER \"em++\"" >> settings.h
	echo "#define CC_OPT_DEBUG_INFO \"-g\"" >> settings.h
	echo "#define CC_OPT_NO_WARNINGS \"-w\"" >> settings.h
	echo "#define LINKER_OPT_SPECIAL_LIB \"--pre-js\"" >> settings.h
	echo "#define LINKER_FLAGS \"$(LDFLAGS)\"" >> settings.h
	echo "#define SEED7_LIB \"$(SEED7_LIB)\"" >> settings.h
	echo "#define DRAW_LIB \"$(DRAW_LIB)\"" >> settings.h
	echo "#define CONSOLE_LIB \"$(CONSOLE_LIB)\"" >> settings.h
	echo "#define DATABASE_LIB \"$(DATABASE_LIB)\"" >> settings.h
	echo "#define COMP_DATA_LIB \"$(COMP_DATA_LIB)\"" >> settings.h
	echo "#define COMPILER_LIB \"$(COMPILER_LIB)\"" >> settings.h
	echo "#define SPECIAL_LIB \"$(SPECIAL_LIB)\"" >> settings.h

version.h: chkccomp base.h settings.h
	./chkccomp version.h "S7_LIB_DIR=$(S7_LIB_DIR)" "SEED7_LIBRARY=$(SEED7_LIBRARY)" "CC_ENVIRONMENT_INI=$(CC_ENVIRONMENT_INI)"
	rm -f ctest*.wasm
	env > ../bin/$(CC_ENVIRONMENT_INI)
	cp version.h vers_emccl.h

chkccomp: chkccomp.c chkccomp.h base.h settings.h
	gcc chkccomp.c -o chkccomp

wrdepend: version.h wrdepend.c
	gcc wrdepend.c -o wrdepend

depend: version.h wrdepend
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(SRC) "> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(SEED7_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(DRAW_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(CONSOLE_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(DATABASE_LIB_SRC_STD_INCL) ">> depend"
	./wrdepend OPTION=DB2_INCLUDE_OPTION $(CFLAGS) -M -c sql_db2.c ">> depend"
	./wrdepend OPTION=INFORMIX_INCLUDE_OPTION $(CFLAGS) -M -c sql_ifx.c ">> depend"
	./wrdepend OPTION=SQL_SERVER_INCLUDE_OPTION $(CFLAGS) -M -c sql_srv.c ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(COMP_DATA_LIB_SRC) ">> depend"
	./wrdepend OPTION=INCLUDE_OPTIONS $(CFLAGS) -M -c $(COMPILER_LIB_SRC) ">> depend"
	@echo
	@echo "  Use 'make' (with your make command) to create the interpreter."
	@echo

../bin/$(SEED7_LIB): $(SEED7_LIB_OBJ)
	emar r ../bin/$(SEED7_LIB) $(SEED7_LIB_OBJ)

../bin/$(DRAW_LIB): $(DRAW_LIB_OBJ)
	emar r ../bin/$(DRAW_LIB) $(DRAW_LIB_OBJ)

../bin/$(CONSOLE_LIB): $(CONSOLE_LIB_OBJ)
	emar r ../bin/$(CONSOLE_LIB) $(CONSOLE_LIB_OBJ)

../bin/$(DATABASE_LIB): $(DATABASE_LIB_OBJ)
	emar r ../bin/$(DATABASE_LIB) $(DATABASE_LIB_OBJ)

../bin/$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	emar r ../bin/$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

../bin/$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	emar r ../bin/$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

../bin/$(SPECIAL_LIB): pre_js.js
	cp pre_js.js ../bin/$(SPECIAL_LIB)

../bin/%.js: ../prg/%.sd7 ../bin/s7c.js
	$(NODE) --stack-size=8192 ../bin/s7c.js -l ../lib -b ../bin -O3 -oc3 $<
	mv $(<:.sd7=.js) ../bin
	mv $(<:.sd7=.wasm) ../bin

bas7: ../bin/bas7.js
bigfiles: ../bin/bigfiles.js
calc7: ../bin/calc7.js
cat: ../bin/cat.js
comanche: ../bin/comanche.js
db7: ../bin/db7.js
diff7: ../bin/diff7.js
find7: ../bin/find7.js
findchar: ../bin/findchar.js
ftp7: ../bin/ftp7.js
ftpserv: ../bin/ftpserv.js
hd: ../bin/hd.js
ide7: ../bin/ide7.js
make7: ../bin/make7.js
portfwd7: ../bin/portfwd7.js
pv7: ../bin/pv7.js
s7check: ../bin/s7check.js
sql7: ../bin/sql7.js
sydir7: ../bin/sydir7.js
tar7: ../bin/tar7.js
toutf8: ../bin/toutf8.js
which: ../bin/which.js

utils: ../bin/bas7.js ../bin/bigfiles.js ../bin/calc7.js ../bin/cat.js ../bin/comanche.js \
       ../bin/diff7.js ../bin/find7.js ../bin/findchar.js ../bin/ftp7.js ../bin/ftpserv.js ../bin/hd.js \
       ../bin/make7.js ../bin/portfwd7.js ../bin/pv7.js ../bin/s7check.js ../bin/sql7.js \
       ../bin/sydir7.js ../bin/tar7.js ../bin/toutf8.js ../bin/which.js

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
