#include <stdio.h>
#include <stdlib.h>
#include <ctrl.h>
#include <string.h>
#include "InputManager.h"



bool InputManager::CheckInput(){
	
	sceCtrlReadBufferPositive(0, &ctrlResult, 1);

	ctrl_l_pressed = (ctrlResult.buttons & SCE_CTRL_L) == SCE_CTRL_L;
	ctrl_r_pressed = (ctrlResult.buttons & SCE_CTRL_R) == SCE_CTRL_R;
	cross_pressed = (ctrlResult.buttons & SCE_CTRL_CROSS) == SCE_CTRL_CROSS;
	triangle_pressed = (ctrlResult.buttons & SCE_CTRL_TRIANGLE) == SCE_CTRL_TRIANGLE;
	circle_pressed = (ctrlResult.buttons & SCE_CTRL_CIRCLE) == SCE_CTRL_CIRCLE;
	arrow_left_pressed = (ctrlResult.buttons & SCE_CTRL_LEFT) == SCE_CTRL_LEFT;
	arrow_right_pressed = (ctrlResult.buttons & SCE_CTRL_RIGHT) == SCE_CTRL_RIGHT;

	return (ctrl_l_pressed||ctrl_r_pressed||cross_pressed||triangle_pressed||circle_pressed||arrow_left_pressed||arrow_right_pressed);
}