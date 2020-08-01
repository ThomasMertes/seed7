COMPILING THE INTERPRETER
=========================

    The way to compile the interpreter is dependend on the
  operating system and the development tools used. You need a
  stand alone C compiler and a make utility to compile the
  interpreter. A C compiler which is only usable from an IDE
  is not so useful, since some Seed7 programs (e.g. The
  Seed7 to C compiler comp.sd7) need to call the C compiler
  as well.


THE MAKEFILES

    Several makefiles are prepared vor various combinations
  of operating system, make utility, C compiler and shell:

  makefile name|operating system |make prog|compiler|shell
  -------------+-----------------+---------+--------+--------
  mk_linux.mak | linux/unix/bsd  | (g)make | gcc    | sh
  mk_cygw.mak  | windows (Cygwin)| (g)make | gcc    | sh
  mk_msys.mak  | windows (MinGW) | (g)make | gcc    | sh
  mk_mingw.mak | windows (MinGW) | (g)make | gcc    | cmd.exe
  mk_nmake.mak | windows (MinGW) | nmake   | gcc    | cmd.exe
  mk_msvc.mak  | windows (MSVC)  | nmake   | cl     | cmd.exe
  mk_bcc32.mak | windows (bcc32) | make    | bcc32  | cmd.exe

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


COMPILING UNDER WINDOWS WITH GCC

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

  After the compilation the interpreter is copied to prg/hi.exe.


COMPILING UNDER WINDOWS WITH CL FROM MSVC

    To utilize compilation with nmake and cl (the command line
  compiler of a big big software company) use a console, go
  to the 'src' directory and type:

    copy mk_msvc.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter is copied to prg/hi.exe.


COMPILING UNDER WINDOWS WITH BCC32

    To utilize compilation with make and bcc32 (the make utility
  and the command line compiler of a smaller software company)
  use a console, go to the 'src' directory and type:

    copy mk_bcc32.mak makefile
    make depend
    make

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


WHAT ABOUT THE WARNINGS THAT HAPPEN DURING THE COMPILATION?

    The warnings can usually be ignored. Seed7 is compiled with
  the higest warning level (-Wall). Additionally there are also
  some warnings requested (such as -Wstrict-prototypes) which are
  not part of -Wall. The warnings can be classified to the
  following cases:

   - Warnings about float used instead of double because of the
     prototype: There are functions which use float parameters or
     return float values. Gcc has the opinion that only double
     parameters and double results should be used and warns about
     that.
   - Warnings about signed/unsigned instead of unsigned/signed
     because of the prototype.
   - Warnings about 'variablename' may be used uninitialized:
     This are false complaints. Interestingly gcc is not able to
     recognize when the states of two variables are connected.
     Such as a global fail_flag variable and a local condition
     variable (cond). The connection is: As long as fail_flag is 
     FALSE the cond variable is initialised. When the fail_flag
     is TRUE the cond variable is not used and therefore it could
     be in an uninitialized state. At several places I use such
     connected variable states which are not recognized by the
     gcc optimizer and are therefore flagged with a warning. I
     accept such warnings in performance critical paths. I am not
     willing to do "unnecessary" initialisations in performance
     critical paths of the program. At places that are not
     performance critical I do some of this "unnecessary"
     initialisations just to avoid such warnings.


HOW TO USE THE GMP LIBRARY?

    The functions to operate with bigInteger values are defined
  in the file 'big_rtl.c'. This functions provide reasonable
  performance for the usual bigInteger computations. If for some
  reason the bigInteger performance of 'big_rtl.c' is not enough
  it is possible to replace 'big_rtl.c' with 'big_gmp.c'.
  The file 'big_gmp.c' uses the GMP library to do the bigInteger
  computations. To use the GMP library use the following steps:

  You need the GMP library (one of gmp.lib/gmp.dll/gmp.a/gmp.so)
  and the gmp.h include file.

  Every makefile contains a line which defines the C 'LIBS' to
  be used when the 'hi' interpreter is linked. E.g.:

    LIBS = -lX11 -lncurses -lm

  The next line starts with # (which means it is commented out)
  and additionally contains the command to add the gmp library:

    # LIBS = -lX11 -lncurses -lm -lgmp

  The old 'LIBS' line needs to be commented out and the line
  which links also 'gmp' needs to be activated:

    # LIBS = -lX11 -lncurses -lm
    LIBS = -lX11 -lncurses -lm -lgmp

  There are also four lines which define which files contain
  the interface functions for bigInteger:

    USE_BIG_RTL_LIBRARY = define
    BIGINT_LIB = big_rtl
    # USE_BIG_RTL_LIBRARY = undef
    # BIGINT_LIB = big_gmp

  This four lines must be changed to

    # USE_BIG_RTL_LIBRARY = define
    # BIGINT_LIB = big_rtl
    USE_BIG_RTL_LIBRARY = undef
    BIGINT_LIB = big_gmp

  After the makefile changes it is necessary to start the
  compilation process from scratch with (use the corresponding
  make command (gmake, nmake, ...) for your make tool):

    make clear
    make depend
    make


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

  ESCAPE_SPACES_IN_COMMANDS: Depending on the shell/os the C
                             functions system() and popen() need
                             to get processed shell commands.
                             When the macro is defined, a
                             backslash (\) is added before every
                             space. When the macro is not
                             defined (undef), the whole shell
                             command is surrounded by double
                             quotes (") when it contains a space.

  USE_BIG_RTL_LIBRARY: Defined when the big_rtl library is used
                       to implement the bigInteger functions.
                       Not defined (undef) when the big_gmp
                       library is used to implement the
                       bigInteger functions.

  FTELL_WRONG_FOR_PIPE: The ftell() function should return -1
                        when it is called with a pipe (since a
                        pipe is not seekable). This macro is
                        defined when ftell() does not return
                        -1 for pipes. In this case the function
                        improved_ftell is used which returns -1
                        when the check with fstat() does not
                        verify that the parameter is a regular
                        file.

  RSHIFT_DOES_SIGN_EXTEND: The C standard specifies that the
                           right shift of signed integers is
                           implementation defined, when the
                           shifted values are negative. This
                           macro is set when the sign of negative
                           signed integers is preserved with a
                           right shift ( -1 >> 1 == -1 ).

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
