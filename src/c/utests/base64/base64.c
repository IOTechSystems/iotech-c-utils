/*
 * Copyright (c) 2020-2026
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "base64.h"
#include "CUnit.h"

static int suite_init (void)
{
  return 0;
}

static int suite_clean (void)
{
  return 0;
}

#define BASE64_SRC_LEN 16

static void test_rtrip1 (void)
{
  uint8_t input[BASE64_SRC_LEN];
  char encoded[BASE64_SRC_LEN * 2];
  uint8_t decoded[BASE64_SRC_LEN];

  size_t outlen;

  srandom (7);
  for (unsigned i = 0; i < BASE64_SRC_LEN; i++)
  {
    input[i] = (uint8_t) (random () % 256);
  }

  for (size_t size = 1; size <= BASE64_SRC_LEN; size++)
  {
    memset (encoded, 0, sizeof (encoded));
    memset (decoded, 0, sizeof (decoded));
    CU_ASSERT (iot_b64_encode (input, size, encoded, sizeof (encoded)))
    outlen = size;
    CU_ASSERT (iot_b64_decode (encoded, decoded, &outlen))
    CU_ASSERT (size == outlen)
    CU_ASSERT (memcmp (input, decoded, size) == 0)
  }
}

static bool test_decode (const char * in, const char * expected)
{
  uint8_t decoded[BASE64_SRC_LEN];
  size_t outlen = sizeof (decoded);
  bool ok = iot_b64_decode (in, decoded, &outlen);
  if (ok && expected) ok = (outlen == strlen (expected)) && (memcmp (decoded, expected, outlen) == 0);
  return ok;
}

static void test_decode_valid (void)
{
  CU_ASSERT (test_decode ("", ""))
  CU_ASSERT (test_decode ("QQ==", "A"))
  CU_ASSERT (test_decode ("QUI=", "AB"))
  CU_ASSERT (test_decode ("QUJD", "ABC"))
  CU_ASSERT (test_decode ("QQ", "A")) // Padding optional
  CU_ASSERT (test_decode ("QUI", "AB"))
  CU_ASSERT (test_decode ("QU\nJD\n", "ABC")) // Newlines ignored
  CU_ASSERT (test_decode ("QQ=\n=\n", "A"))
}

static void test_decode_invalid (void)
{
  CU_ASSERT (! test_decode ("QQ==QUJD", NULL)) // Data after padding
  CU_ASSERT (! test_decode ("QQ==junk", NULL))
  CU_ASSERT (! test_decode ("QUI=Q", NULL))
  CU_ASSERT (! test_decode ("QQ=", NULL)) // Incomplete padding
  CU_ASSERT (! test_decode ("QUI==", NULL)) // Excess padding
  CU_ASSERT (! test_decode ("QUJD=", NULL))
  CU_ASSERT (! test_decode ("=", NULL))
  CU_ASSERT (! test_decode ("Q", NULL)) // Single trailing symbol
  CU_ASSERT (! test_decode ("QUJDR", NULL))
  CU_ASSERT (! test_decode ("Q===", NULL))
  CU_ASSERT (! test_decode ("QU*D", NULL)) // Invalid character
}

void cunit_base64_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("base64", suite_init, suite_clean);
  CU_add_test (suite, "test_rtrip1", test_rtrip1);
  CU_add_test (suite, "test_decode_valid", test_decode_valid);
  CU_add_test (suite, "test_decode_invalid", test_decode_invalid);
}
