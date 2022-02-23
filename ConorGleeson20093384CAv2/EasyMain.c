/*

 * ictSortArea.c

 *

 *  Created on: 11 Nov 2021

 *      Author: JDALY

 */

#include "xmc_common.h"


#include <xmc_gpio.h>


/* Inputs */

#define SortAreaSensor P0_5

#define SortAreaMetalDetected P0_8

#define AsmAreaSensor P0_0

#define RejectAreaSensor P0_6

#define CheckAreaUnitAssembled P0_7 //capacitive ARE sensor

#define IRThruBeam P0_9

#define CheckAreaUnitDetected P0_14 //capcitive proximity sensor

#define InductiveProxSensor P0_15

unsigned int ThruBeamActivated = 0;
unsigned int InducProxActivated = 0;

unsigned int RejectAreaSensorActivated = 0;
unsigned int CheckAreaUnitDetectedActivated = 0;



/* Outputs */

#define SortSol P2_0

#define RotarySol P2_6

#define RejectSol P2_7

#define ChainConv P2_9

#define BeltConv 2_10

unsigned int RotarySolActivated = 0;
unsigned int RejectSolActivated = 0;
unsigned int ChainConvActivated = 0;
unsigned int BeltCovActivated = 0;


unsigned int counter = 0;

unsigned int counterAsmQueue =0;

unsigned int SortSolActivated = 0;

//flags
unsigned int ThruBeamFlag = 0;
unsigned int InducProxFlag = 0;

unsigned int RejectAreaSensorFlag = 0;
unsigned int CheckAreaUnitDetectedFlag = 0;


//int timer[4];

int timer[10][4];

void TenthSecTick(void){
for(int i = 0; i<10; i++){
		if(timer[i][2] == 1){            //if timer is activated

			timer[i][0]++;    //increment timer current value

			if(timer[i][0] >= timer[i][1]){         //if current value >= preset

				timer[i][3] = 1;  //Signal Timer has timed-out

			}

			else{

				timer[i][3] = 0;   //Otherwise, signal Timer has not timed-out

			}

		}
		}

}


void SysTick_Handler(void){

	TenthSecTick();

	  XMC_GPIO_SetOutputHigh(LED2); //Testing

}



void initPortPins(){

	XMC_GPIO_SetMode(SortAreaSensor, XMC_GPIO_MODE_INPUT_PULL_DOWN );

	XMC_GPIO_SetMode(SortAreaMetalDetected, XMC_GPIO_MODE_INPUT_PULL_DOWN );



	XMC_GPIO_SetMode(SortSol, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

	//mine
	XMC_GPIO_SetMode( AsmAreaSensor, XMC_GPIO_MODE_INPUT_PULL_DOWN );

	XMC_GPIO_SetMode(RotarySol, XMC_GPIO_MODE_OUTPUT_PUSH_PULL );

	XMC_GPIO_SetMode(RejectSol, XMC_GPIO_MODE_OUTPUT_PUSH_PULL );

	XMC_GPIO_SetMode(IRThruBeam, XMC_GPIO_MODE_INPUT_PULL_DOWN  );




	XMC_GPIO_SetMode(CheckAreaUnitDetected, XMC_GPIO_MODE_INPUT_PULL_DOWN);
	XMC_GPIO_SetMode(RejectAreaSensor, XMC_GPIO_MODE_INPUT_PULL_DOWN);

}


void setUpTimers(void){

	/* Sort Timers */

	//timer[0] = 0;  	//timerCurrent in 0.1 Second steps

	//timer[1] = 10;   //timerPreset in 0.1 Second steps

//	timer[2] = 0;	//timerEnable - 0 = Disabled, 1=Enabled

	//timer[3] = 0;	//timerOutput - 0=Off, 1=On

	//two dimensional timers
	for(int i = 0; i <10; i++){
		timer[i][0] = 0;

		timer[i][1] = 10;

		timer[i][2] = 0;

		timer[i][3] = 0;
	}


}

/* Main */

int main(void)

{

	initPortPins(); //method to initiate ports I/O pins

	setUpTimers(); //method to set up timer data structure
	//turn on converyrs here after pins

  /* System timer configuration */

  SysTick_Config(SystemCoreClock /10);  // interrupt called every 0.1 seconds


  while(1)

  {

	  /* Sort Area */


	//if the sort solenoid is not active, the sort area sensor is equal to 1 and the sort area metal sensor is equal to 0,

	  //then activate the sort solenoid and increment the counter.


if(!SortSolActivated && XMC_GPIO_GetInput(SortAreaSensor) == 1  && XMC_GPIO_GetInput(SortAreaMetalDetected) == 0 && counterAsmQueue < 5) {

	SortSolActivated = 1; //set sort solenoid activation marker to 1

	timer[0][2] = 1;  //enable the timer

	XMC_GPIO_SetOutputHigh(SortSol); //activate Sort Solenoid

	counterAsmQueue++; //increment counter to show how many rings in the queue

	}

// else if the sort solenoid is active and the timer output is equal to 1,

//then deactivate the sort solenoid.

else if(SortSolActivated && timer[0][3] == 1) {

	SortSolActivated = 0;

	timer[0][2] = 0;  //de-activate the timer

	timer[0][0] = 0; //reset current value

	XMC_GPIO_SetOutputLow(SortSol);

	}


//assembly area code

if(!RotarySolActivated && XMC_GPIO_GetInput(AsmAreaSensor) == 0 && counterAsmQueue > 1 ){
	RotarySolActivated = 1;

	timer[1][2] = 1;

	XMC_GPIO_SetOutputHigh(RotarySol);



}
else if (RotarySolActivated && timer[1][3] == 1){
	timer[1][2] = 0;

	timer[1][0] = 0;

	XMC_GPIO_SetOutputLow(RotarySol);

	counterAsmQueue--;
}

//metal peg detection on lower conveyor
if(XMC_GPIO_GetInput(IRThruBeam) ==1 ){
	ThruBeamFlag == 1;

}



if  ( XMC_GPIO_GetInput(RejectAreaSensor) == 1){
	RejectSolActivated = 1;
		timer[2][2] = 1;
	if( ThruBeamFlag ==1  || InudcProxFlag == 1 ||  CheckAreaUnitDetectedFlag == 1){   //combine 254 and 261  //if not made properly then start rejectSol and its timer.
		XMC_GPIO_SetOutputHigh(RejectSol);
	}


}
else if(RejectSolActivated == 1 && timer[1][3] == 1){ //reset flags after timer is timed out

		timer[1][2] = 0;
}

		timer[1][0] = 0;

		XMC_GPIO_SetOutputLow(RejectSol);

			ThruBeamFlag = 0;
			InducProxFlag = 0;
			CheckAreaUnitDetectedFlag  = 0;
}

//Ring above peg checking
 //needs to be much the same as above combine the checks  put above reject area can only do reject code once nothing to be done with reject solenoid incorperate


}
