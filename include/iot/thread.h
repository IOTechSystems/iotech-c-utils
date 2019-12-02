//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _IOT_THREAD_H_
#define _IOT_THREAD_H_

#include "iot/os.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_THREAD_NO_AFFINITY -1
#define IOT_THREAD_NO_PRIORITY -1

typedef void * (*iot_thread_fn_t) (void * arg);

extern int iot_thread_create (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity);
extern int iot_thread_current_get_priority (void);
extern bool iot_thread_current_set_priority (int priority);
extern int iot_thread_get_priority (pthread_t thread);
extern bool iot_thread_set_priority (pthread_t thread, int priority);
extern bool iot_thread_priority_valid (int priority);

extern void iot_mutex_init (pthread_mutex_t * mutex);


#ifdef __cplusplus
}
#endif
#endif
