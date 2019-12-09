#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CubeLogic.h"

CubeLogic::CubeLogic(){
	char tmp = 'A';
	for(int i = 0; i < 6; i++, tmp++){
		for(int j = 0; j < 3; j++){
			for(int k = 0; k < 3; k++){
				Cube[i][j][k] = tmp;
			}
		}
	}
}

uint32_t CubeLogic::getColor(int side, int row, int column) const{
	
	char color = Cube[side][row][column];

	switch(color){

		case 'A': return 0xffffffff;	//Weiß

		case 'B': return 0xffff0000;	//Blau

		case 'C': return 0xff008000;	//Grün

		case 'D': return 0xff00ffff;	//Gelb

		case 'E': return 0xff0000ff;	//Rot

		case 'F': return 0xff0066ff;	//Orange

		default: return 0;
	}
}

void CubeLogic::print_Cube() {
	for(int i = 0; i < 6; i++){
		for(int j = 0; j < 3; j++){
			for(int k = 0; k < 3; k++){
				printf(" %c ", Cube[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}
}


void CubeLogic::cube_turn_slice(int slicetype, int sliceindex, int rotationProgress){


	if(rotationProgress < 0){
		if( slicetype == 0){
			cube_turn_left_x(sliceindex);
		}
		else if(slicetype == 1){
			cube_turn_left_y(sliceindex);
		}
		else if(slicetype == 2){
			cube_turn_left_z(sliceindex);
		}
	}else if (rotationProgress > 0){
		if( slicetype == 0){
			cube_turn_right_x(sliceindex);
		}
		else if(slicetype == 1){
			cube_turn_right_y(sliceindex);
		}
		else if(slicetype == 2){
			cube_turn_right_z(sliceindex);
		}
	}

}


// Cube [side][row][column]
void CubeLogic::cube_turn_left_x(int row){	
	for(int i = 2; i > -1; i--){
		char tmp = Cube[0][row][i];
		Cube[0][row][i] = Cube[3][row][i];
		Cube[3][row][i] = Cube[2][row][i];
		Cube[2][row][i] = Cube[1][row][i];
		Cube[1][row][i] = tmp;
	}
	if(row==0){
		cube_turn_side(E);

	}

	if(row==2){
		cube_turn_side(F);
		cube_turn_side(F);
		cube_turn_side(F);
	}
}

void CubeLogic::cube_turn_right_x(int row){
	cube_turn_left_x(row);
	cube_turn_left_x(row);
	cube_turn_left_x(row);
}

// Cube [side][row][column]
void CubeLogic::cube_turn_right_y(int col){
	char tmp;

	if(col == 0){
		
		tmp = Cube[0][0][0];
		Cube[0][0][0] = Cube[4][0][0];
		Cube[4][0][0] = Cube[2][2][2];
		Cube[2][2][2] = Cube[5][0][0];
		Cube[5][0][0] = tmp;
		
		tmp = Cube[0][1][0];
		Cube[0][1][0] = Cube[4][1][0];
		Cube[4][1][0] = Cube[2][1][2];
		Cube[2][1][2] = Cube[5][1][0];
		Cube[5][1][0] = tmp;
		
		tmp = Cube[0][2][0];
		Cube[0][2][0] = Cube[4][2][0];
		Cube[4][2][0] = Cube[2][0][2];
		Cube[2][0][2] = Cube[5][2][0];
		Cube[5][2][0] = tmp;
		
		cube_turn_side(D);		//3x weil die Methode hier falsch herum drehen würde
		cube_turn_side(D);
		cube_turn_side(D);
	}

	else if(col == 1){

		tmp = Cube[0][0][1];
		Cube[0][0][1] = Cube[4][0][1];
		Cube[4][0][1] = Cube[2][2][1];
		Cube[2][2][1] = Cube[5][0][1];
		Cube[5][0][1] = tmp;

		tmp = Cube[0][1][1];
		Cube[0][1][1] = Cube[4][1][1];
		Cube[4][1][1] = Cube[2][1][1];
		Cube[2][1][1] = Cube[5][1][1];
		Cube[5][1][1] = tmp;

		tmp = Cube[0][2][1];
		Cube[0][2][1] = Cube[4][2][1];
		Cube[4][2][1] = Cube[2][0][1];
		Cube[2][0][1] = Cube[5][2][1];
		Cube[5][2][1] = tmp;
	}

	else if(col == 2){

		tmp = Cube[0][0][2];
		Cube[0][0][2] = Cube[4][0][2];
		Cube[4][0][2] = Cube[2][2][0];
		Cube[2][2][0] = Cube[5][0][2];
		Cube[5][0][2] = tmp;

		tmp = Cube[0][1][2];
		Cube[0][1][2] = Cube[4][1][2];
		Cube[4][1][2] = Cube[2][1][0];
		Cube[2][1][0] = Cube[5][1][2];
		Cube[5][1][2] = tmp;

		tmp = Cube[0][2][2];
		Cube[0][2][2] = Cube[4][2][2];
		Cube[4][2][2] = Cube[2][0][0];
		Cube[2][0][0] = Cube[5][2][2];
		Cube[5][2][2] = tmp;

		cube_turn_side(B);
	}
}

void CubeLogic::cube_turn_left_y(int col){
	cube_turn_right_y(col);
	cube_turn_right_y(col);
	cube_turn_right_y(col);
}

void CubeLogic::cube_turn_right_z(int row){
	char tmp;

	if(row == 0){

		tmp = Cube[4][2][0];
		Cube[4][2][0] = Cube[1][0][0];
		Cube[1][0][0] = Cube[5][0][2];
		Cube[5][0][2] = Cube[3][2][2];			
		Cube[3][2][2] = tmp;

		tmp = Cube[4][2][1];
		Cube[4][2][1] = Cube[1][1][0];
		Cube[1][1][0] = Cube[5][0][1];
		Cube[5][0][1] = Cube[3][1][2];
		Cube[3][1][2] = tmp;

		tmp = Cube[4][2][2];
		Cube[4][2][2] = Cube[1][2][0];
		Cube[1][2][0] = Cube[5][0][0];
		Cube[5][0][0] = Cube[3][0][2];
		Cube[3][0][2] = tmp;

		cube_turn_side(A);
	}

	else if(row == 1){
		tmp = Cube[4][1][0];
		Cube[4][1][0] = Cube[1][0][1];
		Cube[1][0][1] = Cube[5][1][2];
		Cube[5][1][2] = Cube[3][2][1];
		Cube[3][2][1] = tmp;

		tmp = Cube[4][1][1];
		Cube[4][1][1] = Cube[1][1][1];
		Cube[1][1][1] = Cube[5][1][1];
		Cube[5][1][1] = Cube[3][1][1];
		Cube[3][1][1] = tmp;

		tmp = Cube[4][1][2];
		Cube[4][1][2] = Cube[1][2][1];
		Cube[1][2][1] = Cube[5][1][0];
		Cube[5][1][0] = Cube[3][0][1];
		Cube[3][0][1] = tmp;
	}

	else if(row == 2){

		tmp = Cube[4][0][0];
		Cube[4][0][0] = Cube[1][0][2];
		Cube[1][0][2] = Cube[5][2][2];
		Cube[5][2][2] = Cube[3][2][0];
		Cube[3][2][0] = tmp;

		tmp = Cube[4][0][1];
		Cube[4][0][1] = Cube[1][1][2];
		Cube[1][1][2] = Cube[5][2][1];
		Cube[5][2][1] = Cube[3][1][0];
		Cube[3][1][0] = tmp;

		tmp = Cube[4][0][2];
		Cube[4][0][2] = Cube[1][2][2];
		Cube[1][2][2] = Cube[5][2][0];
		Cube[5][2][0] = Cube[3][0][0];
		Cube[3][0][0] = tmp;

		cube_turn_side(C);
		cube_turn_side(C);
		cube_turn_side(C);
	}
}

void CubeLogic::cube_turn_left_z(int row){
	cube_turn_right_z(row);
	cube_turn_right_z(row);
	cube_turn_right_z(row);
}
	
void CubeLogic::cube_turn_side(int side){

	/*Ecken*/
	char tmp = Cube[side][0][0];			//Feld 0 merken		 Würfelfelder
	Cube[side][0][0] = Cube[side][0][2];	//2 -> 0				0 1 2
	Cube[side][0][2] = Cube[side][2][2];	//8 -> 2				3 4 5
	Cube[side][2][2] = Cube[side][2][0];	//6 -> 8				6 7 8
	Cube[side][2][0] = tmp;					//0 -> 6


	/*Randmitten*/
	tmp = Cube[side][0][1];					//Feld 1 merken
	Cube[side][0][1] = Cube[side][1][2];	//5 -> 1
	Cube[side][1][2] =  Cube[side][2][1];	//7 -> 5
	Cube[side][2][1] =  Cube[side][1][0];	//3 -> 7
	Cube[side][1][0] =  tmp;				//1 -> 3
}

