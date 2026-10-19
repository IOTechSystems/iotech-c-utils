#include "iot/hash.h"

int main (int argc, char ** argv)
{
  if (argc != 2)
  {
    fprintf (stderr, "Usage: %s <string>\n", argv[0]);
    return 1;
  }
  uint32_t hash = iot_hash (argv[1]);
  printf ("%" PRIu32 "\n", hash);
  return 0;
}
