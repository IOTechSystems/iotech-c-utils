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

#ifdef IOT_HAS_FILE

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
#else
#ifdef _AZURESPHERE_
char * iot_file_config_loader (const char * name, const char * uri)
{
  assert (name);

  char * ret = NULL;
  size_t size = strlen (name) + ((uri) ? (strlen (uri) + 7) : 6);
  char *  path = calloc (1, size);

  if (uri)
  {
    strcpy (path, uri);
    strcat (path, "/");
  }
  strcat (path, name);
  strcat (path, ".json");
  int fd = Storage_OpenFileInImagePackage (path);
  if (fd != -1)
  {
    ret = calloc (1, 1024);
    ssize_t no = read (fd, ret, 1024);
    Log_Debug ("Config: %s %d : %s\n", path, no, ret);
    close (fd);
  }
  else
  {
    Log_Debug ("Error opening config: %s %s (%d)\n", path, strerror (errno), errno);
  }

  free (path);
  return ret;
}
#endif
#endif