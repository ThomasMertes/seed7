# Makefile for nmake from windows and cl from msvc. Commands executed by: cmd.exe
# To compile use a windows console and call:
#   nmake /f mk_msvc.mak depend
#   nmake /f mk_msvc.mak
# If you use MinGW you should use mk_mingw.mak, mk_nmake.mak or mk_msys.mak instead.
# If you use bcc32 you should use mk_bcc32.mak instead.

# CFLAGS = -AL -Ozax -Gr -Gs -Gm -G0 -W4
CFLAGS = -O2 -W4
# CFLAGS = -O2 -W4 /Zi /Yd
# CFLAGS = -W4 /Zi /Yd /GZ
# CFLAGS = -O2 -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -pg -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LFLAGS = /F0x400000
# LFLAGS = -pg
LIBS = user32.lib gdi32.lib ws2_32.lib
# LIBS = user32.lib gdi32.lib ws2_32.lib gmp.lib
SEED7_LIB = seed7_05.lib
COMP_DATA_LIB = s7_data.lib
COMPILER_LIB = s7_comp.lib
CC = cl
GET_CC_VERSION_INFO = $(CC) 2>

BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
BIGINT_LIB = big_rtl
# BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
# BIGINT_LIB = big_gmp

# TERMINFO_OR_TERMCAP = USE_TERMINFO
# SCREEN_OBJ = scr_inf.obj kbd_inf.obj trm_inf.obj
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_inf.c
# TERMINFO_OR_TERMCAP = USE_TERMCAP
# SCREEN_OBJ = scr_inf.obj kbd_inf.obj trm_cap.obj
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_cap.c
# TERMINFO_OR_TERMCAP = USE_TERMINFO
# SCREEN_OBJ = scr_inf.obj kbd_poll.obj trm_inf.obj
# SCREEN_SRC = scr_inf.c kbd_poll.c trm_inf.c
# TERMINFO_OR_TERMCAP = USE_TERMCAP
# SCREEN_OBJ = scr_inf.obj kbd_poll.obj trm_cap.obj
# SCREEN_SRC = scr_inf.c kbd_poll.c trm_cap.c

# SCREEN_OBJ = scr_cur.obj
# SCREEN_SRC = scr_cur.c
# SCREEN_OBJ = scr_cap.obj
# SCREEN_SRC = scr_cap.c
# SCREEN_OBJ = scr_tcp.obj
# SCREEN_SRC = scr_tcp.c
# SCREEN_OBJ = scr_x11.obj
# SCREEN_SRC = scr_x11.c
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
GOBJ1 = syvarutl.obj traceutl.obj actutl.obj executl.obj blockutl.obj
GOBJ2 = entutl.obj identutl.obj chclsutl.obj sigutl.obj
ROBJ1 = arr_rtl.obj bln_rtl.obj bst_rtl.obj chr_rtl.obj cmd_rtl.obj dir_rtl.obj drw_rtl.obj fil_rtl.obj flt_rtl.obj
ROBJ2 = hsh_rtl.obj int_rtl.obj kbd_rtl.obj scr_rtl.obj set_rtl.obj soc_rtl.obj str_rtl.obj tim_rtl.obj ut8_rtl.obj
ROBJ3 = heaputl.obj striutl.obj
DOBJ1 = $(BIGINT_LIB).obj $(SCREEN_OBJ) tim_win.obj drw_win.obj dir_win.obj
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
GSRC1 = syvarutl.c traceutl.c actutl.c executl.c blockutl.c
GSRC2 = entutl.c identutl.c chclsutl.c sigutl.c
RSRC1 = arr_rtl.c bln_rtl.c bst_rtl.c chr_rtl.c cmd_rtl.c dir_rtl.c drw_rtl.c fil_rtl.c flt_rtl.c
RSRC2 = hsh_rtl.c int_rtl.c kbd_rtl.c scr_rtl.c set_rtl.c soc_rtl.c str_rtl.c tim_rtl.c ut8_rtl.c
RSRC3 = heaputl.c striutl.c
DSRC1 = $(BIGINT_LIB).c $(SCREEN_SRC) tim_win.c drw_win.c dir_win.c
SRC = $(MSRC1)
SEED7_LIB_SRC = $(RSRC1) $(RSRC2) $(RSRC3) $(DSRC1)
COMP_DATA_LIB_SRC = typ_data.c rfl_data.c ref_data.c listutl.c flistutl.c typeutl.c datautl.c
COMPILER_LIB_SRC = $(PSRC1) $(LSRC1) $(LSRC2) $(LSRC3) $(ESRC1) $(ASRC1) $(ASRC2) $(ASRC3) $(GSRC1) $(GSRC2)

hi: $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB)
	$(CC) $(LFLAGS) -o hi $(OBJ) $(COMPILER_LIB) $(COMP_DATA_LIB) $(SEED7_LIB) $(LIBS)
	copy hi.exe ..\prg /Y
	.\hi level

hi.gp: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o /usr/local/bin/hi.gp
	hi level


clear: clean

clean:
	del version.h
	del *.obj
	del *.lib

dep: depend

strip:
	strip /usr/local/bin/hi

version.h:
	echo #define ANSI_C > version.h
	echo #define USE_DIRWIN >> version.h
	echo #define PATH_DELIMITER '\\' >> version.h
	echo #define ALLOW_DRIVE_LETTERS >> version.h
	echo #define NO_EMPTY_STRUCTS >> version.h
	echo #define CATCH_SIGNALS >> version.h
	echo #define USE_ALTERNATE_UTIME >> version.h
	echo #undef  USE_MMAP >> version.h
	echo #undef  INCL_NCURSES_TERM >> version.h
	echo #undef  INCL_CURSES_BEFORE_TERM >> version.h
	echo #define INCLUDE_SYS_UTIME >> version.h
	echo #define REMOVE_FAILS_FOR_EMPTY_DIRS >> version.h
	echo #define ISNAN_WITH_UNDERLINE >> version.h
	echo #define CHECK_INT_DIV_BY_ZERO >> version.h
	echo #define FLOAT_ZERO_DIV_ERROR >> version.h
	echo #define USE_MYUNISTD_H >> version.h
	echo #define OS_PATH_WCHAR >> version.h
	echo #define os_chdir _wchdir >> version.h
	echo #define os_getcwd _wgetcwd >> version.h
	echo #define os_mkdir(path,mode) _wmkdir(path) >> version.h
	echo #define os_rmdir _wrmdir >> version.h
	echo #define os_opendir wopendir >> version.h
	echo #define os_readdir wreaddir >> version.h
	echo #define os_closedir wclosedir >> version.h
	echo #define os_DIR WDIR >> version.h
	echo #define os_dirent_struct struct wdirent >> version.h
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
	echo #define os_fseek _fseeki64 >> version.h
	echo #define os_ftell _ftelli64 >> version.h
	echo #define os_off_t __int64 >> version.h
	echo #define USE_WINSOCK >> version.h
	echo #define $(BIGINT_LIB_DEFINE) >> version.h
	$(GET_CC_VERSION_INFO) cc_version
	echo #include "stdlib.h" > chkccomp.c
	echo #include "stdio.h" >> chkccomp.c
	echo #include "time.h" >> chkccomp.c
	echo int main (int argc, char **argv) >> chkccomp.c
	echo { >> chkccomp.c
	echo FILE *aFile; >> chkccomp.c
	echo time_t timestamp; >> chkccomp.c
	echo struct tm *local_time; >> chkccomp.c
	echo long number; >> chkccomp.c
	echo int ch; >> chkccomp.c
	echo aFile = fopen("cc_version","r"); >> chkccomp.c
	echo printf("\043define C_COMPILER_VERSION \042"); >> chkccomp.c
	echo for (ch=getc(aFile); ch!=EOF ^&^& ch!=10 ^&^& ch!=13; ch=getc(aFile)) { >> chkccomp.c
	echo if (ch^>=' ' ^&^& ch^<='~') { >> chkccomp.c
	echo if (ch==34 ^|^| ch==39 ^|^| ch==92) putchar(92); >> chkccomp.c
	echo putchar(ch); >> chkccomp.c
	echo } else { >> chkccomp.c
	echo putchar(92); >> chkccomp.c
	echo printf("%3o", ch); >> chkccomp.c
	echo } >> chkccomp.c
	echo } >> chkccomp.c
	echo puts("\042"); >> chkccomp.c
	echo fclose(aFile); >> chkccomp.c
	echo aFile = _popen("dir","r"); >> chkccomp.c
	echo if (ftell(aFile) != -1) { >> chkccomp.c
	echo puts("\043define FTELL_WRONG_FOR_PIPE"); >> chkccomp.c
	echo } >> chkccomp.c
	echo if ((aFile = fopen("tmp_test_file","w")) != NULL) { >> chkccomp.c
	echo fwrite("asdf",1,4,aFile); >> chkccomp.c
	echo fclose(aFile); >> chkccomp.c
	echo if ((aFile = fopen("tmp_test_file","r")) != NULL) { >> chkccomp.c
	echo if (fwrite("qwert",1,5,aFile) != 0) { >> chkccomp.c
	echo puts("\043define FWRITE_WRONG_FOR_READ_ONLY_FILES"); >> chkccomp.c
	echo } >> chkccomp.c
	echo fclose(aFile); >> chkccomp.c
	echo } >> chkccomp.c
	echo remove("tmp_test_file"); >> chkccomp.c
	echo } >> chkccomp.c
	echo printf("\043define POINTER_SIZE %d", 8 * sizeof(char *)); >> chkccomp.c
	echo puts(""); >> chkccomp.c
	echo printf("\043define FLOAT_SIZE %d", 8 * sizeof(float)); >> chkccomp.c
	echo puts(""); >> chkccomp.c
	echo printf("\043define DOUBLE_SIZE %d", 8 * sizeof(double)); >> chkccomp.c
	echo puts(""); >> chkccomp.c
	echo printf("\043define TIME_T_SIZE %d", 8 * sizeof(time_t)); >> chkccomp.c
	echo puts(""); >> chkccomp.c
	echo timestamp = -2147483648; >> chkccomp.c
	echo local_time = localtime(^&timestamp); >> chkccomp.c
	echo if (local_time != NULL ^&^& local_time-^>tm_year == 1) { >> chkccomp.c
	echo puts("\043define TIME_T_SIGNED"); >> chkccomp.c
	echo } >> chkccomp.c
	echo if (sizeof(int) == 4) { >> chkccomp.c
	echo puts("\043define INT32TYPE int"); >> chkccomp.c
	echo puts("\043define INT32TYPE_STRI \"int\""); >> chkccomp.c
	echo puts("\043define UINT32TYPE unsigned int"); >> chkccomp.c
	echo puts("\043define UINT32TYPE_STRI \"unsigned int\""); >> chkccomp.c
	echo } else if (sizeof(long) == 4) { >> chkccomp.c
	echo puts("\043define INT32TYPE long"); >> chkccomp.c
	echo puts("\043define INT32TYPE_STRI \"long\""); >> chkccomp.c
	echo puts("\043define UINT32TYPE unsigned long"); >> chkccomp.c
	echo puts("\043define UINT32TYPE_STRI \"unsigned long\""); >> chkccomp.c
	echo puts("\043define INT32TYPE_SUFFIX_L"); >> chkccomp.c
	echo puts("\043define INT32TYPE_FORMAT_L"); >> chkccomp.c
	echo } >> chkccomp.c
	echo if (sizeof(long) == 8) { >> chkccomp.c
	echo puts("\043define INT64TYPE long"); >> chkccomp.c
	echo puts("\043define INT64TYPE_STRI \"long\""); >> chkccomp.c
	echo puts("\043define UINT64TYPE unsigned long"); >> chkccomp.c
	echo puts("\043define UINT64TYPE_STRI \"unsigned long\""); >> chkccomp.c
	echo puts("\043define INT64TYPE_SUFFIX_L"); >> chkccomp.c
	echo puts("\043define INT64TYPE_FORMAT_L"); >> chkccomp.c
	echo } else if (sizeof(__int64) == 8) { >> chkccomp.c
	echo puts("\043define INT64TYPE __int64"); >> chkccomp.c
	echo puts("\043define INT64TYPE_STRI \"__int64\""); >> chkccomp.c
	echo puts("\043define UINT64TYPE unsigned __int64"); >> chkccomp.c
	echo puts("\043define UINT64TYPE_STRI \"unsigned __int64\""); >> chkccomp.c
	echo puts("\043define INT64TYPE_FORMAT_I64"); >> chkccomp.c
	echo } >> chkccomp.c
	echo number = -1; >> chkccomp.c
	echo if (number ^>^> 1 == (long) -1) { >> chkccomp.c
	echo puts("\043define RSHIFT_DOES_SIGN_EXTEND"); >> chkccomp.c
	echo } >> chkccomp.c
	echo if (~number == (long) 0) { >> chkccomp.c
	echo puts("\043define TWOS_COMPLEMENT_INTTYPE"); >> chkccomp.c
	echo } >> chkccomp.c
	echo number = 1; >> chkccomp.c
	echo if (((char *) ^&number)[0] == 1) { >> chkccomp.c
	echo puts("\043define LITTLE_ENDIAN_INTTYPE"); >> chkccomp.c
	echo } >> chkccomp.c
	echo return 0; >> chkccomp.c
	echo } >> chkccomp.c
	$(CC) -o chkccomp chkccomp.c
	.\chkccomp >> version.h
	del chkccomp.c
	del chkccomp.obj
	del chkccomp.exe
	del cc_version
	echo #define OBJECT_FILE_EXTENSION ".obj" >> version.h
	echo #define EXECUTABLE_FILE_EXTENSION ".exe" >> version.h
	echo #define C_COMPILER "$(CC)" >> version.h
	echo #define GET_CC_VERSION_INFO "$(GET_CC_VERSION_INFO)" >> version.h
	echo #define CC_OPT_DEBUG_INFO "-Zi -Yd" >> version.h
	echo #define CC_OPT_NO_WARNINGS "-w" >> version.h
	echo #define REDIRECT_C_ERRORS "2>NUL: >" >> version.h
	echo #define LINKER_OPT_OUTPUT_FILE "-o " >> version.h
	echo #define LINKER_FLAGS "$(LFLAGS)" >> version.h
	echo #define SYSTEM_LIBS "$(LIBS)" >> version.h
	echo #include "stdio.h" > setpaths.c
	echo #include "stddef.h" >> setpaths.c
	echo int chdir(char *path); >> setpaths.c
	echo char *getcwd(char *buf, size_t size); >> setpaths.c
	echo int main (int argc, char **argv) >> setpaths.c
	echo { >> setpaths.c
	echo char buffer[4096]; >> setpaths.c
	echo int position; >> setpaths.c
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
	.\setpaths >> version.h
	del setpaths.c
	del setpaths.exe

hi.obj: hi.c
	$(CC) $(CFLAGS) -c hi.c

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: version.h
	echo Working without dependency file

level.h:
	hi level

$(SEED7_LIB): $(SEED7_LIB_OBJ)
	lib /out:$(SEED7_LIB) $(SEED7_LIB_OBJ)

$(COMP_DATA_LIB): $(COMP_DATA_LIB_OBJ)
	lib /out:$(COMP_DATA_LIB) $(COMP_DATA_LIB_OBJ)

$(COMPILER_LIB): $(COMPILER_LIB_OBJ)
	lib /out:$(COMPILER_LIB) $(COMPILER_LIB_OBJ)

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
