COMPILING THE INTERPRETER
=========================

    The way to compile the interpreter depends on the operating
  system and the development tools used. You need a stand-alone
  C compiler and a make utility to compile the interpreter.
  A C compiler, which is only usable from an IDE, is not so
  useful, since some Seed7 programs (e.g. The Seed7 compiler
  s7c) need to call the C compiler as well. In case a make
  utility is missing under Windows the program make7.exe can
  be downloaded from

    https://sourceforge.net/projects/seed7/files/bin/

  In this directory is also a Seed7 installer for Windows.


THE MAKEFILES

    Several makefiles are prepared for various combinations
  of operating system, make utility, C compiler and shell:

  makefile name|operating system |make prog     |C compiler  |shell
  -------------+-----------------+--------------+------------+--------
  mk_linux.mak | Linux/Unix/BSD  | (g)make      | gcc        | sh
  mk_clang.mak | Linux/Unix/BSD  | (g)make      | clang      | sh
  mk_icc.mak   | Linux/Unix/BSD  | (g)make      | icc        | sh
  mk_tcc_l.mak | Linux/Unix/BSD  | (g)make      | tcc        | sh
  mk_cygw.mak  | Windows (Cygwin)| (g)make      | gcc        | sh
  mk_msys.mak  | Windows (MSYS)  | mingw32-make | gcc        | sh
  mk_mingw.mak | Windows (MinGW) | mingw32-make | gcc        | cmd.exe
  mk_nmake.mak | Windows (MinGW) | nmake        | gcc        | cmd.exe
  mk_msvc.mak  | Windows (MSVC)  | nmake        | cl         | cmd.exe
  mk_bcc32.mak | Windows (bcc32) | make         | bcc32      | cmd.exe
  mk_bccv5.mak | Windows (bcc32) | make         | bcc32 V5.5 | cmd.exe
  mk_clangw.mak| Windows (clang) | (g)make      | clang      | cmd.exe
  mk_tcc_w.mak | Windows (tcc)   | (g)make      | tcc        | cmd.exe
  mk_djgpp.mak | DOS             | (g)make      | gcc        | cmd.exe
  mk_osx.mak   | Mac OS X        | make         | gcc        | sh
  mk_osxcl.mak | Mac OS X        | make         | clang      | sh
  mk_freebsd.mk| FreeBSD         | make         | clang/gcc  | sh
  mk_emccl.mak | Linux/Unix/BSD  | make         | emcc + gcc | sh
  mk_emccw.mak | Windows (emcc)  | mingw32-make | emcc + gcc | cmd.exe

  In the optimal case you just copy one of this files to
  'makefile' and do (with the make program from the table above):

    make depend
    make

  When the interpreter is compiled successfully the executable
  and the libraries are placed in the 'bin' directory.
  Additionally a symbolic link to the executable is placed in
  the 'prg' directory (Under Windows symbolic links are not
  supported, so a copy of the executable is placed in the 'prg'
  directory). The Seed7 compiler (s7c) is compiled with:

    make s7c

  The compiler executable is copied to the 'bin' directory. To
  check interpreter and compiler with a test suite (chk_all.sd7)
  use the command:

    make test

  Finally Seed7 can be installed with:

    sudo make install

  To do several compilation attempts in succession you need to
  execute the command

    make clean

  before you do 'make depend' again.


COMPILING UNDER LINUX

    For Linux the compilation process is the simplest. The
  file 'makefile' is (almost) identical to 'mk_linux.mak' and
  that file is already prepared to compile under Linux.
  Go to the 'seed7/src' directory and type:

    make depend
    make

  After the compilation the interpreter executable can be found
  in the 'bin' directory and the 'prg' directory will contain a
  link to the executable.

  In some distributions the X11 library directory is not in
  $PATH. In this case the value of the SYSTEM_DRAW_LIBS variable
  in the makefile must be changed. Replace the -lX11 by the path
  and the name of your libXll (e.g. /usr/X11/lib/libX11.so or
  /usr/X11R6/lib/libX11.a).


COMPILING UNDER BSD AND UNIX

    Probably you can do it just the same way as under Linux.
  If there are problems and the makefile needs to be adjusted
  please let me know. If you send the changed makefile to me
  I can include it in the release.


COMPILING UNDER WINDOWS WITH GCC FROM MINGW

    If gcc and mingw32-make from MinGW are installed use a
  console, go to the 'seed7\src' directory and type:

    copy mk_mingw.mak makefile
    mingw32-make depend
    mingw32-make

  To compile under MSYS (a Unix shell window from the MinGW
  tools) go to the 'seed7/src' directory and type:

    cp mk_msys.mak makefile
    mingw32-make depend
    mingw32-make

  If gmake uses Unix shell commands even if started from a
  Windows console, you can use gmake together with mk_msys.mak
  from a Windows console also.

  To compile with gcc from MinGW and nmake from Windows use
  a console, go to the 'seed7\src' directory and type:

    copy mk_nmake.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter executable can be found
  in the 'bin' directory and it is also copied to prg/s7.exe.


COMPILING UNDER WINDOWS WITH CL FROM MSVC

    To compile Seed7 with cl and nmake (C compiler and make
  utility from a big big software company) it is necessary, that
  cl and nmake can be executed from a console window. The script
  vcvarsall.bat sets up the environment for cl and nmake. This
  script is in the directory

    ...Visual Studio\2019\Community\VC\Auxiliary\Build

  Note that 2019 is the year of the MSVC release. In a 32-bit
  operating system use a console and execute:
  
    ...Studio\2019\Community\VC\Auxiliary\Build\vcvarsall x86

  In older versions of MSVS the script was named vcvars32.bat.
  In a 64-bit operating system you need to execute:

    ...Studio\2019\Community\VC\Auxiliary\Build\vcvarsall x64

  Afterwards the environment is set up for MSVC. Now you can go
  to the 'seed7\src' directory and type:

    copy mk_msvc.mak makefile
    nmake depend
    nmake

  After the compilation the interpreter executable can be found
  in the 'bin' directory and it is also copied to prg/s7.exe.
  Note that the build process saves all environment variables
  in the file seed7/bin/cl_env.ini. Every time the Seed7
  compiler calls cl it restores the environment variables from
  the file seed7/bin/cl_env.ini beforehand. This change of the
  environment happens in the Seed7 compiler and the original
  environment of the console remains unchanged. This means that
  the Seed7 compiler (s7c) can be used without the need to call
  vcvarsall.bat. 


COMPILING UNDER WINDOWS WITH BCC32

    To compile Seed7 with bcc32 and make (C compiler and make
  utility from a smaller software company) use a console, go
  to the 'seed7\src' directory and type:

    copy mk_bcc32.mak makefile
    make depend
    make

  After the compilation the interpreter executable can be found
  in the 'bin' directory and it is also copied to prg/s7.exe.
  For the older version of bcc32 (Version 5.5, which is available
  in the Internet) use:

    copy mk_bccv5.mak makefile
    make depend
    make


COMPILING UNDER WINDOWS WITH TCC

    To compile Seed7 with tcc (Tiny C Compiler) and mingw32-make
  you need version 0.9.27 of tcc. The support for tcc under
  Windows is experimental.

  You need to decide, which archiver utility should be used
  to create libraries. In the makefile mk_tcc_w.mak the value
  ARCHIVER defines the archiver utility to be used. For the ar
  utility from MinGW (ar should be in the search path) use the
  following lines (this is the default):

    ARCHIVER = ar -r
    # ARCHIVER = tiny_libmaker

  As an alternative the tiny_libmaker utility can be used.
  Note that tiny_libmaker supports only 32-bit executables.
  You might need to copy tiny_libmaker.exe to the directory
  tcc. You need to change mk_tcc_w.mak also. Use the lines:

    # ARCHIVER = ar -r
    ARCHIVER = tiny_libmaker

  Additionally you need some things:

    - The include file winsock2.h (copy it to tcc\include)

  Then use a console, go to the 'seed7\src' directory and type:

    copy mk_tcc_w.mak makefile
    make depend
    make

  After the compilation the interpreter executable can be found
  in the 'bin' directory and it is also copied to prg/s7.exe.
  Note that tcc for Windows has some bugs. The program chk_all.sd7
  shows this bugs. Therefore Seed7 does not support the
  compilation with tcc under windows officially.


COMPILING UNDER WINDOWS WITH CYGWIN

    To compile Seed7 with Cygwin several Cygwin packages need to
  be installed. From the category Devel the following packages are
  needed: binutils, gcc-core, libncurses-devel and make. From
  category X11 the package libX11-devel is needed. To use graphic
  programs it is necessary to install Cygwin/X. Instructions to
  install Cygwin/X can be found in the Internet. To test the
  installation start a Cygwin window and do:

    make --version
    gcc --version

  If the Cygwin gcc does not have the name gcc it is necessary
  to edit the file 'mk_cygw.mak'. The line containing

    CC = gcc

  must be changed to define the name of your gcc. If gcc and
  make work well change to the 'seed7/src' directory and do:

    make -f mk_cygw.mak depend
    make -f mk_cygw.mak

  After the compilation the interpreter executable can be found
  in the 'bin' directory and the 'prg' directory will contain a
  link to the executable.

  Cygwin sometimes has problems with symlinks. This is indicated
  by an error message which ends with:

    bin/as.exe: cannot execute binary file

  The presence of a Cygwin symlink problem can be checked in
  the directory /usr/i686-pc-cygwin/bin (or the corresponding
  directory mentioned in the error message). If the file
  /usr/i686-pc-cygwin/bin/as.exe is a small file with less than
  100 bytes and starts with

    !<symlink>

  it is some Cygwin symlink. To fix this error just copy the
  symlink destination over the file as.exe (after making a
  backup of the original symlink file as.exe). Additionally
  it might also be necessary to do the same for ld.exe (and
  maybe for some other symlinks as well).


COMPILING WITH EMCC FROM EMSCRIPTEN

    The makefiles mk_emccl.mak and mk_emccw.mak are provided
  for compiling with emcc under Linux and Windows. Besides emcc
  you need also gcc (under Windows use gcc from MinGW) and
  node.js. When you download emsdk you get also a version of
  node.js.

  To compile Seed7 under Linux use a terminal window and
  activate the PATH and other environment variables for emcc
  (with 'source ./emsdk_env.sh' in the emsdk directory).
  Afterwards go to the directory 'seed7/src' (with cd) and
  type:

    make -f mk_emccl.mak depend
    make -f mk_emccl.mak

  To compile Seed7 under Windows use a console window and
  activate the PATH and other environment variables for emcc
  (with 'emsdk_env.bat' in the emsdk directory). Afterwards
  go to the directory 'seed7\src' (with cd) and type:

    mingw32-make -f mk_emccw.mak depend
    mingw32-make -f mk_emccw.mak

  After compilation of Seed7 the Seed7 interpreter (s7.js) can
  be started (in the directory 'seed7/prg') with:

    node s7.js hello

  Note that the Emscripten version of Seed7 is experimental.
  Due to limitations of Emscripten and missing Seed7 driver
  libraries several things do not work as they should:

    - In Emscripten stdout is always line buffered.
    - Reading from stdin is not possible (EOF is reached
      immediately).
    - JavaScript and Wasm currently do not support
      synchronous I/O.
    - Processes cannot be started.
    - Sockets cannot be used.
    - Graphics is supported from the browser, but
      without synchronous I/O its usage is limited.
    - Graphics from node.js would need libraries that
      access win32 or X11 via native calls.

  When you execute

    node s7.js aS7Program

  you might get an error like:

    exception thrown: RangeError: Maximum call stack size exceeded

  In this case you can raise the allowed stack size. Execute
  node.js as follows (chose a sufficient stack_size):

    node --stack_size=8192 s7.js chkstr

  You can also raise the allowed stack size of node.js
  permanently. Under Windows this is done with the command
  editbin (from Visual-C). The stack is increased (in the
  directory of node.exe) with:

    editbin /stack:33554432 node.exe

  If the stack problems are triggered by emscripten the file
  .emscripten in your home directory must be also adjusted.
  Change the NODE_JS entry to:

    NODE_JS = ['node', '--stack_size=8192']


COMPILING UNDER DOS WITH DJGPP

    You need gcc and make from DJGPP. Make sure that the search
  PATH leads to gcc and make from DJGPP. Use the command line,
  go to the 'seed7\src' directory and type:

    copy mk_djgpp.mak makefile
    make depend
    make

  After the compilation the interpreter executable can be found
  in the 'bin' directory and it is also copied to prg/s7.exe.
  If your get errors you can try mk_djgp2.mak instead.

  DOS usually supports only files with 8.3 file names (8 Ascii
  character name + dot + 3 Ascii character extension). The C
  source and header files of Seed7 all use this convention.
  Seed7 library files use longer file names. The DOS version of
  Seed7 maps long file names like reference.s7i to REFERE~1.S7I.
  Dosbox uses the same mapping so it should just work. Dosemu
  uses a different mapping, so the libraries are not found.
  In that case I suggest to copy files with long names in the
  'lib' directory:

    copy reference.s7i REFERE~1.S7I
    copy hashsetof.s7i HASHSE~1.S7I
    copy environment.s7i ENVIRO~1.S7I
    copy null_file.s7i NULL_F~1.S7I
    copy external_file.s7i EXTERN~1.S7I
    copy clib_file.s7i CLIB_F~1.S7I
    copy enable_io.s7i ENABLE~1.S7i
    copy graph_file.s7i GRAPH_~1.S7I

  Note that the DOS version of Seed7 currently does not support
  graphics, sockets, processes and databases.


COMPILING UNDER MAC OS X

    To compile under Mac OS X make sure that the command line
  tools for OS X are installed. They can be obtained from Xcode
  (Xcode Menu: Xcode->Preferences->Downloads). Alternatively
  the command line tools can be downloaded directly. The tools
  contain the C compilers clang and gcc. They also provide a
  'make' utility. Depending on the version of OS X it might be
  necessary to install also XQuartz (the X11 support of OS X).
  In newer versions of OS X gcc is based on clang. You can use
  this gcc, but it is not the original gcc. To obtain the
  original gcc you have to download it from a repository like
  Homebrew. To compile Seed7 with gcc (the clang based gcc or
  the original gcc) start a terminal, go to the 'seed7/src'
  directory and type:

    cp mk_osx.mak makefile
    make depend
    make

  To compile Seed7 with clang use the following commands:

    cp mk_osxcl.mak makefile
    make depend
    make

  After the compilation the interpreter executable can be found
  in the 'bin' directory and the 'prg' directory will contain a
  link to the executable.

  Under Mac OS X the X11 library is usually found in
  '/usr/X11R6/lib'. For unknown reasons the Xcode (Mac OS X)
  linker normally does not search libraries in '/usr/X11R6/lib'.
  Therefore 'mk_osx.mak' defines the following linker flag:

    LDFLAGS = -L/usr/X11R6/lib

  Besides this 'mk_osx.mak' is almost identical to
  'mk_linux.mak' and 'makefile'. If the X11 library is in a
  different directory you need to change the LDFLAGS value to
  that directory.


COMPILING UNDER ANDROID

    To compile under Android you need to install the termux
  app on the Android device. Afterwards use the following
  commands:

    pkg install clang
    pkg install make
    pkg install git
    git clone https://github.com/ThomasMertes/seed7.git
    cd seed7/src

  Under termux the command gcc refers also to clang. There are
  two ways to compile Seed7 with termux. The first way uses
  the makefile mk_clang.mak. To compile with mk_clang.mak use
  the following commands:

    make -f mk_clang.mak depend
    make -f mk_clang.mak
    make -f mk_clang.mak s7c

  The second way to compile Seed7 uses makefile. To compile
  with makefile use the following commands:

    make depend
    make
    make s7c


PACKAGES FOR DATABASES UNDER LINUX

    Seed7 supports database access. Therefore the client library
  packages of the databases must be installed. On my computer
  the names of the client library packages are:

  - MariaDb:     mariadb-client
  - Sqlite:      sqlite3
  - PostgreSql:  postgresql
  - ODBC:        unixODBC
  - Firebird:    firebird
  - SQL Server:  libtdsodbc0

  The client libraries are sufficient for Seed7 to access the
  databases. Seed7 provides replacements for the C header files
  used by the database client libraries. To use the original
  header files under Linux database client development packages
  must be installed. On my computer the names of the client
  development packages are:

  - MariaDb:     libmariadb-devel
  - Sqlite:      sqlite3-devel
  - PostgreSql:  postgresql-devel
  - ODBC:        unixODBC-devel
  - Firebird:    libfbclient-devel
  - SQL Server:  freetds-devel


WHAT TO DO IF ERRORS HAPPEN DURING THE COMPILATION?

    In most cases errors indicate that some development package
  of your distribution is missing. If your operating system is
  Linux, BSD or Unix not all development packages with header
  files might be installed. In this case you get some errors
  after typing 'make depend'.

 --- Errors such as

    chkccomp.c:56:20: fatal error: stdlib.h: No such file or directory
    s7.c:30:20: fatal error: stdlib.h: No such file or directory

  indicate that the development package of the C library is
  missing. I don't know the name of this package in your
  distribution (under Ubuntu it has the name libc6-dev), but
  you can search for C development libraries and header files.

 --- Errors such as

    con_inf.c:54:18: error: term.h: No such file or directory
    kbd_inf.c:53:18: error: term.h: No such file or directory
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

    common.h:35:17: fatal error: gmp.h: No such file or directory

  indicate that the GMP development package is missing.
  I don't know the name of this package in your distribution.
  Under Linux Mint this package has the name libgmp-dev and is
  described as: Multiprecision arithmetic library developers
  tools.

 --- Errors such as

    gcc chkccomp.c -o chkccomp
    chkccomp.c:28:10: fatal error: base.h: No such file or directory
    compilation terminated.

  or

    del version.h
    process_begin: CreateProcess(NULL, del version.h, ...) failed.
    make (e=2): The system cannot find the file specified.
    mingw32-make: *** [clean] Error 2

  indicate that your makefile contains commands for the cmd.exe
  (or command.com) Windows console, but your 'make' program uses
  a Unix shell (/usr/bin/sh) to execute them. Either use a
  makefile which uses Unix shell commands (e.g. mk_msys.mak or
  mk_cygw.mak) or take care that the 'make' program uses cmd.exe
  (or command.com) to execute the commands.

 --- Errors such as

    s7.c:28:21: error: version.h: No such file or directory

  indicate that you forgot to run 'make depend' before running
  'make'. Since such an attempt produces several unneeded files it
  is necessary now to run 'make clean', 'make depend' and 'make'.

 --- The 'make' utility sometimes writes an error like

    depend:1: *** multiple target patterns.  Stop.

   This indicates that an old 'depend' file (probably a relict of
   an earlier 'make' command with a different makefile) does not
   fit to the current makefile. To get rid of this error remove
   the file 'depend'. After that the compilation process must be
   started from scratch with 'make clean', 'make depend' and
   'make'.

 --- If you use bcc32 an error like

    Error E2194: Could not find file 'Studio\7.0\bin\bcc32.exe'

  can happen. This indicates that the search path where bcc32.exe
  is found contains a space and the 'make' program is not capable
  to handle spaces in a search path. Creating the batch file
  'seed7\src\bcc32.bat' with the content

    bcc32.exe %*

  helps. As alternate solution 'bcc32.exe' and 'bcc32.cfg' can
  be copied to the 'seed7\src' directory (this solution should be
  avoided, since it does not consider updates of bcc32).

 --- If you use cl (from msvc) an error like

    NMAKE : fatal error U1077: 'cl' : return code '0xc0000135'

  indicates that you forgot to execute vcvars32 before executing
  'make depend' or 'make'. A message box, which complains that
  mspdb100.dll was not found, indicates also that vcvars32 was
  not executed.

 --- If you use clang under Windows an error like

    chkccomp.c:60:10: fatal error: 'stdlib.h' file not found

  indicates that the include file search path is not set up.
  If clang is based on the VC toolchain it is necessary to
  call vcvars32 from the commandline to set up the environment.

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

 --- If you use icc an error like

   .../compiler/include/math.h(1216):
   error: identifier "_LIB_VERSION_TYPE" is undefined
     _LIBIMF_EXT _LIB_VERSIONIMF_TYPE _LIBIMF_PUBVAR _LIB_VERSIONIMF;

  indicates, that the math.h include file of icc uses
  _LIB_VERSION_TYPE. Recently the support for_LIB_VERSION_TYPE
  has been removed from glibc. Fortunately there is a definition
  of _LIB_VERSION_TYPE in math.h just a few lines above in
  a part deactivated by an #if. The #if line starts with:

    #if (!defined(__linux__) || !defined(__USE_MISC)) && ...

  I added a condition and the line now starts with:

    #if (1 || !defined(__linux__) || !defined(__USE_MISC)) && ...

  This fixed the error.

 --- Other errors

  If you got other errors I would like to know about.
  Please send an mail with detailed information to
  seed7-users@lists.sourceforge.net or to my mail address,
  which can be found at the Seed7 Homepage (look for Links).
  The detailed information should include:

    - Operating system
    - Distribution used
    - C compiler
    - The version of Seed7 you wanted to compile.
    - The complete log of error messages
    - The file src/version.h


WHAT ABOUT THE WARNINGS THAT HAPPEN DURING THE COMPILATION?

    The warnings can usually be ignored. Seed7 is compiled with
  the highest warning level (-Wall). Additionally there are also
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
     recognize if the states of two variables are connected.
     Such as a global fail_flag variable and a local condition
     variable (cond). The connection is: As long as fail_flag is
     FALSE the cond variable is initialized. If the fail_flag
     is TRUE the cond variable is not used and therefore it could
     be in an uninitialized state. At several places I use such
     connected variable states which are not recognized by the
     gcc optimizer and are therefore flagged with a warning. I
     accept such warnings in performance critical paths. I am not
     willing to do "unnecessary" initializations in performance
     critical paths of the program. At places that are not
     performance critical I do some of this "unnecessary"
     initializations just to avoid such warnings.
   - Warnings about passing argument with different width due to
     prototype: Some compilers write such warnings for formal
     boolean (boolType) parameters and actual boolean arguments.
     Since the types of parameter and argument are identical and
     a prototype is specified this warning can be considered as
     false alarm.


WHAT TO DO WITH ERRORS TRIGGERED BY SEED7 PROGRAMS?

    Sometimes errors are triggered, when a Seed7 program runs.
  This can happen because of reasons unrelated to Seed7.

 --- A run-time error like

    error while loading shared libraries: libtinfo.so.5:
    cannot open shared object file: No such file or directory

  indicates that the library libtinfo.so.5 is missing. This
  happened, when starting a program that was dynamically linked
  to ncurses. Obviously the dependency of the ncurses package
  was set wrong. I fixed this by downloading libtinfo.so.5 and
  copying it to the directory /lib64.


HOW TO VERIFY THAT THE INTERPRETER WORKS CORRECT?

    A comprehensive test of the s7 interpreter and the s7c
  compiler can be done in the directory 'prg' with the command:

    ./s7 chk_all

  Under Windows using ./ might not work. Just omit the ./ and
  type:

    s7 chk_all

  The program 'chk_all' uses several check programs to do its
  work. First a check program is interpreted and the output
  is compared to a reference. Then the program is compiled and
  executed and this output is also checked. Finally the C code
  generated by the compiled compiler is checked against the C
  code generated by the interpreted compiler. The checks of
  the compiler are repeated with several compiler options. If
  everything works correct the output is (after the usual
  information from the interpreter):

    compiling the compiler - okay
    chkint ........... okay
    chkovf ........... okay
    chkflt ........... okay
    chkbin ........... okay
    chkchr ........... okay
    chkstr ........... okay
    chkidx ........... okay
    chkbst ........... okay
    chkarr ........... okay
    chkprc ........... okay
    chkbig ........... okay
    chkbool ........... okay
    chkbitdata ........... okay
    chkset ........... okay
    chkhsh ........... okay
    chkfil ........... okay
    chkexc ........... okay


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

  When Seed7 is compiled with

    make depend

  the program chkccomp.c checks if it is possible to compile and
  link a small test program with the GMP library. If the small
  test program works correct the definitions to use 'big_gmp.c'
  are written to version.h:

    #define BIG_GMP_LIBRARY 2
    #define BIGINT_LIB BIG_GMP_LIBRARY

  Additionally the option to link the GMP library is added to
  ADDITIONAL_SYSTEM_LIBS. ADDITIONAL_SYSTEM_LIBS can be found
  in version.h:

    #define ADDITIONAL_SYSTEM_LIBS "  ...  -lgmp"

  The file macros will also contain a definition of
  ADDITIONAL_SYSTEM_LIBS:

    ADDITIONAL_SYSTEM_LIBS =  ...  -lgmp

  If everything works as expected the GMP library will be
  used automatically.

  If chkccomp.c does not succceed with the small GMP test
  program the file version.h will contain definitions to use
  'big_rtl.c' as bigInteger library:

    #define BIG_RTL_LIBRARY 1
    #define BIGINT_LIB BIG_RTL_LIBRARY

  In this case either the include file gmp.h is missing or
  the GMP library cannot be linked. By default chkccomp.c
  uses "-lgmp" as option to link the GMP library. If a
  different option should be used it can be added to the
  makefile used to compile Seed7. The section to create
  chkccomp.h: can be extended with a line like

    echo "#define BIGINT_LIBS \" option to link GMP \"" >> chkccomp.h

  to write a definition of BIGINT_LIBS to the file chkccomp.h.


SOURCE FILES
============

    The C code of Seed7 can be grouped into several categories:
  Interpreter main, Parser, Interpreter core, Primitive action functions,
  General helper functions, Runtime library, Drivers and Compiler data
  library.


INTERPRETER MAIN

    The main() function of the interpreter calls the Parser to
  create an internal program representation and afterwards it calls
  the Interpreter core to execute this program.

    s7.c       Main program of the s7 Interpreter.

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
    libpath.c  Procedures to manage the include library search path.
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
    binlib.c   binary (BIN_*) actions
    blnlib.c   boolean (BLN_*) actions
    bstlib.c   byte string (BST_*) actions
    chrlib.c   char (CHR_*) actions
    cmdlib.c   Directory, file and system command (CMD_*) actions
    conlib.c   Text console (CON_*) actions
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
    pcslib.c   Process (PCS_*) actions
    pollib.c   Poll (POL_*) actions
    prclib.c   proc/statement (PRC_*) actions
    prglib.c   Program (PRG_*) actions
    reflib.c   reference (REF_*) actions
    rfllib.c   ref_list (RFL_*) actions
    sctlib.c   struct (SCT_*) actions
    setlib.c   set (SET_*) actions
    soclib.c   PRIMITIVE_SOCKET (SOC_*) actions
    sqllib.c   database and sqlStatement (SQL_*) actions
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
    executl.c  Initialization operation procedures used at runtime.
    blockutl.c Procedures to maintain objects of type blocktype.
    entutl.c   Procedures to maintain objects of type entitytype.
    identutl.c Procedures to maintain objects of type identtype.
    chclsutl.c Compute the type of a character very quickly.
    arrutl.c   Maintain objects of type arraytype.

  The general helper functions are licensed under the GPL.


RUNTIME LIBRARY

    The functions defined in the runtime library are called from
  interpreted and compiled code (The Seed7 runtime library is
  linked to every compiled Seed7 program).

    arr_rtl.c  Primitive actions for the array type.
    bln_rtl.c  Primitive actions for the boolean type.
    bst_rtl.c  Primitive actions for the byte string type.
    chr_rtl.c  Primitive actions for the integer type.
    cmd_rtl.c  Directory, file and other system functions.
    con_rtl.c  Primitive actions for console/terminal output.
    dir_rtl.c  Primitive actions for the directory type.
    drw_rtl.c  Platform idependent drawing functions.
    fil_rtl.c  Primitive actions for the C library file type.
    flt_rtl.c  Primitive actions for the float type.
    hsh_rtl.c  Primitive actions for the hash map type.
    int_rtl.c  Primitive actions for the integer type.
    itf_rtl.c  Primitive actions for the interface type.
    pcs_rtl.c  Platform idependent process handling functions.
    set_rtl.c  Primitive actions for the set type.
    soc_rtl.c  Primitive actions for the socket type.
    sql_rtl.c  Database access functions.
    str_rtl.c  Primitive actions for the string type.
    tim_rtl.c  Time access using the C capabilities.
    ut8_rtl.c  Primitive actions for the UTF-8 file type.
    heaputl.c  Procedures for heap allocation and maintenance.
    numutl.c   Numeric utility functions.
    sigutl.c   Driver shutdown and signal handling.
    striutl.c  Procedures to work with wide char strings.

  The runtime library is licensed under the LGPL.


DRIVERS

    The drivers are also part of the runtime library.
  Depending on operating system and C compiler the makefile
  decides, which driver is used.

    big_gmp.c  Alternate bigInteger library based on GMP.
    big_rtl.c  Functions for the built-in bigInteger support.
    cmd_unx.c  Command functions which call the Unix API.
    cmd_win.c  Command functions which call the Windows API.
    con_cap.c  Driver for termcap console access.
    con_con.c  Driver for conio console access.
    con_cur.c  Driver for curses console access.
    con_dos.c  Driver for dos console access.
    con_inf.c  Driver for terminfo console access.
    con_tcp.c  Driver for termcap console access.
    con_wat.c  Driver for watcom console access.
    con_win.c  Driver for windows console access.
    con_x11.c  Driver for X11 text console access.
    dir_dos.c  Directory functions which call the Dos API.
    dir_win.c  Directory functions which call the Windows API.
    dll_dos.c  Dynamic link library support (dummy).
    dll_unx.c  Dynamic link library (*.so) support.
    dll_win.c  Dynamic link library (*.dll) support.
    drw_dos.c  Graphic access using the dos capabilities.
    drw_win.c  Graphic access using the windows capabilities.
    drw_x11.c  Graphic access using the X11 capabilities.
    fil_dos.c  File functions which call the Dos API.
    fil_unx.c  File functions which call the Unix API.
    fil_win.c  File functions which call the Windows API.
    gkb_rtl.c  Generic keyboard support for graphics keyboard.
    gkb_win.c  Keyboard and mouse access for windows.
    gkb_x11.c  Keyboard and mouse access with X11 capabilities.
    kbd_inf.c  Driver for terminfo keyboard access.
    kbd_poll.c Driver for terminfo keyboard access using poll().
    kbd_rtl.c  Platform idependent console keyboard support.
    pcs_dos.c  Process functions using the dos capabilities.
    pcs_unx.c  Process functions which use the Unix API.
    pcs_win.c  Process functions which use the Windows API.
    pol_dos.c  Poll type and function using DOS capabilities.
    pol_sel.c  Poll type and function based on select function.
    pol_unx.c  Poll type and function using UNIX capabilities.
    soc_none.c Dummy functions for the socket type.
    sql_base.c Basic database functions.
    sql_fire.c Database access functions for Firebird/InterBase.
    sql_lite.c Database access functions for SQLite.
    sql_my.c   Database access functions for MariaDB and MySQL.
    sql_oci.c  Database access functions for OCI.
    sql_odbc.c Database access functions for the ODBC interface.
    sql_post.c Database access functions for PostgreSQL.
    trm_cap.c  Driver for termcap screen access.
    trm_inf.c  Driver for terminfo screen access.
    tim_dos.c  Time functions which call the Dos API.
    tim_unx.c  Time functions which call the Unix API.
    tim_win.c  Time functions which call the Windows API.

  The drivers are licensed under the LGPL.


COMPILER DATA LIBRARY

    The functions in the compiler data library manipulate
  compiler and interpreter internal data such as "type",
  "ref_list" and "reference". The compiler data library is
  linked to the interpreter and to some compiled Seed7
  programs such as the Seed7 compiler (s7c) itself.

    typ_data.c Primitive actions for the type type.
    rfl_data.c Primitive actions for the ref_list type.
    ref_data.c Primitive actions for the reference type.
    listutl.c  Procedures to maintain objects of type listtype.
    flistutl.c Procedures for free memory list maintenance.
    typeutl.c  Procedures to maintain objects of type typetype.
    datautl.c  Procedures to maintain objects of type identtype.

  The compiler data library is licensed under the GPL.


PROGRAMS USED BY THE MAKEFILES

    The makefiles use programs to write definitions to
  version.h . This are stand-alone programs that are not
  linked to the interpreter or to the runtime library.

    chkccomp.c  Check properties of C compiler and runtime.
    setpaths.c  Write definitions for Seed7 specific paths.
    setwpath.c  Set the search path (PATH variable) under Windows.
    sudo.c      Execute command as administrator under Windows.

  The programs used by the makefiles are licensed under the GPL.


MACROS WRITTEN TO VERSION.H BY THE MAKEFILE

    One of the main jobs of the makefile is creating a version.h
  file. This is done with 'make depend' (or 'gmake depend' or
  'nmake depend'). The version.h file contains several #defines
  which contain information over available features and the way
  they are available. Other #defines can be found in config.h.

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

  USE_WMAIN: Defined if the main function is named wmain.
             This is a way to support Unicode command line
             arguments under Windows. An alternate way to
             support Unicode command line arguments under
             Windows uses the functions getUtf16Argv and
             freeUtf16Argv (both defined in "cmd_win.c").

  USE_WINMAIN: Defined if the main function is named WinMain.

  SEARCH_PATH_DELIMITER: The paths in the PATH environment
                         variable are separated with this
                         character. Linux/Unix/BSD use ':' and
                         Windows uses ';'. If it is unknown at
                         compile-time it is set to 0 and at
                         run-time a function tries to guess it.

  AWAIT_WITH_POLL: The function timAwait() uses the function
                   poll() to implement waiting for a time.
                   Only one #define of AWAIT_WITH_xxx is
                   allowed.

  AWAIT_WITH_PPOLL: The function timAwait() uses the function
                    ppoll() to implement waiting for a time.
                    Only one #define of AWAIT_WITH_xxx is
                    allowed.

  AWAIT_WITH_SELECT: The function timAwait() uses the function
                     select() to implement waiting for a time.
                     Only one #define of AWAIT_WITH_xxx is
                     allowed.

  AWAIT_WITH_SIGACTION: The function timAwait() uses the
                        functions sigaction(), setitimer(),
                        pause() and a signal handler function to
                        catch a SIGALRM signal. Only one #define
                        of AWAIT_WITH_xxx is allowed.

  AWAIT_WITH_SIGNAL: The function timAwait() uses the functions
                     signal(), setitimer(), pause() and a signal
                     handler function to catch a SIGALRM signal.
                     Only one #define of AWAIT_WITH_xxx is
                     allowed.

  USE_TERMINFO: Defined if the console should be used with
                terminfo. Only one #define of USE_TERMxxx is
                allowed.

  USE_TERMCAP: Defined if the console should be used with
               termcap. Only one #define of USE_TERMxxx is
               allowed.

  SIGNAL_HANDLER_CAN_DO_IO: Defined if I/O in signal handlers
                            works.

  CONSOLE_WCHAR: Defined if the console uses wide characters.

  CONSOLE_UTF8: Defined if the console uses UTF-8 characters.

  OS_STRI_WCHAR: Defined if the system calls (os_...) use
                 wide characters (type wchar_t) for string
                 and path parameters. In this case functions
                 like _wgetcwd(), wreaddir() and _wstati64()
                 together with types like 'WDIR', 'wdirent'
                 and 'struct _stati64' must be used. It is
                 therefore necessary to define the os_...
                 macros accordingly.

  OS_STRI_UTF8: Defined if the system calls (os_...) use
                UTF-8 characters (type char) for string and
                path parameters. In this case functions like
                getcwd(), readdir() and stat() together with
                types like 'DIR', 'dirent' and 'struct stat'
                must be used. If this functions and types do
                not use the POSIX/SUS names it is necessary
                to define the os_... macros accordingly.

  OS_STRI_USES_CODE_PAGE: Defined if the system calls (os_...)
                          use characters (type char) encoded
                          with a code page (only the code pages
                          437 and 850 are supported). In this
                          case functions like getcwd(),
                          readdir() and stat() together with
                          types like 'DIR', 'dirent' and
                          'struct stat' must be used.

  ESCAPE_SHELL_COMMANDS: Depending on the shell/os the C
                         functions system() and popen() need
                         to get processed shell commands.
                         If the macro is defined, a
                         backslash (\) is added before the
                         chars tab, space, !, ", $, &, ', (,
                         ), *, ;, <, >, ?, \, `, and |. If
                         the macro is not defined (undef),
                         the whole shell command is surrounded
                         by double quotes (") if it contains
                         a space, &, comma, ;, =, ^, ~ or the
                         non-breaking space.

  QUOTE_WHOLE_SHELL_COMMAND: Defined if shell commands,
                             starting with double quotes ("),
                             need to be quoted a again. In this
                             case the shell command, together
                             with its parameters, needs to be
                             quoted with double quotes (The
                             final command string starts with
                             two double quotes).

  OBJECT_FILE_EXTENSION: The extension used by the C compiler for
                         object files (Several object files and
                         libraries are linked together to an
                         executable). Under Linux/Unix/BSD this
                         is usually ".o" Under Windows this is
                         ".o" for MinGW and Cygwin, but ".obj"
                         for MSVC and bcc32.

  LIBRARY_FILE_EXTENSION: The extension used by the linker for
                          static libraries. Several object files
                          can be grouped to a library. Under
                          Linux/Unix/BSD this is usually ".a".
                          Under Windows this is ".a" for MinGW
                          and Cygwin, but ".lib" for other linkers.

  EXECUTABLE_FILE_EXTENSION: The extension which is used by the
                             operating system for executables.
                             Since executable extensions are not
                             used under Linux/Unix/BSD it is ""
                             for them. Under Windows the value
                             ".exe" is used.

  LINKED_PROGRAM_EXTENSION: The extension of the file produced by
                            compiling and linking a program.
                            Normally this is identical to the
                            EXECUTABLE_FILE_EXTENSION, but in case
                            of Emscripten this is independent from
                            the EXECUTABLE_FILE_EXTENSION.

  INTERPRETER_FOR_LINKED_PROGRAM: Defines an interpreter that is
                                  used if compiler and linker
                                  create a file that must be
                                  interpreted.

  C_COMPILER: Contains the command to call the stand-alone C
              compiler and linker (Most IDEs provide also a
              stand-alone compiler/linker). If the C compiler is
              called via a script C_COMPILER_SCRIPT is defined and
              C_COMPILER is not defined by a makefile. In that
              case C_COMPILER is defined by setpaths.c together
              with the flag CALL_C_COMPILER_FROM_SHELL.

  CPLUSPLUS_COMPILER: Contains the command to call the stand-alone
                      C++ compiler and linker.

  C_COMPILER_SCRIPT: Relative path of a script that calls the
                     stand-alone C compiler and linker.

  CALL_C_COMPILER_FROM_SHELL: Flag that is defined if the
                              stand-alone C compiler and linker is
                              called via a script (defined with
                              C_COMPILER_SCRIPT).

  CC_ERROR_FILDES: File descriptor to which the C compiler writes
                   errors. The MSVC stand-alone C compiler (CL)
                   writes the error messages to standard output
                   (file descriptor 1). The C compilers of
                   Linux/Unix/BSD and the compilers from MinGW and
                   Cygwin write the error messages to the error
                   output (file descriptor 2).

  CC_FLAGS: Contains C compiler flags, which should be used when
            C programs are compiled.

  CC_OPT_DEBUG_INFO: Contains the C compiler option to add source
                     level debugging information to the object file.

  CC_OPT_LINK_TIME_OPTIMIZATION: Contains the compiler option for
                                 link time optimization (e.g.:
                                 "-flto").

  CC_OPT_NO_WARNINGS: Contains the C compiler option to suppress
                      all warnings.

  CC_OPT_OPTIMIZE_1: C compiler option to optimize with a level
                     of -O1.

  CC_OPT_OPTIMIZE_2: C compiler option to optimize with a level
                     of -O2.

  CC_OPT_OPTIMIZE_3: C compiler option to optimize with a level
                     of -O3.

  CC_OPT_TRAP_OVERFLOW: C compiler option to generate traps for
                        signed integer overflow. An integer overflow
                        in the compiled program will trigger the
                        signal defined with OVERFLOW_SIGNAL.
                        CC_OPT_TRAP_OVERFLOW is not defined, if the
                        C compiler does not support such an option
                        or if traps would not lead to a performance
                        advantage.

  CC_OPT_VERSION_INFO: C compiler option to write the C compiler
                       version information.

  CC_SOURCE_UTF8: Defined if the C compiler accepts UTF-8 encoded
                  file names in #line directives. The file names
                  from #line directives are used by the debugger to
                  allow source code debugging.

  CC_VERSION_INFO_FILEDES: File descriptor to which the C compiler
                           writes its version info.

  LINKER: Defined if C_COMPILER does just invoke the stand-alone
          C compiler. In that case LINKER contains the command to
          call the stand-alone linker.

  LINKER_FLAGS: Contains options for the stand-alone linker to link
                a compiled Seed7 program.

  LINKER_OPT_DEBUG_INFO: Contains the linker option to add source
                         level debugging information to the
                         executable file. Many compiler/linker
                         combinations don't need this option
                         to do source level debugging.

  LINKER_OPT_NO_DEBUG_INFO: Linker option to be used without
                            source level debugging. This option
                            can strip debug information (e.g.:
                            "-Wl,--strip-debug").

  LINKER_OPT_OUTPUT_FILE: Contains the linker option to provide the
                          output filename (e.g.: "-o "). If no
                          such option exists the definition of
                          LINKER_OPT_OUTPUT_FILE should be omitted.

  LINKER_OPT_STATIC_LINKING: Contains the linker option to force
                             static linking (e.g.: "-static").

  SYSTEM_LIBS: Options to link system libraries to a compiled
               program. This is intended for options to link
               libraries required by the Seed7 runtime library.
               E.g. libraries for math or socket.

  SYSTEM_BIGINT_LIBS Options to link system bigint libraries to a
                     compiled program. This is intended for options
                     to link libraries required by the Seed7
                     bigint.s7i runtime library (e.g.: "-lgmp").

  SYSTEM_CONSOLE_LIBS: Options to link system console libraries to
                       a compiled program. This is intended for
                       options to link libraries required by the
                       Seed7 console runtime library
                       (e.g.: "-lncurses").

  SYSTEM_DRAW_LIBS: Options to link system graphic libraries to a
                    compiled program. This is intended for options
                    to link libraries required by the Seed7 graphic
                    runtime library (e.g.: "-lX11").

  SYSTEM_DATABASE_LIBS: Options to link system database libraries
                        to a compiled program. This is intended for
                        options to link libraries required by the
                        Seed7 database runtime libraries
                        (e.g.: "-lmysqlclient").

  SYSTEM_MATH_LIBS: Options to link system mathematic libraries to
                    a compiled program. This is intended for
                    options to link libraries required by the
                    Seed7 math.s7i runtime library. (e.g.: "-lm").

  DEFINE_COMMAND_LINE_TO_ARGV_W: Defined if the function
                                 CommandLineToArgvW() is missing or
                                 buggy. In this case the definition
                                 of CommandLineToArgvW() in
                                 cmd_win.c is be used instead.

  SUPPORTS_PARTIAL_LINKING: Defined in chkccomp.h, if partial
                            linking (with the option -r) is
                            supporded and the tool objcopy is
                            present.

  ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS: Defined in chkccomp.h, if
                                       system header files for X11
                                       and ncurses can be replaced
                                       with header files provided
                                       by Seed7.


MACROS WRITTEN TO VERSION.H BY CHKCCOMP.C

    The program chkccomp.c is compiled and executed by the
  makefile. Chkccomp checks properties of C compiler, include
  files, linker, run-time library and operating systems. The result
  is written to version.h

  UNISTD_H_PRESENT: TRUE if the include file <unistd.h> is present.

  HAS_SIGNAL: TRUE if the function signal() is present.

  HAS_SIGACTION: TRUE if the function sigaction() is present.

  SIGNAL_RESETS_HANDLER: TRUE if a signal resets the signal
                         handling to SIG_DFL. If it is FALSE the
                         signal handler stays unchanged, if a
                         signal is raised. If a signal handler is
                         set with signal() an incoming signal might
                         trigger a reset of the signal handling.
                         This depends on the C run-time. If a
                         signal handler is set with sigaction()
                         an incoming signal always leaves the
                         signal handling unchanged.

  restrict: Defined if the C compiler does not support the
            restrict keyword.

  likely: Macro that expands to a __builtin_expect expression,
          if __builtin_expect is supported by the C compiler.
          Without support of __builtin_expect the macro expands
          to its argument.

  unlikely: Macro that expands to a __builtin_expect expression,
            if __builtin_expect is supported by the C compiler.
            Without support of __builtin_expect the macro expands
            to its argument.

  NORETURN: Macro that expands to __attribute__ ((noreturn)),
            if the C compiler supports this attribute. Without
            support for this attribute the macro expands to analyze
            empty expression.

  MACRO_DEFS: String with macro definitions for likely, unlikely
              and NORETURN.

  NO_SOCKETS Defined as -1. The meaning is: There is no socket
             library.

  UNIX_SOCKETS Defined as 1. The meaning is: The operating system
               uses Unix sockets.

  WINSOCK_SOCKETS Defined as 2. The meaning is: The operating system
               uses Windows sockets.

  SOCKET_LIB: Defines the socket library used. The value is one of
              NO_SOCKETS, UNIX_SOCKETS and WINSOCK_SOCKETS.

  NO_DIRECTORY: Defined as -1. The meaning is: No header file with
                definitions for opendir(), readdir() and closedir()
                is present.

  DIRENT_DIRECTORY: Defined as 1. The meaning is: The prototypes
                    of opendir(), readdir() and closedir() are in
                    the header file <dirent.h>.

  DIRECT_DIRECTORY: Defined as 2. The meaning is: The prototypes
                    of opendir(), readdir() and closedir() are in
                    the header file <direct.h>.

  DIRDOS_DIRECTORY: Defined as 3. The meaning is: The opendir(),
                    readdir() and closedir() functions from
                    dir_dos.c are used. This functions are based
                    on _dos_findfirst() and _dos_findnext().

  DIRWIN_DIRECTORY: Defined as 4. The meaning is: The wopendir(),
                    wreaddir() and wclosedir() functions from
                    dir_win.c are used. This functions are based
                    on FindFirstFileW() and FindNextFileW().
                    Additionally the file dir_win.c contains also
                    definitions of opendir(), readdir() and
                    closedir(). This functions are based on
                    FindFirstFileA() and FindNextFileA().

  DIR_LIB: Defines the directory library used. The value is one
           of NO_DIRECTORY, DIRENT_DIRECTORY, DIRECT_DIRECTORY,
           DIRDOS_DIRECTORY and DIRWIN_DIRECTORY.

  os_DIR: Type to be used instead of 'DIR' under the target
          operating system. If not defined 'DIR' is used.

  os_dirent_struct: Type to be used instead of 'struct dirent'
                    under the target operating system. If not
                    defined 'struct dirent' is used.

  os_opendir: Function to be used instead of opendir() under the
              target operating system. If not defined opendir()
              is used.

  os_readdir: Function to be used instead of readdir() under the
              target operating system. If not defined readdir()
              is used.

  os_closedir: Function to be used instead of closedir() under
               the target operating system. If not defined
               closedir() is used.

  os_chdir: Function to be used instead of chdir() under the
            target operating system. If not defined chdir()
            is used.

  OS_GETCWD_MAX_BUFFER_SIZE: Defined if there is a maximum
                             buffer size supported by
                             os_getcwd().

  OS_GETCWD_RETURNS_SLASH: Defined if os_getcwd() returns a
                           path with slashes instead of
                           PATH_DELIMITER.

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

  os_chmod: Function to be used instead of chmod() under the
            target operating system. If not defined chmod() is
            used.

  os_chown(name,uid,gid): Function to be used instead of chown()
                          under the target operating system.
                          If not defined chown() is used. If
                          chown() is not supported this macro is
                          defined empty.

  os_utimbuf_struct: Type to be used instead of
                     'struct utimbuf' under the target operating
                     system. If not defined 'struct utimbuf' is
                     used.

  USE_ALTERNATE_UTIME: Defined if alternate_utime should be
                       used instead of os_utime_orig.

  os_utime_orig: Defined if the function alternate_utime is
                 used instead of the function defined with
                 os_utime. In this case os_utime_orig refers to
                 the original os_utime function and the macro
                 os_utime is redefined to refer to
                 alternate_utime.

  os_utime: Function to be used instead of utime() under the
            target operating system. If not defined utime() is
            used.

  os_remove: Function to be used instead of remove() under the
             target operating system. If not defined remove() is
             used.

  os_rename: Function to be used instead of rename() under the
             target operating system. If not defined rename() is
             used.

  os_system: Function to be used instead of system() under the
             target operating system. If not defined system() is
             used.

  os_fopen: Function to be used instead of fopen() under the
            target operating system. If not defined fopen() is
            used.

  os_popen: Function to be used instead of popen() under the
            target operating system. If not defined popen() is
            used.

  os_pclose: Function to be used instead of pclose() under the
             target operating system. If not defined pclose() is
             used.

  DEFINE_WGETENV: Defined if an own implementation of wgetenv
                  should be used.

  os_getenv: Function to be used instead of getenv() under the
             target operating system. If not defined getenv() is
             used.

  os_setenv: Function to be used instead of setenv() under the
             target operating system. If not defined setenv() is
             used.

  FILENO_WORKS_FOR_NULL: TRUE if the fileno() function works
                         for NULL and returns -1.

  FOPEN_SUPPORTS_CLOEXEC_MODE: TRUE if fopen() supports a
                               mode of "e".

  HAS_FCNTL_SETFD_CLOEXEC: TRUE if fcntl() supports
                           F_SETFD,FD_CLOEXEC.

  HAS_GETADDRINFO: TRUE if the function getaddrinfo() is
                   available.

  HAS_PIPE2: TRUE if the function pipe2() is available.

  HAS_SNPRINTF: TRUE if the function snprintf() is available.

  HAS_VSNPRINTF: TRUE if the function vsnprintf() is available.

  HAS_POPEN: TRUE if the function popen() is available.

  STAT_MISSING: Defined if the function stat() is missing.
                This macro is only used in chkccomp.c.

  os_fstat: Function to be used instead of fstat() under the
            target operating system. If not defined fstat()
            is used.

  os_lstat: Function to be used instead of lstat() under the
            target operating system. If not defined lstat()
            is used.

  os_stat: Function to be used instead of stat() under the
           target operating system. If not defined stat()
           is used.

  os_stat_struct: Type to be used for the output parameter of
                  os_stat() and os_lstat(). If not defined
                  'struct stat' is used.

  os_fstat_struct: Type to be used for the output parameter of
                   os_fstat(). If not defined 'os_stat_struct'
                   is used.

  os_fseek: Function to be used instead of fseek() under the
            target operating system. If it is not defined
            os_fsetpos() or os_lseek() are used.

  os_ftell: Function to be used instead of ftell()  under the
            target operating system. If it is not defined
            os_fgetpos() or os_lseek() are used.

  os_off_t: Type used for os_fseek(), os_ftell(), offsetSeek(),
            offsetTell() and seekFileLength().

  CHAR_SIZE: Size of a char in bits.

  SHORT_SIZE: Size of a short int in bits.

  INT_SIZE: Size of an int in bits.

  LONG_SIZE: Size of a long int in bits.

  INTTYPE_SIZE: Size of an inttype in bits (either 32 or 64).

  FLOAT_SIZE: Size of a float in bits.

  DOUBLE_SIZE: Size of a double in bits.

  FLOATTYPE_SIZE: Size of an inttype in bits (either FLOAT_SIZE or DOUBLE_SIZE).

  POINTER_SIZE: Size of a pointer in bits.

  OS_OFF_T_SIZE: Size of os_off_t in bits.

  TIME_T_SIZE: Size of time_t in bits.

  INT_RANGE_IN_FLOATTYPE_MAX:
      Maximum from the continuous range of integers that map to floats.
      All integers from -INT_RANGE_IN_FLOATTYPE_MAX to
      INT_RANGE_IN_FLOATTYPE_MAX can be converted to 'floatType'
      and back to 'intType' without loss.

  MINIMUM_TRUNC_ARGUMENT:
      Minimum value that trunc() or round() can convert.
      Values below MINIMUM_TRUNC_ARGUMENT raise RANGE_ERROR,
      if trunc() or round() is applied to them.

  MAXIMUM_TRUNC_ARGUMENT:
      Maximum value that trunc() or round() can convert.
      Values above MAXIMUM_TRUNC_ARGUMENT raise RANGE_ERROR,
      if trunc() or round() is applied to them.

  NULL_DEVICE: Name of the NULL device.
               Under Linux/Unix/BSD this is "/dev/null".
               Under Windows this is "NUL:".

  OS_PATH_HAS_DRIVE_LETTERS: TRUE if the absolute paths of
                             the operating system use drive
                             letters. Determined by chkccomp.c.

  REDIRECT_FILDES_1: Shell parameter to redirect to the file
                     descriptor 1. Under Linux/Unix/BSD and
                     Windows this is ">". The file to which the
                     standard output should be redirected must
                     be appended. E.g.: >myFile.

  REDIRECT_FILDES_2: Shell parameter to redirect to the file
                     descriptor 2. Under Linux/Unix/BSD and
                     Windows this is "2>". The file to which
                     the error output should be redirected must
                     be appended. E.g.: 2>myFile.

  FTELL_SUCCEEDS_FOR_PIPE: The ftell() function should return -1
                           if it is called with a pipe (since a
                           pipe is not seekable). This macro is
                           TRUE if ftell() succeeds for pipes
                           (it does not return -1). In this case
                           the function improved_ftell is used
                           which returns -1 if the check with
                           fstat() does not verify that the
                           parameter is a regular file.

  FSEEK_SUCCEEDS_FOR_STDIN: TRUE if fseek(stdin, 0,  SEEK_SET)
                            succeeds (stdin is not redirected).

  FOPEN_OPENS_DIRECTORIES: TRUE if fopen() is able to open
                           directories (it does not return NULL).

  FWRITE_WRONG_FOR_READ_ONLY_FILES: TRUE if fwrite() to a
                                    read only file succeeds (it
                                    does not return 0).

  REMOVE_FAILS_FOR_EMPTY_DIRS: TRUE if remove() cannot remove
                               an empty directory (it does not
                               return 0).

  HOME_DIR_ENV_VAR: Name of the environment variable that contains
                    the path of the home directory.

  INCLUDE_SYS_UTIME: Defined if utime() respectively _wutime()
                     is defined in <sys/utime.h>.

  USE_ALTERNATE_UTIME: Defined if utime() is not able to change
                       the access and modification times of a
                       directory (utime() does not return 0 and
                       errno has the value EACCES).

  RSHIFT_DOES_SIGN_EXTEND: The C standard specifies that the
                           right shift of signed integers is
                           implementation defined, if the
                           shifted values are negative. The
                           macro is TRUE if the sign of negative
                           signed integers is preserved with a
                           right shift ( -1 >> 1 == -1 ).

  TWOS_COMPLEMENT_INTTYPE: TRUE if signed integers are
                           represented as twos complement
                           numbers. This allows some simplified
                           range checks in compiled programs.
                           This macro is defined if
                           ~(-1) == 0 holds.

  ONES_COMPLEMENT_INTTYPE: TRUE if signed integers are
                           represented as ones complement
                           numbers. This macro is currently
                           not used.

  LITTLE_ENDIAN_INTTYPE: TRUE if the byte ordering of
                         integers is little endian.

  BIG_ENDIAN_INTTYPE: TRUE if the byte ordering of integers
                      is big endian.

  MALLOC_ALIGNMENT: Contains the number of zero bits (counting
                    from the least significant bit) in an address
                    returned by malloc(). The memory management
                    in the parser uses this alignment.

  UNALIGNED_MEMORY_ACCESS_OKAY: Defined if integers can be read
                                from unaligned memory positions.
                                This macro is currently not used.

  NO_EMPTY_STRUCTS: Defined if the C compiler considers an empty
                    struct as syntax error.

  HAS_GETRLIMIT: TRUE if the functions getrlimit() and
                 setrlimit() are available.

  MEMCMP_RETURNS_SIGNUM: TRUE if memcmp() returns only the
                         values -1, 0 and 1.

  HAS_WMEMCMP: TRUE if the function wmemcmp() is available.

  WMEMCMP_RETURNS_SIGNUM: TRUE if wmemcmp() returns only the
                          values -1, 0 and 1.

  HAS_WMEMCHR: TRUE if the function wmemchr() is available.

  HAS_WMEMSET: TRUE if the function wmemset() is available.

  HAS_SETJMP: TRUE if the functions setjmp() and
              longjmp() are available.

  HAS_SIGSETJMP: TRUE if the functions sigsetjmp() and
                 siglongjmp() are available. If it is FALSE
                 the functions setjmp() and longjmp() must be
                 used instead.

  HAS_SYMBOLIC_LINKS: TRUE if the operating system supports
                symlinks.

  HAS_READLINK: TRUE if the function readlink() is available.

  HAS_FIFO_FILES: TRUE if the operating system supports fifo
                  (first in first out) files.

  HAS_SELECT: TRUE if the function select() is available.

  HAS_POLL: TRUE if the function poll() is available.

  HAS_MMAP: TRUE if the function mmap() is available.

  INT_DIV_BY_ZERO_POPUP: Defined if an integer division by zero
                         may trigger a popup window. Consequently
                         chkccomp.c defines CHECK_INT_DIV_BY_ZERO,
                         to avoid the popup.

  DO_SIGFPE_WITH_DIV_BY_ZERO: TRUE if SIGFPE should be raised
                              with an integer division by zero.
                              If it is FALSE raise(SIGFPE) can
                              be called instead. Under Windows it
                              is necessary to trigger SIGFPE this
                              way, to assure that the debugger can
                              catch it. If the compiler (s7c)
                              is called with the option -e the
                              function triggerSigfpe() is used to
                              raise SIGFPE, if an uncaught
                              EXCEPTION occurs.

  CHECK_INT_DIV_BY_ZERO:
      TRUE if integer divisions must be checked for a division by
      zero. This applies to the division operations div and mdiv.
      The generated C code should, if a division by zero occurs,
      raise the exception NUMERIC_ERROR instead of doing the
      illegal divide operation.

  CHECK_INT_DIV_ZERO_BY_ZERO:
      TRUE if the C expression 0/0 might not trigger SIGFPE.
      C compilers assume that so called "undefined behavior" will
      not happen. According to the C standard a division by 0
      triggers undefined behavior. This way a C compiler is allowed
      to optimize the expressions 0/0 and 0/variable to 0. Likewise
      the expression variable/variable can be optimized to 1.
      In Seed7 a division by zero is defined behavior, since it
      raises the exception NUMERIC_ERROR. This configuration
      setting applies to the division operations div and mdiv.
      The generated C code should, if a division by zero occurs,
      raise the exception NUMERIC_ERROR instead of allowing the
      C compiler to do its optimization.

  CHECK_INT_REM_BY_ZERO:
      TRUE if integer remainder must be checked for a division by
      zero. This applies to the division operations rem and mod.
      The generated C code should, if a remainder by zero occurs,
      raise the exception NUMERIC_ERROR instead of doing the
      illegal divide operation.

  CHECK_INT_REM_ZERO_BY_ZERO:
      TRUE if the C expression 0%0 might not trigger SIGFPE.
      C compilers assume that so called "undefined behavior" will
      not happen. According to the C standard a division by 0
      triggers undefined behavior. This way a C compiler is allowed
      to optimize the expressions 0%0 and 0%variable to 0. Likewise
      the expression variable%variable can be optimized to 1.
      In Seed7 a division by zero is defined behavior, since it
      raises the exception NUMERIC_ERROR. This configuration
      setting applies to the division operations rem and mod.
      The generated C code should, if a division by zero occurs,
      raise the exception NUMERIC_ERROR instead of allowing the
      C compiler to do its optimization.

  TURN_OFF_FP_EXCEPTIONS: In Seed7 floating point errors such
                          as the division by zero should create
                          values like Infinite and NaN which
                          are defined in IEEE 754. Some C
                          compilers/libraries raise exceptions
                          for floating point errors. This macro
                          is used to turn off such a behaviour.

  DEFINE_MATHERR_FUNCTION: Some C compilers/libraries call the
                           matherr() function for every floating
                           point error and terminate the program,
                           if the function is not present.
                           To get the Seed7 behaviour of using
                           the IEEE 754 values of Infinite and
                           NaN this function must be defined and
                           it must return 1.

  DEFINE__MATHERR_FUNCTION: Some C compilers/libraries call the
                            _matherr() function for every floating
                            point error and terminate the program,
                            if the function is not present.
                            To get the Seed7 behaviour of using
                            the IEEE 754 values of Infinite and
                            NaN this function must be defined and
                            it must return 1.

  HAS_LOG2: TRUE if the function log2() is available.

  HAS_CBRT: TRUE if the function cbrt() is available.

  FLOAT_NAN_COMPARISON_OKAY: TRUE if comparisons between NaN and
                             any other value return FALSE.
                             Comparison refers to comparisons with
                             ==  <  >  <=  or  >= .

  FLOAT_ZERO_COMPARISON_OKAY: TRUE if comparisons consider negative
                              zero as equal to positive zero.
                              FALSE if this is not the case.
                              Comparison refers to comparisons with
                              ==  <  >  <=  or  >= .

  POW_OF_NAN_OKAY: TRUE if pow(NaN, 0.0) returns 1.0 and
                   pow(NaN, anyOtherExponent) returns NaN.

  POW_OF_ZERO_OKAY: TRUE if the pow() function works correct
                    for a base of zero (0.0 or -0.0) and a negative
                    exponent.

  POW_OF_NEGATIVE_OKAY: TRUE if the pow() function works
                        correct, if the base is negative.

  POW_OF_ONE_OKAY: TRUE if the pow() function always returns
                   1.0 if the base is 1.0 (Even for an exponent
                   of NaN).

  POW_EXP_NAN_OKAY: TRUE if the pow() function always returns
                    NaN if the exponent is NaN and the base is
                    not 1.0 (pow(1.0, NaN) should return 1.0).

  POW_EXP_MINUS_INFINITY_OKAY: TRUE if the pow() function works
                               correct for an exponent of minus
                               infinity.

  LOG_OF_NAN_OKAY: TRUE if log(NaN) returns NaN.

  LOG_OF_ZERO_OKAY: TRUE if log(0.0) returns minus infinity.

  LOG_OF_NEGATIVE_OKAY: TRUE if log() of a negative number
                        returns NaN.

  LOG10_OF_NAN_OKAY: TRUE if log10(NaN) returns NaN.

  LOG10_OF_ZERO_OKAY: TRUE if log10(0.0) returns minus
                      infinity.

  LOG10_OF_NEGATIVE_OKAY: TRUE if log10() of a negative number
                          returns NaN.

  LOG2_OF_NAN_OKAY: TRUE if log2(NaN) returns NaN.

  LOG2_OF_ZERO_OKAY: TRUE if log2(0.0) returns minus infinity.

  LOG2_OF_NEGATIVE_OKAY: TRUE if log2() of a negative number
                         returns NaN.

  FLOAT_ZERO_DIV_ERROR:
      TRUE if floating point divisions by zero cause compilation errors.
      Some C compilers check if the dividend is 0.0 and classify a
      floating point division by zero as fatal compilation error.
      Some C compilers even trigger wrong behavior if the dividend
      is 0.0. In this case FLOAT_ZERO_DIV_ERROR is also TRUE.
      If FLOAT_ZERO_DIV_ERROR is TRUE the generated C code should
      avoid divisions by zero and generate code to return Infinity,
      -Infinity or NaN instead.

  CHECK_FLOAT_DIV_BY_ZERO:
      TRUE if floating point divisions by zero don't conform to IEEE 754.
      According to IEEE 754 a floating point division by zero should
      return Infinity, -Infinity or NaN. In this case the compiler
      generates C code, which checks all float divisions ( / and /:= )
      for division by zero. The generated C code should return
      Infinity, -Infinity or NaN instead of doing the divide operation.

  PRINTF_MAXIMUM_FLOAT_PRECISION: Precision up to which writing a
                                  float with printf (using format
                                  %e or %f) will always work ok.
                                  This can be defined in a makefile
                                  and is used only in chkccomp.c.

  PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION: Precision up to which
                                        writing a float with printf
                                        (using format %e) will
                                        always work ok.

  LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION: Defined if a printf
                                       (using format %e) with a
                                       larger precision crashes.
                                       In that case it is defined
                                       with a low precision limit
                                       as string.

  PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION: Precision up to which
                                        writing a float with printf
                                        (using format %f) will
                                        always work ok.

  LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION: Defined if a printf
                                       (using format %f) with a
                                       larger precision crashes.
                                       In that case it is defined
                                       with a low precision limit
                                       as string.

  ISNAN_WITH_UNDERLINE: Defined if the macro/function _isnan() is
                        defined in <float.h> respectively <math.h>
                        instead of isnan().

  USE_ALTERNATE_LOCALTIME_R: Defined if the function
                             alternate_localtime_r() should be used
                             instead of localtime().

  USE_LOCALTIME_R: Defined if the thread-safe function
                   localtime_r() should be used instead of
                   localtime().

  USE_LOCALTIME_S: Defined if the thread-safe function
                   localtime_s() should be used instead of
                   localtime().

  DEFINE_OS_ENVIRON: Defined if environ respectively _wenviron
                     is not defined in <stdlib.h> or <unistd.h>.
                     In this case environ respectively _wenviron
                     must be defined.

  INITIALIZE_OS_ENVIRON: Defined if environ respectively
                         _wenviron is NULL unless getenv() is
                         called.

  LIMITED_CSTRI_LITERAL_LEN: 0 if the C compiler does not limit
                               the length of string literals and
                               at run-time the string literal has
                               the correct value.
                             -1 if the compiler prohibits long
                               string literals.
                             -2 if at run-time the whole string
                               literal is wrong.
                             >0 if at run-time the string
                               literal is correct up to a position.

  LIMITED_ARRAY_LITERAL_LEN: 0 if the C compiler does not limit
                               the length of array literals and
                               at run-time the array literal has
                               the correct value.
                             -1 if the compiler prohibits long
                               array literals.
                             -2 if at run-time the whole array
                               literal is wrong.
                             >0 if at run-time the array
                               literal is correct up to a position.

  NO_BIG_LIBRARY: Defined as -1. The meaning is: No library to
                  to implement the bigInteger functions.

  BIG_RTL_LIBRARY: Defined as 1. The meaning is: The big_rtl.c
                   library is used to implement the bigInteger
                   functions.

  BIG_GMP_LIBRARY: Defined as 2. The meaning is: The big_gmp.c
                   library is used to implement the bigInteger
                   functions.

  BIGINT_LIB: Defines the bigInteger library used. The value is
              one of NO_BIG_LIBRARY, BIG_RTL_LIBRARY and
              BIG_GMP_LIBRARY.

  C_COMPILER_VERSION: Contains a string describing the version of
                      the C compiler which compiled the Seed7
                      runtime libraries. This string can be used
                      together with GET_CC_VERSION_INFO to compare
                      the C compiler used to compile the Seed7
                      runtime libraries with the actual version of
                      the C compiler.

  CC_NO_OPT_OUTPUT_FILE: Defined if compiling and linking with
                         one command does not work, with the option
                         LINKER_OPT_OUTPUT_FILE.

  SEED7_LIB: Contains the name of the Seed7 runtime library.

  DRAW_LIB, CONSOLE_LIB, DATABASE_LIB, COMP_DATA_LIB, COMPILER_LIB:
               Contain names of other Seed7 runtime libraries.

  CC_ENVIRONMENT_INI: Path to an INI file with the environment for
                      the C compiler. If it is "" the C compiler
                      does not need environment settings. The INI
                      file contains a snapshot of the environment
                      variables at the time Seed7 is compiled. As
                      snapshot it contains also environment
                      variables that are not necessary for the C
                      compiler.

  S7_LIB_DIR: Directory containing the Seed7 runtime libraries.
              This path uses the standard path representation
              (a slash is used as path separator and instead of
              a drive letter like "C:" the path "/c" is used).

  SEED7_LIBRARY: Directory containing the Seed7 include files.
                 This path uses the standard path representation
                 (a slash is used as path separator and instead of
                 a drive letter like "C:" the path "/c" is used).

  INT32TYPE: Name of a signed integer type that is 32 bits wide.

  UINT32TYPE: Name of an unsigned integer type that is 32 bits wide.

  INT64TYPE: Name of a signed integer type that is 64 bits wide.

  UINT64TYPE: Name of an unsigned integer type that is 64 bits wide.

  INT32TYPE_LITERAL_SUFFIX: The suffix used by the literals of the
                            32 bits wide integer type.

  INT64TYPE_LITERAL_SUFFIX: The suffix used by the literals of the
                            64 bits wide integer type.


MACROS DEFINED IN CONFIG.H

  WITH_STRI_CAPACITY:
      TRUE if the Seed7 runtime library uses strings with capacity.
      The capacity of a string can be larger than its size.
      Strings with capacity can be enlarged without calling realloc().

  ALLOW_STRITYPE_SLICES:
      TRUE if the actual characters of a string can be stored elsewhere.
      This allows string slices without the need to copy characters.

  ALLOW_BSTRITYPE_SLICES:
      TRUE if the actual characters of a bstring can be stored elsewhere.
      This allows bstring slices without the need to copy characters.

  WITH_SQL: 1 if SQL should be supported, 0 otherwise.

  MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS: Defined in config.h, if
                                      absolute paths (paths
                                      starting with '/') must
                                      be mapped to operating
                                      system paths with drive
                                      letter. E.g.: "/c" is
                                      mapped to the drive letter
                                      "C:".

  FORBID_DRIVE_LETTERS: Defined in config.h, if a Seed7 path
                        with drive letters must raise
                        RANGE_ERROR.

  EMULATE_ROOT_CWD: Defined in config.h, if the operating
                    system uses drive letters and reading the
                    directory "/" must return a list of
                    available drives.
