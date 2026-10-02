/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 *
 * Matter project configuration overrides, applied to both the application
 * and the Matter libraries.
 */

#pragma once

/*
 * On native_sim (CONFIG_ARCH_POSIX) Matter talks to the network through the
 * host's BSD sockets, so enumerate the host's interfaces as well instead of
 * the Zephyr ones and join multicast groups with regular socket options.
 */
#ifdef CONFIG_ARCH_POSIX
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF          0
#define CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS            1
#define CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API 0
#define IPV6_MULTICAST_IMPLEMENTED

/* Minimal mDNS opens an endpoint per host interface (docker, VPN, ...). */
#define INET_CONFIG_NUM_UDP_ENDPOINTS 32
#endif

/*
 * The ESP32 has much less internal RAM than the ESP32-S3: smaller Matter
 * pools so that Matter, Wi-Fi and Bluetooth LE fit (see
 * socs/esp32_procpu.conf). Three fabrics is below the five that Matter
 * certification requires, which is fine for development.
 */
#ifdef CONFIG_SOC_SERIES_ESP32
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 6
#define CHIP_CONFIG_MAX_FABRICS                   3
#define CHIP_CONFIG_SECURE_SESSION_POOL_SIZE      8
#define CHIP_CONFIG_MAX_GROUP_DATA_PEERS          4

/*
 * Set along with the default CHIP_CONFIG_MAX_FABRICS in
 * platform/Zephyr/CHIPPlatformConfig.h, so skipped once it is overridden.
 */
#define INET_CONFIG_UDP_SOCKET_MREQN                    1
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 0
#endif
