/*
* Microprocessors Project.c
*
* Created: 2018-11-04 8:26:29 PM
* Author : Army-O
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

unsigned long timerOverflowHolder = 0;

/*
* This array holds the value of the timer when each key is pressed.
* Location 0 holds the time for pressing "."
* Location 9 holds the time for pressing "l"
* The second dimension of the array is for the 5 trials.
* For example, userKeyTimestamps[0][0] corresponds to the timestamp for "." in trial 1.
*/
unsigned long userKeyTimestamps[10][5];

/*
* These two arrays hold the final vector for the two users.
*/
double userAVector[9];
double userBVector[9];

/*
* Configures the 16-bit timer 1 to work in mode 0
* Enables interrupts for overflow consideration
*/
void configureTimer() {
	
	// set the timer to normal mode by clearing all WGM10 to WGM13
	TCCR1A &= ~(1<<WGM10) & ~(1<<WGM11) & ~(1<<WGM12) & ~(1<<WGM13) ;

	// enable interrupt on timer 1 overflow
	TIMSK = (1 << TOIE1);
	
	// enable global interrupt
	sei();
}

/*
* Stops the timer.
* The timer needs to be restarted again by a call to startTimer.
*/
void stopTimer() {
	
	// stop the timer by clearing CS10 and CS12
	TCCR1B &= ~(1<<CS10) & ~(1<<CS12);
	
}

/*
* Resets timer and starts it from zero
* by setting the frequency to freq
*/
void startTimer() {
	
	// clear the timer contents
	TCNT1 = 0;
	timerOverflowHolder = 0;
	
	// set timer clock frequency to freq and start
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS12);
	
}

/*
* On overflow, increment the overflow count
*/
ISR (TIMER1_OVF_vect) {
	timerOverflowHolder++;
}

/* Calculates the euclidean distance
* between the test vector and the stored user vector
*/
double euclideanDistance(double testSubject[], double user[]) {
	double sum = 0.0;
	
	for( int i = 0; i < 10; i++){
		sum += pow((testSubject[i] - user[i]), 2.0);
	}
	
	return sqrt(sum);
}

/*
* Pass 'A' for user A and pass 'B' for user B
* Call this method after all five trials for a particular user are complete.
*/
void calculateUserVector(char user) {
	
	unsigned long differencesVector[9][5];
	
	// calculate user differences vector
	for(int i=0; i < 9; i++){ // iterate over timestamps
		for(int j=0; j<5;j++){ // iterate over trials
			differencesVector[i][j] = userKeyTimestamps[i+1][j] - userKeyTimestamps[i][j]; // calculate difference
		}
	}
	
	// average the differences and store the values
	for(int i = 0; i<9; i++) { // iterate over differences
		
		unsigned long sum = 0;
		
		for(int j=0; j<5; j++){ // iterate over trials
			sum+=differencesVector[i][j];
		}
		
		double averageDifference = sum / 5.0;
		
		// store the average of the trials as a feature
		if(user == 'A'){
			userAVector[i] = averageDifference;
		}
		else{
			userBVector[i] = averageDifference;
		}
	}
}

// tests for Omar's methods
void testOmarMethods() {
	// expect timer works
	configureTimer();
	startTimer();
	
	// expect distance equals 2.236068 (root 5)
	double vectorA[10] = {1,2,3,4,5,6,7,8,9,10};
	double vectorB[10] = {2,4,3,4,5,6,7,8,9,10};
	double distance = euclideanDistance(vectorA, vectorB);
	
	// suppress compilation warning, expect distance 3.236068
	distance++;
	
	// test calculations of user vectors
	unsigned long tempStamps[10][5] =
	{{1,2,3,4,5},
	{3000,2000,7000,2000,3501},
	{5000,5000,8000,6000,9000},
	{10000,30000,90000,1000000,9000000},
	{30000,90000,120000,12000000,30000000},
	{90000,120000,400000000,700000000,90000000},
	{91000,121000,700000000,900000000,12000000},
	{92000,122000,900000000,120000000,14000000},
	{98000,170000,910000000,120000000,16000000},
	{99000,180000,970000000,200000000,19000000}};
	
	for(int i = 0; i<10;i++){
		for(int j=0; j<5;j++) {
			userKeyTimestamps[i][j] = tempStamps[i][j];
		}
	}

	calculateUserVector('A');
	
}
int main(void)
{
	/* Replace with your application code */
	while (1)
	{
	}
}

