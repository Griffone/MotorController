/*

	Motor controlls.
	
	by
		Aleksandra Soltan

*/

#ifndef _HEAD_MOTOR
#define _HEAD_MOTOR

#define MOTOR_FULL_REVOLUTION 48 //for full step
#define MOTOR_HALF_REVOLUTION 24 //for full step
#define MOTOR_CLOCKWISE 1
#define MOTOR_COUNTERCLOCKWISE 0
#define MOTOR_FULL_STEP 1
#define MOTOR_HALF_STEP 0

bit motor_direction;	// Should be either MOTOR_CLOCKWISE or MOTOR_COUNTERCLOCKWISE
bit motor_stepSize;		// Should be either MOTOR_FULL_STEP or MOTOR_HALF_STEP

// Initialize motor-related pins
void motor_init();

// Run a single step
void motor_step();

// This function is a delay for the motor to receive voltage change.
// This function is NOT defined in motor.c, as the exact timing routine might differ
void motor_delay();

#endif // !_HEAD_MOTOR