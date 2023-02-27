#pragma once
#include <functional>
#include <stdint.h>

class PowerWatcher {
    private:
        std::function<void(uint8_t)> power_change_cb_;
        uint8_t watch_pin_;
        static void IRAM_ATTR on_gpio_interrupt_();

    public:
        PowerWatcher(uint8_t watch_pin, std::function<void(uint8_t)> power_change_callback);
        ~PowerWatcher();
        void watch();
        void stop();
        uint8_t get_status();

};