#pragma once

#include <stdint.h>
#include <wirish.h>


class Ports
{
public:
	static const int8_t CHANNEL_BLU = PA8;
	static const int8_t CHANNEL_GRN = PA9;
	static const int8_t CHANNEL_RED = PA10;
	static const int8_t BUZZER = PA15;
	static const int8_t ROTENC_A = PB3;
	static const int8_t ROTENC_B = PB4;
	static const int8_t TFT_RST = PC13;
	static const int8_t TFT_DC = PC14;
	static const int8_t TFT_CS = PC15;


	static void setup()
	{
	    pinMode(CHANNEL_RED, PWM);
	    pinMode(CHANNEL_GRN, PWM);
	    pinMode(CHANNEL_BLU, PWM);
	    pwmWrite(CHANNEL_RED, 0);
	    pwmWrite(CHANNEL_GRN, 0);
	    pwmWrite(CHANNEL_BLU, 0);
	    pinMode(BUZZER, OUTPUT);
	    pinMode(ROTENC_A, INPUT_PULLUP);
	    pinMode(ROTENC_B, INPUT_PULLUP);
	}

};
