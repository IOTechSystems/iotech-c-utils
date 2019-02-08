//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#include <pthread.h>
#include "iot/data.h"

#define IOT_DATA_MAX_SIZE 64

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
  uint32_t refs;
  iot_data_type_t type;
  bool release;
};

typedef struct iot_data_value_t
{
  iot_data_t base;
  iot_data_union_t value;
} iot_data_value_t;

typedef struct iot_data_blob_t
{
  iot_data_t base;
  uint8_t * data;
  uint32_t size;
} iot_data_blob_t;

typedef struct iot_data_array_t
{
  iot_data_t base;
  uint32_t size;
  iot_data_t ** values;
} iot_data_array_t;

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
  iot_data_pair_t * pairs;
} iot_data_map_t;

// Data cache and guard mutex

static iot_data_t * iot_data_cache = NULL;
static pthread_mutex_t iot_data_mutex = PTHREAD_MUTEX_INITIALIZER;

static void * iot_data_factory_alloc (size_t size)
{
  assert (size <= IOT_DATA_MAX_SIZE);
  pthread_mutex_lock (&iot_data_mutex);
  iot_data_t * data = iot_data_cache;
  if (data)
  {
    iot_data_cache = data->next;
    memset (data, 0, size);
  }
  pthread_mutex_unlock (&iot_data_mutex);
  data = data ? data : calloc (1, IOT_DATA_MAX_SIZE);
  data->refs = 1;
  return data;
}

static inline void iot_data_factory_free_locked (iot_data_t * data)
{
  data->next = iot_data_cache;
  iot_data_cache = data;
}

static inline iot_data_value_t * iot_data_value_alloc (iot_data_type_t type, bool copy)
{
  iot_data_value_t * val = iot_data_factory_alloc (sizeof (*val));
  val->base.type = type;
  val->base.release = copy;
  return val;
}

static bool iot_data_equal (const iot_data_t * v1, const iot_data_t * v2)
{
  assert (v1 && v2);
  if (v1->type == v2->type)
  {
    switch (v1->type)
    {
      case IOT_DATA_STRING: return (strcmp (((iot_data_value_t*) v1)->value.str, ((iot_data_value_t*) v2)->value.str) == 0);
      case IOT_DATA_BLOB:
      {
        iot_data_blob_t * b1 = (iot_data_blob_t*) v1;
        iot_data_blob_t * b2 = (iot_data_blob_t*) v2;
        return (b1->size == b2->size && (memcmp (b1->data, b2->data, b1->size) == 0));
      }
      default: return (((iot_data_value_t*) v1)->value.ui64 == ((iot_data_value_t*) v2)->value.ui64);
    }
  }
  return false;
}

void iot_data_init (void)
{
}

void iot_data_fini (void)
{
  iot_data_t *data;
  pthread_mutex_lock (&iot_data_mutex);
  while (iot_data_cache)
  {
    data = iot_data_cache;
    iot_data_cache = data->next;
    free (data);
  }
  pthread_mutex_unlock (&iot_data_mutex);
}

void iot_data_addref (iot_data_t * data)
{
  assert (data);
  pthread_mutex_lock (&iot_data_mutex);
  data->refs++;
  pthread_mutex_unlock (&iot_data_mutex);
}

const char * iot_data_type_name (const iot_data_t * data)
{
  static const char * type_names [] = {"Int8","UInt8","Int16","UInt16","Int32","UInt32","Int64","UInt64","Float62","Float64","Bool","String","BLOB","Map","Array"};
  assert (data);
  assert (data->type <= IOT_DATA_ARRAY);
  return type_names[data->type];
}

iot_data_t * iot_data_alloc_map (iot_data_type_t key_type)
{
  assert (key_type < IOT_DATA_MAP);
  iot_data_map_t * map = iot_data_factory_alloc (sizeof (*map));
  map->base.type = IOT_DATA_MAP;
  map->key_type = key_type;
  return (iot_data_t*) map;
}

iot_data_t * iot_data_alloc_array (uint32_t size)
{
  assert (size);
  iot_data_array_t * array = iot_data_factory_alloc (sizeof (*array));
  array->base.type = IOT_DATA_ARRAY;
  array->size = size;
  array->values = calloc (size, sizeof (iot_data_t*));
  return (iot_data_t*) array;
}

iot_data_type_t iot_data_type (const iot_data_t * data)
{
  assert (data);
  return data->type;
}

static void iot_data_free_locked (iot_data_t * data)
{
  if (--data->refs <= 0)
  {
    switch (data->type)
    {
      case IOT_DATA_STRING:
        if (data->release) free (((iot_data_value_t *) data)->value.str);
        break;
      case IOT_DATA_BLOB:
      {
        iot_data_blob_t *blob = (iot_data_blob_t *) data;
        if (blob->base.release) free (blob->data);
        break;
      }
      case IOT_DATA_MAP:
      {
        iot_data_map_t *map = (iot_data_map_t *) data;
        iot_data_pair_t *pair;
        while (map->pairs)
        {
          pair = map->pairs;
          iot_data_free_locked (pair->key);
          iot_data_free_locked (pair->value);
          map->pairs = (iot_data_pair_t *) pair->base.next;
          iot_data_factory_free_locked (&pair->base);
        }
        break;
      }
      case IOT_DATA_ARRAY:
      {
        iot_data_array_t *array = (iot_data_array_t *) data;
        for (uint32_t i = 0; i < array->size; i++)
        {
          iot_data_free_locked (array->values[i]);
        }
        free (array->values);
        break;
      }
      default: break;
    }
    iot_data_factory_free_locked (data);
  }
}

void iot_data_free (iot_data_t * data)
{
  if (data)
  {
    pthread_mutex_lock (&iot_data_mutex);
    iot_data_free_locked (data);
    pthread_mutex_unlock (&iot_data_mutex);
  }
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

iot_data_t * iot_data_alloc_string (const char * val, bool copy)
{
  assert (val);
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_STRING, copy);
  data->value.str = copy ? strdup (val) : (char*) val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_blob (uint8_t * data, uint32_t size, bool copy)
{
  assert (data);
  assert (size);
  iot_data_blob_t * blob = iot_data_factory_alloc (sizeof (*blob));
  blob->base.type = IOT_DATA_BLOB;
  blob->size = size;
  blob->base.release = copy;
  if (copy)
  {
    blob->data = malloc (size);
    memcpy (blob->data, data, size);
  }
  else
  {
    blob->data = data;
  }
  return (iot_data_t*) blob;
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

const uint8_t * iot_data_blob (const iot_data_t * data, uint32_t * size)
{
  assert (data && (data->type == IOT_DATA_BLOB));
  if (size) *size =  ((iot_data_blob_t*) data)->size;
  return ((iot_data_blob_t*) data)->data;
}

static iot_data_pair_t * iot_data_map_find (iot_data_map_t * map, const iot_data_t * key)
{
  iot_data_pair_t * pair =  map->pairs;
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
  iot_data_map_add (map, iot_data_alloc_string (key, false), val);
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
    pair = (iot_data_pair_t*) iot_data_factory_alloc (sizeof (*pair));
    pair->base.next = &mp->pairs->base;
    mp->pairs = pair;
  }
  pair->value = val;
  pair->key = key;
}

const iot_data_t * iot_data_map_get (const iot_data_t * map, const iot_data_t * key)
{
  iot_data_map_t * mp = (iot_data_map_t*) map;

  assert (mp && (mp->base.type == IOT_DATA_MAP));
  assert (key && key->type == mp->key_type);

  iot_data_pair_t * pair = iot_data_map_find (mp, key);
  return pair ? pair->value : NULL;
}

extern const iot_data_t * iot_data_string_map_get (const iot_data_t * map, const char * key)
{
  assert (key);
  iot_data_t * dkey = iot_data_alloc_string (key, false);
  const iot_data_t * value = iot_data_map_get (map, dkey);
  iot_data_free (dkey);
  return value;
}

iot_data_type_t iot_data_map_key_type (const iot_data_t * map)
{
  assert (map);
  return ((iot_data_map_t*) map)->key_type;
}

void iot_data_array_add (iot_data_t * array, uint32_t index, iot_data_t * val)
{
  iot_data_array_t * arr = (iot_data_array_t*) array;
  assert (array && (array->type == IOT_DATA_ARRAY));
  assert (val);
  assert (index < arr->size);
  iot_data_t * element = arr->values[index];
  iot_data_free (element);
  arr->values[index] = val;
}

const iot_data_t * iot_data_array_get (const iot_data_t * array, uint32_t index)
{
  iot_data_array_t * arr = (iot_data_array_t*) array;
  assert (array && (array->type == IOT_DATA_ARRAY));
  assert (index < arr->size);
  return arr->values[index];
}

uint32_t iot_data_array_size (const iot_data_t * array)
{
  assert (array && (array->type == IOT_DATA_ARRAY));
  return ((iot_data_array_t*) array)->size;
}

void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter)
{
  assert (iter);
  assert (map && map->type == IOT_DATA_MAP);
  iter->pair = NULL;
  iter->map = (iot_data_map_t*) map;
}

bool iot_data_map_iter_next (iot_data_map_iter_t * iter)
{
  iter->pair = iter->pair ? (iot_data_pair_t*) iter->pair->base.next : iter->map->pairs;
  return (iter->pair != NULL);
}

const iot_data_t * iot_data_map_iter_key (const iot_data_map_iter_t * iter)
{
  return (iot_data_t*)((iter->pair) ? iter->pair->key : NULL);
}

const iot_data_t * iot_data_map_iter_value (const iot_data_map_iter_t * iter)
{
  return (iot_data_t*)((iter->pair) ? iter->pair->value : NULL);
}

void iot_data_array_iter (const iot_data_t * array, iot_data_array_iter_t * iter)
{
  assert (iter);
  assert (array && array->type == IOT_DATA_ARRAY);
  iter->array = (iot_data_array_t*) array;
  iter->index = 0;
}

bool iot_data_array_iter_next (iot_data_array_iter_t * iter)
{
  iter->index++;
  return (iter->index <= iter->array->size);
}

uint32_t iot_data_array_iter_index (const iot_data_array_iter_t * iter)
{
  return (iter->index - 1);
}

const iot_data_t * iot_data_array_iter_value (const iot_data_array_iter_t * iter)
{
  return (iter->index <= iter->array->size) ? iter->array->values[iter->index - 1] : NULL;
}