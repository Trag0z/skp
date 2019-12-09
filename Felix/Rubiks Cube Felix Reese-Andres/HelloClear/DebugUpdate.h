#ifndef _DEBUGUPDATE_
#define _DEBUGUPDATE_

#include"CubeLogic.h"
#include"InputManager.h"

void DebugUpdate(InputManager *inputManager, CubeLogic *cubeLogic, 
	int &slicetype, int &sliceindex, bool &isRotating, bool &rotateLeft, bool &rotateRight);

#endif