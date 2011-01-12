BINDIR=/usr/sbin
MANDIR=/usr/share/man/man8

WARNFLAGS=-Wall -Wshadow -W -Wformat -Wimplicit-function-declaration -Wimplicit-int
CFLAGS?=-O1 -g ${WARNFLAGS}
CC?=gcc

OBJS = powerdebug.o sensor.o clocks.o regulator.o display.o

default: powerdebug

powerdebug.8.gz: powerdebug.8
	gzip -c $< > $@

powerdebug: $(OBJS) powerdebug.h
	$(CC) ${CFLAGS} $(OBJS) -lncurses -o powerdebug

install: powerdebug powerdebug.8.gz
	install -d ${DESTDIR}${BINDIR} ${DESTDIR}${MANDIR}
	install -m 0755 powerdebug ${DESTDIR}${BINDIR}
	install -m 0644 powerdebug.8.gz ${DESTDIR}${MANDIR}

all: powerdebug powerdebug.8.gz

clean:
	rm -f powerdebug *.o powerdebug.8.gz
