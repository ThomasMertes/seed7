# Makefile for Mac OS X with Xcode. Commands executed by: bash
# To compile use a command shell and call:
#   make -f mk_osx.mak depend
#   make -f mk_osx.mak
# If you are not using Mac OS X with Xcode look into the file read_me.txt for the makefile to use.

# CFLAGS =
# CFLAGS = -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wextra -Wswitch-default -Wswitch-enum -Wcast-qual -Waggregate-return -Wwrite-strings -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wextra -Wswitch-default -Wcast-qual -Waggregate-return -Wwrite-strings -Winline -Wconversion -Wshadow -Wpointer-arith -Wmissing-noreturn -Wno-multichar
CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -Wall
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -funroll-loops -Wall -pg
LFLAGS = -L/usr/X11R6/lib 
# LFLAGS = -pg
# LFLAGS = -pg -lc_p
# LIBS = /usr/Xlib/libX11.so -lncurses -lm
LIBS = -lX11 -lncurses -lm
# LIBS = -lX11 -lncurses -lm -lgmp
# LIBS = -lX11 -lncurses -lm_p -lc_p
SEED7_LIB = seed7_05.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
CC = gcc

BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
BIGINT_LIB = big_rtl
# BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
# BIGINT_LIB = big_gmp

# TERMINFO_OR_TERMCAP = USE_TERMINFO
# SCREEN_OBJ = scr_inf.o kbd_inf.o trm_inf.o
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_inf.c
# TERMINFO_OR_TERMCAP = USE_TERMCAP
# SCREEN_OBJ = scr_inf.o kbd_inf.o trm_cap.o
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_cap.c
TERMINFO_OR_TERMCAP = USE_TERMINFO
SCREEN_OBJ = scr_inf.o kbd_poll.o trm_inf.o
SCREEN_SRC = scr_inf.c kbd_poll.c trm_inf.c
# TERMINFO_OR_TERMCAP = USE_TERMCAP
# SCREEN_OBJ = scr_inf.o kbd_poll.o trm_cap.o
# SCREEN_SRC = scr_inf.c kbd_poll.c trm_cap.c

# SCREEN_OBJ = scr_cur.o
# SCREEN_SRC = scr_cur.c
# SCREEN_OBJ = scr_cap.o
# SCREEN_SRC = scr_cap.c
# SCREEN_OBJ = scr_tcp.o
# SCREEN_SRC = scr_tcp.c
# SCREEN_OBJ = scr_x11.o
# SCREEN_SRC = scr_x11.c
# SCREEN_OBJ = scr_win.o
# SCREEN_SRC = scr_win.c

MOBJ1 = hi.o
POBJ1 = runerr.o option.o primitiv.o
LOBJ1 = actlib.o arrlib.o biglib.o blnlib.o bstlib.o chrlib.o cmdlib.o dcllib.o drwlib.o enulib.o
LOBJ2 = fillib.o fltlib.o hshlib.o intlib.o itflib.o kbdlib.o lstlib.o prclib.o prglib.o reflib.o
LOBJ3 = rfllib.o scrlib.o sctlib.o setlib.o soclib.o strlib.o timlib.o typlib.o ut8lib.o
EOBJ1 = exec.o doany.o memory.o
AOBJ1 = act_comp.o prg_comp.o analyze.o syntax.o token.o parser.o name.o type.o
AOBJ2 = expr.o atom.o object.o scanner.o literal.o numlit.o findid.o
AOBJ3 = error.o infile.o symbol.o info.o stat.o fatal.o match.o
GOBJ1 = syvarutl.o traceutl.o actutl.o executl.o blockutl.o
GOBJ2 = entutl.o identutl.o chclsutl.o sigutl.o
ROBJ1 = arr_rtl.o bln_rtl.o bst_rtl.o chr_rtl.o cmd_rtl.o dir_rtl.o drw_rtl.o fil_rtl.o flt_rtl.o
ROBJ2 = hsh_rtl.o int_rtl.o kbd_rtl.o scr_rtl.o set_rtl.o soc_rtl.o str_rtl.o tim_rtl.o ut8_rtl.o
ROBJ3 = heaputl.o striutl.o
DOBJ1 = $(BIGINT_LIB).o $(SCREEN_OBJ) tim_unx.o drw_x11.o
OBJ = $(MOBJ1)
SEED7_LIB_OBJ = $(ROBJ1) $(ROBJ2) $(ROBJ3) $(DOBJ1)
COMP_DATA_LIB_OBJ = typ_data.o rfl_data.o ref_data.o listutl.o flistutl.o typeutl.o datautl.o
COMPILER_LIB_OBJ = $(POBJ1) $(LOBJ1) $(LOBJ2) $(LOBJ3) $(EOBJ1) $(AOBJ1) $(AOBJ2) $(AOBJ3) $(GOBJ1) $(GOBJ2)

MSRC1 = hi.c
PSRC1 = runerr.c option.c primitiv.c
LSRC1 = actlib.c arrlib.c biglib.c blnlib.c bstlib.c chrlib.c cmdlib.c dcllib.c drwlib.c enulib.c
LSRC2 = fillib.c fltlib.c hshlib.c intlib.c itflib.c kbdlib.c lstlib.c prclib.c prglib.c reflib.c
LSRC3 = rfllib.c scrlib.c sctlib.c setlib.c soclib.c strlib.c timlib.c typlib.c ut8lib.c
ESRC1 = exec.c doany.c memory.c
ASRC1 = act_comp.c prg_comp.c analyze.c syntax.c token.c parser.c name.c type.c
ASRC2 = expr.c atom.c object.c scanner.c literal.c numlit.c findid.c
ASRC3 = error.c infile.c symbol.c info.c stat.c fatal.c match.c
GSRC1 = syvarutl.c traceutl.c actutl.c executl.c blockutl.c
GSRC2 = entutl.c identutl.c chclsutl.c sigutl.c
RSRC1 = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c flt_rtl.c
RSRC2 = hsh_rtl.c int_rtl.c kbd_rtl.c scr_rtl.c set_rtl.c soc_rtl.c str_rtl.c tim_rtl.c ut8_rtl.c
RSRC3 = heaputl.c striutl.c
DSRC1 = $(BIGINT_LIB).c $(SCREEN_SRC) tim_unx.c drw_x11.c
SRC = $(MSRC1)
SEED7_LIB_SRC = $(RSRC1) $(RSRC2) $(RSRC3) $(DSRC1)
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC1) $(LSRC1) $(LSRC2) $(LSRC3) $(ESRC1) $(ASRC1) $(ASRC2) $(ASRC3) $(GSRC1) $(GSRC2)

hi: $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB)
	$(CC) $(LFLAGS) $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB) $(LIBS) -o hi
	$(MAKE) ../prg/hi
	./hi level
#	cp hi /usr/local/bin/hi

hi.gp: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o /usr/local/bin/hi.gp
	hi level

../prg/hi:
	ln -s ../src/hi ../prg


clear: clean

clean:
	rm -f *.o *.a depend a_depend b_depend c_depend version.h

dep: depend

strip:
	strip /usr/local/bin/hi

version.h:
	echo "#define ANSI_C" > version.h
	echo "#define USE_DIRENT" >> version.h
	echo "#define PATH_DELIMITER '/'" >> version.h
	echo "#define CATCH_SIGNALS" >> version.h
	echo "#define HAS_SYMLINKS" >> version.h
	echo "#define HAS_FIFO_FILES" >> version.h
	echo "#define USE_LOCALTIME_R" >> version.h
	echo "#define USE_MMAP" >> version.h
	echo "#define $(TERMINFO_OR_TERMCAP)" >> version.h
	echo "#undef  INCL_NCURSES_TERM" >> version.h
	echo "#undef  INCL_CURSES_BEFORE_TERM" >> version.h
	echo "#define SCREEN_UTF8" >> version.h
	echo "#define OS_PATH_UTF8" >> version.h
	echo "#define _FILE_OFFSET_BITS 64" >> version.h
	echo "#define os_off_t off_t" >> version.h
	echo "#define USE_FSEEKO" >> version.h
	echo "#define ESCAPE_SPACES_IN_COMMANDS" >> version.h
	echo "#define USE_SIGSETJMP" >> version.h
	echo "#define $(BIGINT_LIB_DEFINE)" >> version.h
	echo "#include \"stdio.h\"" > chkccomp.c
	echo "#include \"time.h\"" >> chkccomp.c
	echo "int main (int argc, char **argv)" >> chkccomp.c
	echo "{" >> chkccomp.c
	echo "time_t timestamp;" >> chkccomp.c
	echo "struct tm *local_time;" >> chkccomp.c
	echo "long number;" >> chkccomp.c
	echo "printf(\"#define POINTER_SIZE %lu\", (long unsigned)(8 * sizeof(char *)));" >> chkccomp.c
	echo "puts(\"\");" >> chkccomp.c
	echo "printf(\"#define FLOAT_SIZE %lu\", (long unsigned)(8 * sizeof(float)));" >> chkccomp.c
	echo "puts(\"\");" >> chkccomp.c
	echo "printf(\"#define DOUBLE_SIZE %lu\", (long unsigned)(8 * sizeof(double)));" >> chkccomp.c
	echo "puts(\"\");" >> chkccomp.c
	echo "printf(\"#define TIME_T_SIZE %lu\", (long unsigned)(8 * sizeof(time_t)));" >> chkccomp.c
	echo "puts(\"\");" >> chkccomp.c
	echo "timestamp = -2147483648;" >> chkccomp.c
	echo "local_time = localtime(&timestamp);" >> chkccomp.c
	echo "if (local_time != NULL && local_time->tm_year == 1) {" >> chkccomp.c
	echo "puts(\"#define TIME_T_SIGNED\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "if (sizeof(int) == 4) {" >> chkccomp.c
	echo "puts(\"#define INT32TYPE int\");" >> chkccomp.c
	echo "puts(\"#define INT32TYPE_STRI \\\"int\\\"\");" >> chkccomp.c
	echo "puts(\"#define UINT32TYPE unsigned int\");" >> chkccomp.c
	echo "puts(\"#define UINT32TYPE_STRI \\\"unsigned int\\\"\");" >> chkccomp.c
	echo "} else if (sizeof(long) == 4) {" >> chkccomp.c
	echo "puts(\"#define INT32TYPE long\");" >> chkccomp.c
	echo "puts(\"#define INT32TYPE_STRI \\\"long\\\"\");" >> chkccomp.c
	echo "puts(\"#define UINT32TYPE unsigned long\");" >> chkccomp.c
	echo "puts(\"#define UINT32TYPE_STRI \\\"unsigned long\\\"\");" >> chkccomp.c
	echo "puts(\"#define INT32TYPE_SUFFIX_L\");" >> chkccomp.c
	echo "puts(\"#define INT32TYPE_FORMAT_L\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "if (sizeof(long) == 8) {" >> chkccomp.c
	echo "puts(\"#define INT64TYPE long\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_STRI \\\"long\\\"\");" >> chkccomp.c
	echo "puts(\"#define UINT64TYPE unsigned long\");" >> chkccomp.c
	echo "puts(\"#define UINT64TYPE_STRI \\\"unsigned long\\\"\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_SUFFIX_L\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_FORMAT_L\");" >> chkccomp.c
	echo "} else if (sizeof(long long) == 8) {" >> chkccomp.c
	echo "puts(\"#define INT64TYPE long long\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_STRI \\\"long long\\\"\");" >> chkccomp.c
	echo "puts(\"#define UINT64TYPE unsigned long long\");" >> chkccomp.c
	echo "puts(\"#define UINT64TYPE_STRI \\\"unsigned long long\\\"\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_SUFFIX_LL\");" >> chkccomp.c
	echo "puts(\"#define INT64TYPE_FORMAT_LL\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "number = -1;" >> chkccomp.c
	echo "if (number >> 1 == (long) -1) {" >> chkccomp.c
	echo "puts(\"#define RSHIFT_DOES_SIGN_EXTEND\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "if (~number == (long) 0) {" >> chkccomp.c
	echo "puts(\"#define TWOS_COMPLEMENT_INTTYPE\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "number = 1;" >> chkccomp.c
	echo "if (((char *) &number)[0] == 1) {" >> chkccomp.c
	echo "puts(\"#define LITTLE_ENDIAN_INTTYPE\");" >> chkccomp.c
	echo "}" >> chkccomp.c
	echo "return 0;" >> chkccomp.c
	echo "}" >> chkccomp.c
	$(CC) chkccomp.c -o chkccomp
	./chkccomp >> version.h
	rm chkccomp.c
	rm chkccomp
	echo "#define OBJECT_FILE_EXTENSION \".o\"" >> version.h
	echo "#define EXECUTABLE_FILE_EXTENSION \"\"" >> version.h
	echo "#define C_COMPILER \"$(CC)\"" >> version.h
	echo "#define CC_OPT_DEBUG_INFO \"-g\"" >> version.h
	echo "#define CC_OPT_NO_WARNINGS \"-w\"" >> version.h
	echo "#define REDIRECT_C_ERRORS \"2>\"" >> version.h
	echo "#define LINKER_OPT_OUTPUT_FILE \"-o \"" >> version.h
	echo "#define LINKER_FLAGS \"$(LFLAGS)\"" >> version.h
	echo "#define SYSTEM_LIBS \"$(LIBS)\"" >> version.h
	echo "#define SEED7_LIB \"`pwd`/$(SEED7_LIB)\"" >> version.h
	echo "#define COMP_DATA_LIB \"`pwd`/$(COMP_DATA_LIB)\"" >> version.h
	echo "#define COMPILER_LIB \"`pwd`/$(COMPILER_LIB)\"" >> version.h
	cd ../lib; echo "#define SEED7_LIBRARY" \"`pwd`\" >> ../src/version.h; cd ../src

hi.o: hi.c
	$(CC) $(CFLAGS) -c hi.c

depend: a_depend b_depend c_depend version.h
	$(CC) $(CFLAGS) -M $(SRC) > depend

a_depend: version.h
	$(CC) $(CFLAGS) -M $(SEED7_LIB_SRC) > a_depend

b_depend: version.h
	$(CC) $(CFLAGS) -M $(COMP_DATA_LIB_SRC) > b_depend

c_depend: version.h
	$(CC) $(CFLAGS) -M $(COMPILER_LIB_SRC) > c_depend

level.h:
	hi level

$(SEED7_LIB): $(SEED7_LIB_OBJ)
	ar r $(SEED7_LIB) $(SEED7_LIB_OBJ)

$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r $(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r $(COMPILER_LIB) $(COMPILER_LIB_OBJ)

wc: $(SRC)
	echo SRC:
	wc $(SRC)
	echo SEED7_LIB_SRC:
	wc $(SEED7_LIB_SRC)
	echo COMP_DATA_LIB_SRC:
	wc $(COMP_DATA_LIB_SRC)
	echo COMPILER_LIB_SRC:
	wc $(COMPILER_LIB_SRC)

lint: $(SRC)
	lint -p $(SRC) $(LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(LIBS)

ifeq (depend,$(wildcard depend))
include depend
include a_depend
include b_depend
include c_depend
endif
