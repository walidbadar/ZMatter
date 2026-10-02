/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 *
 * C++ wrapper around the Matter stack: everything that needs the Matter C++
 * API lives here, behind the C interface of matter.h.
 */

#include "matter.h"

#include "light.h"
#include "network.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <app/util/generic-callbacks.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

namespace
{

/* Not yet on a network: commissioned over Bluetooth LE, else on-network. */
#ifdef CONFIG_BT
constexpr RendezvousInformationFlag rendezvous_flag = RendezvousInformationFlag::kBLE;
#else
constexpr RendezvousInformationFlag rendezvous_flag = RendezvousInformationFlag::kOnNetwork;
#endif

void chip_event_handler(const ChipDeviceEvent *event, intptr_t /* arg */)
{
	switch (event->Type) {
	case DeviceEventType::kCommissioningComplete:
		LOG_INF("Commissioning complete");
		break;
	case DeviceEventType::kInterfaceIpAddressChanged:
		LOG_INF("IP address changed");
		break;
	default:
		break;
	}
}

#define RETURN_ON_ERROR(expr)                                                                      \
	do {                                                                                       \
		CHIP_ERROR err = (expr);                                                           \
		if (err != CHIP_NO_ERROR) {                                                        \
			LOG_ERR("%s failed: %" CHIP_ERROR_FORMAT, #expr, err.Format());            \
			return err;                                                                \
		}                                                                                  \
	} while (0)

CHIP_ERROR start_matter()
{
	RETURN_ON_ERROR(Platform::MemoryInit());
	RETURN_ON_ERROR(PlatformMgr().InitChipStack());

	/* Development credentials only: test DAC and test setup payload. */
	SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
	Credentials::SetDeviceAttestationCredentialsProvider(
		Credentials::Examples::GetExampleDACProvider());

	static CommonCaseDeviceServerInitParams initParams;
	RETURN_ON_ERROR(initParams.InitializeStaticResourcesBeforeServerInit());
	initParams.dataModelProvider =
		app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
	RETURN_ON_ERROR(Server::GetInstance().Init(initParams));
	RETURN_ON_ERROR(network_init());

	ConfigurationMgr().LogDeviceConfig();
	PrintOnboardingCodes(RendezvousInformationFlags(rendezvous_flag));

	RETURN_ON_ERROR(PlatformMgr().AddEventHandler(chip_event_handler, 0));
	RETURN_ON_ERROR(PlatformMgr().StartEventLoopTask());

	return CHIP_NO_ERROR;
}

} /* namespace */

extern "C" int matter_init(void)
{
	return (start_matter() == CHIP_NO_ERROR) ? 0 : -EIO;
}

/* Data model callbacks: drive the bulb from the On/Off attribute. */

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath &path, uint8_t type,
				       uint16_t size, uint8_t *value)
{
	ARG_UNUSED(type);
	ARG_UNUSED(size);

	if ((path.mEndpointId == LIGHT_ENDPOINT_ID) && (path.mClusterId == OnOff::Id) &&
	    (path.mAttributeId == OnOff::Attributes::OnOff::Id)) {
		light_set(*value != 0U);
	}
}

/* Restore the light from the persisted OnOff attribute at boot. */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
	bool on = false;

	if (endpoint != LIGHT_ENDPOINT_ID) {
		return;
	}

	if (OnOff::Attributes::OnOff::Get(endpoint, &on) ==
	    Protocols::InteractionModel::Status::Success) {
		light_set(on);
	}
}
