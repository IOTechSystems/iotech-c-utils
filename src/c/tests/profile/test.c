#include "iot/iot.h"

#ifndef NDEBUG
extern void iot_data_map_dump (iot_data_t * map);
#else
#define iot_data_map_dump(n)
#endif

int main (int argc, char ** argv)
{
  if (argc != 2)
  {
    fprintf (stderr, "Usage: test <profile file>\n");
    exit (-1);
  }
  char * profile = iot_store_read (argv[1]);
  if (profile == NULL)
  {
    fprintf (stderr, "Failed to read file: %s\n", argv[1]);
    exit (-1);
  }
  iot_data_t * cache = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * map = iot_data_from_json_with_cache (profile, false, cache);
//  iot_data_map_dump (cache);
  iot_data_free (cache);
  iot_data_free (map);
  free (profile);
  return 0;
}
