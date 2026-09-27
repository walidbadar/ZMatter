/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "light.h"
#include "wifi.h"

#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

void ChipEventHandler(const ChipDeviceEvent *event, intptr_t /* arg */)
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
		CHIP_ERROR __err = (expr);                                                         \
		if (__err != CHIP_NO_ERROR) {                                                      \
			LOG_ERR("%s failed: %" CHIP_ERROR_FORMAT, #expr, __err.Format());         \
			return __err;                                                              \
		}                                                                                  \
	} while (0)

CHIP_ERROR InitMatter()
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

	ConfigurationMgr().LogDeviceConfig();
	PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork));

	RETURN_ON_ERROR(PlatformMgr().AddEventHandler(ChipEventHandler, 0));
	RETURN_ON_ERROR(PlatformMgr().StartEventLoopTask());

	return CHIP_NO_ERROR;
}

} // namespace

int main(void)
{
	int err = LightInit();

	if (err) {
		LOG_ERR("Light init failed: %d", err);
		return err;
	}

	if (InitMatter() != CHIP_NO_ERROR) {
		return -1;
	}

#ifdef CONFIG_WIFI_CREDENTIALS_CONNECT_STORED
	if (WifiInit() != 0) {
		return -1;
	}
#endif

	LOG_INF("ZMatter light bulb ready (endpoint %u)", kLightEndpointId);

	return 0;
}
