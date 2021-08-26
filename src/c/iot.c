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

void iot_init (void)
{
  iot_data_init ();
}

void iot_fini (void)
{
  // Placeholder for any required global cleanup
}

#ifdef IOT_HAS_FILE

static char * iot_file_config_path (const char * name, const char * uri)
{
  assert (name);
  char *  path = malloc (strlen (name) + (uri ? (strlen (uri) + 7) : 6));
  path[0] = '\0';
  if (uri)
  {
    strcpy (path, uri);
    strcat (path, "/");
  }
  strcat (path, name);
  strcat (path, ".json");
  return path;
}

char * iot_file_config_loader (const char * name, const char * uri)
{
  char * path = iot_file_config_path (name, uri);
  char * ret = iot_file_read (path);
  free (path);
  return ret;
}

bool iot_file_config_saver (const char * name, const char * uri, const char * config)
{
  char * path = iot_file_config_path (name, uri);
  bool ok = iot_file_write (path, config);
  free (path);
  return ok;
}

char * iot_file_read (const char * path)
{
  assert (path);
  return (char*) iot_file_read_binary (path, NULL);
}

extern bool iot_file_write (const char * path, const char * str)
{
  assert (path && str);
  return iot_file_write_binary (path, (const uint8_t*) str, strlen (str));
}

#ifdef _AZURESPHERE_

#define IOT_MALLOC_BLOCK_SIZE 512

uint8_t * iot_file_read_binary (const char * path, size_t * len)
{
  uint8_t * buff = NULL;
  int fd = Storage_OpenFileInImagePackage (path);
  if (fd != -1)
  {
    ssize_t ret;
    uint8_t * ptr;
    size_t size = 0;
    while (true)
    {
      buff = realloc (buff, size + IOT_MALLOC_BLOCK_SIZE);
      ptr = buff + size;
      memset (ptr, 0, IOT_MALLOC_BLOCK_SIZE);
      ret = read (fd, ptr, IOT_MALLOC_BLOCK_SIZE);
      if (ret < IOT_MALLOC_BLOCK_SIZE)
      {
        if (len) *len = size;
        break;
      }
      size += IOT_MALLOC_BLOCK_SIZE;
    }
    close (fd);
  }
  else
  {
    if (len) *len = 0;
    Log_Debug ("Error opening file: %s %s (%d)\n", path, strerror (errno), errno);
  }
  return buff;
}

bool iot_file_write_binary (const char * path, const uint8_t * binary, size_t len)
{
  bool ok = false;
  int fd = Storage_OpenFileInImagePackage (path);
  if (fd != -1)
  {
    ok = (write (fd, binary, len) == len);
    close (fd);
  }
  if (! ok)
  {
    Log_Debug ("Error writing to file: %s %s (%d)\n", path, strerror (errno), errno);
  }
  return ok;
}

#else

extern bool iot_file_delete (const char * path)
{
  return (remove (path) == 0);
}

uint8_t * iot_file_read_binary (const char * path, size_t * len)
{
  uint8_t * ret = NULL;
  size_t size = 0;

  FILE * fd = fopen (path, "r");
  if (fd)
  {
    fseek (fd, 0, SEEK_END);
    size = ftell (fd);
    rewind (fd);
    ret = malloc (size + 1); // Allocate extra byte so can be NULL terminated if a string
    size_t items = fread (ret, size, 1u, fd);
    assert (items == 1);
    (void) items;
    ret[size] = 0; // String NULL terminator
    fclose (fd);
  }
  if (len) *len = size;
  return ret;
}

bool iot_file_write_binary (const char * path, const uint8_t * binary, size_t len)
{
  bool ok = false;
  FILE * fd = fopen (path, "w");
  if (fd)
  {
    ok = (fwrite (binary, len, 1u, fd) == 1u);
    fclose (fd);
  }
  return ok;
}

#endif
#endif
