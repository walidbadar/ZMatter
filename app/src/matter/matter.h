/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C interface to the Matter stack, implemented in C++ (matter.cpp).
 */

/*
 * Initialize and start the Matter stack: the server, the Network
 * Commissioning cluster and the event loop. Prints the onboarding codes.
 * Return 0 on success, a negative errno otherwise.
 */
int matter_init(void);

#ifdef __cplusplus
}
#endif
