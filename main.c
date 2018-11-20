/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOver} phase;

void game_peripheral_manager(void const *arg) {
	while(1){
		if(phase == Menu){
			// Potentiometer, INT0 Button
		} else if(phase == Game){ 
			// Joystick, INT0 Button
		} else if(phase == Victory || phase == GameOver) {
			// INT0 Button
		}
	}
}

osThreadDef(game_peripheral_manager, osPriorityNormal, 1, 0);

void game_logic_manager(void const *arg) {
	phase = Menu; // game starts in main menu
	while(1) {
		osThreadYield();
	}	
}

osThreadDef(game_logic_manager, osPriorityNormal, 1, 0);

void display_manager(void const *arg) {
	while(1) {
		if(phase == Menu) {
			// display menu graphics
			while(phase == Menu) {
				// update time limit display
				osThreadYield();
			}
		}
		
		else if(phase == Game) {
			// display game graphics
			while(phase == Game) {
				// update board and timer display
				osThreadYield();
			}
		}
		
		else if(phase == Victory) {
			// display victory screen
		}
		
		else if(phase == GameOver) {
			// display game over screen
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
	osThreadCreate(osThread(game_peripheral_manager), NULL);

	while(1);
}
