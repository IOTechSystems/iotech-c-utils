#ifndef _IOT_OS_ZEPHYR_H_
#define _IOT_OS_ZEPHYR_H_

#include <zephyr.h>

extern char * strdup (const char *s);

#ifndef CONFIG_NET_TCP
#error Zephyr CONFIG_NET_TCP not set
#endif
#ifndef CONFIG_NET_SOCKETS
#error Zephyr CONFIG_NET_SOCKETS not set
#endif
#ifndef CONFIG_NET_IPV4
#error Zephyr CONFIG_NET_IPV4 not set
#endif
#ifndef CONFIG_POSIX_API
#error Zephyr CONFIG_POSIX_API not set
#endif
#ifndef CONFIG_PTHREAD_IPC
#error Zephyr CONFIG_PTHREAD_IPC not set
#endif

#define IOT_ZEPHYR_STACK_SIZE 4096
#define IOT_ZEPHYR_MAX_THREADS 5

#endif
