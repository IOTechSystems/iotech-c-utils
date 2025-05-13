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
#include <regex.h>
#include <sys/inotify.h>

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
      ret = malloc (size + 1u); // Allocate extra byte so can be NULL terminated if a string
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

static bool iot_file_update_binary (const char * path, const uint8_t * binary, size_t len, bool append)
{
  assert (path && binary && len);
  bool ok = false;
  bool exists = iot_file_exists (path);
  bool backup = exists && !append;
  char tmp_path[PATH_MAX];
  if (backup)
  {
    strncpy (tmp_path, path, PATH_MAX - 5u);
    strncat (tmp_path, ".new", 5u);
  }
  FILE *fd = fopen (backup ? tmp_path : path, append ? "a" : "w");
  if (fd)
  {
    ok = (fwrite (binary, len, 1u, fd) == 1u);
    fclose (fd);
    if (ok && backup) rename (tmp_path, path);
  }
  return ok;
}

bool iot_file_write_binary (const char * path, const uint8_t * binary, size_t len)
{
  return iot_file_update_binary (path, binary, len, false);
}

bool iot_file_append_binary (const char * path, const uint8_t * binary, size_t len)
{
  return iot_file_update_binary (path, binary, len, true);
}

char * iot_file_read (const char *path)
{
  return (char *) iot_file_read_binary (path, NULL);
}

bool iot_file_write (const char * path, const char * str)
{
  assert (str);
  return iot_file_update_binary (path, (const uint8_t*) str, strlen (str), false);
}

bool iot_file_append (const char * path, const char * str)
{
  assert (str);
  return iot_file_update_binary (path, (const uint8_t*) str, strlen (str), true);
}

iot_data_t * iot_file_list (const char * directory, const char * regex_str)
{
  iot_data_t * list = NULL;
  DIR *d = opendir (directory);
  if (!d) goto DONE;
  regex_t regex;
  if (regex_str && regcomp (&regex, regex_str, REG_NOSUB) != 0) goto DONE;
  list = iot_data_alloc_list ();
  const struct dirent *dir;
  while ((dir = readdir (d)))
  {
    if (dir->d_type == DT_REG)
    {
      size_t nmatch = 0;
      if (!regex_str || regexec (&regex, dir->d_name, nmatch, NULL, 0) == 0)
      {
        iot_data_list_tail_push (list, iot_data_alloc_string (dir->d_name, IOT_DATA_COPY));
      }
    }
  }
  if (regex_str) regfree (&regex);
  closedir (d);
DONE:
  return list;
}

bool iot_file_exists (const char * path)
{
  assert (path);
  return (access (path, F_OK) == 0);
}

const uint32_t iot_file_self_delete_flag = IN_DELETE_SELF;
const uint32_t iot_file_delete_flag = IN_DELETE;
const uint32_t iot_file_modify_flag = IN_MODIFY;

uint32_t iot_file_watch (const char * path, uint32_t mask)
{
  assert (path);
  uint32_t change = 0u;
  int fd = inotify_init ();
  int wd = inotify_add_watch (fd, path, mask);
  if (wd > 0)
  {
    char buffer[sizeof (struct inotify_event) + NAME_MAX + 1];
    ssize_t ret = read (fd, buffer, sizeof (buffer));
    if (ret > 0)
    {
      const struct inotify_event * event = (const struct inotify_event *) buffer;
      change = event->mask & mask;
    }
    inotify_rm_watch (fd, wd);
    close (fd);
  }
  return change;
}

#endif
#endif
