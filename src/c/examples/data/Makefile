ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif
IOT_INCLUDE_DIR=${IOT_INSTALL_DIR}/include
IOT_LIB_DIR=${IOT_INSTALL_DIR}/lib

CC := gcc
FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INCLUDE_DIR} -L ${IOT_LIB_DIR} -Wl,-rpath=${IOT_LIB_DIR}

.PHONY : all
all : data

.PHONY : clean
clean :
	rm -f data *.o

data : data.c
	${CC} ${FLAGS} data.c -liot -o data