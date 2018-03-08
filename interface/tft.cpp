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


#include "FT8.h"
#include "FT8_commands.h"

#define PROGMEM

/* memory-map defines */
#define MEM_DL_STATIC 0x000ff000 /* start-address of the static part of the display-list, upper 4k of gfx-mem */

uint32_t num_dl_static;

uint8_t tft_active = 0;


#define LAYOUT_W 266
#define LAYOUT_X1 0
#define LAYOUT_X2 267
#define LAYOUT_X3 534
#define LAYOUT_H 379
#define LAYOUT_Y1 101


void initStaticBackground()
{
	FT8_cmd_dl(CMD_DLSTART);

    // Clear screen to black
    FT8_cmd_dl(CLEAR_COLOR_RGB(0,0,0));
    FT8_cmd_dl(CLEAR(1,1,1));            // Clear colour, stencils, tags

    // Set colours to red on black
    FT8_cmd_bgcolor(COLOR_RGB(0,0,0));
    FT8_cmd_dl(DL_COLOR_RGB | COLOR_RGB(255,0,0));

    // Reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default
    FT8_cmd_dl(VERTEX_FORMAT(0));

    // Text
	FT8_cmd_text(LAYOUT_X2 + (LAYOUT_W / 2), 40, 29, FT8_OPT_CENTERX, "FT81x Test");

	FT8_cmd_dl(DL_COLOR_RGB | COLOR_RGB(255,0,0));

	FT8_cmd_text(LAYOUT_X3+150,LAYOUT_Y1+20, 16, 0, "Font 16");
	FT8_cmd_text(LAYOUT_X3+150,LAYOUT_Y1+30, 18, 0, "Font 18");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+60, 23, 0, "Font 23");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+80, 24, 0, "Font 24");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+100, 25, 0, "Font 25");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+136, 26, 0, "Font 26");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+150, 27, 0, "Font 27");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+165, 28, 0, "Font 28");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+185, 29, 0, "Font 29");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+205, 30, 0, "Font 30");
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+230, 31, 0, "Font 31");

#ifdef FT8_81X_ENABLE
	FT8_cmd_romfont(1,32);
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+260, 1, 0, "Font 32");
	FT8_cmd_romfont(1,33);
	FT8_cmd_text(LAYOUT_X3+20,LAYOUT_Y1+300, 1, 0, "Font 33");
	FT8_cmd_romfont(1,34);
	FT8_cmd_text(LAYOUT_X3+150,LAYOUT_Y1+140, 1, 0, "34");
#endif

	FT8_cmd_execute();
	
	num_dl_static = FT8_memRead16(REG_CMD_DL);

	FT8_cmd_memcpy(MEM_DL_STATIC, FT8_RAM_DL, num_dl_static);
	FT8_cmd_execute();
}


/*
	dynamic portion of display-handling, meant to be called every 10ms
	divided into two sections:
		- handling of touch-events and variables
		- sending a new display-list to the FT8xx
*/
void TFT_loop(void)
{
    static uint8_t delay;
    static uint8_t tag = 0;
    static uint16_t toggle_state = 0;
    static uint8_t toggle_lock = 0;

	static uint8_t track = 0;
	static uint32_t tracker_val = 0;
	static uint16_t slider10_val = 30;

	static uint16_t alive_counter = 0;

    uint32_t calc;

	static uint8_t u_unit1 = 0;
	
	uint8_t counter;
	static uint8_t strip_offset = 0;
	static uint8_t strip_delay = 0;

	if(tft_active != 0)
	{
	  	switch(delay)
		{
			case 0:
				if(FT8_busy() == 0) /* is the FT8xx executing the display list?  note: may be working as intended - or not all to indicate the FT8xx is still up and running */
				{
					alive_counter++;
				}

				calc = FT8_get_touch_tag();
				tag = calc;

				switch(tag)
				{
					case 0:
						toggle_lock = 0;
						if(track != 0 )
						{
							FT8_cmd_track(0, 0, 0, 0, 0); /* stop tracking */
							FT8_cmd_execute();
							track = 0;
						}
						break;

					case 8: /* master-switch on top as on/off toggle-button */
						if(toggle_lock == 0)
						{
							toggle_lock = 42;
							if(toggle_state == 0)
							{
								toggle_state = 0xffff;
							}
							else
							{
								toggle_state = 0;
								slider10_val = 30;
							}
						}
						break;
						
					case 11:
						if(toggle_state != 0) /* only execute when master-switch is set to on */
						{
							slider10_val = 30;
						}
						break;

					case 10:
						if(track == 0)
						{
							if(toggle_state != 0)
							{
								FT8_cmd_track(LAYOUT_X1+(5*(LAYOUT_W/6))-25,LAYOUT_Y1+40,50,300,10); /* start tracking */
								FT8_cmd_execute();
								track = 10;
							}
						}
						else
						{
							tracker_val = FT8_memRead32(REG_TRACKER);
							if((tracker_val & 0xff) == 10)
							{
								tracker_val = tracker_val >> 14; /* cutoff lower 8 bit and six lsb more */
								tracker_val = tracker_val / 4369; /* limit value to 60 max */
								slider10_val = tracker_val;
							}
						}
						break;
				}

				u_unit1 = 70 - slider10_val;

				break;

			case 1:
				FT8_cmd_dl(CMD_DLSTART); /* start the display list */
				FT8_cmd_dl(DL_CLEAR_RGB | WHITE); /* set the default clear color to white */
				FT8_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */
				FT8_cmd_dl(TAG(0));

				FT8_cmd_append(MEM_DL_STATIC, num_dl_static); /* insert static part of display-list from copy in gfx-mem */

				/* unit 1*/
				FT8_cmd_number(LAYOUT_X1+175+20+1+32,LAYOUT_Y1+6, 26, FT8_OPT_RIGHTX, u_unit1); /* DC/DC Leistung */
				FT8_cmd_number(LAYOUT_X1+40+11+1+20+1+9+1+40, LAYOUT_Y1+45, 27, FT8_OPT_RIGHTX, 43); /* U_X1 */
				FT8_cmd_number(LAYOUT_X1+40+11+1+20+1+9+1+40, LAYOUT_Y1+45+30, 27, FT8_OPT_RIGHTX, 12); /* I_X1 */
				FT8_cmd_number(LAYOUT_X1+40+11+1+20+1+9+1+40, LAYOUT_Y1+280, 27, FT8_OPT_RIGHTX, 18); /* U_X2 */
				FT8_cmd_number(LAYOUT_X1+40+11+1+20+1+9+1+40, LAYOUT_Y1+280+30, 27, FT8_OPT_RIGHTX, 7); /* I_X2 */

				FT8_cmd_dl(TAG(8));
				FT8_cmd_toggle(50, 45, 90, 29, 0, toggle_state, "off" "\xff" "on");
				
				FT8_cmd_dl(DL_COLOR_RGB | 0x00c0c0c0);
				
				FT8_cmd_dl(TAG(10));
				FT8_cmd_slider(LAYOUT_X1+(5*(LAYOUT_W/6))-20,LAYOUT_Y1+40,20,300,0,slider10_val,60);

				/* unit 2*/
				FT8_cmd_dl(DL_COLOR_RGB | BLACK);
				FT8_cmd_dl(LINE_WIDTH(1 * 16));
	
				FT8_cmd_dl(DL_BEGIN | FT8_LINE_STRIP);


				if(toggle_state != 0) /* only execute when master-switch is set to on */
				{
					strip_delay++;
					if(strip_delay > 1)
					{
						strip_offset++;
						strip_offset &= 0x3f;
						strip_delay = 0;
					}
				}

				FT8_cmd_dl(DL_END);

				FT8_cmd_dl(DL_COLOR_RGB | 0x00aaaaaa);

				FT8_cmd_dl(DL_BEGIN | FT8_RECTS);
				
				FT8_cmd_dl(DL_END);

				FT8_cmd_dl(DL_DISPLAY);	/* instruct the graphics processor to show the list */
				FT8_cmd_dl(CMD_SWAP); /* make this list active */

				FT8_cmd_execute();
				break;
		}

		delay++;
		delay &= 0x01; /* -> delay toggles between 0 and 1 */
	}
}




void TFT_calibrate()
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


void TFT_init(void)
{
    if(FT8_init() != 0)
    {
        tft_active = 1;

        /* send pre-recorded touch calibration values, RVT70 */
        FT8_memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010ad7);
        FT8_memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000000);
        FT8_memWrite32(REG_TOUCH_TRANSFORM_C, 0xffe9d9a5);
        FT8_memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000049);
        FT8_memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010750);
        FT8_memWrite32(REG_TOUCH_TRANSFORM_F, 0xfff85903);

        initStaticBackground();

        // Dim backlight for darkroom use
        // FT8_memWrite8(REG_PWM_DUTY, 5);
        // Bright backlight for testing
        FT8_memWrite8(REG_PWM_DUTY, 60);
    }
}

