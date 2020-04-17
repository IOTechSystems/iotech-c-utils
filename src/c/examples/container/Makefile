ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif

FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INSTALL_DIR}/include -I ../mycomponent/ -L ${IOT_INSTALL_DIR}/lib -Wl,-rpath=${IOT_INSTALL_DIR}/lib

.PHONY : all
all : container

.PHONY : clean
clean :
	rm -f container *.o

mycomponent.o : ../mycomponent/mycomponent.c
	gcc ${FLAGS} -c ../mycomponent/mycomponent.c

container : mycomponent.o
	gcc ${FLAGS} container.c -liot mycomponent.o -o container
