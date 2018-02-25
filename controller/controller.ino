#include "RF24.h"
#include <SPI.h>


const static int PIN_OUT_GREEN = 3;
const static int PIN_OUT_BLUE = 6;
const static int PIN_OUT_RED = 9;

const static uint8_t RADIO_ADDRESS_CONTROLLER[5] = { 0x6B, 0xE3, 0x10, 0xE6, 0xCF };
const static uint8_t RADIO_ADDRESS_INTERFACE[5] =  { 0x6C, 0x28, 0xA4, 0x88, 0x44 };
const static uint8_t PIN_RADIO_CE = 8;
const static uint8_t PIN_RADIO_CSN = 10;
const static uint8_t RADIO_CHANNEL = 80;


struct RadioPacket
{
    uint8_t message;
    uint8_t data8;
    uint16_t data16;
};


static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static RadioPacket _radioData;


void setup()
{
    Serial.begin(115200);

    _radio.begin();
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setAddressWidth(5);
    _radio.setPALevel(RF24_PA_LOW);     // For close-proximity testing

    _radio.openWritingPipe(RADIO_ADDRESS_INTERFACE);
    _radio.openReadingPipe(1, RADIO_ADDRESS_CONTROLLER);

    _radio.startListening();
}


void loop()
{
    if (_radio.available())
    {
        _radio.read(&_radioData, sizeof(_radioData));

        String msg = "Radio ";
        msg += _radioData.message;
        msg += ", ";
        msg += _radioData.data8;
        msg += " ";
        msg += _radioData.data16;
        Serial.println(msg);

        switch(_radioData.message)
        {
        case 0:
            analogWrite(PIN_OUT_GREEN, _radioData.data8);
            break;
        case 1:
            analogWrite(PIN_OUT_BLUE, _radioData.data8);
            break;
        case 2:
            analogWrite(PIN_OUT_RED, _radioData.data8);
            break;
        }
    }
}

