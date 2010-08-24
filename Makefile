WARNFLAGS=-Wall -Wshadow -W -Wformat -Wimplicit-function-declaration -Wimplicit-int
CFLAGS?=-O1 -g ${WARNFLAGS} -lncurses
CC?=gcc

OBJS = powerdebug.o output.o sensor.o display.o


powerdebug: $(OBJS) powerdebug.h
	$(CC) ${CFLAGS} $(OBJS) -o powerdebug

All: powerdebug

clean:
	rm -f powerdebug *.o
