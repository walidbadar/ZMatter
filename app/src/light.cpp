/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "light.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/generic-callbacks.h>

#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app::Clusters;

/* The bulb: led0 (on native_sim a host LED, see boards/native_sim_native_64.overlay). */
static const struct led_dt_spec sLed = LED_DT_SPEC_GET(DT_ALIAS(led0));

static bool sLedReady;

int LightInit(void)
{
	if (!led_is_ready_dt(&sLed)) {
		/* On native_sim this usually means no write access to the host LED. */
		LOG_WRN("LED %s is not ready, the light state is only logged", sLed.dev->name);
		return 0;
	}

	sLedReady = true;

	return led_off_dt(&sLed);
}

static void LightSet(bool on)
{
	if (sLedReady) {
		int err = on ? led_on_dt(&sLed) : led_off_dt(&sLed);

		if (err) {
			LOG_ERR("Failed to set LED: %d", err);
		}
	}

	LOG_INF("Light is %s", on ? "ON" : "OFF");
}

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath &path, uint8_t type,
				       uint16_t size, uint8_t *value)
{
	if (path.mEndpointId == kLightEndpointId && path.mClusterId == OnOff::Id &&
	    path.mAttributeId == OnOff::Attributes::OnOff::Id) {
		LightSet(*value != 0);
	}
}

/* Restore the light from the persisted OnOff attribute at boot. */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
	bool on = false;

	if (endpoint != kLightEndpointId) {
		return;
	}

	if (OnOff::Attributes::OnOff::Get(endpoint, &on) == Protocols::InteractionModel::Status::Success) {
		LightSet(on);
	}
}
