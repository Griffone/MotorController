/*

	Timer and timer-related code.

	by
		Grigory Glukhov
	
*/

#ifndef _HEAD_TIME
#define _HEAD_TIME

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

#endif // !_HEAD_TIME