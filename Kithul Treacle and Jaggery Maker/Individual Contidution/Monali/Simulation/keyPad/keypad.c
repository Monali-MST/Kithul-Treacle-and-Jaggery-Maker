#define F_CPU 8000000UL
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>


////////////////LCD display////////////////

#define LCD_Dir DDRB					/* Define LCD data port direction */
#define LCD_Port PORTB					/* Define LCD data port */
#define RS PB0							/* Define Register Select (data reg./command reg.) signal pin */
#define EN PB1 							/* Define Enable signal pin */

/////////////////////////define key pad////////////////////
#define KEY_PRT PORTC
#define KEY_DDR DDRC
#define KEY_PIN PINC

unsigned char keypad[4][4] = {
	{'7', '4', '1', 'C'},
	{'8', '5', '2', '0'},
	{'9', '6', '3', '='},
	{'/', 'x', '-', '+'}};

	unsigned char colloc, rowloc;

	char keyfind();

void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);				/* RS=0, command reg. */
	LCD_Port |= (1<<EN);				/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);				/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)					/* LCD Initialize function */
{
	LCD_Dir = 0xFF;						/* Make LCD command port direction as o/p */
	_delay_ms(20);						/* LCD Power ON delay always >15ms */

	LCD_Command(0x33);
	LCD_Command(0x32);		    		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              	/* Use 2 line and initialize 5*7 matrix in (4-bit mode)*/
	LCD_Command(0x0c);              	/* Display on cursor off*/
	LCD_Command(0x06);              	/* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              	/* Clear display screen*/
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

void LCD_String (char *str)				/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)				/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_String(str);					/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);					/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

int main(void)
{
	LCD_Init();
	LCD_Command(0xc1);
	LCD_String("welcome");
	_delay_ms(1000);
	LCD_Clear();

	 while (1)
	 {
		char a = keyfind();
		LCD_Char(a);
		_delay_ms(1000);
		LCD_Clear();
		LCD_Clear();
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
				_delay_ms(20);             /* 20ms key debounce time */
				colloc = (KEY_PIN & 0x0F); /* read status of column */
				} while (colloc == 0x0F);      /* check for any key press */
				_delay_ms(40);                 /* 20 ms key debounce time */
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


