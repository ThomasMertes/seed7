# CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall -Wstrict-prototypes -Winline -Wconversion -Wshadow -Wpointer-arith
# CFLAGS = -O2 -funroll-loops -Wall -pg
LFLAGS = -O2
LIBS = /usr/X11/lib/libX11.a -lncurses -lm
CC = gcc

SCREEN_OBJ = scr_inf.o kbd_inf.o trm_inf.o
SCREEN_SRC = scr_inf.c kbd_inf.c trm_inf.c
# SCREEN_OBJ = scr_inf.o kbd_inf.o trm_cap.o
# SCREEN_SRC = scr_inf.c kbd_inf.c trm_cap.c
# SCREEN_OBJ = scr_cur.o
# SCREEN_SRC = scr_cur.c
# SCREEN_OBJ = scr_cap.o
# SCREEN_SRC = scr_cap.c
# SCREEN_OBJ = scr_tcp.o
# SCREEN_SRC = scr_tcp.c

MOBJ1 = hi.o option.o primitiv.o
LOBJ1 = actlib.o arrlib.o chrlib.o drwlib.o enulib.o fillib.o fltlib.o
LOBJ2 = hallib.o intlib.o kbdlib.o lamlib.o loclib.o lstlib.o modlib.o
LOBJ3 = prclib.o reflib.o rfllib.o scrlib.o strlib.o timlib.o typlib.o unxlib.o
EOBJ1 = interpr.o create.o doany.o
ROBJ1 = random.o memory.o runfile.o runerr.o runlist.o runobj.o
AOBJ1 = analyze.o syntax.o param.o parser.o import.o name.o proc.o meta.o
AOBJ2 = list.o expr.o atom.o object.o select.o scanner.o literal.o
AOBJ3 = numlit.o findid.o error.o infile.o symbol.o info.o stat.o fatal.o match.o
GOBJ1 = sysvar.o trace.o action.o listutil.o objutil.o block.o striutil.o
GOBJ2 = ident.o chclass.o flist.o heap.o util.o
DOBJ1 = $(SCREEN_OBJ) tim_unx.o drw_x11.o
OBJ = $(MOBJ1) $(LOBJ1) $(LOBJ2) $(LOBJ3) $(EOBJ1) $(ROBJ1) $(AOBJ1) $(AOBJ2) $(AOBJ3) $(GOBJ1) $(GOBJ2) $(DOBJ1)

MSRC1 = hi.c option.c primitiv.c
LSRC1 = actlib.c arrlib.c chrlib.c drwlib.c enulib.c fillib.c fltlib.c
LSRC2 = hallib.c intlib.c kbdlib.c lamlib.c loclib.c lstlib.c modlib.c
LSRC3 = prclib.c reflib.c rfllib.c scrlib.c strlib.c timlib.c typlib.c unxlib.c
ESRC1 = interpr.c create.c doany.c
RSRC1 = random.c memory.c runfile.c runerr.c runlist.c runobj.c
ASRC1 = analyze.c syntax.c param.c parser.c import.c name.c proc.c meta.c
ASRC2 = list.c expr.c atom.c object.c select.c scanner.c literal.c
ASRC3 = numlit.c findid.c error.c infile.c symbol.c info.c stat.c fatal.c
GSRC1 = sysvar.c trace.c action.c listutil.c objutil.c block.c striutil.c
GSRC2 = ident.c chclass.c flist.c heap.c util.c
DSRC1 = $(SCREEN_SRC) tim_unx.c drw_x11.c
SRC = $(MSRC1) $(LSRC1) $(LSRC2) $(LSRC3) $(ESRC1) $(RSRC1) $(ASRC1) $(ASRC2) $(ASRC3) $(GSRC1) $(GSRC2) $(DSRC1)

#hi: hallib.a
#	$(CC) $(LFLAGS) hallib.a $(LIBS) -o /usr/local/bin/hi

hi: $(OBJ) depend
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o hi
	hi level
	cp hi /usr/local/bin/hi

hi.gp: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o /usr/local/bin/hi.gp
	hi level

clear:
	rm *.o
	rm version.h

dep: depend

strip:
	strip /usr/local/bin/hi

version.h:
	echo "#define ANSI_C" > version.h
	echo "#define USE_DIRENT" >> version.h
	echo "#define PATH_DELIMITER '/'" >> version.h
	echo "#define CATCH_SIGNALS" >> version.h
	echo "#define USE_MMAP" >> version.h
	echo "#define INCL_NCURSES_TERM" >> version.h

hi.o: hi.c depend
	$(CC) $(CFLAGS) -c hi.c

depend: $(SRC) version.h
	$(CC) -M $(SRC) > depend

level.h:
	hi level

hallib: $(MOBJ1) $(LOBJ1) $(LOBJ2) $(LOBJ3) $(EOBJ1) $(ROBJ1) $(AOBJ1) $(AOBJ2) $(AOBJ3) $(GOBJ1) $(GOBJ2) $(DOBJ1)
	ar r hallib.a $(MOBJ1) $(LOBJ1) $(LOBJ2) $(LOBJ3) $(EOBJ1) $(ROBJ1) $(AOBJ1) $(AOBJ2) $(AOBJ3) $(GOBJ1) $(GOBJ2) $(DOBJ1)

wc: $(SRC)
	wc $(GSRC1) $(GSRC2)
	wc $(ASRC1) $(ASRC2) $(ASRC3)
	wc $(RSRC1)
	wc $(ESRC1)
	wc $(LSRC1) $(LSRC2) $(LSRC3)
	wc $(DSRC1)
	wc $(MSRC1)
	wc $(SRC)

lint: $(SRC)
	lint -p $(SRC) $(LIBS)

lint2: $(SRC)
	lint -Zn2048 $(SRC) $(LIBS)

ifeq (depend,$(wildcard depend))
include depend
endif
