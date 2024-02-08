/*
 * Copyright (c) 2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/file.h"
#include <limits.h>

#ifdef IOT_HAS_FILE
#ifdef _AZURESPHERE_
#include <applibs/log.h>
#include <applibs/storage.h>
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

extern bool iot_file_delete (const char * path)
{
  (void) path;
  return (Storage_DeleteMutableFile () == 0);
}

#else // _AZURESPHERE_
#include <dirent.h>

bool iot_file_delete (const char * path)
{
  assert (path);
  return (remove (path) == 0);
}

uint8_t * iot_file_read_binary (const char * path, size_t * len)
{
  assert (path);
  uint8_t * ret = NULL;
  size_t size = 0;

  FILE * fd = fopen (path, "r");
  if (fd)
  {
    fseek (fd, 0, SEEK_END);
    size = ftell (fd);
    if (size) // Return NULL if file empty
    {
      rewind (fd);
      ret = malloc (size + 1); // Allocate extra byte so can be NULL terminated if a string
      size_t items = fread (ret, size, 1u, fd);
      assert (items == 1);
      (void) items;
      ret[size] = 0; // String NULL terminator
    }
    fclose (fd);
  }
  if (len) *len = size;
  return ret;
}

bool iot_file_write_binary (const char * path, const uint8_t * binary, size_t len)
{
  assert (path && binary);
  bool ok = false;
  char tmp_path[PATH_MAX];
  strncpy (tmp_path, path, PATH_MAX - 5);
  tmp_path[PATH_MAX-5] = '\0';
  strcat (tmp_path, ".new");
  FILE *fd = fopen (tmp_path, "w");
  if (fd)
  {
    ok = (fwrite (binary, len, 1u, fd) == 1u);
    fclose (fd);
    if (ok) rename (tmp_path, path);
  }
  return ok;
}

char * iot_file_read (const char *path)
{
  assert (path);
  return (char *) iot_file_read_binary (path, NULL);
}

bool iot_file_write (const char * path, const char * str)
{
  assert (str);
  return iot_file_write_binary (path, (const uint8_t*) str, strlen (str));
}

iot_data_t * iot_file_list (const char * directory, const char * remove_extension)
{
  iot_data_t * list = NULL;
  DIR *d = opendir (directory);
  if (!d) goto DONE;
  list = iot_data_alloc_list ();
  const struct dirent *dir;
  while ((dir = readdir (d)))
  {
    if (dir->d_type != DT_REG) continue;
    size_t file_len = strlen (dir->d_name);
    if (remove_extension && strlen(dir->d_name) > strlen(remove_extension))
    {
      const char *extension = dir->d_name + file_len - strlen (remove_extension);
      if (strcmp (extension, remove_extension) == 0)
      {
        file_len -= strlen (remove_extension);
      }
    }
    const char *name = strndup (dir->d_name, file_len);
    iot_data_list_tail_push (list, iot_data_alloc_string (name, IOT_DATA_TAKE));
  }
  closedir (d);
DONE:
  return list;
}

#endif
#endif
