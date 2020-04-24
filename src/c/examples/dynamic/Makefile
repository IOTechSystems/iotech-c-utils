ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif

FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INSTALL_DIR}/include -I ../mycomponent/ -L ${IOT_INSTALL_DIR}/lib -Wl,-rpath=${IOT_INSTALL_DIR}/lib

.PHONY : all
all : dynamic

.PHONY : clean
clean :
	rm -f dynamic *.o libmycomponent.so

mycomponent.o : ../mycomponent/mycomponent.c
	gcc ${FLAGS} -c -fpic ../mycomponent/mycomponent.c

libmycomponent.so : mycomponent.o
	gcc -shared -o libmycomponent.so mycomponent.o

dynamic : dynamic.c libmycomponent.so
	gcc ${FLAGS} dynamic.c -liot -o dynamic
