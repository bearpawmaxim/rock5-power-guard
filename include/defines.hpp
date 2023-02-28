#pragma once

#define RELAY_PIN           D1
#define RELAY_ON            1
#define RELAY_OFF           0

#define WATCH_PIN           D7

#define LED_PIN             D4

#define SERIAL_BAUD_RATE    57600
#define SERIAL_BUFF_SIZE    32

#define DEEP_SLEEP_TIMEOUT  60000000

enum SerialCommandType: uint8_t {
    NONE = 0,
    POWER_OFF = 1,
    POWER_ON = 2,
    REPORT_STATE = 3,
    DEEP_SLEEP = 4
};

class SerialCommand {
    public:
        SerialCommand() {}
        SerialCommand(uint8_t commandType, ulong commandArg) { this->command_type = commandType; this->command_arg = commandArg; }
        uint8_t command_type;
        ulong command_arg;
};
