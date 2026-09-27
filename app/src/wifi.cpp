/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#include "wifi.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

/* Delay before retrying after a disconnect or a failed connect */
#define WIFI_RECONNECT_DELAY K_SECONDS(5)

static struct k_work_delayable sReconnectWork;
static struct net_mgmt_event_callback sWifiMgmtCb;

/* Start joining the network from the stored (build-time) credentials. */
static int WifiConnect(void)
{
	struct net_if *iface = net_if_get_wifi_sta();
	int ret;

	if (iface == NULL) {
		LOG_ERR("No Wi-Fi STA interface");
		return -ENODEV;
	}

	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);
	if (ret != 0) {
		LOG_ERR("Connect stored failed: %d", ret);
		return ret;
	}

	LOG_INF("Connecting to WLAN");

	return 0;
}

static void ReconnectHandler(struct k_work *work)
{
	ARG_UNUSED(work);

	if (WifiConnect() != 0) {
		k_work_reschedule(&sReconnectWork, WIFI_RECONNECT_DELAY);
	}
}

static void WifiEventHandler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
			     struct net_if *iface)
{
	const struct wifi_status *status = (const struct wifi_status *)cb->info;

	ARG_UNUSED(iface);

	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		if (status->status != 0) {
			LOG_WRN("Wi-Fi connect failed (%d), retrying", status->status);
			k_work_reschedule(&sReconnectWork, WIFI_RECONNECT_DELAY);
		} else {
			LOG_INF("Wi-Fi connected");
			k_work_cancel_delayable(&sReconnectWork);
		}
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_WRN("Wi-Fi disconnected (reason %d), reconnecting", status->disconn_reason);
		k_work_reschedule(&sReconnectWork, WIFI_RECONNECT_DELAY);
		break;
	default:
		break;
	}
}

int WifiInit(void)
{
	k_work_init_delayable(&sReconnectWork, ReconnectHandler);

	net_mgmt_init_event_callback(&sWifiMgmtCb, WifiEventHandler,
				     NET_EVENT_WIFI_CONNECT_RESULT |
					     NET_EVENT_WIFI_DISCONNECT_RESULT);
	net_mgmt_add_event_callback(&sWifiMgmtCb);

	/* First attempt runs from the work queue, like every retry. */
	k_work_schedule(&sReconnectWork, K_NO_WAIT);

	return 0;
}
