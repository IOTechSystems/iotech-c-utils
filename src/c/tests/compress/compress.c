#include "iot/iot.h"

int main (int argc, char ** argv)
{
  char * json;
  int count;
  if (argc == 1)
  {
    printf ("Usage: %s <json_file(s)>\n", argv[0]);
    exit (1);
  }
  iot_data_t *cache = iot_data_alloc_typed_map (IOT_DATA_MULTI, IOT_DATA_MULTI);
  for (count = 1; count < argc; count++)
  {
    printf ("Loading JSON file: %s\n", argv[count]);
    json = iot_file_read (argv[count]);
    iot_data_t * map = iot_data_from_json (json);
    free (json);
    iot_data_compress_with_cache (map, cache);
    printf ("Cache size: %d", iot_data_map_size (cache));
  }

  iot_data_map_iter_t iter;
  iot_data_map_iter (cache, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    /*
    if (iot_data_type (key) == IOT_DATA_STRING)
    {
      printf ("Cache String: %s # %" PRIu32 "\n", iot_data_string (key), iot_data_ref_count (key));
    }
    if (iot_data_type (key) == IOT_DATA_INT64)
    {
      printf ("Cache Int: %" PRIu64 " # %" PRIu32 "\n", iot_data_i64 (key), iot_data_ref_count (key));
    }
     */
    if (iot_data_type (key) == IOT_DATA_MAP)
    {
      printf ("Cache Map: # %" PRIu32 "\n", iot_data_ref_count (key));
    }
    if (iot_data_type (key) == IOT_DATA_VECTOR)
    {
      printf ("Cache Vector:  # %" PRIu32 "\n", iot_data_ref_count (key));
    }
  }

  iot_data_free (cache);
  return 0;
}
