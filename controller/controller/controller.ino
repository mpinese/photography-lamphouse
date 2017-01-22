#include <SPI.h> 
#include <ILI_SdSpi.h>
#include <ILI_SdFatConfig.h>
#include <ILI9341_due.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "ugui.h"
#ifdef __cplusplus
}
#endif

const uint8_t TFT_RST = PC13;
const uint8_t TFT_DC =  PC14;
const uint8_t TFT_CS =  PC15;

ILI9341_due tft = ILI9341_due(TFT_CS, TFT_DC, TFT_RST);
UG_GUI gui;

UG_COLOR RGB888_to_RGB565(UG_COLOR c888)
{
  uint16_t c565;

  c888 >>= 3;
  c565 = (c888 & 0x1F);
  c888 >>= 7;
  c565 |= (c888 & 0x3F) << 5;
  c888 >>= 9;
  c565 |= (c888 & 0x1F) << 11;

  return c565;
}

void ugui_driver_pset(UG_S16 x, UG_S16 y, UG_COLOR c)
{
  tft.drawPixel(x, y, RGB888_to_RGB565(c));
}

UG_RESULT ugui_driver_drawline(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
{
  tft.drawLine(x0, y0, x1, y1, RGB888_to_RGB565(c));
  return UG_RESULT_OK;
}

UG_RESULT ugui_driver_fillframe(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
{
  tft.fillRect(x0, y0, x1+1, y1+1, RGB888_to_RGB565(c));
  return UG_RESULT_OK;
}


void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(iliRotation270);  
  tft.fillScreen(0x071F);

  UG_Init(&gui, ugui_driver_pset, 320, 240);
  UG_DriverRegister(DRIVER_DRAW_LINE, (void*) ugui_driver_drawline);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void*) ugui_driver_fillframe);
  UG_DriverEnable(DRIVER_DRAW_LINE);
  UG_DriverEnable(DRIVER_FILL_FRAME);
}

void loop()
{
  delay(1000);
  UG_FillScreen(C_RED);
  delay(1000);
  UG_FillScreen(C_GREEN);
}

