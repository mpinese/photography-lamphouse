#include <SPI.h> 
#include <ILI_SdSpi.h>
#include <ILI_SdFatConfig.h>
#include <ILI9341_due.h>

const uint8_t TFT_RST = PC13;
const uint8_t TFT_DC =  PC14;
const uint8_t TFT_CS =  PC15;

ILI9341_due tft = ILI9341_due(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(iliRotation270);  
  tft.fillScreen(0x071F);
}

void loop()
{
 delay(5000);
}  

