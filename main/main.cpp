
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "globals.h"
#include "lib/core/ble_manager/ble_manager.h"
#include "lib/core/wifi_manager/wifi_manager.h"

extern "C" void app_main(void)
{
    //auto& wifi = core::wifi_ctrl::instance();
    //wifi.scan();
    //wifi.attack_start(core::wifi_attack_type::deauth);
    auto& ble = core::ble_ctrl::instance();

    ble.spam_start({core::spam_type::samsung});
    
}
