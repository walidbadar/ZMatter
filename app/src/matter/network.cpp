/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network.h"

#include <app/clusters/network-commissioning/CodegenInstance.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef CONFIG_CHIP_WIFI
#include <platform/Zephyr/wifi/ZephyrWifiDriver.h>
#else
#include <inet/InetInterface.h>

#include <string.h>
#endif

using namespace chip;
using namespace chip::DeviceLayer::NetworkCommissioning;

namespace
{

/* Network Commissioning lives on the root endpoint. */
constexpr EndpointId root_endpoint_id = 0;

#ifdef CONFIG_CHIP_WIFI

ZephyrWifiDriver &get_driver()
{
	return ZephyrWifiDriver::Instance();
}

#else

/*
 * The host network on native_sim: one, always connected, network named after
 * the first host interface that is up and not a loopback.
 */
class HostEthernetDriver final: public EthernetDriver
{
      public:
	uint8_t GetMaxNetworks() override
	{
		return 1;
	}

	NetworkIterator *GetNetworks() override
	{
		iterator.Reset();
		return &iterator;
	}

      private:
	class HostNetworkIterator final: public NetworkIterator
	{
	      public:
		void Reset()
		{
			exhausted = false;
		}

		size_t Count() override
		{
			return 1;
		}

		bool Next(Network &item) override
		{
			if (exhausted) {
				return false;
			}

			exhausted = true;
			item.networkIDLen = 0;
			item.connected = true;

			for (Inet::InterfaceIterator it; it.HasCurrent(); (void)it.Next()) {
				char name[Inet::InterfaceId::kMaxIfNameLength];

				if (!it.IsUp() || it.IsLoopback() ||
				    it.GetInterfaceName(name, sizeof(name)) != CHIP_NO_ERROR) {
					continue;
				}

				item.networkIDLen =
					static_cast<uint8_t>(strnlen(name, sizeof(item.networkID)));
				(void)memcpy(item.networkID, name, item.networkIDLen);
				break;
			}

			return true;
		}

		/* Owned by the driver, nothing to free. */
		void Release() override
		{
		}

	      private:
		bool exhausted = false;
	};

	HostNetworkIterator iterator;
};

HostEthernetDriver &get_driver()
{
	static HostEthernetDriver driver;

	return driver;
}

#endif /* CONFIG_CHIP_WIFI */

} /* namespace */

CHIP_ERROR network_init(void)
{
	static app::Clusters::NetworkCommissioning::Instance instance(root_endpoint_id,
								      &get_driver());

	return instance.Init();
}
