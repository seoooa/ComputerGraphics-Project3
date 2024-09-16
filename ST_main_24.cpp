//
//  main.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2024 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include "LoadScene.h"
#include "ST_DrawScene_24.h"

SCENE scene;

int main(int argc, char* argv[]) {
	
	read3DSceneFromFile(&scene);
	ST_drawScene_24(argc, argv);
	freeData(&scene);

	return 1;
}
 