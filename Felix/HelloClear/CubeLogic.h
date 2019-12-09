#ifndef _CUBELOGIC_H_
#define _CUBELOGIC_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sceerror.h>

#include <gxm.h>
#include <kernel.h>
#include <ctrl.h>
#include <display.h>
#include <libdbg.h>

/*
Seiten des Würfels
	|0 1 2
____|_____
0	|0 1 2
1	|3 4 5
2	|6 7 8

Cube [0][0][0] ist Seite A Feld A0 usw.
Cube [0][2][1] ist Seite A Feld A7

Cube [side][row][column]
*/

class CubeLogic{

public:
	

	CubeLogic();

	uint32_t getColor(int side, int row, int column) const;

	/*
	
	A -> Weiß
	B -> Blau
	C -> Grün
	D -> Gelb
	E -> Rot
	F -> Orange
	
	*/

	enum Side {A, B, C, D, E, F};

	void print_Cube();

	void cube_turn_slice(int slicetype, int sliceindex, int rotationProgress);

	void cube_turn_right_x(int row);
	void cube_turn_right_y(int col);
	void cube_turn_right_z(int row);

	void cube_turn_left_x(int row);
	void cube_turn_left_y(int col);
	void cube_turn_left_z(int row);

	void cube_turn_side(int side);

private:
	char Cube[6][3][3];
};


#endif