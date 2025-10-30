#
# Make file for GRP_MEM
#
GRP_LIB = ../../../../lib/grp_lib.a
SRC = grp_mem_vl_pool.c
OBJ = grp_mem_vl_pool.o

CFLAGS = -O -Wall -I../../include -I../../../mdep_xxx/include

all: ${GRP_LIB}

${GRP_LIB}:: grp_mem_vl_pool.o
	ar rv ${GRP_LIB} grp_mem_vl_pool.o
	ranlib ${GRP_LIB}

.c.o:
	${CC} -c ${CFLAGS} $*.c

save:
	tar cvzf grp_mem.tgz *.c *.h Makefile

clean:
	rm -f *.o

depend:
	${CC} -MD -c ${CFLAGS} ${SRC}

grp_mem_vl_pool.o: grp_mem_vl_pool.c \
	../../include/grp_types.h ../../include/grp_mem.h \
	../../include/grp_sem.h ../../../mdep_xxx/include/grp_mdep_sem.h \
	grp_mem_vl_pool.h
