#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "gba.h"
#include "startscreen.h"
#include "gamescreen.h"
#include "losescreen.h"

typedef enum {
	START,
	PLAY,
	LOSE,
} GBAState;

int main(void) {
	u32 currentButtons, previousButtons;
	currentButtons = previousButtons = BUTTONS;

	GBAState state = START;

	REG_DISPCNT = MODE3 | BG2_ENABLE;
	videoBuffer[1208] = 0x7fff;

	drawFullScreenImageDMA(startscreen);
	
	while (1) {
		PS = CS;

		previousButtons = currentButtons;
		currentButtons = BUTTONS;

		switch (state) {
		case START:

			CS.player.r = 60;
			CS.player.c = 155;
			CS.player.color = BLUE;
			CS.player.cdx = CS.player.rdy = 2;
			CS.player.sideLength = 5;
			CS.score = CS.objectCount = 0;

			//reused from lecture code
			int deltas[] = { -1, -1, -1, 1, 1, 1 };
			int deltas2[] = { -1, -2, 1, 2, 1, 1, -1, -1, -1, 0 };

			int ndeltas = sizeof(deltas) / sizeof(deltas[0]);
			int ndeltas2 = sizeof(deltas2) / sizeof(deltas2[0]);

			unsigned short colors[] = { RED, BLUE, GREEN, WHITE, CYAN, YELLOW, MAGENTA };
			int ncolors = sizeof(colors) / sizeof(colors[0]);

			for (int i = 0; i < MAXAMOUNTOFOBJECTS; i++) {
				CS.objects[i].r = rand() % 100;
				CS.objects[i].c = rand() % 120 + 80;
				CS.objects[i].rdy = deltas[rand() % ndeltas];
				CS.objects[i].cdx = deltas2[rand() % ndeltas2];

				CS.objects[i].size = 3;
				CS.objects[i].color = colors[i % ncolors];
			}

			int scoreTimerCounter = 0;
			int scoreInc = 30;
			int objectInc = 100;

			if (KEY_DOWN(BUTTON_START, BUTTONS)) {
				fillScreenDMA(0);
				state = PLAY;
			}
			break;

		case PLAY:
			scoreTimerCounter++;
			
			if (scoreTimerCounter % scoreInc == 0) {
				CS.score++;
			}
			if (CS.objectCount < MAXAMOUNTOFOBJECTS && scoreTimerCounter % objectInc == 0) {
				CS.objectCount++;
			}
			if (CS.score % 10 == 0 && scoreInc > 10) {
				scoreInc -= 5;
				if (objectInc > 10) {
					objectInc -= 15;
				}
			}

			drawImageDMA(4, 0, GAMESCREEN_WIDTH, GAMESCREEN_HEIGHT, gamescreen); 
			drawRectDMA(130, 80, 160, 4, WHITE);
			drawString(140, 45, "Score: ", GREEN);
			drawString(140, 125, "Object Count: ", RED);

			if (KEY_DOWN(BUTTON_LEFT, BUTTONS) && ~KEY_JUST_PRESSED(BUTTON_LEFT, currentButtons, previousButtons)) {
				if (CS.player.c > 80) {
					CS.player.c -= CS.player.cdx;
				}
			}
			if (KEY_DOWN(BUTTON_RIGHT, BUTTONS) && ~KEY_JUST_PRESSED(BUTTON_RIGHT, currentButtons, previousButtons)) {
				if (CS.player.c + CS.player.sideLength + CS.player.cdx < 240) {
					CS.player.c += CS.player.cdx;
				}
			}
			if (KEY_DOWN(BUTTON_UP, BUTTONS) && ~KEY_JUST_PRESSED(BUTTON_UP, currentButtons, previousButtons)) {
				if (CS.player.r - CS.player.rdy > -1) {
					CS.player.r -= CS.player.rdy;
				}
			}
			if (KEY_DOWN(BUTTON_DOWN, BUTTONS) && ~KEY_JUST_PRESSED(BUTTON_DOWN, currentButtons, previousButtons)) {
				if (CS.player.r + CS.player.sideLength + CS.player.rdy < 130) {
					CS.player.r += CS.player.rdy;
				}
			}

			for (int i = 0; i < CS.objectCount; i++) {
				CS.objects[i].c += CS.objects[i].cdx;
				CS.objects[i].r += CS.objects[i].rdy;

				if (CS.objects[i].c < 80) {
					CS.objects[i].c = 80;
					CS.objects[i].cdx *= -1;
				}
				if (CS.objects[i].c + CS.objects[i].size > 240) {
					CS.objects[i].c = 240 - CS.objects[i].size;
					CS.objects[i].cdx *= -1;
				}
				if (CS.objects[i].r < 1) {
					CS.objects[i].r = 1;
					CS.objects[i].rdy *= -1;
				}
				if (CS.objects[i].r > 127) {
					CS.objects[i].r = 127;
					CS.objects[i].rdy *= -1;
				}
			}

			//collision detection
			for (int i = 0; i < CS.objectCount; i++) {

				int A = (CS.player.c - CS.objects[i].size) <= CS.objects[i].c 
					&& CS.objects[i].c <= (CS.player.c + CS.player.sideLength);
				
				int B = (CS.player.r - CS.objects[i].size) <= CS.objects[i].r
					&& CS.objects[i].r <= (CS.player.r + CS.player.sideLength);

				int C = CS.player.c <= CS.objects[i].c 
					&& CS.objects[i].c <= CS.player.c + CS.player.sideLength;

				int D = CS.player.r <= CS.objects[i].r
					&& CS.objects[i].r <= CS.player.r + CS.player.sideLength;

				if ((A && B) || (B && C) || (C && D) || (D && A)) {
					state = LOSE;
				}
			}

			if (KEY_DOWN(BUTTON_SELECT, BUTTONS)) {
				drawFullScreenImageDMA(startscreen);
				state = START;
			}
			break;

		case LOSE:
			drawFullScreenImageDMA(losescreen);

			char buffer[25];
			sprintf(buffer, "%d", CS.score);
			drawString(92, 100, buffer, BLACK);


			if (KEY_DOWN(BUTTON_SELECT, BUTTONS)) {
				drawFullScreenImageDMA(startscreen);
				state = START;
			}
			break;
		}

		waitForVBlank();

		if (state == PLAY) {
			drawRectDMA(PS.player.r, PS.player.c,
				PS.player.sideLength, PS.player.sideLength, BLACK);
			drawRectDMA(CS.player.r, CS.player.c,
				CS.player.sideLength, CS.player.sideLength, CS.player.color);

			for (int i = 0; i < PS.objectCount; i++) {
				drawRectDMA(PS.objects[i].r, PS.objects[i].c, PS.objects[i].size,
					PS.objects[i].size, BLACK);
			}
			for (int i = 0; i < CS.objectCount; i++) {
				drawRectDMA(CS.objects[i].r, CS.objects[i].c, CS.objects[i].size,
					CS.objects[i].size, CS.objects[i].color);
			}

			drawRectDMA(140, 80, 45, 30, BLACK);
			drawRectDMA(140, 205, 15, 30, BLACK);

			char buffer[25];
			sprintf(buffer, "%d", CS.score);
			drawString(140, 80, buffer, GREEN);

			char buffer2[25];
			sprintf(buffer2, "%d", CS.objectCount);
			drawString(140, 205, buffer2, RED);
		}
		previousButtons = currentButtons;

	}
	return 0;
}
