#include <Arduino.h>
#include <Ticker.h>
#include <user_interface.h>
#include "defines.hpp"
#include "power_watcher.hpp"
#include "serial_worker.hpp"
#include "CircularBuffer.h"

PowerWatcher* power_watcher;
SerialWorker* serial_worker;
Ticker ticker;
CircularBuffer<SerialCommand*, 10> command_queue;
SerialCommand* current_command = NULL;
unsigned long prev_time = 0;
bool should_report_state;

void add_deep_sleep_cmd() {
    SerialCommand* sleed_cmd = new SerialCommand(SerialCommandType::DEEP_SLEEP, DEEP_SLEEP_TIMEOUT);
    command_queue.push(sleed_cmd);
}

void add_report_state_cmd(uint8_t state) {
    SerialCommand* cmd = new SerialCommand(SerialCommandType::REPORT_STATE, state);
    command_queue.push(cmd);
}

void on_change_power(uint8_t state) {
    add_report_state_cmd(state);
}

void on_serial_command(SerialCommand* cmd) {
    command_queue.push(cmd);
}

void toggle_relay(uint8_t state) {
    digitalWrite(RELAY_PIN, state);
}

void on_ticker_tick(void) {
    uint8_t power_state = power_watcher->get_status();
    add_report_state_cmd(power_state);
}

void wait_for_serial() {
    ulong const serial_begin_time = millis();
    while (!Serial && (millis() - serial_begin_time > 3000)) {
        ;
    }
}

void setup() {
    pinMode(WATCH_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    digitalWrite(LED_PIN, 1);
    
    power_watcher = new PowerWatcher(WATCH_PIN, on_change_power);
    serial_worker = new SerialWorker(SERIAL_BAUD_RATE, on_serial_command);

    wait_for_serial();

    should_report_state = true;
    uint8_t current_power_state = power_watcher->get_status();
    if (current_power_state) {
        toggle_relay(RELAY_ON);
    } else {
        should_report_state = false;
        add_deep_sleep_cmd();
    }

    if (should_report_state) {
        ticker.attach(5, on_ticker_tick);
    }

    power_watcher->watch();
}

bool handle_relay_command(uint8_t relay_state) {
    unsigned long time = millis();
    unsigned long elapsed = time - prev_time;
    prev_time = time;
    current_command->command_arg -= elapsed;
    if (current_command->command_arg <= 0) {
        toggle_relay(relay_state);
        delete current_command;
        return true;
    }
    return false;
}

void handle_command() {
    if (current_command == NULL && !command_queue.isEmpty()) {
        current_command = command_queue.pop();
    }

    if (current_command != NULL) {
        switch(current_command->command_type) {
            case SerialCommandType::POWER_OFF: {
                if (handle_relay_command(RELAY_OFF)) {
                    add_deep_sleep_cmd();
                }
                break;
            }
            case SerialCommandType::POWER_ON: {
                handle_relay_command(RELAY_ON);
                break;
            }
            case SerialCommandType::REPORT_STATE: {
                serial_worker->report_state(current_command->command_arg);
                delete current_command;
                break;
            }
            case SerialCommandType::DEEP_SLEEP: {
                digitalWrite(LED_PIN, 0);
                power_watcher->stop();
                ESP.deepSleep(current_command->command_arg, RF_DISABLED);
                delete current_command;
                break;
            }
            default:
                break;
        }
    } else {
        prev_time = millis();
        delay(300);
    }
}

void loop() {
    serial_worker->loop();
    handle_command();
}