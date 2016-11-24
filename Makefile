CFLAGS+=-std=c99 -Wextra
LDFLAGS+=
CC=gcc
LD=gcc
AR=ar rcu
RANLIB=ranlib

CFLAGS+=-g -O0
#CFLAGS+=-O3

SOURCES_LIB=plcdd_progress.c plcdd_window.c plcdd_display.c plcdd_cmd.c
OBJECTS_LIB=$(patsubst %.c,%.o,${SOURCES_LIB})

SOURCES_PLCDD=plcdd.c
OBJECTS_PLCDD=$(patsubst %.c,%.o,${SOURCES_PLCDD})

all:		plcdd

clean:
		rm -vf depends.inc ${OBJECTS_LIB} ${OBJECTS_PLCDD} plcdd

plcdd:		${OBJECTS_LIB} ${OBJECTS_PLCDD}
		${LD} $^ ${LDFLAGS} -o $@

%.o:		%.c
		${CC} ${CFLAGS} -c $< -o $@

depends.inc:	${SOURCES_LIB} ${SOURCES_PLCDD}
		${CC} ${CFLAGS} -MM $^ > $@

.PHONY:		all clean depends.inc

include depends.inc

