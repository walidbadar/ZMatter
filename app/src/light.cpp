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

/* The "bulb": native_sim has no lamp, so the state is reported in the log. */
static void LightSet(bool on)
{
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
