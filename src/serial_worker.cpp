#include <Arduino.h>
#include "serial_worker.hpp"

SerialWorker::SerialWorker(uint16_t baud_rate, std::function<void(SerialCommand*)> command_cb) {
    command_cb_ = command_cb;
    Serial.begin(baud_rate);
}

void SerialWorker::report_state(uint8_t state) {
    Serial.printf("rpwrst=%d\n", state);
}

void SerialWorker::receive_data_() {
    static boolean recv_in_progress = false;
    static uint8_t idx = 0;
    char start_marker = '!';
    char end_marker = '\n';
    char rc;

    while (Serial.available() > 0 && new_data_ == false) {
        rc = Serial.read();
        if (recv_in_progress == true) {
            if (rc != end_marker) {
                received_chars_[idx] = rc;
                idx ++;
                if (idx >= SERIAL_BUFF_SIZE) {
                    idx = SERIAL_BUFF_SIZE - 1;
                }
            } else {
                received_chars_[idx] = '\0';
                recv_in_progress = false;
                idx = 0;
                new_data_ = true;
            }
        } else if (rc == start_marker) {
            recv_in_progress = true;
        }
    }
}

long SerialWorker::find_delay_argument() {
    char* strtok_idx;
    strtok_idx = strtok(temp_chars_, ",");
    if (strtok_idx == NULL) {
        return -1;
    }
    strtok_idx = strtok(NULL, ",");
    if (strtok_idx == NULL) {
        return -1;
    }
    return atol(strtok_idx);
}

SerialCommand* SerialWorker::parse_cmd_() {
    SerialCommand* cmd = new SerialCommand();
    cmd->command_type = SerialCommandType::NONE;

    if (strncmp(pwr_off_cmd_, temp_chars_, 7) == 0) {

        // parse power off command
        cmd->command_type = SerialCommandType::POWER_OFF;
    } else if (strncmp(pwr_on_cmd_, temp_chars_, 7) == 0) {

        // parse power on command
        cmd->command_type = SerialCommandType::POWER_ON;
    }

    cmd->command_arg = find_delay_argument();
    return cmd;
}

void SerialWorker::loop() {
    receive_data_();
    if (new_data_) {
        strcpy(temp_chars_, received_chars_);
        SerialCommand* command = parse_cmd_();
        if (command->command_type != SerialCommandType::NONE) {
            command_cb_(command);
        }
        new_data_ = false;
    }
}