/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Endpoint hosting the On/Off Light device type (see light.zap). */
#define LIGHT_ENDPOINT_ID 1

/* Configure the LED that represents the bulb (alias bulb-strip, else led0). */
int light_init(void);

/* Turn the bulb on or off. */
void light_set(bool on);

#ifdef __cplusplus
}
#endif
