// 320x240

#include <SPI.h>
#include <ILI9341_due.h>
//#include <ILI9341_due_gText.h>
//#include "fonts\Arial14.h"
//#include "fonts\Arial_bold_14.h"

#include "ugui.h"

#define PIN_TFT_DC  10
#define PIN_TFT_CS  8
#define PIN_TFT_RST 9

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9341_due Display = ILI9341_due(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
//ILI9341_due_gText Textbox(&Display);

UG_GUI gui;

typedef enum
{
  INTERFACE_GREEN,
  INTERFACE_BLUE,
} interface_channel_t;

typedef struct
{
  interface_channel_t interface_channel;
} state_t;

state_t State;


void ugDriver_SetPixel(UG_S16 x, UG_S16 y, UG_COLOR c)
{
  Display.drawPixel(x, y, c);
}

void ugDriver_DrawLine(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
{
  Display.drawLine(x0, y0, x1, y1, c);
}

void ugDriver_FillFrame(UG_S16 x0, UG_S16 y0, UG_S16 x1, UG_S16 y1, UG_COLOR c)
{
  Display.fillRect(x0, y0, x1, y1, c);
}


void setup()
{
  // Set up the initial state
  State.interface_channel = INTERFACE_GREEN;

  // Set up the TFT
  Display.begin();
  Display.setRotation(iliRotation270);
  Display.fillScreen(ILI9341_BLACK);
  //  Textbox.defineArea(0, 0, 320, 240);

  // Set up the uGUI driver
  Display.fillScreen(ILI9341_RED);
  UG_Init(&gui, &ugDriver_SetPixel, 320, 240);
  Display.fillScreen(ILI9341_GREEN);
//  UG_DriverRegister(DRIVER_DRAW_LINE, (void*) &ugDriver_DrawLine);
//  UG_DriverRegister(DRIVER_FILL_FRAME, (void*) &ugDriver_FillFrame);
//  UG_DriverEnable(DRIVER_DRAW_LINE);
//  UG_DriverEnable(DRIVER_FILL_FRAME);

  //UG_DrawFrame(0, 0, 100, 100, C_YELLOW);
}


//void showDisplayMain()
//{
//  Display.fillScreen(ILI9341_BLACK);
//
//  Textbox.selectFont(Arial_bold_14);
//
//  State.interface_channel == INTERFACE_GREEN ? Textbox.setFontColor(ILI9341_BLACK, ILI9341_RED) : Textbox.setFontColor(ILI9341_RED, ILI9341_BLACK);
//  Textbox.drawString("  GRN  ", 80-25, 0);
//
//  State.interface_channel == INTERFACE_BLUE ? Textbox.setFontColor(ILI9341_BLACK, ILI9341_RED) : Textbox.setFontColor(ILI9341_RED, ILI9341_BLACK);
//  Textbox.drawString("  BLU  ", 240-25, 0);
//
//  Display.drawFastVLine(160, 0, 240, ILI9341_RED);
//}


/* better rand() function */
UG_U32 randx( void )
{
   static UG_U32 z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
   UG_U32 b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4);
}


void loop()
{
  //  showDisplayMain();
  //  delay(1000);
  //  State.interface_channel == INTERFACE_GREEN ? State.interface_channel = INTERFACE_BLUE : State.interface_channel = INTERFACE_GREEN;
  UG_U32 xs, xe, ys, ye, c;
            xs = randx() % 320;
            xe = randx() % 320;
            ys = randx() % 240;
            ye = randx() % 240;
            c = randx() % 0xFFFFFF;
            //UG_FillFrame( xs, ys, xe, ye, c );
            Display.drawPixel(xs, ys, c);
  //UG_Update();
  delay(100);
}


/*
   Design thoughts.

   Need independent control and saving of G, B channels.
   Need push-button activation of either R channel, or W (all three channels).
   Need restart capability for B, G channels.
   Need log-scale entry for B, G channels.
   Need coarse control of brightness for B, G channels.

   Desire rotary encoder control for value entry.
   Desire fine control of brightness for all channels, as config option OK.
   Desire support for automated test patch creation in either B, G channels.
   Desire combined (R),B,G exposure support.

   Interface options:
   Unimodal desirable.
   Bimodal OK -- one for exposure, and one for step.
   Separate config also OK.
   Display is fixed at 320x240.  ~40 chars wide, 17 high (for font Arial14).

       0        1         2         3         4
       1234567890123456789012345678901234567890
      +----------------------------------------
    1 |
    2 |
    3 |
    4 |
    5 |
    6 |
    7 |
    8 |
    9 |
   10 |
   11 |
   12 |
   13 |
   14 |
   15 |
   16 |
   17 |

*/




























