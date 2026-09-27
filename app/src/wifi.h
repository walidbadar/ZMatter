/*
 * Copyright (c) 2026 Muhammad Waleed Badar
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/*
 * Join the Wi-Fi network from the stored credentials, and reconnect from a
 * work item whenever the link drops or a connect attempt fails.
 */
int WifiInit(void);
