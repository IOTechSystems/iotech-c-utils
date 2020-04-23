ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif
ifeq ($(IOT_INCLUDE_DIR),)
  IOT_INCLUDE_DIR=${IOT_INSTALL_DIR}/include
endif
ifeq ($(IOT_LIB_DIR),)
  IOT_LIB_DIR=${IOT_INSTALL_DIR}/lib
endif

FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INCLUDE_DIR} -I ../mycomponent/ -L ${IOT_LIB_DIR} -Wl,-rpath=${IOT_LIB_DIR}

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



