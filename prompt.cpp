#include<iostream>
using namespace std;


main() {

	int protocol;
	int packetSize;
	int timeoutInterval;
	int slidingWindowSize;
	int sequenceNumberRange;
	int situationalErrors;
	
	cout << "1. Which protocol: ";
	cout << endl;
	cin >> protocol;

	cout << "2. What would you like for size of Packet: ";
	cout << endl;
	cin >> packetSize;

	cout << "3. Timeout interval: "; 
	cout << endl;
	cin >> timeoutInterval;

	cout << "4. Size of sliding window: ";
	cout << endl;
	cin >> slidingWindowSize;

	cout << "5. Range of sequence numbers: ";
	cout << endl;
	cin >> sequenceNumberRange;

	cout << "6. Situational Errors: ";
	cout << endl;
	cin >> situationalErrors;
 

	return 0;
	}
