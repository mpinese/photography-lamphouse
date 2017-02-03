#include "ports.hpp"
#include "SPI.h"
#include "gui.hpp"
#include "exptimer.h"
#include "interface.hpp"

#define _TASK_TIMECRITICAL
#define _TASK_STATUS_REQUEST
#define _TASK_LTS_POINTER
#include <TaskScheduler.h>


// TASK_SECOND         Task interval of 1 second
// TASK_MINUTE         Task interval of 1 minute
// TASK_HOUR           Task interval of 1 hour
// TASK_FOREVER        Task number of iterations for infinite number of iterations
// TASK_ONCE           Task single iteration
// TASK_IMMEDIATE      Task interval for immediate execution
Scheduler scheduler;


Task tMonitorEncoder(10, TASK_FOREVER, &task_monitor_encoder, &scheduler);
Task tMonitorButtons(10, TASK_FOREVER, &task_monitor_buttons, &scheduler);


void setup()
{
    Serial.begin(115200);

    Serial.println("Initialising ports...");
    Ports::setup();

    Serial.println("Initialising user interface...");
    Gui::setup();

    Serial.println("Initialising scheduler...");
    tMonitorEncoder.enable();


    UG_FillScreen(C_BLACK);
    UG_FontSelect(&FONT_8X14);
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_RED);

    UG_PutString(60, 0, "GRN");
    UG_PutString(5, 26, "H  4.14   3.92");     // H_e
    UG_PutString(5, 26*2, "t 15.01  12.72");       // t
    UG_PutString(5, 26*3, "P -2.21");            // Phi_e

    UG_PutString(160+60, 0, "BLU");
    UG_PutString(165, 26, "H  4.14   3.92");
    UG_PutString(165, 26*2, "t 15.01  12.72");
    UG_PutString(165, 26*3, "P -2.21");

    UG_PutString(5, 40*4+10, "f/5.6    s50");
    UG_PutString(165, 40*4+10, "W -0.0 -0.0");

    UG_DrawLine(0, 40*4, 320, 40*4, C_RED);
    UG_DrawLine(160, 0, 160, 40*4, C_RED);
}


void loop(void)
{
    scheduler.execute();
}
