COMPILING THE INTERPRETER
=========================

    The way to compile the interpreter is dependend on the
  operating system and the development tools used. You need a
  stand alone C compiler and a make utility to compile the
  interpreter. A C compiler which is only usable from an IDE
  is not so useful, since some Seed7 programs (e.g. The
  Seed7 to C compiler comp.sd7) needs to call the C compiler
  as well.


THE MAKEFILES

    Several makefiles are prepared vor various combinations
  of operating system, make utility, C compiler and shell:

  makefile name|operating system |make prog|compiler|shell
  -------------+-----------------+---------+--------+--------
  mk_cygw.mak  | windows (Cygwin)| (g)make | gcc    | sh
  mk_linux.mak | linux/unix/bsd  | (g)make | gcc    | sh
  mk_mingw.mak | windows (MinGW) | (g)make | gcc    | cmd.exe
  mk_msvc.mak  | windows (MinGW) | nmake   | cl     | cmd.exe
  mk_msys.mak  | windows (MSVC)  | (g)make | gcc    | sh
  mk_nmake.mak | windows (MinGW) | nmake   | gcc    | cmd.exe

  In the optimal case you just copy one of this files to
  'makefile' and do:

    make depend
    make

  If you try different makefiles in succession you need to do

    make clear

  before you start a new attempt.


COMPILING UNTER LINUX

    For linux the compilation process is the simplest. The
  file 'makefile' is (almost) identical to 'mk_linux.mak' and
  that file is already prepared to compile under linux.
  Just type:

    make depend
    make

  In some distributions the X11 library directory is not in
  $PATH. In this case the value of the LIBS variable in the
  makefile must be changed. Replace the -lX11 by the path and
  the name of your libXll (e.g. /usr/X11/lib/libX11.so or
  /usr/X11R6/lib/libX11.a).


COMPILING UNDER BSD AND UNIX

    Probably you can do it just the same way as under linux.
  If there are problems and the makefile needs to be adjusted
  please let me know. If you send the changed makefile to me
  I can include it in the release.


COMPILING UNDER WINDOWS

    You need gcc and gmake from MinGW. Use a console and
  change the PATH variable to contain the MinGW tools. Then
  go to the 'src' directory and type:

    copy mk_mingw.mak makefile
    gmake depend
    gmake

  To compile under MSYS (a unix shell window from the MinGW
  tools) go to the 'src' directory and type:

    cp mk_msys.mak makefile
    gmake depend
    gmake

  Sometimes the gmake utility uses unix shell commands even
  when started from a windows console. In this case use a
  windows console, go to the 'src' directory and type:

    copy mk_msys.mak makefile
    gmake depend
    gmake

  To utilize nmake from windows and gcc from MinGW use a
  console, go to the 'src' directory and type:

    copy mk_nmake.mak makefile
    nmake depend
    nmake

  To utilize the experimental compilation with nmake and cl
  (the command line compiler of a big big software company)
  use a console, go to the 'src' directory and type:

    copy mk_msvc.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter is copied to prg/hi.exe.


WHAT TO DO WHEN ERRORS HAPPEN DURING THE COMPILATION?

    In most cases errors indicate that some development package
  of your distribution is missing. If your operating system is
  linux, bsd or unix not all development packages with header
  files might be installed. In this case you get some errors
  after typing 'make depend'. Errors such as

    scr_inf.c:57:18: error: term.h: No such file or directory
    kbd_inf.c:55:18: error: term.h: No such file or directory
    trm_inf.c:47:18: error: term.h: No such file or directory

  indicate that the curses or ncurses development package is
  missing. I don't know the name of this package in your
  distribution (under Ubuntu it has the name libncurses5-dev),
  but you can search in your package manager for a curses/ncurses
  package which mentions that it contains the header files.
  To execute programs you need also to install the non-developer
  package of curses/ncurses (in most cases it will already
  be installed because it is needed by other packages).
  Errors such as

    drw_x11.c:38:19: error: X11/X.h: No such file or directory
    drw_x11.c:39:22: error: X11/Xlib.h: No such file or directory
    drw_x11.c:40:23: error: X11/Xutil.h: No such file or directory
    drw_x11.c:45:24: error: X11/keysym.h: No such file or directory

  indicate that the X11 development package is missing.
  Under Ubuntu this package has the name libx11-dev and is
  described as: X11 client-side library (development headers)
  Note that under X11 'client' means: The program which wants
  to draw. A X11 'server' is the place where the drawings are
  displayed. So you have to search for a X11 client developer
  package with headers. If you use X11 in some way (you don't
  do everything from the text console) the non-developer package
  of X11 will already be installed.
  Errors such as

    echo char *getcwd(char *buf, size_t size); >> seed7lib.c
    /usr/bin/sh: -c: line 0: syntax error near unexpected token `('

  indicate that your makefile contains commands for the cmd.exe
  (or command.com) windows console, but your 'make' program uses
  a unix shell (/usr/bin/sh) to execute them. Either use a
  makefile which uses unix shell commands (e.g. mk_msys.mak or
  mk_cygw.mak) or take care that the 'make' program uses cmd.exe
  (or command.com) to execute the commands.

  When you got other errors I would like to know about. Please
  send a mail with detailed information (name and version) of
  your operating system, distribution, compiler, the version of
  Seed7 you wanted to compile and the complete log of error
  messages to seed7-users@lists.sourceforge.net .


THE VERSION.H FILE

    One of the main jobs of the makefile is creating a version.h
  file. This is done with 'make depend' (or 'gmake depend' or
  'nmake depend'). The version.h file contains several #defines
  which contain information over available features and the way
  they are available. I try to explain some of them here:

  ANSI_C: Defined when the ansi C prototypes are used. If
          it is not defined K&R C function headers are used.
          It seems to be antiquated to still support K&R C,
          but I really saw UNIX systems where the K&R C
          compiler was available for free and the ansi C
          compiler was a chargeable option (and in most
          situations you don't have the possibility to force
          your boss to buy a C compiler just to compile some
          open source program).

  USE_DIRENT: The header file containing the definitions for
              opendir(), readdir() and closedir() has the name
              <dirent.h>. Only one #define of USE_DIRxxx is
              allowed.

  USE_DIRECT: The header file containing the definitions for
              opendir(), readdir() and closedir() has the name
              <direct.h>. Only one #define of USE_DIRxxx is
              allowed.

  USE_DIRWIN: The opendir(), readdir() and closedir() functions
              from dir_win.c are used. This functions are based
              on _findfirst() and _findnext(). Only one #define
              of USE_DIRxxx is allowed.

  USE_DIRDOS: The opendir(), readdir() and closedir() functions
              from dir_dos.c are used. This functions are based
              on _dos_findfirst() and _dos_findnext(). Only one
              #define of USE_DIRxxx is allowed.

  PATH_DELIMITER: This is '/' for most operating systems except
                  for windows where it is '\\'.

  MKDIR_WITH_ONE_PARAMETER: Under windows the mkdir() function
                            usually has only one parameter while
                            under unix/linux/bsd mkdir() has two
                            parameters.

  OBJECT_FILE_EXTENSION: The extension used by the C compiler for
                         object files (later several object files
                         and libraries are linked together to an
                         executable). Under linux/unix/bsd this
                         is usually ".o" Uunder windows this is
                         ".o" for MinGW and cygwin. Under MSVC it
                         is ".obj".

  EXECUTABLE_FILE_EXTENSION: The extension which is used by the
                             operating systemfor executables.
                             Since executable extensions are not
                             used under linux/unix/bsd it is ""
                             for them. Under windows the value
                             ".exe" is used.

  C_COMPILER: Contains the command to call the stand-alone C
              compiler can be called (Most IDEs also provide also
              a stand-alone compiler).

  REDIRECT_C_ERRORS The redirect command to redirect the errors
                    of the compiler to a file. The MSVC
                    stand-alone C compiler (CL) writes the error
                    messages to standard output (use: "2>NUL: >"),
                    while the unix C compliers including MinGW and
                    cygwin write the error messages to the error
                    output (use "2>").

  SYSTEM_LIBS: Contains options that the stand-alone
               compiler/linker needs to link the system libraries.

  SEED7_LIB: Contains the path and the name of the seed7 runtime
             library.

  COMP_DATA_LIB, COMPILER_LIB: Contain path and name of other seed7
                               runtime libraries.

  SEED7_LIBRARY: Contains the path where the Seed7 include files
                 are available.
