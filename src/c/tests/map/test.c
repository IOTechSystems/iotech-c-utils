#include "iot/iot.h"

#define LOTS 1000

#ifndef NDEBUG
extern void iot_data_map_dump (iot_data_t * map);
#else
#define iot_data_map_dump(n)
#endif

/* Test cases from: https://algorithmtutor.com/Data-Structures/Tree/Red-Black-Trees/
 * Validate that generated tree structures match examples cases.
 */

static void check (iot_data_t * map, const uint32_t size)
{
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  assert (size == iot_data_map_size (map));
  printf ("\n");
  while (iot_data_map_iter_next (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    printf ("%" PRIu32 " ", iot_data_ui32 (key));
  }
  printf ("\n");
  while (iot_data_map_iter_prev (&iter))
  {
    const iot_data_t * key = iot_data_map_iter_key (&iter);
    printf ("%" PRIu32 " ", iot_data_ui32 (key));
  }
  printf ("\n");
  //iot_data_map_dump (map);
  iot_data_free (map);
}

static void case_add_3_1 (void)
{
  printf ("\n### Add Case 3.1 (Parent Red, Uncle Red)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_map_add (map, iot_data_alloc_ui32 (61), iot_data_alloc_string ("61", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (52), iot_data_alloc_string ("52", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (85), iot_data_alloc_string ("85", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (76), iot_data_alloc_string ("76", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (93), iot_data_alloc_string ("93", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (100), iot_data_alloc_string ("100", IOT_DATA_REF));
  check (map, 6);
}

static void case_add_3_2_1 (void)
{
  printf ("\n### Add Case 3.2.1\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_map_add (map, iot_data_alloc_ui32 (61), iot_data_alloc_string ("61", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (52), iot_data_alloc_string ("52", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (85), iot_data_alloc_string ("85", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (93), iot_data_alloc_string ("93", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (100), iot_data_alloc_string ("100", IOT_DATA_REF));
  check (map, 5);
}

static void case_add_3_2_2 (void)
{
  printf ("\n### Add Case 3.2.2\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_map_add (map, iot_data_alloc_ui32 (61), iot_data_alloc_string ("61", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (52), iot_data_alloc_string ("52", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (85), iot_data_alloc_string ("85", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (93), iot_data_alloc_string ("93", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (87), iot_data_alloc_string ("87", IOT_DATA_REF));
  check (map, 5);
}

static void case_del_1 (void)
{
  printf ("\n### Remove Case 1 (Node Red)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (5);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 4);
}

static void case_del_2 (void)
{
  printf ("\n### Remove Case 2 (Red Child)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (4);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 4);
}

static void case_del_3_1 (void)
{
  printf ("\n### Remove Case 3.1 (Black Node, Red Sibling)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (1);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (6), iot_data_alloc_string ("6", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 5);
}

static void case_del_3_2 (void)
{
  printf ("\n### Remove Case 3.2 (Black Node, Black Sibling, Sibling Children Black)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (1);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (6), iot_data_alloc_string ("6", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (7), iot_data_alloc_string ("7", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 6);
}

static void case_del_3_3 (void)
{
  printf ("\n### Remove Case 3.3 (Black Node, Black Sibling, Sibling Left Child Red, Sibling Right Child Black)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (1);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (6), iot_data_alloc_string ("6", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (7), iot_data_alloc_string ("7", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (8), iot_data_alloc_string ("8", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 7);
}

static void case_del_3_4 (void)
{
  printf ("\n### Remove Case 3.4 (Black node, Sibling is Black, Sibling Right Child is Red)\n");
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  iot_data_t * key = iot_data_alloc_ui32 (1);
  iot_data_map_add (map, iot_data_alloc_ui32 (1), iot_data_alloc_string ("1", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (2), iot_data_alloc_string ("2", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (3), iot_data_alloc_string ("3", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (4), iot_data_alloc_string ("4", IOT_DATA_REF));
  iot_data_map_add (map, iot_data_alloc_ui32 (5), iot_data_alloc_string ("5", IOT_DATA_REF));
  iot_data_map_remove (map, key);
  iot_data_free (key);
  check (map, 4);
}

static uint32_t get_rand (void)
{
  return (rand () % LOTS);
}

static void fill_array (int32_t arr[LOTS])
{
  uint32_t count = 0;
  while (count < LOTS) arr[count++] = -1;
  srand (time (0));
  count = 0;
  while (count < LOTS)
  {
    uint32_t rnd = get_rand ();
    if (arr[rnd] == -1)
    {
      arr[rnd] = count++;
    }
  }
}

static iot_data_t * fill_map (int32_t arr[LOTS])
{
  uint32_t count = 0;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_UINT32);
  fill_array (arr);
  while (count < LOTS)
  {
    iot_data_map_add (map, iot_data_alloc_ui32 (arr[count++]), iot_data_alloc_string ("value", IOT_DATA_REF));
  }
  return map;
}

static void add_lots (void)
{
  int32_t arr[LOTS];
  iot_data_t * map = fill_map (arr);
  check (map, LOTS);
}

static void del_lots (void)
{
  uint32_t count = 0;
  int32_t arr[LOTS];
  iot_data_t * map = fill_map (arr);
  iot_data_t * key;
  while (count < LOTS)
  {
    key = iot_data_alloc_ui32 (arr[count++]);
    iot_data_map_remove (map, key);
    iot_data_free (key);
  }
  check (map, 0);
}
static void find_lots (void)
{
  uint32_t count = 0;
  int32_t arr[LOTS];
  iot_data_t * map = fill_map (arr);
  iot_data_t * key;
  while (count < LOTS)
  {
    key = iot_data_alloc_ui32 (arr[count++]);
    assert (iot_data_map_get (map, key));
    iot_data_free (key);
  }
  check (map, LOTS);
}

int main (void)
{
  case_add_3_1 ();
  case_add_3_2_1 ();
  case_add_3_2_2 ();
  case_del_1 ();
  case_del_2 ();
  case_del_3_1 ();
  case_del_3_2 ();
  case_del_3_3 ();
  case_del_3_4 ();
  add_lots ();
  del_lots ();
  find_lots ();
  return 0;
}
