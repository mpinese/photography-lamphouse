#define DEBUG

#include "comms.h"
#include "tft.h"

#include "shared.h"

/* Notes:
 *  Timing logic susceptible to overflow.  Will only be a problem if system is on
 *  continuously for ~ 50 days.  Would cause freeze.
 */


InterfaceStatus _interface_status;


void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    _interface_status.is_controller_connected = false;
    
    initialise_radio();
    display_init();

#ifdef DEBUG
    Serial.println("Interface: Init done");
#endif
}


void loop()
{
    display_loop();
}


void display_loop(void)
{
    static uint32_t display_next_update = 0;
    static uint8_t mode = 0;
    uint32_t tick = millis();

    if (tick < display_next_update)
        return;

    //  Mode    Action
    //  0       Radio query
    //  1       Display update
    //  2       Touch
    //  3       Display update
    //  4       Touch

    if (mode == 0)
        display_query_controller_state();
    else if (mode % 2 == 0)
        display_process_touch();
    else
        display_update();

    mode++;
    if (mode == 5)
        mode = 0;
    
    display_next_update = tick + 20;    // Display refreshes at 25 Hz (as display loop at 50 Hz, and only half of the calls result in a display refresh)
}

