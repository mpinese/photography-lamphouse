#include "ports.h"
#include "SPI.h"
#include "gui.hpp"
#include "exptimer.h"
#include "debouncer.hpp"

#define _TASK_TIMECRITICAL
#define _TASK_STATUS_REQUEST
#define _TASK_LTS_POINTER
#include <TaskScheduler.h>



Scheduler scheduler;
Gui <Ports::TFT_CS, Ports::TFT_DC, Ports::TFT_RST> gui;
//Gui gui;


// TASK_SECOND         Task interval of 1 second
// TASK_MINUTE         Task interval of 1 minute
// TASK_HOUR           Task interval of 1 hour
// TASK_FOREVER        Task number of iterations for infinite number of iterations
// TASK_ONCE           Task single iteration
// TASK_IMMEDIATE      Task interval for immediate execution
void task_monitor_encoder();
void task_monitor_switch();


Task tMonitorEncoder(10, TASK_FOREVER, &task_monitor_encoder, &scheduler);



void task_monitor_encoder()
{
    // State data, in bit mask.  Format:
    // Bit 7|6|5|4|3|2|1|0
    //     .|c|.|C|a|b|A|B
    // A: Last debounced state of the A quadrature signal
    // B: Last debounced state of the B quadrature signal
    // a: Holding location for new value of A
    // b: Holding location for new value of B
    // C: Was the last registered movement clockwise?
    // c: Holding location for new value of C
    // .: Unused
    static uint8_t state;
    static Debouncer<0b1111> port_A(ports.ROTENC_A);
    static Debouncer<0b1111> port_B(ports.ROTENC_B);

    bool A_hi, B_hi;

    A_hi = port_A.poll();
    B_hi = port_B.poll();

    // Load the new values of a and b into state
    state = (state & 0b10011) | (A_hi << 3) | (B_hi << 2);

    // Handle the state transitions
    // Encoder transitions (. is low, # is high, clockwise
    // rotation from L to R, two revolutions):
    // A  ..##..##
    // B  .##..##.
    //    01320132  <-- state & 0b11
    int8_t rot;     // Steps rotated.  Positive => clockwise

    // Initialise a default value corresponding to two steps
    // in the previously observed direction.  This is a 
    // reasonable default for ambiguous transitions, which 
    // are handled by bare break statements in the switch below.
    if (state & 0b10000)
        rot = +2;
    else
        rot = -2;

    switch (state & 0b01111)
    {
        case 0b0000:           return;   // No change in state
        case 0b0001: rot = -1; break;    // 1->0 CCW
        case 0b0010: rot = +1; break;    // 2->0 CW
        case 0b0011:           break;    // 3->0 Ambiguous
        case 0b0100: rot = +1; break;    // 0->1 CW
        case 0b0101:           return;   // No change in state
        case 0b0110:           break;    // 2->1 Ambiguous
        case 0b0111: rot = -1; break;    // 3->1 CCW
        case 0b1000: rot = -1; break;    // 0->2 CCW
        case 0b1001:           break;    // 1->2 Ambiguous
        case 0b1010:           return;   // No change in state
        case 0b1011: rot = +1; break;    // 3->2 CW
        case 0b1100:           break;    // 0->3 Ambiguous
        case 0b1101: rot = +1; break;    // 1->3 CW
        case 0b1110: rot = -1; break;    // 2->3 CCW
        case 0b1111:           return;   // No change in state
    }

    Serial.print("Rotary encoder signal: ");
    Serial.println(rot);
    // TODO: Dispatch tasks to handle the rotation.
//    if (rot > 0)
  //      else
    //rot ? x : y;

    // Update the state.
    state = ((state & 0b00001100) | ((rot > 0) << 7)) >> 2;
}


void setup()
{
    Serial.begin(115200);

    Serial.println("Initialising ports...");
    ports.setup();

    Serial.println("Initialising user interface...");
    gui.setup();

    Serial.println("Initialising scheduler...");
    tMonitorEncoder.enable();



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
