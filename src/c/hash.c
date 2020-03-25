/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/hash.h"

/* Version 2 of the Bernstein djb2 hash function. */

uint32_t iot_hash (const char * str)
{
  uint32_t hash = 538u;
  while (*str)
  {
    hash = ((hash << 5u) + hash) ^ (uint8_t) *str++;
  }
  return hash;
}
