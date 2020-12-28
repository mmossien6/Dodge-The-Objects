#ifndef GAME_H
#define GAME_H

#define MAXAMOUNTOFOBJECTS 100

#include "gba.h"

struct Object {
	unsigned short color;
	int r;
	int c;
	int rdy;
	int cdx;
	int isDisplayed;
	int size;
};

struct PlayerSquare {
	unsigned short color;
	int r;
	int c;
	int sideLength;
	int rdy;
	int cdx;
};

struct State {
	int objectCount;
	struct Object objects[MAXAMOUNTOFOBJECTS];
	struct PlayerSquare player;
	int score; 
} CS, PS;

#endif
