#include "iot/os.h"

char *strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  char * copy = malloc (len);

  if (copy)
  {
    strcpy (copy, s);
  }

  return copy;
}

