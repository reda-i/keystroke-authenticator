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
#define INPUT_CHAR_SIZE 30 // the size of the input chars buffer
#define USER_SWITCH 0
#define TEST_SWITCH 1

unsigned char isStarted = 0;						// the flag that indicates whether a start bit was received or not
unsigned char *eepromStart = (unsigned char *)0x0;  // pointer used for the eeprom saves while testing
unsigned char keyboardData = 0;						// the keyboard code being received
unsigned char bitCount = 0;							// The number of the bit coming from the keyboard
volatile unsigned char inputChars[INPUT_CHAR_SIZE]; // A buffer holding the input keys from the keyboard
volatile int inputCharsHead = 0;					// counting the index of the next character to leave the buffer
volatile int inputCharsTail = 0;					// counting the index of the next character to get into the buffer
unsigned char offsetCount = 0;						// offset for the first 7 bytes sent by the keyboard on startup
unsigned char nextIgnored = 0;						// the flag indicates having a keyboard break code to ignore
unsigned char shiftPressed = 0;						// the flag indicates whether the user is pressing shift or not
unsigned char *password = ".tie5Ronal";

/*A function that adds a key to the buffer*/
void addKeyToBuffer(char key)
{
	inputChars[inputCharsTail++] = key;
	if (inputCharsTail >= INPUT_CHAR_SIZE)
	{ // once you pass the buffer size, reset the index to 0
		inputCharsTail = 0;
	}
}

unsigned char getChar()
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

/*--------------------------------------------------- END OF KEYBOARD CONTROL LOGIC ------------------------------------------------------*/

volatile unsigned long timerOverflowHolder = 0;

/*
* This array holds the value of the timer when each key is pressed.
* Location 0 holds the time for pressing "."
* Location 9 holds the time for pressing "l"
* The second dimension of the array is for the 5 trials.
* For example, userKeyTimestamps[0][0] corresponds to the timestamp for "." in trial 1.
*/
unsigned long userKeyTimestamps[10][5];
unsigned long testKeyTimestamps[10];
/*
* These two arrays hold the final vector for the two users.
*/
double userAVector[9];
double userBVector[9];
double testVector[9];

/*
* Configures the 16-bit timer 1 to work in mode 0
* Enables interrupts for overflow consideration
*/
void configureTimer()
{

	// set the timer to normal mode by clearing all WGM10 to WGM13
	TCCR1A &= ~(1 << WGM10) & ~(1 << WGM11) & ~(1 << WGM12) & ~(1 << WGM13);

	// enable interrupt on timer 1 overflow
	TIMSK = (1 << TOIE1);

	// enable global interrupt
	sei();
}

/*
* Stops the timer.
* The timer needs to be restarted again by a call to startTimer.
*/
void stopTimer()
{

	// stop the timer by clearing CS10 and CS12
	TCCR1B &= ~(1 << CS10) & ~(1 << CS12);
}

/*
* Resets timer and starts it from zero
* by setting the frequency to freq
*/
void startTimer()
{

	// clear the timer contents
	TCNT1 = 0;
	timerOverflowHolder = 0;

	// set timer clock frequency to freq and start
	TCCR1B |= (1 << CS10);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS12);
}

/*
* On overflow, increment the overflow count
*/
ISR(TIMER1_OVF_vect)
{
	timerOverflowHolder++;
}

/* Calculates the euclidean distance
* between the test vector and the stored user vector
*/
double euclideanDistance(double testSubject[], double user[])
{
	double sum = 0.0;

	for (int i = 0; i < 10; i++)
	{
		sum += pow((testSubject[i] - user[i]), 2.0);
	}

	return sqrt(sum);
}

/*
* Pass 'A' for user A and pass 'B' for user B
* Call this method after all five trials for a particular user are complete.
*/
void calculateUserVector(char user)
{

	unsigned long differencesVector[9][5];

	// calculate user differences vector
	for (int i = 0; i < 9; i++)
	{ // iterate over timestamps
		for (int j = 0; j < 5; j++)
		{																					 // iterate over trials
			differencesVector[i][j] = userKeyTimestamps[i + 1][j] - userKeyTimestamps[i][j]; // calculate difference
		}
	}

	// average the differences and store the values
	for (int i = 0; i < 9; i++)
	{ // iterate over differences

		unsigned long sum = 0;

		for (int j = 0; j < 5; j++)
		{ // iterate over trials
			sum += differencesVector[i][j];
		}

		double averageDifference = sum / 5.0;

		// store the average of the trials as a feature
		if (user == 'A')
		{
			userAVector[i] = averageDifference;
		}
		else 
		{
			userBVector[i] = averageDifference;
		}
	}
}

void calculateTestVector() {
	for (int i = 0; i < 9; i++) {
		testVector[i] = testKeyTimestamps[i + 1] - testKeyTimestamps[i];
	}
}

/* LED config and functions*/
void initLED()
{
	//TODO: set output ports config for the LEDs
}

int main(void)
{
	initLED();
	initKeyboard();
	configureTimer();
	sei();
	while (1)
	{
		// Training Phase
		// User A
		while(PINB & (1 << USER_SWITCH));										// User A = PINB[USER_SWITCH] = 0
		startTimer();
		for(int i = 0; i < 5; i++) {
			for(int j = 0; j < 10; j++) {
				unsigned char character = getChar();							// Reading Entered Character
				
				if(password[j] != character) {									// Checking Character With Saved Password
					j = -1;
					continue;
				}
				
				userKeyTimestamps[i][j] = (timerOverflowHolder << 16) | TCNT1;	// Saving Timestamps
			}
		}
		stopTimer();
		calculateUserVector('A');												// Calculating User A Vector
		
		// User B
		while(!(PINB & (1 << USER_SWITCH)));									// User B = PINB[USER_SWITCH] = 1
		startTimer();
		for(int i = 0; i < 5; i++) {
			for(int j = 0; j < 10; j++) {
				unsigned char character = getChar();							// Reading Entered Character
				
				if(password[j] != character) {									// Checking Character With Saved Password
					j = -1;
					continue;
				}
				
				userKeyTimestamps[i][j] = (timerOverflowHolder << 16) | TCNT1;	// Saving Timestamps
			}
		}
		stopTimer();
		calculateUserVector('B');												// Calculating User B Vector
		
		// Testing Phase
		while(!(PINB & (1 << USER_SWITCH)));
		startTimer();
		for(int j = 0; j < 10; j++) {
			unsigned char character = getChar();							// Reading Entered Character
			
			if(password[j] != character) {									// Checking Character With Saved Password
				j = -1;
				continue;
			}
			
			testKeyTimestamps[j] = (timerOverflowHolder << 16) | TCNT1;		// Saving Timestamps
		}
		stopTimer();
		calculateTestVector();
		
		double userAEuclidean = euclideanDistance(testVector, userAVector);
		double userBEuclidean = euclideanDistance(testVector, userBVector);
		
		if(userAEuclidean < userBEuclidean) {
			// TODO: USER A WINS
		} else {
			// TODO: USER B WINS
		}
	}
}