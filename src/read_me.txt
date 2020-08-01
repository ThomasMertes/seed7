COMPILING THE INTERPRETER
=========================

    The way to compile the interpreter is dependent on the
  operating system and the development tools used. You need a
  stand alone C compiler and a make utility to compile the
  interpreter. A C compiler which is only usable from an IDE
  is not so useful, since some Seed7 programs (e.g. The
  Seed7 to C compiler comp.sd7) need to call the C compiler
  as well.


THE MAKEFILES

    Several makefiles are prepared for various combinations
  of operating system, make utility, C compiler and shell:

  makefile name|operating system |make prog     |compiler    |shell
  -------------+-----------------+--------------+------------+--------
  mk_linux.mak | Linux/Unix/BSD  | (g)make      | gcc        | sh
  mk_cygw.mak  | Windows (Cygwin)| (g)make      | gcc        | sh
  mk_msys.mak  | Windows (MinGW) | (g)make      | gcc        | sh
  mk_mingw.mak | Windows (MinGW) | mingw32-make | gcc        | cmd.exe
  mk_nmake.mak | Windows (MinGW) | nmake        | gcc        | cmd.exe
  mk_msvc.mak  | Windows (MSVC)  | nmake        | cl         | cmd.exe
  mk_bcc32.mak | Windows (bcc32) | make         | bcc32      | cmd.exe
  mk_bccv5.mak | Windows (bcc32) | make         | bcc32 V5.5 | cmd.exe
  mk_osx.mak   | Mac OS X (Xcode)| (g)make      | gcc        | sh

  In the optimal case you just copy one of this files to
  'makefile' and do (with the make program from the table above):

    make depend
    make

  When the interpreter is compiled successfully the executable
  and the libraries are placed in the 'bin' directory.
  Additionally a symbolic link to the executable is placed in
  the 'prg' directory (Under Windows symbolic links are not
  supported, so a copy of the executable is placed in the 'prg'
  directory). If you do several compilation attempts in
  succession you need to do

    make clean

  before you start a new attempt.


COMPILING UNTER LINUX

    For Linux the compilation process is the simplest. The
  file 'makefile' is (almost) identical to 'mk_linux.mak' and
  that file is already prepared to compile under Linux.
  Go to the 'src' directory and type:

    make depend
    make

  In some distributions the X11 library directory is not in
  $PATH. In this case the value of the LIBS variable in the
  makefile must be changed. Replace the -lX11 by the path and
  the name of your libXll (e.g. /usr/X11/lib/libX11.so or
  /usr/X11R6/lib/libX11.a).


COMPILING UNDER BSD AND UNIX

    Probably you can do it just the same way as under Linux.
  If there are problems and the makefile needs to be adjusted
  please let me know. If you send the changed makefile to me
  I can include it in the release.


COMPILING UNDER WINDOWS WITH GCC FROM MINGW

    When gcc and mingw32-make from MinGW are installed use a
  console, go to the 'src' directory and type:

    copy mk_mingw.mak makefile
    mingw32-make depend
    mingw32-make

  To compile under MSYS (a Unix shell window from the MinGW
  tools) go to the 'src' directory and type:

    cp mk_msys.mak makefile
    gmake depend
    gmake

  When gmake uses Unix shell commands even when started from
  a Windows console, you can use gmake totether with
  mk_msys.mak from a Windows console also.

  To utilize nmake from Windows and gcc from MinGW use a
  console, go to the 'src' directory and type:

    copy mk_nmake.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter is copied to prg/hi.exe.


COMPILING UNDER WINDOWS WITH CL FROM MSVC

    To utilize compilation with nmake and cl (the C compiler of
  a big big software company) use a console, go to the 'src'
  directory and type:

    copy mk_msvc.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter is copied to prg/hi.exe.


COMPILING UNDER WINDOWS WITH BCC32

    To utilize compilation with make and bcc32 (the make utility
  and the C compiler of a smaller software company) use a
  console, go to the 'src' directory and type:

    copy mk_bcc32.mak makefile
    make depend
    make

  After the compilation the interpreter is copied to prg/hi.exe.
  For the older version of bcc32 (Version 5.5, which is available
  in the internet) use:

    copy mk_bccv5.mak makefile
    make depend
    make


COMPILING UNDER WINDOWS WITH CYGWIN

    To compile Seed7 with cygwin several cygwin packages need to
  be installed. From the category Devel the following packages are
  needed: binutils, gcc4, gcc4-core, libncurses-devel and make.
  From category X11 the package libX11-devel is needed. To test
  the installation start a cygwin window and do:

    make --version
    gcc-4 --version

  If the cygwin gcc does not have the name gcc-4 it is necessary
  to edit the file mk_cygw.mak . The line containing

    CC = gcc-4

  must be changed to define the name of your gcc. When gcc and
  make work well change to the seed7/src directory and do:

    make -f mk_cygw.mak depend
    make -f mk_cygw.mak

  Cygwin sometimes has problems with symlinks. This is indicated
  by an error message which ends with:

    bin/as.exe: cannot execute binary file

  The presence of a cygwin symlink problem can be checked in
  the directory /usr/i686-pc-cygwin/bin (or the corresponding
  directory mentioned in the error message). When the file
  /usr/i686-pc-cygwin/bin/as.exe is a small file with less than
  100 bytes and starts with

    !<symlink>

  it is some cygwin symlink. To fix this error just copy the
  symlink destination over the file as.exe (after making a
  backup of the original symlink file as.exe). Additionally
  it might also be necessary to do the same for ld.exe (and
  maybe for some other symlinks as well).


COMPILING UNTER MAC OS X

    To compile under Mac OS X make sure that Xcode is installed.
  The Xcode package contains a C compiler (gcc) and a 'make'
  utility. To compile Seed7 start a command shell, go to the
  'src' directory and type:

    cp mk_osx.mak makefile
    make depend
    make

  Under Mac OS X the X11 library is usually found in
  '/usr/X11R6/lib'. For unknown reasons the Xcode (Mac OS X)
  linker normally does not search libraries in '/usr/X11R6/lib'.
  Therefore 'mk_osx.mak' defines the following linker flag:

    LDFLAGS = -L/usr/X11R6/lib 

  Besides this 'mk_osx.mak' is almost identical to
  'mk_linux.mak' and 'makefile'. When the X11 library is in a
  different directory you need to change the LDFLAGS value to
  that directory.


WHAT TO DO WHEN ERRORS HAPPEN DURING THE COMPILATION?

    In most cases errors indicate that some development package
  of your distribution is missing. If your operating system is
  Linux, BSD or Unix not all development packages with header
  files might be installed. In this case you get some errors
  after typing 'make depend'.

 --- Errors such as

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

 --- Errors such as

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
  
 --- Errors such as

    echo char *getcwd(char *buf, size_t size); >> seed7lib.c
    /usr/bin/sh: -c: line 0: syntax error near unexpected token `('

  indicate that your makefile contains commands for the cmd.exe
  (or command.com) Windows console, but your 'make' program uses
  a Unix shell (/usr/bin/sh) to execute them. Either use a
  makefile which uses Unix shell commands (e.g. mk_msys.mak or
  mk_cygw.mak) or take care that the 'make' program uses cmd.exe
  (or command.com) to execute the commands.

 --- Errors such as

    hi.c:28:21: error: version.h: No such file or directory

  indicate that you forgot to run 'make depend' before running
  'make'. Since such an attempt produces several unneeded files it
  is necessary now to run 'make clean', 'make depend' and 'make'.

 --- The 'make' utility sometimes writes an error like

    depend:1: *** multiple target patterns.  Stop.

   This indicates that an old 'depend' file (probably a relict of
   an earlier 'make' command with a different makefile) does not
   fit to the current makefile. To get rid of this error remove
   the files 'depend', 'a_depend', 'b_depend' and 'c_depend'.
   After that the compilation process must be started from
   scratch with 'make clean', 'make depend' and 'make'.

 --- When using bcc32 an error like

    Error E2194: Could not find file 'Studio\7.0\bin\bcc32.exe'

  can happen. This indicates that the search path where bcc32.exe
  is found contains a space and the 'make' program is not capable
  to handle spaces in a search path. Creating the batch file
  'src\bcc32.bat' with the content

    bcc32.exe %*

  helps. As alternate solution 'bcc32.exe' and 'bcc32.cfg' can
  be copied to the 'src' directory (this solution should be
  avoided, since it does not consider updates of bcc32).

 --- A linker error like

    ld: library not found for -lX11

  indicates that the linker was not able to find the X11 library.
  The X11 library can have the name libX11.so (dynamic library)
  or libX11.a (static library). You need to search for this
  library (the dynamic library should normally be preferred).
  After you found it the LDFLAGS definition in your makefile must
  be changed. E.g.: If you found libX11.so in '/usr/X11R6/lib'
  the LDFLAGS assignment should be changed from

    LDFLAGS =

  to

    LDFLAGS = -L/usr/X11R6/lib 

 --- Other errors

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
   - Warnings about unused parameter 'arguments'. The primitive
     actions all use one parameter named 'arguments'. This is
     necessary to access primitive actions with function
     pointers. Some primitive actions do not use 'arguments'
     which causes this warning.
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


HOW TO VERIFY THAT THE INTERPRETER WORKS CORRECT?

    A comprehensive test of the 'hi' interpreter can be done in
  the 'prg' directory with the command:

    ./hi chk_all

  Under Windows using ./ might not work. Just omit the ./ and
  type:

    hi chk_all

  The 'chk_all' program uses several check programs to do its
  work. First a check program is interpreted and the output
  is compared to a reference. Then the program is compiled and
  executed and this output is also checked. Finally the C code
  generated by the compiled compiler is checked against the C
  code generated by the interpreted compiler. If everything
  works correct the output is (after the usual information from
  the interpreter):

    compiling the compiler - okay
    chkint - okay
    chkstr - okay
    chkprc - okay
    chkbig - okay
    chkbool - okay
    chkset - okay
    chkexc - okay

  This verifies that interpreter and compiler work correct.


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

    BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
    BIGINT_LIB = big_rtl
    # BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
    # BIGINT_LIB = big_gmp

  This four lines must be changed to

    # BIGINT_LIB_DEFINE = USE_BIG_RTL_LIBRARY
    # BIGINT_LIB = big_rtl
    BIGINT_LIB_DEFINE = USE_BIG_GMP_LIBRARY
    BIGINT_LIB = big_gmp

  After the changes in the makefile it is necessary to start the
  compilation process from scratch with (use the corresponding
  make command (gmake, nmake, ...) for your make tool):

    make clean
    make depend
    make


SOURCE FILES
============

    The C code of Seed7 can be grouped into several categorys:
  Interpreter main, Parser, Interpreter core, Primitive action functions,
  General helper functions, Runtime library and Compiler data library.


INTERPRETER MAIN

    The main() function of the interpreter calls the Parser to
  create an internal program representation and afterwards it calls
  the Interpreter core to execute this program.

    hi.c       Main program of the hi Interpreter.

  The interpreter main is licensed under the GPL.


PARSER

    The Seed7 parser (analyzer) reads Seed7 programs from a
  file or string and creates an internal representation for them.

    analyze.c  Main procedure of the analyzing phase.
    syntax.c   Generate new syntax descriptions out of expressions.
    token.c    Procedures to maintain objects of type tokentype.
    parser.c   Main procedures of the parser.
    name.c     Enter an object in a specified declaration level.
    type.c     Parse and assign a type of an object.
    expr.c     Read the next expression from the source file.
    atom.c     Read the next object from the source file.
    object.c   Procedures to maintain objects and lists.
    scanner.c  Read the next symbol from the source file.
    literal.c  Read next char or string literal from the source file.
    numlit.c   Read the next number from the source file.
    findid.c   Procedures to maintain the identifier table.
    error.c    Submit normal compile time error messages.
    infile.c   Procedures to open, close and read the source file.
    symbol.c   Maintains the current symbol of the scanner.
    info.c     Procedures for compile time info.
    stat.c     Procedures for maintaining an analyze phase statistic.
    fatal.c    Submit fatal compile time error messages.
    match.c    Procedures to do static match on expressions.
    act_comp.c Primitive actions for the action type.
    prg_comp.c Primitive actions for the program type.

  The Seed7 parser is licensed under the GPL.


INTERPRETER CORE

    The interpreter core executes the internal representation
  of a program. In doing so primitive action functions are called.

    exec.c     Main interpreter procedures.
    doany.c    Procedures to call several Seed7 functions from C.
    objutl.c   isit_.. and bld_.. functions for primitive datatypes.

  The interpreter core is licensed under the GPL.


PRIMITIVE ACTION FUNCTIONS

    The *lib.c files contain interpreter functions which execute
  "PRIMITIVE ACTIONS" (in doing so they can call functions from
  the runtime library. The PRIMITIVE ACTION functions are called
  from the interpreter core.

    actlib.c   ACTION (ACT_*) actions
    arrlib.c   array (ARR_*) actions
    biglib.c   bigInteger (BIG_*) actions
    blnlib.c   boolean (BLN_*) actions
    bstlib.c   byte string (BST_*) actions
    chrlib.c   char (CHR_*) actions
    cmdlib.c   Directory, file and system command (CMD_*) actions
    dcllib.c   Declaration (DCL_*) actions
    drwlib.c   Drawing (DRW_*) actions
    enulib.c   Enumeration (ENU_*) actions
    fillib.c   PRIMITIVE_FILE (FIL_*) actions
    fltlib.c   float (FLT_*) actions
    hshlib.c   hash (HSH_*) actions
    intlib.c   integer (INT_*) actions
    itflib.c   interface (ITF_*) actions
    kbdlib.c   Keyboard (KBD_*) actions
    lstlib.c   List (LST_*) actions
    prclib.c   proc/statement (PRC_*) actions
    prglib.c   Program (PRG_*) actions
    reflib.c   reference (REF_*) actions
    rfllib.c   ref_list (RFL_*) actions
    scrlib.c   Screen (SCR_*) actions
    sctlib.c   struct (SCT_*) actions
    setlib.c   set (SET_*) actions
    soclib.c   PRIMITIVE_SOCKET (SOC_*) actions
    strlib.c   string (STR_*) actions
    timlib.c   time and duration (TIM_*) actions
    typlib.c   type (TYP_*) actions
    ut8lib.c   utf8_file (UT8_*) actions

  The primitive action functions are licensed under the GPL.


GENERAL HELPER FUNCTIONS

    General helper functions are used by Parser, Interpreter
  core and Primitive action functions.

    runerr.c   Runtime error and exception handling procedures.
    option.c   Reads and interprets the command line options.
    primitiv.c Table definitions for all primitive actions.
    syvarutl.c Maintains the interpreter system variables.
    traceutl.c Tracing and protocol procedures.
    actutl.c   Conversion of strings to ACTIONs and back.
    executl.c  Initalisation operation procedures used at runtime.
    blockutl.c Procedures to maintain objects of type blocktype.
    entutl.c   Procedures to maintain objects of type entitytype.
    identutl.c Procedures to maintain objects of type identtype.
    chclsutl.c Compute the type of a character very quickly.
    sigutl.c   Driver shutdown and signal handling.

  The general helper functions are licensed under the GPL.


RUNTIME LIBRARY

    The functions defined in the runtime library are called from
  interpreted and compiled code (The Seed7 runtime library is
  linked to every compiled Seed7 program).

    arr_rtl.c  array library
    big_rtl.c  bigInteger library
    bln_rtl.c  boolean library
    bst_rtl.c  byte string library
    chr_rtl.c  char library
    cmd_rtl.c  Various directory, file and other commands
    dir_rtl.c  Directory library
    drw_rtl.c  Drawing library
    fil_rtl.c  File library
    flt_rtl.c  float library
    hsh_rtl.c  hash library
    int_rtl.c  integer library
    kbd_rtl.c  Keyboard library
    scr_rtl.c  Screen (text console/terminal) library
    set_rtl.c  set library
    soc_rtl.c  Socket library
    str_rtl.c  string library
    tim_rtl.c  time library
    ut8_rtl.c  utf8_file library
    heaputl.c  heap utility library
    striutl.c  string utility library
    big_gmp.c  Alternate bigInteger library based on GMP
    cmd_unx.c  Unix functions for commands
    cmd_win.c  Windows functions for commands
    dir_dos.c  Dos directory access functions
    dir_win.c  Windows directory access functions
    scr_inf.c  Terminfo screen (text console/terminal) driver
    scr_win.c  Windows screen (text console/terminal) driver
    kbd_inf.c  Terminfo keybord driver
    kbd_poll.c Terminfo keyboard driver using poll()
    trm_inf.c  Terminfo support
    tim_unx.c  Unix time driver
    tim_win.c  Windows time driver
    drw_dos.c  Dos drawing functions (dummy functions)
    drw_win.c  Windows drawing functions
    drw_x11.c  X11 drawing functions

  The runtime library is licensed under the LGPL.


COMPILER DATA LIBRARY

    The functions in the compiler data library manipulate
  compiler and interpreter internal data such as "type",
  "ref_list" and "reference". The compiler data library is
  linked to the interpreter and to some compiled Seed7
  programs such as the compiler (comp.sd7) itself.

    typ_data.c Primitive actions for the type type.
    rfl_data.c Primitive actions for the ref_list type.
    ref_data.c Primitive actions for the reference type.
    listutl.c  Procedures to maintain objects of type listtype.
    flistutl.c Procedures for free memory list maintainance.
    typeutl.c  Procedures to maintain objects of type typetype.
    datautl.c  Procedures to maintain objects of type identtype.

  The compiler data library is licensed under the GPL.


THE VERSION.H FILE

    One of the main jobs of the makefile is creating a version.h
  file. This is done with 'make depend' (or 'gmake depend' or
  'nmake depend'). The version.h file contains several #defines
  which contain information over available features and the way
  they are available. I try to explain some of them here:

  ANSI_C: Defined when the ansi C prototypes are used. If
          it is not defined K&R C function headers are used.
          It seems to be antiquated to still support K&R C,
          but I really saw Unix systems where the K&R C
          compiler was available for free and the ansi C
          compiler was a chargeable option (and in most
          situations you don't have the possibility to force
          your boss to buy a C compiler just to compile some
          open source program).

  USE_WMAIN: Defined when the main function is named wmain.
             This is a way to support Unicode command line
             arguments under Windows. An alternate way to
             support Unicode command line arguments under
             Windows uses the functions getUtf16Argv and
             freeUtf16Argv (both defined in "cmd_win.c").

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
              on FindFirstFileA() and FindNextFileA(). Only one
              #define of USE_DIRxxx is allowed. Additionally the
              file dir_win.c contains also definitions of the
              wopendir(), wreaddir() and wclosedir() based
              on FindFirstFileW() and FindNextFileW().

  USE_DIRDOS: The opendir(), readdir() and closedir() functions
              from dir_dos.c are used. This functions are based
              on _dos_findfirst() and _dos_findnext(). Only one
              #define of USE_DIRxxx is allowed.

  AWAIT_WITH_POLL: The function timAwait() uses the function
                   poll() to implement waiting for a time.
                   Only one #define of AWAIT_WITH_xxx is
                   allowed.

  AWAIT_WITH_PPOLL: The function timAwait() uses the function
                    ppoll() to implement waiting for a time.
                    Only one #define of AWAIT_WITH_xxx is
                    allowed.

  AWAIT_WITH_SIGACTION: The function timAwait() uses the
                        functions sigaction(), sigsetjmp(),
                        setitimer(), pause(), siglongjmp() and 
                        a signal handler function to catch
                        a SIGALRM signal. Only one #define of
                        AWAIT_WITH_xxx is allowed.

  AWAIT_WITH_SIGNAL: The function timAwait() uses the
                        functions signal(), setjmp(),
                        setitimer(), pause(), longjmp() and 
                        a signal handler function to catch
                        a SIGALRM signal. Only one #define of
                        AWAIT_WITH_xxx is allowed.

  OS_PATH_WCHAR: Defined when the system calls (os_...) use
                 wide characters (type wchar_t) for the
                 parameters describing a path. In this case
                 functions like _wgetcwd(), wreaddir() and
                 _wstati64() together with types like 'WDIR',
                 'wdirent' and 'struct _stati64' must be used.
                 It is therefore necessary to define the os_...
                 macros accordingly.

  OS_PATH_UTF8: Defined when the system calls (os_...) use
                UTF-8 characters (type char) for the parameters
                describing a path. In this case functions like
                getcwd(), readdir() and stat() together  with
                types like 'DIR', 'dirent' and 'struct stat'
                must be used. When this functions and types do
                not use the POSIX/SUS names it is necessary to
                define the os_... macros accordingly.

  os_chdir: Function to be used instead of chdir() under the
            target operating system. If not defined chdir()
            is used.

  os_getcwd: Function to be used instead of getcwd() under the
             target operating system. If not defined getcwd()
             is used.

  os_mkdir(path,mode): Function to be used instead of mkdir()
                       under the target operating system.
                       If not defined mkdir() is used. Under
                       Windows the mkdir() function usually
                       has only one parameter while under
                       Unix/Linux/BSD mkdir() has two
                       parameters.

  os_rmdir: Function to be used instead of rmdir() under the
            target operating system. If not defined rmdir()
            is used.

  os_opendir: Function to be used instead of opendir() under the
              target operating system. If not defined opendir()
              is used.

  os_readdir: Function to be used instead of readdir() under the
              target operating system. If not defined readdir()
              is used.

  os_closedir: Function to be used instead of closedir() under
               the target operating system. If not defined
               closedir() is used.

  os_DIR: Type to be used instead of 'DIR' under the target
          operating system. If not defined 'DIR' is used.

  os_dirent_struct: Type to be used instead of 'struct dirent'
                    under the target operating system. If not
                    defined 'struct dirent' is used.

  os_fstat: Function to be used instead of fstat() under the
            target operating system. If not defined fstat()
            is used.

  os_lstat: Function to be used instead of lstat() under the
            target operating system. If not defined lstat()
            is used.

  os_stat: Function to be used instead of stat() under the
           target operating system. If not defined stat()
           is used.

  os_stat_struct: Type to be used instead of 'struct stat'
                  under the target operating system. If not
                  defined 'struct stat' is used.

  os_chown(name,uid,gid): Function to be used instead of chown()
                          under the target operating system.
                          If not defined chown() is used. When
                          chown() is not supported this macro is
                          defined empty.

  os_chmod: Function to be used instead of chmod() under the
            target operating system. If not defined chmod() is
            used.

  os_utime: Function to be used instead of utime() under the
            target operating system. If not defined utime() is
            used.

  os_utimbuf_struct: Type to be used instead of
                     'struct utimbuf' under the target operating
                     system. If not defined 'struct utimbuf' is
                     used.

  os_remove: Function to be used instead of remove() under the
            target operating system. If not defined remove() is
            used.

  os_rename: Function to be used instead of rename() under the
            target operating system. If not defined rename() is
            used.

  USE_WFOPEN: Use the function _wfopen() to open files with
              wide (unicode) characters instead of fopen() with
              UTF-8 encoded characters.

  USE_WINSOCK: Use thw winsocket functions instead of the
               normal Unix socket functions.

  PATH_DELIMITER: This is the path delimiter character used by
                  the command shell of the operating system. It
                  is defined as '/' for most operating systems,
                  except for Windows, where it is defined as
                  '\\'. The PATH_DELIMITER macro is used when
                  the functions popen() and system() are called.
                  Seed7 programs are portable and do not need to
                  distinguish between different path delimiter
                  characters. Instead Seed7 programs must always
                  use '/' as path delimiter.

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

  TWOS_COMPLEMENT_INTTYPE: Defined when signed integers are
                           represented as twos complement
                           numbers. This allows some simplified
                           range checks in compiled programs.
                           This macro is defined when
                           ~(-1) == 0 holds.

  TURN_OFF_FP_EXCEPTIONS: In Seed7 floating point errors such
                          as the division by zero should create
                          values like Infinite and NaN which
                          are defined in IEEE 754. Some C
                          compilers/libraries raise exceptions
                          for floating point errors. This macro
                          is used to turn off such a behaviour.
                          
  DEFINE_MATHERR_FUNCTION: Some C compilers/libraries call the
                           _matherr() function for every floating
                           point error and terminate the program
                           when the function is not present.
                           To get the Seed7 behaviour of using
                           the IEEE 754 values of Infinite and
                           NaN this function must be defined and
                           it must return 1.

  CHECK_INT_DIV_BY_ZERO: Instruct the Seed7 to C compiler to
                         generate C code which checks all integer
                         divisions (div, rem, mdiv and mod) for
                         division by zero. The generated C code
                         should, when executed, raise the
                         exception MEMORY_ERROR instead of doing
                         the illegal divide operation.

  OBJECT_FILE_EXTENSION: The extension used by the C compiler for
                         object files (Several object files and
                         libraries are linked together to an
                         executable). Under Linux/Unix/BSD this
                         is usually ".o" Under Windows this is
                         ".o" for MinGW and cygwin, but ".obj"
                         for MSVC and bcc32.

  EXECUTABLE_FILE_EXTENSION: The extension which is used by the
                             operating systemfor executables.
                             Since executable extensions are not
                             used under Linux/Unix/BSD it is ""
                             for them. Under Windows the value
                             ".exe" is used.

  C_COMPILER: Contains the command to call the stand-alone C
              compiler and linker (Most IDEs provide also a
              stand-alone compiler/linker).

  C_COMPILER_VERSION: Contains a string describing the version of
                      the C compiler which compiled the Seed7
                      runtime libraries. This string can be used
                      together with GET_CC_VERSION_INFO to compare
                      the C compiler used to compile the Seed7
                      runtime libraries with the actual version of
                      the C compiler.

  GET_CC_VERSION_INFO: Contains a shell command that causes the
                       C compiler to write its version information
                       into a given file. In Seed7 it is used with
                       cmd_sh(GET_CC_VERSION_INFO & fileName);
                       Afterwards the file fileName contains the
                       version information of the C compiler.
                       Reading the first line of the file should
                       give the same string as C_COMPILER_VERSION.

  CC_OPT_DEBUG_INFO: Contains the C compiler option to add source
                     level debugging information to the object file.

  CC_OPT_NO_WARNINGS: Contains the C compiler option to suppress
                      all warnings.

  CC_FLAGS: Contains C compiler flags which should be used when
            C programs are compiled.

  REDIRECT_C_ERRORS: The redirect command to redirect the errors
                     of the C compiler to a file. The MSVC
                     stand-alone C compiler (CL) writes the error
                     messages to standard output (use: "2>NUL: >"),
                     while the Unix C compliers including MinGW and
                     cygwin write the error messages to the error
                     output (use "2>").

  LINKER_OPT_DEBUG_INFO: Contains the linker option to add source
                         level debugging information to the
                         executable file. Many compiler/linker
                         combinations don't need this option
                         to do source level debugging.

  LINKER_OPT_OUTPUT_FILE: Contains the linker option to provide the
                          output filename (e.g.: "-o "). When no
                          such option exists the definition of
                          LINKER_OPT_OUTPUT_FILE should be ommited.

  LINKER_FLAGS: Contains options for the stand-alone linker to link
                a compiled Seed7 program.

  SYSTEM_LIBS: Contains system libraries for the stand-alone linker
               to link a compiled Seed7 program.

  SEED7_LIB: Contains the path and the name of the Seed7 runtime
             library.

  COMP_DATA_LIB, COMPILER_LIB: Contain path and name of other Seed7
                               runtime libraries.

  SEED7_LIBRARY: Contains the path where the Seed7 include files
                 are available.

  INT32TYPE: A signed integer type that is 32 bits wide.

  UINT32TYPE: An unsigned integer type that is 32 bits wide.

  INT64TYPE: A signed integer type that is 64 bits wide.

  UINT64TYPE: An unsigned integer type that is 64 bits wide.
