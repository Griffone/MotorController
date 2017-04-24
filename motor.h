/*

	Motor controlls.
	
	by
		Aleksandra Soltan

*/

#ifndef _HEAD_MOTOR
#define _HEAD_MOTOR

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

#endif // !_HEAD_MOTOR