/*

	A single file version of the project
	
	Main file. This is where the compilation starts and ends.
	
	by
		Aleksandra Soltan
		Grigory Glukhov
	
*/

// Standard libraries and processor type includes
#include "deps/16F690.h"
#include "deps/int16Cxx.h"



// Preprocessor pragmas for correct memory usage
#pragma config |= 0x00D4 



// Time definitions
#pragma bit IF_TIME @ T0IF

#define TIME_TICK_PERIOD	159
#define TIME_PRESCALE		0b001	// 1:16
#define TIME_RESET			(255 - TIME_TICK_PERIOD)	// this will make the timer overflow every tick period

// Current tick (roughly 1/1580th of a second, because we want 2 ticks in 1 motor pulse (we presume motor's max pulserate is 790))
extern unsigned long time_tick;

// Initialized the timer and timer-related interrupt
void time_init();

// Is to be called on timer interrupt, this will update the tick
void time_update();

// Wait until provided number of ticks pass
void time_wait(unsigned long);



// IO definitions
// Max size of the input string (should be of length |max legal command| + 1)
#define IO_SIZE_IN 16

// An input string buffer, is to be used by parsing functions
extern char io_in[IO_SIZE_IN];	

// Initialize everything IO-related (uart ports, control registers and interrupts)
void io_init();

// Will return pointer to the first character of input string, or 0 if still receiveing input
const char * io_getInput();

// Returns 1 if strings are the same until a becomes null
// Will always return 1 if compares a null pointer to anything
bit strcmp(const char * a, const char * b);

// Retuns a number, represented by provided string (assumed it's base 10) or 0 if the string is garbage
unsigned long stoi(const char *);

// Simply prints the given string
void io_print(const char *);

size2 const char * toString(unsigned long);



// Motor definitions
#define MOTOR_FULL_REVOLUTION 48 //for full step
#define MOTOR_HALF_REVOLUTION 24 //for full step
#define MOTOR_COUNTERCLOCKWISE 1
#define MOTOR_CLOCKWISE 0
#define MOTOR_FULL_STEP 1
#define MOTOR_HALF_STEP 0

#pragma bit PORT_MOTOR_STEP_SIZE	@ PORTC.0	// Should be either MOTOR_FULL_STEP or MOTOR_HALF_STEP
#pragma bit PORT_MOTOR_DIRECTION	@ PORTC.1	// Should be either MOTOR_CLOCKWISE or MOTOR_COUNTERCLOCKWISE
#pragma bit PORT_MOTOR_STEP			@ PORTC.2	// Used internally by motor_step()

// Initialize motor-related pins
void motor_init();

// Run a single step
void motor_step();



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



// Time source
unsigned long time_tick;

void time_init() {
	time_tick = 0;

	T0CS = 0;					// Timer0 will use internal oscilator
	TMR0 = 0;					// reset timer
	
	OPTION.3 = 0;				// prescler is used with Timer0
	
	OPTION &= ~0b111;			// reset to 'xxxxx000'
	OPTION |= TIME_PRESCALE;
	
	T0IE = 1;					// enable timer0 - related overflow interrupt
}

void time_update() {
	time_tick++;
	TMR0 = TIME_RESET;	// This will give roughly 1/790th of a second period if used with 1:32 prescale
	IF_TIME = 0;		// reset interrupt flag
}

void time_wait(unsigned long t) {
	unsigned long end = time_tick + t;
	while (time_tick != end);
}


// IO source
char inputPos;
char io_in[IO_SIZE_IN];
bit io_echo;

void io_init() {
	
	io_echo = FALSE;
	
	// Enable pins, EUSART will reconfigure them as necessary
	TRISB.6 = 1;
	TRISB.7 = 1;
	
	/*
		To calculate baud rate period:
		T = Main oscilator time (4 MHz in our case)
		D = Desired baud rate (9600 in our case)
		p = period, value we're calculating
		
		D = T/(16 *(p+1))
		
		p = T/D/16-1
		p = 4 000 000 / 6900 / 16 - 1 = 25
	*/
	BRGH = 1;	// Make period more precise (divide D by 16 not 64)
	SPBRGH = 0;	// high byte of desired baud rate 
	SPBRG = 25;	// specify our period
	
	TX9 = 0;	// use 8-bit characters
	SYNC = 0;	// set it to asynchrounous transmishion
	TXEN = 1;	// enable UART circuitry
	
	SPEN = 1;	// enable EUSART and automatically configure TX/CK as output
	
	CREN = 1;	// enable reciever circuitry
	RX9 = 0;	// use 8-bit characters
}

const char * io_getInput() {
	
	CREN = 1;	// Just in case an error occured
	
	// Receiveing data
	if (RCIF) {
		if (inputPos >= IO_SIZE_IN)
			inputPos = 0;	// silently wrap around, bad practice but functional

		io_in[inputPos] = RCREG;	// read input here
		if (io_echo)
			TXREG = io_in[inputPos];
		if (io_in[inputPos] == '\0' || io_in[inputPos] == '\n' || io_in[inputPos] == '\r') {
			io_in[inputPos] = '\0';
			inputPos = 0;
			return io_in;
		} else
			inputPos++;
	}
	
	return 0;
}

void io_print(const char * s) {
	while (*s) {
		while (!TRMT);	// wait until character is transmitted
		TXREG = *s;
		s++;
	}
}

bit strcmp(const char * a, const char * b) {
	char ac, bc;
	while (*a) {
		ac = *a;
		bc = *b;
		if (ac != bc)
			return FALSE;	// Hit a different char
		a++;
		b++;
	}
	return TRUE;	// Hit a null-char
}

unsigned long stoi(const char * s) {
	unsigned long r = 0;
	while (*s) {
		// abuse ASCII notation, numbers follow each other
		if (*s >= '0' && *s <= '9') {
			r *= 10;
			r += *s - '0';
		}
		s++;
	}
	return r;
}

size2 const char * toString(unsigned long n) {
	char str[12];	// 6 + 6, because str[5] is somehow broken (no idea why) 65535 is max value, which requires 5 chars + 1 null char
	str[11] = '\0';
	int i = 10;
	int t;
	str[i] = '0';
	do {
		t = n % 10;
		str[i--] = '0' + t;
		n /= 10;
	} while (n);
	return &str[i + 1];
}



// Motor source
#define MOTOR_DELAY() time_wait(1)

void motor_init() {
	TRISC &= ~0b111;	// '11111000' for outputs at motor-related pins
	PORTC &= ~0b111;	// 'xxxxx000' for initial value
}

void motor_step() {
	PORT_MOTOR_STEP = 1;
	MOTOR_DELAY();
	PORT_MOTOR_STEP = 0;
}



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
            |RC5/CCP                  RC0|->-!HSM
            |RC4                      RC1|->-DIR
            |RC3/AN7                  RC2|->-!STEP
            |RC6/AN8             AN10/RB4|
            |RC7/AN9               RB5/Rx|-<-UART_IN
 UART_OUT-<-|RB7/Tx                   RB6|
            |____________________________|                                      
*/ 
/*           _____________  _____________ 
            |             \/             |
    COIL<---|PB2        PBD3517       VCC|---+6V
    COIL<---|PB1                      VSS|
      Gnd---|GND                       LB|
    COIL<---|PA1                       LA|
    COIL<---|PA2                       RC|
      RC1->-|DIR                      INH|---Gnd
      RC2->-|!STEP                   !HSM|-<-RC0
            |OB                        OA|
            |____________________________|                                      
*/
