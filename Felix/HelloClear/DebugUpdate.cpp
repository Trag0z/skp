#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctrl.h>

#include "DebugUpdate.h"

void DebugUpdate(InputManager *inputManager, CubeLogic *cubeLogic, 
	int &slicetype, int &sliceindex, bool &isRotating, bool &rotateLeft, bool &rotateRight){

	if(!inputManager->CheckInput()){
			inputManager->buttonPressed = false;
			return;								//beim auslagern zum return;
		}

		if(inputManager->buttonPressed)			//kein Knopf gedrückt
			return;
		
		inputManager->buttonPressed = true;


		if(inputManager->ctrl_l_pressed)
		{
			isRotating = true;
			rotateLeft = true;

			if(slicetype == 0){
				cubeLogic->cube_turn_left_x(sliceindex);
			}
			else if(slicetype == 1){
				cubeLogic->cube_turn_left_y(sliceindex);
			}
			else if(slicetype == 2){
				cubeLogic->cube_turn_left_z(sliceindex);
			}
			//printf("Cube Slice %i Dimension %i wurde nach links gedreht\n", slicetype, sliceindex);
			//cubeLogic->print_Cube();
			
		}

		if(inputManager->ctrl_r_pressed)
		{
			isRotating = true;
			rotateRight = true;

			if(slicetype == 0){
				cubeLogic->cube_turn_right_x(sliceindex);
			}
			else if(slicetype == 1){
				cubeLogic->cube_turn_right_y(sliceindex);
			}
			else if(slicetype == 2){
				cubeLogic->cube_turn_right_z(sliceindex);
			}
			//printf("Cube Slice %i Dimension %i wurde nach rechts gedreht\n", slicetype, sliceindex);
			//cubeLogic->print_Cube();
		}

		if(inputManager->circle_pressed){
			slicetype = 0;
			//printf("circle -> slicetype = 0\n");
			//printf("0 = x-Achse\n1 = y-Achse\n2 = z-Achse\n");
		}

		if(inputManager->cross_pressed){
			slicetype = 1;
			//printf("cross -> slicetype = 1\n");
			//printf("0 = x-Achse\n1 = y-Achse\n2 = z-Achse\n");
		}

		if(inputManager->triangle_pressed){
			slicetype = 2;
			//printf("triangle -> slicetype = 2\n");
			//printf("0 = x-Achse\n1 = y-Achse\n2 = z-Achse\n");
		}


		if(inputManager->arrow_left_pressed){
			sliceindex = (sliceindex + 2) %3;
			//printf("sliceindex %i\n", sliceindex);
		}

		if(inputManager->arrow_right_pressed){
			sliceindex = (sliceindex + 1) %3;
			//printf("sliceindex %i\n", sliceindex);
		}
}