//
// Copyright (c) 2018 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_OS_H_
#define _IOT_OS_H_

#ifdef __ZEPHYR__
#include "iot/os/zephyr.h"
#else
#include "iot/os/linux.h"
#ifndef _REDHAT_SEAWOLF_
#ifndef __LIBMUSL__
#define IOT_HAS_CPU_AFFINITY
#define IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
#endif
#define IOT_HAS_PR_GET_NAME
#endif
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
