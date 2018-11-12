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