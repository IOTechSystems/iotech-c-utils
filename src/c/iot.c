/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"
#ifdef _AZURESPHERE_
#include <applibs/log.h>
#include <applibs/storage.h>
#endif

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
  assert (name);

  char * ret;
  char *  path = malloc (strlen (name) + ((uri) ? (strlen (uri) + 7) : 6));
  path[0] = '\0';

  if (uri)
  {
    strcpy (path, uri);
    strcat (path, "/");
  }
  strcat (path, name);
  strcat (path, ".json");
  ret = iot_file_read (path);
  free (path);
  return ret;
}

#ifdef IOT_HAS_FILE

char * iot_file_read (const char * path)
{
  char * ret = NULL;
  size_t size;
  size_t items;

  FILE * fd = fopen (path, "r");
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
  return ret;
}
#else
#ifdef _AZURESPHERE_

#define IOT_MALLOC_BLOCK_SIZE 512

char * iot_file_read (const char * path)
{
  char * str = NULL;
  int fd = Storage_OpenFileInImagePackage (path);
  if (fd != -1)
  {
    ssize_t ret;
    char * ptr;
    size_t size = 0;
    do
    {
      str = realloc (str, size + IOT_MALLOC_BLOCK_SIZE);
      ptr = str + size;
      memset (ptr, 0, IOT_MALLOC_BLOCK_SIZE);
      ret = read (fd, ptr, IOT_MALLOC_BLOCK_SIZE);
      size += IOT_MALLOC_BLOCK_SIZE;
    }
    while (ret == IOT_MALLOC_BLOCK_SIZE);
    close (fd);
  }
  else
  {
    Log_Debug ("Error opening file: %s %s (%d)\n", path, strerror (errno), errno);
  }
  Log_Debug ("READ: %s\n", str);
  return str;
}

#endif
#endif