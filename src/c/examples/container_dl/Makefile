ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif

FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INSTALL_DIR}/include -I ../mycomponent/ -L ${IOT_INSTALL_DIR}/lib -Wl,-rpath=${IOT_INSTALL_DIR}/lib

.PHONY : all
all : container_dl

.PHONY : clean
clean :
	rm -f container_dl *.o libmycomponent.so

mycomponent.o : ../mycomponent/mycomponent.c
	gcc ${FLAGS} -c -fpic ../mycomponent/mycomponent.c

libmycomponent.so : mycomponent.o
	gcc -shared -o libmycomponent.so mycomponent.o

container_dl : libmycomponent.so
	gcc ${FLAGS} container_dl.c -liot -o container_dl



