//
// Copyright (c) 2019-2022 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/data.h"
#include "data-impl.h"
#include "iot/base64.h"
#include "iot/hash.h"
#include "iot/uuid.h"
#include <stdarg.h>
#include <math.h>
#include <float.h>

#define IOT_DATA_IS_COMPOSED_TYPE(t) ((t) >= IOT_DATA_VECTOR && (t) <= IOT_DATA_MAP)

#if (defined (_GNU_SOURCE) || defined (_ALPINE_)) && !defined (__arm__)
#define IOT_HAS_SPINLOCK
#endif

#if defined (NDEBUG) || defined (_AZURESPHERE_)
#define IOT_DATA_CACHE
#endif

#define IOT_DATA_TYPES (IOT_DATA_INVALID + 1)
#define IOT_MEMORY_BLOCK_SIZE 4096u
#define IOT_VAL_BUFF_SIZE 31u
#define IOT_STR_BUFF_DOUBLING_LIMIT 4096u
#define IOT_STR_BUFF_INCREMENT 1024u

static const char * iot_data_type_names [IOT_DATA_TYPES] = {"Int8","UInt8","Int16","UInt16","Int32","UInt32","Int64","UInt64","Float32","Float64","Bool","Pointer","String","Null","Binary","Array","Vector","List","Map","Multi", "Invalid"};
static const uint8_t iot_data_type_sizes [IOT_DATA_BINARY + 1] = {1u, 1u, 2u, 2u, 4u, 4u, 8u, 8u, 4u, 8u, sizeof (bool), sizeof (void*), sizeof (char*), 0u, 1u };
static _Thread_local bool iot_data_alloc_from_heap = false; /* Thread specific memory allocation policy */
iot_data_static_t iot_data_order = { 0 };
static const char * iot_data_const_strings [] = { "category","config","name","state","type",NULL };

iot_data_consts_t iot_data_consts = { 0 };

typedef enum iot_node_colour_t
{
  IOT_NODE_BLACK = 0,
  IOT_NODE_RED = 1
} __attribute__ ((__packed__)) iot_node_colour_t;

typedef union iot_data_union_t
{
  int8_t i8;
  uint8_t ui8;
  int16_t i16;
  uint16_t ui16;
  int32_t i32;
  uint32_t ui32;
  int64_t i64;
  uint64_t ui64;
  float f32;
  double f64;
  bool bl;
  char * str;
} iot_data_union_t;

typedef struct iot_data_value_base_t
{
  iot_data_t base;
  iot_data_union_t value;
} iot_data_value_base_t;

typedef struct iot_data_array_t
{
  iot_data_t base;
  uint32_t length;
  void * data;
} iot_data_array_t;

typedef struct iot_data_vector_t
{
  iot_data_t base;
  uint32_t size;
  iot_data_t ** values;
} iot_data_vector_t;

typedef struct iot_node_t
{
  struct iot_node_t * parent;
  struct iot_node_t * left;
  struct iot_node_t * right;
  iot_data_t * key;
  iot_data_t * value;
  iot_node_colour_t colour;
  bool heap : 1;
} iot_node_t;

typedef struct iot_data_map_t
{
  iot_data_t base;
  uint32_t size;
  iot_node_t * tree;
} iot_data_map_t;

typedef struct iot_element_t
{
  struct iot_element_t * next;
  struct iot_element_t * prev;
  iot_data_t * value;
  uint32_t priority;
  uint32_t length;
  bool heap : 1;
} iot_element_t;

/* Note: Due to size constraints, list length is carried by head element */

typedef struct iot_data_list_t
{
  iot_data_t base;
  iot_element_t * head;
  iot_element_t * tail;
} iot_data_list_t;

typedef struct iot_data_pointer_t
{
  iot_data_t base;
  void * value;
  void (*free_fn) (void*);
} iot_data_pointer_t;

// Determine minimum block size that can hold all iot_data types, maximum size of
// value string cache buffer and number of blocks per allocated memory chunk.

#define IOT_DATA_MAX (sizeof (iot_node_t))
#define IOT_DATA_BLOCK_SIZE (((IOT_DATA_MAX + 7) / 8) * 8)
#define IOT_DATA_BLOCKS ((IOT_MEMORY_BLOCK_SIZE / IOT_DATA_BLOCK_SIZE) - 1)
#define IOT_DATA_VALUE_BUFF_SIZE (IOT_DATA_BLOCK_SIZE - sizeof (iot_data_value_base_t))
#define IOT_DATA_ALLOCATING ((iot_block_t*) 1)

typedef struct iot_data_value_t
{
  iot_data_t base;
  iot_data_union_t value;
  char buff [IOT_DATA_VALUE_BUFF_SIZE];
} iot_data_value_t;

// Total size of this struct should be <= IOT_MEMORY_BLOCK_SIZE, chunks must be 8 byte aligned.
typedef struct iot_memory_block_t
{
  uint64_t chunks [(IOT_DATA_BLOCKS * IOT_DATA_BLOCK_SIZE) / (sizeof (uint64_t))];
  struct iot_memory_block_t * next;
} iot_memory_block_t;

typedef struct iot_data_struct_dummy_t
{
  iot_data_static_t s1;
  iot_data_static_t s2;
} iot_data_struct_dummy_t;

// Data size and alignment sanity checks

_Static_assert ((IOT_DATA_BLOCK_SIZE % 8) == 0, "IOT_DATA_BLOCK_SIZE not 8 byte multiple");
_Static_assert (sizeof (iot_data_value_t) == IOT_DATA_BLOCK_SIZE, "size of iot_data_value not equal to IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_map_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_map bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_pointer_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_pointer bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_vector bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_array_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_array bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_typecode_t) <= IOT_DATA_BLOCK_SIZE, "iot_typecode bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_memory_block_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_memory_block bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_data_vector bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_map_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_data_map bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_array_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_data_array bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_list_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_data_list bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_node_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_node bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_element_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_element bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_static_t) == sizeof (iot_data_value_base_t), "iot_data_static not equal to iot_data_value_base");
_Static_assert (sizeof (iot_data_list_static_t) == sizeof (iot_data_list_t), "iot_data_list_static not equal to iot_data_list");
_Static_assert (sizeof (iot_data_struct_dummy_t) == 2 * sizeof (iot_data_static_t), "iot_data_static_t structs not aligned for iot_data_static_t");
_Static_assert (sizeof (int) == sizeof (int32_t) || sizeof (int) == sizeof (int64_t), "int not a uint32_t or int64_t");

// Data cache usually disabled for debug builds as otherwise too difficult to trace leaks

#ifdef IOT_DATA_CACHE
static iot_block_t * iot_data_cache = NULL;
static iot_memory_block_t * iot_data_blocks = NULL;
#ifdef IOT_HAS_SPINLOCK
static pthread_spinlock_t iot_data_slock;
#endif
static pthread_mutex_t iot_data_mutex;
#endif

/* Static values for boolean and null types */

static iot_data_value_base_t iot_data_bool_true = { .value.bl = true, .base.type = IOT_DATA_BOOL, .base.element_type = IOT_DATA_INVALID, .base.key_type = IOT_DATA_INVALID, .base.constant = true };
static iot_data_value_base_t iot_data_bool_false = { .value.bl = false, .base.type = IOT_DATA_BOOL, .base.element_type = IOT_DATA_INVALID, .base.key_type = IOT_DATA_INVALID, .base.constant = true };
static iot_data_value_base_t iot_data_null = { .base.type = IOT_DATA_NULL, .base.element_type = IOT_DATA_INVALID, .base.key_type = IOT_DATA_INVALID, .base.constant = true };

extern void iot_data_map_dump (iot_data_t * map);
static void iot_node_free (iot_data_map_t * map, iot_node_t * node);
static iot_node_t * iot_node_next (iot_node_t * iter);
static iot_node_t * iot_node_prev (iot_node_t * iter);
static bool iot_node_add (iot_data_map_t * map, iot_data_t * key, iot_data_t * value);
static bool iot_node_remove (iot_data_map_t * map, const iot_data_t * key);
static iot_node_t * iot_node_find (const iot_node_t * node, const iot_data_t * key);

__attribute__((constructor)) static void iot_data_init (void);

static bool iot_data_string_cmp_fn (const iot_data_t * data, const void * arg)
{
  assert (data);
  return (arg && (data->type == IOT_DATA_STRING) && (strcmp (iot_data_string (data), (const char *) arg) == 0));
}
iot_data_cmp_fn iot_data_string_cmp = iot_data_string_cmp_fn;

uint32_t iot_data_type_size (iot_data_type_t type)
{
  return (type <= IOT_DATA_BINARY) ? iot_data_type_sizes[type] : 0u;
}

bool iot_data_alloc_heap (bool set)
{
  bool old = iot_data_alloc_from_heap;
  iot_data_alloc_from_heap = set;
  return old;
}

static void * iot_data_block_alloc (void)
{
  iot_block_t * data;
#ifdef IOT_DATA_CACHE
  iot_block_t * new_data_cache;
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_lock (&iot_data_slock);
#else
  pthread_mutex_lock (&iot_data_mutex);
#endif
#ifdef IOT_HAS_SPINLOCK
  while (iot_data_cache <= IOT_DATA_ALLOCATING)
  {
    new_data_cache = NULL;
    bool allocate = (iot_data_cache == NULL);
    if (allocate) iot_data_cache = IOT_DATA_ALLOCATING;
    pthread_spin_unlock (&iot_data_slock);
    pthread_mutex_lock (&iot_data_mutex);
#else
    bool allocate = (iot_data_cache == NULL);
    new_data_cache = NULL;
#endif
    if (allocate)
    {
      iot_memory_block_t * block = calloc (1, IOT_MEMORY_BLOCK_SIZE);
      block->next = iot_data_blocks;
      iot_data_blocks = block;

      uint8_t * iter = (uint8_t*) block->chunks;
      new_data_cache = (iot_block_t*) iter;
      for (unsigned i = 0; i < (IOT_DATA_BLOCKS - 1); i++)
      {
        iot_block_t * prev = (iot_block_t*) iter;
        iter += IOT_DATA_BLOCK_SIZE;
        prev->next = (iot_block_t*) iter;
      }
    }
#ifdef IOT_HAS_SPINLOCK
    pthread_mutex_unlock (&iot_data_mutex);
    pthread_spin_lock (&iot_data_slock);
    if (allocate) iot_data_cache = new_data_cache;
  }
#else
  if (allocate) iot_data_cache = new_data_cache;
#endif
  data = iot_data_cache;
  iot_data_cache = data->next;
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_unlock (&iot_data_slock);
#else
  pthread_mutex_unlock (&iot_data_mutex);
#endif
  data = (data) ? memset (data, 0, IOT_DATA_BLOCK_SIZE) : calloc (1, IOT_DATA_BLOCK_SIZE);
#else
  data = calloc (1, IOT_DATA_BLOCK_SIZE);
#endif
  return data;
}

static inline void iot_data_block_free (iot_block_t * block)
{
#ifdef IOT_DATA_CACHE
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_lock (&iot_data_slock);
  while (iot_data_cache == IOT_DATA_ALLOCATING)
  {
    pthread_spin_unlock (&iot_data_slock);
    pthread_mutex_lock (&iot_data_mutex);
    pthread_mutex_unlock (&iot_data_mutex);
    pthread_spin_lock (&iot_data_slock);
  }
#else
  pthread_mutex_lock (&iot_data_mutex);
#endif
  block->next = iot_data_cache;
  iot_data_cache = block;
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_unlock (&iot_data_slock);
#else
  pthread_mutex_unlock (&iot_data_mutex);
#endif
#else
  free (block);
#endif
}

static inline void iot_data_map_hash (iot_data_t * map, const iot_data_t * key, const iot_data_t * value)
{
  uint32_t key_hash = iot_data_hash (key);
  uint32_t val_hash = iot_data_hash (value);
  map->hash ^= key_hash;
  if (val_hash != key_hash) map->hash ^= val_hash; // Only apply value hash if different from key hash or hashes cancel out
}

uint32_t iot_data_hash (const iot_data_t * data)
{
  iot_data_t * da = (iot_data_t*) data;
  if (da && da->rehash)
  {
    da->hash = 0;
    da->rehash = false;
    if (da->type == IOT_DATA_VECTOR)
    {
      iot_data_vector_iter_t iter;
      iot_data_vector_iter (da, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        da->hash ^= iot_data_hash (iot_data_vector_iter_value (&iter));
      }
    }
    else if (da->type == IOT_DATA_LIST)
    {
      iot_data_list_iter_t iter;
      iot_data_list_iter (da, &iter);
      while (iot_data_list_iter_next (&iter))
      {
        da->hash ^= iot_data_hash (iot_data_list_iter_value (&iter));
      }
    }
    else // IOT_DATA_MAP
    {
      iot_data_map_iter_t iter;
      iot_data_map_iter (da, &iter);
      while (iot_data_map_iter_next (&iter))
      {
        iot_data_map_hash (da, iot_data_map_iter_key (&iter), iot_data_map_iter_value (&iter));
      }
    }
  }
  return da ? da->hash : 0u;
}

static inline void iot_element_free (iot_element_t * element)
{
  (element->heap) ? free (element) : iot_data_block_free ((iot_block_t*) element);
}

static iot_element_t * iot_element_alloc (void)
{
  bool heap = iot_data_alloc_from_heap;
  iot_element_t * element = heap ? calloc (1, IOT_DATA_BLOCK_SIZE) : iot_data_block_alloc ();
  element->heap = heap;
  return element;
}

static inline void iot_data_block_free_data (iot_data_t * data)
{
  (data->heap) ? free (data) : iot_data_block_free ((iot_block_t*) data);
}

static void iot_data_block_init (iot_data_t * data, iot_data_type_t type)
{
  atomic_store (&data->refs, 1u);
  data->type = type;
  data->element_type = (type >= IOT_DATA_ARRAY && type <= IOT_DATA_MAP) ? IOT_DATA_MULTI : IOT_DATA_INVALID;
  data->key_type = (type == IOT_DATA_MAP) ? IOT_DATA_MULTI : IOT_DATA_INVALID;
}

static void * iot_data_block_alloc_data (iot_data_type_t type)
{
  bool heap = iot_data_alloc_from_heap;
  iot_data_t * data = heap ? calloc (1, IOT_DATA_BLOCK_SIZE) : iot_data_block_alloc ();
  data->heap = heap;
  data->composed = IOT_DATA_IS_COMPOSED_TYPE (type);
  iot_data_block_init (data, type);
  return data;
}

static inline iot_data_value_t * iot_data_value_alloc (iot_data_type_t type, iot_data_ownership_t own)
{
  iot_data_value_t * val = iot_data_block_alloc_data (type);
  val->base.release = (own != IOT_DATA_REF);
  return val;
}

static void iot_data_fini (void)
{
#ifdef IOT_DATA_CACHE
  while (iot_data_blocks)
  {
    iot_memory_block_t * block = iot_data_blocks;
    iot_data_blocks = block->next;
    free (block);
  }
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_destroy (&iot_data_slock);
#endif
  pthread_mutex_destroy (&iot_data_mutex);
#endif
}

static void iot_data_init (void)
{
/*
  printf ("sizeof (iot_data_t): %zu\n", sizeof (iot_data_t));
  printf ("sizeof (iot_data_value_t): %zu\n", sizeof (iot_data_value_t));
  printf ("sizeof (iot_data_map_t): %zu\n", sizeof (iot_data_map_t));
  printf ("sizeof (iot_node_t): %zu\n", sizeof (iot_node_t));
  printf ("sizeof (iot_element_t): %zu\n", sizeof (iot_element_t));
  printf ("sizeof (iot_data_vector_t): %zu\n", sizeof (iot_data_vector_t));
  printf ("sizeof (iot_data_array_t): %zu\n", sizeof (iot_data_array_t));
  printf ("sizeof (iot_data_list_t): %zu\n", sizeof (iot_data_list_t));
  printf ("sizeof (iot_data_pointer_t): %zu\n", sizeof (iot_data_pointer_t));
  printf ("sizeof (iot_typecode_t): %zu\n", sizeof (iot_typecode_t));
  printf ("sizeof (iot_data_value_base_t): %zu\n", sizeof (iot_data_value_base_t));
  printf ("sizeof (iot_data_static_t): %zu\n", sizeof (iot_data_static_t));
  printf ("sizeof (iot_data_list_static_t): %zu\n", sizeof (iot_data_list_static_t));
  printf ("IOT_DATA_BLOCK_SIZE: %zu IOT_DATA_BLOCKS: %zu\n", IOT_DATA_BLOCK_SIZE, IOT_DATA_BLOCKS);
  printf ("IOT_DATA_VALUE_BUFF_SIZE: %zu\n", IOT_DATA_VALUE_BUFF_SIZE);
*/
#ifdef IOT_DATA_CACHE
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_init (&iot_data_slock, 0);
#endif
  pthread_mutex_init (&iot_data_mutex, NULL);
  iot_data_block_free (iot_data_block_alloc ());  // Initialize data cache
#endif
  iot_data_alloc_const_pointer (&iot_data_order, &iot_data_order);
  const char ** str = iot_data_const_strings;
  iot_data_static_t * ptr = (iot_data_static_t*) &iot_data_consts;
  while (*str) iot_data_alloc_const_string (ptr++, *str++);
  atexit (iot_data_fini);
}

iot_data_t * iot_data_add_ref (const iot_data_t * data)
{
  if (data) atomic_fetch_add (&((iot_data_t*) data)->refs, 1u);
  return (iot_data_t*) data;
}

uint32_t iot_data_ref_count (const iot_data_t * data)
{
  return data ? atomic_load (&((iot_data_t*) data)->refs) : 0u;
}

iot_data_type_t iot_data_name_type (const char * name)
{
  iot_data_type_t type = 0;
  while (type < IOT_DATA_INVALID)
  {
    if (strcasecmp (name, iot_data_type_names[type]) == 0) break;
    type++;
  }
  return type;
}

const char * iot_data_type_string (iot_data_type_t type)
{
  return iot_data_type_names[(type > IOT_DATA_INVALID) ? IOT_DATA_INVALID : type];
}

const char * iot_data_type_name (const iot_data_t * data)
{
  assert (data);
  return iot_data_type_string (data->type);
}

extern void iot_data_set_metadata (iot_data_t * data, iot_data_t * metadata, const iot_data_t * key)
{
  if (data && metadata && key)
  {
    if (data->base.meta == NULL) data->base.meta = iot_data_alloc_map (IOT_DATA_MULTI);
    iot_data_map_add (data->base.meta, iot_data_add_ref (key), metadata);
  }
}

extern const iot_data_t * iot_data_get_metadata (const iot_data_t * data, const iot_data_t * key)
{
  return (data && data->base.meta && key) ? iot_data_map_get (data->base.meta, key): NULL;
}

int iot_data_compare (const iot_data_t * v1, const iot_data_t * v2)
{
  if (v1 == v2) return 0;
  if (v1 == NULL || v2 == NULL) return (v1 == NULL) ? -1 : 1;
  if (v1->type != v2->type) return (v1->type < v2->type) ? -1 : 1;

  switch (v1->type)
  {
    case IOT_DATA_FLOAT32:
      return (((const iot_data_value_t*) v1)->value.f32 == ((const iot_data_value_t*) v2)->value.f32) ? 0 :
        (((const iot_data_value_t*) v1)->value.f32 < ((const iot_data_value_t*) v2)->value.f32) ? -1 : 1;
    case IOT_DATA_FLOAT64:
      return (((const iot_data_value_t*) v1)->value.f64 == ((const iot_data_value_t*) v2)->value.f64) ? 0 :
        (((const iot_data_value_t*) v1)->value.f64 < ((const iot_data_value_t*) v2)->value.f64) ? -1 : 1;
    case IOT_DATA_BOOL:
      return (((const iot_data_value_t*) v1)->value.bl == ((const iot_data_value_t*) v2)->value.bl) ? 0 :
        (((const iot_data_value_t*) v1)->value.bl) ? 1 : -1;
    case IOT_DATA_POINTER:
      return (((const iot_data_pointer_t*) v1)->value == ((const iot_data_pointer_t*) v2)->value) ? 0 :
        (((const iot_data_pointer_t*) v1)->value < ((const iot_data_pointer_t*) v2)->value) ? -1 : 1;
    case IOT_DATA_STRING:
      return (((const iot_data_value_t*) v1)->value.str == ((const iot_data_value_t*) v2)->value.str) ? 0 :
        strcmp (((const iot_data_value_t*) v1)->value.str, ((const iot_data_value_t*) v2)->value.str);
    case IOT_DATA_NULL: return 0;
    case IOT_DATA_BINARY:
    case IOT_DATA_ARRAY:
    {
      const iot_data_array_t * a1 = (const iot_data_array_t*) v1;
      const iot_data_array_t * a2 = (const iot_data_array_t*) v2;
      if (a1->length != a2->length) return a1->length < a2->length ? -1 : 1;
      if (v1->element_type != v2->element_type) return v1->element_type < v2->element_type ? -1 : 1;
      return memcmp (a1->data, a2->data, a1->length * iot_data_type_sizes[a1->base.element_type]);
    }
    case IOT_DATA_VECTOR:
    {
      int ret = 0;
      uint32_t size1 = ((const iot_data_vector_t*) v1)->size;
      uint32_t size2 = ((const iot_data_vector_t*) v2)->size;
      if (size1 != size2) return size1 < size2 ? -1 : 1;
      uint32_t hash1 = iot_data_hash (v1);
      uint32_t hash2 = iot_data_hash (v2);
      if (hash1 != hash2) return hash1 < hash2 ? -1 : 1;
      iot_data_vector_iter_t iter1;
      iot_data_vector_iter_t iter2;
      iot_data_vector_iter (v1, &iter1);
      iot_data_vector_iter (v2, &iter2);
      while (iot_data_vector_iter_next (&iter1) && iot_data_vector_iter_next (&iter2))
      {
        ret = iot_data_compare (iot_data_vector_iter_value (&iter1), iot_data_vector_iter_value (&iter2));
        if (ret != 0) break;
      }
      return ret;
    }
    case IOT_DATA_LIST:
    {
      int ret = 0;
      uint32_t len1 = iot_data_list_length (v1);
      uint32_t len2 = iot_data_list_length (v2);
      if (len1 != len2) return len1 < len2 ? -1 : 1;
      uint32_t hash1 = iot_data_hash (v1);
      uint32_t hash2 = iot_data_hash (v2);
      if (hash1 != hash2) return hash1 < hash2 ? -1 : 1;
      iot_data_list_iter_t iter1;
      iot_data_list_iter_t iter2;
      iot_data_list_iter (v1, &iter1);
      iot_data_list_iter (v2, &iter2);
      while (iot_data_list_iter_next (&iter1) && iot_data_list_iter_next (&iter2))
      {
        ret = iot_data_compare (iot_data_list_iter_value (&iter1), iot_data_list_iter_value (&iter2));
        if (ret != 0) break;
      }
      return ret;
    }
    case IOT_DATA_MAP:
    {
      int ret = 0;
      uint32_t size1 = ((const iot_data_map_t*) v1)->size;
      uint32_t size2 =((const iot_data_map_t*) v2)->size;
      if (size1 != size2) return size1 < size2 ? -1 : 1;
      uint32_t hash1 = iot_data_hash (v1);
      uint32_t hash2 = iot_data_hash (v2);
      if (hash1 != hash2) return hash1 < hash2 ? -1 : 1;
      iot_data_map_iter_t iter1;
      iot_data_map_iter_t iter2;
      iot_data_map_iter (v1, &iter1);
      iot_data_map_iter (v2, &iter2);
      while (iot_data_map_iter_next (&iter1) && iot_data_map_iter_next (&iter2))
      {
        ret = iot_data_compare (iot_data_map_iter_key (&iter1), iot_data_map_iter_key (&iter2));
        if (ret != 0) break;
        ret = iot_data_compare (iot_data_map_iter_value (&iter1), iot_data_map_iter_value (&iter2));
        if (ret != 0) break;
      }
      return ret;
    }
    default: break;
  }
  return (((const iot_data_value_t*) v1)->value.ui64 == ((const iot_data_value_t*) v2)->value.ui64) ? 0 :
    (((const iot_data_value_t*) v1)->value.ui64 < ((const iot_data_value_t*) v2)->value.ui64) ? -1 : 1;
}

bool iot_data_equal (const iot_data_t * v1, const iot_data_t * v2)
{
  return ((iot_data_hash (v1) == iot_data_hash (v2)) && iot_data_compare (v1, v2) == 0);
}

static bool iot_data_cast_val (const iot_data_union_t in, void * out, iot_data_type_t in_type, iot_data_type_t out_type)
{
#define VALUE() (dfloat ? uval.f64 : (usign ? (double) uval.ui64 : (double) uval.i64))
#define CAST(T) (*((T*) out) = dfloat ? (T) uval.f64 : (usign ? (T) uval.ui64 : (T) uval.i64))

  static const double max_vals[IOT_DATA_FLOAT64] = { INT8_MAX, UINT8_MAX, INT16_MAX, UINT16_MAX, INT32_MAX, UINT32_MAX, INT64_MAX, UINT64_MAX, FLT_MAX };
  static const double min_vals[IOT_DATA_FLOAT64] = { INT8_MIN, 0.0f, INT16_MIN, 0.0f, INT32_MIN, 0.0f, INT64_MIN, 0.0f , -FLT_MAX };

  bool usign = (in_type == IOT_DATA_UINT64) || (in_type == IOT_DATA_BOOL);
  bool dfloat = (in_type <= IOT_DATA_UINT32) || (in_type == IOT_DATA_FLOAT32) || (in_type == IOT_DATA_FLOAT64);
  iot_data_union_t uval;

  if ((in_type > IOT_DATA_BOOL) || (out_type > IOT_DATA_BOOL)) return false;

  // Cast up data value, note all integer types apart from 64 bit integers can be represented completely as a double
  switch (in_type)
  {
    case IOT_DATA_INT8: uval.f64 = in.i8; break;
    case IOT_DATA_UINT8: uval.f64 = in.ui8; break;
    case IOT_DATA_INT16: uval.f64 = in.i16; break;
    case IOT_DATA_UINT16: uval.f64 = in.ui16; break;
    case IOT_DATA_INT32: uval.f64 = in.i32; break;
    case IOT_DATA_UINT32: uval.f64 = in.ui32; break;
    case IOT_DATA_INT64: uval.i64 = in.i64; break;
    case IOT_DATA_UINT64: uval.ui64 = in.ui64; break;
    case IOT_DATA_FLOAT32: uval.f64 = in.f32; break;
    case IOT_DATA_FLOAT64: uval.f64 = in.f64; break;
    default: uval.ui64 = in.bl ? 1u : 0u; break;
  }
  // Check value within max/min for target type
  if ((out_type < IOT_DATA_FLOAT64) && (VALUE () > max_vals[out_type] || VALUE () < min_vals[out_type])) return false;

  // Cast to target type
  switch (out_type)
  {
    case IOT_DATA_INT8: CAST (int8_t); break;
    case IOT_DATA_UINT8: CAST (uint8_t); break;
    case IOT_DATA_INT16: CAST (int16_t); break;
    case IOT_DATA_UINT16: CAST (uint16_t); break;
    case IOT_DATA_INT32: CAST (int32_t); break;
    case IOT_DATA_UINT32: CAST (uint32_t); break;
    case IOT_DATA_INT64: CAST (int64_t); break;
    case IOT_DATA_UINT64: CAST (uint64_t); break;
    case IOT_DATA_FLOAT32: CAST (float); break;
    case IOT_DATA_FLOAT64: CAST (double); break;
    default: CAST (bool); break;
  }
  return true;
#undef CAST
#undef VALUE
}

bool iot_data_cast (const iot_data_t * data, iot_data_type_t type, void * val)
{
  assert (data && val);
  return iot_data_cast_val (((const iot_data_value_t *) data)->value, val, data->type, type);
}

iot_data_t * iot_data_transform (const iot_data_t * data, iot_data_type_t type)
{
  assert (data);
  iot_data_union_t out = { 0 };
  if (data->type == type) return iot_data_add_ref (data);
  if (iot_data_cast_val (((const iot_data_value_t*) data)->value, &out, data->type, type))
  {
    switch (type)
    {
      case IOT_DATA_INT8: return iot_data_alloc_i8 (out.i8);
      case IOT_DATA_UINT8: return iot_data_alloc_ui8 (out.ui8);
      case IOT_DATA_INT16: return iot_data_alloc_i16 (out.i16);
      case IOT_DATA_UINT16: return iot_data_alloc_ui16 (out.ui16);
      case IOT_DATA_INT32: return iot_data_alloc_i32 (out.i32);
      case IOT_DATA_UINT32: return iot_data_alloc_ui32 (out.ui32);
      case IOT_DATA_INT64: return iot_data_alloc_i64 (out.i64);
      case IOT_DATA_UINT64: return iot_data_alloc_ui64 (out.ui64);
      case IOT_DATA_FLOAT32: return iot_data_alloc_f32 (out.f32);
      case IOT_DATA_FLOAT64: return iot_data_alloc_f64 (out.f64);
      case IOT_DATA_BOOL: return iot_data_alloc_bool (out.bl);
      default: break;
    }
  }
  return NULL;
}

iot_data_t * iot_data_array_transform (const iot_data_t * array, iot_data_type_t type)
{
  assert (array && (array->type == IOT_DATA_ARRAY) && (type <= IOT_DATA_BOOL));
  uint32_t len = iot_data_array_length (array);
  void * data = NULL;
  uint32_t asize = 0u;

  if (len)
  {
    uint32_t esize = iot_data_type_sizes[type];
    data = calloc (len, esize);
    uint8_t * ptr = (uint8_t*) data;
    iot_data_array_iter_t iter;
    iot_data_array_iter (array, &iter);
    while (iot_data_array_iter_next (&iter))
    {
      const iot_data_union_t * element = iot_data_array_iter_value (&iter);
      if (iot_data_cast_val (*element, ptr, array->element_type, type))
      {
        ptr += esize;
        asize++;
      }
    }
    if (asize == 0)
    {
      free (data);
      data = NULL;
    }
  }
  return iot_data_alloc_array (data, asize, type, IOT_DATA_TAKE);
}

iot_data_t * iot_data_alloc_map (iot_data_type_t key_type)
{
  assert (key_type != IOT_DATA_NULL);
  iot_data_map_t * map = iot_data_block_alloc_data (IOT_DATA_MAP);
  map->base.key_type = key_type;
  return (iot_data_t*) map;
}

iot_data_t * iot_data_alloc_typed_map (iot_data_type_t key_type, iot_data_type_t element_type)
{
  iot_data_t * map = iot_data_alloc_map (key_type);
  map->element_type = element_type;
  return map;
}

iot_data_type_t iot_data_map_type (const iot_data_t * map)
{
  assert (map);
  return map->element_type;
}

iot_data_t * iot_data_alloc_vector (uint32_t size)
{
  iot_data_vector_t * vector = iot_data_block_alloc_data (IOT_DATA_VECTOR);
  if (size)
  {
    vector->size = size;
    vector->values = calloc (size, sizeof (iot_data_t *));
  }
  return (iot_data_t*) vector;
}

iot_data_t * iot_data_alloc_typed_vector (uint32_t size, iot_data_type_t element_type)
{
  iot_data_t * vector = iot_data_alloc_vector (size);
  vector->element_type = element_type;
  return vector;
}

iot_data_type_t iot_data_vector_type (const iot_data_t * vector)
{
  assert (vector);
  return vector->element_type;
}

iot_data_t * iot_data_alloc_pointer (void * ptr, iot_data_free_fn free_fn)
{
  iot_data_pointer_t * pointer = iot_data_block_alloc_data (IOT_DATA_POINTER);
  pointer->free_fn = free_fn;
  pointer->value = ptr;
  pointer->base.hash = (uint32_t) (intptr_t) ptr;
  return (iot_data_t*) pointer;
}

iot_data_t * iot_data_alloc_list (void)
{
  return (iot_data_t*) iot_data_block_alloc_data (IOT_DATA_LIST);
}

iot_data_t * iot_data_alloc_const_list (iot_data_list_static_t * data)
{
  iot_data_list_t * val = (iot_data_list_t*) data;
  memset (data, 0, sizeof (*data));
  iot_data_block_init (&val->base, IOT_DATA_LIST);
  val->base.composed = true;
  val->base.constant = true;
  return (iot_data_t*) val;
}

iot_data_t * iot_data_alloc_typed_list (iot_data_type_t element_type)
{
  iot_data_t * list = iot_data_block_alloc_data (IOT_DATA_LIST);
  list->element_type = element_type;
  return list;
}

iot_data_type_t iot_data_list_type (const iot_data_t * list)
{
  assert (list);
  return list->element_type;
}

uint32_t iot_data_list_length (const iot_data_t * list)
{
  assert (list);
  const iot_data_list_t * impl = (const iot_data_list_t*) list;
  return impl->head ? impl->head->length : 0;
}

static iot_element_t * iot_data_list_find_element (const iot_data_list_t * list, iot_data_cmp_fn cmp, const void * arg)
{
  assert (list && cmp);
  iot_element_t * iter = list->tail;
  while (iter)
  {
    if (cmp (iter->value, arg)) break;
    iter = iter->next;
  }
  return iter;
}

const iot_data_t * iot_data_list_find (const iot_data_t * list, iot_data_cmp_fn cmp, const void * arg)
{
  const iot_element_t * element = iot_data_list_find_element ((const iot_data_list_t*) list, cmp, arg);
  return element ? element->value : NULL;
}

static void iot_data_list_remove_element (iot_data_list_t * list, iot_element_t * element)
{
  iot_data_t * value = element->value;
  if (element == list->head) // Remove from head of list
  {
    iot_data_list_head_pop ((iot_data_t*) list);
  }
  else if (element == list->tail) // Remove from tail of list
  {
    iot_data_list_tail_pop ((iot_data_t*) list);
  }
  else // Remove from middle of list
  {
    element->next->prev = element->prev;
    element->prev->next = element->next;
    list->head->length--;
    list->base.rehash = true;
    iot_element_free (element);
  }
  iot_data_free (value);
}

extern bool iot_data_list_remove (iot_data_t * list, iot_data_cmp_fn cmp, const void * arg)
{
  assert (list && cmp);
  iot_data_list_t * impl = (iot_data_list_t*) list;
  iot_element_t * element = iot_data_list_find_element (impl, cmp, arg);
  if (element) iot_data_list_remove_element (impl, element);
  return (element != NULL);
}

void iot_data_list_iter (const iot_data_t * list, iot_data_list_iter_t * iter)
{
  assert (iter && list && list->type == IOT_DATA_LIST);
  iter->_list = (const iot_data_list_t*) list;
  iter->_element = NULL;
}

bool iot_data_list_iter_next (iot_data_list_iter_t * iter)
{
  assert (iter);
  iter->_element = (iter->_element) ? iter->_element->next : iter->_list->tail;
  return (iter->_element != NULL);
}

bool iot_data_list_iter_has_next (const iot_data_list_iter_t * iter)
{
  assert (iter);
  return ((iter->_element == NULL) ||  iter->_element->next);
}

bool iot_data_list_iter_prev (iot_data_list_iter_t * iter)
{
  assert (iter);
  iter->_element = (iter->_element) ? iter->_element->prev : iter->_list->head;
  return (iter->_element != NULL);
}

const iot_data_t * iot_data_list_iter_value (const iot_data_list_iter_t * iter)
{
  assert (iter);
  return (iter->_element) ? iter->_element->value : NULL;
}

const char * iot_data_list_iter_string_value (const iot_data_list_iter_t * iter)
{
  assert (iter);
  return (iter->_element) ? iot_data_string (iter->_element->value) : NULL;
}

const void * iot_data_list_iter_pointer_value (const iot_data_list_iter_t * iter)
{
  assert (iter);
  return (iter->_element) ? iot_data_pointer (iter->_element->value) : NULL;
}

iot_data_t * iot_data_list_iter_replace (const iot_data_list_iter_t * iter, iot_data_t * value)
{
  assert (iter && iter->_list && value && (iter->_list->base.element_type == IOT_DATA_MULTI || iter->_list->base.element_type == value->type));
  iot_data_t * res = (iter->_element) ? iter->_element->value : NULL;
  if (res)
  {
    iot_data_t * base = (iot_data_t*) &iter->_list->base;
    base->rehash = true;
    iter->_element->value = value;
  }
  return res;
}

bool iot_data_list_iter_remove (iot_data_list_iter_t * iter)
{
  assert (iter && iter->_list);
  iot_data_list_t * impl = (iot_data_list_t*) iter->_list;
  iot_element_t * element = iter->_element;
  if (element)
  {
    iot_data_list_iter_prev (iter);
    iot_data_list_remove_element (impl, element);
  }
  return element != NULL;
}

void iot_data_list_tail_push (iot_data_t * list, iot_data_t * value)
{
  assert (list && value && (list->element_type == IOT_DATA_MULTI || list->element_type == value->type)); // Check element type matches for fixed type list
  iot_data_list_t * impl = (iot_data_list_t*) list;
  iot_element_t * element = impl->tail;
  impl->tail = iot_element_alloc ();
  impl->tail->value = value;
  impl->tail->next = element;
  if (element)
  {
    element->prev = impl->tail;
  }
  else
  {
    impl->head = impl->tail;
  }
  impl->head->length++;
  list->hash ^= iot_data_hash (value);
}

iot_data_t * iot_data_list_tail_pop (iot_data_t * list)
{
  iot_data_t * value = NULL;
  assert (list);
  iot_data_list_t * impl = (iot_data_list_t*) list;
  iot_element_t * element = impl->tail;
  if (element)
  {
    value = element->value;
    impl->tail = element->next;
    if (impl->tail)
    {
      impl->tail->prev = NULL;
      impl->head->length--;
    }
    else
    {
      impl->head = NULL;
    }
    list->rehash = true;
    iot_element_free (element);
  }
  return value;
}

void iot_data_list_head_push (iot_data_t * list, iot_data_t * value)
{
  assert (list && value && (list->element_type == IOT_DATA_MULTI || list->element_type == value->type)); // Check element type matches for fixed type list
  iot_data_list_t * impl = (iot_data_list_t*) list;
  iot_element_t * element = impl->head;
  impl->head = iot_element_alloc ();
  impl->head->value = value;
  impl->head->prev = element;
  if (element)
  {
    impl->head->length = element->length + 1;
    element->next = impl->head;
  }
  else
  {
    impl->tail = impl->head;
    impl->head->length = 1;
  }
  list->hash ^= iot_data_hash (value);
}

iot_data_t * iot_data_list_head_pop (iot_data_t * list)
{
  assert (list);
  iot_data_t * value = NULL;
  iot_data_list_t * impl = (iot_data_list_t*) list;
  iot_element_t * element = impl->head;
  if (element)
  {
    value = element->value;
    impl->head = element->prev;
    if (impl->head)
    {
      impl->head->next = NULL;
      impl->head->length = element->length - 1;
    }
    else
    {
      impl->tail = NULL;
    }
    list->rehash = true;
    iot_element_free (element);
  }
  return value;
}

iot_data_type_t iot_data_type (const iot_data_t * data)
{
  assert (data);
  return data->type;
}

bool iot_data_is_of_type (const iot_data_t * data, iot_data_type_t type)
{
  return (data && (data->type == type));
}

bool iot_data_is_static (const iot_data_t * data)
{
  return (data && data->constant);
}

bool iot_data_is_nan (const iot_data_t *data)
{
  assert (data);
  iot_data_type_t type = iot_data_type (data);
  if (type != IOT_DATA_FLOAT32 && type != IOT_DATA_FLOAT64) return false;
  return type == IOT_DATA_FLOAT32 ? iot_data_f32 (data) != iot_data_f32 (data) : iot_data_f64 (data) != iot_data_f64 (data);
}

static void iot_data_cache_add (iot_data_t * cache, iot_data_t ** data)
{
  iot_data_t * cached = (iot_data_t*) iot_data_map_get (cache, *data);
  if (cached)
  {
    iot_data_free (*data);
    iot_data_add_ref (cached);
    *data = cached;
  }
  else
  {
    iot_data_map_add (cache, iot_data_add_ref (*data), iot_data_add_ref (*data));
    if ((*data)->composed)
    {
      iot_data_compress_with_cache (*data, cache);
    }
  }
}

void iot_data_compress_with_cache (iot_data_t * data, iot_data_t * cache)
{
  assert (data && cache && (cache->key_type == IOT_DATA_MULTI) && (cache->element_type == IOT_DATA_MULTI));
  if (data->composed)
  {
    if (data->type == IOT_DATA_VECTOR)
    {
      iot_data_vector_iter_t iter;
      iot_data_vector_iter (data, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        iot_data_cache_add (cache, &(iter._vector->values[iter._index]));
      }
    }
    else if (data->type == IOT_DATA_LIST)
    {
      iot_data_list_iter_t iter;
      iot_data_list_iter (data, &iter);
      while (iot_data_list_iter_next (&iter))
      {
        iot_data_cache_add (cache, &(iter._element->value));
      }
    }
    else // Map
    {
      iot_data_map_iter_t iter;
      iot_data_map_iter (data, &iter);
      while (iot_data_map_iter_next (&iter))
      {
        iot_data_cache_add (cache, &(iter._node->value));
        iot_data_cache_add (cache, &(iter._node->key));
      }
    }
  }
}

void iot_data_compress (iot_data_t * data)
{
  iot_data_t * cache = iot_data_alloc_typed_map (IOT_DATA_MULTI, IOT_DATA_MULTI);
  iot_data_compress_with_cache (data, cache);
  iot_data_free (cache);
}

const void * iot_data_address (const iot_data_t * data)
{
  if (data)
  {
    if (data->type == IOT_DATA_POINTER) return ((const iot_data_pointer_t *) data)->value;
    else if (data->type < IOT_DATA_BINARY) return (const void *) &(((const iot_data_value_t *) data)->value);
    else if (data->type == IOT_DATA_BINARY || data->type == IOT_DATA_ARRAY) return ((const iot_data_array_t *) data)->data;
  }
  return NULL;
}

void iot_data_free (iot_data_t * data)
{
  if (data && !data->constant && ((uint32_t) atomic_fetch_sub (&data->refs, 1u) <= 1u))
  {
    if (data->base.meta) iot_data_free (data->base.meta);
    switch (data->type)
    {
      case IOT_DATA_STRING:
      {
        iot_data_value_t * val = (iot_data_value_t*) data;
        if (data->release && (val->value.str != val->buff))
        {
          data->release_block ? iot_data_block_free ((iot_block_t*) val->value.str) : free (val->value.str);
        }
        break;
      }
      case IOT_DATA_BINARY:
      case IOT_DATA_ARRAY:
      {
        if (data->release) free (((iot_data_array_t*) data)->data);
        break;
      }
      case IOT_DATA_MAP:
      {
        iot_node_free ((iot_data_map_t*) data, ((iot_data_map_t*) data)->tree);
        break;
      }
      case IOT_DATA_VECTOR:
      {
        iot_data_vector_t * vector = (iot_data_vector_t*) data;
        for (uint32_t i = 0; i < vector->size; i++)
        {
          iot_data_free (vector->values[i]);
        }
        free (vector->values);
        vector->size = 0;
        break;
      }
      case IOT_DATA_POINTER:
      {
        iot_data_pointer_t * pointer = (iot_data_pointer_t*) data;
        if (pointer->free_fn) (pointer->free_fn) (pointer->value);
        break;
      }
      case IOT_DATA_LIST:
      {
        iot_element_t * iter = ((iot_data_list_t*) data)->tail;
        while (iter)
        {
          iot_element_t * element = iter;
          iter = iter->next;
          iot_data_free (element->value);
          iot_element_free (element);
        }
        break;
      }
      default: break;
    }
    iot_data_block_free_data (data);
  }
}

iot_data_t * iot_data_alloc_from_string (iot_data_type_t type, const char * value)
{
  static const char * fmt [] =  { "%"SCNd8,"%"SCNu8,"%"SCNd16,"%"SCNu16,"%"SCNd32,"%"SCNu32,"%"SCNd64,"%"SCNu64,"%f","%lf" };
  assert (value && ((type == IOT_DATA_STRING) || strlen (value)));

  if (type < IOT_DATA_BOOL)
  {
    iot_data_union_t un = { 0 };
    if (sscanf (value, fmt[type], &un) == 1)
    {
      switch (type)
      {
        case IOT_DATA_INT8: return iot_data_alloc_i8 (un.i8);
        case IOT_DATA_UINT8: return iot_data_alloc_ui8 (un.ui8);
        case IOT_DATA_INT16: return iot_data_alloc_i16 (un.i16);
        case IOT_DATA_UINT16: return iot_data_alloc_ui16 (un.ui16);
        case IOT_DATA_INT32: return iot_data_alloc_i32 (un.i32);
        case IOT_DATA_UINT32: return iot_data_alloc_ui32 (un.ui32);
        case IOT_DATA_INT64: return iot_data_alloc_i64 (un.i64);
        case IOT_DATA_UINT64: return iot_data_alloc_ui64 (un.ui64);
        case IOT_DATA_FLOAT32: return iot_data_alloc_f32 (un.f32);
        case IOT_DATA_FLOAT64: return iot_data_alloc_f64 (un.f64);
        default: assert (0);
      }
    }
  }
  if (type == IOT_DATA_BOOL) return iot_data_alloc_bool (value[0] == 't' || value[0] == 'T');
  if (type == IOT_DATA_STRING) return iot_data_alloc_string (value, IOT_DATA_COPY);
  return NULL;
}

static void iot_data_inc_dec (iot_data_t * data, bool inc)
{
  assert (data);
  iot_data_union_t * val = &((iot_data_value_t*) data)->value;
  switch (data->type)
  {
    case IOT_DATA_INT8: inc ? val->i8++ : val->i8--; break;
    case IOT_DATA_UINT8: inc ? val->ui8++ : val->ui8--; break;
    case IOT_DATA_INT16: inc ? val->i16++ : val->i16--; break;
    case IOT_DATA_UINT16: inc ? val->ui16++ : val->ui16--; break;
    case IOT_DATA_INT32: inc ? val->i32++ : val->i32--; break;
    case IOT_DATA_UINT32: inc ? val->ui32++ : val->ui32--; break;
    case IOT_DATA_INT64: inc ? val->i64++ : val->i64--; break;
    case IOT_DATA_UINT64: inc ? val->ui64++ : val->ui64--; break;
    case IOT_DATA_FLOAT32: inc ? val->f32++ : val->f32--; break;
    case IOT_DATA_FLOAT64: inc ? val->f64++ : val->f64--; break;
    default: break;
  }
}

void iot_data_increment (iot_data_t * data)
{
  iot_data_inc_dec (data, true);
}

void iot_data_decrement (iot_data_t * data)
{
  iot_data_inc_dec (data, false);
}

iot_data_t * iot_data_alloc_from_strings (const char * type, const char * value)
{
  return iot_data_alloc_from_string (iot_data_name_type (type), value);
}

iot_data_t * iot_data_alloc_i8 (int8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT8, false);
  data->value.i8 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui8 (uint8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT8, false);
  data->value.ui8 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i16 (int16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT16, false);
  data->value.i16 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui16 (uint16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT16, false);
  data->value.ui16 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i32 (int32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT32, false);
  data->value.i32 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui32 (uint32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT32, false);
  data->value.ui32 = val;
  data->base.hash = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i64 (int64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT64, false);
  data->value.i64 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui64 (uint64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT64, false);
  data->value.ui64 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_const_ui64 (iot_data_static_t * data, uint64_t val)
{
  iot_data_value_base_t * bval = (iot_data_value_base_t*) data;
  memset (data, 0, sizeof (*data));
  iot_data_block_init (&bval->base, IOT_DATA_UINT64);
  bval->value.ui64 = val;
  bval->base.hash = (uint32_t) val;
  bval->base.constant = true;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f32 (float val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT32, false);
  data->value.f32 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f64 (double val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT64, false);
  data->value.f64 = val;
  data->base.hash = (uint32_t) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_bool (bool val)
{
  return val ? &iot_data_bool_true.base : &iot_data_bool_false.base;
}

iot_data_t * iot_data_alloc_null (void)
{
  return &iot_data_null.base;
}

iot_data_t * iot_data_alloc_uuid_string (void)
{
  char *uuid_str = malloc (UUID_STR_LEN);
  iot_uuid_t uuid;
  iot_uuid_generate (uuid);
  iot_uuid_unparse (uuid, uuid_str);
  return iot_data_alloc_string (uuid_str, IOT_DATA_TAKE);
}

iot_data_t * iot_data_alloc_uuid (void)
{
  iot_uuid_t uuid;
  iot_uuid_generate (uuid);
  return iot_data_alloc_array (uuid, sizeof (iot_uuid_t), IOT_DATA_UINT8, IOT_DATA_COPY);
}

iot_data_t * iot_data_alloc_const_string (iot_data_static_t * data, const char * str)
{
  iot_data_value_base_t * val = (iot_data_value_base_t*) data;
  memset (data, 0, sizeof (*data));
  iot_data_block_init (&val->base, IOT_DATA_STRING);
  val->value.str = (char*) str;
  val->base.hash = iot_hash (str);
  val->base.constant = true;
  return (iot_data_t*) val;
}

iot_data_t * iot_data_alloc_const_pointer (iot_data_static_t * data, const void * ptr)
{
  iot_data_pointer_t * val = (iot_data_pointer_t*) data;
  memset (data, 0, sizeof (*data));
  iot_data_block_init (&val->base, IOT_DATA_POINTER);
  val->value = (void*) ptr;
  val->base.hash =  (uint32_t) (intptr_t) ptr;
  val->base.constant = true;
  return (iot_data_t*) val;
}

iot_data_t * iot_data_alloc_string (const char * val, iot_data_ownership_t ownership)
{
  assert (val);
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_STRING, ownership);
  data->value.str = (char*) val;
  data->base.hash = iot_hash (val);
  if (ownership == IOT_DATA_COPY)
  {
    size_t len = strlen (val);
    if (len < IOT_DATA_VALUE_BUFF_SIZE) // If string small enough save in iot_data_value_t buffer
    {
      data->value.str = data->buff;
      strcpy (data->buff, val);
    }
    else if (len < IOT_DATA_BLOCK_SIZE) // If less than size of block save in block
    {
      data->value.str = iot_data_block_alloc ();
      strcpy (data->value.str, val);
      data->base.release_block = true;
    }
    else // Allocate as last resort
    {
      data->value.str = strdup (val);
    }
  }
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_string_fmt (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  size_t n = 1u + (size_t) vsnprintf (NULL, 0, format, args);
  va_end (args);

  char * str = malloc (n);
  va_start (args, format);
  vsprintf (str, format, args);
  va_end (args);
  return iot_data_alloc_string (str, IOT_DATA_TAKE);
}

extern iot_data_t * iot_data_alloc_binary (void * data, uint32_t length, iot_data_ownership_t ownership)
{
  iot_data_t * bin = iot_data_alloc_array (data, length, IOT_DATA_UINT8, ownership);
  bin->type = IOT_DATA_BINARY;
  return bin;
}

extern void * iot_data_binary_take (iot_data_t * data, uint32_t * len)
{
  void * ret;
  assert (data && len && (data->type == IOT_DATA_BINARY || data->type == IOT_DATA_ARRAY));
  iot_data_array_t * array = (iot_data_array_t*) data;
  *len = iot_data_array_size (data);
  if (array->base.release && (atomic_load (&(data)->refs) == 1u))
  {
    ret = array->data;
    array->data = NULL;
    array->length = 0u;
  }
  else
  {
    ret = malloc (array->length);
    memcpy (ret, array->data, *len);
  }
  return ret;
}


extern void iot_data_array_to_binary (iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_ARRAY || data->type == IOT_DATA_BINARY) && data->element_type == IOT_DATA_UINT8);
  data->type = IOT_DATA_BINARY;
}

extern void iot_data_binary_to_array (iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_ARRAY || data->type == IOT_DATA_BINARY) && data->element_type == IOT_DATA_UINT8);
  data->type = IOT_DATA_ARRAY;
}

extern iot_data_t * iot_data_binary_from_string (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_STRING));
  const char * ptr = iot_data_string (data);
  return iot_data_alloc_binary ((void*) ptr, strlen (ptr), IOT_DATA_COPY);
}

extern iot_data_t * iot_data_string_from_binary (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_BINARY));
  uint32_t len = iot_data_array_length (data);
  char * str = malloc (len + 1u);
  memcpy (str, iot_data_address (data), len);
  str[len] = '\0';
  return iot_data_alloc_string (str, IOT_DATA_TAKE);
}

extern iot_data_t * iot_data_alloc_array (void * data, uint32_t length, iot_data_type_t type, iot_data_ownership_t ownership)
{
  assert ((type <= IOT_DATA_BOOL) && ((length > 0 && data != NULL) || length == 0));
  iot_data_array_t * array = iot_data_block_alloc_data (IOT_DATA_ARRAY);
  size_t size = iot_data_type_sizes[type] * length;
  array->base.element_type = type;
  array->data = length ? data : NULL;
  array->length = length;
  array->base.hash = data ? iot_hash_data (data, size) : 0;
  array->base.release = data ? (ownership != IOT_DATA_REF) : false;
  if (length && data && (ownership == IOT_DATA_COPY))
  {
    array->data = malloc (size);
    memcpy (array->data, data, size);
  }
  return (iot_data_t*) array;
}

extern iot_data_type_t iot_data_array_type (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY || array->type == IOT_DATA_BINARY));
  return array->element_type;
}

extern bool iot_data_array_is_of_type (const iot_data_t * array, iot_data_type_t type)
{
  return (array && (array->type == IOT_DATA_ARRAY || array->type == IOT_DATA_BINARY) && (array->element_type == type));
}

extern uint32_t iot_data_array_size (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY || array->type == IOT_DATA_BINARY));
  const iot_data_array_t * arr = (const iot_data_array_t *) array;
  return (arr->length * iot_data_type_sizes[arr->base.element_type]);
}

extern uint32_t iot_data_array_length (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY || array->type == IOT_DATA_BINARY));
  return ((const iot_data_array_t*) array)->length;
}

iot_data_t * iot_data_alloc_array_from_base64 (const char * value)
{
  iot_data_t * result = NULL;
  assert (value);
  size_t  len = iot_b64_maxdecodesize (value);
  uint8_t * data = malloc (len);

  if (iot_b64_decode (value, data, &len))
  {
    result = iot_data_alloc_array (data, (uint32_t) len, IOT_DATA_UINT8, IOT_DATA_TAKE);
  }
  else
  {
    free (data);
  }
  return result;
}

int8_t iot_data_i8 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT8));
  return ((const iot_data_value_t*) data)->value.i8;
}

uint8_t iot_data_ui8 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT8));
  return ((const iot_data_value_t*) data)->value.ui8;
}

int16_t iot_data_i16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT16));
  return ((const iot_data_value_t*) data)->value.i16;
}

uint16_t iot_data_ui16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT16));
  return ((const iot_data_value_t*) data)->value.ui16;
}

int32_t iot_data_i32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT32));
  return ((const iot_data_value_t*) data)->value.i32;
}

uint32_t iot_data_ui32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT32));
  return ((const iot_data_value_t*) data)->value.ui32;
}

int64_t iot_data_i64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT64));
  return ((const iot_data_value_t*) data)->value.i64;
}

uint64_t iot_data_ui64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT64));
  return ((const iot_data_value_t*) data)->value.ui64;
}

float iot_data_f32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT32));
  return ((const iot_data_value_t*) data)->value.f32;
}

double iot_data_f64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT64));
  return ((const iot_data_value_t*) data)->value.f64;
}

bool iot_data_bool (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_BOOL));
  return ((const iot_data_value_t*) data)->value.bl;
}

const char * iot_data_string (const iot_data_t * data)
{
  assert (data);
  return (data->type == IOT_DATA_STRING) ? ((const iot_data_value_t*) data)->value.str : NULL;
}

const void * iot_data_pointer (const iot_data_t * data)
{
  assert (data);
  return (data->type == IOT_DATA_POINTER) ? ((const iot_data_pointer_t*) data)->value : NULL;
}

bool iot_data_map_remove (iot_data_t * map, const iot_data_t * key)
{
  bool ret = false;
  assert (map && (map->type == IOT_DATA_MAP));
  if (key)
  {
    iot_data_map_t * mp = (iot_data_map_t*) map;
    if ((ret = iot_node_remove (mp, key)))
    {
      mp->base.rehash = true;
      mp->size--;
    }
  }
  return ret;
}

void iot_data_string_map_add (iot_data_t * map, const char * key, iot_data_t * val)
{
  assert (key);
  iot_data_map_add (map, iot_data_alloc_string (key, IOT_DATA_REF), val);
}

bool iot_data_string_map_remove (iot_data_t * map, const char * key)
{
  bool ret = false;
  if (key)
  {
    iot_data_static_t skey;
    iot_data_alloc_const_string (&skey, key);
    ret = iot_data_map_remove (map, IOT_DATA_STATIC (&skey));
  }
  return ret;
}

void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val)
{
  assert (map && key && val);
  assert (map->type == IOT_DATA_MAP);
  assert (key->type == map->key_type || map->key_type == IOT_DATA_MULTI);
  assert (map->element_type == val->type || map->element_type == IOT_DATA_MULTI);
  if (iot_node_add ((iot_data_map_t*) map, key, val))
  {
    ((iot_data_map_t*) map)->size++;
  }
}

bool iot_data_map_add_unused (iot_data_t * map, iot_data_t * key, iot_data_t * val)
{
  assert (map && key && val);
  assert (map->type == IOT_DATA_MAP);
  assert (key->type == map->key_type || map->key_type == IOT_DATA_MULTI);
  assert (map->element_type == val->type || map->element_type == IOT_DATA_MULTI);
  bool add = iot_node_find (((const iot_data_map_t*) map)->tree, key) == NULL;
  if (add)
  {
    iot_node_add ((iot_data_map_t *) map, key, val);
    ((iot_data_map_t *) map)->size++;
  }
  return add;
}

void iot_data_map_merge (iot_data_t * map, const iot_data_t * add)
{
  assert (map && (map->type == IOT_DATA_MAP));
  if (add)
  {
    assert (add->type == IOT_DATA_MAP);
    assert (map->key_type == IOT_DATA_MULTI || (map->key_type == add->key_type));
    iot_data_map_iter_t iter;
    iot_data_map_iter (add, &iter);
    while (iot_data_map_iter_next (&iter))
    {
      iot_data_map_add (map, iot_data_add_ref (iot_data_map_iter_key (&iter)), iot_data_add_ref (iot_data_map_iter_value (&iter)));
    }
  }
}

uint32_t iot_data_map_size (const iot_data_t * map)
{
  assert (map && (map->type == IOT_DATA_MAP));
  return ((const iot_data_map_t*) map)->size;
}

bool iot_data_map_base64_to_array (const iot_data_t * map, const iot_data_t * key)
{
  assert (map && (map->type == IOT_DATA_MAP));
  assert (key && key->type == map->key_type);

  iot_data_t * array = NULL;
  const iot_data_map_t * mp = (const iot_data_map_t*) map;
  iot_node_t * node = iot_node_find (mp->tree, key);

  if (node && (node->value->type == IOT_DATA_STRING))
  {
    array = iot_data_alloc_array_from_base64 (((iot_data_value_t*) node->value)->value.str);
    if (array)
    {
      iot_data_free (node->value);
      node->value = array;
    }
  }
  return (array != NULL);
}

const iot_data_t * iot_data_map_get (const iot_data_t * map, const iot_data_t * key)
{
  assert (map && key && (map->type == IOT_DATA_MAP));
  const iot_node_t * node = iot_node_find (((const iot_data_map_t*) map)->tree, key);
  return node ? node->value : NULL;
}

const iot_data_t * iot_data_map_get_typed (const iot_data_t * map, const iot_data_t * key, iot_data_type_t type)
{
  assert (map && key && (map->type == IOT_DATA_MAP));
  const iot_node_t * node = iot_node_find (((const iot_data_map_t*) map)->tree, key);
  return (node && (node->value->type == type)) ? node->value : NULL;
}

const iot_data_t * iot_data_string_map_get (const iot_data_t * map, const char * key)
{
  assert (map && key);
  iot_data_static_t skey;
  iot_data_alloc_const_string (&skey, key);
  return iot_data_map_get (map, IOT_DATA_STATIC (&skey));
}

const char * iot_data_map_get_string (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_STRING)) ? iot_data_string (data) : NULL;
}

bool iot_data_map_get_number (const iot_data_t * map, const iot_data_t * key, iot_data_type_t type, void * val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return data ? iot_data_cast (data, type, val) : false;
}

extern bool iot_data_map_get_int (const iot_data_t * map, const iot_data_t * key, int * val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return data ? iot_data_cast (data, (sizeof (int) == sizeof (int32_t)) ? IOT_DATA_INT32 : IOT_DATA_INT64, val) : false;
}

int64_t iot_data_map_get_i64 (const iot_data_t * map, const iot_data_t * key, int64_t default_val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_INT64)) ? iot_data_i64 (data) : default_val;
}

uint64_t iot_data_map_get_ui64 (const iot_data_t * map, const iot_data_t * key, uint64_t default_val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_UINT64)) ? iot_data_ui64 (data) : default_val;
}

bool iot_data_map_get_bool (const iot_data_t * map, const iot_data_t * key, bool default_val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_BOOL)) ? iot_data_bool (data) : default_val;
}

double iot_data_map_get_f64 (const iot_data_t * map, const iot_data_t * key, double default_val)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_FLOAT64)) ? iot_data_f64 (data) : default_val;
}

const void * iot_data_map_get_pointer (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_POINTER)) ? iot_data_pointer (data) : NULL);
}

const iot_data_t * iot_data_map_get_vector (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_VECTOR)) ? data : NULL);
}

const iot_data_t * iot_data_map_get_map (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_MAP)) ? data : NULL);
}

const iot_data_t * iot_data_map_get_array (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_ARRAY)) ? data : NULL);
}

const iot_data_t * iot_data_map_get_list (const iot_data_t * map, const iot_data_t * key)
{
  const iot_data_t * data = iot_data_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_LIST)) ? data : NULL);
}

const char * iot_data_string_map_get_string (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_STRING)) ? iot_data_string (data) : NULL;
}

bool iot_data_string_map_get_number (const iot_data_t * map, const char * key, iot_data_type_t type, void * val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return data ? iot_data_cast (data, type, val) : false;
}

extern bool iot_data_string_map_get_int (const iot_data_t * map, const char * key, int * val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return data ? iot_data_cast (data, (sizeof (int) == sizeof (int32_t)) ? IOT_DATA_INT32 : IOT_DATA_INT64, val) : false;
}

int64_t iot_data_string_map_get_i64 (const iot_data_t * map, const char * key, int64_t default_val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_INT64)) ? iot_data_i64 (data) : default_val;
}

uint64_t iot_data_string_map_get_ui64 (const iot_data_t * map, const char * key, uint64_t default_val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_UINT64)) ? iot_data_ui64 (data) : default_val;
}

bool iot_data_string_map_get_bool (const iot_data_t * map, const char * key, bool default_val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_BOOL)) ? iot_data_bool (data) : default_val;
}

double iot_data_string_map_get_f64 (const iot_data_t * map, const char * key, double default_val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_FLOAT64)) ? iot_data_f64 (data) : default_val;
}

const void * iot_data_string_map_get_pointer (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_POINTER)) ? iot_data_pointer (data) : NULL);
}

const iot_data_t * iot_data_string_map_get_vector (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_VECTOR)) ? data : NULL);
}

const iot_data_t * iot_data_string_map_get_map (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_MAP)) ? data : NULL);
}

const iot_data_t * iot_data_string_map_get_list (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return ((data && (iot_data_type (data) == IOT_DATA_LIST)) ? data : NULL);
}

iot_data_type_t iot_data_map_key_type (const iot_data_t * map)
{
  assert (map && (map->type == IOT_DATA_MAP));
  return map->key_type;
}

extern bool iot_data_map_key_is_of_type (const iot_data_t * map, iot_data_type_t type)
{
  return (map && (map->type == IOT_DATA_MAP) && (map->key_type == type));
}

void iot_data_vector_add (iot_data_t * vector, uint32_t index, iot_data_t * val)
{
  iot_data_vector_t * arr = (iot_data_vector_t*) vector;
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  assert (index < arr->size);
  assert ((val == NULL) || (val && (vector->element_type == IOT_DATA_MULTI || vector->element_type == val->type))); // Check element type matches for fixed type vector
  iot_data_t * element = arr->values[index];
  if (element)
  {
    vector->rehash = true;
    iot_data_free (element);
  }
  else
  {
    vector->hash ^= iot_data_hash (val);
  }
  arr->values[index] = val;
}

const iot_data_t * iot_data_vector_get (const iot_data_t * vector, uint32_t index)
{
  const iot_data_vector_t * arr = (const iot_data_vector_t*) vector;
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  assert (index < arr->size);
  return arr->values[index];
}

void iot_data_vector_resize (iot_data_t * vector, uint32_t size)
{
  iot_data_vector_t * vec = (iot_data_vector_t*) vector;
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  if (size < vec->size)
  {
    for (uint32_t i = size; i < vec->size; i++)
    {
      iot_data_free (vec->values[i]);
    }
  }
  else if (size > vec->size)
  {
    vec->values = realloc (vec->values, size * sizeof (iot_data_t*));
    memset (&vec->values[vec->size], 0, (size - vec->size) * sizeof (iot_data_t*));
  }
  vector->rehash = vec->size != size;
  vec->size = size;
}

uint32_t iot_data_vector_compact (iot_data_t * vector)
{
  iot_data_vector_t * vec = (iot_data_vector_t*) vector;
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  uint32_t new_size = vec->size;
  for (uint32_t i = 0; i < vec->size; i++)
  {
    if (vec->values[i] == NULL)
    {
      new_size--;
      for (uint32_t j = i + 1; j < vec->size; j++)
      {
        if (vec->values[j])
        {
          vec->values[i] = vec->values[j];
          vec->values[j] = NULL;
          new_size++;
          break;
        }
      }
    }
  }
  vec->size = new_size;
  return new_size;
}

uint32_t iot_data_vector_size (const iot_data_t * vector)
{
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  return ((const iot_data_vector_t*) vector)->size;
}

static inline uint32_t iot_data_vector_dimension (const iot_data_t * vector)
{
  // Only regard as mult-dimensional vector if all elements also vectors
  uint32_t size = iot_data_vector_size (vector);
  return (size == iot_data_vector_element_count (vector, IOT_DATA_VECTOR, false)) ? size : 0u;
}

static uint32_t iot_data_vector_depth (const iot_data_t * vector)
{
  uint32_t depth = 0u;
  if (iot_data_vector_dimension (vector))
  {
    iot_data_vector_iter_t iter;
    iot_data_vector_iter (vector, &iter);
    while (iot_data_vector_iter_next (&iter))
    {
      uint32_t dp = iot_data_vector_depth (iot_data_vector_iter_value (&iter));
      if (depth == 0) depth = dp;
      if (dp != depth) // All vector sub-dimensions must be the same depth
      {
        depth = 0;
        break;
      }
    }
  }
  return depth + 1u;
}

static bool iot_data_vector_dims (const iot_data_t * vector, uint32_t * dims, uint32_t * total)
{
  uint32_t size = iot_data_vector_size (vector);
  uint32_t vecs = iot_data_vector_element_count (vector, IOT_DATA_VECTOR, false);
  if (*dims == 0)
  {
    *dims = size;
    *total = *total ? *total * size : size;
  }
  bool ok = (*dims == size);
  if (ok && (size == vecs)) // Only regard as multidimensional vector if all elements also vectors
  {
    dims++;
    iot_data_vector_iter_t iter;
    iot_data_vector_iter (vector, &iter);
    while (ok && iot_data_vector_iter_next (&iter))
    {
      ok = iot_data_vector_dims (iot_data_vector_iter_value (&iter), dims, total);
    }
  }
  return ok;
}

iot_data_t * iot_data_vector_dimensions (const iot_data_t * vector, uint32_t * total)
{
  assert (total && vector && (vector->type == IOT_DATA_VECTOR));
  iot_data_t * dimarray = NULL;
  uint32_t depth = iot_data_vector_depth (vector);
  uint32_t * dims = calloc (depth, sizeof (uint32_t));
  *total = 0u;
  if (iot_data_vector_dims (vector, dims, total))
  {
    dimarray = iot_data_alloc_array (dims, depth, IOT_DATA_UINT32, IOT_DATA_TAKE);
  }
  else
  {
    free (dims);
    *total = 0u;
  }
  return dimarray;
}

static uint32_t iot_data_vector_elements (const iot_data_t * vector, iot_data_type_t type, bool recurse, bool castable)
{
  uint32_t count = 0;
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    const iot_data_t * element = iot_data_vector_iter_value (&iter);
    if (element)
    {
      iot_data_type_t etype = element->type;
      if (recurse && (etype == IOT_DATA_VECTOR))
      {
        count += iot_data_vector_elements (element, type, true, castable);
      }
      else if ((etype == type) || (!castable && (type == IOT_DATA_MULTI)) || (castable && (etype <= IOT_DATA_BOOL)))
      {
        count++;
      }
    }
  }
  return count;
}

uint32_t iot_data_vector_element_count (const iot_data_t * vector, iot_data_type_t type, bool recurse)
{
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  return iot_data_vector_elements (vector, type, recurse, false);
}

static uint32_t iot_data_vector_copy_to_array (const iot_data_t * vector, iot_data_type_t type, uint8_t ** ptr, uint32_t esize, bool recurse)
{
  uint32_t copied = 0u;
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    const iot_data_t * element = iot_data_vector_iter_value (&iter);
    if (element)
    {
      if (recurse && (element->type == IOT_DATA_VECTOR))
      {
        copied += iot_data_vector_copy_to_array (element, type, ptr, esize, true);
      }
      else if (iot_data_cast (element, type, *ptr))
      {
        *ptr += esize;
        copied++;
      }
    }
  }
  return copied;
}

static void iot_data_vector_copy_to_vector (const iot_data_t * from, iot_data_t * to, uint32_t * index, bool recurse)
{
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (from, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    const iot_data_t * element = iot_data_vector_iter_value (&iter);
    if (element)
    {
      if (recurse && (element->type == IOT_DATA_VECTOR))
      {
        iot_data_vector_copy_to_vector (element, to, index, true);
      }
      else
      {
        iot_data_t * val = iot_data_transform (element, to->element_type);
        if (val) iot_data_vector_add (to, (*index)++, val);
      }
    }
  }
}

iot_data_t * iot_data_vector_to_array (const iot_data_t * vector, iot_data_type_t type, bool recurse)
{
  assert (vector && (vector->type == IOT_DATA_VECTOR) && (type <= IOT_DATA_BOOL));
  uint32_t vsize = iot_data_vector_elements (vector, type, recurse, true);
  void * data = NULL;
  uint32_t asize = 0u;

  if (vsize)
  {
    uint32_t esize = iot_data_type_sizes[type];
    data = calloc (vsize, esize);
    uint8_t * ptr = data;
    asize = iot_data_vector_copy_to_array (vector, type, &ptr, esize, recurse);
    if (asize == 0)
    {
      free (data);
      data = NULL;
    }
  }
  return iot_data_alloc_array (data, asize, type, IOT_DATA_TAKE);
}

iot_data_t * iot_data_vector_to_vector (const iot_data_t * vector, iot_data_type_t type, bool recurse)
{
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  uint32_t size = iot_data_vector_elements (vector, type, recurse, true);
  void * to = iot_data_alloc_typed_vector (size, type);

  if (size)
  {
    uint32_t index = 0u;
    iot_data_vector_copy_to_vector (vector, to, &index, recurse);
    ((iot_data_vector_t*) to)->size = index;
  }
  return to;
}

void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter)
{
  assert (iter && map && map->type == IOT_DATA_MAP);
  iter->_map = (const iot_data_map_t*) map;
  iter->_node = NULL;
  iter->_count = 0;
}

static inline iot_node_t * iot_node_start (iot_node_t * node)
{
  if (node) while (node->left) node = node->left;
  return node;
}

extern void iot_data_map_empty (iot_data_t * map)
{
  const iot_node_t * node;
  while ((node = iot_node_start (((iot_data_map_t*) map)->tree))) iot_data_map_remove (map, node->key);
}

bool iot_data_map_iter_next (iot_data_map_iter_t * iter)
{
  assert (iter);
  iter->_node = (iter->_node) ? iot_node_next (iter->_node) : iot_node_start (iter->_map->tree);
  iter->_count = (iter->_count < iter->_map->size) ? iter->_count + 1u : 0u;
  return (iter->_node != NULL);
}

bool iot_data_map_iter_has_next (const iot_data_map_iter_t * iter)
{
  assert (iter && iter->_map);
  return (iter->_count < iter->_map->size);
}

static inline iot_node_t * iot_node_end (iot_node_t * node)
{
  if (node) while (node->right) node = node->right;
  return node;
}

const iot_data_t * iot_data_map_start (iot_data_t * map)
{
  assert (map);
  const iot_node_t * node = iot_node_start (((iot_data_map_t*) map)->tree);
  return node ? node->value : NULL;
}

const void * iot_data_map_start_pointer (iot_data_t * map)
{
  const iot_data_t * value = iot_data_map_start (map);
  return value ? iot_data_pointer (value) : NULL;
}

const iot_data_t * iot_data_map_end (iot_data_t * map)
{
  assert (map);
  const iot_node_t * node = iot_node_end (((iot_data_map_t*) map)->tree);
  return node ? node->value : NULL;
}

const void * iot_data_map_end_pointer (iot_data_t * map)
{
  const iot_data_t * value = iot_data_map_end (map);
  return value ? iot_data_pointer (value) : NULL;
}

bool iot_data_map_iter_prev (iot_data_map_iter_t * iter)
{
  assert (iter);
  iter->_node = (iter->_node) ? iot_node_prev (iter->_node) : iot_node_end (iter->_map->tree);
  if (iter->_count > 0) iter->_count--;
  return (iter->_node != NULL);
}

const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->_node) ? iter->_node->key : NULL;
}

const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return iter->_node ? iter->_node->value : NULL;
}

iot_data_t * iot_data_map_iter_replace_value (const iot_data_map_iter_t * iter, iot_data_t *value)
{
  assert (iter && iter->_map && value && (iter->_map->base.element_type == IOT_DATA_MULTI || iter->_map->base.element_type == value->type));
  iot_data_t * res = (iter->_node) ? iter->_node->value : NULL;
  if (res) iter->_node->value = value;
  return res;
}

const char * iot_data_map_iter_string_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->_node) ? iot_data_string (iot_data_map_iter_key (iter)) : NULL;
}

const char * iot_data_map_iter_string_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->_node) ? iot_data_string (iot_data_map_iter_value (iter)) : NULL;
}

const void * iot_data_map_iter_pointer_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->_node) ? iot_data_pointer (iot_data_map_iter_value (iter)) : NULL;
}

void iot_data_array_iter (const iot_data_t * array, iot_data_array_iter_t * iter)
{
  assert (iter && array && (array->type == IOT_DATA_ARRAY || array->type == IOT_DATA_BINARY));
  iter->_array = (const iot_data_array_t*) array;
  iter->_index = iter->_array->length;
}

bool iot_data_array_iter_next (iot_data_array_iter_t * iter)
{
  assert (iter);
  iter->_index = (iter->_index < iter->_array->length) ? iter->_index + 1 : 0;
  return (iter->_index < iter->_array->length);
}

bool iot_data_array_iter_has_next (const iot_data_array_iter_t * iter)
{
  assert (iter);
  return ((iter->_index == iter->_array->length) || (iter->_index + 1 < iter->_array->length));
}

bool iot_data_array_iter_prev (iot_data_array_iter_t * iter)
{
  assert (iter);
  iter->_index = (iter->_index != 0) ? iter->_index - 1 : iter->_array->length;
  return (iter->_index < iter->_array->length);
}

uint32_t iot_data_array_iter_index (const iot_data_array_iter_t * iter)
{
  assert (iter);
  return iter->_index;
}

const void * iot_data_array_iter_value (const iot_data_array_iter_t * iter)
{
  assert (iter && iter->_array);
  return (iter->_index < iter->_array->length) ? ((uint8_t*) (iter->_array->data) + (iter->_index) * iot_data_type_sizes[iter->_array->base.element_type]) : NULL;
}

void iot_data_vector_iter (const iot_data_t * vector, iot_data_vector_iter_t * iter)
{
  assert (iter && vector && vector->type == IOT_DATA_VECTOR);
  iter->_vector = (const iot_data_vector_t*) vector;
  iter->_index = iter->_vector->size;
}

bool iot_data_vector_iter_next (iot_data_vector_iter_t * iter)
{
  assert (iter);
  iter->_index = (iter->_index < iter->_vector->size) ? iter->_index + 1 : 0;
  return (iter->_index < iter->_vector->size);
}

bool iot_data_vector_iter_has_next (const iot_data_vector_iter_t * iter)
{
  return ((iter->_index == iter->_vector->size) || (iter->_index + 1 < iter->_vector->size));
}

bool iot_data_vector_iter_prev (iot_data_vector_iter_t * iter)
{
  assert (iter);
  iter->_index = (iter->_index != 0) ? iter->_index - 1 : iter->_vector->size;
  return (iter->_index < iter->_vector->size);
}

uint32_t iot_data_vector_iter_index (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return iter->_index;
}

const iot_data_t * iot_data_vector_iter_value (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return (iter->_index < iter->_vector->size) ? iter->_vector->values[iter->_index] : NULL;
}

const char * iot_data_vector_iter_string_value (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  const iot_data_t * data = iot_data_vector_iter_value (iter);
  return data ? iot_data_string (data) : NULL;
}

const void * iot_data_vector_iter_pointer_value (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  const iot_data_t * data = iot_data_vector_iter_value (iter);
  return data ? iot_data_pointer (data) : NULL;
}

iot_data_t * iot_data_vector_iter_replace_value (const iot_data_vector_iter_t * iter, iot_data_t * value)
{
  assert (iter && iter->_vector && value && (iter->_vector->base.element_type == IOT_DATA_MULTI || iter->_vector->base.element_type == value->type));
  iot_data_t * res = NULL;
  iot_data_vector_t * vector = (iot_data_vector_t*) iter->_vector;
  if (iter->_index < vector->size)
  {
    res = vector->values[iter->_index];
    if (res)
    {
      vector->base.rehash = true;
    }
    else
    {
      if (value) vector->base.hash ^= iot_data_hash (value);
    }
    vector->values[iter->_index] = value;
  }
  return res;
}

const char * iot_data_vector_iter_string (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return (iter->_index < iter->_vector->size) ? iot_data_string (iter->_vector->values[iter->_index]) : NULL;
}

const iot_data_t * iot_data_vector_find (const iot_data_t * vector, iot_data_cmp_fn cmp, const void * arg)
{
  assert (vector && cmp);
  const iot_data_t * result = NULL;
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    result = iot_data_vector_iter_value (&iter);
    if (cmp (result, arg)) break;
    result = NULL;
  }
  return result;
}

static size_t iot_data_repr_size (char c)
{
  static uint8_t size_map[] =
  {
    6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };
  return size_map[(uint8_t) c];
}

void iot_data_holder_realloc (iot_string_holder_t * holder, size_t required)
{
  size_t inc = holder->size > IOT_STR_BUFF_DOUBLING_LIMIT ? IOT_STR_BUFF_INCREMENT : holder->size;
  if (inc < required) inc = required;
  holder->size += inc;
  holder->free += inc;
  holder->str = realloc (holder->str, holder->size);
}

void iot_data_strcat_escape (iot_string_holder_t * holder, const char * add, bool escape)
{
  size_t len = strlen (add);
  size_t adj_len = len;
  size_t i;
  if (escape)
  {
    adj_len = 0;
    for (i = 0; i < len; i++)
    {
      adj_len += iot_data_repr_size (add[i]);
    }
  }
  if (holder->free < adj_len)
  {
    iot_data_holder_realloc (holder, adj_len);
  }
  if (len == adj_len)
  {
    strcpy (holder->str + holder->size - holder->free - 1, add);
  }
  else
  {
    static const char * hex = "0123456789abcdef";
    assert (strlen (holder->str) == (holder->size - holder->free - 1));
    uint8_t * ptr = (uint8_t*) holder->str + holder->size - holder->free - 1;
    for (i = 0; i < len; i++)
    {
      uint8_t c = add[i];
      switch (iot_data_repr_size (c))
      {
        case 1:
          *ptr++ = c;
          break;
        case 2:
          *ptr++ = '\\';
          switch (c)
          {
            case '\b': *ptr++ = 'b'; break;
            case '\f': *ptr++ = 'f'; break;
            case '\n': *ptr++ = 'n'; break;
            case '\r': *ptr++ = 'r'; break;
            case '\t': *ptr++ = 't'; break;
            case '\"': *ptr++ = '\"'; break;
            case '\\': *ptr++ = '\\'; break;
            default: break;
          }
          break;
        default: // 6
          *ptr++ = '\\';
          *ptr++ = 'u';
          *ptr++ = '0';
          *ptr++ = '0';
          *ptr++ = (c & 0x10u) ? '1' : '0';
          *ptr++ = hex[c & 0x0fu];
          break;
      }
    }
    *ptr = '\0';
  }
  holder->free -= adj_len;
}

static inline void iot_data_strcat (iot_string_holder_t * holder, const char * add)
{
  iot_data_strcat_escape (holder, add, true);
}

static inline void iot_data_add_quote (iot_string_holder_t * holder)
{
  iot_data_strcat_escape (holder, "\"", false);
}

iot_data_t * iot_data_copy (const iot_data_t * data)
{
  iot_data_t * ret = NULL;

  if (data == NULL) return ret;
  if (data->constant) return (iot_data_t*) data;

  switch (data->type)
  {
    case IOT_DATA_STRING:
    {
      const iot_data_value_t * val = (const iot_data_value_t *) data;
      ret = iot_data_alloc_string (val->value.str, val->base.release ? IOT_DATA_COPY : IOT_DATA_REF);
      break;
    }
    case IOT_DATA_BINARY:
    case IOT_DATA_ARRAY:
    {
      const iot_data_array_t * array = (const iot_data_array_t *) data;
      ret = iot_data_alloc_array (array->data, array->length, array->base.element_type, array->base.release ? IOT_DATA_COPY : IOT_DATA_REF);
      ret->type = data->type; // May be binary or array
      break;
    }
    case IOT_DATA_MAP:
    {
      iot_data_map_iter_t iter;
      ret = iot_data_alloc_typed_map (data->key_type, data->element_type);
      iot_data_map_iter (data, &iter);
      while (iot_data_map_iter_next (&iter))
      {
        iot_data_t * key = iot_data_copy (iot_data_map_iter_key (&iter));
        iot_data_t * value = iot_data_copy (iot_data_map_iter_value (&iter));
        iot_data_map_add (ret, key, value);
      }
      break;
    }
    case IOT_DATA_VECTOR:
    {
      iot_data_vector_iter_t iter;
      ret = iot_data_alloc_typed_vector (iot_data_vector_size (data), data->element_type);
      iot_data_vector_iter (data, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        iot_data_t * val = iot_data_copy (iot_data_vector_iter_value (&iter));
        iot_data_vector_add (ret, iter._index, val);
      }
      break;
    }
    case IOT_DATA_LIST:
    {
      iot_data_list_iter_t iter;
      ret = iot_data_alloc_typed_list (data->element_type);
      iot_data_list_iter (data, &iter);
      while (iot_data_list_iter_next (&iter))
      {
        iot_data_t * val = iot_data_copy (iot_data_list_iter_value (&iter));
        iot_data_list_head_push (ret, val);
      }
      break;
    }
    case IOT_DATA_POINTER:
    {
      ret = (iot_data_t*) data;
      iot_data_add_ref (ret);
      break;
    }
    default: // basic types
    {
      iot_data_value_t * val = iot_data_value_alloc (data->type, false);
      val->value.ui64 = (((const iot_data_value_t*) data)->value.ui64);
      val->base.hash = data->hash;
      ret = (iot_data_t*) val;
    }
  }
  ret->base.meta = iot_data_add_ref (data->base.meta);
  return ret;
}

extern bool iot_typecode_equal (const iot_typecode_t * tc1, const iot_typecode_t * tc2)
{
  if (tc1 == tc2) return true;
  if (tc1 == NULL || tc2 == NULL) return false;
  bool ret = (tc1->type == tc2->type);
  if (ret && ((tc1->type >= IOT_DATA_ARRAY) && (tc1->type <= IOT_DATA_MULTI))) ret = (tc1->element_type == tc2->element_type);
  if (ret && (tc1->type == IOT_DATA_MAP)) ret = (tc1->key_type == tc2->key_type);
  return ret;
}

extern bool iot_data_matches (const iot_data_t * data, const iot_typecode_t * tc)
{
  assert (data);
  iot_typecode_t dtc = { .type = data->type, .element_type = data->element_type, .key_type = data->key_type };
  return iot_typecode_equal (tc, &dtc);
}

extern void iot_data_typecode (const iot_data_t * data, iot_typecode_t * tc)
{
  assert (data && tc);
  tc->type = data->type;
  tc->element_type = data->element_type;
  tc->key_type = data->key_type;
}

extern iot_data_t * iot_data_shallow_copy (const iot_data_t * src)
{
  iot_data_t * result;
  assert (src);
  switch (src->type)
  {
    case IOT_DATA_MAP:
    {
      result = iot_data_alloc_map (iot_data_map_type (src));
      iot_data_map_iter_t iter;
      iot_data_map_iter (src, &iter);
      while (iot_data_map_iter_next (&iter))
      {
        iot_data_map_add (result, iot_data_add_ref (iot_data_map_iter_key (&iter)), iot_data_add_ref (iot_data_map_iter_value (&iter)));
      }
      break;
    }
    case IOT_DATA_VECTOR:
    {
      uint32_t size = iot_data_vector_size (src);
      result = iot_data_alloc_vector (size);
      for (uint32_t i = 0; i < size; i++)
      {
        iot_data_vector_add (result, i, iot_data_add_ref (iot_data_vector_get (src, i)));
      }
      break;
    }
    case IOT_DATA_LIST:
    {
      result = iot_data_alloc_list ();
      iot_data_list_iter_t iter;
      iot_data_list_iter (src, &iter);
      while (iot_data_list_iter_next (&iter))
      {
        iot_data_list_head_push (result, iot_data_add_ref (iot_data_list_iter_value (&iter)));
      }
      break;
    }
    default:
    {
      result = iot_data_add_ref (src);
      break;
    }
  }
  return result;
}

extern const iot_data_t * iot_data_get_at (const iot_data_t * data, const iot_data_t * path)
{
  assert (data && path);
  const iot_data_t * result = data;
  if (iot_data_list_length (path) > 0)
  {
    assert (data->type == IOT_DATA_MAP || data->type == IOT_DATA_VECTOR);
    iot_data_t * p = iot_data_copy (path);
    iot_data_t * index = iot_data_list_head_pop (p);
    result = (data->type == IOT_DATA_MAP) ? iot_data_get_at (iot_data_map_get (data, index), p) : iot_data_get_at (iot_data_vector_get (data, iot_data_ui32 (index)), p);
    iot_data_free (index);
    iot_data_free (p);
  }
  return result;
}

extern void iot_data_list_empty (iot_data_t * list)
{
  iot_data_t * element;
  while ((element = iot_data_list_head_pop (list))) iot_data_free (element);
}

extern iot_data_t * iot_data_add_at (const iot_data_t * data, const iot_data_t * path, iot_data_t * val)
{
  assert (data && path && val);
  iot_data_t * result;
  if (iot_data_list_length (path) > 0)
  {
    assert (data->type == IOT_DATA_MAP || data->type == IOT_DATA_VECTOR);
    iot_data_t * p = iot_data_copy (path);
    iot_data_t * index = iot_data_list_head_pop (p);
    result = iot_data_shallow_copy (data);
    (data->type == IOT_DATA_MAP) ?
      iot_data_map_add (result, iot_data_add_ref (index), iot_data_add_at (iot_data_map_get (data, index), p, val)) :
      iot_data_vector_add (result, iot_data_ui32 (index), iot_data_add_at (iot_data_vector_get (data, iot_data_ui32 (index)), p, val));
    iot_data_free (index);
    iot_data_free (p);
  }
  else
  {
    result = val;
  }
  return result;
}

extern iot_data_t * iot_data_remove_at (const iot_data_t * data, const iot_data_t * path)
{
  assert (data && path);
  iot_data_t * result = iot_data_shallow_copy (data);
  if (iot_data_list_length (path) > 0)
  {
    assert (data->type == IOT_DATA_MAP || data->type == IOT_DATA_VECTOR);
    iot_data_t * p = iot_data_copy (path);
    iot_data_t * index = iot_data_list_head_pop (p);
    if (data->type == IOT_DATA_MAP)
    {
      if (iot_data_list_length (p) > 0)
      {
        iot_data_map_add (result, iot_data_add_ref (index), iot_data_remove_at (iot_data_map_get (result, index), p));
      }
      else
      {
        iot_data_map_remove (result, index);
      }
    }
    else
    {
      uint32_t i = iot_data_ui32 (index);
      if (iot_data_list_length (p) > 0)
      {
        iot_data_vector_add (result, i, iot_data_remove_at (iot_data_vector_get (result, i), p));
      }
      else
      {
        iot_data_vector_add (result, i, NULL);
        iot_data_vector_compact (result);
      }
    }
    iot_data_free (index);
    iot_data_free (p);
  }
  return result;
}

extern iot_data_t * iot_data_update_at (const iot_data_t * data, const iot_data_t * path, iot_data_update_fn fn, void * arg)
{
  iot_data_t * result;
  assert (data && path && fn);
  if (iot_data_list_length (path) == 0)
  {
    result = fn (data, arg);
  }
  else
  {
    result = iot_data_shallow_copy (data);
    iot_data_t * p = iot_data_copy (path);
    iot_data_t * index = iot_data_list_head_pop (p);
    if (data->type == IOT_DATA_MAP)
    {
      const iot_data_t * val = iot_data_map_get (data, index);
      if (val) iot_data_map_add (result, iot_data_add_ref (index), iot_data_update_at (val, p, fn, arg));
    }
    else if (data->type == IOT_DATA_VECTOR)
    {
      iot_data_vector_add (result, iot_data_ui32 (index), iot_data_update_at (iot_data_vector_get (data, iot_data_ui32 (index)), p, fn, arg));
    }
    iot_data_free (index);
    iot_data_free (p);
  }
  return result;
}

/* Red/Black binary tree manipulation functions. Implements iot_data_map_t.
 *
 * https://algorithmtutor.com/Data-Structures/Tree/Red-Black-Trees/
 *
 * Note that the logic regards NULL nodes as Black. So all colour get/set operations
 * need checking for node being NULL.
 */

static inline iot_node_colour_t iot_node_colour (const iot_node_t * node)
{
  return node ? node->colour : IOT_NODE_BLACK;
}

static inline void iot_node_set_colour (iot_node_t * node, iot_node_colour_t colour)
{
  if (node) node->colour = colour;
}

#define IS_BLACK(n) (iot_node_colour (n) == IOT_NODE_BLACK)
#define IS_RED(n) (iot_node_colour (n) == IOT_NODE_RED)
#define IS_LEFT_BLACK(n) ((n) == NULL || IS_BLACK ((n)->left))
#define IS_RIGHT_BLACK(n) ((n) == NULL || IS_BLACK ((n)->right))
#define IS_LEFT(n) ((n) == (n)->parent->left)
#define IS_RIGHT(n) ((n) == (n)->parent->right)

static inline iot_node_t * iot_node_grandparent (const iot_node_t * node)
{
  return (node && node->parent) ? node->parent->parent : NULL;
}

static inline iot_node_t * iot_node_uncle (const iot_node_t * node)
{
  iot_node_t * gp = iot_node_grandparent (node);
  return gp ? ((node->parent == gp->left) ? gp->right : gp->left) : NULL;
}

static inline iot_node_t * iot_node_sibling (const iot_node_t * node)
{
  return IS_LEFT (node) ? node->parent->right : node->parent->left;
}

static inline iot_node_t * iot_node_alloc (iot_node_t * parent, iot_data_t * key, iot_data_t * value)
{
  bool heap = iot_data_alloc_from_heap;
  iot_node_t * node = heap ? calloc (1, IOT_DATA_BLOCK_SIZE) : iot_data_block_alloc ();
  node->heap = heap;
  node->value = value;
  node->key = key;
  node->parent = parent;
  node->colour = IOT_NODE_RED;
  return node;
}

static void iot_node_delete (iot_node_t * node)
{
  iot_data_free (node->key);
  iot_data_free (node->value);
  (node->heap) ? free (node) : iot_data_block_free ((iot_block_t*) node);
}

static inline iot_node_t * iot_node_minimum (iot_node_t * node)
{
  while (node->left) node = node->left;
  return node;
}

static void iot_node_ror (iot_data_map_t * map, iot_node_t * x)
{
  iot_node_t * y = x->left;
  x->left = y->right;
  if (y->right) y->right->parent = x;
  y->parent = x->parent;
  if (x->parent == NULL) map->tree = y;
  else if (x == x->parent->right) x->parent->right = y;
  else x->parent->left = y;
  y->right = x;
  x->parent = y;
}

static void iot_node_rol (iot_data_map_t * map, iot_node_t * x)
{
  iot_node_t * y = x->right;
  x->right = y->left;
  if (y->left) y->left->parent = x;
  y->parent = x->parent;
  if (x->parent == NULL) map->tree = y;
  else if (IS_LEFT (x)) x->parent->left = y;
  else x->parent->right = y;
  y->left = x;
  x->parent = y;
}

static void iot_node_insert_balance (iot_data_map_t * map, iot_node_t * k)
{
  while (IS_RED (k->parent))
  {
    iot_node_t * u = iot_node_uncle (k);
    if (k->parent == k->parent->parent->right)
    {
      if (IS_RED (u)) // case 3.1
      {
        u->colour = IOT_NODE_BLACK;
        k->parent->colour = IOT_NODE_BLACK;
        k->parent->parent->colour = IOT_NODE_RED;
        k = k->parent->parent;
      }
      else
      {
        if (IS_LEFT (k)) // case 3.2.2
        {
          k = k->parent;
          iot_node_ror (map, k);
        }
        // case 3.2.1
        k->parent->colour = IOT_NODE_BLACK;
        k->parent->parent->colour = IOT_NODE_RED;
        iot_node_rol (map, k->parent->parent);
      }
    }
    else
    {
      if (IS_RED (u)) // case 3.1
      {
        u->colour = IOT_NODE_BLACK;
        k->parent->colour = IOT_NODE_BLACK;
        k->parent->parent->colour = IOT_NODE_RED;
        k = k->parent->parent;
      }
      else
      {
        if (IS_RIGHT (k)) // case 3.2.2
        {
          k = k->parent;
          iot_node_rol (map, k);
        }
        // case 3.2.1
        k->parent->colour = IOT_NODE_BLACK;
        k->parent->parent->colour = IOT_NODE_RED;
        iot_node_ror (map, k->parent->parent);
      }
    }
    if (k == map->tree) break;
  }
  map->tree->colour = IOT_NODE_BLACK;
}

static void iot_node_remove_balance (iot_data_map_t * map, iot_node_t * x)
{
  while (x != map->tree && IS_BLACK (x))
  {
    iot_node_t * s = iot_node_sibling (x);
    if (IS_LEFT (x))
    {
      if (IS_RED (s)) // case 3.1
      {
        s->colour = IOT_NODE_BLACK;
        x->parent->colour = IOT_NODE_RED;
        iot_node_rol (map, x->parent);
        s = x->parent->right;
      }
      if (IS_LEFT_BLACK (s) && IS_RIGHT_BLACK (s)) // case 3.2
      {
        iot_node_set_colour (s, IOT_NODE_RED);
        x = x->parent;
      }
      else
      {
        if (IS_BLACK (s->right)) // case 3.3
        {
          iot_node_set_colour (s->left, IOT_NODE_BLACK);
          iot_node_set_colour (s, IOT_NODE_RED);
          iot_node_ror (map, s);
          s = x->parent->right;
        }
        // case 3.4
        iot_node_set_colour (s, iot_node_colour (x->parent));
        x->parent->colour = IOT_NODE_BLACK;
        if (s) iot_node_set_colour (s->right, IOT_NODE_BLACK);
        iot_node_rol (map, x->parent);
        x = map->tree;
      }
    }
    else
    {
      if (IS_RED (s)) // case 3.1
      {
        s->colour = IOT_NODE_BLACK;
        x->parent->colour = IOT_NODE_RED;
        iot_node_ror (map, x->parent);
        s = x->parent->left;
      }
      if (IS_RIGHT_BLACK (s) && IS_LEFT_BLACK (s)) // case 3.2
      {
        iot_node_set_colour (s, IOT_NODE_RED);
        x = x->parent;
      }
      else
      {
        if (IS_BLACK (s->left)) // case 3.3
        {
          iot_node_set_colour (s->right, IOT_NODE_BLACK);
          iot_node_set_colour (s, IOT_NODE_RED);
          iot_node_rol (map, s);
          s = x->parent->left;
        }
        // case 3.4
        iot_node_set_colour (s, iot_node_colour (x->parent));
        x->parent->colour = IOT_NODE_BLACK;
        if (s) iot_node_set_colour (s->left, IOT_NODE_BLACK);
        iot_node_ror (map, x->parent);
        x = map->tree;
      }
    }
  }
  x->colour = IOT_NODE_BLACK;
}

static inline iot_node_t * iot_node_find (const iot_node_t * node, const iot_data_t * key)
{
  while (node)
  {
    int cmp = iot_data_compare (node->key, key);
    if (cmp == 0) break;
    node = (cmp > 0) ? node->left : node->right;
  }
  return (iot_node_t*) node;
}

static void iot_node_insert (iot_data_map_t * map, iot_data_t * key, iot_data_t * value)
{
  iot_node_t * node = iot_node_alloc (NULL, key, value);
  iot_node_t * y = NULL;
  iot_node_t * x = map->tree;
  while (x)
  {
    y = x;
    x = (iot_data_compare (key, x->key) < 0) ? x->left : x->right;
  }
  node->parent = y;
  if (y == NULL) map->tree = node;
  else if (iot_data_compare (key, y->key) < 0) y->left = node;
  else y->right = node;

  if (node->parent)
  {
    if (node->parent->parent) iot_node_insert_balance (map, node);
  }
  else
  {
    node->colour = IOT_NODE_BLACK;
  }
}

static void iot_node_transplant (iot_data_map_t * map, iot_node_t * u, iot_node_t * v)
{
  if (u->parent == NULL) map->tree = v;
  else if (IS_LEFT (u)) u->parent->left = v;
  else u->parent->right = v;
  if (v) v->parent = u->parent;
}

static bool iot_node_remove (iot_data_map_t * map, const iot_data_t * key)
{
  iot_node_t * z = iot_node_find (map->tree, key);
  if (z)
  {
    iot_node_t * x;
    iot_node_t * y = z;
    iot_node_colour_t col = y->colour;
    if (z->left == NULL)
    {
      x = z->right;
      iot_node_transplant (map, z, z->right);
    }
    else if (z->right == NULL)
    {
      x = z->left;
      iot_node_transplant (map, z, z->left);
    }
    else
    {
      y = iot_node_minimum (z->right);
      col = y->colour;
      x = y->right;
      if (y->parent == z)
      {
        if (x) x->parent = y;
      }
      else
      {
        iot_node_transplant (map, y, y->right);
        y->right = z->right;
        y->right->parent = y;
      }
      iot_node_transplant (map, z, y);
      y->left = z->left;
      y->left->parent = y;
      y->colour = z->colour;
    }
    iot_node_delete (z);
    if (x && (col == IOT_NODE_BLACK)) iot_node_remove_balance (map, x);
  }
  return (z != NULL);
}

static bool iot_node_add (iot_data_map_t * map, iot_data_t * key, iot_data_t * value)
{
  iot_node_t * node = iot_node_find (map->tree, key);
  if (node)
  {
    map->base.rehash = true;
    iot_data_free (key);
    iot_data_free (node->value);
    node->value = value;
  }
  else
  {
    iot_node_insert (map, key, value);
    iot_data_map_hash (&map->base, key, value);
  }
  return (node == NULL);
}

static void iot_node_free (iot_data_map_t * map, iot_node_t * node)
{
  if (node)
  {
    iot_node_free (map, node->left);
    iot_node_free (map, node->right);
    iot_node_delete (node);
  }
}

static iot_node_t * iot_node_next (iot_node_t * iter)
{
  if (iter->right)
  {
    // Right then left to the end
    for (iter = iter->right; iter->left != NULL; iter = iter->left);
  }
  else
  {
    // While the right child, chain up parent link
    iot_node_t * n = iter->parent;
    while (n && iter == n->right)
    {
      iter = n;
      n = n->parent;
    }
    iter = n;
  }
  return iter;
}

static iot_node_t * iot_node_prev (iot_node_t * iter)
{
  if (iter->left)
  {
    // Left then right to the end
    for (iter = iter->left; iter->right != NULL; iter = iter->right);
  }
  else
  {
    // While the left child, chain up parent link
    iot_node_t * n = iter->parent;
    while (n && iter == n->left)
    {
      iter = n;
      n = n->parent;
    }
    iter = n;
  }
  return iter;
}

#ifndef NDEBUG

static void iot_node_dump (iot_node_t * node, const char * msg)
{
  static uint32_t level = 0;
  if (node)
  {
    level++;
    iot_node_dump (node->left, "Left");
    level--;
    char * key = iot_data_to_json (node->key);
    for (uint32_t i = level; i > 0; i --) printf ("   ");
    printf ("%s %s Key: %s\n", msg, (node->colour == IOT_NODE_BLACK) ? "Black" : "Red", key);
    free (key);
    level++;
    iot_node_dump (node->right, "Right");
    level--;
  }
}

extern void iot_data_map_dump (iot_data_t * map)
{
  iot_data_map_t * mp = (iot_data_map_t*) map;
  printf ("\nMap size: %d\n", iot_data_map_size (map));
  iot_node_dump (mp->tree, "Root");
}

#endif
