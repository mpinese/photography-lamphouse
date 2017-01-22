#include <Adafruit_ILI9341_STM.h>

#define  USE_FONT_12X20

extern "C" {
  #include "uGUI/ugui.h"
}

#include "SPI.h"

//#include "exptimer.h"


const uint8_t TFT_CS = PC15;
const uint8_t TFT_DC = PC14;
const uint8_t TFT_RST = PC13;


Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST);
UG_GUI gui;


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


void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);

  UG_Init(&gui, ugui_driver_pset, 320, 240);
  UG_DriverRegister(DRIVER_DRAW_LINE, (void*) ugui_driver_drawline);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*) ugui_driver_fillframe);
  UG_DriverEnable(DRIVER_DRAW_LINE);
  UG_DriverEnable(DRIVER_FILL_FRAME);

  UG_FillScreen(C_BLACK);
  UG_FontSelect(&FONT_12X20);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_RED);

  UG_PutString(60, 0, "GRN");
  UG_PutString(5, 26, "E  4.1  3.9");   // E_e
  UG_PutString(5, 26*2, "t 15.0 12.7"); // t
  UG_PutString(5, 26*3, "P -2.2");      // Phi_e

  UG_PutString(160+60, 0, "BLU");
  UG_PutString(165, 26, "E  4.1  3.9");
  UG_PutString(165, 26*2, "t 15.0 12.7");
  UG_PutString(165, 26*3, "P -2.2");

  UG_PutString(5, 40*4+10, "f/5.6  s50");
  UG_PutString(165, 40*4+10, "W -0.0 -0.0");

  UG_DrawLine(0, 40*4, 320, 40*4, C_RED);
  UG_DrawLine(160, 0, 160, 40*4, C_RED);
}


void loop(void)
{
//    send_display_diagnostics();
//    delay(1000);
//    UG_FillScreen(C_WHITE);
//
//    for (uint16_t i = 0; i < 240; i++)
//    {      
//      UG_DrawLine(0, i, 320, i, C_BLACK);
//      delay(10);
//    }
//
//    delay(1000);    
//    UG_FillScreen(C_WHITE);
//
//    for (uint16_t i = 0; i < 320; i++)
//    {      
//      UG_DrawLine(i, 0, i, 240, C_BLACK);
//      delay(10);
//    }
//
//    UG_FillScreen(C_WHITE);
}

