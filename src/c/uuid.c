//
// Copyright (c) 2021-2026 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef __APPLE__
#include <sys/random.h>
#endif
#include "iot/iot.h"
#include "iot/uuid.h"

#define RAND_READ_DELAY 1000u
static const char  * hexdigits = "0123456789abcdef";

// UUIDs are read from the operating system cryptographically secure random number generator, so cannot be
// predicted from previously generated UUIDs, and are not repeated by processes forked from the same parent

static void uuid_random (uint8_t * buf, size_t len)
{
#ifdef __APPLE__
  arc4random_buf (buf, len);
#else
  while (len)
  {
    ssize_t ret = getrandom (buf, len, 0);
    if (ret > 0)
    {
      buf += ret;
      len -= (size_t) ret;
    }
    else if (errno != EINTR)
    {
      iot_wait_usecs (RAND_READ_DELAY);
    }
  }
#endif
}

void iot_uuid_generate (iot_uuid_t out)
{
  assert (out);
  uuid_random (out, sizeof (iot_uuid_t));
  out[6] = (uint8_t) ((out[6] & 0xf) | 0x40); // Flag as randomly generated uuid
  out[8] = (uint8_t) ((out[8] & 0x3f) | 0x80);
}

void iot_uuid_unparse (const iot_uuid_t uuid, char * out)
{
  for (unsigned i = 0; i < 16; i++)
  {
    if (i == 4 || i == 6 || i == 8 || i == 10)
    {
      *out++ = '-';
    }
    size_t tmp = uuid[i];
    *out++ = hexdigits[tmp >> 4];
    *out++ = hexdigits[tmp & 15];
  }
  *out = '\0';
}
