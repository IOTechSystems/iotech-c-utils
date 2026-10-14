/*
 * Copyright (c) 2022
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iot/file.h"
#include <limits.h>

#ifdef IOT_HAS_FILE

#include <dirent.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define IOT_FILE_READ_SIZE 4096u

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
    // Read until end of file, as size reported for special files (/proc, /sys, pipes) may be incorrect, and a file
    // may change size while being read. Buffer sized for regular files, so read once (plus end of file check).
    struct stat st;
    bool regular = (fstat (fileno (fd), &st) == 0) && S_ISREG (st.st_mode) && (st.st_size > 0);
    size_t capacity = regular ? (size_t) st.st_size + 1u : IOT_FILE_READ_SIZE;
    ret = malloc (capacity + 1u); // Allocate extra byte so can be NULL terminated if a string
    while (true)
    {
      size += fread (ret + size, 1u, capacity - size, fd);
      if (size < capacity) break; // End of file or error
      capacity *= 2u;
      ret = realloc (ret, capacity + 1u);
    }
    if (ferror (fd))
    {
      free (ret);
      ret = NULL;
      size = 0;
    }
    else
    {
      ret[size] = 0; // String NULL terminator
    }
    fclose (fd);
  }
  if (len) *len = size;
  return ret;
}

static bool iot_file_write_all (int fd, const uint8_t * data, size_t len)
{
  while (len)
  {
    ssize_t written = write (fd, data, len);
    if (written < 0)
    {
      if (errno == EINTR) continue;
      return false;
    }
    data += written;
    len -= (size_t) written;
  }
  return true;
}

static bool iot_file_update_binary (const char * path, const uint8_t * binary, size_t len, bool append)
{
  assert (path && (binary || len == 0u));
  bool ok = false;
  struct stat st;
  bool replace = !append && (lstat (path, &st) == 0) && S_ISREG (st.st_mode); // Existing regular file replaced via temporary file, so never partially written (symbolic links and special files written directly)
  char tmp_path[PATH_MAX];
  int fd;
  if (replace)
  {
    int n = snprintf (tmp_path, sizeof (tmp_path), "%s.XXXXXX", path);
    if ((n < 0) || ((size_t) n >= sizeof (tmp_path))) return false; // Temporary file path too long
    fd = mkstemp (tmp_path); // Unique temporary file in same directory, so rename is atomic
    if (fd != -1)
    {
      if (fchown (fd, st.st_uid, st.st_gid) != 0) {} // Keep ownership of replaced file (only possible if privileged)
      if (fchmod (fd, st.st_mode & 07777) != 0) // Keep permissions of replaced file
      {
        close (fd);
        unlink (tmp_path);
        fd = -1;
      }
    }
  }
  else
  {
    fd = open (path, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0666);
  }
  if (fd != -1)
  {
    ok = iot_file_write_all (fd, binary, len);
    if (replace) ok = (fsync (fd) == 0) && ok; // Data must be on disk before rename, or file may be empty after power loss
    ok = (close (fd) == 0) && ok;
    if (replace)
    {
      if (ok) ok = (rename (tmp_path, path) == 0);
      if (! ok) unlink (tmp_path);
    }
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
  assert (directory);
  iot_data_t * list = NULL;
  regex_t regex;
  if (regex_str && regcomp (&regex, regex_str, REG_NOSUB) != 0) return NULL;
  DIR * d = opendir (directory);
  if (d)
  {
    list = iot_data_alloc_list ();
    const struct dirent * dir;
    while ((dir = readdir (d)))
    {
      bool regular = (dir->d_type == DT_REG);
      if (dir->d_type == DT_UNKNOWN) // Some file systems do not return file type, so check directly
      {
        struct stat st;
        regular = (fstatat (dirfd (d), dir->d_name, &st, AT_SYMLINK_NOFOLLOW) == 0) && S_ISREG (st.st_mode);
      }
      if (regular && (!regex_str || regexec (&regex, dir->d_name, 0, NULL, 0) == 0))
      {
        iot_data_list_tail_push (list, iot_data_alloc_string (dir->d_name, IOT_DATA_COPY));
      }
    }
    closedir (d);
  }
  if (regex_str) regfree (&regex);
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
  static const uint32_t stop_flags = IN_CLOSE_WRITE | IN_DELETE_SELF | IN_DELETE;
  assert (path);
  uint32_t flags = 0u;
  mask &= (iot_file_self_delete_flag | iot_file_delete_flag | iot_file_modify_flag); // Only allow supported flags
  int fd = inotify_init ();
  if (fd == -1) return 0u;
  int wd = inotify_add_watch (fd, path, mask | stop_flags); // Add stop flags to watch set for termination
  if (wd != -1)
  {
    char buffer[sizeof (struct inotify_event) + NAME_MAX + 1]  __attribute__ ((aligned(8)));
    while (true)
    {
      ssize_t ret = read (fd, buffer, sizeof (buffer));
      if (ret < 0)
      {
        if (errno == EINTR) continue;
        break; // Unrecoverable error, stop watching
      }
      const char * ptr = buffer;
      while (ret > 0) // Process set of events from read
      {
        const struct inotify_event * event = (const struct inotify_event *) ptr;
        flags |= event->mask; // Update change flags
        size_t size = sizeof (*event) + event->len;
        ret -= size;
        ptr += size;
      }
      if (flags & stop_flags) break; // Stop watching if stop flag set
    }
    inotify_rm_watch (fd, wd);
  }
  close (fd);
  return flags & mask; // Only return user supplied flags
}

#endif
