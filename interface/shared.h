#ifndef _SHARED_H
#define _SHARED_H

#include <stdint.h>


enum ControllerState {
    CONTROLLER_STATE_NOT_EXPOSING                   = 0,
    CONTROLLER_STATE_EXPOSING                       = 1
};

enum InterfaceState {

};

enum CommsMessage {
    MESSAGE_OK                                      = 0,
    MESSAGE_INVALID_COMMAND                         = 1,
    MESSAGE_CANNOT_SET_EXPOSURE_WHILE_EXPOSING      = 2,
    MESSAGE_EXPOSURE_ALREADY_UNDERWAY               = 3,
    MESSAGE_NOT_EXPOSING                            = 4,
    MESSAGE_NO_RECEIVER                             = 5,
    MESSAGE_SET_FAILED                              = 6,
    MESSAGE_TIMEOUT                                 = 7
};

enum CommsCommand {
    COMMAND_REPORT_STATUS                           = 0,
    COMMAND_SET_EXPOSURE                            = 1,
    COMMAND_START_EXPOSURE                          = 2,
    COMMAND_STOP_EXPOSURE                           = 3,
    COMMAND_SET_CHANNEL_POWER                       = 4
};


struct InterfaceStatus
{
    bool is_controller_connected;
};


extern const uint8_t RADIO_ADDRESS_CONTROLLER[5];
extern const uint8_t RADIO_ADDRESS_INTERFACE[5];
extern const uint8_t RADIO_CHANNEL;

extern const uint8_t CHANNEL_POWER_SAFE;

extern const char *_comms_controller_state_strings[];
extern const char *_comms_command_strings[];
extern const char *_comms_status_strings[];


extern const uint8_t PACKET_SIZE;
typedef uint8_t RadioPacket;
/* Packet format:
 * RadioPacket[0]   CommsCommand (if master -> slave) or ControllerState (upper 2 bits) | CommsMessage (lower 6 bits) (if slave -> master)
 * RadioPacket[1]   Red channel power
 * RadioPacket[2]   Green channel power
 * RadioPacket[3]   Blue channel power
 * RadioPacket[4]   -+
 * RadioPacket[5]    |-- Target exposure time (milliseconds)
 * RadioPacket[6]    |   4 is MSB
 * RadioPacket[7]   -+
 * RadioPacket[8]   -+
 * RadioPacket[9]    |-- Achieved exposure time (milliseconds)
 * RadioPacket[10]   |   4 is MSB
 * RadioPacket[11]  -+
 * RadioPacket[12]  -+
 * RadioPacket[13]   |-- Reserved
 * RadioPacket[14]  -+
 * RadioPacket[15]  -- Packet counter
 */

#endif
