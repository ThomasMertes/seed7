# Makefile for bcc32 and Borland make under windows. Commands executed by: cmd.exe
# To compile use a windows console and call:
#   make -f mk_bcc32.mak depend
#   make -f mk_bcc32.mak
# If you use MinGW you should use mk_mingw.mak, mk_nmake.mak or mk_msys.mak instead.
# If you use msvc you should use mk_msvc.mak instead.

# CFLAGS =
# CFLAGS = -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2
CFLAGS = -O2 -v
# CFLAGS = -O2 -v -w-
# CFLAGS = -O2 -g -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
# CFLAGS = -O2 -funroll-loops -Wall -pg
LFLAGS = -lS:0x400000
# LFLAGS = -pg
LIBS = user32.lib gdi32.lib ws2_32.lib
# LIBS = user32.lib gdi32.lib ws2_32.lib gmp.lib
SEED7_LIB = seed7_05.lib
COMP_DATA_LIB = s7_data.lib
COMPILER_LIB = s7_comp.lib
CC = bcc32

BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
BIGINT_LIB = big_rtl
# BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
# BIGINT_LIB = big_gmp

# SCREEN_OBJ = scr_x11.obj
# SCREEN_SRC = scr_x11.c
# SCREEN_OBJ = scr_infi.obj kbd_infi.obj trm_inf.obj
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_inf.c
# SCREEN_OBJ = scr_infp.obj kbd_infp.obj trm_cap.obj
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_cap.c
# SCREEN_OBJ = scr_cur.obj
# SCREEN_SRC = scr_cur.c
# SCREEN_OBJ = scr_cap.obj
# SCREEN_SRC = scr_cap.c
# SCREEN_OBJ = scr_tcp.obj
# SCREEN_SRC = scr_tcp.c
SCREEN_OBJ = scr_win.obj
SCREEN_SRC = scr_win.c

MOBJ1 = hi.obj
POBJ1 = runerr.obj option.obj primitiv.obj
LOBJ1 = actlib.obj arrlib.obj biglib.obj blnlib.obj bstlib.obj chrlib.obj cmdlib.obj dcllib.obj drwlib.obj enulib.obj
LOBJ2 = fillib.obj fltlib.obj hshlib.obj intlib.obj itflib.obj kbdlib.obj lstlib.obj prclib.obj prglib.obj reflib.obj
LOBJ3 = rfllib.obj scrlib.obj sctlib.obj setlib.obj soclib.obj strlib.obj timlib.obj typlib.obj ut8lib.obj
EOBJ1 = exec.obj doany.obj memory.obj
AOBJ1 = act_comp.obj prg_comp.obj analyze.obj syntax.obj token.obj parser.obj name.obj type.obj
AOBJ2 = expr.obj atom.obj object.obj scanner.obj literal.obj numlit.obj findid.obj
AOBJ3 = error.obj infile.obj symbol.obj info.obj stat.obj fatal.obj match.obj
GOBJ1 = syvarutl.obj traceutl.obj actutl.obj arrutl.obj executl.obj blockutl.obj
GOBJ2 = entutl.obj identutl.obj chclsutl.obj sigutl.obj
ROBJ1 = arr_rtl.obj bln_rtl.obj bst_rtl.obj chr_rtl.obj cmd_rtl.obj dir_rtl.obj drw_rtl.obj fil_rtl.obj flt_rtl.obj
ROBJ2 = hsh_rtl.obj int_rtl.obj kbd_rtl.obj scr_rtl.obj set_rtl.obj soc_rtl.obj str_rtl.obj tim_rtl.obj ut8_rtl.obj
ROBJ3 = heaputl.obj striutl.obj
DOBJ1 = $(BIGINT_LIB).obj $(SCREEN_OBJ) tim_win.obj drw_win.obj
OBJ = $(MOBJ1)
SEED7_LIB_OBJ = $(ROBJ1) $(ROBJ2) $(ROBJ3) $(DOBJ1)
COMP_DATA_LIB_OBJ = typ_data.obj rfl_data.obj ref_data.obj listutl.obj flistutl.obj typeutl.obj datautl.obj
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
GSRC1 = syvarutl.c traceutl.c actutl.c arrutl.c executl.c blockutl.c
GSRC2 = entutl.c identutl.c chclsutl.c sigutl.c
RSRC1 = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c flt_rtl.c
RSRC2 = hsh_rtl.c int_rtl.c kbd_rtl.c scr_rtl.c set_rtl.c soc_rtl.c str_rtl.c tim_rtl.c ut8_rtl.c
RSRC3 = heaputl.c striutl.c
DSRC1 = $(BIGINT_LIB).c $(SCREEN_SRC) tim_win.c drw_win.c
SRC = $(MSRC1)
SEED7_LIB_SRC = $(RSRC1) $(RSRC2) $(RSRC3) $(DSRC1)
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC1) $(LSRC1) $(LSRC2) $(LSRC3) $(ESRC1) $(ASRC1) $(ASRC2) $(ASRC3) $(GSRC1) $(GSRC2)

hi: $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB)
	$(CC) $(LFLAGS) -o hi.exe $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB) $(LIBS)
	copy hi.exe ..\prg /Y
	.\hi level

hi.gp: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o /usr/local/bin/hi.gp
	hi level

scr_x11.obj: scr_x11.c version.h scr_drv.h trm_drv.h
	$(CC) $(CFLAGS) -c scr_x11.c

scr_infi.obj: scr_inf.c version.h scr_drv.h trm_drv.h
	echo "#undef  USE_TERMCAP" > inf_conf.h
	$(CC) $(CFLAGS) -c scr_inf.c
	mv scr_inf.obj scr_infi.obj

scr_infp.obj: scr_inf.c version.h scr_drv.h trm_drv.h
	echo "#define USE_TERMCAP" > inf_conf.h
	$(CC) $(CFLAGS) -c scr_inf.c
	mv scr_inf.obj scr_infp.obj

kbd_infi.obj: kbd_inf.c version.h kbd_drv.h trm_drv.h
	echo "#undef  USE_TERMCAP" > inf_conf.h
	$(CC) $(CFLAGS) -c kbd_inf.c
	mv kbd_inf.obj kbd_infi.obj

kbd_infp.obj: kbd_inf.c version.h kbd_drv.h trm_drv.h
	echo "#define USE_TERMCAP" > inf_conf.h
	$(CC) $(CFLAGS) -c kbd_inf.c
	mv kbd_inf.obj kbd_infp.obj

trm_inf.obj: trm_inf.c version.h trm_drv.h
	$(CC) $(CFLAGS) -c trm_inf.c

trm_cap.obj: trm_cap.c version.h trm_drv.h
	$(CC) $(CFLAGS) -c trm_cap.c

scr_cur.obj: scr_cur.c version.h scr_drv.h
	$(CC) $(CFLAGS) -c scr_cur.c


clear: clean

clean:
	del version.h
	del calltlib.exe
	del a_depend
	del b_depend
	del c_depend
	del depend
	del *.obj
	del *.lib
	del *.tds
	del *.d

dep: depend

strip:
	strip /usr/local/bin/hi

version.h:
	cmd /S /C "echo #define ANSI_C" > version.h
	cmd /S /C "echo #define USE_DIRENT" >> version.h
	cmd /S /C "echo #define PATH_DELIMITER '\\'" >> version.h
	cmd /S /C "echo #define NO_EMPTY_STRUCTS" >> version.h
	cmd /S /C "echo #define CATCH_SIGNALS" >> version.h
	cmd /S /C "echo #define USE_LOCALTIME_R" >> version.h
	cmd /S /C "echo #define USE_ALTERNATE_LOCALTIME_R" >> version.h
	cmd /S /C "echo #define USE_ALTERNATE_UTIME" >> version.h
	cmd /S /C "echo #undef  USE_MMAP" >> version.h
	cmd /S /C "echo #undef  INCL_NCURSES_TERM" >> version.h
	cmd /S /C "echo #undef  INCL_CURSES_BEFORE_TERM" >> version.h
	cmd /S /C "echo #define TURN_OFF_FP_EXCEPTIONS" >> version.h
	cmd /S /C "echo #define DEFINE_MATHERR_FUNCTION" >> version.h
	cmd /S /C "echo #define ISNAN_WITH_UNDERLINE" >> version.h
	cmd /S /C "echo #define CHECK_INT_DIV_BY_ZERO" >> version.h
	cmd /S /C "echo #define USE_MYUNISTD_H" >> version.h
	cmd /S /C "echo #define INT64TYPE __int64" >> version.h
	cmd /S /C "echo #define UINT64TYPE unsigned __int64" >> version.h
	cmd /S /C "echo #define INT64TYPE_SUFFIX_LL" >> version.h
	cmd /S /C "echo #define OS_PATH_WCHAR" >> version.h
	cmd /S /C "echo #define OS_WIDE_DIR_INCLUDE_DIR_H" >> version.h
	cmd /S /C "echo #define OS_CHMOD_INCLUDE_IO_H" >> version.h
	cmd /S /C "echo #define os_chdir _wchdir" >> version.h
	cmd /S /C "echo #define os_getcwd _wgetcwd" >> version.h
	cmd /S /C "echo #define os_mkdir(path,mode) _wmkdir(path)" >> version.h
	cmd /S /C "echo #define os_rmdir _wrmdir" >> version.h
	cmd /S /C "echo #define os_opendir wopendir" >> version.h
	cmd /S /C "echo #define os_readdir wreaddir" >> version.h
	cmd /S /C "echo #define os_closedir wclosedir" >> version.h
	cmd /S /C "echo #define os_DIR wDIR" >> version.h
	cmd /S /C "echo #define os_dirent_struct struct wdirent >> version.h
	cmd /S /C "echo #define os_fstat _fstat" >> version.h
	cmd /S /C "echo #define os_lstat _wstat" >> version.h
	cmd /S /C "echo #define os_stat _wstat" >> version.h
	cmd /S /C "echo #define os_stat_struct struct _stat" >> version.h
	cmd /S /C "echo #define os_chown(NAME,UID,GID)" >> version.h
	cmd /S /C "echo #define os_chmod _wchmod" >> version.h
	cmd /S /C "echo #define os_utime_orig _wutime" >> version.h
	cmd /S /C "echo #define os_utime alternate_utime" >> version.h
	cmd /S /C "echo #define os_utimbuf_struct struct utimbuf" >> version.h
	cmd /S /C "echo #define os_remove _wremove" >> version.h
	cmd /S /C "echo #define os_rename _wrename" >> version.h
	cmd /S /C "echo #define wide_fopen _wfopen" >> version.h
	cmd /S /C "echo #define USE_WINSOCK" >> version.h
	cmd /S /C "echo #define popen _popen" >> version.h
	cmd /S /C "echo #define $(BIGINT_LIB_DEFINE)" >> version.h
	cmd /S /C "echo #include "stdio.h"" > chkftell.c
	cmd /S /C "echo int main (int argc, char **argv)" >> chkftell.c
	cmd /S /C "echo {" >> chkftell.c
	cmd /S /C "echo FILE *aFile;" >> chkftell.c
	cmd /S /C "echo aFile = _popen("dir","r");" >> chkftell.c
	cmd /S /C "echo if (ftell(aFile) != -1) {" >> chkftell.c
	cmd /S /C "echo puts("\043define FTELL_WRONG_FOR_PIPE");" >> chkftell.c
	cmd /S /C "echo }" >> chkftell.c
	cmd /S /C "echo return 0;" >> chkftell.c
	cmd /S /C "echo }" >> chkftell.c
	$(CC) chkftell.c
	chkftell >> version.h
	del chkftell.c
	del chkftell.obj
	del chkftell.tds
	del chkftell.exe
	cmd /S /C "echo #include "stdio.h"" > chkccomp.c
	cmd /S /C "echo int main (int argc, char **argv)" >> chkccomp.c
	cmd /S /C "echo {" >> chkccomp.c
	cmd /S /C "echo long number;" >> chkccomp.c
	cmd /S /C "echo number = -1;" >> chkccomp.c
	cmd /S /C "echo if (number ^>^> 1 == (long) -1) {" >> chkccomp.c
	cmd /S /C "echo puts("\043define RSHIFT_DOES_SIGN_EXTEND");" >> chkccomp.c
	cmd /S /C "echo }" >> chkccomp.c
	cmd /S /C "echo if (~number == (long) 0) {" >> chkccomp.c
	cmd /S /C "echo puts("\043define TWOS_COMPLEMENT_INTTYPE");" >> chkccomp.c
	cmd /S /C "echo }" >> chkccomp.c
	cmd /S /C "echo return 0;" >> chkccomp.c
	cmd /S /C "echo }" >> chkccomp.c
	$(CC) chkccomp.c
	chkccomp >> version.h
	del chkccomp.c
	del chkccomp.obj
	del chkccomp.tds
	del chkccomp.exe
	cmd /S /C "echo #define OBJECT_FILE_EXTENSION ".obj"" >> version.h
	cmd /S /C "echo #define EXECUTABLE_FILE_EXTENSION ".exe"" >> version.h
	cmd /S /C "echo #define C_COMPILER "$(CC)"" >> version.h
	cmd /S /C "echo #define INHIBIT_C_WARNINGS "-w-"" >> version.h
	cmd /S /C "echo #define REDIRECT_C_ERRORS "\076"" >> version.h
	cmd /S /C "echo #define LINKER_FLAGS "$(LFLAGS)"" >> version.h
	cmd /S /C "echo #define SYSTEM_LIBS "$(LIBS)"" >> version.h
	cmd /S /C "echo #include "stdio.h"" > setpaths.c
	cmd /S /C "echo #include "stddef.h"" >> setpaths.c
	cmd /S /C "echo int chdir(char *path);" >> setpaths.c
	cmd /S /C "echo char *getcwd(char *buf, size_t size);" >> setpaths.c
	cmd /S /C "echo int main (int argc, char **argv)" >> setpaths.c
	cmd /S /C "echo {" >> setpaths.c
	cmd /S /C "echo char buffer[4096];" >> setpaths.c
	cmd /S /C "echo int position;" >> setpaths.c
	cmd /S /C "echo getcwd(buffer, sizeof(buffer));" >> setpaths.c
	cmd /S /C "echo printf("\043define SEED7_LIB \042");" >> setpaths.c
	cmd /S /C "echo for (position = 0; buffer[position] != '\0'; position++) {" >> setpaths.c
	cmd /S /C "echo   putchar(buffer[position] == '\\' ? '/' : buffer[position]);" >> setpaths.c
	cmd /S /C "echo }" >> setpaths.c
	cmd /S /C "echo printf("/$(SEED7_LIB)\042\n");" >> setpaths.c
	cmd /S /C "echo getcwd(buffer, sizeof(buffer));" >> setpaths.c
	cmd /S /C "echo printf("\043define COMP_DATA_LIB \042");" >> setpaths.c
	cmd /S /C "echo for (position = 0; buffer[position] != '\0'; position++) {" >> setpaths.c
	cmd /S /C "echo   putchar(buffer[position] == '\\' ? '/' : buffer[position]);" >> setpaths.c
	cmd /S /C "echo }" >> setpaths.c
	cmd /S /C "echo printf("/$(COMP_DATA_LIB)\042\n");" >> setpaths.c
	cmd /S /C "echo getcwd(buffer, sizeof(buffer));" >> setpaths.c
	cmd /S /C "echo printf("\043define COMPILER_LIB \042");" >> setpaths.c
	cmd /S /C "echo for (position = 0; buffer[position] != '\0'; position++) {" >> setpaths.c
	cmd /S /C "echo   putchar(buffer[position] == '\\' ? '/' : buffer[position]);" >> setpaths.c
	cmd /S /C "echo }" >> setpaths.c
	cmd /S /C "echo printf("/$(COMPILER_LIB)\042\n");" >> setpaths.c
	cmd /S /C "echo chdir("../lib");" >> setpaths.c
	cmd /S /C "echo getcwd(buffer, sizeof(buffer));" >> setpaths.c
	cmd /S /C "echo printf("\043define SEED7_LIBRARY \042");" >> setpaths.c
	cmd /S /C "echo for (position = 0; buffer[position] != '\0'; position++) {" >> setpaths.c
	cmd /S /C "echo   putchar(buffer[position] == '\\' ? '/' : buffer[position]);" >> setpaths.c
	cmd /S /C "echo }" >> setpaths.c
	cmd /S /C "echo printf("\042\n");" >> setpaths.c
	cmd /S /C "echo return 0;" >> setpaths.c
	cmd /S /C "echo }" >> setpaths.c
	$(CC) setpaths.c
	setpaths >> version.h
	del setpaths.c
	del setpaths.obj
	del setpaths.tds
	del setpaths.exe
	cmd /S /C "echo #include "stdio.h"" > calltlib.c
	cmd /S /C "echo #include "string.h"" >> calltlib.c
	cmd /S /C "echo int main (int argc, char **argv)" >> calltlib.c
	cmd /S /C "echo {" >> calltlib.c
	cmd /S /C "echo char buffer[4096];" >> calltlib.c
	cmd /S /C "echo int number;" >> calltlib.c
	cmd /S /C "echo strcpy(buffer, "tlib /C ");" >> calltlib.c
	cmd /S /C "echo strcat(buffer, argv[1]);" >> calltlib.c
	cmd /S /C "echo strcat(buffer, " ");" >> calltlib.c
	cmd /S /C "echo for (number = 2; number ^< argc; number++) {" >> calltlib.c
	cmd /S /C "echo   strcat(buffer, "-+");" >> calltlib.c
	cmd /S /C "echo   strcat(buffer, argv[number]);" >> calltlib.c
	cmd /S /C "echo }" >> calltlib.c
	cmd /S /C "echo puts(buffer);" >> calltlib.c
	cmd /S /C "echo system(buffer);" >> calltlib.c
	cmd /S /C "echo return 0;" >> calltlib.c
	cmd /S /C "echo }" >> calltlib.c
	$(CC) calltlib.c
	del calltlib.c
	del calltlib.obj
	del calltlib.tds

hi.obj: hi.c
	$(CC) $(CFLAGS) -c hi.c

.c.obj:
	$(CC) $(CFLAGS) -c $<

depend: a_depend b_depend c_depend version.h
	$(CC) $(CFLAGS) -c -w- -m -md $(SRC)
	copy *.d depend
	del *.d
	del $(OBJ)

a_depend: version.h
	$(CC) $(CFLAGS) -c -w- -m -md $(SEED7_LIB_SRC)
	copy *.d a_depend
	del *.d
	del $(SEED7_LIB_OBJ)

b_depend: version.h
	$(CC) $(CFLAGS) -c -w- -m -md $(COMP_DATA_LIB_SRC)
	copy *.d b_depend
	del *.d
	del $(COMP_DATA_LIB_OBJ)

c_depend: version.h
	$(CC) $(CFLAGS) -c -w- -m -md $(COMPILER_LIB_SRC)
	copy *.d c_depend
	del *.d
	del $(COMPILER_LIB_OBJ)

level.h:
	hi level

$(SEED7_LIB): $(SEED7_LIB_OBJ)
	calltlib $(SEED7_LIB) $(SEED7_LIB_OBJ)

$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	calltlib $(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	calltlib $(COMPILER_LIB) $(COMPILER_LIB_OBJ)

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

!if "exist depend"
!include depend
!endif
!if "exist a_depend"
!include a_depend
!endif
!if "exist b_depend"
!include b_depend
!endif
!if "exist c_depend"
!include c_depend
!endif
