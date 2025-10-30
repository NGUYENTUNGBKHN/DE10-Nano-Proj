#
# Make file for GRP_TIME_LIB
#
GRP_LIB = ../../../../lib/grp_lib.a
SRC = grp_time_lib.c
OBJ = grp_time_lib.o

CFLAGS = -O -Wall -I../../include

all: ${GRP_LIB}

${GRP_LIB}:: grp_time_lib.o
	ar rv ${GRP_LIB} grp_time_lib.o
	ranlib ${GRP_LIB}

.c.o:
	${CC} -c ${CFLAGS} $*.c

save:
	tar cvzf grp_time_lib.tgz *.c ../../include/grp_time_lib.h Makefile

clean:
	rm -f *.o

depend:
	${CC} -MD -c ${CFLAGS} ${SRC}

grp_time_lib.o: grp_time_lib.c \
	../../include/grp_time_lib.h ../../include/grp_types.h
