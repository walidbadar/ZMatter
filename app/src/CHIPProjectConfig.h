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
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF 0
#define CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS 1
#define CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API 0
#define IPV6_MULTICAST_IMPLEMENTED

/* Minimal mDNS opens an endpoint per host interface (docker, VPN, ...). */
#define INET_CONFIG_NUM_UDP_ENDPOINTS 32
#endif
