
#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

void stepper();

int main(void)
{
	DDRA = 0xFF;
	PORTA = 0x00;

    /* Replace with your application code */
    stepper();
}

void stepper(){
               //All pins of PORTC as output                //Initially all pins as output high
	  while(1)
	  {
		    PORTA = 0b00010010;
		    _delay_ms(100);
		    PORTA = 0b00000110;
		    _delay_ms(100);
		    PORTA = 0b00001100;
		    _delay_ms(100);
		    PORTA = 0b00011000;
		    _delay_ms(100);
	  }
}



