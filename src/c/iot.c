/*
 * Copyright (c) 2020
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

__attribute__((constructor)) static void iot_init (void);
__attribute__((destructor)) static void iot_fini (void);

extern void iot_data_init (void);

static void iot_init (void)
{
  iot_data_init ();
}

static void iot_fini (void)
{
  // Placeholder for any required global cleanup
}
