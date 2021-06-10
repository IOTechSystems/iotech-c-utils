/**
 * Copyright (c) 2018 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/random.h>

#include "uuid.h"

#define RAND_READ_DELAY 1000
static const char  *hexdigits_lower = "0123456789abcdef";

static uint64_t seed[2];

static uint64_t xorshift128plus(uint64_t *s) {
  /* http://xorshift.di.unimi.it/xorshift128plus.c */
  uint64_t s1 = s[0];
  const uint64_t s0 = s[1];
  s[0] = s0;
  s1 ^= s1 << 23;
  s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
  return s[1] + s0;
}


static int uuid_init(void)
{
  int ret = 0;
  while (1)
  {
    if (getrandom (seed, sizeof (seed), GRND_NONBLOCK)) 
    {
      ret = 1;
      break;
    }
    else
    {  
      if (errno == EAGAIN) /* no entropy, wait and try again */
      {
        usleep (RAND_READ_DELAY);
      }
      else 
      {
        printf ("getrandom() err: %s\n", strerror(errno));
        break;
      }
    }
  }
  return ret;
}


void uuid_generate(uuid_t out)
{
  static const char *template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
  union { unsigned char b[16]; uint64_t word[2]; } s;
  const char *p;
  int i, n;
  
  if (uuid_init())
  {
    /* get random */
    s.word[0] = xorshift128plus(seed);
    s.word[1] = xorshift128plus(seed);
  
    /* build string */
    p = template;
    i = 0;
    while (*p)
    {
      n = s.b[i >> 1];
      n = (i & 1) ? (n >> 4) : (n & 0xf);
      switch (*p)
      {
        case 'x'  : *out = hexdigits_lower[n];              i++;  break;
        case 'y'  : *out = hexdigits_lower[(n & 0x3) + 8];  i++;  break;
        default   : *out = *p;
      }
      out++, p++;
    }
    out = '\0';
  }
  else
  {
    printf ("Unable to get seed, uuid_generation failed\n");
  }  
}

/* Ref: unparse.c from util_linux/libuuid/src) */
void uuid_unparse(const uuid_t uuid, char *out)
{
  char *p = out;
  int i;

  for (i = 0; i < 16; i++)
  {
    if (i == 4 || i == 6 || i == 8 || i == 10)
    {
      *p++ = '-';
    }
    size_t tmp = uuid[i];
    *p++ = hexdigits_lower[tmp >> 4];
    *p++ = hexdigits_lower[tmp & 15];
  }
  *p = '\0';
}
