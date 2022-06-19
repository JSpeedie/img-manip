# jpegborder - A C program to add a border, or frame to a jpeg
# © 2022 Julian Speedie

PREFIX = /usr/local
MANPREFIX = /usr/share/man
# gcc flags for includes
# -ljpeg to link the libjpeg library
INCS = -I. -I/usr/include -ljpeg
LIBS = -L/usr/lib
# Flags
CFLAGS = -Wall
# Compiler and linker
# CC = gcc -ggdb
CC = gcc -g

SRC = jpegborder.c
DEP =
OBJ = ${SRC:.c=.o}
BIN = ${SRC:.c=}
# MAN = $(SRC:.c=.1.gz)


# "compile" first because we want "make" to just compile the program, and the
# default target is always the the first one that doesn't begin with "."
compile: jpegborder.c $(DEP)
	$(CC) $(CFLAGS) jpegborder.c $(DEP) $(INCS) $(LIBS) -o jpegborder


all: compile install


install: $(BIN)
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f $(BIN) ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/jpegborder

# @mkdir -p $(MANPREFIX)/man1/
# @cp -f $(MAN) $(MANPREFIX)/man1/
#

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@for i in $(BIN); do \
		rm -f ${DESTDIR}${PREFIX}/bin/$$i; \
	done
	# @for page in $(MAN); do \
	# 	rm -f $(MANPREFIX)/man1/$$page; \
	# done
	#

# Needs to be fixed
clean:
	@echo cleaning
	@rm -f jpegborder ${OBJ} jpegborder-${VERSION}.tar.gz
