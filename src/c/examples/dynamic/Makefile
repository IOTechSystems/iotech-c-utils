ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot/1.3
endif
IOT_INCLUDE_DIR=${IOT_INSTALL_DIR}/include
IOT_LIB_DIR=${IOT_INSTALL_DIR}/lib

CC := gcc
FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INCLUDE_DIR} -I ../mycomponent/ -L ${IOT_LIB_DIR} -Wl,-rpath=${IOT_LIB_DIR}

.PHONY : all
all : dynamic

.PHONY : clean
clean :
	rm -f dynamic *.o libmycomponent.so

mycomponent.o : ../mycomponent/mycomponent.c
	${CC} ${FLAGS} -c -fpic ../mycomponent/mycomponent.c

libmycomponent.so : mycomponent.o
	${CC} -shared -o libmycomponent.so mycomponent.o

dynamic : dynamic.c libmycomponent.so
	${CC} ${FLAGS} dynamic.c -liot -o dynamic
