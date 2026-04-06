#pragma once
#include "lib/core/wifi_manager/wifi_manager.h"

namespace module {
    inline core::wifi_ctrl& wifi = core::wifi_ctrl::instance();
}
