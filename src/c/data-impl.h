//
// Copyright (c) 2023 IOTech Ltd
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_DATA_IMPL_H_
#define _IOT_DATA_IMPL_H_

#include "iot/data.h"

typedef struct iot_block_t
{
  struct iot_block_t * next;
} iot_block_t;

typedef union iot_data_base_t
{
  iot_block_t * next;
  iot_data_t * meta;
} iot_data_base_t;

struct iot_data_t
{
  iot_data_base_t base;
  _Atomic uint32_t refs;
  uint32_t hash;
  iot_data_type_t type;
  iot_data_type_t element_type;
  iot_data_type_t key_type;
  bool release : 1;
  bool release_block : 1;
  bool heap : 1;
  bool constant : 1;
  bool composed : 1;
  bool rehash : 1;
};

extern iot_data_static_t iot_data_order;

#endif
