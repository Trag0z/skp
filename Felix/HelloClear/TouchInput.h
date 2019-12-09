#ifndef _TOUCHINPUT_
#define _TOUCHINPUT_


#include <stdio.h>
#include <stdlib.h>
#include <libdbgfont.h>
#include <touch.h>


class TouchInput{

public:
	TouchInput();
	float* getInput();

private:

	char debug_text[512];

	SceTouchData result;
	float* DisplayCoord_To_NormalizedDC();
	float deviceCoordinates[2];
};


#endif
