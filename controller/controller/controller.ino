#include <SPI.h> 
#include <ILI_SdSpi.h>
#include <ILI_SdFatConfig.h>
#include <ILI9341_due_gText.h>
#include <ILI9341_due.h>
#include "fonts\Arial14.h"

const uint8_t TFT_RST = PC13;
const uint8_t TFT_DC =  PC14;
const uint8_t TFT_CS =  PC15;

ILI9341_due tft = ILI9341_due(TFT_CS, TFT_DC, TFT_RST);
ILI9341_due_gText t1(&tft);

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(iliRotation270);  
  tft.fillScreen(ILI9341_BLACK);
  t1.defineArea(0, 0, 320, 240);
  t1.selectFont(Arial14);
}

void loop()
{
  delay(1000);
}

