#ifndef _SOURCE_TIME
#define _SOURCE_TIME

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

#endif // !_SOURCE_TIME