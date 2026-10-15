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
  // Declare the C11 stdatomic.h names in the global namespace (as the C++23 stdatomic.h header does), so headers
  // using C atomics can be compiled as C++, without bringing the rest of namespace std into scope
  #include <atomic>
  #ifndef _Atomic
  #define _Atomic(T) std::atomic<T>
  #endif
  using std::memory_order;
  using std::memory_order_relaxed;
  using std::memory_order_consume;
  using std::memory_order_acquire;
  using std::memory_order_release;
  using std::memory_order_acq_rel;
  using std::memory_order_seq_cst;
  using std::atomic_flag;
  using std::atomic_bool;
  using std::atomic_char;
  using std::atomic_schar;
  using std::atomic_uchar;
  using std::atomic_short;
  using std::atomic_ushort;
  using std::atomic_int;
  using std::atomic_uint;
  using std::atomic_long;
  using std::atomic_ulong;
  using std::atomic_llong;
  using std::atomic_ullong;
  using std::atomic_char16_t;
  using std::atomic_char32_t;
  using std::atomic_wchar_t;
  using std::atomic_int8_t;
  using std::atomic_uint8_t;
  using std::atomic_int16_t;
  using std::atomic_uint16_t;
  using std::atomic_int32_t;
  using std::atomic_uint32_t;
  using std::atomic_int64_t;
  using std::atomic_uint64_t;
  using std::atomic_int_least8_t;
  using std::atomic_uint_least8_t;
  using std::atomic_int_least16_t;
  using std::atomic_uint_least16_t;
  using std::atomic_int_least32_t;
  using std::atomic_uint_least32_t;
  using std::atomic_int_least64_t;
  using std::atomic_uint_least64_t;
  using std::atomic_int_fast8_t;
  using std::atomic_uint_fast8_t;
  using std::atomic_int_fast16_t;
  using std::atomic_uint_fast16_t;
  using std::atomic_int_fast32_t;
  using std::atomic_uint_fast32_t;
  using std::atomic_int_fast64_t;
  using std::atomic_uint_fast64_t;
  using std::atomic_intptr_t;
  using std::atomic_uintptr_t;
  using std::atomic_size_t;
  using std::atomic_ptrdiff_t;
  using std::atomic_intmax_t;
  using std::atomic_uintmax_t;
  using std::atomic_is_lock_free;
  using std::atomic_init;
  using std::atomic_load;
  using std::atomic_load_explicit;
  using std::atomic_store;
  using std::atomic_store_explicit;
  using std::atomic_exchange;
  using std::atomic_exchange_explicit;
  using std::atomic_compare_exchange_strong;
  using std::atomic_compare_exchange_strong_explicit;
  using std::atomic_compare_exchange_weak;
  using std::atomic_compare_exchange_weak_explicit;
  using std::atomic_fetch_add;
  using std::atomic_fetch_add_explicit;
  using std::atomic_fetch_sub;
  using std::atomic_fetch_sub_explicit;
  using std::atomic_fetch_and;
  using std::atomic_fetch_and_explicit;
  using std::atomic_fetch_or;
  using std::atomic_fetch_or_explicit;
  using std::atomic_fetch_xor;
  using std::atomic_fetch_xor_explicit;
  using std::atomic_flag_test_and_set;
  using std::atomic_flag_test_and_set_explicit;
  using std::atomic_flag_clear;
  using std::atomic_flag_clear_explicit;
  using std::atomic_thread_fence;
  using std::atomic_signal_fence;
#else
  #include <stdatomic.h>
#endif

#endif
