#pragma once
#include "esp_timer.h"
#include "globals.h"
#include "version.h"

inline const char* kernel_version = KERNEL_VERSION;

inline uint64_t millis() {
    return esp_timer_get_time() / 1000; // like arduino sdk
}
inline uint32_t seconds(){
    return esp_timer_get_time() / 1000000ULL;
}