#pragma once
#include "globals.h"
#include <atomic>
#include <initializer_list>

namespace core {

    enum class spam_type : uint8_t {
        microsoft = 0,
        apple,
        samsung,
        google,
        flipper_zero,
        all_sequential,
        random
    };

    class ble_ctrl {
    private:
        ble_ctrl();

    public:
        ble_ctrl(const ble_ctrl&) = delete;
        ble_ctrl& operator=(const ble_ctrl&) = delete;
        ble_ctrl(ble_ctrl&&) = delete;
        ble_ctrl& operator=(ble_ctrl&&) = delete;
        ~ble_ctrl();

        [[nodiscard]] static ble_ctrl& instance();

        void deinit();

        void spam_start(spam_type type);
        void spam_start(std::initializer_list<spam_type> types);
        void spam_stop();

        [[nodiscard]] bool get_init_status() const;
        [[nodiscard]] bool get_ble_ready() const;
        [[nodiscard]] bool get_spam_status() const;
    };
}