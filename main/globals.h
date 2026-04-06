#pragma once

#include <algorithm>
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>	
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_system.h"
#include <ctype.h>

#ifdef __cplusplus
}
#endif

#include <array>
#include <string>
#include <map>
#include <vector>
#include <atomic>
#include "lib/core/framework.h"