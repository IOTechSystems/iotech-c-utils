//
// Copyright (c) 2020 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_OS_LINUX_H_
#define _IOT_OS_LINUX_H_

/**
 * @file
 * @brief IOTech Linux API
 */

#include <sched.h>
#include <unistd.h>

#define IOT_HAS_CPU_AFFINITY
#define IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
#define IOT_HAS_PRCTL
#define IOT_HAS_FILE
#define IOT_HAS_UUID

#ifdef _REDHAT_SEAWOLF_
#undef IOT_HAS_CPU_AFFINITY
#undef IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
#undef IOT_HAS_PRCTL
#endif

#ifdef _ALPINE_
#undef IOT_HAS_CPU_AFFINITY
#undef IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
#endif

#ifdef _AZURESPHERE_
#undef IOT_HAS_CPU_AFFINITY
#undef IOT_HAS_PTHREAD_MUTEXATTR_SETPROTOCOL
#undef IOT_HAS_PRCTL
#undef IOT_HAS_UUID
#endif

#endif
