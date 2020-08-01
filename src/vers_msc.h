#define ANSI_C
#define USE_WMAIN
#define USE_DIRWIN
#define PATH_DELIMITER '\\'
#define MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#define NO_EMPTY_STRUCTS
#define CATCH_SIGNALS
#define USE_ALTERNATE_UTIME
#define INCLUDE_SYS_UTIME
#define ISNAN_WITH_UNDERLINE
#define CHECK_INT_DIV_BY_ZERO
#define FLOAT_ZERO_DIV_ERROR
#define USE_MYUNISTD_H
#define OS_STRI_WCHAR
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
#define os_lstat _wstati64
#define os_stat _wstati64
#define os_stat_struct struct _stati64
#define os_chown(name,uid,gid)
#define os_chmod _wchmod
#define os_utime_orig _wutime
#define os_utime alternate_utime
#define os_utimbuf_struct struct _utimbuf
#define os_remove _wremove
#define os_rename _wrename
#define os_system _wsystem
#define os_pclose _pclose
#define os_popen _wpopen
#define os_fopen _wfopen
#define os_fseek _fseeki64
#define os_ftell _ftelli64
#define os_off_t __int64
#define os_getenv _wgetenv
#define os_putenv _wputenv
#define USE_WINSOCK
#define USE_BIG_RTL_LIBRARY
#define C_COMPILER_VERSION "Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 12.00.8168 for 80x86"
#define FTELL_WRONG_FOR_PIPE
#define REMOVE_FAILS_FOR_EMPTY_DIRS
#define POINTER_SIZE 32
#define FLOAT_SIZE 32
#define DOUBLE_SIZE 64
#define TIME_T_SIZE 32
#define INT32TYPE int
#define INT32TYPE_STRI "int"
#define UINT32TYPE unsigned int
#define UINT32TYPE_STRI "unsigned int"
#define INT64TYPE __int64
#define INT64TYPE_STRI "__int64"
#define UINT64TYPE unsigned __int64
#define UINT64TYPE_STRI "unsigned __int64"
#define INT64TYPE_FORMAT_I64
#define RSHIFT_DOES_SIGN_EXTEND
#define TWOS_COMPLEMENT_INTTYPE
#define LITTLE_ENDIAN_INTTYPE
#define OBJECT_FILE_EXTENSION ".obj"
#define LIBRARY_FILE_EXTENSION ".lib"
#define EXECUTABLE_FILE_EXTENSION ".exe"
#define C_COMPILER "cl"
#define GET_CC_VERSION_INFO "cl 2>"
#define CC_OPT_DEBUG_INFO "-Zi -Yd"
#define CC_OPT_NO_WARNINGS "-w"
#define CC_FLAGS "-Zm800"
#define REDIRECT_C_ERRORS "2>NUL: >"
#define LINKER_OPT_OUTPUT_FILE "-o "
#define LINKER_FLAGS "/F0x400000"
#define SYSTEM_LIBS "user32.lib ws2_32.lib"
#define SYSTEM_DRAW_LIBS "gdi32.lib"
#define SEED7_LIB "seed7_05.lib"
#define DRAW_LIB "s7_draw.lib"
#define COMP_DATA_LIB "s7_data.lib"
#define COMPILER_LIB "s7_comp.lib"
#define S7_LIB_DIR "/c/Dokumente und Einstellungen/tm/Eigene Dateien/seed7_5/bin"
#define SEED7_LIBRARY "/c/Dokumente und Einstellungen/tm/Eigene Dateien/seed7_5/prg"
