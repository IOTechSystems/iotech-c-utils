#include "iot/os.h"

#ifdef __ZEPHYR__
char * iot_strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  char * copy = malloc (len);

  if (copy)
  {
    strcpy (copy, s);
  }

  return copy;
}
#endif
