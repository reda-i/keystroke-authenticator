/*
* Microprocessors Project.c
*
* Created: 2018-11-04 8:26:29 PM
* Author : Army-O
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <math.h>
#define KEYBOARD_PIN_NAME PINC
#define KEYBOARD_PIN_NUM 0
#define INPUT_CHAR_SIZE 30
unsigned char isStarted = 0;
unsigned char *eepromStart = (unsigned char *)0x0;
unsigned char keyboardData = 0;						// the keyboard code being received
unsigned char bitCount = 0;							// The number of the bit coming from the keyboard
volatile unsigned char inputChars[INPUT_CHAR_SIZE]; // A buffer holding the input keys from the keyboard
volatile int inputCharsHead = 0;					// counting the index of the next character to leave the buffer
volatile int inputCharsTail = 0;					// counting the index of the next character to get into the buffer
unsigned char offsetCount = 0;
unsigned char nextIgnored = 0;
unsigned char shiftPressed = 0;
/*A function that adds a key to the buffer*/
void addKeyToBuffer(char key)
{
	inputChars[inputCharsTail++] = key;
	if (inputCharsTail >= INPUT_CHAR_SIZE)
	{ // once you pass the buffer size, reset the index to 0
		inputCharsTail = 0;
	}

	if (inputCharsTail == 10)
	{
		for (int i = 0; i < 10; i++)
		{
			eeprom_write_byte(eepromStart++, inputChars[i]);
		}
		inputCharsTail = 0;
	}
}

char getChar()
{
	while (inputCharsHead == inputCharsTail)
		;
	char key = inputChars[inputCharsHead++];
	if (inputCharsHead >= INPUT_CHAR_SIZE)
	{ // once you pass the buffer size, reset the index to 0
		inputCharsHead = 0;
	}
	return key;
}

/* Keyboard config and functions */
void initKeyboard()
{
	GICR |= (1 << INT0);
	MCUCR |= (1 << ISC01); // set MCUCR to 2 to accept requests coming from INT0 on the falling edge
	DDRC &= ~(1 << DDC0);
	DDRD &= ~(1 << DDD2);
	DDRD |= (1 << DDD5) | (1 << DDD6);
}

void decodeKeys(unsigned char data)
{
	if (offsetCount < 7) // the first 7 bytes are sent from the keyboard on startup, so we discard
		offsetCount++;
	else if (nextIgnored == 0) // If I detect a keyboard break code, I ignore It. We only process make codes.
		if (shiftPressed)
		{ // In case the user pressed a shift pass a capital R or pass invalid character
			if (data == 0x2D)
				addKeyToBuffer('R');
			else
				addKeyToBuffer('0');
			shiftPressed = 0;
		}
		else
			switch (data) // Decode only the relevant characters, and send 0 otherwise
			{
			case 0xF0:			 // In case I received a 0xFO that mean I am processing a break code
				nextIgnored = 1; // since this is a break code, I ignore the next byte.
				break;
			case 0x12:
				shiftPressed = 1;
				break;
			case 0x59:
				shiftPressed = 1;
				break;
			case 0x49:
				addKeyToBuffer('.');
				break; // char is "."
			case 0x2C:
				addKeyToBuffer('t');
				break; // char is "t"
			case 0x43:
				addKeyToBuffer('i');
				break; // char is "i"
			case 0x24:
				addKeyToBuffer('e');
				break; // char is "e"
			case 0x2E:
				addKeyToBuffer('5');
				break; // char is "5"
			case 0x2D:
				addKeyToBuffer('r');
				break; // char is "r"
			case 0x44:
				addKeyToBuffer('o');
				break; // char is "o"
			case 0x31:
				addKeyToBuffer('n');
				break; // char is "n"
			case 0x1C:
				addKeyToBuffer('a');
				break; // char is "a"
			case 0x4B:
				addKeyToBuffer('l');
				break; // char is "l"
			default:
				addKeyToBuffer('0');
				break; // add null character to buffer in order to discard it and reset the trial
			}
	else
		nextIgnored = 0;
}

/*
interrupt service routine for handling the keyboard input on INT0
INT0 will be connected to the keyboard clock
*/

// set bitCount to 0
ISR(INT0_vect)
{

	if (isStarted)
	{
		// perform the reading operations normally, expect 10 bits
		bitCount++;
		if (bitCount <= 8)
		{
			keyboardData = (keyboardData >> 1);
			// add bit to keyboard key
			if (PINC & 0x01)
				keyboardData |= 0x80;
			else
				keyboardData &= 0x7F;
			// shift for next bit
		}
		else
		{
			// done with 8 bits and with parity and stop
			if (bitCount == 10)
			{
				isStarted = 0;
				bitCount = 0;

				decodeKeys(keyboardData);
			}
		}
	}
	else
	{
		// the start signal has arrived
		if ((PINC & 0x01) == 0)
		{
			// set start signal
			isStarted = 1;
			// reset data
			keyboardData = 0;
		}
		// if the start signal has not arrived, don't do anything
	}
}

/* LED config and functions*/
void initLED()
{
	//TODO: set output ports config for the LEDs
}

int main(void)
{
	initKeyboard();
	sei();
	while (1)
	{
	}
}
