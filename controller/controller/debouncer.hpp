#pragma once

#include <stdint.h>


template <uint8_t mask> class Debouncer
{
private:
	uint8_t buffer;
	uint8_t port;
	bool state;

public:
	Debouncer(uint8_t port)
	{
		port = port;
		buffer = 0;
	}

	bool poll()
	{
		buffer <<= 1;
		buffer |= digitalRead(port);

		if (!(~buffer & mask))
			state = true;
		else if (!(buffer & mask))
			state = false;

		return getstate();
	}

	bool getstate()	{ return state; }
};
