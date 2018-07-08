#include "tft.h"

#include "FT8_config.h"
#include "FT8.h"
#include "FT8_commands.h"

#include "comms.h"
#include "shared.h"


#define DARKRED   0x00400000

struct DisplayState
{
    bool hc, red, on;
    uint16_t dial_angle;
    uint16_t set_time_lc, set_time_hc, current_time_lc, current_time_hc, start_time_lc, start_time_hc;
};


SPIClass SPI_2(2);
DisplayState _display_state;


// Defined in interface.ino
extern InterfaceStatus _interface_status;


void display_init()
{
    _display_state.hc = false;
    _display_state.red = false;
    _display_state.on = false;
    _display_state.dial_angle = 0x8000;
    _display_state.set_time_lc = 0;
    _display_state.set_time_hc = 0;
    _display_state.current_time_lc = 0;
    _display_state.current_time_hc = 0;
    _display_state.start_time_lc = 0;
    _display_state.start_time_hc = 0;
    
    digitalWrite(FT8_CS, HIGH);
    pinMode(FT8_CS, OUTPUT);
    digitalWrite(FT8_PDN, HIGH);
    pinMode(FT8_PDN, OUTPUT);

    SPI_2.begin(); /* sets up the SPI to run in Mode 0 and 1 MHz */
    SPI_2.setClockDivider(SPI_CLOCK_DIV32);

    FT8_init();
    FT8_cmd_setrotate(2);
    FT8_cmd_track(480/2, 800/2+25, 1, 1, TAG(5));       // Register tracking for the spinner
    FT8_cmd_execute();

//    display_calibrate_touch();

    /* send pre-recorded touch calibration values, RVT70, rotation 0 */
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010ad7);
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000000);
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_C, 0xffe9d9a5);
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000049);
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010750);
//    FT8_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfff85903);

    /* send pre-recorded touch calibration values, RVT70, rotation 2 */
    FT8_memWrite32(REG_TOUCH_TRANSFORM_A, 0xfffffd3c);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_B, 0xfffee719);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_C, 0x01f1d6f1);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00010d36);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00000396);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_F, 0xffe44224);

#ifdef DEBUG
    FT8_memWrite8(REG_PWM_DUTY, 30);    // Bright backlight for testing
#else
    FT8_memWrite8(REG_PWM_DUTY, 2);     // Dim backlight for darkroom use
#endif
}


void display_process_touch()
{
    display_process_touch_buttons();
    display_process_touch_dial();
}


void display_process_touch_buttons()
{
    // TODO: beep
    
    static uint32_t last_processed_touch_millis = 0;

    // Rudimentary debouncing.
    if (millis() - last_processed_touch_millis < 200)
        return;

    uint8_t tag = FT8_get_touch_tag();

    if (tag >= 1 && tag <= 4)
        last_processed_touch_millis = millis();

    switch(tag)
    {
        case 1:     // HC/LC toggle
            if (!_display_state.on)
                _display_state.hc = !_display_state.hc;
            break;
        case 2:     // Red channel toggle
            if (!_interface_status.is_controller_connected)
                break;
            if (set_channel_power(_display_state.red ? 0 : CHANNEL_POWER_SAFE, 0, 0) == MESSAGE_OK)
                _display_state.red = !_display_state.red;
            break;
        case 3:     // Start/Stop
            if (!_interface_status.is_controller_connected)
                break;
            if (_display_state.on)
            {
                stop_exposure();
                _display_state.on = false;
            }
            else
            {
                // Send the time to the controller, and start the exposure.
                // We need to convert the time in _display_state to ms for use by the controller.
                // _display_state.current_time_lc*1.5625 is time in ms
                // 1.5625 is 25/16
                // 
                // However, the display only has a resolution of 100 ms, even though the internal
                // state has a much higher resolution.  Round the internal state value to match the
                // displayed one, so that the exposure time is exactly what's shown on the display.
                //
                // To do this conversion:
                // time_s = (current_time >> 6) / 10
                // time_ds = (current_time >> 6) % 10
                // time_ms = time_s*1000 + time_ds*100
                uint16_t& set_time_ref = _display_state.hc ? _display_state.set_time_hc : _display_state.set_time_lc;
                uint16_t& current_time_ref = _display_state.hc ? _display_state.current_time_hc : _display_state.current_time_lc;
                uint16_t& start_time_ref = _display_state.hc ? _display_state.start_time_hc : _display_state.start_time_lc;
                
                if (set_time_ref <= current_time_ref)
                    break;
                
                start_time_ref = current_time_ref;
                uint8_t time_s = ((set_time_ref - current_time_ref) >> 6) / 10;
                uint8_t time_ds = ((set_time_ref - current_time_ref) >> 6) % 10;
                uint32_t target_millis = uint32_t(time_s)*1000 + uint32_t(time_ds)*100;
                // Old code (kept fractional time not shown on the rounded display):
                // uint32_t target_millis = (uint32_t(set_time_ref - current_time_ref)*25) >> 4;

                set_channel_power(_display_state.red ? CHANNEL_POWER_SAFE : 0, 0, 0);
                if (set_controller_exposure(_display_state.hc ? 0 : CHANNEL_POWER_LC, _display_state.hc ? CHANNEL_POWER_HC : 0, target_millis) != MESSAGE_OK)
                    break;
                if (start_exposure() != MESSAGE_OK)
                    break;
                _display_state.on = true;
            }
            break;
        case 4:     // Reset
            if (!_display_state.on)
            {
                uint16_t& set_time_ref = _display_state.hc ? _display_state.set_time_hc : _display_state.set_time_lc;
                uint16_t& current_time_ref = _display_state.hc ? _display_state.current_time_hc : _display_state.current_time_lc;
                uint16_t& start_time_ref = _display_state.hc ? _display_state.start_time_hc : _display_state.start_time_lc;

                start_time_ref = 0;
                if (current_time_ref == 0)
                    set_time_ref = 0;
                else
                    current_time_ref = 0;
            }
            break;
    }
}


void display_process_touch_dial()
{
    // Simple approach -- no acceleration.
    // 16 revolutions for 102.4 seconds 
    // => 1024 angle units per tenth of a second.
    static bool touch_helddown = false;
    static uint32_t last_touch_millis = 0;
    
    uint32_t tracker = FT8_memRead32(REG_TRACKER);
    if ((tracker & 0xff) == 5)
    {
        uint16_t new_angle = tracker >> 16;
        int32_t delta_angle;
        
        if (_display_state.dial_angle > 0xf000 && new_angle < 0x1000)
            delta_angle = new_angle + (0xffff - _display_state.dial_angle);                 // Clockwise movement passing 6 o'clock.
        else if (_display_state.dial_angle < 0x1000 && new_angle > 0xf000)
            delta_angle = int32_t(0xffff - new_angle) - int32_t(_display_state.dial_angle); // Anti-clockwise movement passing 6 o'clock.
        else
            delta_angle = int32_t(new_angle) - int32_t(_display_state.dial_angle);

        // delta_angle > 0 => clockwise movement.

        // Track whether the dial is being continuously touched by
        // a finger, and use a less stringent angle filter if so.
        uint32_t this_touch_millis = millis();
        if (this_touch_millis - last_touch_millis < 200)
            touch_helddown = true;
        else
            touch_helddown = false;
        last_touch_millis = this_touch_millis;

        // Ignore very large angle changes (0x1000 => 22.5 degrees, 0x3000 => 67.5 degrees)
        if ((!touch_helddown && (delta_angle < -0x1000 || delta_angle > 0x1000)) || 
            (touch_helddown && (delta_angle < -0x3000 || delta_angle > 0x3000)))
            return;

        uint16_t& set_time_ref = _display_state.hc ? _display_state.set_time_hc : _display_state.set_time_lc;

        if (delta_angle < 0)
        {
            if (set_time_ref < uint16_t(-delta_angle) >> 4)
                set_time_ref = 0;
            else
                set_time_ref -= uint16_t(-delta_angle) >> 4;
        }
        else
        {
            if (0xffff - set_time_ref <  uint16_t(delta_angle) >> 4)
                set_time_ref = 0xffff;
            else
                set_time_ref += uint16_t(delta_angle) >> 4;
        }

        _display_state.dial_angle = new_angle;
    }
}


void display_query_controller_state()
{
    ControllerExternalStatus controller_status;
    CommsMessage comms_status;

    comms_status = send_command(COMMAND_REPORT_STATUS, &controller_status);

    _interface_status.is_controller_connected = comms_status == MESSAGE_OK;
    
    if (comms_status != MESSAGE_OK)
        return;

    // TODO: synchronise with controller state when connection lost

    if (_display_state.on)
    {
        // Either exposure is in progress, or exposure has just completed.  Either way assign the achieved_millis to the relevant achieved time variable.
        uint16_t achieved_time = uint16_t((controller_status.achieved_millis << 4) / 25);
        uint16_t& current_time_ref = _display_state.hc ? _display_state.current_time_hc : _display_state.current_time_lc;
        uint16_t& start_time_ref = _display_state.hc ? _display_state.start_time_hc : _display_state.start_time_lc;

        current_time_ref = achieved_time + start_time_ref;

        // Update the interface state to match the controller
        _display_state.on = controller_status.state == CONTROLLER_STATE_EXPOSING;

        if (!_display_state.on)
            set_channel_power(_display_state.red ? CHANNEL_POWER_SAFE : 0, 0, 0);  // We've just transitioned from ON to OFF.  Set the red channel to the last value.
    }
}


void display_update()
{   
    if (FT8_busy())
        return;
 
    FT8_cmd_dl(CMD_DLSTART);
    FT8_cmd_dl(DL_CLEAR_RGB | BLACK);
    FT8_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    //FT8_cmd_dl(TAG(0));

    FT8_cmd_dl(TAG(1));
    FT8_cmd_dl(DL_COLOR_RGB | (_display_state.hc ? BLACK : RED));
    FT8_cmd_fgcolor(_display_state.hc ? RED : DARKRED);
    FT8_cmd_romfont(1, 32);
    FT8_cmd_button(15, 800-660-125, 200, 125, 1, FT8_OPT_FLAT, _display_state.hc ? "HC" : "LC");

    FT8_cmd_dl(TAG(2));
    FT8_cmd_dl(DL_COLOR_RGB | (_display_state.red ? BLACK : RED));
    FT8_cmd_fgcolor(_display_state.red ? RED : DARKRED);
    FT8_cmd_romfont(1, 32);
    FT8_cmd_button(480-200-15, 800-660-125, 200, 125, 1, FT8_OPT_FLAT, "R");

    FT8_cmd_dl(TAG(3));
    FT8_cmd_dl(DL_COLOR_RGB | (_display_state.on ? BLACK : RED));
    FT8_cmd_fgcolor(_display_state.on ? RED : DARKRED);
    FT8_cmd_romfont(1, 32);
    FT8_cmd_button(15, 800-15-125, 200, 125, 1, FT8_OPT_FLAT, _display_state.on ? "STOP" : "START");

    FT8_cmd_dl(TAG(4));
    FT8_cmd_dl(DL_COLOR_RGB | (!_display_state.on ? BLACK : RED));
    FT8_cmd_fgcolor(!_display_state.on ? RED : DARKRED);
    FT8_cmd_romfont(1, 32);
    FT8_cmd_button(480-200-15, 800-15-125, 200, 125, 1, FT8_OPT_FLAT, "RESET");

    FT8_cmd_dl(TAG(5));
    FT8_cmd_dl(DL_COLOR_RGB | RED);
    FT8_cmd_fgcolor(DARKRED);
    FT8_cmd_dial(480/2, 800/2+25, 140, FT8_OPT_FLAT, _display_state.dial_angle);

    char buf[32];
    uint16_t& set_time_ref = _display_state.hc ? _display_state.set_time_hc : _display_state.set_time_lc;
    uint16_t& current_time_ref = _display_state.hc ? _display_state.current_time_hc : _display_state.current_time_lc;

    FT8_cmd_dl(DL_COLOR_RGB | RED);
    FT8_cmd_romfont(1, 34);
    sprintf(&buf[0], "% 2d.%d/% 2d.%d", (current_time_ref >> 6) / 10, (current_time_ref >> 6) % 10, (set_time_ref >> 6) / 10, (set_time_ref >> 6) % 10);
    FT8_cmd_text(30, 160, 1, 0, &buf[0]);

    sprintf(&buf[0], "%d.%d / %d.%d", (_display_state.current_time_lc >> 6) / 10, (_display_state.current_time_lc >> 6) % 10, (_display_state.set_time_lc >> 6) / 10, (_display_state.set_time_lc >> 6) % 10);
    FT8_cmd_text(30, 590, 29, 0, "LC");
    FT8_cmd_text(75, 590, 29, 0, &buf[0]);
    sprintf(&buf[0], "%d.%d / %d.%d", (_display_state.current_time_hc >> 6) / 10, (_display_state.current_time_hc >> 6) % 10, (_display_state.set_time_hc >> 6) / 10, (_display_state.set_time_hc >> 6) % 10);
    FT8_cmd_text(30, 615, 29, 0, "HC");
    FT8_cmd_text(75, 615, 29, 0, &buf[0]);

    FT8_cmd_text(270, 590, 29, 0, _interface_status.is_controller_connected ? "CON" : "DIS");
//    FT8_cmd_text(340, 590, 29, 0, "SYNC");

    FT8_cmd_dl(DL_DISPLAY);
    FT8_cmd_dl(CMD_SWAP);

    FT8_cmd_execute();
}



void display_calibrate_touch()
{
     /* calibrate touch and displays values to screen */
    FT8_cmd_dl(CMD_DLSTART);
    FT8_cmd_dl(DL_CLEAR_RGB | BLACK);
    FT8_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    FT8_cmd_text((FT8_HSIZE/2), (FT8_VSIZE/2), 27, FT8_OPT_CENTER, "Please tap on the dot.");
    FT8_cmd_calibrate();
    FT8_cmd_dl(DL_DISPLAY);
    FT8_cmd_dl(CMD_SWAP);
    FT8_cmd_execute();

    uint32_t touch_a, touch_b, touch_c, touch_d, touch_e, touch_f;

    touch_a = FT8_memRead32(REG_TOUCH_TRANSFORM_A);
    touch_b = FT8_memRead32(REG_TOUCH_TRANSFORM_B);
    touch_c = FT8_memRead32(REG_TOUCH_TRANSFORM_C);
    touch_d = FT8_memRead32(REG_TOUCH_TRANSFORM_D);
    touch_e = FT8_memRead32(REG_TOUCH_TRANSFORM_E);
    touch_f = FT8_memRead32(REG_TOUCH_TRANSFORM_F);

    FT8_cmd_dl(CMD_DLSTART);
    FT8_cmd_dl(DL_CLEAR_RGB | BLACK);
    FT8_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    FT8_cmd_dl(TAG(0));

    FT8_cmd_text(5, 30, 28, 0, "TOUCH_TRANSFORM_A:");
    FT8_cmd_text(5, 50, 28, 0, "TOUCH_TRANSFORM_B:");
    FT8_cmd_text(5, 70, 28, 0, "TOUCH_TRANSFORM_C:");
    FT8_cmd_text(5, 90, 28, 0, "TOUCH_TRANSFORM_D:");
    FT8_cmd_text(5, 110, 28, 0, "TOUCH_TRANSFORM_E:");
    FT8_cmd_text(5, 130, 28, 0, "TOUCH_TRANSFORM_F:");

#ifdef FT8_81X_ENABLE
    FT8_cmd_setbase(16L); /* FT81x only */
#endif
    FT8_cmd_number(250, 30, 28, 0, touch_a);
    FT8_cmd_number(250, 50, 28, 0, touch_b);
    FT8_cmd_number(250, 70, 28, 0, touch_c);
    FT8_cmd_number(250, 90, 28, 0, touch_d);
    FT8_cmd_number(250, 110, 28, 0, touch_e);
    FT8_cmd_number(250, 130, 28, 0, touch_f);

    FT8_cmd_dl(DL_DISPLAY); /* instruct the graphics processor to show the list */
    FT8_cmd_dl(CMD_SWAP);   /* make this list active */
    FT8_cmd_execute();
    while(1);               // Display values until reset.
}

