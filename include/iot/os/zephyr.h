//
// Copyright (c) 2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_OS_ZEPHYR_H_
#define _IOT_OS_ZEPHYR_H_

/**
 * @file
 * @brief IOTech Zephyr API
 */

#include <zephyr.h>

/**
 * @brief Duplicate the specified string
 *
 * @param s  String to duplicate
 * @return   Pointer to the duplicated string
 */
extern char * iot_strdup (const char * s);

/**
 * @brief Used to extract token from a specified string
 *
 * @param str     String containing token
 * @param delim   Deliminator
 * @param saveptr Points to character after the specified deliminator (if present in string), else points to NULL
 * @return        Pointer to string containing extracted token
 */
extern char * iot_ctok_r (char *str, const char delim, char **saveptr);

/** Function-like macro - Replace any occurrence of strdup() with iot_strdup() */
#define strdup(s) iot_strdup (s)

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

/** Default stack size */
#define IOT_ZEPHYR_STACK_SIZE 4096
/** Maximum number of threads */
#define IOT_ZEPHYR_MAX_THREADS 4

#endif
