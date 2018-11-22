/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "display_manager.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOver} phase;
uint32_t time_remaining = 50;

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
			Display_ShowMainMenu();
			while(phase == Menu) {
				// update time limit display
				osThreadYield();
			}
		}
		
		else if(phase == Game) {
			Display_ShowGameBoard();
			while(phase == Game) {
				Display_UpdateTimeRemaining(time_remaining);
				osThreadYield();
			}
		}
		
		else if(phase == Victory) {
			Display_ShowVictory();
		}
		
		else if(phase == GameOver) {
			Display_ShowGameOver();
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
