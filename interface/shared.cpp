#include "shared.h"

const uint8_t RADIO_ADDRESS_CONTROLLER[5] = { 0x6B, 0xE3, 0x10, 0xE6, 0xCF };
const uint8_t RADIO_ADDRESS_INTERFACE[5] =  { 0x6C, 0x28, 0xA4, 0x88, 0x44 };
const uint8_t RADIO_CHANNEL = 80;
const uint8_t PACKET_SIZE = 16;

const uint8_t CHANNEL_POWER_SAFE = 255;


const char *_comms_controller_state_strings[] = {
    "Not exposing",
    "Exposing"
};

const char *_comms_command_strings[] = {
    "Report status",
    "Set exposure",
    "Start exposure",
    "Stop exposure",
    "Set channel power"
};

const char *_comms_status_strings[] = {
    "OK",
    "Invalid message",
    "Cannot set exposure while exposing",
    "Exposure already underway",
    "Not exposing",
    "No receiver",
    "Set failed",
    "Timeout"
};

