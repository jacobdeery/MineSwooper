/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "display_manager.h"
#include "game_boards.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOverExploded, GameOverTime} phase;

uint8_t board[9][9];

uint8_t current_game_board_num = 3;
uint8_t time_remaining = 90;
uint8_t mines_remaining = 10;
uint8_t tiles_remaining = 71;
uint8_t cursor_x = 4;
uint8_t cursor_y = 4;

void game_logic_manager(void const *arg) {
	phase = Game; // game starts in main menu
	while(1) {
		osThreadYield();
	}
}

osThreadDef(game_logic_manager, osPriorityNormal, 1, 0);

void display_manager(void const *arg) {
	Display_Init();

	while(1) {
		if(phase == Menu) {
			current_game_board_num = (current_game_board_num + 1) % 4;
			Display_Clear();
			Display_ShowMainMenu();
			while(phase == Menu) {
				Display_UpdateTimeLimit(time_remaining);
				osThreadYield();
			}
		}
		
		else if(phase == Game) {
			Board_GetGameBoardInstance(current_game_board_num, board);
			Display_Clear();
			Display_ShowGameBoard();
			Display_UpdateTimeRemaining(time_remaining);
			Display_UpdateMinesRemaining(mines_remaining);
			Display_DrawCursor(cursor_x, cursor_y);
			while(phase == Game) {
				osDelay(8000);
				
				if(0) { // revealing a tile
					uint8_t revealed_tile = Board_RevealTileAt(board, cursor_x, cursor_y);
					if(revealed_tile == R_M) { // we exploded :(
						Display_DrawCell(cursor_x, cursor_y, 5);
						osDelay(8000);
						phase = GameOverExploded;
					} else if(revealed_tile < F_0) { // tile was not already revealed
						tiles_remaining--;
						if(revealed_tile == R_0) {
							Display_DrawCell(cursor_x, cursor_y, 2);
						} else { // tile has a number
							Display_DrawCellNum(cursor_x, cursor_y, revealed_tile-10);
						}
						if(tiles_remaining == 0) { // we've revealed all of the tiles
							osDelay(8000);
							phase = Victory;
						}
					}

				} else if(0) { // flagging a tile
					uint8_t flag_status = Board_FlagTileAt(board, cursor_x, cursor_y);
					if(flag_status == 1) {
						mines_remaining--;
						Display_DrawCell(cursor_x, cursor_y, 3);
					} else if(flag_status == 0) {
						mines_remaining++;
						Display_DrawCell(cursor_x, cursor_y, 1);
					}
					Display_UpdateMinesRemaining(mines_remaining);

				} else if(1) { // moving cursor
					uint8_t previous_tile = board[cursor_y][cursor_x];
					if(previous_tile < R_0) {
						Display_DrawCell(cursor_x, cursor_y, 1);
					} else if(previous_tile > R_M) {
						Display_DrawCell(cursor_x, cursor_y, 3);
					} else {
						Display_DrawCellNum(cursor_x, cursor_y, previous_tile-10);
					}
					Display_DrawCursor(3, 3);
					cursor_x = 3;
					cursor_y = 3;
				}

				osThreadYield();
			}
		}
		
		else if(phase == Victory) {
			Display_Clear();
			Display_ShowVictory();
			while(phase == Victory) osThreadYield();
		}
		
		else if(phase == GameOverExploded) {
			Display_Clear();
			Display_ShowGameOver(1);
			while(phase == GameOverExploded) osThreadYield();
		}

		else if(phase == GameOverTime) {
			Display_Clear();
			Display_ShowGameOver(0);
			while(phase == GameOverTime) osThreadYield();
		}
	}
}

osThreadDef(display_manager, osPriorityNormal, 1, 0);

int main() {
	printf("Initializing...");
	
	osKernelInitialize();
	osKernelStart();
	
	osThreadCreate(osThread(game_logic_manager), NULL);
	osThreadCreate(osThread(display_manager), NULL);

	while(1);
}
