#ifndef _SOURCE_IO
#define _SOURCE_IO

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

#endif // !_SOURCE_IO