#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_STM.h>

#include "SPI.h"


const uint8_t TFT_CS = PC15;
const uint8_t TFT_DC = PC14;
const uint8_t TFT_RST = PC13;


Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST);


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
}


void loop(void)
{
    delay(1000);
    send_display_diagnostics();
}

