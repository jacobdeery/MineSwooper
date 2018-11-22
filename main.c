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

uint32_t time_remaining = 50;
const uint16_t perFreq = 1; 		// Frequency in Hz

osMailQDef(q1, 10, game_peripheral_message);
osMailQId q1_id;

void game_peripheral_manager(void const *arg) {
	//Pot config
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x1<<12);
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);

	while(1){
		osDelay(8000/perFreq);
		game_peripheral_message message = {Not_Pressed, Not_Pressed, None, 0}; 
		
		if(phase == Menu){
			//ADC conversion 
			LPC_ADC->ADCR |= (0x1<<24);
			while(!((LPC_ADC->ADGDR >> 31) & 0x1));
			
			message.pot = (uint32_t)((LPC_ADC->ADGDR >> 4) & 0xFFF);
			printf("%u \n",message.pot);

		} else if(phase == Game){ 
			//Joystick
			uint32_t joy = LPC_GPIO1->FIOPIN >> 20; 

			//Joystick Button
			if(!(joy & 0x1)) message.joyButton = Pressed;
			
			//Joystick Direction
			if(!(joy>>3 & 0x1)) { 
				message.joyDirection = Left;
			} else if(!(joy>>4 & 0x1)) {
				message.joyDirection = Up;
			} else if(!(joy>>5 & 0x1)) {
				message.joyDirection = Right;
			} else if(!(joy>>6 & 0x1)) { 
				message.joyDirection = Down;
			} else { 
				message.joyDirection = None;
			}
			
		}

		if((LPC_GPIO2->FIOPIN >> 10) & 0x1) { 
			message.blueButton = Not_Pressed;
		} else {
			message.blueButton = Pressed; 
		}

		game_peripheral_message *p_message = osMailAlloc(q1_id, 0);
		if (p_message != NULL) osMailPut(q1_id, p_message);

		osThreadYield();
	}
}

osThreadDef(game_peripheral_manager, osPriorityNormal, 1, 0);

void game_logic_manager(void const *arg) {
	phase = Menu; // game starts in main menu
	
	while(1) {
		game_peripheral_message message; 
		
		osEvent evt = osMailGet(q1_id, osWaitForever);
		
		if(evt.status == osEventMail) {
			memcpy(&message, evt.value.p, sizeof(game_peripheral_message));
			osMailFree(q1_id, evt.value.p);
		}

		if(phase == Menu) {
			if(message.blueButton == Pressed){
				phase = Game; 
				time_remaining = ((float)message.pot/4095)*50; 
			}
		} else if(phase == Game) {
			if(message.blueButton == Pressed) {
				//place flag
			}
			if (message.joyDirection != None) {
				//Update cursor position
			}
			if (message.joyButton == Pressed) {
				//Reveal tile
			}
		} else if(phase == Victory) {
			if(message.blueButton == Pressed) {
				phase = Menu; 
				time_remaining = 50; 
			}	
		} else if(phase == GameOver) {
			if(message.blueButton == Pressed) {
				phase = Menu; 
				time_remaining = 50; 
			}
		}
		osThreadYield(); 
	}	
}

/*
	NOTES: 
		1. Clear the queue between game states? 
		2. How fast should we be running peripherals to avoid "duplicate" actions? 	
	STRUCTURE: 
		1. Extract peripheral message from queue
		2. Check game state 
		3. Read peripheral message
		4. Perform action
		5. Update map structure
	
	PERIPHERAL PRECEDENCE 
		INT0 > MOVE > BUTTON PUSH > POT
		Need to discuss if we're going to allow "compound actions" or w.e. 
*/

osThreadDef(game_logic_manager, osPriorityNormal, 1, 0);

void display_manager(void const *arg) {
	while(1) {
		printf("%u \n", time_remaining);
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
	printf("Initializing... \n");

	q1_id = osMailCreate(osMailQ(q1), NULL);

	osKernelInitialize();
	osKernelStart();

	osThreadCreate(osThread(game_logic_manager), NULL);
	osThreadCreate(osThread(display_manager), NULL);
	osThreadCreate(osThread(game_peripheral_manager), NULL);

	while(1);
}
