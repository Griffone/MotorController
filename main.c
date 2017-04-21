/*

	Main file. This is where the compilation starts and ends.
	
	by
		Aleksandra Soltan
		Grigory Glukhov
	
*/

// No guard blocks here, as this file is not supposed to be included in any other

// Standard libraries and processor type includes
#include "deps/16F690.h"
#include "deps/int16Cxx.h"

// Preprocessor pragmas for correct memory usage
#pragma config |= 0x00D4 
#pragma char X @ 0x115

// Header includes before interrupt routine
#include "time.h"
#include "io.h"
#include "motor.h"

// Function definitions
void delay10(char);
void init_io_ports( void );
void init_serial( void );
void init_interrupt( void );
void putchar( char);
void printf(const char *string, char variable);

// Variable definitions
bit motor_enable;				// is motor enabled
unsigned long motor_period;		// period of the motor's step

// Interrupt routine, because of the compiler spicifics, we need to define it before any other code...
#pragma origin 4
interrupt int_server() {
	int_save_registers
	/* New interrupts are automaticaly disabled            */
	/* "Interrupt on change" at pin RA1 from PK2 UART-tool */

	if (IF_TIME)
		time_update();

	RABIF = 0;    /* Reset the RABIF-flag before leaving   */
	int_restore_registers
}

// Source file definitions
// In standard C this would not be necessary, but Cc5x is not a standard compiler, so we assist it a little
#include "time.c"
#include "io.c"
#include "motor.c"

void motor_delay() {
	time_wait(1);
}

// Program entry point
void main(void) {

	time_init();
	io_init();
	motor_init();
	
	ANSEL = 0;	// we don't need any AD-inputs
	GIE = 1;	// enable interrupts
	
	motor_enable = 1;
	motor_direction = MOTOR_COUNTERCLOCKWISE;
	motor_stepSize = MOTOR_FULL_STEP;
	motor_period = 2;	// minimal tickrate = max speed
	
	unsigned long motor_lastTick = 0;
	unsigned long lastPrintTick = 0;
	
	char c;
	
	while(1) {
		motor_direction = !motor_direction;
		for (c = 0; c < MOTOR_FULL_REVOLUTION; c++) {
			time_wait(99);
			motor_step();
		}
	}
	
	// Core loop
	while (1) {
		io_update();	// Send and receive any characters if necessary
		
		// TODO: parsing here
		
		if (motor_enable) {
			motor_step();
			unsigned long delta = time_tick - motor_lastTick;
			if (delta >= motor_period)
				motor_step();
		}
		
		unsigned long pDelta = time_tick - lastPrintTick;
		if (pDelta >= 790)
			io_print("Test\n");
	}
}

/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */

/*           _____________  _____________ 
            |             \/             |
      +5V---|Vdd        16F690        Vss|---Gnd
			|RA5            RA0/AN0/(PGD)|bbTx->- PK2 UART-tool
			|RA4/AN3            RA1/(PGC)|bbRx-<- PK2 UART-tool
            |RA3/!MCLR/(Vpp)  RA2/AN2/INT|
            |RC5/CCP                  RC0|->-!STEP
            |RC4                      RC1|->-DIR
            |RC3/AN7                  RC2|->-!HSM
            |RC6/AN8             AN10/RB4|
            |RC7/AN9               RB5/Rx|
            |RB7/Tx                   RB6|
            |____________________________|                                      
*/ 