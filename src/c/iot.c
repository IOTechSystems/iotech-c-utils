#include "iot/iot.h"

extern void iot_data_init (void);
extern void iot_data_fini (void);

void iot_init (void)
{
  iot_data_init ();
}

void iot_fini (void)
{
  iot_data_fini ();
}
