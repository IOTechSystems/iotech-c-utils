#include "iot/data.h"

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
  iot_data_type_t type;
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
  iot_data_t * key;
  iot_data_t * value;
  struct iot_data_pair_t * next;
} iot_data_pair_t ;

typedef struct iot_data_map_t
{
  iot_data_t base;
  iot_data_type_t key_type;
  iot_data_pair_t * pairs;
} iot_data_map_t;

static inline iot_data_value_t * iot_data_value_alloc (iot_data_type_t type)
{
  iot_data_value_t * val = calloc (1, sizeof (*val));
  val->base.type = type;
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

const char * iot_data_type_name (const iot_data_t * data)
{
  static const char * type_names [] = {"Int8","UInt8","Int16","UInt16","Int32","UInt32","Int64","UInt64","Float62","Float64","Bool","String","BLOB","Map","Array"};
  assert (data);
  assert (data->type <= IOT_DATA_ARRAY);
  return type_names[data->type];
}

iot_data_t * iot_data_map_alloc (iot_data_type_t key_type)
{
  assert (key_type < IOT_DATA_MAP);
  iot_data_map_t * map = calloc (1, sizeof (*map));
  map->base.type = IOT_DATA_MAP;
  map->key_type = key_type;
  return (iot_data_t*) map;
}

iot_data_t * iot_data_array_alloc (const uint32_t size)
{
  assert (size);
  iot_data_array_t * array = calloc (1, sizeof (iot_data_array_t));
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

void iot_data_free (iot_data_t * data)
{
  if (data)
  {
    switch (data->type)
    {
      case IOT_DATA_STRING: free (((iot_data_value_t*) data)->value.str); break;
      case IOT_DATA_BLOB: free (((iot_data_blob_t*) data)->data); break;
      case IOT_DATA_MAP:
      {
        iot_data_map_t * map = (iot_data_map_t*) data;
        iot_data_pair_t * pair;
        while (map->pairs)
        {
          pair = map->pairs;
          iot_data_free ((iot_data_t*) pair->key);
          iot_data_free (pair->value);
          map->pairs = pair->next;
          free (pair);
        }
        break;
      }
      case IOT_DATA_ARRAY:
      {
        iot_data_array_t * array = (iot_data_array_t*) data;
        for (uint32_t i = 0; i < array->size; i++)
        {
          iot_data_free (array->values[i]);
        }
        free (array->values);
        break;
      }
      default: break;
    }
    free (data);
  }
}

iot_data_t * iot_data_alloc_i8 (int8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT8);
  data->value.i8 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui8 (uint8_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT8);
  data->value.ui8 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i16 (int16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT16);
  data->value.i16 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui16 (uint16_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT16);
  data->value.ui16 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i32 (int32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT32);
  data->value.i32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui32 (uint32_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT32);
  data->value.ui32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_i64 (int64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_INT64);
  data->value.i64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_ui64 (uint64_t val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_UINT64);
  data->value.ui64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f32 (float val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT32);
  data->value.f32 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_f64 (double val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_FLOAT64);
  data->value.f64 = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_bool (bool val)
{
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_BOOL);
  data->value.bl = val;
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_string (const char * val)
{
  assert (val);
  iot_data_value_t * data = iot_data_value_alloc (IOT_DATA_STRING);
  data->value.str = strdup (val);
  return (iot_data_t*) data;
}

iot_data_t * iot_data_alloc_blob (uint8_t * data, uint32_t size)
{
  assert (data);
  assert (size);
  iot_data_blob_t * blob = malloc (sizeof (*blob));
  blob->base.type = IOT_DATA_BLOB;
  blob->size = size;
  blob->data = data;
  return (iot_data_t*) blob;
}

int8_t iot_data_get_i8 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT8));
  return ((iot_data_value_t*) data)->value.i8;
}

uint8_t iot_data_get_ui8 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT8));
  return ((iot_data_value_t*) data)->value.ui8;
}

int16_t iot_data_get_i16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT16));
  return ((iot_data_value_t*) data)->value.i16;
}

uint16_t iot_data_get_ui16 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT16));
  return ((iot_data_value_t*) data)->value.ui16;
}

int32_t iot_data_get_i32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT32));
  return ((iot_data_value_t*) data)->value.i32;
}

uint32_t iot_data_get_ui32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT32));
  return ((iot_data_value_t*) data)->value.ui32;
}

int64_t iot_data_get_i64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_INT64));
  return ((iot_data_value_t*) data)->value.i64;
}

uint64_t iot_data_get_ui64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_UINT64));
  return ((iot_data_value_t*) data)->value.ui64;
}

float iot_data_get_f32 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT32));
  return ((iot_data_value_t*) data)->value.f32;
}

double iot_data_get_f64 (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_FLOAT64));
  return ((iot_data_value_t*) data)->value.f64;
}

bool iot_data_get_bool (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_BOOL));
  return ((iot_data_value_t*) data)->value.bl;
}

const char * iot_data_get_string (const iot_data_t * data)
{
  assert (data && (data->type == IOT_DATA_STRING));
  return ((iot_data_value_t*) data)->value.str;
}

const uint8_t * iot_data_get_blob (const iot_data_t * data, uint32_t * size)
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
    pair = pair->next;
  }
  return pair;
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
    iot_data_free ((iot_data_t*) pair->key);
  }
  else
  {
    pair = (iot_data_pair_t*) calloc (1, sizeof (iot_data_pair_t));
    pair->next = mp->pairs;
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
  return (iot_data_t*) (pair ? &pair->value : NULL);
}

void iot_data_array_add (iot_data_t * array, const uint32_t index, iot_data_t * val)
{
  iot_data_array_t * arr = (iot_data_array_t*) array;
  assert (array && (array->type == IOT_DATA_ARRAY));
  assert (val);
  assert (index < arr->size);
  iot_data_t * element = arr->values[index];
  iot_data_free (element);
  arr->values[index] = val;
}

const iot_data_t * iot_data_array_get (const iot_data_t * array, const uint32_t index)
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
  iter->pair = iter->pair ? iter->pair->next : iter->map->pairs;
  return (iter->pair != NULL);
}

const iot_data_t * iot_data_map_iter_get_key (const iot_data_map_iter_t * iter)
{
  return (iot_data_t*)((iter->pair) ? iter->pair->key : NULL);
}

const iot_data_t * iot_data_map_iter_get_value (const iot_data_map_iter_t * iter)
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

uint32_t iot_data_array_iter_get_index (const iot_data_array_iter_t * iter)
{
  return (iter->index - 1);
}

const iot_data_t * iot_data_array_iter_get_value (const iot_data_array_iter_t * iter)
{
  return (iter->index <= iter->array->size) ? iter->array->values[iter->index - 1] : NULL;
}