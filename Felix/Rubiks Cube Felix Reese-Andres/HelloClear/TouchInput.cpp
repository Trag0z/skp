include"TouchInput.h"
#include <stdio.h>
#include <stdlib.h>
#include <libdbgfont.h>

TouchInput::TouchInput(){

	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
}

float* TouchInput::DisplayCoord_To_NormalizedDC(){

	sceTouchRead(SCE_TOUCH_PORT_FRONT, &result, 1);
	deviceCoordinates[0] =	2.0f * result.report[0].x / 1919.0f - 1.0f;
	deviceCoordinates[1] = - 2.0f * result.report[0].y / 1087.0f + 1.0f;

	return deviceCoordinates;
}

float* TouchInput::getInput(){

	sceTouchRead(SCE_TOUCH_PORT_FRONT, &result, 1);

	if(result.reportNum > 0){	

		DisplayCoord_To_NormalizedDC();
		//sprintf(debug_text, "X Pos: %f, Y Pos: %f, ID: %d", deviceCoordinates[0]/*result.report[0].x*/, deviceCoordinates[1]/*result.report[0].y*/, result.report[0].id);
	}

	else{
		deviceCoordinates[0] = 0.0f;
		deviceCoordinates[1] = 0.0f;
		sprintf(debug_text, "No touch");
	}
	//sceDbgFontPrint(20, 20, 0xffffffffff, (const SceChar8*)&(debug_text[0]));
	return deviceCoordinates;
}
