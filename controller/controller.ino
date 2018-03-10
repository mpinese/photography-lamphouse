#include "RF24.h"
#include <SPI.h>


/* Notes:
 *  Timing logic susceptible to overflow.  Will only be a problem if system is on
 *  continuously for ~ 50 days.
 */

const static uint8_t RADIO_ADDRESS_CONTROLLER[5] = { 0x6B, 0xE3, 0x10, 0xE6, 0xCF };
const static uint8_t RADIO_ADDRESS_INTERFACE[5] =  { 0x6C, 0x28, 0xA4, 0x88, 0x44 };
const static uint8_t PIN_RADIO_CE = 8;
const static uint8_t PIN_RADIO_CSN = 10;
const static uint8_t RADIO_CHANNEL = 80;

const static int PIN_OUT_GREEN = 3;
const static int PIN_OUT_BLUE = 6;
const static int PIN_OUT_RED = 9;


enum Status {
    STATUS_OK                                   = 0,
    STATUS_INVALID_MESSAGE                      = 1,
    STATUS_CANNOT_SET_EXPOSURE_WHILE_EXPOSING   = 2,
    STATUS_EXPOSURE_ALREADY_UNDERWAY            = 3,
    STATUS_NOT_EXPOSING                         = 4,
    STATUS_NO_RECEIVER                          = 5,
    STATUS_SET_FAILED                           = 6,
    STATUS_TIMEOUT                              = 7
};

struct __attribute__((packed)) RadioPacket
{
    uint8_t message;
    uint8_t state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t achieved_millis;
};

// FSM States:
// 0: Not exposing
// 1: Exposing
struct State
{
    uint8_t state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t start_millis;
    uint32_t end_millis;
};


void process_timers();
Status process_message();
Status send_status();
Status set_exposure();
Status start_exposure();
Status stop_exposure();


static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static RadioPacket _radioData;
static State _state;

/* ATmega168 RF24 notes:
 *  Receive -> Send state change takes 270 us
 *  Send -> Receive state change takes 250 us
 */
 

void setup()
{
    Serial.begin(115200);
    
    _state.state = 0;
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

    // Prepare the radio for receiving
    _radio.begin();
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setAddressWidth(5);
    _radio.setPALevel(RF24_PA_MIN);     // For close-proximity testing
    _radio.openWritingPipe(RADIO_ADDRESS_INTERFACE);
    _radio.openReadingPipe(1, RADIO_ADDRESS_CONTROLLER);
    _radio.startListening();

    Serial.println("Controller: Init done");
}


void loop()
{
    uint8_t pipe_no;
    while (_radio.available(&pipe_no))
    {
        _radio.read(&_radioData, sizeof(_radioData));
        Serial.println("Controller: Radio message received:");
        Serial.print("Controller:   Message: ");
        Serial.println(_radioData.message);
        Serial.print("Controller:   Power:   ");
        Serial.print(_radioData.channel_power[0]);
        Serial.print("  ");
        Serial.print(_radioData.channel_power[1]);
        Serial.print("  ");
        Serial.println(_radioData.channel_power[2]);
        Serial.print("Controller:   Target:  ");
        Serial.println(_radioData.target_millis);
        Status return_status = process_message();
    }
    process_timers();
}


/* 
 *  Messages:
 *  Byte    Interpretation
 *  0       Query state.
 *  1       Set exposure (channel power x 3, time).  Resets exposure accumulator.
 *  2       Start exposure
 *  3       Stop exposure
 *  
 *  States:
 *  State A: Waiting
 *  State B: Exposing
 *  
 *  State transitions:
 *  A -> B: On message 2
 *  B -> A: On message 3, or completion of exposure
 *  
 */


Status process_message()
{
    switch(_radioData.message)
    {
        case 0: return send_status();
        case 1: return set_exposure();
        case 2: return start_exposure();
        case 3: return stop_exposure();
        default: return STATUS_INVALID_MESSAGE;
    }
}


Status send_status()
{
    Serial.println("Controller: Preparing to send");
    _radio.stopListening();
    _radioData.message = STATUS_OK;
    _radioData.state = _state.state;
    _radioData.channel_power[0] = _state.channel_power[0];
    _radioData.channel_power[1] = _state.channel_power[1];
    _radioData.channel_power[2] = _state.channel_power[2];
    _radioData.target_millis = _state.target_millis;
    _radioData.achieved_millis = _state.end_millis - _state.start_millis;
    Serial.println("Controller: Sending status packet");
    Serial.println(sizeof(_radioData));
    if (!_radio.write(&_radioData, sizeof(_radioData)))
    {
        Serial.println("Controller: No receiver found");
        return STATUS_NO_RECEIVER;
    }
    Serial.println("Controller: Returning to receive mode");
    _radio.startListening();

    return STATUS_OK;
}


void process_timers()
{
    if (_state.state != 1)
        return;       // Not exposing
        
    _state.end_millis = millis();
    uint32_t exposed_time = _state.end_millis - _state.start_millis;

    if (exposed_time >= _state.target_millis)
        stop_exposure();
}


Status set_exposure()
{
    if (_state.state == 1)
        return STATUS_CANNOT_SET_EXPOSURE_WHILE_EXPOSING;
    
    _state.channel_power[0] = _radioData.channel_power[0];
    _state.channel_power[1] = _radioData.channel_power[1];
    _state.channel_power[2] = _radioData.channel_power[2];
    _state.target_millis = _radioData.target_millis;
    _state.start_millis = 0;
    _state.end_millis = 0;

    return STATUS_OK;
}


Status start_exposure()
{
    if (_state.state == 1)
        return STATUS_EXPOSURE_ALREADY_UNDERWAY;

    _state.state = 1;
    analogWrite(PIN_OUT_RED, _state.channel_power[0]);
    _state.start_millis = millis();
    analogWrite(PIN_OUT_BLUE, _state.channel_power[2]);
    analogWrite(PIN_OUT_GREEN, _state.channel_power[1]);

    return STATUS_OK;
}


Status stop_exposure()
{
    if (_state.state == 0)
        return STATUS_NOT_EXPOSING;
    
    _state.state = 0;

    analogWrite(PIN_OUT_BLUE, 0);
    analogWrite(PIN_OUT_GREEN, 0);
    _state.end_millis = millis();
    analogWrite(PIN_OUT_RED, 0);

    return STATUS_OK;
}

