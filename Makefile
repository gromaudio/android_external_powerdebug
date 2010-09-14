BINDIR=/usr/bin
MANDIR=/usr/share/man/man8

WARNFLAGS=-Wall -Wshadow -W -Wformat -Wimplicit-function-declaration -Wimplicit-int
CFLAGS?=-O1 -g ${WARNFLAGS}
CC?=gcc

OBJS = powerdebug.o output.o sensor.o display.o

powerdebug.8.gz: powerdebug.8
	gzip -c $< > $@

powerdebug: $(OBJS) powerdebug.h
	$(CC) ${CFLAGS} $(OBJS) -lncurses -o powerdebug

install: powerdebug powerdebug.8.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp powerdebug ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp powerdebug.8.gz ${DESTDIR}${MANDIR}

All: install

clean:
	rm -f powerdebug *.o powerdebug.8.gz
