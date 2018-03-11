#include "comms.h"
#include "tft.h"

#include "shared.h"

#undef DEBUG

/* Notes:
 *  Timing logic susceptible to overflow.  Will only be a problem if system is on
 *  continuously for ~ 50 days.
 */


void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    initialise_radio();
    initialise_display();

#ifdef DEBUG
    Serial.println("Interface: Init done");
#endif
}


void loop()
{
    static uint8_t i = 0;
    
    uint8_t red_power, green_power, blue_power;
    uint32_t target_millis = 1100;
    CommsMessage message_status;
    ControllerExternalStatus controller_status;

    red_power = 255*(i & 1);
    green_power = 255*((i >> 1) & 1);
    blue_power = 255*((i >> 2) & 1);

    Serial.print("Interface: attempting to set exposure to ");
    Serial.print(red_power);
    Serial.print("/");
    Serial.print(green_power);
    Serial.print("/");
    Serial.print(blue_power);
    Serial.print("//");
    Serial.println(target_millis);
    
    message_status = set_controller_exposure(red_power, green_power, blue_power, target_millis);

    Serial.print("Interface: message returned: ");
    Serial.println(_comms_status_strings[size_t(message_status)]);
    
    if (message_status == MESSAGE_OK)
    {
        Serial.println("Interface: attempting to start exposure");
        message_status = start_exposure();
        Serial.print("Interface: message returned: ");
        Serial.println(_comms_status_strings[size_t(message_status)]);
    }
    delay(1000);

    i++;
    if (i == 8)
        i = 0;

    delay(4); /* TFT_loop() must not be called to often as the FT8xx do not like more than about 60 screens per second */
    display_loop();  /* at 1 MHz SPI this returns in <1ms/~12ms which makes a total refresh of 4+1+4+12 = 21ms or about 48 frames per second */
}

