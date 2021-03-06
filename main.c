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



// Header includes before interrupt routine
#include "time.h"
#include "io.h"
#include "motor.h"



// Type definitions:
typedef enum {
	CMD_NULL = 0,
	CMD_HELP,
	CMD_INFO,
	CMD_START,
	CMD_STOP,
	CMD_SPEED,
	CMD_DIR,
	CMD_SIZE,
	CMD_STEP
} Command;
#define TRUE	1
#define FALSE	0



// Variable definitions
bit motor_enable;				// start command sets this to 1 until stop or step command
unsigned long motor_steps;		// step command will set this to some value, that will go down with each motor tick
unsigned long motor_period;		// period of the motor's step
const char * pArg;				// pointer to the first char of argument in input string
Command cmd;					// command enum, necessary due to compiler limitation


// Function definitions
void parseInput(const char *);


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



// Program entry point
void main(void) {

	time_init();
	io_init();
	motor_init();
	
	ANSEL = 0;	// we don't need any AD-inputs
	ANSELH = 0;
	GIE = 1;	// enable interrupts
	
	motor_enable = FALSE;
	motor_steps = 0;
	PORT_MOTOR_DIRECTION = MOTOR_CLOCKWISE;
	PORT_MOTOR_STEP_SIZE = MOTOR_FULL_STEP;
	
	motor_period = 790;	// half second period
	
	unsigned long motor_tick = 0;	// the tick for motor
	unsigned long lastTick = 0;
	unsigned long arg;
	
	io_echo = FALSE;
	
	const char * input;
	
	// Core loop
	while (1) {
		PORTC.5 = RCIF;
		input = io_getInput();
		
		if (input) {
			// parseInput updates cmd (this is due to compiler limitations, otherwise I'd make it return a value)
			parseInput(input);
			if (cmd) {
				switch (cmd) {
				
				case CMD_HELP:
					io_print("Availabe commands:\r\n");
					io_print("?/help - displays this message\r\n");
					io_print("info - displays motor info\r\n");
					io_print("start - starts the motor\r\n");
					io_print("stop - stops the motor\r\n");
					io_print("speed [x] - sets the speed of the motor to x (3-65535)\r\n");
					io_print("dir [x] - sets the direction to x (\"cc\" or \"cw\")\r\n");
					io_print("size [x] - sets the step size to x (\"full\" or \"half)\"\r\n");
					io_print("step [x] - makes motor step x (1-65535) times\r\n");
				break;
				
				case CMD_INFO:
					io_print("Motor is ");
					if (motor_enable || motor_steps)
						io_print("ON\r\n");
					else
						io_print("OFF\r\n");
					
					io_print("Period = ");
					io_print(toString(motor_period));
					
					io_print(" ticks (tickrate = 1580 Hz)\r\n");
					io_print("Direction is ");
					if (PORT_MOTOR_DIRECTION == MOTOR_COUNTERCLOCKWISE)
						io_print("counter ");
					io_print("clockwise\r\n");
					
					io_print("Step size is ");
					if (PORT_MOTOR_STEP_SIZE == MOTOR_FULL_STEP)
						io_print("full");
					else
						io_print("half");
					io_print(" steps\r\n");
				break;
				
				case CMD_START:
					motor_enable = TRUE;
				break;
				
				case CMD_STOP:
					motor_enable = FALSE;
					motor_steps = 0;
				break;
				
				case CMD_SPEED:
					arg = stoi(pArg);
					if (arg >= 3) {		// speeds lower than 3 are below rated pulserate
						motor_period = arg;
						motor_tick = 0;	// reset motor tick, so that the new speed applies
					}
					
					io_print("Stepping every ");
					io_print(toString(motor_period));
					io_print(" ticks\r\n");
				break;
				
				case CMD_DIR:
					if (*pArg)
						if (strcmp(" cc", pArg))
							PORT_MOTOR_DIRECTION = MOTOR_COUNTERCLOCKWISE;
						else if (strcmp(" cw", pArg))
							PORT_MOTOR_DIRECTION = MOTOR_CLOCKWISE;
						else
							io_print("Unknown direction\r\n");
					
					io_print("Motor direction is ");
					if (PORT_MOTOR_DIRECTION == MOTOR_COUNTERCLOCKWISE)
						io_print("counter ");
					io_print("clockwise\r\n");
				break;
				
				case CMD_SIZE:
					if (*pArg)
						if (strcmp(" full", pArg))
							PORT_MOTOR_STEP_SIZE = MOTOR_FULL_STEP;
						else if (strcmp(" half", pArg))
							PORT_MOTOR_STEP_SIZE = MOTOR_HALF_STEP;
						else
							io_print("Unknown step size\r\n");
					
					io_print("Stepping with ");
					if (PORT_MOTOR_STEP_SIZE == MOTOR_FULL_STEP)
						io_print("full");
					else
						io_print("half");
					io_print(" steps\r\n");
				break;
				
				case CMD_STEP:
					arg = stoi(pArg);
					if (arg) {
						motor_steps = arg;
						motor_enable = FALSE;
					}
					
					io_print("Stepping for another ");
					io_print(toString(motor_steps));
					io_print(" steps\n");
				break;
				}
			}
		}
		
		// Run io_update again, but ignore everything else
		if (lastTick == time_tick)
			continue;
		
		// We assume time_tick is equal to lastTick + 1
		if (++motor_tick == motor_period) {
			motor_tick = 0;

			if (motor_enable)
				motor_step();

			if (motor_steps) {
				motor_steps--;
				motor_step();
			}
		}
		lastTick = time_tick;
	}
}

void parseInput(const char * s) {
	cmd = CMD_NULL;
	
	if (strcmp("?", s)) {
		cmd = CMD_HELP;
		return;
	}
	
	if (strcmp("help", s)){
		cmd = CMD_HELP;
		return;
	}
	
	if (strcmp("info", s)){
		cmd = CMD_INFO;
		return;
	}
	
	if (strcmp("start", s)){
		cmd = CMD_START;
		return;
	}
		
	if (strcmp("stop", s)){
		cmd = CMD_STOP;
		return;
	}
	
	if (strcmp("speed", s)){
		cmd = CMD_SPEED;
		pArg = &s[5];
		return;
	}
	
	if (strcmp("dir", s)){
		cmd = CMD_DIR;
		pArg = &s[3];
		return;
	}
	
	if (strcmp("size", s)) {
		cmd = CMD_SIZE;
		pArg = &s[4];
		return;
	}
	
	if (strcmp("step", s)){
		cmd = CMD_STEP;
		pArg = &s[4];
		return;
	}
}

/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */

/*           _____________  _____________ 
            |             \/             |
      +5V---|Vdd        16F690        Vss|---Gnd
			|RA5            RA0/AN0/(PGD)|
			|RA4/AN3            RA1/(PGC)|
            |RA3/!MCLR/(Vpp)  RA2/AN2/INT|
            |RC5/CCP                  RC0|->-!STEP
            |RC4                      RC1|->-DIR
            |RC3/AN7                  RC2|->-!HSM
            |RC6/AN8             AN10/RB4|
            |RC7/AN9               RB5/Rx|-<-UART_IN
 UART_OUT-<-|RB7/Tx                   RB6|
            |____________________________|                                      
*/ 