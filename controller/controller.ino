#include "RF24.h"
#include <SPI.h>

#include "shared.h"

#undef DEBUG

/* Notes:
 *  Timing logic susceptible to overflow.  Will only be a problem if system is on
 *  continuously for ~ 50 days.
 */
/* ATmega168 RF24 notes:
 *  Receive -> Send state change takes 270 us
 *  Send -> Receive state change takes 250 us
 */

const static uint8_t PIN_RADIO_CE = 8;
const static uint8_t PIN_RADIO_CSN = 10;

const static int PIN_OUT_GREEN = 3;     // D3
const static int PIN_OUT_BLUE = 6;      // D6
const static int PIN_OUT_RED = 9;       // D9


struct ControllerInternalStatus
{
    ControllerState state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t start_millis;
    uint32_t end_millis;
};

#ifdef DEBUG
struct ControllerExternalStatus
{
    ControllerState state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t achieved_millis;
};
#endif



void initialise_outputs();
void initialise_radio();

void communicate_with_master();
void process_timers();

CommsMessage process_command(const RadioPacket* in_packet);
void construct_return_packet(CommsMessage message, RadioPacket* return_packet);
CommsMessage respond_to_master(const RadioPacket* return_packet);
CommsMessage set_exposure(const RadioPacket* in_packet);
CommsMessage start_exposure();
CommsMessage stop_exposure();
CommsMessage set_channel_power(const RadioPacket* in_packet);


#ifdef DEBUG
CommsMessage interpret_packet(const uint8_t* returned_packet, ControllerExternalStatus* controller_status);
void print_packet_raw(const RadioPacket* packet);
void print_packet(const RadioPacket* packet);
#endif


static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static ControllerInternalStatus _state;


void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    initialise_outputs();
    initialise_radio();

#ifdef DEBUG
    Serial.println("Controller: Init done");
#endif
}


void initialise_outputs()
{
    _state.state = CONTROLLER_STATE_NOT_EXPOSING;
    _state.channel_power[0] = 0;
    _state.channel_power[1] = 0;
    _state.channel_power[2] = 0;
    _state.target_millis = 0;
    _state.start_millis = 0;
    _state.end_millis = 0;

    // Set outputs to off
    pinMode(PIN_OUT_RED, OUTPUT);
    pinMode(PIN_OUT_GREEN, OUTPUT);
    pinMode(PIN_OUT_BLUE, OUTPUT);
    analogWrite(PIN_OUT_RED, 0);
    analogWrite(PIN_OUT_GREEN, 0);
    analogWrite(PIN_OUT_BLUE, 0);
}


void initialise_radio()
{
    _radio.begin();
    _radio.setRetries(15,15);
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setAddressWidth(5);
#ifdef DEBUG
    _radio.setPALevel(RF24_PA_MIN);     // For close-proximity testing
#else
    _radio.setPALevel(RF24_PA_LOW);
#endif
    _radio.openWritingPipe(RADIO_ADDRESS_INTERFACE);
    _radio.openReadingPipe(1, RADIO_ADDRESS_CONTROLLER);
    _radio.startListening();
}


void loop()
{
    communicate_with_master();
    process_timers();
}


void communicate_with_master()
{
    bool message_received = false;
    RadioPacket in_packet[PACKET_SIZE], return_packet[PACKET_SIZE];
    CommsMessage return_message;
    
    while (_radio.available())
    {
        message_received = true;
        _radio.read(&in_packet, PACKET_SIZE);
#ifdef DEBUG
        Serial.println("Recieved packet:");
        print_packet(&in_packet[0]);
#endif
        return_message = process_command(&in_packet[0]);
    }
    
    if (message_received)
    {
        construct_return_packet(return_message, &return_packet[0]);
#ifdef DEBUG
        Serial.println("Sending packet:");
        print_packet(&return_packet[0]);
#endif
        respond_to_master(&return_packet[0]);
    }
}


CommsMessage process_command(const RadioPacket* in_packet)
{
    CommsCommand command;
    command = CommsCommand(in_packet[0]);
    
    switch (command)
    {
        case COMMAND_REPORT_STATUS:     return MESSAGE_OK;
        case COMMAND_SET_EXPOSURE:      return set_exposure(in_packet);
        case COMMAND_START_EXPOSURE:    return start_exposure();
        case COMMAND_STOP_EXPOSURE:     return stop_exposure();
        case COMMAND_SET_CHANNEL_POWER: return set_channel_power(in_packet);
    }
    
    return MESSAGE_INVALID_COMMAND;
}


void construct_return_packet(CommsMessage message, RadioPacket* return_packet)
{
    uint32_t achieved_millis = _state.end_millis - _state.start_millis;
    
    return_packet[0] = (uint8_t(message) & 0x3F) | (uint8_t(_state.state) << 6);
    
    return_packet[1] = _state.channel_power[0];
    return_packet[2] = _state.channel_power[1];
    return_packet[3] = _state.channel_power[2];
    
    return_packet[4] = _state.target_millis >> 32;
    return_packet[5] = (_state.target_millis >> 16) & 0xFF;
    return_packet[6] = (_state.target_millis >> 8) & 0xFF;
    return_packet[7] = _state.target_millis & 0xFF;
    
    return_packet[8] = achieved_millis >> 32;
    return_packet[9] = (achieved_millis >> 16) & 0xFF;
    return_packet[10] = (achieved_millis >> 8) & 0xFF;
    return_packet[11] = achieved_millis & 0xFF;
}


CommsMessage respond_to_master(const RadioPacket* return_packet)
{
    _radio.stopListening();
    if (!_radio.write(return_packet, PACKET_SIZE))
    {
        _radio.startListening();
        return MESSAGE_NO_RECEIVER;
    }
    _radio.startListening();

    return MESSAGE_OK;
}


CommsMessage set_exposure(const RadioPacket* in_packet)
{
    if (_state.state == CONTROLLER_STATE_EXPOSING)
        return MESSAGE_CANNOT_SET_EXPOSURE_WHILE_EXPOSING;
    
    _state.channel_power[1] = in_packet[2];
    _state.channel_power[2] = in_packet[3];
    _state.target_millis = in_packet[4];
    _state.target_millis <<= 8;
    _state.target_millis |= in_packet[5];
    _state.target_millis <<= 8;
    _state.target_millis |= in_packet[6];
    _state.target_millis <<= 8;
    _state.target_millis |= in_packet[7];
    _state.start_millis = 0;
    _state.end_millis = 0;

    return MESSAGE_OK;
}


CommsMessage start_exposure()
{
    if (_state.state == CONTROLLER_STATE_EXPOSING)
        return MESSAGE_EXPOSURE_ALREADY_UNDERWAY;

    _state.state = CONTROLLER_STATE_EXPOSING;
    _state.start_millis = millis();
    analogWrite(PIN_OUT_BLUE, _state.channel_power[2]);
    analogWrite(PIN_OUT_GREEN, _state.channel_power[1]);

    return MESSAGE_OK;    
}


CommsMessage stop_exposure()
{
    if (_state.state == CONTROLLER_STATE_NOT_EXPOSING)
        return MESSAGE_NOT_EXPOSING;
    
    _state.state = CONTROLLER_STATE_NOT_EXPOSING;

    analogWrite(PIN_OUT_BLUE, 0);
    analogWrite(PIN_OUT_GREEN, 0);
    _state.end_millis = millis();

    return MESSAGE_OK;    
}


void process_timers()
{
    if (_state.state != CONTROLLER_STATE_EXPOSING)
        return;       // Not exposing
        
    _state.end_millis = millis();
    uint32_t exposed_time = _state.end_millis - _state.start_millis;

    if (exposed_time >= _state.target_millis)
        stop_exposure();
}


CommsMessage set_channel_power(const RadioPacket* in_packet)
{
    if (_state.state == CONTROLLER_STATE_EXPOSING)
        return MESSAGE_CANNOT_SET_EXPOSURE_WHILE_EXPOSING;
    
    _state.channel_power[0] = in_packet[1];
    _state.channel_power[1] = in_packet[2];
    _state.channel_power[2] = in_packet[3];
    _state.start_millis = 0;
    _state.end_millis = 0;
    analogWrite(PIN_OUT_RED, _state.channel_power[0]);
    analogWrite(PIN_OUT_GREEN, _state.channel_power[1]);
    analogWrite(PIN_OUT_BLUE, _state.channel_power[2]);

    return MESSAGE_OK;   
}



#ifdef DEBUG
CommsMessage interpret_packet(const uint8_t* returned_packet, ControllerExternalStatus* controller_status)
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
    
    msg = interpret_packet(packet, &controller_status);

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

