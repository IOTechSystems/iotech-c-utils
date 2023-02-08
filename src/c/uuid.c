//
// Copyright (c) 2021-2022 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include <sys/random.h>
#include "iot/iot.h"
#include "iot/uuid.h"

#define RAND_READ_DELAY 1000u
static const char  * hexdigits = "0123456789abcdef";
static uint64_t uuid_seed[2];
static bool uuid_initialized = false;
static pthread_mutex_t uuid_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint64_t xorshift128plus (uint64_t *s)
{
  uint64_t s1 = s[0];
  const uint64_t s0 = s[1];
  s[0] = s0;
  s1 ^= s1 << 23;
  s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
  return s[1] + s0;
}

static void uuid_init (void)
{
#ifdef __APPLE__
  arc4random_buf (uuid_seed, sizeof (uuid_seed));
#else
  while (getrandom (uuid_seed, sizeof (uuid_seed), 0) != sizeof (uuid_seed))
  {
    iot_wait_usecs (RAND_READ_DELAY);
  }
#endif
  uuid_initialized = true;
}

void iot_uuid_generate (iot_uuid_t out)
{
  union { uint8_t b[16]; uint64_t word[2]; } s;
  assert (out);
  pthread_mutex_lock (&uuid_mutex);
  if (! uuid_initialized) uuid_init ();
  s.word[0] = xorshift128plus (uuid_seed);
  s.word[1] = xorshift128plus (uuid_seed);
  pthread_mutex_unlock (&uuid_mutex);
  s.b[6] = (uint8_t) ((s.b[6] & 0xf) | 0x40); // Flag as randomly generated uuid
  s.b[8] = (uint8_t) ((s.b[8] & 0x3f) | 0x80);
  memcpy (out, s.b, sizeof (s.b));
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
