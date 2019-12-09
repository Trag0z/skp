#ifndef _INPUTMANAGER_
#define _INPUTMANAGER_

class InputManager{

public:
	SceCtrlData ctrlResult;

	bool buttonPressed;

	bool ctrl_l_pressed;
	bool ctrl_r_pressed;
	bool cross_pressed;
	bool triangle_pressed; 
	bool circle_pressed;
	bool arrow_left_pressed;
	bool arrow_right_pressed;

	bool CheckInput();
};

#endif