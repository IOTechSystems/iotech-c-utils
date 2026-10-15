//
// Copyright (c) 2026 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_THREAD_IMPL_H_
#define _IOT_THREAD_IMPL_H_

#include "iot/thread.h"

// As iot_thread_create, but the thread is created joinable, so must be joined (or detached) by the caller
extern bool iot_thread_create_joinable (pthread_t * tid, iot_thread_fn_t func, void * arg, int priority, int affinity, iot_logger_t * logger);

#endif
