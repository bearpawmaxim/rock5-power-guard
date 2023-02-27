#include <Arduino.h>
#include "power_watcher.hpp"

static PowerWatcher* instance_;

PowerWatcher::PowerWatcher(uint8_t watch_pin, std::function<void(uint8_t)> power_change_callback) {
    watch_pin_ = watch_pin;
    power_change_cb_ = power_change_callback;
    instance_ = this;
}

PowerWatcher::~PowerWatcher() {
    stop();
    instance_ = NULL;
}

void IRAM_ATTR PowerWatcher::on_gpio_interrupt_() {
    int status = instance_->get_status();
    instance_->power_change_cb_(status);
}

void PowerWatcher::watch() {
    attachInterrupt(digitalPinToInterrupt(watch_pin_), on_gpio_interrupt_, CHANGE);
}

void PowerWatcher::stop() {
    detachInterrupt(digitalPinToInterrupt(watch_pin_));
}

uint8_t PowerWatcher::get_status() {
    return !digitalRead(watch_pin_);
}
