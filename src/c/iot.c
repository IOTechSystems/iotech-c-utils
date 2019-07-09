#include "iot/iot.h"

extern void iot_data_init (void);
extern void iot_data_fini (void);

extern void iot_init (void)
{
  iot_data_init ();
}

extern void iot_fini (void)
{
  iot_data_fini ();
}
