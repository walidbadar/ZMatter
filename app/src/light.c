/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "light.h"

#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static bool led_ready;

#if DT_NODE_EXISTS(DT_ALIAS(bulb_strip))

#include <zephyr/drivers/led_strip.h>

/*
 * The bulb: an RGB LED strip (alias bulb-strip), lit white when on, e.g.
 * the WS2812 LED of boards/uedx32480035e_wb_a_esp32s3_procpu.overlay.
 */
static const struct device *const strip = DEVICE_DT_GET(DT_ALIAS(bulb_strip));
static struct led_rgb pixels[DT_PROP(DT_ALIAS(bulb_strip), chain_length)];

/* Full brightness is uncomfortably bright on a bare WS2812. */
#define BULB_LEVEL 0x40U

static int led_write(bool on)
{
	for (size_t i = 0; i < ARRAY_SIZE(pixels); i++) {
		uint8_t level = on ? BULB_LEVEL : 0U;

		pixels[i].r = level;
		pixels[i].g = level;
		pixels[i].b = level;
	}

	return led_strip_update_rgb(strip, pixels, ARRAY_SIZE(pixels));
}

static const char *led_name(void)
{
	return strip->name;
}

static bool led_is_ready(void)
{
	return device_is_ready(strip);
}

#elif DT_NODE_EXISTS(DT_ALIAS(led0))

#include <zephyr/drivers/led.h>

/* The bulb: led0 (on native_sim a host LED, see boards/native_sim_native_64.overlay). */
static const struct led_dt_spec led = LED_DT_SPEC_GET(DT_ALIAS(led0));

static int led_write(bool on)
{
	return on ? led_on_dt(&led) : led_off_dt(&led);
}

static const char *led_name(void)
{
	return led.dev->name;
}

static bool led_is_ready(void)
{
	return led_is_ready_dt(&led);
}

#else

/* No bulb on this board (e.g. esp32_devkitc): the light state is only logged. */
static int led_write(bool on)
{
	ARG_UNUSED(on);

	return -ENODEV;
}

static const char *led_name(void)
{
	return "(none)";
}

static bool led_is_ready(void)
{
	return false;
}

#endif

int light_init(void)
{
	if (!led_is_ready()) {
		/* On native_sim this usually means no write access to the host LED. */
		LOG_WRN("LED %s is not ready, the light state is only logged", led_name());
		return 0;
	}

	led_ready = true;

	return led_write(false);
}

void light_set(bool on)
{
	if (led_ready) {
		int err = led_write(on);

		if (err != 0) {
			LOG_ERR("Failed to set LED: %d", err);
		}
	}

	LOG_INF("Light is %s", on ? "ON" : "OFF");
}
