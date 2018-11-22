/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/

#include <lpc17xx.h>
#include "stdio.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include <string.h>

#define PER_FREQ  2  //Frequency in Hz
#define DEFAULT_TIME 120 //Time in Seconds

enum gamePhase{Menu, Game, Victory, GameOver} phase;
enum actionMessage{MoveUp, MoveDown, MoveLeft, MoveRight, RevealTile, PlaceFlag, None};

uint32_t time_remaining = 50;

osMailQDef(q1, 10, enum actionMessage);
osMailQId q1_id;

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
	printf("%u", time_remaining);
}

void game_peripheral_manager(void const *arg) {
	//Pot config
	LPC_PINCON->PINSEL1 &= ~(0x03<<18);
	LPC_PINCON->PINSEL1 |= (0x01<<18);
	LPC_SC->PCONP |= (0x1<<12);
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);

	while(1){
		osDelay(8000/PER_FREQ);

		enum actionMessage message = None;
		uint32_t blueButton = (LPC_GPIO2->FIOPIN >> 10) & 0x1;

		if ((phase != Game) && (!blueButton)) {
			LPC_ADC->ADCR |= (0x1<<24);					//ADC Conversion
			while(!((LPC_ADC->ADGDR >> 31) & 0x1));
			
			time_remaining = (uint32_t)((LPC_ADC->ADGDR >> 4) & 0xFFF);
			phase = Game;

		} else if(phase == Game){ 
			uint32_t joy = LPC_GPIO1->FIOPIN >> 20;  	// Joystick

			if(!(joy & 0x1)) {
				message = RevealTile; 
			} else if(!blueButton) {
				message = PlaceFlag; 
			} if(!(joy>>3 & 0x1)) { 
				message = MoveLeft; 
			} else if(!(joy>>4 & 0x1)) {
				message = MoveUp; 
			} else if(!(joy>>5 & 0x1)) {
				message = MoveRight;
			} else if(!(joy>>6 & 0x1)) { 
				message = MoveDown;
			} 			
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

void game_logic_manager(void const *arg) {
	phase = Menu; // game starts in main menu
	
	while(1) {
		printf("\n");
		enum actionMessage message = None; 
		
		osEvent evt = osMailGet(q1_id, osWaitForever);
		
		if(evt.status == osEventMail) {
			message = *((enum actionMessage *)(evt.value.p));
			osMailFree(q1_id, evt.value.p);	
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
