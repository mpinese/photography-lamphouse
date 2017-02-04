#include "gui.hpp"
#include "display.hpp"


void Gui::draw_overlay()
{
    UG_DrawLine(0, 80, 320, 80, C_RED);
    UG_DrawLine(0, 179, 320, 179, C_RED);
    UG_DrawLine(0, 180, 320, 180, C_RED);
    UG_DrawLine(0, 181, 320, 181, C_RED);
    UG_DrawLine(159, 0, 159, 180, C_RED);
    UG_DrawLine(160, 0, 160, 180, C_RED);
    UG_DrawLine(161, 0, 161, 180, C_RED);
    UG_FontSelect(&FONT_12X20);
    UG_PutString(5, 0, "G");
    UG_PutString(165, 0, "B");
}


void Gui::draw_text()
{
    UG_FontSelect(&FONT_24X40);
    UG_PutString(0, 5, "-14.14");
    UG_PutString(160, 5, "  4.14");

    UG_FontSelect(&FONT_12X20);
    UG_PutString(10, 50, "-2.21");
    UG_PutString(10, 90, "10.04  4.10");
    UG_PutString(10, 120, "12.72 15.01");

    UG_PutString(170, 50, "-2.21");
    UG_PutString(170, 90, "10.04  4.10");
    UG_PutString(170, 120, "12.72 15.01");

    UG_PutString(5, 190, "f/5.6  s50");
    UG_PutString(165, 190, "W -0.0 -0.0");
    UG_PutString(5, 220, "1 2 3 4 5 6 7 8 9");	
}


void Gui::setup()
{
    UG_FillScreen(C_BLACK);
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_RED);

    draw_text();

    draw_overlay();
}