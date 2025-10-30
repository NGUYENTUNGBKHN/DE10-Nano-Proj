#
# Make file for GRP_STDIO
#
GRP_FILE_LIB = ../../../../lib/grp_file.a
SRC = grp_stdio_default_io.c grp_stdio_fclose.c grp_stdio_fflush.c \
	grp_stdio_fill.c grp_stdio_fopen.c grp_stdio_fprintf.c \
	grp_stdio_fread.c grp_stdio_fseek.c grp_stdio_ftell.c \
	grp_stdio_fwrite.c grp_stdio_getc.c grp_stdio_putc.c grp_stdio_gets.c \
	grp_stdio_vfprintf.c grp_stdio_fseek4G.c grp_stdio_ftell4G.c
OBJ = grp_stdio_default_io.o grp_stdio_fclose.o grp_stdio_fflush.o \
	grp_stdio_fill.o grp_stdio_fopen.o grp_stdio_fprintf.o \
	grp_stdio_fread.o grp_stdio_fseek.o grp_stdio_ftell.o \
	grp_stdio_fwrite.o grp_stdio_getc.o grp_stdio_putc.o grp_stdio_gets.o \
	grp_stdio_vfprintf.o grp_stdio_fseek4G.o grp_stdio_ftell4G.o

CFLAGS = -O -Wall -I../../include -I../../../mdep_xxx/include

all: ${GRP_FILE_LIB}

${GRP_FILE_LIB}:: ${OBJ}
	ar -r ${GRP_FILE_LIB} ${OBJ}
	ranlib ${GRP_FILE_LIB}

.c.o:
	${CC} -c ${CFLAGS} $*.c

save:
	tar cvzf grp_stdio.tgz *.c ../../include/grp_stdio.h Makefile

clean:
	rm -f *.o

depend:
	${CC} -MD -c ${CFLAGS} ${SRC}

grp_stdio_default_io.o: grp_stdio_default_io.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fclose.o: grp_stdio_fclose.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fflush.o: grp_stdio_fflush.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fill.o: grp_stdio_fill.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fopen.o: grp_stdio_fopen.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fprintf.o: grp_stdio_fprintf.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fread.o: grp_stdio_fread.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fseek.o: grp_stdio_fseek.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_ftell.o: grp_stdio_ftell.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fwrite.o: grp_stdio_fwrite.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_getc.o: grp_stdio_getc.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_gets.o: grp_stdio_gets.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_putc.o: grp_stdio_putc.c \
	../../include/grp_stdio.h    ../../include/grp_fs_conv.h \
	../../include/grp_fs_if.h    ../../include/grp_types.h \
	../../include/grp_fs_param.h ../../include/grp_mem.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_vfprintf.o: grp_stdio_vfprintf.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_fseek4G.o: grp_stdio_fseek4G.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
grp_stdio_ftell4G.o: grp_stdio_ftell4G.c \
	../../include/grp_stdio.h    ../../include/grp_mem.h \
	../../include/grp_fs_conv.h  ../../include/grp_fs_if.h \
	../../include/grp_types.h    ../../include/grp_fs_param.h \
	../../../mdep_xxx/include/grp_fs_mdep_types.h
