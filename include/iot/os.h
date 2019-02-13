//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
/** @file
 *  @brief Standard C99 includes
 */
#ifndef _IOT_OS_H_
#define _IOT_OS_H_

#ifdef __ZEPHYR__
#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include "iot/strdup.h"
#else
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#endif

#endif
