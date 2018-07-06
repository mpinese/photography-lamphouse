#include <SPI.h>
#include "nRF24L01_STM32.h"
#include "RF24_STM32.h"

#include "comms.h"
#include "shared.h"

const static uint8_t PIN_RADIO_CE = PA15;
const static uint8_t PIN_RADIO_CSN = PC15;

static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);


void initialise_radio()
{
    digitalWrite(PIN_RADIO_CSN, HIGH);
    pinMode(PIN_RADIO_CSN, OUTPUT);
    digitalWrite(PIN_RADIO_CE, HIGH);
    pinMode(PIN_RADIO_CE, OUTPUT);
    
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    _radio.begin();
    _radio.setRetries(15,15);
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setAddressWidth(5);
#ifdef DEBUG
    _radio.setPALevel(RF24_PA_MIN);     // For close-proximity testing
#else
    _radio.setPALevel(RF24_PA_LOW);
#endif
    _radio.openWritingPipe(RADIO_ADDRESS_CONTROLLER);
    _radio.openReadingPipe(1, RADIO_ADDRESS_INTERFACE);
    _radio.stopListening();    
}


CommsMessage communicate_with_slave(const RadioPacket* out_packet, RadioPacket* returned_packet)
{
    static uint8_t packet_counter = 0;

    RadioPacket out_packet_copy[PACKET_SIZE];
    RadioPacket* out_packet_copy_ptr = &out_packet_copy[0];
    memcpy(out_packet_copy_ptr, out_packet, sizeof(out_packet_copy));
    out_packet_copy[15] = packet_counter++;
    
    _radio.stopListening();

#ifdef DEBUG
    Serial.println("Interface: Sending packet:");
    print_packet(out_packet_copy_ptr);
#endif

    if (!_radio.write(out_packet_copy_ptr, PACKET_SIZE))
        return MESSAGE_NO_RECEIVER;

    _radio.startListening();

    uint32_t end_time = millis() + 40;
    bool message_received = false;
    while (message_received == false && millis() < end_time)
    {
        delay(1);
        while (_radio.available())
        {
            message_received = true;
            _radio.read(returned_packet, PACKET_SIZE);
        }
    }
    _radio.stopListening();

#ifdef DEBUG
    Serial.println("Interface: Received packet:");
    print_packet(returned_packet);
#endif

    if (message_received == false)
        return MESSAGE_TIMEOUT;

    return MESSAGE_OK;
}


CommsMessage interpret_return_packet(const uint8_t* returned_packet, ControllerExternalStatus* controller_status)
{
    controller_status->state = ControllerState(returned_packet[0] >> 6);
    
    controller_status->channel_power[0] = returned_packet[1];
    controller_status->channel_power[1] = returned_packet[2];
    controller_status->channel_power[2] = returned_packet[3];
    
    controller_status->target_millis = returned_packet[4];
    controller_status->target_millis <<= 8;
    controller_status->target_millis |= returned_packet[5];
    controller_status->target_millis <<= 8;
    controller_status->target_millis |= returned_packet[6];
    controller_status->target_millis <<= 8;
    controller_status->target_millis |= returned_packet[7];

    controller_status->achieved_millis = returned_packet[8];
    controller_status->achieved_millis <<= 8;
    controller_status->achieved_millis |= returned_packet[9];
    controller_status->achieved_millis <<= 8;
    controller_status->achieved_millis |= returned_packet[10];
    controller_status->achieved_millis <<= 8;
    controller_status->achieved_millis |= returned_packet[11];

    return CommsMessage(returned_packet[0] & 0x3F);
}


CommsMessage set_controller_exposure(uint8_t green_power, uint8_t blue_power, uint32_t target_millis)
{
    CommsMessage comms_message;
    RadioPacket out_packet[PACKET_SIZE], returned_packet[PACKET_SIZE];
    ControllerExternalStatus controller_status;

    out_packet[0] = uint8_t(COMMAND_SET_EXPOSURE);
    out_packet[1] = 0;
    out_packet[2] = green_power;
    out_packet[3] = blue_power;
    out_packet[4] = target_millis >> 32;
    out_packet[5] = (target_millis >> 16) & 0xFF;
    out_packet[6] = (target_millis >> 8) & 0xFF;
    out_packet[7] = target_millis & 0xFF;

    comms_message = communicate_with_slave(&out_packet[0], &returned_packet[0]);

    if (comms_message != MESSAGE_OK)
        return comms_message;

    comms_message = interpret_return_packet(&returned_packet[0], &controller_status);

    if (comms_message != MESSAGE_OK)
        return comms_message;

#ifdef DEBUG
    print_paired_status(0, green_power, blue_power, target_millis, &controller_status);
#endif

    if (
        controller_status.channel_power[1] == green_power &&
        controller_status.channel_power[2] == blue_power &&
        controller_status.target_millis == target_millis)
        return MESSAGE_OK;

    return MESSAGE_SET_FAILED;
}


CommsMessage set_channel_power(uint8_t red_power, uint8_t green_power, uint8_t blue_power)
{
    CommsMessage comms_message;
    RadioPacket out_packet[PACKET_SIZE], returned_packet[PACKET_SIZE];
    ControllerExternalStatus controller_status;

    out_packet[0] = uint8_t(COMMAND_SET_CHANNEL_POWER);
    out_packet[1] = red_power;
    out_packet[2] = green_power;
    out_packet[3] = blue_power;
    out_packet[4] = 0;
    out_packet[5] = 0;
    out_packet[6] = 0;
    out_packet[7] = 0;

    comms_message = communicate_with_slave(&out_packet[0], &returned_packet[0]);

    if (comms_message != MESSAGE_OK)
        return comms_message;

    comms_message = interpret_return_packet(&returned_packet[0], &controller_status);

    if (comms_message != MESSAGE_OK)
        return comms_message;
    
    if (
        controller_status.channel_power[0] == red_power &&
        controller_status.channel_power[1] == green_power &&
        controller_status.channel_power[2] == blue_power)
        return MESSAGE_OK;

    return MESSAGE_SET_FAILED;
}


CommsMessage send_command(CommsCommand command, ControllerExternalStatus* controller_status)
{
    CommsMessage comms_message;
    RadioPacket out_packet[PACKET_SIZE], returned_packet[PACKET_SIZE];

    out_packet[0] = uint8_t(command);

    comms_message = communicate_with_slave(&out_packet[0], &returned_packet[0]);

    if (comms_message != MESSAGE_OK)
        return comms_message;

    return interpret_return_packet(&returned_packet[0], controller_status);
}


CommsMessage start_exposure()
{
    ControllerExternalStatus controller_status;
    CommsMessage comms_message;

    comms_message = send_command(COMMAND_START_EXPOSURE, &controller_status);

    if (comms_message != MESSAGE_OK)
        return comms_message;

    if (controller_status.state != CONTROLLER_STATE_EXPOSING)
        return MESSAGE_SET_FAILED;

    return MESSAGE_OK;
}


CommsMessage stop_exposure()
{
    ControllerExternalStatus controller_status;
    CommsMessage comms_message;

    comms_message = send_command(COMMAND_STOP_EXPOSURE, &controller_status);

    if (comms_message != MESSAGE_OK)
        return comms_message;

    if (controller_status.state != CONTROLLER_STATE_NOT_EXPOSING)
        return MESSAGE_SET_FAILED;

    return MESSAGE_OK;
}


#ifdef DEBUG
void print_status(const ControllerExternalStatus* controller_status)
{
    Serial.println("CONTROLLER STATUS:");
    Serial.println("Value\tController");
    Serial.print("Red\t");
    Serial.println(controller_status->channel_power[0]);
    Serial.print("Green\t");
    Serial.println(controller_status->channel_power[1]);
    Serial.print("Blue\t");
    Serial.println(controller_status->channel_power[2]);
    Serial.print("\t");
    Serial.println(controller_status->target_millis);
}


void print_paired_status(uint8_t red_power, uint8_t green_power, uint8_t blue_power, uint32_t target_millis, const ControllerExternalStatus* controller_status)
{
    Serial.println("CONTROLLER-INTERFACE AGREEMENT:");
    Serial.println("Value\tInterface\tController");
    Serial.print("Red\t");
    Serial.print(red_power);
    Serial.print("\t\t");
    Serial.println(controller_status->channel_power[0]);
    Serial.print("Green\t");
    Serial.print(green_power);
    Serial.print("\t\t");
    Serial.println(controller_status->channel_power[1]);
    Serial.print("Blue\t");
    Serial.print(blue_power);
    Serial.print("\t\t");
    Serial.println(controller_status->channel_power[2]);
    Serial.print("Target\t");
    Serial.print(target_millis);
    Serial.print("\t\t");
    Serial.println(controller_status->target_millis);
}


void print_packet_raw(const RadioPacket* packet)
{
    Serial.println("RAW PACKET:");
    for (size_t i = 0; i < PACKET_SIZE; i++)
    {
        Serial.print(packet[i]);
        Serial.print(" ");
    }
    Serial.println();
}


void print_packet(const RadioPacket* packet)
{
    CommsMessage msg;
    ControllerExternalStatus controller_status;
    
    msg = interpret_return_packet(packet, &controller_status);

    print_packet_raw(packet);
    Serial.println("PACKET:");
    Serial.print("Com/Msg:  ");
    Serial.println(_comms_command_strings[size_t(msg)]);
    Serial.print("State:    ");
    Serial.println(controller_status.state);
    Serial.print("Red:      ");
    Serial.println(controller_status.channel_power[0]);
    Serial.print("Green:    ");
    Serial.println(controller_status.channel_power[1]);
    Serial.print("Blue:     ");
    Serial.println(controller_status.channel_power[2]);
    Serial.print("Target:   ");
    Serial.println(controller_status.target_millis);
    Serial.print("Achieved: ");
    Serial.println(controller_status.achieved_millis);
    Serial.print("Reserved: ");
    Serial.print(packet[12]);
    Serial.print(" ");
    Serial.print(packet[13]);
    Serial.print(" ");
    Serial.print(packet[14]);
    Serial.print(" ");
    Serial.println(packet[15]);
}
#endif
