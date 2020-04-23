ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif
ifeq ($(IOT_INCLUDE_DIR),)
  IOT_INCLUDE_DIR=${IOT_INSTALL_DIR}/include
endif
ifeq ($(IOT_LIB_DIR),)
  IOT_LIB_DIR=${IOT_INSTALL_DIR}/lib
endif

FLAGS:= -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INCLUDE_DIR} -L ${IOT_LIB_DIR} -Wl,-rpath=${IOT_LIB_DIR}

.PHONY : all
all : scheduler

.PHONY : clean
clean :
	rm -f scheduler *.o

scheduler : scheduler.c
	gcc ${FLAGS} scheduler.c -liot -o scheduler