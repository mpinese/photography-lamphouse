#pragma once

#include "ports.hpp"
#include "Adafruit_ILI9341_STM.h"

extern "C" {
#include "ugui.h"
}


namespace Display
{

void setup(void);
void send_display_diagnostics(void);

};
