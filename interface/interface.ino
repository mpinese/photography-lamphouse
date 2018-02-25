#include <SPI.h>
#include "nRF24L01_STM32.h"
#include "RF24_STM32.h"

#include "tft.h"
#include "FT8_config.h"


const static uint8_t RADIO_ADDRESS_CONTROLLER[5] = { 0x6B, 0xE3, 0x10, 0xE6, 0xCF };
const static uint8_t RADIO_ADDRESS_INTERFACE[5] =  { 0x6C, 0x28, 0xA4, 0x88, 0x44 };
const static uint8_t PIN_RADIO_CE = PA15;
const static uint8_t PIN_RADIO_CSN = PC15;
const static uint8_t RADIO_CHANNEL = 80;


struct RadioPacket
{
    uint8_t message;
    uint8_t data8;
    uint16_t data16;
};

SPIClass SPI_2(2);

static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static RadioPacket _radioData;


void setup()
{
    // Radio init
    _radioData.message = 0;
    _radioData.data8 = 0;
    _radioData.data16 = 0;

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
    _radio.setPALevel(RF24_PA_LOW);

    _radio.openWritingPipe(RADIO_ADDRESS_CONTROLLER);
    _radio.openReadingPipe(1, RADIO_ADDRESS_INTERFACE);


    // Display init
    digitalWrite(FT8_CS, HIGH);
    pinMode(FT8_CS, OUTPUT);
    digitalWrite(FT8_PDN, HIGH);
    pinMode(FT8_PDN, OUTPUT);

    SPI_2.begin(); /* sets up the SPI to run in Mode 0 and 1 MHz */
    //SPI_2.setClockDivider(SPI_CLOCK_DIV2);

    TFT_init();
}


void loop()
{
    if (_radioData.data8 == 255)
    {
        _radioData.data8 = 0;
        _radio.write(&_radioData, sizeof(_radioData));
        _radioData.message++;
    }
    else
        _radioData.data8++;
    
    if (_radioData.message == 3)
        _radioData.message = 0;

    _radio.write(&_radioData, sizeof(_radioData));

    delay(4); /* TFT_loop() must not be called to often as the FT8xx do not like more than about 60 screens per second */
    TFT_loop();  /* at 1 MHz SPI this returns in <1ms/~12ms which makes a total refresh of 4+1+4+12 = 21ms or about 48 frames per second */
}


