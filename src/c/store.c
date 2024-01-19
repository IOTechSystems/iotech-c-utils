/*
 * Copyright (c) 2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/iot.h"

#ifdef IOT_HAS_FILE
#include "iot/file.h"
static iot_store_read_fn iot_store_reader = iot_file_read_binary;
static iot_store_write_fn iot_store_writer = iot_file_write_binary;
static iot_store_delete_fn iot_store_deleter = iot_file_delete;
#else
static iot_store_read_fn iot_store_reader = NULL;
static iot_store_write_fn iot_store_writer = NULL;
static iot_store_delete_fn iot_store_deleter = NULL;
#endif

void iot_store_config (iot_store_read_fn read_fn, iot_store_write_fn write_fn, iot_store_delete_fn del_fn)
{
  iot_store_reader = read_fn;
  iot_store_writer = write_fn;
  iot_store_deleter = del_fn;
}

char * iot_store_read (const char * path)
{
  return (char*) iot_store_read_binary (path, NULL);
}

bool iot_store_write (const char * path, const char * str)
{
  assert (str);
  return iot_store_write_binary (path, (const uint8_t*) str, strlen (str));
}

uint8_t * iot_store_read_binary (const char * path, size_t * len)
{
  assert (path);
  return iot_store_reader ? iot_store_reader (path, len) : NULL;
}

bool iot_store_write_binary (const char * path, const uint8_t * binary, size_t len)
{
  assert (path && binary && len);
  return iot_store_writer ? iot_store_writer (path, binary, len) : false;
}

bool iot_store_delete (const char * path)
{
  assert (path);
  return iot_store_deleter ? iot_store_deleter (path) : false;
}

static char * iot_store_config_path (const char * name, const char * uri)
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

char * iot_store_config_load (const char * name, const char * uri)
{
  char * path = iot_store_config_path (name, uri);
  char * ret = iot_store_read (path);
  free (path);
  return ret;
}

bool iot_store_config_save (const char * name, const char * uri, const char * config)
{
  char * path = iot_store_config_path (name, uri);
  bool ok = iot_store_write (path, config);
  free (path);
  return ok;
}

bool iot_store_config_delete (const char * name, const char * uri)
{
  char * path = iot_store_config_path (name, uri);
  bool ok = iot_store_delete (path);
  free (path);
  return ok;
}
