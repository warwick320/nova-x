idf_component_register(SRCS "main.cpp"
                             "lib/core/wifi_manager/wifi_manager.cpp"
                             "lib/core/ble_manager/ble_manager.cpp"
                             "lib/module/wifi/beacon.cpp"
                             "lib/module/wifi/deauth.cpp"
                             "lib/module/ble/ios.cpp"
                             "lib/module/ble/samsung.cpp"
                             "lib/module/ble/andriod.cpp"
                             "lib/module/ble/window.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES bt esp_wifi nvs_flash)

target_link_options(${COMPONENT_LIB} INTERFACE -Wl,--allow-multiple-definition)
