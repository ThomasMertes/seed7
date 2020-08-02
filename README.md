## Welcome to the world of Seed7

Seed7 is a general purpose programming language described at the [Seed7 Homepage](http://seed7.sourceforge.net/index.htm).

Seed7 is a higher level language compared to Ada, C++ and Java. The Seed7 [interpreter](http://seed7.sourceforge.net/faq.htm#interpreter) and the [example programs](http://seed7.sourceforge.net/scrshots/index.htm) are open-source software. There is also an open-source Seed7 [compiler](http://seed7.sourceforge.net/scrshots/s7c.htm). The compiler translates Seed7 programs to C programs which are subsequently compiled to machine code.

## Features of Seed7

* As an [extensible programming language](http://seed7.sourceforge.net/faq.htm#extensible_programming) it supports user defined [statements](http://seed7.sourceforge.net/examples/declstat.htm) and [operators](http://seed7.sourceforge.net/examples/operator.htm).
* Types are first class objects ([Templates and generics](http://seed7.sourceforge.net/examples/for_decl.htm) can be defined easily without special syntax).
* Predefined constructs like arrays or [for-loops](http://seed7.sourceforge.net/examples/declstat.htm) are defined in libraries.
* [Object orientation](http://seed7.sourceforge.net/manual/objects.htm) supports interfaces and [multiple dispatch](http://seed7.sourceforge.net/faq.htm#multiple_dispatch).
* [Static type checking](http://seed7.sourceforge.net/faq.htm#static_type_checking) and the absence of [automatic casts](http://seed7.sourceforge.net/faq.htm#automatic_casts) help to find errors at compile-time.
* There is an [automatic memory management](http://seed7.sourceforge.net/faq.htm#garbage_collection) that works without a garbage collection process.
* [Exception handling](http://seed7.sourceforge.net/manual/errors.htm#Exceptions) and [source code debugging](http://seed7.sourceforge.net/manual/errors.htm#Stack_trace) are supported.
* If an integer computation [overflows](http://seed7.sourceforge.net/faq.htm#integer_overflow) the exception [OVERFLOW\_ERROR](http://seed7.sourceforge.net/manual/errors.htm#OVERFLOW_ERROR) is raised.
* Numbers which have unlimited size are provided with the types [bigInteger](http://seed7.sourceforge.net/manual/types.htm#bigInteger) and [bigRational](http://seed7.sourceforge.net/manual/types.htm#bigRational).
* Functions, operators and statements can be [overloaded](http://seed7.sourceforge.net/faq.htm#overloading).
* There are various [predefined types](http://seed7.sourceforge.net/manual/types.htm) like [array](http://seed7.sourceforge.net/manual/types.htm#array), [hash](http://seed7.sourceforge.net/manual/types.htm#hash), [set](http://seed7.sourceforge.net/manual/types.htm#set), [struct](http://seed7.sourceforge.net/manual/types.htm#struct), [color](http://seed7.sourceforge.net/manual/types.htm#color), [time](http://seed7.sourceforge.net/manual/types.htm#time), [duration](http://seed7.sourceforge.net/manual/types.htm#duration), etc.
* Seed7 programs are [source code portable](http://seed7.sourceforge.net/faq.htm#portable) without any need to change the code.
* Seed7 provides a [database independent API](http://seed7.sourceforge.net/faq.htm#database_access), which can connect to MySQL, MariaDB, SQLLite, PostgreSQL, Oracle, ODBC, Firebird, Interbase, Db2 and SQL Server databases databases.
* Seed7 runs under Linux, various Unix versions and Windows.
* The interpreter and the example programs use the GPL license, while the runtime library uses the LGPL license.

## What is contained in this package

The Seed7 package contains the Seed7 interpreter in source besides documentation files, include files and program examples. The following sub directories exist:

* src     The source of the interpreter
* prg    Program examples
* lib      Include / library files
* doc    Documentation files
* bin     Executables and runtime libraries

Using the extension several files can be distinguished:

* \*.txt      Documentation files
* \*.sd7    Program example files
* \*.s7i     Include / library files
* \*.dna   Include file for the dna program
* \*.c        C source file
* \*.h       C include file

## How to compile the interpreter

For Linux this is quite simple (to compile under other operating systems see [seed7/src/read\_me.txt](https://github.com/ThomasMertes/seed7/blob/master/src/read_me.txt)). The makefile is prepared for Linux. First make sure that gcc, make and the development packages for X11 and ncurses are installed. Then go to the seed7/src directory and type:

    make depend
    make

After the compilation the interpreter is linked to the file seed7/prg/s7. The Seed7 compiler (s7c) is compiled with:

    make s7c

The compiler executable is copied to the bin directory. To check interpreter and compiler with a test suite (chk\_all.sd7) use the command:

    make test

Finally Seed7 can be installed with:

    sudo make install

To do several compilation attempts in succession you need to execute the command

    make clean

before you do `make depend` again.

The file [seed7/src/read\_me.txt](https://github.com/ThomasMertes/seed7/blob/master/src/read_me.txt) contains a detailed explanation of the compilation process.

## How to compile under other operating systems

Seed7 supports several operating systems such as Windows, Mac OS X and various Unix variants. How to compile under these operating systems is explained in the file [seed7/src/read\_me.txt](https://github.com/ThomasMertes/seed7/blob/master/src/read_me.txt).

## How to execute a program

After compiling the interpreter you can make the first test. Switch to the seed7/prg directory and type:

    ./s7 hello

This executes the hello world program which is in the hello.sd7 file. Every file with the .sd7 extension can be executed with the s7 interpreter.

A list of the program files with a short description can be found in the file: [prg/files.txt](https://github.com/ThomasMertes/seed7/blob/master/prg/files.txt).

## What to do when there are problems with the compilation

The file [seed7/src/read\_me.txt](https://github.com/ThomasMertes/seed7/blob/master/src/read_me.txt) discusses the compilation process with more detail.

## How to send a patch

Just send a pull request to the GitHub repository of Seed7. Participation is always welcome. If you send a pull request, it is assumed that your change is released under the GPL (or LGPL for libraries) license.

Greetings Thomas Mertes
