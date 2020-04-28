#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <bits/stdc++.h>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

using namespace std;

string buffToString(char *buf, int bytes);
void printHex(string str, int length);
string vecToString(vector<char> buffer);

int main(int argc, char *argv[])
{
	//	Create a socket

	int protocol;
	int packetSize;
	int timeoutInterval;
	int slidingWindowSize;
	int sequenceNumberRange;
	int situationalErrors;

	cout << "1. Which protocol: ";
	cout << endl;
	cin >> protocol;

	// cout << "2. What would you like for size of Packet: ";
	// cout << endl;
	// cin >> packetSize;

	// cout << "3. Timeout interval: ";
	// cout << endl;
	// cin >> timeoutInterval;

	// cout << "4. Size of sliding window: ";
	// cout << endl;
	// cin >> slidingWindowSize;

	// cout << "5. Range of sequence numbers: ";
	// cout << endl;
	// cin >> sequenceNumberRange;

	// cout << "6. Situational Errors: ";
	// cout << endl;
	// cin >> situationalErrors;

	// int sock = socket(AF_INET, SOCK_STREAM, 0);
	// if (sock == -1)
	// {
	// 	return 1;
	// }

	// //	Create a hint structure for the server we're connecting with
	int port = 9808;
	string ipAddress = "10.35.195.46";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// //--------------------------------------------------------------->

	// char buf[4096];
	// string userInput;

	// Create a socket and connect to the reciever------------------>

	int recieverSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (recieverSock == -1)
	{
		cout << "Ending";
		cout << endl;
		return 1;
	}
	cout << "socket";
	cout << endl;
	port = 9802;
	string ipAddressReciever = "10.35.195.22"; //Thing2
	// string ipAddressReciever = "10.35.195.49"; //Thing3
	sockaddr_in hitReciever;
	hitReciever.sin_family = AF_INET;
	hitReciever.sin_port = htons(port);
	inet_pton(AF_INET, ipAddressReciever.c_str(), &hitReciever.sin_addr);
	int connectRe = connect(recieverSock, (sockaddr *)&hitReciever, sizeof(hitReciever));
	if (connectRe == -1)
	{
		return 1;
	}

	//--------------------------------------------------------------->

	//sets data out
	char bufArray[4096];
	string input;
	unsigned int clientSize = sizeof(hint);

	// Reciever Nonce_B encrypted by session key from reciever--------->
	int sendRe = sendto(recieverSock, "ABC", 3, 0, (sockaddr *)&hint, sizeof(hint));
	memset(bufArray, 0, 4096);
	int bytesReceived = recvfrom(recieverSock, bufArray, 4096, 0, (sockaddr *)&hint, &clientSize);

	int bytesRec = recv(recieverSock, bufArray, 4096, 0);
	if (bytesRec == -1)
	{
		cout << "There was an error getting response from server\r\n";
		return -1;
	}

	//		Display response
	string recStr = buffToString(bufArray, bytesRec);

	// Send to Reciever
	// int sendRe = send(recieverSock, aftEncStr.c_str(), aftEncStr.size(), 0);
	// if (sendRe == -1)
	// {
	// 	cout << "Could not send to server! Whoops!\r\n";
	// }

	cout << endl;

	// Encrypt and Send to Reciever for Step 5---------------------->

	/* 
		*README*
		When Starting on the Thingys, start KDC then Sender then Reciever.
		when inputing keys the length doesn't matter because they are padded to 
		the nearest multiple of 8. I don't think the Nonce length matters, but
		I haven't tested it extensively. You might need to use the touch * command
		because for some reason it says the build may be incomplete.
		
		Blowfish using Session key = sessionBlow
	 */

	// File transfer should theoretically be inserted here*********************************************

	// ~File transfer~
	string filename;

	int bufferSize = 2048;
	cout << "input file name for testing: ";
	getline(cin, filename);

	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	int sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

	int numBuffers = (int)sizeOfFile / bufferSize; //numBuffers will always be an integer
	int remainingBytesInFile = (int)sizeOfFile % bufferSize;
	cout << "numBuffers: " << numBuffers << "\n";
	cout << "remainingBytesInFile: " << remainingBytesInFile << "\n";

	string numberB = to_string(numBuffers);
	string numberRem = to_string(remainingBytesInFile);

	char start[1];
	start[0] = 'n';
	while (start[0] == 'n')
	{
		recv(recieverSock, start, 1, 0);

	} //wait for 'y'

	sendRe = send(recieverSock, numberB.c_str(), numberB.size(), 0);
	if (sendRe == -1)
	{
		cout << "Could not send to server! Whoops!\r\n";
	}

	sendRe = send(recieverSock, numberRem.c_str(), numberRem.size(), 0);
	if (sendRe == -1)
	{
		cout << "Could not send to server! Whoops!\r\n";
	}

	ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
	vector<char> buffer(bufferSize, 0);
	//ofstream output(outputname.c_str(), std::ios::out | std::ios::binary);//used for writing out to a file

	char finished[1];
	finished[0] = 'y';
	int numThreads = 5;
	string result;
	for (int i = 0; i < numBuffers; i++)
	{
		fin.read(buffer.data(), buffer.size());

		//TODO turn buffer to string
		result = vecToString(buffer);

		//==================================get a char from the server==================================

		while (finished[0] == 'n')
		{
			recv(recieverSock, finished, 1, 0);

		} //wait for 'y'
		//cout << "Finished: " << i;
		finished[0] = 'n'; //change back to

		//==============================================================================================

		sendRe = send(recieverSock, result.c_str(), result.size(), 0);
		if (sendRe == -1)
		{
			cout << "Could not send to server! Whoops!\r\n";
		}
	}
	if (fin.good())
	{
		std::vector<char> lastBuffer(remainingBytesInFile, 0);
		fin.read(lastBuffer.data(), remainingBytesInFile);

		sendRe = send(recieverSock, result.c_str(), result.size(), 0);
		if (sendRe == -1)
		{
			cout << "Could not send to server! Whoops!!\r\n";
		}
	}
	fin.close();
	//output.close();

	// Read chunk from file

	// break it up into 2048 sized chunks

	// encrypt all chunks at the same time on separate threads

	// send each chunk, waiting for previous chunk to be sent

	// File transfer should theoretically be inserted here*********************************************

	//	Close the socket
	close(recieverSock);

	return 0;
}

void printHex(string str, int length)
{
	for (int i = 0; i < str.length(); i++)
	{
		std::cout << std::hex << (int)str.at(i);
		cout << ":";
	}
	cout << endl;
}

string vecToString(vector<char> vec)
{
	string s(vec.begin(), vec.end());
	return s;
}
string buffToString(char *buf, int bytes)
{
	string part(buf, 0, bytes);
	return part;
}
