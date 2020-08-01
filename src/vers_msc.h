#define ANSI_C
#define USE_DIRWIN
#define PATH_DELIMITER '/'
#define CATCH_SIGNALS
#undef  USE_MMAP
#undef  INCL_NCURSES_TERM
#undef  INCL_CURSES_BEFORE_TERM
#define MKDIR_WITH_ONE_PARAMETER
#define CHOWN_MISSING
#undef  CHMOD_MISSING
#define DEFINE_IEEE_754_STUFF
#define FLOAT_ZERO_DIV_ERROR
#define USE_MYUNISTD_H
#undef  USE_FSEEKO64
#define USE_WINSOCK
#define popen _popen
#define OBJECT_FILE_EXTENSION "obj"
#define C_COMPILER "cl"
#define LINKER_LIBS "seed7_05.lib user32.lib gdi32.lib"
