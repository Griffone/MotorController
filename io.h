/*

	Everything IO related.
	Provides mid-level communication services over UART.

	by
		Grigory Glukhov
		
*/

#ifndef _HEAD_IO
#define _HEAD_IO

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

#endif // !_HEAD_IO