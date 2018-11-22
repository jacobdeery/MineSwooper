/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "display_manager.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOverExploded, GameOverTime} phase;
uint32_t time_remaining = 50;
uint32_t mines_remaining = 50;

void game_logic_manager(void const *arg) {
	phase = Victory; // game starts in main menu
	while(1) {
		osThreadYield();
	}	
}

osThreadDef(game_logic_manager, osPriorityNormal, 1, 0);

void display_manager(void const *arg) {
	Display_Init();

	while(1) {
		if(phase == Menu) {
			Display_Clear();
			Display_ShowMainMenu();
			while(phase == Menu) {
				Display_UpdateTimeLimit(time_remaining);
				osThreadYield();
			}
		}
		
		else if(phase == Game) {
			Display_Clear();
			Display_ShowGameBoard();
			while(phase == Game) {
				Display_UpdateTimeRemaining(time_remaining);
				Display_UpdateMinesRemaining(mines_remaining);
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
