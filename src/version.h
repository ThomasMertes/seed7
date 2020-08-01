#define ANSI_C
#define USE_DIRENT
#define PATH_DELIMITER '/'
#define CATCH_SIGNALS
#define USE_MMAP
#undef  INCL_NCURSES_TERM
#undef  INCL_CURSES_BEFORE_TERM
#undef  MKDIR_WITH_ONE_PARAMETER
#undef  CHOWN_MISSING
#undef  CHMOD_MISSING
#define _FILE_OFFSET_BITS 64
#define LINKER_LIBS "-lX11 -lncurses -lm"
#define SEED7_LIBRARY "/home/tm/seed7_5/lib"
