#include "ports.hpp"
#include "SPI.h"
#include "gui.hpp"
#include "display.hpp"
#include "exptimer.h"
#include "interface.hpp"

#define _TASK_TIMECRITICAL
#include <TaskScheduler.h>


// TASK_SECOND         Task interval of 1 second
// TASK_MINUTE         Task interval of 1 minute
// TASK_HOUR           Task interval of 1 hour
// TASK_FOREVER        Task number of iterations for infinite number of iterations
// TASK_ONCE           Task single iteration
// TASK_IMMEDIATE      Task interval for immediate execution
Scheduler scheduler;


ExposureSettings exposure;
Gui gui;


void task_simulate_scroll();


Task tMonitorEncoder(10, TASK_FOREVER, &task_monitor_encoder, &scheduler);
Task tMonitorButtons(10, TASK_FOREVER, &task_monitor_buttons, &scheduler);
Task tSimulateScroll(1000, TASK_FOREVER, &task_simulate_scroll, &scheduler);



void task_simulate_scroll()
{
    
}



void setup()
{
    Serial.begin(115200);

    Serial.println("Initialising ports...");
    Ports::setup();

    Serial.println("Initialising display...");
    Display::setup();

    Serial.println("Initialising GUI...");
    gui.setup();

    Serial.println("Initialising scheduler...");
    tMonitorEncoder.enable();
}


void loop(void)
{
    scheduler.execute();
}
