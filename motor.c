#ifndef _SOURCE_MOTOR
#define _SOURCE_MOTOR

void motor_init() {
	TRISC &= ~0b111;	// '11111000' for outputs at motor-related pins
	PORTC &= ~0b111;	// 'xxxxx000' for initial value
	
}

void motor_step() {
	PORTC.0 = motor_stepSize;
	PORTC.1 = motor_direction;
	PORTC.2 = 1;
	motor_delay();
	PORTC.2 = 0;
}

#endif // !_SOURCE_MOTOR