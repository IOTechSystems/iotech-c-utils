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
    input[i] = random() % 256;
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

void cunit_base64_test_init (void)
{
  CU_pSuite suite = CU_add_suite ("base64", suite_init, suite_clean);
  CU_add_test (suite, "test_rtrip1", test_rtrip1);
}
