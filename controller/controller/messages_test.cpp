#define _DEBUG

#include <stdint.h>

#include "messages.hpp"


const uint8_t LED_PIN = PB3;


MessageQueue<int16_t, 5> queue;


uint16_t simulated_measured_variable;


void callback_simdata(int16_t payload, unsigned long skew)
{
    Serial.println("+callback_simdata");
    simulated_measured_variable = random(65535);
    uint16_t delay_until_next = random(500);
    Serial.print("  Changing variable to: ");
    Serial.println(simulated_measured_variable);
    Serial.print("  Sending simulate repeat message ");
    bool status = queue.send(&callback_simdata, 0, delay_until_next);
    if (status == true)
        Serial.println("    Message sent successfully");
    else
        Serial.println("    Queue full");
    Serial.println("-callback_simdata");
}


void callback_poll_data(int16_t payload, unsigned long skew)
{
    bool status;
    Serial.println("+callback_poll_data");
    static uint16_t last_measurement = 0;

    if (last_measurement != simulated_measured_variable)
    {
        Serial.println("  Detected change.  Sending processing message.");
        status = queue.send(&callback_process_data, simulated_measured_variable, 0);
        if (status == true)
            Serial.println("    Message sent successfully");
        else
            Serial.println("    Queue full");
        last_measurement = simulated_measured_variable;
    }

    Serial.println("Sending poll repeat message")
    status = queue.send(&callback_poll_data, 0, 50);
    if (status == true)
        Serial.println("    Message sent successfully");
    else
        Serial.println("    Queue full");

    Serial.println("-callback_poll_data");
}


void callback_process_data(int16_t payload, unsigned long skew)
{
    Serial.print("+callback_process_data");
    Serial.print("  ");
    Serial.println(payload);
    Serial.print("-callback_process_data");
}


void setup()
{
    queue.send(&callback_simdata, 0, 0);
    queue.send(&callback_poll_data, 0, 0);
}


void loop()
{
    queue.run();
}
