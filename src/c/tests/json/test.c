#include "iot/iot.h"

int main (int argc, char ** argv)
{
  if (argc < 2)
  {
    char buff[2];
    buff[1] = '\0';
    for (int i = 1; i < 128; i++)
    {
      buff[0] = (char) i;
      iot_data_t * ch = iot_data_alloc_string (buff, IOT_DATA_REF);
      char * char_str = iot_data_to_json (ch);
      printf ("%d: %s\n", i, char_str);
      free (char_str);
      iot_data_free (ch);
    }
  }
  else
  {
    char * json = iot_store_read (argv[1]);
    if (json == NULL)
    {
      fprintf (stderr, "Failed to read file: %s\n", argv[1]);
      exit (-1);
    }

    if (argc == 2)
    {
      iot_data_t * map = iot_data_from_json (json);
      free (json);
      for (int i = 0; i < 1000; i++)
      {
        json = iot_data_to_json (map);
        ((volatile char*) json)[0] = json[0]; // prevent json beeing optimised out 
        free (json);
      }
      iot_data_free (map);
    }
    else if (argc == 3)
    {
      iot_store_config_save (argv[2], ".", json);
      free (json);
    }
  }
  return 0;
}
