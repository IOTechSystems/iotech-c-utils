#include "iot/iot.h"

static void cache_purge (iot_data_t ** cache)
{
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_MULTI);
  iot_data_map_iter_t iter;

  iot_data_map_iter (*cache, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t *  key = iot_data_map_iter_key (&iter);
    if (iot_data_ref_count (key) > 3u)
    {
      iot_data_map_add (map, iot_data_add_ref (key), iot_data_add_ref (key));
    }
  }
  iot_data_free (*cache);
  *cache = map;
}

int main (int argc, char ** argv)
{
  iot_data_t * map;
  char * json;
  int count;
  uint32_t dup_strings = 0u;
  uint32_t dup_maps = 0u;
  uint32_t dup_vectors = 0u;
  uint32_t dup_ints = 0u;

  if (argc == 1)
  {
    printf ("Usage: %s <json_file(s)>\n", argv[0]);
    exit (1);
  }
  iot_data_alloc_heap (true);
  iot_data_t *cache = iot_data_alloc_typed_map (IOT_DATA_MULTI, IOT_DATA_MULTI);
  for (count = 1; count < argc; count++)
  {
    printf ("Loading JSON file: %s\n", argv[count]);
    json = iot_file_read (argv[count]);
    map = iot_data_from_json (json);
    free (json);
    iot_data_compress_with_cache (map, cache);
    cache_purge (&cache);
//    iot_data_free (map);
  }

  iot_data_map_iter_t iter;
  iot_data_map_iter (cache, &iter);
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    if (iot_data_type (key) == IOT_DATA_STRING)
    {
      dup_strings++;
    }
    if (iot_data_type (key) == IOT_DATA_INT64)
    {
      dup_ints++;
    }
    if (iot_data_type (key) == IOT_DATA_MAP)
    {
      dup_maps++;
    }
    if (iot_data_type (key) == IOT_DATA_VECTOR)
    {
      dup_vectors++;
    }
  }
  printf ("Cache size: %d\n", iot_data_map_size (cache));
  printf ("Cached Strings: # %" PRIu32 "\n", dup_strings);
  printf ("Cached Maps: # %" PRIu32 "\n", dup_maps);
  printf ("Cached Ints: # %" PRIu32 "\n", dup_ints);
  printf ("Cached Vectors: # %" PRIu32 "\n", dup_vectors);

  //iot_data_free (cache);
  /*
  json = iot_data_to_json (map);
  iot_file_write ("/tmp/test.json", json);
  free (json);
   */
  return 0;
}
