#include <wirish.h>

#include "interface.hpp"
#include "ports.hpp"
#include "debouncer.hpp"


Debouncer<0b1111> debounced_rotenc_A(Ports::ROTENC_A);
Debouncer<0b1111> debounced_rotenc_B(Ports::ROTENC_B);


static int8_t decode_rotary_encoder(uint8_t state)
{
    // Given the state variable from task_monitor_encoder, 
    // identify the state transition that has occurred.
    // Encoder transitions (. is low, # is high, clockwise
    // rotation from L to R, two revolutions):
    // A  ..##..##
    // B  .##..##.
    //    01320132  <-- state & 0b11
    int8_t rot;     // Steps rotated.  Positive => clockwise

    // This state table associates values of state & 0b01111
    // with rotation direction.  Entries are as follows:
    //   Value   Meaning
    //       0   No change in state (no rotation)
    //      -1   Counter-clockwise rotation
    //      +1   Clockwise rotation
    //      +2   Ambiguous (two stops rotated).  Use last observed
    //           direction as a guide.
    const int8_t state_table[] = {
    // Value    // State   Meaning
         0,     // 0b0000  No change in state
        -1,     // 0b0001  1->0 CCW
        +1,     // 0b0010  2->0 CW
        +2,     // 0b0011  3->0 Ambiguous
        +1,     // 0b0100  0->1 CW
         0,     // 0b0101  No change in state
        +2,     // 0b0110  2->1 Ambiguous
        -1,     // 0b0111  3->1 CCW
        -1,     // 0b1000  0->2 CCW
        +2,     // 0b1001  1->2 Ambiguous
         0,     // 0b1010  No change in state
        +1,     // 0b1011  3->2 CW
        +2,     // 0b1100  0->3 Ambiguous
        +1,     // 0b1101  1->3 CW
        -1,     // 0b1110  2->3 CCW
         0      // 0b1111  No change in state
    };

    rot = state_table[state & 0b01111];

    if (rot != 2)
        return rot;

    // rot == 2 (ie the movement direction is unknown).  
    // Use two steps in the last observed direction.
    if (state & 0b10000)
        return +2;
    else
        return -2;
}


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

    bool A_hi, B_hi;

    A_hi = debounced_rotenc_A.poll();
    B_hi = debounced_rotenc_B.poll();

    // Load the new values of a and b into state
    state = (state & 0b10011) | (A_hi << 3) | (B_hi << 2);

    int8_t rot = decode_rotary_encoder(state);
    if (rot == 0)
        return;

    // TODO: Dispatch tasks to handle the rotation.

    // Update the state.
    state = ((state & 0b00001100) | ((rot > 0) << 7)) >> 2;
}


void task_monitor_buttons()
{

}
