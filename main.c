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
enum actionMessage{MoveUp, MoveDown, MoveLeft, MoveRight, RevealTile, PlaceFlag, None};

uint8_t board[9][9];

uint8_t current_game_board_num = 3;
uint8_t time_remaining = 90;
int8_t mines_remaining = 10;
uint8_t tiles_remaining = 71;
uint8_t cursor_x = 4;
uint8_t cursor_y = 4;

uint8_t reveal_queue[162];
uint8_t *head = reveal_queue;
uint8_t *tail = reveal_queue;

const uint8_t peripheralFrequency = 5;

osMailQDef(q1, 10, enum actionMessage);
osMailQId q1_id;

osMutexDef(graphics_mutex);
osMutexId graphics_mutex_id;

osTimerId game_timer_id;

void timer_cb(void const *arg) {
	if(phase == Game) {
		time_remaining--;
		osMutexWait(graphics_mutex_id, osWaitForever);
		Display_UpdateTimeRemaining(time_remaining);
		osMutexRelease(graphics_mutex_id);
		if(time_remaining == 0) {
			osTimerStop(game_timer_id);
			phase = GameOverTime;
		}
	}
}

osTimerDef(game_timer, timer_cb);

void printMessage(enum actionMessage *message) {
	if (*message == MoveUp) {
		printf("Move Up \n");
	} else if (*message == MoveDown) {
		printf("Move Down \n");
	} else if (*message == MoveLeft) {
		printf("Move Left \n");
	} else if (*message == MoveRight) {
		printf("Move Right \n");
	} else if (*message == RevealTile) {
		printf("Reveal Tile \n");
	} else if (*message == PlaceFlag) {
		printf("Place Flag \n");
	} else if (*message == None) {
		printf("None \n");
	}
	printf("%u\n", time_remaining);
}

void game_peripheral_manager(void const *arg) {
	//Pot config
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x1<<12);
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);

	while(1){
		osDelay(8000/peripheralFrequency);

		enum actionMessage message = None;
		uint32_t blueButton = (LPC_GPIO2->FIOPIN >> 10) & 0x1;

		if (phase == Menu) {
			LPC_ADC->ADCR |= (0x1<<24);					//ADC Conversion
			while(!((LPC_ADC->ADGDR >> 31) & 0x1));
			time_remaining = ((float)((uint32_t)((LPC_ADC->ADGDR >> 4) & 0xFFF)))*150/4096 + 30;
			if(!(blueButton)){
				phase = Game;
			}
		} else if(phase == Game){ 
			uint32_t joy = LPC_GPIO1->FIOPIN >> 20;  	// Joystick
			
			if(!(joy & 0x1)) {
				message = RevealTile; 
			} else if(!blueButton) {
				message = PlaceFlag; 
			} if(!(joy>>3 & 0x1)) { 
				message = MoveUp; 
			} else if(!(joy>>4 & 0x1)) {
				message = MoveRight; 
			} else if(!(joy>>5 & 0x1)) {
				message = MoveDown;
			} else if(!(joy>>6 & 0x1)) { 
				message = MoveLeft;
			}
		} else if((phase == Victory || phase == GameOverExploded || phase == GameOverTime) && (!blueButton)) {
			phase = Menu;
		} 

		enum actionMessage *p_message = osMailAlloc(q1_id, 0);
		
		if (p_message != NULL) {
			*((enum actionMessage *)(p_message)) = message; 
			osMailPut(q1_id, p_message);
		}

		osThreadYield();
	}
}

osThreadDef(game_peripheral_manager, osPriorityNormal, 1, 0);

void conditional_reveal_and_queue(uint8_t board[9][9], uint8_t x, uint8_t y) {
	if(board[y][x] < R_0 || board[y][x] > R_M) {
		Board_RevealTileAt(board, x, y);
		tiles_remaining--;
		*tail = x;
		*(tail+1) = y;
		tail += 2;
	}
}

void reveal_group(uint8_t board[9][9], uint8_t x, uint8_t y) {
	*tail = x;
	*(tail+1) = y;
	tail += 2;
	uint8_t revealed_tile = R_0;

	while(head != tail) {
		uint8_t curr_x = *head;
		uint8_t curr_y = *(head+1);

		revealed_tile = board[curr_y][curr_x];

		if(revealed_tile == R_0) {
			osMutexWait(graphics_mutex_id, osWaitForever);
			Display_DrawCell(curr_x, curr_y, 2);
			osMutexRelease(graphics_mutex_id);
			if(curr_x != 0) {
				conditional_reveal_and_queue(board, curr_x-1, curr_y);
				if(curr_y != 0) {
					conditional_reveal_and_queue(board, curr_x-1, curr_y-1);
				}
				if(curr_y != 8) {
					conditional_reveal_and_queue(board, curr_x-1, curr_y+1);
				}
			}
			if(curr_x != 8) {
				conditional_reveal_and_queue(board, curr_x+1, curr_y);
				if(curr_y != 0) {
					conditional_reveal_and_queue(board, curr_x+1, curr_y-1);
				}
				if(curr_y != 8) {
					conditional_reveal_and_queue(board, curr_x+1, curr_y+1);
				}
			}
			if(curr_y != 0) {
				conditional_reveal_and_queue(board, curr_x, curr_y-1);
			}
			if(curr_y != 8) {
				conditional_reveal_and_queue(board, curr_x, curr_y+1);
			}
		} else if(revealed_tile < F_0) {
			osMutexWait(graphics_mutex_id, osWaitForever);
			Display_DrawCellNum(curr_x, curr_y, revealed_tile-10);
			osMutexRelease(graphics_mutex_id);
		}

		head += 2;
	}
	head = reveal_queue;
	tail = reveal_queue;
}

void board_manager(void const *arg) {
	Display_Init();
	phase = Menu;

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
			cursor_x = 4;
			cursor_y = 4;
			tiles_remaining = 71;
			mines_remaining = 10;
			Board_GetGameBoardInstance(current_game_board_num, board);

			osMutexWait(graphics_mutex_id, osWaitForever);
			Display_Clear();
			Display_ShowGameBoard();
			Display_UpdateTimeRemaining(time_remaining);
			Display_UpdateMinesRemaining(mines_remaining);
			Display_DrawCursor(cursor_x, cursor_y);
			osMutexRelease(graphics_mutex_id);

			osTimerStart(game_timer_id, 8000);

			while(phase == Game) {
				enum actionMessage message = None; 
			
				osEvent evt = osMailGet(q1_id, osWaitForever);

				if(evt.status == osEventMail) {
					message = *((enum actionMessage *)(evt.value.p));
					printMessage(&message);
					osMailFree(q1_id, evt.value.p);	
				}

				if(message == RevealTile) { // revealing a tile
					uint8_t revealed_tile = Board_RevealTileAt(board, cursor_x, cursor_y);
					if(revealed_tile == R_M) { // we exploded :(
						osMutexWait(graphics_mutex_id, osWaitForever);
						Display_DrawCell(cursor_x, cursor_y, 5);
						Display_DrawCursor(cursor_x, cursor_y);
						osMutexRelease(graphics_mutex_id);
						osDelay(8000);
						phase = GameOverExploded;
					} else if(revealed_tile < F_0) { // tile was not already revealed
						tiles_remaining--;
						if(revealed_tile == R_0) {
							reveal_group(board, cursor_x, cursor_y);
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCursor(cursor_x, cursor_y);
							osMutexRelease(graphics_mutex_id);
						} else { // tile has a number
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCellNum(cursor_x, cursor_y, revealed_tile-10);
							Display_DrawCursor(cursor_x, cursor_y);
							osMutexRelease(graphics_mutex_id);
						}
						if(tiles_remaining == 0) { // we've revealed all of the tiles
							osDelay(8000);
							phase = Victory;
						}
					}

				} else if(message == PlaceFlag) { // flagging a tile
					uint8_t flag_status = Board_FlagTileAt(board, cursor_x, cursor_y);
					if(flag_status == 1) {
						osMutexWait(graphics_mutex_id, osWaitForever);
						Display_DrawCell(cursor_x, cursor_y, 3);
						Display_DrawCursor(cursor_x, cursor_y);
						Display_UpdateMinesRemaining(--mines_remaining);
						osMutexRelease(graphics_mutex_id);
					} else if(flag_status == 0) {
						osMutexWait(graphics_mutex_id, osWaitForever);
						Display_DrawCell(cursor_x, cursor_y, 1);
						Display_DrawCursor(cursor_x, cursor_y);
						Display_UpdateMinesRemaining(++mines_remaining);
						osMutexRelease(graphics_mutex_id);
					}
					
				} else if(message != None) { // moving cursor
					uint8_t move_cursor = 0;
					uint8_t prev_x = cursor_x;
					uint8_t prev_y = cursor_y;

					if(message == MoveUp && cursor_y != 0) {
						cursor_y--;
						move_cursor = 1;
					} else if(message == MoveDown && cursor_y != 8) {
						cursor_y++;
						move_cursor = 1;
					} else if(message == MoveLeft && cursor_x != 0) {
						cursor_x--;
						move_cursor = 1;
					} else if(message == MoveRight && cursor_x != 8) {
						cursor_x++;
						move_cursor = 1;
					}

					if(move_cursor) {
						uint8_t previous_tile = board[prev_y][prev_x];
						if(previous_tile < R_0) {
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCell(prev_x, prev_y, 1);
							osMutexRelease(graphics_mutex_id);
						} else if(previous_tile > R_M) {
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCell(prev_x, prev_y, 3);
							osMutexRelease(graphics_mutex_id);
						} else if(previous_tile == R_0) {
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCell(prev_x, prev_y, 2);
							osMutexRelease(graphics_mutex_id);
						} else {
							osMutexWait(graphics_mutex_id, osWaitForever);
							Display_DrawCellNum(prev_x, prev_y, previous_tile-10);
							osMutexRelease(graphics_mutex_id);
						}
						osMutexWait(graphics_mutex_id, osWaitForever);
						Display_DrawCursor(cursor_x, cursor_y);
						osMutexRelease(graphics_mutex_id);
					}
				}

				osThreadYield();
			}
		}
		
		else if(phase == Victory) {
			osTimerStop(game_timer_id);
			Display_Clear();
			Display_ShowVictory();
			while(phase == Victory) osThreadYield();
		}
		
		else if(phase == GameOverExploded) {
			osTimerStop(game_timer_id);
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

osThreadDef(board_manager, osPriorityNormal, 1, 0);

int main() {
	printf("Initializing... \n");

	osKernelInitialize();
	osKernelStart();

	q1_id = osMailCreate(osMailQ(q1), NULL);
	graphics_mutex_id = osMutexCreate(osMutex(graphics_mutex));
	game_timer_id = osTimerCreate(osTimer(game_timer), osTimerPeriodic, NULL);
	
	osThreadCreate(osThread(board_manager), NULL);
	osThreadCreate(osThread(game_peripheral_manager), NULL);

	while(1);
}
