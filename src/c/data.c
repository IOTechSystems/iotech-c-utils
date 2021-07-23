//
// Copyright (c) 2019-2021 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/typecode.h"
#include "iot/json.h"
#include "iot/base64.h"
#include "iot/hash.h"

#ifdef IOT_HAS_UUID
#include <uuid/uuid.h>
#else
#include "iot/uuid.h"
#endif

#ifdef IOT_HAS_XML
#include "yxml.h"
#define YXML_PARSER_BUFF_SIZE 4096
#define YXML_BUFF_SIZE 512
#endif

#if defined (_GNU_SOURCE) || defined (_ALPINE_)
#define IOT_HAS_SPINLOCK
#endif

#if defined (NDEBUG) || defined (_AZURESPHERE_)
#define IOT_DATA_CACHE
#endif

#define IOT_MEMORY_BLOCK_SIZE 4096
#define IOT_JSON_BUFF_SIZE 512
#define IOT_VAL_BUFF_SIZE 128
#define IOT_JSON_BUFF_DOUBLING_LIMIT 4096
#define IOT_JSON_BUFF_INCREMENT 1024

static const char * iot_data_type_names [] = {"Int8","UInt8","Int16","UInt16","Int32","UInt32","Int64","UInt64","Float32","Float64","Bool","String","Null","Array","Map","Vector","Pointer"};
static const uint8_t iot_data_type_size [] = { 1u, 1u, 2u, 2u, 4u, 4u, 8u, 8u, 4u, 8u, sizeof (bool), sizeof (char*), 0u };
static const char * ORDERING_KEY = "ordering";

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

struct iot_data_t
{
  iot_data_t * metadata;
  iot_data_t * next;
  atomic_uint_fast32_t refs;
  uint32_t hash;
  iot_data_type_t type;
  iot_data_type_t sub_type;
  bool release : 1;
  bool release_block : 1;
};

struct iot_typecode_t
{
  iot_data_type_t type;
  iot_data_type_t key_type;
  iot_typecode_t * element_type;
};

typedef struct iot_data_value_base_t
{
  iot_data_t base;
  iot_data_union_t value;
} iot_data_value_base_t;

typedef struct iot_data_array_t
{
  iot_data_t base;
  uint32_t length;
  uint32_t size;
  void * data;
} iot_data_array_t;

typedef struct iot_data_vector_t
{
  iot_data_t base;
  uint32_t size;
  iot_data_t ** values;
} iot_data_vector_t;

typedef struct iot_string_holder_t
{
  char * str;
  size_t size;
  size_t free;
} iot_string_holder_t;

typedef struct iot_node_t
{
  struct iot_node_t * parent;
  struct iot_node_t * left;
  struct iot_node_t * right;
  iot_data_t * key;
  iot_data_t * value;
  iot_node_colour_t colour;
} iot_node_t;

typedef struct iot_data_map_t
{
  iot_data_t base;
  uint32_t size;
  iot_node_t * tree;
} iot_data_map_t;

typedef struct iot_data_pointer_t
{
  iot_data_t base;
  void * value;
  void (*free_fn) (void*);
} iot_data_pointer_t;

// Determine minimum block size that can hold all iot_data types, maximum size of
// value string cache buffer and number of blocks per allocated memory chunk.

#define IOT_DATA_BLOCK_SIZE (((sizeof (iot_data_array_t) + 7) / 8) * 8)
#define IOT_DATA_BLOCKS ((IOT_MEMORY_BLOCK_SIZE / IOT_DATA_BLOCK_SIZE) - 1)
#define IOT_DATA_VALUE_BUFF_SIZE (IOT_DATA_BLOCK_SIZE - sizeof (iot_data_value_base_t))
#define IOT_DATA_ALLOCATING ((iot_data_t*) 1)

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

// Data size and alignment sanity checks

_Static_assert ((IOT_DATA_BLOCK_SIZE % 8) == 0, "IOT_DATA_BLOCK_SIZE not 8 byte multiple");
_Static_assert (sizeof (iot_data_value_t) == IOT_DATA_BLOCK_SIZE, "size of iot_data_value_t not equal to IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_map_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_map bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_pointer_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_pointer bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_vector_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_array_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_array_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_typecode_t) <= IOT_DATA_BLOCK_SIZE, "iot_typecode_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_memory_block_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_memory_block_t bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= sizeof (iot_data_array_t), "iot_data_vector_t bigger than iot_data_array");
_Static_assert (sizeof (iot_data_map_t) <= sizeof (iot_data_array_t), "iot_data_map bigger than iot_data_array");

// Data cache usually disabled for debug builds as otherwise too difficult to trace leaks

#ifdef IOT_DATA_CACHE
static iot_data_t * iot_data_cache = NULL;
static iot_memory_block_t * iot_data_blocks = NULL;
#ifdef IOT_HAS_SPINLOCK
static pthread_spinlock_t iot_data_slock;
#endif
static pthread_mutex_t iot_data_mutex;
#endif

extern void iot_data_init (void);
extern void iot_data_map_dump (iot_data_t * map);
static iot_data_t * iot_data_all_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered);
static void iot_node_free (iot_node_t * node);
static iot_node_t * iot_node_start (iot_node_t * node);
static iot_node_t * iot_node_next (iot_node_t * iter);
static bool iot_node_add (iot_data_map_t * map, iot_data_t * key, iot_data_t * value);
static bool iot_node_remove (iot_data_map_t * map, const iot_data_t * key);
static iot_node_t * iot_node_find (iot_node_t * node, const iot_data_t * key);

static void * iot_data_block_alloc (void)
{
  iot_data_t * data;
#ifdef IOT_DATA_CACHE

#ifdef IOT_HAS_SPINLOCK
  pthread_spin_lock (&iot_data_slock);
#else
  pthread_mutex_lock (&iot_data_mutex);
#endif
#ifdef IOT_HAS_SPINLOCK
  while (iot_data_cache <= IOT_DATA_ALLOCATING)
  {
    bool allocate = (iot_data_cache == NULL);
    iot_data_t * new_data_cache = NULL;
    if (allocate) iot_data_cache = IOT_DATA_ALLOCATING;
    pthread_spin_unlock (&iot_data_slock);
    pthread_mutex_lock (&iot_data_mutex);
#else
    bool allocate = (iot_data_cache == NULL);
    iot_data_t * new_data_cache = NULL;
#endif
    if (allocate)
    {
      iot_memory_block_t * block = calloc (1, IOT_MEMORY_BLOCK_SIZE);
      block->next = iot_data_blocks;
      iot_data_blocks = block;

      uint8_t * iter = (uint8_t*) block->chunks;
      new_data_cache = (iot_data_t*) iter;
      for (unsigned i = 0; i < (IOT_DATA_BLOCKS - 1); i++)
      {
        iot_data_t * prev = (iot_data_t*) iter;
        iter += IOT_DATA_BLOCK_SIZE;
        prev->next = (iot_data_t*) iter;
      }
    }
#ifdef IOT_HAS_SPINLOCK
    pthread_mutex_unlock (&iot_data_mutex);
    pthread_spin_lock (&iot_data_slock);
    if (allocate) iot_data_cache = new_data_cache;
  }
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

static inline void iot_data_block_free (iot_data_t * data)
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
  data->next = iot_data_cache;
  iot_data_cache = data;
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_unlock (&iot_data_slock);
#else
  pthread_mutex_unlock (&iot_data_mutex);
#endif
#else
  free (data);
#endif
}

static void * iot_data_factory_alloc (iot_data_type_t type)
{
  iot_data_t * data = iot_data_block_alloc ();
  atomic_store (&data->refs, 1);
  data->type = type;
  return data;
}

static inline iot_data_value_t * iot_data_value_alloc (iot_data_type_t type, iot_data_ownership_t own)
{
  iot_data_value_t * val = iot_data_factory_alloc (type);
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

void iot_data_init (void)
{
/*
  printf ("sizeof (iot_data_value_t): %zu\n", sizeof (iot_data_value_t));
  printf ("sizeof (iot_data_map_t): %zu\n", sizeof (iot_data_map_t));
  printf ("sizeof (iot_node_t): %zu\n", sizeof (iot_node_t));
  printf ("sizeof (iot_data_vector_t): %zu\n", sizeof (iot_data_vector_t));
  printf ("sizeof (iot_data_array_t): %zu\n", sizeof (iot_data_array_t));
  printf ("sizeof (iot_data_pointer_t): %zu\n", sizeof (iot_data_pointer_t));
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
  atexit (iot_data_fini);
}

iot_data_t * iot_data_add_ref (const iot_data_t * data)
{
  if (data) atomic_fetch_add (&((iot_data_t*) data)->refs, 1);
  return (iot_data_t*) data;
}

iot_data_type_t iot_data_name_type (const char * name)
{
  int type = 0;
  while (type >= 0)
  {
    if (strcasecmp (name, iot_data_type_names[type]) == 0) break;
    type = (type == IOT_DATA_POINTER) ? -1 : (type + 1);
  }
  return type;
}

const char * iot_data_type_string (iot_data_type_t type)
{
  assert (type <= IOT_DATA_POINTER);
  return iot_data_type_names[type];
}

const char * iot_data_type_name (const iot_data_t * data)
{
  assert (data && (data->type <= IOT_DATA_POINTER));
  return iot_data_type_names[data->type];
}

extern void iot_data_set_metadata (iot_data_t * data, iot_data_t * metadata)
{
  assert (data);
  if (data->metadata) iot_data_free (data->metadata);
  if (metadata) iot_data_add_ref (metadata);
  data->metadata = metadata;
}

extern const iot_data_t * iot_data_get_metadata (const iot_data_t * data)
{
  return data ? data->metadata : NULL;
}

static int iot_data_key_cmp (const iot_data_t * v1, const iot_data_t * v2)
{
  if (v1->type == v2->type)
  {
    switch (v1->type)
    {
      case IOT_DATA_STRING:
        return strcmp (((iot_data_value_t *) v1)->value.str, ((iot_data_value_t *) v2)->value.str);
      case IOT_DATA_ARRAY:
        return (v1->hash == v2->hash) ? 0 : ((v1->hash < v2->hash) ? -1 : 1);
      default:
      {
        uint64_t ui1 = ((iot_data_value_t *) v1)->value.ui64;
        uint64_t ui2 = ((iot_data_value_t *) v2)->value.ui64;
        return (ui1 == ui2) ? 0 : ((ui1 < ui2) ? -1 : 1);
      }
    }
  }
  return (v1->type < v2->type) ? -1 : 1;
}

bool iot_data_equal (const iot_data_t * v1, const iot_data_t * v2)
{
  if (v1 == v2) return true;
  if (v1 == NULL || v2 == NULL) return false;
  if (v1->type == v2->type)
  {
    switch (v1->type)
    {
      case IOT_DATA_STRING:
        return ((iot_data_value_t*) v1)->value.str == ((iot_data_value_t*) v2)->value.str || ((v1->hash == v2->hash) && (strcmp (((iot_data_value_t*) v1)->value.str, ((iot_data_value_t*) v2)->value.str) == 0));
      case IOT_DATA_NULL: return true;
      case IOT_DATA_ARRAY:
      {
        iot_data_array_t * a1 = (iot_data_array_t*) v1;
        iot_data_array_t * a2 = (iot_data_array_t*) v2;
        return  ((a1->size == a2->size) && (v1->sub_type == v2->sub_type) && ((a1->data == a2->data) || ((v1->hash == v2->hash) && (memcmp (a1->data, a2->data, a1->size) == 0))));
      }
      case IOT_DATA_VECTOR:
      {
        if (iot_data_vector_size (v1) != iot_data_vector_size (v2)) return false;

        iot_data_vector_iter_t iter1;
        iot_data_vector_iter_t iter2;
        iot_data_vector_iter (v1, &iter1);
        iot_data_vector_iter (v2, &iter2);

        while ((iot_data_vector_iter_next (&iter1)) && (iot_data_vector_iter_next (&iter2)))
        {
          const iot_data_t * data1 = iot_data_vector_get (v1, (iter1.index - 1));
          const iot_data_t * data2 = iot_data_vector_get (v2, (iter2.index - 1));
          if (!iot_data_equal (data1, data2)) return false;
        }
        return true;
      }
      case IOT_DATA_MAP:
      {
        if (iot_data_map_size (v1) != iot_data_map_size (v2)) return false;

        iot_data_map_iter_t iter1;
        iot_data_map_iter_t iter2;
        iot_data_map_iter (v1, &iter1);
        iot_data_map_iter (v2, &iter2);

        while ((iot_data_map_iter_next (&iter1)) && (iot_data_map_iter_next (&iter2)))
        {
          const iot_data_t * key1 = iot_data_map_iter_key (&iter1);
          const iot_data_t * value1 = iot_data_map_iter_value (&iter1);
          const iot_data_t * key2 = iot_data_map_iter_key (&iter2);
          const iot_data_t * value2 = iot_data_map_iter_value (&iter2);

          if (!iot_data_equal (key1, key2) || !iot_data_equal (value1, value2)) return false;
        }
        return true;
      }
      case IOT_DATA_POINTER: return (((iot_data_pointer_t*) v1)->value == ((iot_data_pointer_t*) v2)->value);
      default: return (((iot_data_value_t*) v1)->value.ui64 == ((iot_data_value_t*) v2)->value.ui64);
    }
  }
  return false;
}

iot_data_t * iot_data_alloc_map (iot_data_type_t key_type)
{
  assert (key_type < IOT_DATA_MAP && key_type != IOT_DATA_NULL);
  iot_data_map_t * map = iot_data_factory_alloc (IOT_DATA_MAP);
  map->base.sub_type = key_type;
  return (iot_data_t*) map;
}

iot_data_t * iot_data_alloc_vector (uint32_t size)
{
  iot_data_vector_t * vector = iot_data_factory_alloc (IOT_DATA_VECTOR);
  vector->size = size;
  vector->values = calloc (size, sizeof (iot_data_t*));
  return (iot_data_t*) vector;
}

iot_data_t * iot_data_alloc_pointer (void * ptr, iot_data_free_fn free_fn)
{
  iot_data_pointer_t * pointer = iot_data_factory_alloc (IOT_DATA_POINTER);
  pointer->free_fn = free_fn;
  pointer->value = ptr;
  return (iot_data_t*) pointer;
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

void * iot_data_address (const iot_data_t * data)
{
  if (data == NULL) return NULL;
  else if (data->type < IOT_DATA_ARRAY) return (void*) &(((iot_data_value_t*) data)->value);
  else if (data->type == IOT_DATA_ARRAY) return ((iot_data_array_t*) data)->data;
  else if (data->type == IOT_DATA_POINTER) return ((iot_data_pointer_t*) data)->value;
  return NULL;
}

void iot_data_free (iot_data_t * data)
{
  if (data && (atomic_fetch_add (&data->refs, -1) <= 1))
  {
    if (data->metadata) iot_data_free (data->metadata);
    switch (data->type)
    {
      case IOT_DATA_STRING:
      {
        iot_data_value_t * val = (iot_data_value_t*) data;
        if (data->release && (val->value.str != val->buff))
        {
          if (data->release_block)
          {
            iot_data_block_free ((iot_data_t*) val->value.str);
          }
          else
          {
            free (val->value.str);
          }
        }
        break;
      }
      case IOT_DATA_ARRAY:
      {
        if (data->release) free (((iot_data_array_t*) data)->data);
        break;
      }
      case IOT_DATA_MAP:
      {
        iot_node_free (((iot_data_map_t*) data)->tree);
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
      default: break;
    }
    iot_data_block_free (data);
  }
}

iot_data_t * iot_data_alloc_from_string (iot_data_type_t type, const char * value)
{
  assert (value && strlen (value));
  switch (type)
  {
    case IOT_DATA_INT8: return iot_data_alloc_i8 ((int8_t) atoi (value));
    case IOT_DATA_UINT8: return iot_data_alloc_ui8 ((uint8_t) atoi (value));
    case IOT_DATA_INT16: return iot_data_alloc_i16 ((int16_t) atoi (value));
    case IOT_DATA_UINT16: return iot_data_alloc_ui16 ((uint16_t) atoi (value));
    case IOT_DATA_INT32: return iot_data_alloc_i32 ((int32_t) atol (value));
    case IOT_DATA_UINT32: return iot_data_alloc_ui32 ((uint32_t) strtoul (value, NULL, 10));
    case IOT_DATA_INT64: return iot_data_alloc_i64 ((int64_t) atoll (value));
    case IOT_DATA_UINT64: return iot_data_alloc_ui64 ((uint64_t) strtoull (value, NULL, 10));
    case IOT_DATA_FLOAT32: return iot_data_alloc_f32 ((float) atof (value));
    case IOT_DATA_FLOAT64: return iot_data_alloc_f64 (atof (value));
    case IOT_DATA_BOOL: return iot_data_alloc_bool (value[0] == 't' || value[0] == 'T');
    case IOT_DATA_STRING: return iot_data_alloc_string (value, IOT_DATA_COPY);
    default: break;
  }
  return NULL;
}

static void iot_data_inc_dec (iot_data_t * data, int8_t val)
{
  assert (data);
  switch (data->type)
  {
    case IOT_DATA_INT8: ((iot_data_value_t*) data)->value.i8 += val; break;
    case IOT_DATA_UINT8: ((iot_data_value_t*) data)->value.ui8 += val; break;
    case IOT_DATA_INT16: ((iot_data_value_t*) data)->value.i16 += val; break;
    case IOT_DATA_UINT16: ((iot_data_value_t*) data)->value.ui16 += val; break;
    case IOT_DATA_INT32: ((iot_data_value_t*) data)->value.i32 += val; break;
    case IOT_DATA_UINT32: ((iot_data_value_t*) data)->value.ui32 += val; break;
    case IOT_DATA_INT64: ((iot_data_value_t*) data)->value.i64 += val; break;
    case IOT_DATA_UINT64: ((iot_data_value_t*) data)->value.ui64 += val; break;
    case IOT_DATA_FLOAT32: ((iot_data_value_t*) data)->value.f32 += val; break;
    case IOT_DATA_FLOAT64: ((iot_data_value_t*) data)->value.f64 += val; break;
    default: break;
  }
}

void iot_data_increment (iot_data_t * data)
{
  iot_data_inc_dec (data, 1);
}

void iot_data_decrement (iot_data_t * data)
{
  iot_data_inc_dec (data, -1);
}

iot_data_t * iot_data_alloc_from_strings (const char * type, const char * value)
{
  return iot_data_alloc_from_string (iot_data_name_type (type), value);
}

iot_data_t * iot_data_alloc_i8 (int8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT8, false);
  data->value.i8 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui8 (uint8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT8, false);
  data->value.ui8 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i16 (int16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT16, false);
  data->value.i16 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui16 (uint16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT16, false);
  data->value.ui16 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i32 (int32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT32, false);
  data->value.i32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui32 (uint32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT32, false);
  data->value.ui32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i64 (int64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT64, false);
  data->value.i64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui64 (uint64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT64, false);
  data->value.ui64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f32 (float val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT32, false);
  data->value.f32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f64 (double val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT64, false);
  data->value.f64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_bool (bool val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_BOOL, false);
  data->value.bl = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_null (void)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_NULL, false);
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_uuid_string (void)
{
  char uuid_str[UUID_STR_LEN];
  uuid_t uuid;
  uuid_generate (uuid);
  uuid_unparse (uuid, uuid_str);
  return iot_data_alloc_string (uuid_str, IOT_DATA_COPY);
}

iot_data_t * iot_data_alloc_uuid (void)
{
  uuid_t uuid;
  uuid_generate (uuid);
  return iot_data_alloc_array (uuid, sizeof (uuid_t), IOT_DATA_UINT8, IOT_DATA_COPY);
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

extern iot_data_t * iot_data_alloc_array (void * data, uint32_t length, iot_data_type_t type, iot_data_ownership_t ownership)
{
  assert ((type < IOT_DATA_STRING) && ((length > 0 && data != NULL) || length == 0));
  iot_data_array_t * array = iot_data_factory_alloc (IOT_DATA_ARRAY);
  array->base.sub_type = type;
  array->data = length ? data : NULL;
  array->length = length;
  array->size = iot_data_type_size[type] * length;
  array->base.hash = data ? iot_hash_data (data, array->size) : 0;
  array->base.release = data ? (ownership != IOT_DATA_REF) : false;
  if (length && (ownership == IOT_DATA_COPY))
  {
    array->data = malloc (array->size);
    memcpy (array->data, data, array->size);
  }
  return (iot_data_t*) array;
}

extern iot_data_type_t iot_data_array_type (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY));
  return array->sub_type;
}

extern bool iot_data_array_is_of_type (const iot_data_t * array, iot_data_type_t type)
{
  return (array && (array->type == IOT_DATA_ARRAY) && (array->sub_type == type));
}

extern uint32_t iot_data_array_size (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY));
  return ((iot_data_array_t*) array)->size;
}

extern uint32_t iot_data_array_length (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY));
  return ((iot_data_array_t*) array)->length;
}

iot_data_t * iot_data_alloc_array_from_base64 (const char * value)
{
  size_t len;
  uint8_t * data;
  iot_data_t * result = NULL;
  assert (value);
  len = iot_b64_maxdecodesize (value);
  data = malloc (len);
  assert (data);

  if (iot_b64_decode (value, data, &len))
  {
    result = iot_data_alloc_array (data, len, IOT_DATA_UINT8, IOT_DATA_TAKE);
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
  return ((iot_data_value_t*) data)->value.i8;
}

uint8_t iot_data_ui8 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT8));
  return ((iot_data_value_t*) data)->value.ui8;
}

int16_t iot_data_i16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT16));
  return ((iot_data_value_t*) data)->value.i16;
}

uint16_t iot_data_ui16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT16));
  return ((iot_data_value_t*) data)->value.ui16;
}

int32_t iot_data_i32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT32));
  return ((iot_data_value_t*) data)->value.i32;
}

uint32_t iot_data_ui32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT32));
  return ((iot_data_value_t*) data)->value.ui32;
}

int64_t iot_data_i64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT64));
  return ((iot_data_value_t*) data)->value.i64;
}

uint64_t iot_data_ui64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT64));
  return ((iot_data_value_t*) data)->value.ui64;
}

float iot_data_f32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT32));
  return ((iot_data_value_t*) data)->value.f32;
}

double iot_data_f64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT64));
  return ((iot_data_value_t*) data)->value.f64;
}

bool iot_data_bool (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_BOOL));
  return ((iot_data_value_t*) data)->value.bl;
}

const char * iot_data_string (const iot_data_t * data)
{
  assert (data);
  return (data->type == IOT_DATA_STRING) ? ((iot_data_value_t*) data)->value.str : NULL;
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
    iot_data_t * k = iot_data_alloc_string (key, IOT_DATA_REF);
    ret = iot_data_map_remove (map, k);
    iot_data_free (k);
  }
  return ret;
}

void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val)
{
  assert (map && (map->type == IOT_DATA_MAP));
  assert (key && key->type == map->sub_type);
  if (iot_node_add ((iot_data_map_t*) map, key, val))
  {
    ((iot_data_map_t*) map)->size++;
  }
}

uint32_t iot_data_map_size (const iot_data_t * map)
{
  assert (map && (map->type == IOT_DATA_MAP));
  return ((iot_data_map_t*) map)->size;
}

bool iot_data_map_base64_to_array (iot_data_t * map, const iot_data_t * key)
{
  assert (map && (map->type == IOT_DATA_MAP));
  assert (key && key->type == map->sub_type);

  bool result = false;
  iot_data_map_t * mp = (iot_data_map_t*) map;

  iot_node_t * node = iot_node_find (mp->tree, key);
  if (node && (node->value->type == IOT_DATA_STRING))
  {
    const char * str = ((iot_data_value_t*) node->value)->value.str;
    iot_data_t * array = iot_data_alloc_array_from_base64 (str);

    result = (array != NULL);

    if (result)
    {
      iot_data_free (node->value);
      node->value = array;
    }
  }
  return result;
}

const iot_data_t * iot_data_map_get (const iot_data_t * map, const iot_data_t * key)
{
  assert (map && key && (map->type == IOT_DATA_MAP));
  iot_node_t * node = iot_node_find (((iot_data_map_t*) map)->tree, key);
  return node ? node->value : NULL;
}

const iot_data_t * iot_data_string_map_get (const iot_data_t * map, const char * key)
{
  assert (map && key);
  iot_data_t * dkey = iot_data_alloc_string (key, IOT_DATA_REF);
  const iot_data_t * value = iot_data_map_get (map, dkey);
  iot_data_free (dkey);
  return value;
}

const char * iot_data_string_map_get_string (const iot_data_t * map, const char * key)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_STRING)) ? iot_data_string (data) : NULL;
}

int64_t iot_data_string_map_get_i64 (const iot_data_t * map, const char * key, int64_t default_val)
{
  const iot_data_t * data = iot_data_string_map_get (map, key);
  return (data && (iot_data_type (data) == IOT_DATA_INT64)) ? iot_data_i64 (data) : default_val;
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

iot_data_type_t iot_data_map_key_type (const iot_data_t * map)
{
  assert (map && (map->type == IOT_DATA_MAP));
  return map->sub_type;
}

extern bool iot_data_map_key_is_of_type (const iot_data_t * map, iot_data_type_t type)
{
  return (map && (map->type == IOT_DATA_MAP) && (map->sub_type == type));
}

void iot_data_vector_add (iot_data_t * vector, uint32_t index, iot_data_t * val)
{
  iot_data_vector_t * arr = (iot_data_vector_t*) vector;
  assert (val && vector && (vector->type == IOT_DATA_VECTOR));
  assert (index < arr->size);
  iot_data_t * element = arr->values[index];
  iot_data_free (element);
  arr->values[index] = val;
}

const iot_data_t * iot_data_vector_get (const iot_data_t * vector, uint32_t index)
{
  iot_data_vector_t * arr = (iot_data_vector_t*) vector;
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
  vec->size = size;
}

uint32_t iot_data_vector_size (const iot_data_t * vector)
{
  assert (vector && (vector->type == IOT_DATA_VECTOR));
  return ((iot_data_vector_t*) vector)->size;
}

void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter)
{
  assert (iter && map && map->type == IOT_DATA_MAP);
  iter->map = (iot_data_map_t*) map;
  iter->node = NULL;
}

bool iot_data_map_iter_next (iot_data_map_iter_t * iter)
{
  assert (iter);
  iter->node = (iter->node) ? iot_node_next (iter->node) : iot_node_start (iter->map->tree);
  return (iter->node != NULL);
}

const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->node) ? iter->node->key : NULL;
}

const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return iter->node ? iter->node->value : NULL;
}

iot_data_t * iot_data_map_iter_replace_value (iot_data_map_iter_t * iter, iot_data_t *value)
{
  assert (iter);
  iot_data_t * res = (iter->node) ? iter->node->value : NULL;
  if (res) iter->node->value = value;
  return res;
}

const char * iot_data_map_iter_string_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->node) ? iot_data_string (iot_data_map_iter_key (iter)) : NULL;
}

const char * iot_data_map_iter_string_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->node) ? iot_data_string (iot_data_map_iter_value (iter)) : NULL;
}

void iot_data_array_iter (const iot_data_t * array, iot_data_array_iter_t * iter)
{
  assert (iter && array && array->type == IOT_DATA_ARRAY);
  iter->array = (iot_data_array_t*) array;
  iter->index = 0;
}

bool iot_data_array_iter_next (iot_data_array_iter_t * iter)
{
  assert (iter);
  iter->index = (iter->index <= iter->array->length) ? iter->index + 1 : 1;
  return (iter->index <= iter->array->length);
}

uint32_t iot_data_array_iter_index (const iot_data_array_iter_t * iter)
{
  assert (iter);
  return (iter->index - 1);
}

const void * iot_data_array_iter_value (const iot_data_array_iter_t * iter)
{
  assert (iter);
  return (iter->index <= iter->array->length) ? ((char*) (iter->array->data) + (iter->index - 1) * iot_data_type_size[iter->array->base.sub_type]) : NULL;
}

void iot_data_vector_iter (const iot_data_t * vector, iot_data_vector_iter_t * iter)
{
  assert (iter && vector && vector->type == IOT_DATA_VECTOR);
  iter->vector = (iot_data_vector_t*) vector;
  iter->index = 0;
}

bool iot_data_vector_iter_next (iot_data_vector_iter_t * iter)
{
  assert (iter);
  iter->index = (iter->index <= iter->vector->size) ? iter->index + 1 : 1;
  return (iter->index <= iter->vector->size);
}

uint32_t iot_data_vector_iter_index (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return (iter->index - 1);
}

const iot_data_t * iot_data_vector_iter_value (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return (iter->index <= iter->vector->size) ? iter->vector->values[iter->index - 1] : NULL;
}

iot_data_t * iot_data_vector_iter_replace_value (iot_data_vector_iter_t * iter, iot_data_t *value)
{
  assert (iter);
  iot_data_t *res = NULL;
  if (iter->index <= iter->vector->size)
  {
    res = iter->vector->values[iter->index - 1];
    iter->vector->values[iter->index - 1] = value;
  }
  return res;
}

const char * iot_data_vector_iter_string (const iot_data_vector_iter_t * iter)
{
  assert (iter);
  return (iter->index <= iter->vector->size) ? iot_data_string (iter->vector->values[iter->index - 1]) : NULL;
}

const iot_data_t * iot_data_vector_find (const iot_data_t * vector, iot_data_cmp_fn cmp, const void * arg)
{
  const iot_data_t * result = NULL;
  iot_data_vector_iter_t iter;
  iot_data_vector_iter (vector, &iter);
  while (iot_data_vector_iter_next (&iter))
  {
    const iot_data_t * val = iot_data_vector_iter_value (&iter);
    if (cmp (val, arg))
    {
      result = val;
      break;
    }
  }
  return result;
}

static size_t iot_data_repr_size (char c)
{
  return (strchr ("\"\\\b\f\n\r\t", c)) ? 2 : ((c >= '\x00' && c <=  '\x1f') ? 6 : 1);
}

static void iot_data_holder_realloc (iot_string_holder_t * holder, size_t required)
{
  size_t inc = holder->size > IOT_JSON_BUFF_DOUBLING_LIMIT ? IOT_JSON_BUFF_INCREMENT : holder->size;
  if (inc < required) inc = required;
  holder->size += inc;
  holder->free += inc;
  holder->str = realloc (holder->str, holder->size);
}

static void iot_data_strcat_escape (iot_string_holder_t * holder, const char * add, bool escape)
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
        {
          *ptr++ = c;
          break;
        }
        case 2:
        {
          *ptr++ = '\\';
          switch (c)
          {
            case '\"': *ptr++ = '\"'; break;
            case '\\': *ptr++ = '\\'; break;
            case '\b': *ptr++ = 'b'; break;
            case '\f': *ptr++ = 'f'; break;
            case '\n': *ptr++ = 'n'; break;
            case '\r': *ptr++ = 'r'; break;
            case '\t': *ptr++ = 't'; break;
          }
          break;
        }
        case 6:
        {
          *ptr++ = '\\';
          *ptr++ = 'u';
          *ptr++ = '0';
          *ptr++ = '0';
          *ptr++ = (c & 0x10u) ? '1' : '0';
          *ptr++ = hex[c & 0x0fu];
          break;
        }
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

static void iot_data_base64_encode (iot_string_holder_t * holder, const iot_data_t * array)
{
  uint32_t inLen = iot_data_array_size (array);
  if (inLen)
  {
    const uint8_t *data = iot_data_address (array);
    assert (strlen (holder->str) == (holder->size - holder->free - 1));
    size_t len = iot_b64_encodesize (inLen) - 1; /* Allow for string terminator */
    char *out;

    if (holder->free < len)
    {
      iot_data_holder_realloc (holder, len);
    }

    out = holder->str + holder->size - holder->free - 1;
    iot_b64_encode (data, inLen, out, holder->free + 1);
    holder->free -= len;
    assert (strlen (holder->str) == (holder->size - holder->free - 1));
  }
}

static void iot_data_dump_raw (iot_string_holder_t * holder, const iot_data_t * data)
{
  char buff [IOT_VAL_BUFF_SIZE];

  switch (data->type)
  {
    case IOT_DATA_INT8: sprintf (buff, "%" PRId8 , iot_data_i8 (data)); break;
    case IOT_DATA_UINT8: sprintf (buff, "%" PRIu8, iot_data_ui8 (data)); break;
    case IOT_DATA_INT16: sprintf (buff, "%" PRId16, iot_data_i16 (data)); break;
    case IOT_DATA_UINT16: sprintf (buff, "%" PRIu16, iot_data_ui16 (data)); break;
    case IOT_DATA_INT32: sprintf (buff, "%" PRId32, iot_data_i32 (data)); break;
    case IOT_DATA_UINT32: sprintf (buff, "%" PRIu32, iot_data_ui32 (data)); break;
    case IOT_DATA_INT64: sprintf (buff, "%" PRId64, iot_data_i64 (data)); break;
    case IOT_DATA_UINT64: sprintf (buff, "%" PRIu64, iot_data_ui64 (data)); break;
    case IOT_DATA_FLOAT32: snprintf (buff, IOT_VAL_BUFF_SIZE, "%.8e", iot_data_f32 (data)); break;
    case IOT_DATA_FLOAT64: snprintf (buff, IOT_VAL_BUFF_SIZE, "%.16e", iot_data_f64 (data)); break;
    case IOT_DATA_NULL: strcpy (buff, "null"); break;
    default: strcpy (buff, iot_data_bool (data) ? "true" : "false"); break;
  }
  iot_data_strcat_escape (holder, buff, false);
}

static void iot_data_dump (iot_string_holder_t * holder, const iot_data_t * data)
{
  switch (data->type)
  {
    case IOT_DATA_STRING:
    {
      iot_data_add_quote (holder);
      iot_data_strcat (holder, iot_data_string (data));
      iot_data_add_quote (holder);
      break;
    }
    case IOT_DATA_ARRAY:
    {
      iot_data_add_quote (holder);
      iot_data_base64_encode (holder, data);
      iot_data_add_quote (holder);
      break;
    }
    case IOT_DATA_MAP:
    {
      const iot_data_t * metadata = iot_data_get_metadata (data);
      const iot_data_t * ordering = metadata ? iot_data_string_map_get (metadata, ORDERING_KEY) : NULL;
      iot_data_map_iter_t iter;
      bool first = true;
      iot_data_vector_iter_t vec_iter = {};
      if (ordering) iot_data_vector_iter (ordering, &vec_iter);
      iot_data_map_iter (data, &iter);
      iot_data_strcat (holder, "{");
      while (ordering ? iot_data_vector_iter_next (&vec_iter) : iot_data_map_iter_next (&iter))
      {
        const iot_data_t * key;
        const iot_data_t * value;
        if (ordering)
        {
          key = iot_data_vector_iter_value (&vec_iter);
          value = iot_data_map_get (data, key);
        }
        else
        {
          key = iot_data_map_iter_key (&iter);
          value = iot_data_map_iter_value (&iter);
        }
        if (value)
        {
          if (! first) iot_data_strcat (holder, ",");
          if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
          iot_data_dump (holder, key);
          if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
          iot_data_strcat (holder, ":");
          iot_data_dump (holder, value);
          first = false;
        }
      }
      iot_data_strcat (holder, "}");
      break;
    }
    case IOT_DATA_VECTOR:
    {
      iot_data_vector_iter_t iter;
      iot_data_vector_iter (data, &iter);
      iot_data_strcat (holder, "[");
      while (iot_data_vector_iter_next (&iter))
      {
        const iot_data_t * value = iot_data_vector_iter_value (&iter);
        iot_data_dump (holder, value);
        if (iter.index < iter.vector->size)
        {
          iot_data_strcat (holder, ",");
        }
      }
      iot_data_strcat (holder, "]");
      break;
    }
    case IOT_DATA_POINTER: break;
    default: iot_data_dump_raw (holder, data);
  }
}

char * iot_data_to_json (const iot_data_t * data)
{
  return iot_data_to_json_with_size (data, IOT_JSON_BUFF_SIZE);
}

char * iot_data_to_json_with_size (const iot_data_t * data, uint32_t size)
{
  iot_string_holder_t holder;
  assert (data && size > 0);
  holder.str = calloc (1, size);
  holder.size = size;
  holder.free = size - 1; // Allowing for string terminator
  iot_data_dump (&holder, data);
  return holder.str;
}

static char * iot_data_string_from_json_token (const char * json, iot_json_tok_t * token)
{
  size_t len = (size_t) (token->end - token->start);
  char * str = malloc (len + 1);
  if (token->type == IOT_JSON_STRING_ESC)
  {
    const char *src = json + token->start;
    char *dst = str;
    while (src < json + token->start + len)
    {
      if (*src == '\\')
      {
        switch (*++src)
        {
          case 'b':
            *dst++ = '\b';
            break;
          case 'f':
            *dst++ = '\f';
            break;
          case 'r':
            *dst++ = '\r';
            break;
          case 'n':
            *dst++ = '\n';
            break;
          case 't':
            *dst++ = '\t';
            break;
          case 'u':         // leave escaped unicode in place
            *dst++ = '\\';
            *dst++ = 'u';
            break;
          default:
            *dst++ = *src;
        }
        src++;
      }
      else
      {
        *dst++ = *src++;
      }
    }
    *dst = '\0';
  }
  else
  {
    memcpy (str, json + token->start, len);
    str[len] = 0;
  }
  return str;
}

static iot_data_t * iot_data_string_from_json (iot_json_tok_t ** tokens, const char * json)
{
  char * str = iot_data_string_from_json_token (json, *tokens);
  (*tokens)++;
  return iot_data_alloc_string (str, IOT_DATA_TAKE);
}

static iot_data_t * iot_data_primitive_from_json (iot_json_tok_t ** tokens, const char * json)
{
  iot_data_t * ret;
  char * str = iot_data_string_from_json_token (json, *tokens);
  (*tokens)++;
  switch (str[0])
  {
    case 't': case 'f': ret = iot_data_alloc_bool (str[0] == 't'); break; // true/false
    case 'n': ret = iot_data_alloc_null (); break; // null
    default: // Handle all floating point numbers as doubles and integers as uint64_t
      ret = (strchr (str, '.') || strchr (str, 'e') || strchr (str, 'E')) ?
        iot_data_alloc_f64 (strtod (str, NULL)) : iot_data_alloc_i64 (strtol (str, NULL, 0));
      break;
  }
  free (str);
  return ret;
}

static iot_data_t * iot_data_map_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered)
{
  uint32_t elements = (*tokens)->size;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * ordering;
  iot_data_t * metadata;
  uint32_t i = 0;

  (*tokens)++;
  if (ordered)
  {
    ordering = iot_data_alloc_vector (elements);
    metadata = iot_data_alloc_map (IOT_DATA_STRING);
  }
  while  (elements--)
  {
    iot_data_t * key = iot_data_string_from_json (tokens, json);
    if (ordered) iot_data_vector_add (ordering, i++, iot_data_add_ref (key));
    iot_data_map_add (map, key, iot_data_all_from_json (tokens, json, ordered));
  }
  if (ordered)
  {
    iot_data_string_map_add (metadata, ORDERING_KEY, ordering);
    iot_data_set_metadata (map, metadata);
    iot_data_free (metadata);
  }
  return map;
}

static iot_data_t * iot_data_vector_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered)
{
  uint32_t elements = (*tokens)->size;
  uint32_t index = 0;
  iot_data_t * vector = iot_data_alloc_vector (elements);

  (*tokens)++;
  while (elements--)
  {
    iot_data_vector_add (vector, index++, iot_data_all_from_json (tokens, json, ordered));
  }
  return vector;
}

static iot_data_t * iot_data_all_from_json (iot_json_tok_t ** tokens, const char * json, bool ordered)
{
  iot_data_t * data = NULL;
  switch ((*tokens)->type)
  {
    case IOT_JSON_PRIMITIVE: data = iot_data_primitive_from_json (tokens, json); break;
    case IOT_JSON_OBJECT: data = iot_data_map_from_json (tokens, json, ordered); break;
    case IOT_JSON_ARRAY: data = iot_data_vector_from_json (tokens, json, ordered); break;
    default: data = iot_data_string_from_json (tokens, json); break;
  }
  return data;
}

iot_data_t * iot_data_from_json (const char * json)
{
  return iot_data_from_json_with_ordering (json, false);
}

iot_data_t * iot_data_from_json_with_ordering (const char * json, bool ordered)
{
  iot_data_t * data = NULL;
  const char * ptr = json;

  if (ptr && *ptr)
  {
    iot_json_parser parser;
    int32_t used;
    uint32_t count = 1;

    // Approximate token count
    while (*ptr != '\0')
    {
      switch (*ptr)
      {
        case ',':
        case '{':
          count++;
          break;
        case ':':
        case '[':
          count += 2;
          break;
        default:
          break;
      }
      ptr++;
    }
    iot_json_tok_t *tokens = calloc (1, sizeof (*tokens) * count);
    iot_json_tok_t *tptr = tokens;

    iot_json_init (&parser);
    used = iot_json_parse (&parser, json, strlen (json), tptr, count);
    if (used && (used <= count))
    {
      data = iot_data_all_from_json (&tptr, json, ordered);
    }
    free (tokens);
  }
  return data ? data : iot_data_alloc_null ();
}

#ifdef IOT_HAS_XML
static iot_data_t * iot_data_map_from_xml (bool root, yxml_t * x, iot_string_holder_t * holder, const char ** str)
{
  iot_data_t * elem = iot_data_alloc_map (IOT_DATA_STRING);
  iot_data_t * children = NULL;
  iot_data_t * attrs = iot_data_alloc_map (IOT_DATA_STRING);
  char * elem_name = x->elem;
  bool more = true;
  holder->str[0] = '\0';
  holder->free = holder->size - 1;
  iot_data_string_map_add (elem, "name", iot_data_alloc_string (elem_name, IOT_DATA_COPY));
  iot_data_string_map_add (elem, "attributes", attrs);
  while (more && **str)
  {
    switch (yxml_parse (x, *(*str)++))
    {
      case YXML_ELEMSTART:
      {
        iot_data_t * child = iot_data_map_from_xml (false, x, holder, str);
        if (child)
        {
          if (root)
          {
            iot_data_free (elem);
            elem = child;
            more = false;
          }
          else
          {
            uint32_t size;
            if (!children)
            {
              children = iot_data_alloc_vector (1);
              iot_data_string_map_add (elem, "children", children);
              size = 0;
            }
            else
            {
              size = iot_data_vector_size (children);
              iot_data_vector_resize (children, size + 1);
            }
            iot_data_vector_add (children, size, child);
          }
        }
        else
        {
          iot_data_free (elem);
          elem = NULL;
          more = false;
        }
        break;
      }
      case YXML_ELEMEND:
      {
        if (holder->str[0] != '\0')
        {
          iot_data_string_map_add (elem, "content", iot_data_alloc_string (holder->str, IOT_DATA_COPY));
          holder->str[0] = '\0';
          holder->free = holder->size - 1;
        }
        more = false;
        break;
      }
      case YXML_ATTRVAL:
      case YXML_CONTENT:
      {
        iot_data_strcat_escape (holder, x->data, false);
        break;
      }
      case YXML_ATTREND:
      {
        iot_data_map_add (attrs, iot_data_alloc_string (x->attr, IOT_DATA_COPY), iot_data_alloc_string (holder->str, IOT_DATA_COPY));
        holder->str[0] = '\0';
        holder->free = holder->size - 1;
        break;
      }
      case YXML_EEOF:
      case YXML_EREF:
      case YXML_ECLOSE:
      case YXML_ESTACK:
      case YXML_ESYN:
      {
        // Error
        iot_data_free (elem);
        elem = NULL;
        more = false;
        break;
      }
      default:
      {
        break;
      }
    }
  }

  return elem;
}

iot_data_t * iot_data_from_xml (const char * xml)
{
  iot_data_t * result;
  yxml_t * x = malloc (sizeof (yxml_t) + YXML_PARSER_BUFF_SIZE);
  iot_string_holder_t holder;
  holder.str = calloc (1, YXML_BUFF_SIZE);
  holder.size = YXML_BUFF_SIZE;
  holder.free = YXML_BUFF_SIZE - 1; // Allowing for string terminator
  yxml_init (x, x+1, YXML_PARSER_BUFF_SIZE);
  result = iot_data_map_from_xml (true, x, &holder, &xml);
  free (x);
  free (holder.str);
  return result;
}
#endif

iot_data_t * iot_data_copy (const iot_data_t * src)
{
  iot_data_t * data = (iot_data_t*) src;
  iot_data_t * ret;

  if (src == NULL) return NULL;

  switch (data->type)
  {
    case IOT_DATA_STRING:
    {
      iot_data_value_t * val = (iot_data_value_t *) data;
      ret = iot_data_alloc_string (val->value.str, val->base.release ? IOT_DATA_COPY : IOT_DATA_REF);
      break;
    }
    case IOT_DATA_ARRAY:
    {
      iot_data_array_t * array = (iot_data_array_t*) data;
      ret = iot_data_alloc_array (array->data, array->length, array->base.sub_type, array->base.release ? IOT_DATA_COPY : IOT_DATA_REF);
      break;
    }
    case IOT_DATA_MAP:
    {
      iot_data_map_iter_t iter;
      ret = iot_data_alloc_map (iot_data_map_key_type (src));

      iot_data_map_iter (src, &iter);
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
      ret = iot_data_alloc_vector (iot_data_vector_size (src));

      iot_data_vector_iter (src, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        iot_data_t * val = iot_data_copy (iot_data_vector_iter_value (&iter));
        iot_data_vector_add (ret, iter.index-1, val);
      }
      break;
    }
    case IOT_DATA_POINTER:
    {
      ret = (iot_data_t*) iot_data_alloc_pointer (((iot_data_pointer_t*) data)->value, ((iot_data_pointer_t*) data)->free_fn);
      iot_data_add_ref (ret);
      break;
    }
    default: // basic types
    {
      iot_data_value_t * val = iot_data_value_alloc (data->type, false);
      val->value.ui64 = (((iot_data_value_t*) data)->value.ui64);
      ret = (iot_data_t*) val;
    }
  }
  iot_data_set_metadata (ret, data->metadata);
  return ret;
}

static iot_typecode_t iot_basic_tcs [13] =
{
  { .type = IOT_DATA_INT8 }, { .type = IOT_DATA_UINT8 }, { .type = IOT_DATA_INT16 }, { .type = IOT_DATA_UINT16 },
  { .type = IOT_DATA_INT32 }, { .type = IOT_DATA_UINT32 }, { .type = IOT_DATA_INT64 }, { .type = IOT_DATA_UINT64 },
  { .type = IOT_DATA_FLOAT32 }, { .type = IOT_DATA_FLOAT64, }, { .type = IOT_DATA_BOOL }, { .type = IOT_DATA_STRING }, { .type = IOT_DATA_NULL }
};

extern iot_typecode_t * iot_typecode_alloc_basic (iot_data_type_t type)
{
  assert (type < IOT_DATA_ARRAY);
  return &iot_basic_tcs[type];
}

extern iot_typecode_t * iot_typecode_alloc_array (iot_data_type_t element_type)
{
  static iot_typecode_t iot_array_tcs [11] =
  {
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[0] }, { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[1] },
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[2] }, { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[3] },
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[4] }, { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[5] },
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[6] }, { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[7] },
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[8] }, { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[9] },
    { .type = IOT_DATA_ARRAY, .element_type = &iot_basic_tcs[10] }
  };
  assert (element_type < IOT_DATA_STRING);
  return &iot_array_tcs[element_type];
}

extern iot_typecode_t * iot_typecode_alloc_map (iot_data_type_t key_type, iot_typecode_t * element_type)
{
  iot_typecode_t * tc = iot_data_block_alloc ();
  tc->type = IOT_DATA_MAP;
  tc->key_type = key_type;
  tc->element_type = element_type;
  return tc;
}

extern iot_typecode_t * iot_typecode_alloc_vector (iot_typecode_t * element_type)
{
  iot_typecode_t * tc = iot_data_block_alloc ();
  tc->type = IOT_DATA_VECTOR;
  tc->element_type = element_type;
  return tc;
}

extern iot_typecode_t * iot_typecode_alloc_pointer (void)
{
  static iot_typecode_t tc = { .type = IOT_DATA_POINTER, .element_type = NULL };
  return &tc;
}

extern void iot_typecode_free (iot_typecode_t * typecode)
{
  if (typecode && (typecode->type > IOT_DATA_ARRAY) && (typecode->type != IOT_DATA_POINTER))
  {
    iot_data_block_free ((iot_data_t*) typecode);
  }
}

extern iot_data_type_t iot_typecode_type (const iot_typecode_t * typecode)
{
  assert (typecode);
  return typecode->type;
}

const char * iot_typecode_type_name (const iot_typecode_t * typecode)
{
  assert (typecode && (typecode->type <= IOT_DATA_POINTER));
  return iot_data_type_names[typecode->type];
}

extern iot_data_type_t iot_typecode_key_type (const iot_typecode_t * typecode)
{
  assert (typecode && (typecode->type == IOT_DATA_MAP));
  return typecode->key_type;
}

extern const iot_typecode_t * iot_typecode_element_type (const iot_typecode_t * typecode)
{
  assert (typecode && (typecode->type >= IOT_DATA_ARRAY) && (typecode->type != IOT_DATA_POINTER));
  return typecode->element_type;
}

extern bool iot_typecode_equal (const iot_typecode_t * tc1, const iot_typecode_t * tc2)
{
  if (tc1 == tc2) return true;
  if (tc1 == NULL || tc2 == NULL) return false;
  if ((tc1->type != tc2->type) || (tc1->key_type != tc2->key_type)) return false;
  return iot_typecode_equal (tc1->element_type, tc2->element_type);
}

extern bool iot_data_matches (const iot_data_t * data, const iot_typecode_t * typecode)
{
  iot_typecode_t * tc = iot_data_typecode (data);
  bool match = iot_typecode_equal (tc, typecode);
  iot_typecode_free (tc);
  return match;
}

extern iot_typecode_t * iot_data_typecode (const iot_data_t * data)
{
  assert (data);
  iot_typecode_t * tc;
  iot_data_type_t type = data->type;

  if (type < IOT_DATA_ARRAY)
  {
    tc = iot_typecode_alloc_basic (type);
  }
  else if (type == IOT_DATA_ARRAY)
  {
    tc = iot_typecode_alloc_array (data->sub_type);
  }
  else if (type == IOT_DATA_POINTER)
  {
    tc = iot_typecode_alloc_pointer ();
  }
  else
  {
    tc = iot_data_block_alloc ();
    tc->type = type;
    if (type == IOT_DATA_MAP)
    {
      iot_data_map_iter_t iter;
      iot_typecode_t * etype;
      iot_data_map_iter (data, &iter);
      tc->key_type = data->sub_type;
      while (iot_data_map_iter_next (&iter))
      {
        etype = iot_data_typecode (iot_data_map_iter_value (&iter));
        if (tc->element_type && ! iot_typecode_equal (etype, tc->element_type))
        {
          tc->element_type = NULL;
          break;
        }
        tc->element_type = etype;
      }
    }
    else
    {
      iot_data_vector_iter_t iter;
      iot_typecode_t * etype;
      iot_data_vector_iter (data, &iter);
      while (iot_data_vector_iter_next (&iter))
      {
        etype = iot_data_typecode (iot_data_vector_iter_value (&iter));
        if (tc->element_type && ! iot_typecode_equal (etype, tc->element_type))
        {
          tc->element_type = NULL;
          break;
        }
        tc->element_type = etype;
      }
    }
  }
  return tc;
}

/* Red/Black binary tree manipulation functions. Implements iot_data_map_t.
 *
 * https://algorithmtutor.com/Data-Structures/Tree/Red-Black-Trees/
 *
 * Note that logic regards NULL nodes as Black. So all colour get/set operations
 * need checking for node being NULL.
 */

static inline iot_node_colour_t iot_node_colour (iot_node_t * node)
{
  return (node) ? node->colour : IOT_NODE_BLACK;
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

static inline iot_node_t * iot_node_grandparent (iot_node_t * node)
{
  return (node && node->parent) ? node->parent->parent : NULL;
}

static inline iot_node_t * iot_node_uncle (iot_node_t * node)
{
  iot_node_t * gp = iot_node_grandparent (node);
  return (gp) ? ((node->parent == gp->left) ? gp->right : gp->left) : NULL;
}

static inline iot_node_t * iot_node_sibling (iot_node_t * node)
{
  return IS_LEFT (node) ? node->parent->right : node->parent->left;
}

static inline iot_node_t * iot_node_alloc (iot_node_t * parent, iot_data_t * key, iot_data_t * value)
{
  iot_node_t * nn = (iot_node_t*) iot_data_block_alloc ();
  nn->value = value;
  nn->key = key;
  nn->parent = parent;
  nn->colour = IOT_NODE_RED;
  return nn;
}

static void iot_node_delete (iot_node_t * node)
{
  iot_data_free (node->key);
  iot_data_free (node->value);
  iot_data_block_free ((iot_data_t*) node);
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
        if (IS_RIGHT_BLACK (s)) // case 3.3
        {
          if (s) iot_node_set_colour (s->left, IOT_NODE_BLACK);
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
        if (IS_LEFT_BLACK (s)) // case 3.3
        {
          if (s) iot_node_set_colour (s->right, IOT_NODE_BLACK);
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

static iot_node_t * iot_node_find (iot_node_t * node, const iot_data_t * key)
{
  while (node)
  {
    int cmp = iot_data_key_cmp (node->key, key);
    if (cmp == 0) break;
    node = (cmp > 0) ? node->left : node->right;
  }
  return node;
}

static void iot_node_insert (iot_data_map_t * map, iot_data_t * key, iot_data_t * value)
{
  iot_node_t * node = iot_node_alloc (NULL, key, value);
  iot_node_t * y = NULL;
  iot_node_t * x = map->tree;
  while (x)
  {
    y = x;
    x = (iot_data_key_cmp (key, x->key) < 0) ? x->left : x->right;
  }
  node->parent = y;
  if (y == NULL) map->tree = node;
  else if (iot_data_key_cmp (key, y->key) < 0) y->left = node;
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
    iot_data_free (key);
    iot_data_free (node->value);
    node->value = value;
  }
  else
  {
    iot_node_insert (map, key, value);
  }
  return (node == NULL);
}

static void iot_node_free (iot_node_t * node)
{
  if (node)
  {
    iot_node_free (node->left);
    iot_node_free (node->right);
    iot_node_delete (node);
  }
}

static iot_node_t * iot_node_start (iot_node_t * node)
{
  iot_node_t * start = node;
  if (start) while (start->left) start = start->left;
  return start;
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
