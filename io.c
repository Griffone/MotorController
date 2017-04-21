#ifndef _SOURCE_IO
#define _SOURCE_IO

char io_inputIsValid;
char io_input;
char io_in[IO_SIZE_IN];
const char * io_out;

void io_init() {
	
	io_out = "";	// This will stop io_update from outputting random garbage
	io_input = 0;
	io_inputIsValid = 0;
	
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
	TXEN = 1;	// enable UART circuitry
	SYNC = 0;	// set it to asynchrounous transmishion
	
	
	CREN = 1;	// enable reciever circuitry
	RX9 = 0;	// use 8-bit characters
	
	SPEN = 1;	// enable EUSART and automatically configure TX/CK as output
}

void io_update() {
	
	// Transmitting data
	if (*io_out && TRMT) {	// if io_out points to non-null char
		TXREG = *io_out;
		io_out++;
	}
	
	CREN = 1;	// Just in case an error occured
	// Receiveing data
	if (RCIF) {
		if (io_input >= IO_SIZE_IN)
			io_input = 0;	// silently wrap around, bad practice but functional

		if (io_input < IO_SIZE_IN) {
			io_in[io_input] = RCREG;
			if (io_in[io_input] == '\0' || io_in[io_input] == '\n' || io_in[io_input] == '\r')
				io_inputIsValid = 1;
			io_input++;
		}
	}
}

void io_print(const char * s) {
	io_out = s;
	
	while (*s) {
		while(!TRMT);
		TXREG = *s;
		s++;
	}
}

bit strcmp(const char * a, const char * b) {
	char ac, bc;
	while (*a && *b) {
		ac = *a;
		bc = *b;
		if (ac != bc)
			return 0;	// Hit a different char
		a++;
		b++;
	}
	return 1;	// Hit a null-char
}

#endif // !_SOURCE_IO