CFLAGS+=-std=c99 -Wextra
LDFLAGS+=
CC=gcc
LD=gcc
AR=ar rcu
RANLIB=ranlib

CFLAGS+=-g -O0
#CFLAGS+=-O3

SOURCES=plcdd.c plcdd_window.c plcdd_display.c plcdd_cmd.c
OBJECTS=$(patsubst %.c,%.o,${SOURCES})

all:		plcdd

clean:
		rm -vf depends.inc ${OBJECTS} plcdd

plcdd:		${OBJECTS}
		${LD} $^ ${LDFLAGS} -o $@

%.o:		%.c
		${CC} ${CFLAGS} -c $< -o $@

depends.inc:	${SOURCES}
		${CC} ${CFLAGS} -MM $^ > $@

.PHONY:		all clean depends.inc

include depends.inc

