#define _TASK_TIMECRITICAL
#define _TASK_STATUS_REQUEST
#define _TASK_LTS_POINTER
#include <TaskScheduler.h>

#include "Adafruit_ILI9341_STM.h"

#define USE_FONT_12X20

extern "C" {
#include "ugui.h"
}

#include "SPI.h"

#include "exptimer.h"


const uint8_t TFT_CS = PC15;
const uint8_t TFT_DC = PC14;
const uint8_t TFT_RST = PC13;
const uint8_t CHANNEL_RED = PA10;
const uint8_t CHANNEL_GRN = PA9;
const uint8_t CHANNEL_BLU = PA8;
const uint8_t ROTENC_A = ;
const uint8_t ROTENC_B = ;
const uint8_t ROTENC_SW = ;
const uint8_t BUZZER = ;



Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST);
UG_GUI gui;


Scheduler scheduler;

// 
void task_monitor_encoder();

Task tMonitorEncoder(10, TASK_FOREVER, &task_monitor_encoder, &scheduler);




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


void setup()
{
    Serial.begin(115200);

    Serial.println("Initialising pins...")
    pinMode(CHANNEL_RED, PWM);
    pinMode(CHANNEL_GRN, PWM);
    pinMode(CHANNEL_BLU, PWM);
    pwmWrite(CHANNEL_RED, 0);
    pwmWrite(CHANNEL_GRN, 0);
    pwmWrite(CHANNEL_BLU, 0);
    pinMode(BUZZER, OUTPUT);
    pinMode(ROTENC_A, INPUT_PULLUP);
    pinMode(ROTENC_B, INPUT_PULLUP);
    pinMode(ROTENC_SW, INPUT_PULLUP);
    
    Serial.println("Initialising TFT driver...")
    tft.begin();
    tft.setRotation(3);
    send_display_diagnostics();

    Serial.println("Initialising GUI drivers...")
    UG_Init(&gui, ugui_driver_pset, 320, 240);
    UG_DriverRegister(DRIVER_DRAW_LINE, (void*) ugui_driver_drawline);
    UG_DriverRegister(DRIVER_FILL_FRAME, (void*) ugui_driver_fillframe);
    UG_DriverEnable(DRIVER_DRAW_LINE);
    UG_DriverEnable(DRIVER_FILL_FRAME);

    Serial.println("Initialising scheduler...")
    scheduler.
    scheduler.enableAll(true);
//    scheduler.init();  Not required -- done in constructor
// TASK_SECOND         Task interval of 1 second
// TASK_MINUTE         Task interval of 1 minute
// TASK_HOUR           Task interval of 1 hour
// TASK_FOREVER        Task number of iterations for infinite number of iterations
// TASK_ONCE           Task single iteration
// TASK_IMMEDIATE      Task interval for immediate execution


    UG_FillScreen(C_BLACK);
    UG_FontSelect(&FONT_12X20);
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_RED);

    UG_PutString(60, 0, "GRN");
    UG_PutString(5, 26, "H    4.1    3.9");     // H_e
    UG_PutString(5, 26*2, "t 15.0 12.7"); // t
    UG_PutString(5, 26*3, "P -2.2");            // Phi_e

    UG_PutString(160+60, 0, "BLU");
    UG_PutString(165, 26, "H    4.1    3.9");
    UG_PutString(165, 26*2, "t 15.0 12.7");
    UG_PutString(165, 26*3, "P -2.2");

    UG_PutString(5, 40*4+10, "f/5.6    s50");
    UG_PutString(165, 40*4+10, "W -0.0 -0.0");

    UG_DrawLine(0, 40*4, 320, 40*4, C_RED);
    UG_DrawLine(160, 0, 160, 40*4, C_RED);
}


void loop(void)
{
    scheduler.execute();
}
