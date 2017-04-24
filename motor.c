#ifndef _SOURCE_MOTOR
#define _SOURCE_MOTOR

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

#endif // !_SOURCE_MOTOR