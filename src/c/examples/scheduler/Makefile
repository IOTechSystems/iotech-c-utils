ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot/1.3
endif
IOT_INCLUDE_DIR=${IOT_INSTALL_DIR}/include
IOT_LIB_DIR=${IOT_INSTALL_DIR}/lib

CC := gcc
FLAGS := -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INCLUDE_DIR} -L ${IOT_LIB_DIR} -Wl,-rpath=${IOT_LIB_DIR}

.PHONY : all
all : scheduler

.PHONY : clean
clean :
	rm -f scheduler *.o

scheduler : scheduler.c
	${CC} ${FLAGS} scheduler.c -liot -o scheduler