#pragma once
#include <functional>
#include <stdint.h>
#include "defines.hpp"

using namespace std;

class SerialWorker {
    public:
        SerialWorker(uint16_t baud_rate, std::function<void(SerialCommand)> command_cb);
        void report_state(uint8_t state);
        void loop();

    private:
        const char* pwr_off_cmd_ = "spwrst0";
        const char* pwr_on_cmd_ = "spwrst1";
        bool new_data_ = false;
        char received_chars_[SERIAL_BUFF_SIZE];
        char temp_chars_[SERIAL_BUFF_SIZE];
        char response_chars_[SERIAL_BUFF_SIZE];
        std::function<void(SerialCommand)> command_cb_;
        void receive_data_();
        long find_delay_argument();
        SerialCommand parse_cmd_();

};