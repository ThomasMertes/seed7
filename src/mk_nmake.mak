# Makefile for nmake from Windows and gcc from MinGW. Commands executed by: cmd.exe
# To compile use a Windows console and call:
#   nmake /f mk_nmake.mak depend
#   nmake /f mk_nmake.mak
# When your make utility is mingw32-make, you should use mk_mingw.mak instead.
# When you are using the MSYS console from MinGW, you should use mk_msys.mak instead.

# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LDFLAGS = -Wl,--stack,4194304
# LDFLAGS = -pg
# LDFLAGS = -pg -lc_p
LIBS = -lm -lgdi32 -lws2_32
# LIBS = -lm -lgdi32 -lws2_32 -lgmp
SEED7_LIB = seed7_05.a
COMP_DATA_LIB = s7_data.a
COMPILER_LIB = s7_comp.a
CC = gcc
GET_CC_VERSION_INFO = $(CC) --version >

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
# TERMINFO_OR_TERMCAP = USE_TERMINFO
# SCREEN_OBJ = scr_inf.o kbd_poll.o trm_inf.o
# SCREEN_SRC = scr_inf.c kbd_poll.c trm_inf.c
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
SCREEN_OBJ = scr_win.o
SCREEN_SRC = scr_win.c

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
DOBJ1 = $(BIGINT_LIB).o $(SCREEN_OBJ) tim_win.o drw_win.o cmd_win.o
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
DSRC1 = $(BIGINT_LIB).c $(SCREEN_SRC) tim_win.c drw_win.c cmd_win.c
SRC = $(MSRC1)
SEED7_LIB_SRC = $(RSRC1) $(RSRC2) $(RSRC3) $(DSRC1)
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC1) $(LSRC1) $(LSRC2) $(LSRC3) $(ESRC1) $(ASRC1) $(ASRC2) $(ASRC3) $(GSRC1) $(GSRC2)

..\bin\hi.exe: $(OBJ) ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(SEED7_LIB)
	$(CC) $(LDFLAGS) $(OBJ) ..\bin\$(COMPILER_LIB) ..\bin\$(COMP_DATA_LIB) ..\bin\$(SEED7_LIB) $(LIBS) -o ..\bin\hi
	copy ..\bin\hi.exe ..\prg /Y
	..\bin\hi level

hi: ..\bin\hi.exe

clear: clean

clean:
	del version.h
	del depend
	del a_depend
	del b_depend
	del c_depend
	del *.o
	del ..\bin\*.a

dep: depend

strip:
	strip ..\bin\hi.exe

version.h:
	echo #define ANSI_C > version.h
	echo #define USE_DIRENT >> version.h
	echo #define PATH_DELIMITER '\\' >> version.h
	echo #define ALLOW_DRIVE_LETTERS >> version.h
	echo #define CATCH_SIGNALS >> version.h
	echo #define USE_ALTERNATE_UTIME >> version.h
	echo #define OS_PATH_WCHAR >> version.h
	echo #define os_chdir _wchdir >> version.h
	echo #define os_getcwd _wgetcwd >> version.h
	echo #define os_mkdir(path,mode) _wmkdir(path) >> version.h
	echo #define os_rmdir _wrmdir >> version.h
	echo #define os_opendir _wopendir >> version.h
	echo #define os_readdir _wreaddir >> version.h
	echo #define os_closedir _wclosedir >> version.h
	echo #define os_DIR _WDIR >> version.h
	echo #define os_dirent_struct struct _wdirent >> version.h
	echo #define os_fstat _fstati64 >> version.h
	echo #define os_lstat _wstati64 >> version.h
	echo #define os_stat _wstati64 >> version.h
	echo #define os_stat_struct struct _stati64 >> version.h
	echo #define os_chown(name,uid,gid) >> version.h
	echo #define os_chmod _wchmod >> version.h
	echo #define os_utime_orig _wutime >> version.h
	echo #define os_utime alternate_utime >> version.h
	echo #define os_utimbuf_struct struct _utimbuf >> version.h
	echo #define os_remove _wremove >> version.h
	echo #define os_rename _wrename >> version.h
	echo #define os_system _wsystem >> version.h
	echo #define os_pclose _pclose >> version.h
	echo #define os_popen _wpopen >> version.h
	echo #define wide_fopen _wfopen >> version.h
	echo #define os_fseek fseeko64 >> version.h
	echo #define os_ftell ftello64 >> version.h
	echo #define os_off_t off64_t >> version.h
	echo #define USE_WINSOCK >> version.h
	echo #define $(BIGINT_LIB_DEFINE) >> version.h
	echo #define likely(x)   __builtin_expect((x),1) >> version.h
	echo #define unlikely(x) __builtin_expect((x),0) >> version.h
	echo #include "direct.h" > chkccomp.h
	echo #define WRITE_CC_VERSION_INFO system("$(GET_CC_VERSION_INFO) cc_version"); >> chkccomp.h
	echo #define mkdir(path,mode) mkdir(path) >> chkccomp.h
	echo #define LIST_DIRECTORY_CONTENTS "dir" >> chkccomp.h
	echo #define long_long_EXISTS >> chkccomp.h
	echo #define long_long_SUFFIX_LL >> chkccomp.h
	$(CC) -o chkccomp chkccomp.c
	.\chkccomp.exe >> version.h
	del chkccomp.h
	del chkccomp.exe
	del cc_version
	echo #define OBJECT_FILE_EXTENSION ".o" >> version.h
	echo #define EXECUTABLE_FILE_EXTENSION ".exe" >> version.h
	echo #define C_COMPILER "$(CC)" >> version.h
	echo #define GET_CC_VERSION_INFO "$(GET_CC_VERSION_INFO)" >> version.h
	echo #define CC_OPT_DEBUG_INFO "-g" >> version.h
	echo #define CC_OPT_NO_WARNINGS "-w" >> version.h
	echo #define REDIRECT_C_ERRORS "2>" >> version.h
	echo #define LINKER_OPT_OUTPUT_FILE "-o " >> version.h
	echo #define LINKER_FLAGS "$(LDFLAGS)" >> version.h
	echo #define SYSTEM_LIBS "$(LIBS)" >> version.h
	echo #include "stdio.h" > setpaths.c
	echo #include "stddef.h" >> setpaths.c
	echo int chdir(char *path); >> setpaths.c
	echo char *getcwd(char *buf, size_t size); >> setpaths.c
	echo int main (int argc, char **argv) >> setpaths.c
	echo { >> setpaths.c
	echo char buffer[4096]; >> setpaths.c
	echo int position; >> setpaths.c
	echo chdir("../bin"); >> setpaths.c
	echo getcwd(buffer, sizeof(buffer)); >> setpaths.c
	echo printf("\043define SEED7_LIB \042"); >> setpaths.c
	echo for (position = 0; buffer[position] != '\0'; position++) { >> setpaths.c
	echo putchar(buffer[position] == '\\' ? '/' : buffer[position]); >> setpaths.c
	echo } >> setpaths.c
	echo printf("/$(SEED7_LIB)\042\n"); >> setpaths.c
	echo getcwd(buffer, sizeof(buffer)); >> setpaths.c
	echo printf("\043define COMP_DATA_LIB \042"); >> setpaths.c
	echo for (position = 0; buffer[position] != '\0'; position++) { >> setpaths.c
	echo putchar(buffer[position] == '\\' ? '/' : buffer[position]); >> setpaths.c
	echo } >> setpaths.c
	echo printf("/$(COMP_DATA_LIB)\042\n"); >> setpaths.c
	echo getcwd(buffer, sizeof(buffer)); >> setpaths.c
	echo printf("\043define COMPILER_LIB \042"); >> setpaths.c
	echo for (position = 0; buffer[position] != '\0'; position++) { >> setpaths.c
	echo putchar(buffer[position] == '\\' ? '/' : buffer[position]); >> setpaths.c
	echo } >> setpaths.c
	echo printf("/$(COMPILER_LIB)\042\n"); >> setpaths.c
	echo chdir("../lib"); >> setpaths.c
	echo getcwd(buffer, sizeof(buffer)); >> setpaths.c
	echo printf("\043define SEED7_LIBRARY \042"); >> setpaths.c
	echo for (position = 0; buffer[position] != '\0'; position++) { >> setpaths.c
	echo putchar(buffer[position] == '\\' ? '/' : buffer[position]); >> setpaths.c
	echo } >> setpaths.c
	echo printf("\042\n"); >> setpaths.c
	echo return 0; >> setpaths.c
	echo } >> setpaths.c
	$(CC) -o setpaths setpaths.c
	.\setpaths.exe >> version.h
	del setpaths.c
	del setpaths.exe

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: a_depend b_depend c_depend version.h
	$(CC) $(CFLAGS) -M $(SRC) > depend

a_depend: version.h
	$(CC) $(CFLAGS) -M $(SEED7_LIB_SRC) > a_depend

b_depend: version.h
	$(CC) $(CFLAGS) -M $(COMP_DATA_LIB_SRC) > b_depend

c_depend: version.h
	$(CC) $(CFLAGS) -M $(COMPILER_LIB_SRC) > c_depend

level.h:
	..\bin\hi level

..\bin\$(SEED7_LIB): $(SEED7_LIB_OBJ)
	ar r ..\bin\$(SEED7_LIB) $(SEED7_LIB_OBJ)

..\bin\$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	ar r ..\bin\$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

..\bin\$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	ar r ..\bin\$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

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

!if exist(depend)
!include depend
!endif
!if exist(a_depend)
!include a_depend
!endif
!if exist(b_depend)
!include b_depend
!endif
!if exist(c_depend)
!include c_depend
!endif
