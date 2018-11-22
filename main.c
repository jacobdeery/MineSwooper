/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

/* 
	POTENTIOMETER NOTES: 
	
	0 is pressed, 1 is not pressed
	
	BITMAP: 
	Left, Up, Right, Down, Button
		23  24     25    26      20
	
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOver} phase;
enum button{Pressed, Not_Pressed};
enum joy{Up, Down, Left, Right};

typedef struct {
	enum button blueButton; 
	enum button joyButton;
	enum joy joyStick;
	uint32_t pot; 
} game_peripheral_message;

void game_peripheral_manager(void const *arg) {
	
	//Pot config
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x1<<12);
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);

	while(1){
		if(phase == Menu){
			//ADC conversion 
			LPC_ADC->ADCR |= (0x1<<24);
			while(!((LPC_ADC->ADGDR >> 31) & 0x1));
			
			uint32_t pot = (LPC_ADC->ADGDR >> 4) & 0xFFF;
		} else if(phase == Game){ 
			// Joystick
			uint32_t joy = LPC_GPIO1->FIOPIN >> 20; 
		}
		
		uint32_t button = LPC_GPI02->FIOPIN >> 10; //INT0 button is required in all states, so constantly check it

		osThreadYield();
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
