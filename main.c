/*
Jacob Deery (jbdeery) and Jonathan Parson (jmparson)
*/


#include <lpc17xx.h>
#include "stdio.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include <string.h>

void generic_thread(void const *arg) {
	while(1) {
		osThreadYield();
	}	
}

osThreadDef(generic_thread, osPriorityNormal, 1, 0);

int main() {
	printf("Initializing...");
	
	osKernelInitialize();
	osKernelStart();
	
	osThreadCreate(osThread(generic_thread), NULL);

	while(1);
}
