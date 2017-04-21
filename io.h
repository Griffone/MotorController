/*

	Everything IO related.
	Provides mid-level communication services over UART.

	by
		Grigory Glukhov
		
*/

#ifndef _HEAD_IO
#define _HEAD_IO

#pragma bit IF_IO @ RABIF

// Max size of the input string (should be of length |max legal command| + 1)
#define IO_SIZE_IN 16

// An input string buffer, is to be used by parsing functions
extern char io_in[IO_SIZE_IN];	

// An output string buffer pointer. As long as it's a non-null char io_update will continue to output it
extern const char * io_out;

// Boolean value, that becomes one once io_in contains a legitimate string
extern char io_inputIsValid;

// Initialize everything IO-related (uart ports, control registers and interrupts)
void io_init();

// Will get any incoming characters and push any outgoing ones
void io_update();

// Returns 1 if strings are the same until one of them becomes a null-char
// Will always return 1 if compares a null pointer to anything
bit strcmp(const char * a, const char * b);

// Simply prints the given string
void io_print(const char *);

// Prints the string replacing %x sign with formatted variable
// Supported variables:
// 	none
void io_printf(const char * str, char var);

#endif // !_HEAD_IO