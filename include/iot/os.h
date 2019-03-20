//
// Copyright (c) 2018
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_OS_H_
#define _IOT_OS_H_

#ifdef __ZEPHYR__
#include "iot/os/zephyr.h"
#else
#include "iot/os/linux.h"
#define iot_strdup(s) strdup(s)
#endif

#include <pthread.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>

extern char * iot_ctok_r (char *str, const char delim, char **saveptr);

#endif
