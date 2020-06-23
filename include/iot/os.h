//
// Copyright (c) 2018-2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_OS_H_
#define _IOT_OS_H_

/**
 * @file
 * @brief IOTech OS API
 */

#ifdef __ZEPHYR__
#include "iot/os/zephyr.h"
#elif __APPLE__
#include "iot/os/macos.h"
#else
#include "iot/os/linux.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#ifdef __cplusplus
  #include <atomic>
  #define _Atomic(T) atomic<T>
  using namespace std;
#else
  #include <stdatomic.h>
#endif

#endif
