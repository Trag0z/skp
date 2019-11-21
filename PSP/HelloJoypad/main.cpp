#include <stdio.h>
#include <ctrl.h>

float makeValue(SceUInt8 input) {
	float result = static_cast<float>(input);

	result = (result * 2.0f) / 255.0f - 1.0f;
	return result;
}

int main() {
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG_WIDE);

	bool wasXButton;
	while (true) {
		SceCtrlData ct;
		sceCtrlReadBufferPositive(0, &ct, 1);
		if ((ct.buttons & SCE_CTRL_CROSS) == 0) {
			wasXButton = false;
			continue;
		}

		if (wasXButton) {
			continue;
		}

		wasXButton = true;

		printf("Joystick position %f, %f\n", makeValue(ct.lx), makeValue(ct.ly));
	}

	return 0;
}