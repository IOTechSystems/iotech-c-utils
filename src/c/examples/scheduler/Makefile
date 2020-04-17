ifeq ($(IOT_INSTALL_DIR),)
  IOT_INSTALL_DIR=/opt/iotech/iot
endif

FLAGS:= -std=c11 -pthread -D_GNU_SOURCE ${FLAGS} -I ${IOT_INSTALL_DIR}/include -L ${IOT_INSTALL_DIR}/lib -Wl,-rpath=${IOT_INSTALL_DIR}/lib

.PHONY : all
all : scheduler

.PHONY : clean
clean :
	rm -f scheduler *.o

scheduler : scheduler.c
	gcc ${FLAGS} scheduler.c -liot -o scheduler