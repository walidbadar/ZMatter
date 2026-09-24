/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/DataModelTypes.h>

/* Endpoint hosting the On/Off Light device type (see light.zap). */
inline constexpr chip::EndpointId kLightEndpointId = 1;

/* Configure the LED (devicetree alias led0) that represents the bulb. */
int LightInit(void);
