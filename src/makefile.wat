O = obj
CC = WCC386
# CFLAGS = -mf -w6 -d3 -bt=dos
# CFLAGS = -mf -w6 -oxt -d3 -bt=dos
CFLAGS = -mf -w6 -oxt -bt=dos
LFLAGS = option ST=100000 debug all
# CFLAGS = -AL -Ozax -Gr -Gs -Gm -G0 -W4
# LFLAGS = /NOD/E/F/ST:50000
# LFLAGS = /NOD/E/F/ST:51400
# LFLAGS = /NOD/E/F/ST:52100
# LFLAGS = /NOD/E/F/ST:54750
# LFLAGS = /NOD/E/F/ST:55660
# LFLAGS = /NOD/E/F/ST:56608
LIBS = llibce

MOBJS1 = hi.$O option.$O primitiv.$O
LOBJS1 = actlib.$O arrlib.$O chrlib.$O drwlib.$O enulib.$O fillib.$O fltlib.$O
LOBJS2 = hallib.$O intlib.$O kbdlib.$O lamlib.$O loclib.$O lstlib.$O modlib.$O
LOBJS3 = prclib.$O reflib.$O scrlib.$O strlib.$O timlib.$O unxlib.$O
EOBJS1 = interpr.$O create.$O doany.$O
ROBJS1 = random.$O memory.$O runfile.$O runerr.$O runlist.$O runobj.$O
AOBJS1 = analyze.$O syntax.$O param.$O parser.$O import.$O name.$O proc.$O meta.$O
AOBJS2 = list.$O expr.$O atom.$O object.$O select.$O scanner.$O literal.$O
AOBJS3 = numlit.$O findid.$O error.$O infile.$O symbol.$O stat.$O fatal.$O
GOBJS1 = sysvar.$O trace.$O action.$O listutil.$O objutil.$O block.$O
GOBJS2 = ident.$O chclass.$O flist.$O heap.$O util.$O
DOBJS1 = scr_wat.$O tim_dos.$O dir_dos.$O drw_dos.$O
OBJS = $(MOBJS1) $(LOBJS1) $(LOBJS2) $(LOBJS3) $(EOBJS1) $(ROBJS1) $(AOBJS1) $(AOBJS2) $(AOBJS3) $(GOBJS1) $(GOBJS2) $(DOBJS1)

MNAMS1 = hi,option,primitiv
LNAMS1 = actlib,arrlib,chrlib,drwlib,enulib,fillib,fltlib
LNAMS2 = hallib,intlib,kbdlib,lamlib,loclib,lstlib,modlib
LNAMS3 = prclib,reflib,scrlib,strlib,timlib,unxlib
ENAMS1 = interpr,create,doany
RNAMS1 = random,memory,runfile,runerr,runlist,runobj
ANAMS1 = analyze,syntax,param,parser,import,name,proc,meta
ANAMS2 = list,expr,atom,object,select,scanner,literal
ANAMS3 = numlit,findid,error,infile,symbol,stat,fatal
GNAMS1 = sysvar,trace,action,listutil,objutil,block
GNAMS2 = ident,chclass,flist,heap,util
DNAMS1 = scr_wat,tim_dos,dir_dos,drw_dos
NAMS = $(MNAMS1) $(LNAMS1) $(LNAMS2) $(LNAMS3) $(ENAMS1) $(RNAMS1) $(ANAMS1) $(ANAMS2) $(ANAMS3) $(GNAMS1) $(GNAMS2) $(DNAMS1)

hi: ..\bin\hi.exe

..\bin\hi.exe: $(OBJS)
        WLINK $(LFLAGS) file @<<
$(MNAMS1),
$(LNAMS1),
$(LNAMS2),
$(LNAMS3),
$(ENAMS1),
$(RNAMS1),
$(ANAMS1),
$(ANAMS2),
$(ANAMS3),
$(GNAMS1),
$(GNAMS2),
$(DNAMS1)
name ..\bin\whi.exe
<<

hg: $(GOBJS1) $(GOBJS2)
        link $(LFLAGS) @<<
$(GOBJS1)+
$(GOBJS2)
..\bin\hg.exe

$(LIBS)

<<

ha: $(GOBJS1) $(GOBJS2) $(AOBJS1) $(AOBJS2) $(AOBJS3)
        link $(LFLAGS) @<<
$(GOBJS1)+
$(GOBJS2)+
$(AOBJS1)+
$(AOBJS2)+
$(AOBJS3)
..\bin\ha.exe

$(LIBS)

<<

hr: $(GOBJS1) $(GOBJS2) $(ROBJS1)
        link $(LFLAGS) @<<
$(GOBJS1)+
$(GOBJS2)+
$(ROBJS1)
..\bin\hr.exe

$(LIBS)

<<

he: $(GOBJS1) $(GOBJS2) $(ROBJS1) $(EOBJS1)
        link $(LFLAGS) @<<
$(GOBJS1)+
$(GOBJS2)+
$(ROBJS1)+
$(EOBJS1)
..\bin\he.exe

$(LIBS)

<<

hl: $(GOBJS1) $(GOBJS2) $(ROBJS1) $(LOBJS1) $(LOBJS2) $(LOBJS3)
        link $(LFLAGS) @<<
$(GOBJS1)+
$(GOBJS2)+
$(ROBJS1)+
$(LOBJS1)+
$(LOBJS2)+
$(LOBJS3)
..\bin\hl.exe

$(LIBS)

<<

version.h:
        echo #define ANSI_C > version.h
        echo #define DOS_VERSION >> version.h
        echo #define USE_DIRECT >> version.h
        echo #define PATH_DELIMITER '\\' >> version.h

hi.$O: hi.c version.h
        SET INCLUDE=C:\WATCOM\H;
        $(CC) $(CFLAGS) hi.c

.C.OBJ:
        SET INCLUDE=C:\WATCOM\H;
        $(CC) $(CFLAGS) $*.c

glib:
        lib ..\comp\glib $(GOBJS1), NUL, ..\comp\glib
        lib ..\comp\glib $(GOBJS2), NUL, ..\comp\glib

alib:
        lib ..\comp\alib $(AOBJS1), NUL, ..\comp\alib
        lib ..\comp\alib $(AOBJS2), NUL, ..\comp\alib
        lib ..\comp\alib $(AOBJS3), NUL, ..\comp\alib

rlib:
        lib ..\comp\rlib $(ROBJS1), NUL, ..\comp\rlib

elib:
        lib ..\comp\elib $(EOBJS1), NUL, ..\comp\elib

llib:
        lib ..\comp\llib $(LOBJS1), NUL, ..\comp\llib
        lib ..\comp\llib $(LOBJS2), NUL, ..\comp\llib
        lib ..\comp\llib $(LOBJS3), NUL, ..\comp\llib

blib:
        lib ..\comp\blib $(GOBJS1), NUL, ..\comp\blib
        lib ..\comp\blib $(GOBJS2), NUL, ..\comp\blib
        lib ..\comp\blib $(AOBJS1), NUL, ..\comp\blib
        lib ..\comp\blib $(AOBJS2), NUL, ..\comp\blib
        lib ..\comp\blib $(AOBJS3), NUL, ..\comp\blib
        lib ..\comp\blib $(ROBJS1), NUL, ..\comp\blib
        lib ..\comp\blib $(EOBJS1), NUL, ..\comp\blib
        lib ..\comp\blib $(LOBJS1), NUL, ..\comp\blib
        lib ..\comp\blib $(LOBJS2), NUL, ..\comp\blib
        lib ..\comp\blib $(LOBJS3), NUL, ..\comp\blib

clib:
        lib ..\comp\clib chclass.$O, NUL, ..\comp\clib

tim: tim.exe

tim.exe: tim.$O
        link $(LFLAGS) tim.$O,tim.exe,,$(LIBS),
