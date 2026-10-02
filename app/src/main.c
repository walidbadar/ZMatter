/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "light.h"
#include "matter/matter.h"
#include "wifi.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	int err = light_init();

	if (err != 0) {
		LOG_ERR("Light init failed: %d", err);
		return err;
	}

	err = matter_init();
	if (err != 0) {
		return err;
	}

#ifdef CONFIG_WIFI_CREDENTIALS_CONNECT_STORED
	err = wifi_init();
	if (err != 0) {
		return err;
	}
#endif

	LOG_INF("ZMatter light bulb ready (endpoint %u)", LIGHT_ENDPOINT_ID);

	return 0;
}
