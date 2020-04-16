/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"

extern void iot_data_init (void);
extern void iot_data_fini (void);

void iot_init (void)
{
  iot_data_init ();
}

void iot_fini (void)
{
  iot_data_fini ();
}

char * iot_file_config_loader (const char * name, const char * uri)
{
  char * ret = NULL;
  char * path;
  size_t size;
  size_t items;
  FILE * fd;

  assert (name);
  size = strlen (name) + 6;
  if (uri) size += strlen (uri) + 1;
  path = malloc (size);

  if (uri)
  {
    strcpy (path, uri);
    strcat (path, "/");
    strcat (path, name);
  }
  else
  {
    strcpy (path, name);
  }
  strcat (path, ".json");
  fd = fopen (path, "r");
  if (fd)
  {
    fseek (fd, 0, SEEK_END);
    size = ftell (fd);
    rewind (fd);
    ret = malloc (size + 1);
    items = fread (ret, size, 1, fd);
    assert (items == 1);
    (void) items;
    ret[size] = 0;
    fclose (fd);
  }
  free (path);
  return ret;
}