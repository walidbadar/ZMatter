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

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app::Clusters;

static bool sLedReady;

#if DT_NODE_EXISTS(DT_ALIAS(bulb_strip))

#include <zephyr/drivers/led_strip.h>

/*
 * The bulb: an RGB LED strip (alias bulb-strip), lit white when on, e.g.
 * the WS2812 LED of boards/uedx32480035e_wb_a_esp32s3_procpu.overlay.
 */
static const struct device *const sStrip = DEVICE_DT_GET(DT_ALIAS(bulb_strip));
static struct led_rgb sPixels[DT_PROP(DT_ALIAS(bulb_strip), chain_length)];

/* Full brightness is uncomfortably bright on a bare WS2812. */
#define BULB_LEVEL 0x40

static int LedWrite(bool on)
{
	for (struct led_rgb &pixel : sPixels) {
		pixel.r = pixel.g = pixel.b = on ? BULB_LEVEL : 0;
	}

	return led_strip_update_rgb(sStrip, sPixels, ARRAY_SIZE(sPixels));
}

static const char *LedName(void)
{
	return sStrip->name;
}

static bool LedIsReady(void)
{
	return device_is_ready(sStrip);
}

#else

#include <zephyr/drivers/led.h>

/* The bulb: led0 (on native_sim a host LED, see boards/native_sim_native_64.overlay). */
static const struct led_dt_spec sLed = LED_DT_SPEC_GET(DT_ALIAS(led0));

static int LedWrite(bool on)
{
	return on ? led_on_dt(&sLed) : led_off_dt(&sLed);
}

static const char *LedName(void)
{
	return sLed.dev->name;
}

static bool LedIsReady(void)
{
	return led_is_ready_dt(&sLed);
}

#endif

int LightInit(void)
{
	if (!LedIsReady()) {
		/* On native_sim this usually means no write access to the host LED. */
		LOG_WRN("LED %s is not ready, the light state is only logged", LedName());
		return 0;
	}

	sLedReady = true;

	return LedWrite(false);
}

static void LightSet(bool on)
{
	if (sLedReady) {
		int err = LedWrite(on);

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
