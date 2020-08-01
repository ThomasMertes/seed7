#define ANSI_C
#define USE_DIRWIN
#define PATH_DELIMITER '\\'
#define NO_EMPTY_STRUCTS
#define CATCH_SIGNALS
#undef USE_MMAP
#undef INCL_NCURSES_TERM
#undef INCL_CURSES_BEFORE_TERM
#define ISNAN_WITH_UNDERLINE
#define CHECK_INT_DIV_BY_ZERO
#define FLOAT_ZERO_DIV_ERROR
#define USE_MYUNISTD_H
#define INT64TYPE __int64
#define UINT64TYPE unsigned __int64
#define OS_PATH_WCHAR
#define os_chdir _wchdir
#define os_getcwd _wgetcwd
#define os_mkdir(path,mode) _wmkdir(path)
#define os_rmdir _wrmdir
#define os_opendir wopendir
#define os_readdir wreaddir
#define os_closedir wclosedir
#define os_DIR WDIR
#define os_dirent_struct struct wdirent
#define os_fstat _fstati64
#define os_stat _wstati64
#define os_stat_struct struct _stati64
#define os_chown(name,uid,gid)
#define os_chmod _wchmod
#define os_utime _wutime
#define os_utimbuf_struct struct _utimbuf
#define os_remove _wremove
#define os_rename _wrename
#define wide_fopen _wfopen
#define USE_FSEEKI64
#define USE_WINSOCK
#define popen _popen
#define USE_BIG_RTL_LIBRARY
#define FTELL_WRONG_FOR_PIPE
#define RSHIFT_DOES_SIGN_EXTEND
#define TWOS_COMPLEMENT_INTTYPE
#define OBJECT_FILE_EXTENSION ".obj"
#define EXECUTABLE_FILE_EXTENSION ".exe"
#define C_COMPILER "cl"
#define INHIBIT_C_WARNINGS "-w"
#define REDIRECT_C_ERRORS "2>NUL: >"
#define LINKER_FLAGS "/F0x400000"
#define SYSTEM_LIBS "user32.lib gdi32.lib ws2_32.lib"
#define SEED7_LIB "C:/Dokumente und Einstellungen/tm/Eigene Dateien/seed7/src/seed7_05.lib"
#define COMP_DATA_LIB "C:/Dokumente und Einstellungen/tm/Eigene Dateien/seed7/src/s7_data.lib"
#define COMPILER_LIB "C:/Dokumente und Einstellungen/tm/Eigene Dateien/seed7/src/s7_comp.lib"
#define SEED7_LIBRARY "C:/Dokumente und Einstellungen/tm/Eigene Dateien/seed7/lib"
