#define F_CPU 8000000UL
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t hx711H = 0;	 // Load Scale High Bits
uint16_t hx711L = 0; // Load Scale Low Bits

////////////////LCD display////////////////

#define LCD_Dir DDRB   /* Define LCD data port direction */
#define LCD_Port PORTB /* Define LCD data port */
#define RS PB0		   /* Define Register Select (data reg./command reg.) signal pin */
#define EN PB1		   /* Define Enable signal pin */

/////////////////////////define key pad////////////////////
#define KEY_PRT PORTC
#define KEY_DDR DDRC
#define KEY_PIN PINC

// Thremocuple
//  cofigure values for thermocouple
char array[10];
int ADC_value;

float hx = 0;
char buf[100];

float Temperature;
float measure_tempurature();
float loadCellRead();

bool valve_1_state = false;
bool valve_2_state = false;
bool valve_3_state = false;

bool heater_state = true;
bool buzzer_state = false;

void valve1();
void valve2();
void valve3();
void heater();
void buzzer();

// configure the thermocouple sensor
void ADC_Init()
{
	// DDRA &= ~(1<<3);    /* Make ADC port as input */
	DDRA &= 0b11111111;
	ADCSRA = 0x87; /* Enable ADC, fr/128  */
}

unsigned char keypad[4][4] = {
	{'7', '4', '1', 'C'},
	{'8', '5', '2', '0'},
	{'9', '6', '3', '='},
	{'/', 'x', '-', '+'}};

	unsigned char colloc, rowloc;

	char keyfind();
	void stepper();
	void stepper_1_time();

	int ADC_Read();

	void LCD_Command(unsigned char cmnd)
	{
		LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
		LCD_Port &= ~(1 << RS);						  /* RS=0, command reg. */
		LCD_Port |= (1 << EN);						  /* Enable pulse */
		_delay_us(1);
		LCD_Port &= ~(1 << EN);

		_delay_us(200);

		LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4); /* sending lower nibble */
		LCD_Port |= (1 << EN);
		_delay_us(1);
		LCD_Port &= ~(1 << EN);
		_delay_ms(2);
	}

	void LCD_Char(unsigned char data)
	{
		LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
		LCD_Port |= (1 << RS);						  /* RS=1, data reg. */
		LCD_Port |= (1 << EN);
		_delay_us(1);
		LCD_Port &= ~(1 << EN);

		_delay_us(200);

		LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
		LCD_Port |= (1 << EN);
		_delay_us(1);
		LCD_Port &= ~(1 << EN);
		_delay_ms(2);
	}

	void LCD_Init(void) /* LCD Initialize function */
	{
		LCD_Dir = 0xFF; /* Make LCD command port direction as o/p */
		_delay_ms(20);	/* LCD Power ON delay always >15ms */

		LCD_Command(0x33);
		LCD_Command(0x32); /* send for 4 bit initialization of LCD  */
		LCD_Command(0x28); /* Use 2 line and initialize 5*7 matrix in (4-bit mode)*/
		LCD_Command(0x0c); /* Display on cursor off*/
		LCD_Command(0x06); /* Increment cursor (shift cursor to right)*/
		LCD_Command(0x01); /* Clear display screen*/
		_delay_ms(2);
		LCD_Command(0x80); /* Cursor 1st row 0th position */
	}

	void LCD_String(char *str) /* Send string to LCD function */
	{
		int i;
		for (i = 0; str[i] != 0; i++) /* Send each char of string till the NULL */
		{
			LCD_Char(str[i]);
		}
	}

	void LCD_String_xy(char row, char pos, char *str) /* Send string to LCD with xy position */
	{
		if (row == 0 && pos < 16)
		LCD_Command((pos & 0x0F) | 0x80); /* Command of first row and required position<16 */
		else if (row == 1 && pos < 16)
		LCD_Command((pos & 0x0F) | 0xC0); /* Command of first row and required position<16 */
		LCD_String(str);					  /* Call LCD string function */
	}

	void LCD_Clear()
	{
		LCD_Command(0x01); /* Clear display */
		_delay_ms(2);
		LCD_Command(0x80); /* Cursor 1st row 0th position */
	}

	int main(void)
	{
		DDRD = 0xFF;
		//PORTD = 0x00;
		//DDRD |= (1 << 6);	// Load cell clock pin
		PORTD &= ~(1 << 6); // Clock pin low
		// DDRA &= 0b11111111;
		DDRA = 0xFF;
		PORTA = 0x00;
		
         //buzzer();
		 //heater();
		char buf[100];

		LCD_Init();
		LCD_Command(0x80);
		LCD_String("welcome");
		_delay_ms(2000);
		LCD_Clear();

		LCD_String("We are team 18");
		_delay_ms(3000);
		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("This is our");
		LCD_Command(0xc1);
		LCD_String("Kithul treacle");
		_delay_ms(3000);
		LCD_Clear();
		LCD_Command(0x80);
		LCD_String(" and jaggery ");
		LCD_Command(0xc1);
		LCD_String(" Making Machine");
		_delay_ms(3000);
		LCD_Clear();

		LCD_String("put sap ");
		LCD_Command(0xc1);
		LCD_String("to the container");
		_delay_ms(2000);

		while (1)
		{
			hx = loadCellRead();
			if (hx > 15)
			{
				break;
			}
			dtostrf(hx, 2, 2, buf);
			LCD_Clear();
			LCD_String(buf);
			_delay_ms(1000);
			
		}

		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("container filled");
		_delay_ms(2000);
		LCD_Clear();
		LCD_String("press 1");
        buzzer();
		valve1();

		while (1)
		{
			char a = keyfind();
			buzzer();
			if (a == '1')
			{
				LCD_Clear();
				LCD_Command(0x80);
				LCD_String("select option");
				break;
			}
		}

		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("press 1:treacle");
		LCD_Command(0xc1);
		LCD_String("press 2:juggery");

		while (1)
		{
			char a = keyfind();
			if (a == '1')
			{
				LCD_Clear();
				LCD_Command(0x80);
				LCD_String("processing....");
				heater();
				_delay_ms(2000);
				valve1();
				while (1)
				{
					stepper_1_time();
					Temperature = measure_tempurature();
					dtostrf(Temperature, 3, 2, array);
					LCD_Clear();

					if (Temperature > 200)
					{
						heater();
						LCD_String("treacle created");
						break;
					}
					LCD_String(array);
				}

				buzzer();
				LCD_Clear();
				LCD_String("press 1 to ");
				LCD_Command(0xc1);
				LCD_String("get treacle ");

				while (1)
				{
					a = keyfind();
					if (a == '1')
					{
						buzzer();
						valve2();
						LCD_Clear();
						LCD_Command(0x80);
						LCD_String("Valve is open");
						_delay_ms(2000);
						LCD_Clear();
						LCD_String("press 2 to");
						LCD_Command(0xc1);
						LCD_String("close valve");
					}
					
					if (a == '2')
					{
						valve2();
						LCD_Clear();
						LCD_Command(0x80);
						LCD_String("Thank You");
						_delay_ms(3000);
						break;
					}
					
				}
			}
			else if (a == '2')
			{
				LCD_Clear();
				LCD_Command(0x80);
				LCD_String("processing...");
				heater();
				_delay_ms(2000);
				valve1();
				while (1)
				{
					stepper_1_time();
					Temperature = measure_tempurature();
					dtostrf(Temperature, 3, 2, array);
					LCD_Clear();

					if (Temperature > 200)
					{
						heater();
						LCD_String("Jaggery created");
						buzzer();
						break;
					}
					LCD_String(array);
				}
				_delay_ms(2000);
				buzzer();
				LCD_Clear();
				LCD_String("press 1 to");
				LCD_Command(0xc1);
				LCD_String("take Jaggery out");

				while (1)
				{

					a = keyfind();
					if (a == '1')
					{
						buzzer();
						valve3();
						LCD_Clear();
						LCD_Command(0x80);
						LCD_String("Valve is open");
						_delay_ms(3000);
						LCD_Clear();
						LCD_String("press 2 to"); 
						LCD_Command(0xc1);
						LCD_String("close valve");
						
					}
					if (a == '2')
					{
						valve3();
						LCD_Clear();
						LCD_Command(0x80);
						LCD_String("Thank You");
						_delay_ms(3000);
						break;
					}
				}
			}
		}
	}

	char keyfind()
	{
		while (1)
		{
			KEY_DDR = 0xF0; /* set port direction as input-output */
			KEY_PRT = 0xFF;
			do
			{
				KEY_PRT &= 0x0F; /* mask PORT for column read only */
				asm("NOP");
				colloc = (KEY_PIN & 0x0F); /* read status of column */
			} while (colloc != 0x0F);
			do
			{
				do
				{
					_delay_ms(20);			   /* 20ms key debounce time */
					colloc = (KEY_PIN & 0x0F); /* read status of column */
					} while (colloc == 0x0F);	   /* check for any key press */
					_delay_ms(40);				   /* 20 ms key debounce time */
					colloc = (KEY_PIN & 0x0F);
				} while (colloc == 0x0F);
				/* now check for rows */
				KEY_PRT = 0xEF; /* check for pressed key in 1st row */
				asm("NOP");
				colloc = (KEY_PIN & 0x0F);
				if (colloc != 0x0F)
				{
					rowloc = 0;
					break;
				}
				KEY_PRT = 0xDF; /* check for pressed key in 2nd row */
				asm("NOP");
				colloc = (KEY_PIN & 0x0F);
				if (colloc != 0x0F)
				{
					rowloc = 1;
					break;
				}
				KEY_PRT = 0xBF; /* check for pressed key in 3rd row */
				asm("NOP");
				colloc = (KEY_PIN & 0x0F);
				if (colloc != 0x0F)
				{
					rowloc = 2;
					break;
				}
				KEY_PRT = 0x7F; /* check for pressed key in 4th row */
				asm("NOP");
				colloc = (KEY_PIN & 0x0F);
				if (colloc != 0x0F)
				{
					rowloc = 3;
					break;
				}
			}
			if (colloc == 0x0E)
			return (keypad[rowloc][0]);
			else if (colloc == 0x0D)
			return (keypad[rowloc][1]);
			else if (colloc == 0x0B)
			return (keypad[rowloc][2]);
			else
			return (keypad[rowloc][3]);
		}

		void stepper()
		{
			// All pins of PORTC as output                //Initially all pins as output high
			while (1)
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
		void stepper_1_time()
		{
			// All pins of PORTC as output                //Initially all pins as output high

			PORTA = 0b00010010;
			_delay_ms(100);
			PORTA = 0b00000110;
			_delay_ms(100);
			PORTA = 0b00001100;
			_delay_ms(100);
			PORTA = 0b00011000;
			_delay_ms(100);
		}

		int ADC_Read()
		{
			ADMUX = 0x40;		   /* Vref: Avcc, ADC channel: 0 */
			ADCSRA |= (1 << ADSC); /* Start conversion */
			while ((ADCSRA & (1 << ADIF)) == 0)
			;				   /*monitor end of conversion interrupt flag */
			ADCSRA |= (1 << ADIF); /* Set the ADIF bit of ADCSRA register */
			return (ADCW);		   /* Return the ADCW */
		}

		float measure_tempurature()
		{

			ADC_Init(); /* Initialize the ADC */

			ADC_value = ADC_Read(); /* store the analog data on a variable */

			/* convert analog voltage into ?C and subtract the offset voltage */

			// Temperature = (((ADC_value * 4.88)-0.0027) / 10.0)-26;
			// dtostrf(Temperature, 3, 2, array);

			return Temperature = (((ADC_value * 4.88) - 0.0027) / 10.0) - 26;
		}

		float loadCellRead()
		{
			hx711H = 0;
			hx711L = 0; // clear variables
			for (uint8_t i = 0; i < 8; i++)
			{					   // Load cell data high 8 bits
				PORTD |= (1 << 6); // Clock pin high
				_delay_us(10);
				if ((PIND & (1 << 5)) >> 5) // read data pin
				{
					hx711H |= (1 << (7 - i)); // set hx 711 varible
				}
				else
				{
					hx711H &= ~(1 << (7 - i));
				}
				PORTD &= ~(1 << 6); // Clock pin low
				_delay_us(5);
			}

			for (uint8_t i = 0; i < 16; i++)
			{					   // Load cell data low 16 bits
				PORTD |= (1 << 6); // Clock pin high
				_delay_us(10);
				if ((PIND & (1 << 5)) >> 5) // read data pin
				{
					hx711L |= (1 << (15 - i));
				}
				else
				{
					hx711L &= ~(1 << (15 - i));
				}
				PORTD &= ~(1 << 6); // Clock pin low
				_delay_us(5);
			}

			hx711L = hx711L >> 1; // shift bits

			if (hx711H & 1) // bit setup
			{
				hx711L |= (1 << 15);
			}
			else
			{
				hx711L &= ~(1 << 15);
			}
			hx711H = hx711H >> 1;

			return (hx711H * (65536 / 18029.6)) + hx711L / 18029.6; // load cell calibration
		}

		void valve1()
		{

			if (valve_1_state == false)
			{
				PORTA |= (1 << PINA5);
				valve_1_state = true;
			}
			else
			{
				PORTA &= ~(1 << PINA5);
				valve_1_state = false;
			}
		}

		void valve2()
		{

			if (valve_2_state == false)
			{
				PORTA |= (1 << PINA6);
				valve_2_state = true;
			}
			else
			{
				PORTA &= ~(1 << PINA6);
				valve_2_state = false;
			}
		}

		void valve3()
		{

			if (valve_3_state == false)
			{
				PORTA |= (1 << PINA7);
				valve_3_state = true;
			}
			else
			{
				PORTA &= ~(1 << PINA7);
				valve_3_state = false;
			}
		}
		void heater()
		{

			if (heater_state == true)
			{
				PORTD |= (1 << PIND0);
				heater_state = false;
			}
			else
			{
				PORTD &= ~(1 << PINA0);
				heater_state = true;
			}
		}

		void buzzer()
		{

			if (buzzer_state == false)
			{
				PORTD |= (1 << PIND1);
				buzzer_state = true;
			}
			else
			{
				PORTD &= ~(1 << PIND1);
				buzzer_state = false;
			}
		}