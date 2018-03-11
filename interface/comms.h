#pragma once

#include "shared.h"


struct ControllerExternalStatus
{
    ControllerState state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t achieved_millis;
};


void initialise_radio();

CommsMessage communicate_with_slave(const RadioPacket* out_packet, RadioPacket* returned_packet);
CommsMessage interpret_return_packet(const uint8_t* returned_packet, ControllerExternalStatus* controller_status);
CommsMessage set_controller_exposure(uint8_t red_power, uint8_t green_power, uint8_t blue_power, uint32_t target_millis);
CommsMessage send_command(CommsCommand command, ControllerExternalStatus* controller_status);
CommsMessage start_exposure();
CommsMessage stop_exposure();

#ifdef DEBUG
void print_status(const ControllerExternalStatus* controller_status);
void print_paired_status(uint8_t red_power, uint8_t green_power, uint8_t blue_power, uint32_t target_millis, const ControllerExternalStatus* controller_status);
void print_packet_raw(const RadioPacket* packet);
void print_packet(const RadioPacket* packet);
#endif

