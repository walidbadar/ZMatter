/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>

/*
 * Register the Network Commissioning cluster on the root endpoint: Wi-Fi,
 * whose credentials are provisioned during commissioning (e.g. over
 * Bluetooth LE), or the host network on native_sim. Call after the Matter
 * server is initialized.
 */
CHIP_ERROR network_init(void);
