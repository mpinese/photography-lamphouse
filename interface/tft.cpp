/*
@file    tft.c
@brief   TFT handling functions for FT8xx_Test project, the layout is for 800x480 displays
@version 1.3
@date    2017-04-02
@author  Rudolph Riedel

@section History

1.0
- initial release

1.1
- added a simple .png image that is drawn beneath the clock just as example of how it could be done 

1.2
- replaced the clock with a scrolling line-strip
- moved the .png image over to the font-test section to make room
- added a scrolling bar display
- made scrolling depending on the state of the on/off button
- reduced the precision of VERTEX2F to 1 Pixel with VERTEXT_FORMAT() (FT81x only) to avoid some pointless "*16" multiplications

1.3
- adapted to release 3 of Ft8xx library with FT8_ and ft_ prefixes changed to FT8_
- removed "while (FT8_busy());" lines after FT8_cmd_execute() since it does that by itself now
- removed "FT8_cmd_execute();" line after FT8_cmd_loadimage(MEM_PIC1, FT8_OPT_NODL, pngpic, pngpic_size); as FT8_cmd_loadimage() executes itself now

 */ 

#include "tft.h"

#include "FT8_config.h"
#include "FT8.h"
#include "FT8_commands.h"


#define DARKRED   0x00400000


struct DisplayState
{
    bool red, green, blue, on;
};


SPIClass SPI_2(2);
DisplayState _state;


void initialise_display()
{
    _state.red = false;
    _state.green = false;
    _state.blue = false;
    _state.on = false;
    
    digitalWrite(FT8_CS, HIGH);
    pinMode(FT8_CS, OUTPUT);
    digitalWrite(FT8_PDN, HIGH);
    pinMode(FT8_PDN, OUTPUT);

    SPI_2.begin(); /* sets up the SPI to run in Mode 0 and 1 MHz */
    //SPI_2.setClockDivider(SPI_CLOCK_DIV2);

    /* send pre-recorded touch calibration values, RVT70 */
    FT8_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010ad7);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000000);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_C, 0xffe9d9a5);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000049);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010750);
    FT8_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfff85903);

#ifdef DEBUG
    FT8_memWrite8(REG_PWM_DUTY, 60);    // Bright backlight for testing
#else
    FT8_memWrite8(REG_PWM_DUTY, 5);  // Dim backlight for darkroom use
#endif

//    FT8_cmd_dl(CMD_DLSTART);
//
//    // Clear screen to black
//    FT8_cmd_dl(CLEAR_COLOR_RGB(0,0,0));
//    FT8_cmd_dl(CLEAR(1,1,1));            // Clear colour, stencils, tags
//
//    // Set colours to red on black
//    FT8_cmd_bgcolor(COLOR_RGB(0,0,0));
//    FT8_cmd_dl(DL_COLOR_RGB | COLOR_RGB(255,0,0));
//
//    // Reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default
//    FT8_cmd_dl(VERTEX_FORMAT(0));
//
//    FT8_cmd_text(150, 100, 23, 0, "Font 23");
//
//    FT8_cmd_execute();
}


void display_loop(void)
{
    static bool mode = false;

    if (mode)
        display_process_touch();
    else
        display_update();

    mode = !mode;
}


void display_process_touch()
{
    uint8_t tag;

    tag = FT8_get_touch_tag();

    switch(tag)
    {
        case 1: _state.red = !_state.red; break;
        case 2: _state.green = !_state.green; break;
        case 3: _state.blue = !_state.blue; break;
        case 4: _state.on = !_state.on; break;
    }
}


void display_update()
{
    FT8_cmd_dl(CMD_DLSTART);
    FT8_cmd_dl(DL_CLEAR_RGB | BLACK);
    FT8_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    FT8_cmd_dl(TAG(0));

    FT8_cmd_dl(TAG(1));
    FT8_cmd_dl(DL_COLOR_RGB | (_state.red ? BLACK : RED));
    FT8_cmd_fgcolor(_state.red ? RED : DARKRED);
    FT8_cmd_button(220, 100, 100, 50, 30, FT8_OPT_FLAT, "R");

    FT8_cmd_dl(TAG(2));
    FT8_cmd_dl(DL_COLOR_RGB | (_state.green ? BLACK : RED));
    FT8_cmd_fgcolor(_state.green ? RED : DARKRED);
    FT8_cmd_button(350, 100, 100, 50, 30, FT8_OPT_FLAT, "G");

    FT8_cmd_dl(TAG(3));
    FT8_cmd_dl(DL_COLOR_RGB | (_state.blue ? BLACK : RED));
    FT8_cmd_fgcolor(_state.blue ? RED : DARKRED);
    FT8_cmd_button(480, 100, 100, 50, 30, FT8_OPT_FLAT, "B");

    FT8_cmd_dl(TAG(4));
    FT8_cmd_dl(DL_COLOR_RGB | (_state.on ? BLACK : RED));
    FT8_cmd_fgcolor(_state.on ? RED : DARKRED);
    FT8_cmd_button(220, 180, 360, 50, 30, FT8_OPT_FLAT, "GO");
    
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

