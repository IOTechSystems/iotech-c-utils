//
// Copyright (c) 2019-2020 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/typecode.h"
#include "iot/json.h"
#include "iot/base64.h"

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

static const char * iot_data_type_names [] = {"Int8","UInt8","Int16","UInt16","Int32","UInt32","Int64","UInt64","Float32","Float64","Bool","String","Array","Map","Vector"};
static const uint8_t iot_data_type_size [] = { 1u, 1u, 2u, 2u, 4u, 4u, 8u, 8u, 4u, 8u, sizeof (bool), sizeof (char*) };

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
  iot_data_t * next;
  iot_data_t * metadata;
  atomic_uint_fast32_t refs;
  iot_data_type_t type : 8;
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
  iot_data_type_t type;
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

typedef struct iot_data_pair_t
{
  iot_data_t base;
  iot_data_t * key;
  iot_data_t * value;
} iot_data_pair_t;

typedef struct iot_data_map_t
{
  iot_data_t base;
  iot_data_type_t key_type;
  uint32_t size;
  iot_data_pair_t * head;
  iot_data_pair_t * tail;
} iot_data_map_t;

typedef struct iot_string_holder_t
{
  char * str;
  size_t size;
  size_t free;
} iot_string_holder_t;

// Determine minimum block size that can hold all iot_data types, maximum size of
// value string cache buffer and number of blocks per allocated memory chunk.

#define IOT_DATA_BLOCK_SIZE (sizeof (iot_data_map_t))
#define IOT_DATA_BLOCKS ((IOT_MEMORY_BLOCK_SIZE / IOT_DATA_BLOCK_SIZE) - 1)
#define IOT_DATA_VALUE_BUFF_SIZE (IOT_DATA_BLOCK_SIZE - sizeof (iot_data_value_base_t))

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

_Static_assert ((IOT_DATA_BLOCK_SIZE % 8) == 0, "IOT_DATA_BLOCK_SIZE not 8 byte aligned");
_Static_assert (sizeof (iot_data_value_t) == IOT_DATA_BLOCK_SIZE, "size of iot_data_value_t not equal to IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_map_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_map_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_vector_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_array_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_array_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_pair_t) <= IOT_DATA_BLOCK_SIZE, "iot_data_pair_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_typecode_t) <= IOT_DATA_BLOCK_SIZE, "iot_typecode_t bigger than IOT_DATA_BLOCK_SIZE");
_Static_assert (sizeof (iot_memory_block_t) <= IOT_MEMORY_BLOCK_SIZE, "iot_memory_block_t bigger than IOT_MEMORY_BLOCK_SIZE");
_Static_assert (sizeof (iot_data_vector_t) <= sizeof (iot_data_map_t), "iot_data_vector_t bigger than iot_data_map_t");
_Static_assert (sizeof (iot_data_array_t) <= sizeof (iot_data_map_t), "iot_data_array_t bigger than iot_data_map_t");

extern void iot_data_init (void);
extern void iot_data_fini (void);

// Data cache usually disabled for debug builds as otherwise too difficult to trace leaks

#ifdef IOT_DATA_CACHE
static iot_data_t * iot_data_cache = NULL;
static iot_memory_block_t * iot_data_blocks = NULL;
#ifdef IOT_HAS_SPINLOCK
static pthread_spinlock_t iot_data_slock;
#endif
static pthread_mutex_t iot_data_mutex;
#endif

static iot_data_t * iot_data_all_from_json (iot_json_tok_t ** tokens, const char * json);

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
  while (iot_data_cache <= (iot_data_t *) 1)
  {
    bool allocate = (iot_data_cache == NULL);
    if (allocate) iot_data_cache = (iot_data_t *) 1;
    pthread_spin_unlock (&iot_data_slock);
    pthread_mutex_lock (&iot_data_mutex);
#else
    bool allocate = (iot_data_cache == NULL);
#endif
    if (allocate)
    {
      iot_memory_block_t * block = calloc (1, IOT_MEMORY_BLOCK_SIZE);
      block->next = iot_data_blocks;
      iot_data_blocks = block;

      uint8_t * iter = (uint8_t*) block->chunks;
      iot_data_cache = (iot_data_t*) iter;
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

static void * iot_data_factory_alloc (void)
{
  iot_data_t * data = iot_data_block_alloc ();
  atomic_store (&data->refs, 1);
  return data;
}

static inline iot_data_value_t * iot_data_value_alloc (iot_data_type_t type, iot_data_ownership_t own)
{
  iot_data_value_t * val = iot_data_factory_alloc ();
  val->base.type = type;
  val->base.release = (own != IOT_DATA_REF);
  return val;
}

void iot_data_init (void)
{
/*
  printf ("sizeof (iot_data_value_t): %zu\n", sizeof (iot_data_value_t));
  printf ("sizeof (iot_data_map_t): %zu\n", sizeof (iot_data_map_t));
  printf ("sizeof (iot_data_vector_t): %zu\n", sizeof (iot_data_vector_t));
  printf ("sizeof (iot_data_array_t): %zu\n", sizeof (iot_data_array_t));
  printf ("sizeof (iot_data_pair_t): %zu\n", sizeof (iot_data_pair_t));
  printf ("IOT_DATA_BLOCK_SIZE %zu IOT_DATA_BLOCKS: %zu\n", IOT_DATA_BLOCK_SIZE, IOT_DATA_BLOCKS);
*/

#ifdef IOT_DATA_CACHE
#ifdef IOT_HAS_SPINLOCK
  pthread_spin_init (&iot_data_slock, 0);
#endif
  pthread_mutex_init (&iot_data_mutex, NULL);
#endif
}

void iot_data_fini (void)
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

void iot_data_add_ref (iot_data_t * data)
{
  assert (data);
  atomic_fetch_add (&data->refs, 1);
}

iot_data_type_t iot_data_name_type (const char * name)
{
  int type = 0;
  while (type >= 0)
  {
    if (strcasecmp (name, iot_data_type_names[type]) == 0) break;
    type = (type == IOT_DATA_VECTOR) ? -1 : (type + 1);
  }
  return type;
}

const char * iot_data_type_string (iot_data_type_t type)
{
  assert (type <= IOT_DATA_VECTOR);
  return iot_data_type_names[type];
}

const char * iot_data_type_name (const iot_data_t * data)
{
  assert (data && (data->type <= IOT_DATA_VECTOR));
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
  assert (data);
  return data->metadata;
}

bool iot_data_equal (const iot_data_t * v1, const iot_data_t * v2)
{
  assert (v1 && v2);
  if (v1 == v2) return true;
  if (v1->type == v2->type)
  {
    switch (v1->type)
    {
      case IOT_DATA_STRING: return ((iot_data_value_t*) v1)->value.str == ((iot_data_value_t*) v2)->value.str || (strcmp (((iot_data_value_t*) v1)->value.str, ((iot_data_value_t*) v2)->value.str) == 0);
      case IOT_DATA_ARRAY:
      {
        iot_data_array_t * a1 = (iot_data_array_t*) v1;
        iot_data_array_t * a2 = (iot_data_array_t*) v2;
        return  ((a1->size == a2->size) && (a1->type == a2->type) && ((a1->data == a2->data) || (memcmp (a1->data, a2->data, a1->size) == 0)));
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
      default: return (((iot_data_value_t*) v1)->value.ui64 == ((iot_data_value_t*) v2)->value.ui64);
    }
  }
  return false;
}


iot_data_t * iot_data_alloc_map (iot_data_type_t key_type)
{
  assert (key_type < IOT_DATA_MAP);
  iot_data_map_t * map = iot_data_factory_alloc ();
  map->base.type = IOT_DATA_MAP;
  map->key_type = key_type;
  return (iot_data_t*) map;
}

iot_data_t * iot_data_alloc_vector (uint32_t size)
{
  iot_data_vector_t * vector = iot_data_factory_alloc ();
  vector->base.type = IOT_DATA_VECTOR;
  vector->size = size;
  vector->values = calloc (size, sizeof (iot_data_t*));
  return (iot_data_t*) vector;
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
  return (data && data->type <= IOT_DATA_ARRAY) ? ((data->type == IOT_DATA_ARRAY) ? ((iot_data_array_t*) data)->data : (void*)&(((iot_data_value_t*) data)->value)) : NULL;
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
        iot_data_map_t * map = (iot_data_map_t*) data;
        iot_data_pair_t * pair;
        while ((pair = map->head))
        {
          iot_data_free (pair->key);
          iot_data_free (pair->value);
          map->head = (iot_data_pair_t *) pair->base.next;
          iot_data_block_free (&pair->base);
        }
        map->size = 0;
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
    case IOT_DATA_UINT32: return iot_data_alloc_ui32 ((uint32_t) atol (value));
    case IOT_DATA_INT64: return iot_data_alloc_i64 ((int64_t) atoll (value));
    case IOT_DATA_UINT64: return iot_data_alloc_ui64 ((uint64_t) atoll (value));
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

iot_data_t * iot_data_alloc_string (const char * val, iot_data_ownership_t ownership)
{
  assert (val);
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_STRING, ownership);
  data->value.str = (char*) val;
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
  assert (data && length && (type < IOT_DATA_STRING));
  iot_data_array_t * array = iot_data_factory_alloc ();
  array->base.type = IOT_DATA_ARRAY;
  array->type = type;
  array->data = data;
  array->length = length;
  array->size = iot_data_type_size[type] * length;
  array->base.release = (ownership != IOT_DATA_REF);
  if (ownership == IOT_DATA_COPY)
  {
    array->data = malloc (array->size);
    memcpy (array->data, data, array->size);
  }
  return (iot_data_t*) array;
}

extern iot_data_type_t iot_data_array_type (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY));
  return ((iot_data_array_t*) array)->type;
}

extern bool iot_data_array_is_of_type (const iot_data_t * array, iot_data_type_t type)
{
  return (array && (array->type == IOT_DATA_ARRAY) && (((iot_data_array_t*) array)->type == type));
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
  assert (data && (data->type == IOT_DATA_STRING));
  return ((iot_data_value_t*) data)->value.str;
}

static iot_data_pair_t * iot_data_map_find (iot_data_map_t * map, const iot_data_t * key)
{
  iot_data_pair_t * pair = map->head;
  while (pair)
  {
    if (iot_data_equal (pair->key, key))
    {
      break;
    }
    pair = (iot_data_pair_t*) pair->base.next;
  }
  return pair;
}

void iot_data_string_map_add (iot_data_t * map, const char * key, iot_data_t * val)
{
  assert (key);
  iot_data_map_add (map, iot_data_alloc_string (key, IOT_DATA_REF), val);
}

void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val)
{
  iot_data_map_t * mp = (iot_data_map_t*) map;

  assert (mp && (mp->base.type == IOT_DATA_MAP));
  assert (key && key->type == mp->key_type);

  iot_data_pair_t * pair = iot_data_map_find (mp, key);
  if (pair)
  {
    iot_data_free (pair->value);
    iot_data_free (pair->key);
  }
  else
  {
    pair = (iot_data_pair_t*) iot_data_factory_alloc ();
    if (mp->tail) mp->tail->base.next = &pair->base;
    mp->tail = pair;
    if (mp->head == NULL) mp->head = pair;
    mp->size++;
  }
  pair->value = val;
  pair->key = key;
}

uint32_t iot_data_map_size (const iot_data_t * map)
{
  iot_data_map_t * mp = (iot_data_map_t*) map;
  assert (mp && (mp->base.type == IOT_DATA_MAP));
  return mp->size;
}

bool iot_data_map_base64_to_array (iot_data_t * map, const iot_data_t * key)
{
  bool result = false;
  iot_data_map_t * mp = (iot_data_map_t*) map;

  assert (mp && (mp->base.type == IOT_DATA_MAP));
  assert (key && key->type == mp->key_type);

  iot_data_pair_t * pair = iot_data_map_find (mp, key);
  if (pair && (pair->value->type == IOT_DATA_STRING))
  {
    const char * str = ((iot_data_value_t*) pair->value)->value.str;
    iot_data_t * array = iot_data_alloc_array_from_base64 (str);

    result = (array != NULL);

    if (result)
    {
      iot_data_free (pair->value);
      pair->value = array;
    }
  }
  return result;
}

const iot_data_t * iot_data_map_get (const iot_data_t * map, const iot_data_t * key)
{
  iot_data_map_t * mp = (iot_data_map_t*) map;
  assert (mp && key && (mp->base.type == IOT_DATA_MAP));
  iot_data_pair_t * pair = iot_data_map_find (mp, key);
  return pair ? pair->value : NULL;
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
  return data ? iot_data_string (data) : NULL;
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
  assert (map);
  return ((iot_data_map_t*) map)->key_type;
}

extern bool iot_data_map_key_is_of_type (const iot_data_t * map, iot_data_type_t type)
{
  return (map && (map->type == IOT_DATA_MAP) && (((iot_data_array_t*) map)->type == type));
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
  iter->pair = NULL;
  iter->map = (iot_data_map_t*) map;
}

bool iot_data_map_iter_next (iot_data_map_iter_t * iter)
{
  assert (iter);
  iter->pair = iter->pair ? (iot_data_pair_t*) iter->pair->base.next : iter->map->head;
  return (iter->pair != NULL);
}

const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->pair) ? iter->pair->key : NULL;
}

const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->pair) ? iter->pair->value : NULL;
}

iot_data_t * iot_data_map_iter_replace_value (iot_data_map_iter_t * iter, iot_data_t *value)
{
  assert (iter);
  iot_data_t *res = (iter->pair) ? iter->pair->value : NULL;
  if (res)
  {
    iter->pair->value = value;
  }
  return res;
}

const char * iot_data_map_iter_string_key (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->pair) ? iot_data_string (iot_data_map_iter_key (iter)) : NULL;
}

const char * iot_data_map_iter_string_value (const iot_data_map_iter_t * iter)
{
  assert (iter);
  return (iter->pair) ? iot_data_string (iot_data_map_iter_value (iter)) : NULL;
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
  return (iter->index <= iter->array->length) ? ((char*) (iter->array->data) + (iter->index - 1) * iot_data_type_size[iter->array->type]) : NULL;
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
    strcat (holder->str, add);
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
  const uint8_t * data = iot_data_address (array);
  assert (strlen (holder->str) == (holder->size - holder->free - 1));
  size_t len = iot_b64_encodesize (inLen) - 1; /* Allow for string terminator */
  char * out;

  if (holder->free < len)
  {
    iot_data_holder_realloc (holder, len);
  }

  out = holder->str + holder->size - holder->free - 1;
  iot_b64_encode (data, inLen, out, holder->free + 1);
  holder->free -= len;
  assert (strlen (holder->str) == (holder->size - holder->free - 1));
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
      iot_data_map_iter_t iter;
      iot_data_map_iter (data, &iter);
      iot_data_strcat (holder, "{");
      while (iot_data_map_iter_next (&iter))
      {
        const iot_data_t * key = iot_data_map_iter_key (&iter);
        const iot_data_t * value = iot_data_map_iter_value (&iter);
        if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
        iot_data_dump (holder, key);
        if (iot_data_type (key) != IOT_DATA_STRING) iot_data_add_quote (holder);
        iot_data_strcat (holder, ":");
        iot_data_dump (holder, value);
        if (iter.pair->base.next)
        {
          iot_data_strcat (holder, ",");
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
  memcpy (str, json + token->start, len);
  str[len] = 0;
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
    case 'n': ret = iot_data_alloc_string ("null", IOT_DATA_REF); break; // null
    default: // Handle all floating point numbers as doubles and integers as uint64_t
      ret = (strchr (str, '.') || strchr (str, 'e') || strchr (str, 'E')) ?
        iot_data_alloc_f64 (strtod (str, NULL)) : iot_data_alloc_i64 (strtol (str, NULL, 0));
      break;
  }
  free (str);
  return ret;
}

static iot_data_t * iot_data_map_from_json (iot_json_tok_t ** tokens, const char * json)
{
  uint32_t elements = (*tokens)->size;
  iot_data_t * map = iot_data_alloc_map (IOT_DATA_STRING);

  (*tokens)++;
  while  (elements--)
  {
    iot_data_t * key = iot_data_string_from_json (tokens, json);
    iot_data_map_add (map, key, iot_data_all_from_json (tokens, json));
  }
  return map;
}

static iot_data_t * iot_data_vector_from_json (iot_json_tok_t ** tokens, const char * json)
{
  uint32_t elements = (*tokens)->size;
  uint32_t index = 0;
  iot_data_t * vector = iot_data_alloc_vector (elements);

  (*tokens)++;
  while (elements--)
  {
    iot_data_vector_add (vector, index++, iot_data_all_from_json (tokens, json));
  }
  return vector;
}

static iot_data_t * iot_data_all_from_json (iot_json_tok_t ** tokens, const char * json)
{
  iot_data_t * data = NULL;
  switch ((*tokens)->type)
  {
    case IOT_JSON_PRIMITIVE: data = iot_data_primitive_from_json (tokens, json); break;
    case IOT_JSON_OBJECT: data = iot_data_map_from_json (tokens, json); break;
    case IOT_JSON_ARRAY: data = iot_data_vector_from_json (tokens, json); break;
    default: data = iot_data_string_from_json (tokens, json); break;
  }
  return data;
}

iot_data_t * iot_data_from_json (const char * json)
{
  iot_json_parser parser;
  iot_data_t * data = NULL;
  int32_t used;
  const char * ptr = json;
  uint32_t count = 1;

  // Approximate token count
  while (*ptr != '\0')
  {
    switch (*ptr)
    {
      case ',': case '{': count++; break;
      case ':': case '[': count += 2; break;
      default: break;
    }
    ptr++;
  }
  iot_json_tok_t * tokens = calloc (1, sizeof (*tokens) * count);
  iot_json_tok_t * tptr = tokens;

  iot_json_init (&parser);
  used = iot_json_parse (&parser, json, strlen (json), tptr, count);
  if (used)
  {
    assert (used <= count);
    data = iot_data_all_from_json (&tptr, json);
  }
  free (tokens);
  return data;
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
  assert (src);
  iot_data_t * data = (iot_data_t*) src;
  iot_data_t * ret;

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
      ret = iot_data_alloc_array (array->data, array->length, array->type, array->base.release ? IOT_DATA_COPY : IOT_DATA_REF);
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
    default: //basic types
    {
      iot_data_value_t * val = iot_data_value_alloc (data->type, false);
      val->value.ui64 = (((iot_data_value_t*) data)->value.ui64);
      ret = (iot_data_t*) val;
    }
  }
  iot_data_set_metadata (ret, data->metadata);
  return ret;
}

static iot_typecode_t iot_basic_tcs [12] =
{
  { .type = IOT_DATA_INT8 }, { .type = IOT_DATA_UINT8 }, { .type = IOT_DATA_INT16 }, { .type = IOT_DATA_UINT16 },
  { .type = IOT_DATA_INT32 }, { .type = IOT_DATA_UINT32 }, { .type = IOT_DATA_INT64 }, { .type = IOT_DATA_UINT64 },
  { .type = IOT_DATA_FLOAT32 }, { .type = IOT_DATA_FLOAT64, }, { .type = IOT_DATA_BOOL }, { .type = IOT_DATA_STRING }
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

extern void iot_typecode_free (iot_typecode_t * typecode)
{
  if (typecode && (typecode->type > IOT_DATA_ARRAY))
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
  assert (typecode && (typecode->type <= IOT_DATA_VECTOR));
  return iot_data_type_names[typecode->type];
}

extern iot_data_type_t iot_typecode_key_type (const iot_typecode_t * typecode)
{
  assert (typecode && (typecode->type == IOT_DATA_MAP));
  return typecode->key_type;
}

extern const iot_typecode_t * iot_typecode_element_type (const iot_typecode_t * typecode)
{
  assert (typecode && (typecode->type >= IOT_DATA_ARRAY));
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
    tc = iot_typecode_alloc_array (((iot_data_array_t *) data)->type);
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
      tc->key_type = ((iot_data_map_t*) data)->key_type;
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
