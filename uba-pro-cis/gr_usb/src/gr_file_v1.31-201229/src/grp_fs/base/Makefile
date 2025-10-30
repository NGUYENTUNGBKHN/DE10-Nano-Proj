#
# Make file for GR-FILE
#

GRP_FILE_LIB = ../../../lib/grp_file.a
CFLAGS       = -O -Wall -I. -I../include -I../../mdep_xxx/include

SRC =   fat.c \
	grp_fat_format.c \
	grp_fs.c \
	grp_fs_cfg.c \
	grp_fs_conv_lib.c \
	grp_fs_dev_io_if.c \
	grp_fs_error.c \
	grp_fs_get_cwd_lib.c \
	grp_fs_get_disk_part.c \
	grp_fs_io_disk_part.c \
	grp_fs_readdir.c \
	grp_fs_set_disk_part.c

#
#	Sample code files
#
#	grp_fat_format_sd.c \
#	grp_fs_dev_io_ram.c \
#	grp_fs_proc_event.c \
#

OBJ =   fat.o \
	grp_fat_format.o \
	grp_fs.o \
	grp_fs_cfg.o \
	grp_fs_conv_lib.o \
	grp_fs_dev_io_if.o \
	grp_fs_error.o \
	grp_fs_get_cwd_lib.o \
	grp_fs_get_disk_part.o \
	grp_fs_io_disk_part.o \
	grp_fs_readdir.o \
	grp_fs_set_disk_part.o

#
#	Sample code files
#
#	grp_fat_format_sd.o \
#	grp_fs_dev_io_ram.o \
#	grp_fs_proc_event.o \
#

CFILES = ${SRC}

all: ${GRP_FILE_LIB}

${GRP_FILE_LIB}:: ${OBJ}
	ar rv ${GRP_FILE_LIB} $?
	ranlib ${GRP_FILE_LIB}

.c.o:
	${CC} -c ${CFLAGS} ${CFILES}

save:
	tar cvzf grp_file.tgz *.c *.h ../include/*.h Makefile

clean:
	rm -f *.o

depend:
	${CC} -MD -c ${CFLAGS} ${CFILES}

