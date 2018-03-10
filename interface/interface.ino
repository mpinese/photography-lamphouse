#include <SPI.h>
#include "nRF24L01_STM32.h"
#include "RF24_STM32.h"

#include "tft.h"
#include "FT8_config.h"


/* Notes:
 *  Timing logic susceptible to overflow.  Will only be a problem if system is on
 *  continuously for ~ 50 days.
 */


const static uint8_t RADIO_ADDRESS_CONTROLLER[5] = { 0x6B, 0xE3, 0x10, 0xE6, 0xCF };
const static uint8_t RADIO_ADDRESS_INTERFACE[5] =  { 0x6C, 0x28, 0xA4, 0x88, 0x44 };
const static uint8_t PIN_RADIO_CE = PA15;
const static uint8_t PIN_RADIO_CSN = PC15;
const static uint8_t RADIO_CHANNEL = 80;

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

const char *_status_strings[] = {
    "OK",
    "Invalid message",
    "Cannot set exposure while exposing",
    "Exposure already underway",
    "Not exposing",
    "No receiver",
    "Set failed",
    "Timeout"
};

struct __attribute__((packed)) RadioPacket
{
    uint8_t message;
    uint8_t state;
    uint8_t channel_power[3];
    uint32_t target_millis;
    uint32_t achieved_millis;
};


Status get_controller_exposure(RadioPacket* exposure);
Status set_controller_exposure(uint8_t channel_power[3], uint32_t target_millis);
Status send_command(uint8_t command);
Status start_exposure();
Status stop_exposure();


SPIClass SPI_2(2);

static RF24 _radio(PIN_RADIO_CE, PIN_RADIO_CSN);
static RadioPacket _radioData;


/* STM32 RF24 notes:
 *  Receive -> Send state change takes 375 us
 *  Send -> Receive state change takes 175 us
 */

void setup()
{
    Serial.begin(115200);
    
    // Radio init
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
    _radio.setPALevel(RF24_PA_MIN);     // For close-proximity testing
    _radio.openWritingPipe(RADIO_ADDRESS_CONTROLLER);
    _radio.openReadingPipe(1, RADIO_ADDRESS_INTERFACE);
    _radio.stopListening();
    
    // Display init
//    digitalWrite(FT8_CS, HIGH);
//    pinMode(FT8_CS, OUTPUT);
//    digitalWrite(FT8_PDN, HIGH);
//    pinMode(FT8_PDN, OUTPUT);
//
//    SPI_2.begin(); /* sets up the SPI to run in Mode 0 and 1 MHz */
//    //SPI_2.setClockDivider(SPI_CLOCK_DIV2);
//
//    TFT_init();

    Serial.println("Interface: Init done");
}


void loop()
{
    static uint8_t channel_power[3] = {255, 0, 0};
    static uint32_t target_time=1000;
    Status retcode;

    Serial.print("Interface: attempting to set exposure to ");
    Serial.print(channel_power[0]);
    Serial.print("/");
    Serial.print(channel_power[1]);
    Serial.print("/");
    Serial.print(channel_power[2]);
    Serial.print("/");
    Serial.println(target_time);
    retcode = set_controller_exposure(&channel_power[0], 1050);

    Serial.print("Status: ");
    Serial.println(_status_strings[size_t(retcode)]);
    
    if (retcode == STATUS_OK)
    {
        Serial.println("Interface: attempting to start exposure");
        retcode = start_exposure();
        Serial.print("Status: ");
        Serial.println(_status_strings[size_t(retcode)]);
    }
    delay(100);

//    delay(4); /* TFT_loop() must not be called to often as the FT8xx do not like more than about 60 screens per second */
//    TFT_loop();  /* at 1 MHz SPI this returns in <1ms/~12ms which makes a total refresh of 4+1+4+12 = 21ms or about 48 frames per second */
}


Status get_controller_exposure(RadioPacket* exposure)
{
    RadioPacket out_packet;
    
    out_packet.message = 0;

    if (!_radio.write(&out_packet, sizeof(out_packet)))
        return STATUS_NO_RECEIVER;

    _radio.startListening();

    uint32_t end_time = millis() + 40;
    bool message_received = false;
    while (message_received == false && millis() < end_time)
    {
        delay(1);
        while (_radio.available())
        {
            message_received = true;
            _radio.read(exposure, sizeof(*exposure));
        }
    }
    _radio.stopListening();
    
    if (message_received == false)
        return STATUS_TIMEOUT;

    return STATUS_OK;
}


Status set_controller_exposure(uint8_t channel_power[3], uint32_t target_millis)
{
    RadioPacket out_packet, ack_packet;
    Status comm_status;

    out_packet.channel_power[0] = channel_power[0];
    out_packet.channel_power[1] = channel_power[1];
    out_packet.channel_power[2] = channel_power[2];
    out_packet.target_millis = target_millis;
    out_packet.message = 1;

    if (!_radio.write(&out_packet, sizeof(out_packet)))
        return STATUS_NO_RECEIVER;

    if ((comm_status = get_controller_exposure(&ack_packet)) != STATUS_OK)
        return comm_status;
    if (
        ack_packet.channel_power[0] == channel_power[0] &&
        ack_packet.channel_power[1] == channel_power[1] &&
        ack_packet.channel_power[2] == channel_power[2] &&
        ack_packet.target_millis == target_millis)
        return STATUS_OK;

    return STATUS_SET_FAILED;
}


Status send_command(uint8_t command)
{
    RadioPacket packet;
    
    packet.message = command;

    if (!_radio.write(&packet, sizeof(packet)))
        return STATUS_NO_RECEIVER;
    
    return STATUS_OK;
}


Status start_exposure()
{
    return send_command(2);
}


Status stop_exposure()
{
    return send_command(3);
}

