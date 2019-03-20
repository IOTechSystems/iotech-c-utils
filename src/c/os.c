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

char * iot_ctok_r (char *str, const char delim, char **saveptr)
{
  char *tok = NULL;
  char c;

  if (str == NULL)
  {
    str = *saveptr;
  }
  *saveptr = NULL;
  if (str)
  {
    while (*str == delim) // Skip deliminators at start
    {
      str++;
    }
    if (*str) // Check not at end
    {
      tok = str; // Start of token
      while ((c = *str))
      {
        if (c == delim)
        {
          *str = 0;
          *saveptr = ++str;
          break;
        }
        str++;
      }
    }
  }
  return tok;
}
