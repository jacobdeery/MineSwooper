/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include <string.h>

enum gamePhase{Menu, Game, Victory, GameOver} phase;
enum button{Pressed, Not_Pressed};
enum joyStick{Up, Down, Left, Right, None};

typedef struct {
	enum button blueButton; 
	enum button joyButton;
	enum joyStick joyDirection;
	uint32_t pot; 
} game_peripheral_message;

osMailQDef(q1, 10, game_peripheral_message);
osMailQId q1_id;

void game_peripheral_manager(void const *arg) {
	//Pot config
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x1<<12);
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);

	while(1){
		game_peripheral_message message = {Not_Pressed, Not_Pressed, None, 0}; 
		
		if(phase == Menu){
			//ADC conversion 
			LPC_ADC->ADCR |= (0x1<<24);
			while(!((LPC_ADC->ADGDR >> 31) & 0x1));
			
			message.pot = (uint32_t)((LPC_ADC->ADGDR >> 4) & 0xFFF);
		} else if(phase == Game){ 
			//Joystick
			uint32_t joy = LPC_GPIO1->FIOPIN >> 20; 

			//Joystick Button
			if(!(joy & 0x1)) message.joyButton = Pressed;

			//Joystick Direction
			if(!(joy>>23 & 0x1))
				message.joyDirection = Left;
			else if(!(joy>>24 & 0x1))
				message.joyDirection = Up;
			else if(!(joy>>25 & 0x1)) 
				message.joyDirection = Right;
			else if(!(joy>>26 & 0x1)) 
				message.joyDirection = Down;
			else 
				message.joyDirection = None;
		}
		
		//INT0 Button
		message.blueButton = (uint32_t)(LPC_GPI02->FIOPIN >> 10);
		
		game_peripheral_message *p_message = osMailAlloc(q1_id, 0);
		if (p_message != NULL) osMailPut(q1_id, p_message);

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

	q1_id = osMailCreate(osMailQ(q1), NULL);

	osKernelInitialize();
	osKernelStart();

	osThreadCreate(osThread(game_logic_manager), NULL);
	osThreadCreate(osThread(display_manager), NULL);
	osThreadCreate(osThread(game_peripheral_manager), NULL);

	while(1);
}
