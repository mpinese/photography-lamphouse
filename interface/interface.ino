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
    initialise_display();

#ifdef DEBUG
    Serial.println("Interface: Init done");
#endif
}


void loop()
{
    radio_loop();
    display_loop();
}


void display_loop(void)
{
    static uint32_t display_update_watchdog = 0;
    static bool mode = false;
    uint32_t tick = millis();

    if (tick < display_update_watchdog)
        return;

    if (mode)
        display_process_touch();
    else
        display_update();

    mode = !mode;
    display_update_watchdog = tick + 20;        // 20 ms between calls, so 40 ms between display refreshes (25 Hz)
}


void radio_loop()
{
    static uint32_t controller_connection_watchdog = 0;

    uint32_t tick = millis();

    if (tick < controller_connection_watchdog)
        return;

    ControllerExternalStatus controller_interface_status;
    CommsMessage comms_interface_status;

    comms_interface_status = send_command(COMMAND_REPORT_STATUS, &controller_interface_status);

    _interface_status.is_controller_connected = comms_interface_status == MESSAGE_OK;

    if (_interface_status.is_controller_connected == true)
        controller_connection_watchdog = tick + 200;
    else
        controller_connection_watchdog = tick + 1000;
}

