/*
* Microprocessors Project.c
*
* Created: 2018-11-04 8:26:29 PM
* Author : Army-O
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>
#include <avr/eeprom.h>
#include <math.h>
#define KEYBOARD_PIN_NAME PINC
#define KEYBORD_PIN_NUM 0
#define INPUT_CHAR_SIZE 30
unsigned char* eepromStart = 0x5;
unsigned char keyboardData = 0; // the keyboard code being received
unsigned char bitCount = 11; // The number of the bit coming from the keyboard
unsigned char inputChars [INPUT_CHAR_SIZE]; // A buffer holding the input keys from the keyboard
int inputCharsHead = 0; // counting the index of the next character to leave the buffer
int inputCharsTail = 0; // counting the index of the next character to get into the buffer

/*A function that adds a key to the buffer*/
void addKeyToBuffer(char key){
	inputChars[inputCharsTail++] = key;
	if(inputCharsTail >= INPUT_CHAR_SIZE){  // once you pass the buffer size, reset the index to 0
		inputCharsTail = 0;
	}
}


char getChar(){
	while(inputCharsHead == inputCharsTail);	
	char key = inputChars[inputCharsHead++];
	if(inputCharsHead >= INPUT_CHAR_SIZE){  // once you pass the buffer size, reset the index to 0
		inputCharsHead = 0;
	}
	return key;
}

/* Keyboard config and functions */
void init_keyboard(){
	MCUCR |= (1 << ISC01);// set MCUCR to 2 to accept requests coming from INT0 on the falling edge 
	DDRC &= ~(1 << DDC2);
	DDRD &= ~(1 << DDD2) | (1 << DDD5) | (1 << DDD6);
	GICR |= (1 << INT0);
	sei();
}


void decodeKeys(unsigned char data){
	PORTD |= (1 << PORTD5);
	_delay_ms(10000);
	PORTD &= (0xff ^ (1 << PORTD5));
	switch(data){
		case 0x49: addKeyToBuffer('.'); break; // char is "."
		case 0x2c: addKeyToBuffer('t'); break; // char is "t"
		case 0x42: addKeyToBuffer('i'); break; // char is "i"
		case 0x24: addKeyToBuffer('e'); break; // char is "e"
		case 0x73: addKeyToBuffer('5'); break; // char is "5"
		case 0x2d: addKeyToBuffer('r'); break; // char is "r"
		case 0x44: addKeyToBuffer('o'); break; // char is "o"
		case 0x31: addKeyToBuffer('n'); break; // char is "n"
		case 0x1c: addKeyToBuffer('a'); break; // char is "a"
		case 0x4b: addKeyToBuffer('l'); break; // char is "l"
		default:   addKeyToBuffer('0'); break; // add null character to buffer in order to discard it and reset the trial
	}
}

/*	
interrupt service routine for handling the keyboard input on INT0
INT0 will be connected to the keyboard cl
*/

ISR(INT0_vect)
{
	//eeprom_write_byte(eepromStart++, (unsigned char)bitCount);
	//eeprom_write_byte(eepromStart++, (unsigned char)(KEYBOARD_PIN_NAME & (1 << KEYBORD_PIN_NUM)));
	PORTD ^= (1 << PORTD6);
	if(bitCount < 11 && bitCount > 2) //intercept data bits
	{
		keyboardData = (keyboardData >> 1); // empty a space for the next data bit in keyboard data
		if(KEYBOARD_PIN_NAME & (1 << KEYBORD_PIN_NUM)){ //if the keyboard input is a 1, store a 1 in the empty space
			keyboardData |= 0x80; // set the most significant bit in keyboardData to 1 
		}
	}
	if(--bitCount == 0){
		decodeKeys(keyboardData);
		bitCount = 11;
	}
	return;
	//eeprom_write_byte(eepromStart++, (unsigned char)(DDRC));
	//eeprom_write_byte(eepromStart++, (unsigned char)(DDRD));
	//eeprom_write_byte(eepromStart++, (unsigned char)(SREG));
	//eeprom_write_byte(eepromStart++, (unsigned char)(GICR));
	//eeprom_write_byte(eepromStart++, (unsigned char)(GIFR));
	//eeprom_write_byte(eepromStart++, (unsigned char)(MCUCR));
	//eeprom_write_byte(eepromStart++, (unsigned char)(MCUCSR));
}

/* LED config and functions*/
void init_LED(){
	//TODO: set output ports config for the LEDs
}


int main(void)
{
	init_keyboard();

	while (1){
		if((PIND & (1 << PIND2)) == 1){
			eeprom_write_byte(eepromStart++, 0xaa);	
		}
	}
}

