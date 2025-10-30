#
# Make file for GRP_CHAR
#
GRP_LIB = ../../../../lib/grp_lib.a
SRC = grp_char_sjis_conv.c
OBJ = grp_char_sjis_conv.o

CFLAGS = -O -Wall -I../../include

all: ${GRP_LIB}

${GRP_LIB}:: ${OBJ}
	ar rv ${GRP_LIB} ${OBJ}
	ranlib ${GRP_LIB}

.c.o:
	${CC} -c ${CFLAGS} $*.c

grp_char_sjis_conv.o: grp_char_sjis_conv.c \
		../../include/grp_types.h ../../include/grp_char_conv.h \
		grp_char_sjis_tbl.h
	${CC} -c ${CFLAGS} grp_char_sjis_conv.c

save:
	tar cvzf grp_char.tgz *.c *.h ../../include/grp_char_conv.h Makefile

clean:
	rm -f *.o

depend:
	${CC} -MD -c ${CFLAGS} ${SRC}
