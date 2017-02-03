#pragma once


#include "ports.hpp"
#include "Adafruit_ILI9341_STM.h"

#define USE_FONT_8X14

extern "C" {
#include "ugui.h"
}


namespace Gui
{

void setup(void);
void send_display_diagnostics(void);

};
