/*
 * Copyright (c) 2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/util.h"

#ifndef UUID_STR_LEN
#define UUID_STR_LEN 37u
#endif

bool iot_util_string_is_uuid (const char * str)
{
  bool ok = false;
  if (str && (strlen (str) == (UUID_STR_LEN - 1)))
  {
    for (unsigned i = 0u; i < (UUID_STR_LEN - 1); i++)
    {
      char c = str[i];
      if (i == 8u || i == 13u || i == 18u || i == 23u)
      {
        if (c != '-') goto FAIL;
      }
      else
      {
        if (isxdigit (c) == 0) goto FAIL;
      }
    }
    ok = true;
  }
FAIL:
  return ok;
}
