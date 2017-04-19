// Standard libraries
#include "deps/16F690.h"
#include "deps/int16Cxx.h"

// Preprocessor pragmas for correct memory usage
#pragma config |= 0x00D4 
#pragma char X @ 0x115

// Local definitions
#define FULL_REVOLUTION 48 //for full step
#define HALF_REVOLUTION 24 //for full step
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0
#define FULL_STEP 1
#define HALF_STEP 0

// Function defenitions
void delay10(char);
void choose_steps(int);
void choose_direction(int);
void choose_speed(int);
void init_io_ports( void );
void init_serial( void );
void init_interrupt( void );
void putchar( char);
void printf(const char *string, char variable);

bit receiver_flag;   /* Signal-flag used by interrupt routine   */
char receiver_byte;  /* Transfer Byte used by interrupt routine */

#pragma origin 4
interrupt int_server( void ) /* the place for the interrupt routine */
{
  int_save_registers
  /* New interrupts are automaticaly disabled            */
  /* "Interrupt on change" at pin RA1 from PK2 UART-tool */
  
  if( PORTA.1 == 0 )  /* Interpret this as the startbit  */
    {  /* Receive one full character   */
      char bitCount, ti;
      /* delay 1,5 bit 156 usec at 4 MHz         */
      /* 5+28*5-1+1+2+9=156 without optimization */
      ti = 28; do ; while( --ti > 0); nop(); nop2();
      for( bitCount = 8; bitCount > 0 ; bitCount--)
       {
         Carry = PORTA.1;
         receiver_byte = rr( receiver_byte);  /* rotate carry */
         /* delay one bit 104 usec at 4 MHz       */
         /* 5+18*5-1+1+9=104 without optimization */ 
         ti = 18; do ; while( --ti > 0); nop(); 
        }
      receiver_flag = 1; /* A full character is now received */
    }
  RABIF = 0;    /* Reset the RABIF-flag before leaving   */
  int_restore_registers
  /* New interrupts are now enabled */
}


void main(void){
  
  init_io_ports();
  init_serial();
  init_interrupt();
  
  delay10(5000); 
  printf("Begin",0);
  
  int steps = 15;
  choose_direction(COUNTERCLOCKWISE);
  printf("counterclockwise",0);
  printf("before delay %b\r\n", (char) PORTC);
  delay10(5000);
  printf("after delay %b\r\n", (char) PORTC);
  choose_steps(HALF_REVOLUTION);
  
  choose_direction(CLOCKWISE);
  printf("clockwise",0);
  printf("before delay %b\r\n", (char) PORTC);
  delay10(5000);
  printf("after delay %b\r\n", (char) PORTC);
  choose_steps(HALF_REVOLUTION);
  
  delay10(5000);
  choose_speed(HALF_STEP);
  choose_steps(HALF_REVOLUTION * 2);

  delay10(5000);
  choose_speed(FULL_STEP);
  choose_steps(HALF_REVOLUTION);
 }


/* *********************************** */
/*            FUNCTIONS                */
/* *********************************** */


void delay10( char n)
/*
  Delays a multiple of 10 milliseconds using the TMR0 timer
  Clock : 4 MHz   => period T = 0.25 microseconds
  1 IS = 1 Instruction Cycle = 1 microsecond
  error: 0.16 percent
*/
{
    char i;

    OPTION = 7;
    do  {
        i = TMR0 + 39; /* 256 microsec * 39 = 10 ms */
        while ( i != TMR0)
            ;
    } while ( --n > 0);
}

void choose_steps(int numberOfSteps){
	int i = 0;
	while(i < numberOfSteps){
       delay10(10);
	   printf("Step\r\n",0);
	   //PORTC.0 = 1; /* PORTC pin 0 "1" */
	   //PORTC = PORTC | 0x01;
	   PORTC = PORTC | 0x04;
	   printf("%b\r\n", (char) PORTC);
	   delay10(10);
	   //PORTC.0 = 0; /* PORTC pin 0 "0" */
	   //PORTC = PORTC & 0xfe;
	   PORTC = PORTC & 0xfb;
	   printf("%b\r\n", (char) PORTC);
	   i++;
    }
}

void choose_direction(int directionValue){
	if(directionValue == COUNTERCLOCKWISE){
		//PORTC.1 = 1;
		PORTC = PORTC | 0x02;
	}
	else{
		//PORTC.1 = 0;
		PORTC = PORTC & 0xfd;
	}
}

void choose_speed(int speedValue){
	if(speedValue == FULL_STEP){
		//PORTC = PORTC | 0x04;
		PORTC = PORTC | 0x01;
	}
	else{
		//PORTC = PORTC & 0xfb;
		PORTC = PORTC & 0xfe;
	}
}


void init_io_ports( void )
{
  TRISC = 0xF8; /* 11111000 0 is for outputbit  */
  PORTC = 0b000;    /* initial value */

  ANSEL =0;     /* not AD-input      */
  TRISA.5 = 1;  /* input rpgA        */
  TRISA.4 = 1;  /* input rpgB        */
  /* Enable week pullup's            */
  OPTION.7 = 0; /* !RABPU bit        */
  WPUA.5   = 1; /* rpgA pullup       */
  WPUA.4   = 1; /* rpgB pullup       */
  X.6 = 1;
  TRISB.6 = 1;  /* PORTB pin 6 input */
  
  return;
}

void init_serial( void )  /* initialise PIC16F690 bitbang serialcom */
{
   ANSEL.0 = 0; /* No AD on RA0             */
   ANSEL.1 = 0; /* No AD on RA1             */
   PORTA.0 = 1; /* marking line             */
   TRISA.0 = 0; /* output to PK2 UART-tool  */
   TRISA.1 = 1; /* input from PK2 UART-tool */
   receiver_flag = 0 ;
   return;      
}

void init_interrupt( void )
{
  IOCA.1 = 1; /* PORTA.1 interrupt on change */
  RABIE =1;   /* interrupt on change         */
  GIE = 1;    /* interrupt enable            */
  return;
}

void putchar( char ch )  /* sends one char */
{
  char bitCount, ti;
  PORTA.0 = 0; /* set startbit */
  for ( bitCount = 10; bitCount > 0 ; bitCount-- )
   {
     /* delay one bit 104 usec at 4 MHz       */
     /* 5+18*5-1+1+9=104 without optimization */ 
     ti = 18; do ; while( --ti > 0); nop(); 
     Carry = 1;     /* stopbit                    */
     ch = rr( ch ); /* Rotate Right through Carry */
     PORTA.0 = Carry;
   }
  return;
}

void printf(const char *string, char variable)
{
  char i, k, m, a, b;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') break;   // at end of string
     if( k == '%')           // insert variable in string
      {
        i++;
        k = string[i];
        switch(k)
         {
           case 'd':         // %d  signed 8bit
             if( variable.7 ==1) putchar('-');
             else putchar(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putchar('0'+a); // print 100's
             b = variable%100;
             a = b/10;
             putchar('0'+a); // print 10's
             a = b%10;
             putchar('0'+a); // print 1's
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putchar('1');
                else putchar('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putchar(variable);
             break;
           case '%':
             putchar('%');
             break;
           default:          // not implemented
             putchar('!');
         }
      }
      else putchar(k);
   }
}

/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */

/*           _____________  _____________ 
            |             \/             |
      +5V---|Vdd        16F690        Vss|---Gnd
			|RA5            RA0/AN0/(PGD)|bbTx->- PK2 UART-tool
			|RA4/AN3            RA1/(PGC)|bbRx-<- PK2 UART-tool
            |RA3/!MCLR/(Vpp)  RA2/AN2/INT|
            |RC5/CCP                  RC0|->-!STEP
            |RC4                      RC1|->-DIR
            |RC3/AN7                  RC2|->-!HSM
            |RC6/AN8             AN10/RB4|
            |RC7/AN9               RB5/Rx|
            |RB7/Tx                   RB6|
            |____________________________|                                      
*/ 