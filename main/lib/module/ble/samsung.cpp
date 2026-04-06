#include "samsung.h"
#include "esp_random.h"
#include <cstdint>
#include <iterator>

namespace {

constexpr uint32_t buds_models[] = {
    0xEE7A0C, 0x9D1700, 0x39EA48, 0xA7C62C, 0x850116,
    0x3D8F41, 0x3B6D02, 0xAE063C, 0xB8B905, 0xEAAA17,
    0xD30704, 0x9DB006, 0x101F1A, 0x859608, 0x8E4503,
    0x2C6740, 0x3F6718, 0x42C519, 0xAE073A, 0x011716,
};

constexpr uint8_t watch_models[] = {
    0x1A, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0xE4, 0xE5, 0x1B, 0x1C,
    0x1D, 0x1E, 0x20, 0xEC, 0xEF,
};

size_t build_buds(uint8_t* buf) {
    uint32_t model = buds_models[esp_random() % std::size(buds_models)];

    uint8_t i = 0;
    buf[i++] = 27;   buf[i++] = 0xFF;  buf[i++] = 0x75;  buf[i++] = 0x00;
    buf[i++] = 0x42; buf[i++] = 0x09;  buf[i++] = 0x81;  buf[i++] = 0x02;
    buf[i++] = 0x14; buf[i++] = 0x15;  buf[i++] = 0x03;  buf[i++] = 0x21;
    buf[i++] = 0x01; buf[i++] = 0x09;
    buf[i++] = (model >> 16) & 0xFF;
    buf[i++] = (model >>  8) & 0xFF;
    buf[i++] = 0x01;
    buf[i++] = model & 0xFF;
    buf[i++] = 0x06; buf[i++] = 0x3C;  buf[i++] = 0x94;  buf[i++] = 0x8E;
    buf[i++] = 0x00; buf[i++] = 0x00;  buf[i++] = 0x00;  buf[i++] = 0x00;
    buf[i++] = 0xC7; buf[i++] = 0x00;

    buf[i++] = 16;   buf[i++] = 0xFF;  buf[i++] = 0x75;

    return i;
}

size_t build_watch(uint8_t* buf) {
    uint8_t model = watch_models[esp_random() % std::size(watch_models)];

    uint8_t i = 0;
    buf[i++] = 14;   buf[i++] = 0xFF;  buf[i++] = 0x75;  buf[i++] = 0x00;
    buf[i++] = 0x01; buf[i++] = 0x00;  buf[i++] = 0x02;  buf[i++] = 0x00;
    buf[i++] = 0x01; buf[i++] = 0x01;  buf[i++] = 0xFF;  buf[i++] = 0x00;
    buf[i++] = 0x00; buf[i++] = 0x43;  buf[i++] = model;

    return i;
}

}

size_t module::samsung::build_adv(uint8_t* buf) {
    return (esp_random() % 2) ? build_buds(buf) : build_watch(buf);
}