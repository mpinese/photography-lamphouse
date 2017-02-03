#pragma once

#include "Adafruit_ILI9341_STM.h"

#define USE_FONT_12X20

extern "C" {
#include "ugui.h"
}


template <int8_t CS, int8_t DC, int8_t RST> class Gui
{
private:
	Adafruit_ILI9341_STM tft;
	UG_GUI ugui;

public:
	Gui() : tft(CS, DC, RST) { }


	void setup()
	{
	    tft.begin();
	    tft.setRotation(3);
	    send_display_diagnostics();

	    UG_Init(&ugui, (void (*)(UG_S16, UG_S16, UG_COLOR)) &Gui<CS, DC, RST>::ugui_driver_pset, 320, 240);
	    UG_DriverRegister(DRIVER_DRAW_LINE, (void *) &Gui<CS, DC, RST>::ugui_driver_drawline);
	    UG_DriverRegister(DRIVER_FILL_FRAME, (void *) &Gui<CS, DC, RST>::ugui_driver_fillframe);
	    UG_DriverEnable(DRIVER_DRAW_LINE);
	    UG_DriverEnable(DRIVER_FILL_FRAME);
	}


	void ugui_driver_pset(UG_S16 x, UG_S16 y, UG_COLOR c)
	{
	    tft.drawPixel(x, y, c);
	}


	UG_RESULT ugui_driver_drawline(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
	{
	    tft.drawLine(x0, y0, x1, y1, c);
	    return UG_RESULT_OK;
	}


	UG_RESULT ugui_driver_fillframe(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
	{
	    if (x0 == 0 && y0 == 0 && x1 == 319 && y1 == 239)
	        tft.fillScreen(c);
	    else
	        tft.fillRect(x0, y0, x1+1, y1+1, c);
	    return UG_RESULT_OK;
	}


	void send_display_diagnostics()
	{
	    Serial.println("ILI9341 diagnostics:");
	    Serial.print("Display Power Mode: 0x");
	    Serial.println(tft.readcommand8(ILI9341_RDMODE), HEX);
	    Serial.print("MADCTL Mode:        0x");
	    Serial.println(tft.readcommand8(ILI9341_RDMADCTL), HEX);
	    Serial.print("Pixel Format:       0x");
	    Serial.println(tft.readcommand8(ILI9341_RDPIXFMT), HEX);
	    Serial.print("Image Format:       0x");
	    Serial.println(tft.readcommand8(ILI9341_RDIMGFMT), HEX);
	    Serial.print("Self Diagnostic:    0x");
	    Serial.println(tft.readcommand8(ILI9341_RDSELFDIAG), HEX);     
	}

};
