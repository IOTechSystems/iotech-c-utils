//
// Copyright (c) 2023 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_DATA_IMPL_H_
#define _IOT_DATA_IMPL_H_

#include "iot/data.h"
#include <stdalign.h>

typedef struct iot_block_t
{
  struct iot_block_t * next;
} iot_block_t;

typedef struct iot_block_batch_t // Header of a batch of blocks in the shared block cache, held in the first block
{
  iot_block_t block;                 // Links blocks within the batch
  struct iot_block_batch_t * batch;  // Next batch in the shared cache
  uint32_t count;                    // Number of blocks in the batch
} iot_block_batch_t;

typedef struct iot_cache_head_t
{
  alignas(2 * sizeof(void*)) iot_block_t * node; // CMPXCHG16B can crash if struct is not 16 byte aligned on 64bit systems
  uintptr_t tag;
} iot_cache_head_t;

// Rehash flag carried in top bit of refs, atomic as hash calculated lazily by iot_data_hash on (possibly shared) data.
// Held in refs rather than a separate field so that iot_data_t fits in 16 bytes on 32 bit systems.

#define IOT_DATA_REHASH 0x80000000u
#define IOT_DATA_REFS_MASK (~IOT_DATA_REHASH)

typedef union iot_data_base_t
{
  iot_block_t * next;
  iot_data_t * meta;
} iot_data_base_t;

struct iot_data_t
{
  iot_data_base_t base;
  _Atomic uint32_t refs; // Reference count, top bit (IOT_DATA_REHASH) records whether hash needs recalculating
  uint32_t hash;
  iot_data_type_t type;
  iot_data_type_t element_type;
  iot_data_type_t key_type;
  bool release : 1;
  bool release_block : 1;
  bool heap : 1;
  bool constant : 1;
  bool composed : 1;
  bool tag1 : 1;
  bool tag2 : 1;
  bool tag3 : 1;
};

typedef struct iot_string_holder_t
{
  char * str;
  size_t size;
  size_t free;
} iot_string_holder_t;

void iot_data_holder_realloc (iot_string_holder_t * holder, size_t required);

void iot_data_strcat_escape (iot_string_holder_t * holder, const char * add, bool escape);

extern iot_data_static_t iot_data_order;

#endif
