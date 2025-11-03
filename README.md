## Welcome to the world of Seed7

The [Seed7 Homepage](https://seed7.net/index.htm) is at [https://seed7.net/index.htm](https://seed7.net/index.htm)

Seed7 is a high level general purpose programming language. The Seed7 project is open-source. There is an [interpreter](https://seed7.net/faq.htm#interpreter) and a [compiler](https://seed7.net/scrshots/s7c.htm). The compiler translates Seed7 programs to C programs which are subsequently compiled to machine code.

## Features of Seed7

* As an [extensible programming language](https://seed7.net/faq.htm#extensible_programming) it supports user defined [statements](https://seed7.net/examples/declstat.htm) and [operators](https://seed7.net/examples/operator.htm).
* The whole language is not hard-coded in the compiler but defined via include-libraries ([syntactically](https://seed7.net/manual/syntax.htm) and [semantically](https://seed7.net/examples/declstat.htm)).
* Types are first class objects ([Templates and generics](https://seed7.net/examples/for_decl.htm) can be defined easily without special syntax).
* All Seed7 functions can be executed at compile-time or at run-time.
* [Object orientation](https://seed7.net/manual/objects.htm) supports interfaces and [multiple dispatch](https://seed7.net/faq.htm#multiple_dispatch).
* [Static type checking](https://seed7.net/faq.htm#static_type_checking) and the absence of [automatic casts](https://seed7.net/faq.htm#automatic_casts) help to find errors at compile-time.
* There is an [automatic memory management](https://seed7.net/faq.htm#garbage_collection) that works without a garbage collection process.
* Seed7 is operating system and architecture independent.
* Seed7 programs are [source code portable](https://seed7.net/faq.htm#portable) without any need to change the code.
* [Exception handling](https://seed7.net/manual/errors.htm#Exceptions) is supported (e.g. an integer [overflow](https://seed7.net/faq.htm#integer_overflow) raises [OVERFLOW_ERROR](https://seed7.net/manual/errors.htm#OVERFLOW_ERROR)).
* [Source code debugging](https://seed7.net/manual/errors.htm#Stack_trace) is supported.
* The types [bigInteger](https://seed7.net/manual/types.htm#bigInteger) and [bigRational](https://seed7.net/manual/types.htm#bigRational) support numbers of unlimited size.
* Functions, operators and statements can be [overloaded](https://seed7.net/faq.htm#overloading).
* There are various [predefined types](https://seed7.net/manual/types.htm) like [array](https://seed7.net/manual/types.htm#array), [hash](https://seed7.net/manual/types.htm#hash), [set](https://seed7.net/manual/types.htm#set), [struct](https://seed7.net/manual/types.htm#struct), [color](https://seed7.net/manual/types.htm#color), [time](https://seed7.net/manual/types.htm#time), [duration](https://seed7.net/manual/types.htm#duration), etc.
* Seed7 runs under Linux, MacOS, various Unix versions and Windows.
* Interpreter, compiler and the example programs use the GPL license, while the runtime library uses the LGPL license.

## Standard libraries of Seed7

* The libraries provide the same functionality under all supported operating systems.
* Numerous [standard libraries](https://seed7.net/libraries) allow writing programs without calling functions from third party libraries.
* There is a [database independent API](https://seed7.net/faq.htm#database_access), which can connect to MySQL, MariaDB, SQLLite, PostgreSQL, Oracle, ODBC, Firebird, Interbase, Db2, Informix and SQL Server databases.
* There is a 2-D [graphic library](https://seed7.net/manual/graphic.htm) which uses X11 under Linux, GDI under Windows and JavaScript in the Browser.
* Parser and interpreter are part of the run-time library.

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
