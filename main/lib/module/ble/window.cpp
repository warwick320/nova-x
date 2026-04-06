#include "window.h"
#include "esp_random.h"
#include <cstdint>
#include <cstring>

namespace {

constexpr char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

}

size_t module::window::build_adv(uint8_t* buf) {
    uint8_t name_len = (esp_random() % 7) + 2;
    char name[9];
    for (uint8_t n = 0; n < name_len; n++)
        name[n] = charset[esp_random() % (sizeof(charset) - 1)];

    uint8_t size = 7 + name_len;
    uint8_t i = 0;

    buf[i++] = size - 1;
    buf[i++] = 0xFF;
    buf[i++] = 0x06;
    buf[i++] = 0x00;
    buf[i++] = 0x03;
    buf[i++] = 0x00;
    buf[i++] = 0x80;
    memcpy(&buf[i], name, name_len);
    i += name_len;

    return i;
}