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

#include "blowfish.h"
#include "convert.h"
#include "function.h"


void printHex(string str, int length);



using namespace std;


void printHex(string str, int length);

//Random Number generator
long f(long nonce) {
    const long A = 48271;
    const long M = 2147483647;
    const long Q = M/A;
    const long R = M%A;

	static long state = 1;
	long t = A * (state % Q) - R * (state / Q);
	
	if (t > 0)
		state = t;
	else
		state = t + M;
	return (long)(((double) state/M)* nonce);
}

int main(int argc, char *argv[]){
	int DEBUG = 1;
	/*
	if(argc > 0 && argv[1] == "-d"){
		# define DEBUG 1 //space inserted to prevent preprocessor because wasn't working for us
	}
	else {
		# define DEBUG 0
	}
	*/
	/*
	if(argc > 0 && argv[1] == "-d"){
		DEBUG = 1;
	}
	else {
		DEBUG = 0;
	}
	*/
    //	Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = 9808;
    string ipAddress = "10.35.195.46";
    

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the sender to the KDC----------------------------->
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        return 1;
    }

    //--------------------------------------------------------------->

    char buf[4096];
    string userInput;

    //---Request-Key------------------------------------------------>

    userInput = "Requesting Session key for (IDB).";

    int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
    if (sendRes == -1) {
        cout << "Could not request the session key of B from the KDC! Whoops!\r\n";
    }

    memset(buf, 0, 4096);

    cout << "Sent (KDC):" << endl;
    cout << "   Request:" << endl;
    cout << "   KS for (IDB)" << endl;
    cout << "   Nonce_A = "; //Test num = 6920123456;
    getline(cin, userInput);

    string nonce = userInput; //Store nonce for later use

    sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
    if (sendRes == -1) {
        cout << "Could not send the Nonce to the KDC! Whoops!\r\n";
    }

    cout << endl;

    //---Request-Key------------------------------------------------>

    // Recieve package---------------------------------------------->

    string encPackage;

    //		Wait for response
    memset(buf, 0, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived == -1){
        cout << "There was an error getting response from (the KDC)\r\n";
    }else{
        //		Store encoded package
        encPackage = string(buf, bytesReceived);
    }
	
	if(DEBUG){
		//send 1
		printHex(encPackage, encPackage.length());
	}

    // Recieve package---------------------------------------------->

    // Get user input----------------------------------------------->

    string aKey;
    cout << "Recieved from (KDC): " << endl;
    cout << "   Enter (IDa) Key: "; //22222222
    getline(cin, aKey);

    int paddingAKey = 8 - (aKey.length() % 8);
	for (int i = 0; i < paddingAKey; i++) {
		aKey = aKey + "0";
	}

    vector<char> senderKey(aKey.begin(), aKey.end());

    // Get user input----------------------------------------------->

    // Decode and parse Package----------------------------------------------->

    vector<char> enc(encPackage.begin(), encPackage.end());
    int vecSize = enc.size();

    Blowfish blowfish(senderKey);

    // Blowfish object used to decrypt the package
    vector<char> dec = blowfish.Decrypt(enc);
    string decPack(dec.begin(), dec.end());

    string delimiter = "/";
    
    // The returned session key
    string sKey = decPack.substr(0, decPack.find(delimiter));
    decPack.erase(0, decPack.find(delimiter) + delimiter.length());
    cout << "   Session Key: " << sKey << endl;

    vector<char> sessKey(sKey.begin(), sKey.end());

    // The original request from IDa
    string req = decPack.substr(0, decPack.find(delimiter));
    decPack.erase(0, decPack.find(delimiter) + delimiter.length());

    // The returned nonce
    string retNonce = decPack.substr(0, decPack.find(delimiter));
    decPack.erase(0, decPack.find(delimiter) + delimiter.length());
    cout << "   Nonce_A: " << retNonce << endl;
    cout << endl;

    //Encrypted package to send to the Reciever
    string recPackage = decPack;
	if(DEBUG){
		cout << "Session and nonce 1:" << endl;
		printHex(recPackage, recPackage.length());
	}

    // Decode Package----------------------------------------------->


	// Create a socket and connect to the reciever------------------>

	Convert con; // For String->Vector<char> conversions
	int recieverSock= socket(AF_INET, SOCK_STREAM, 0);
	if (recieverSock== -1){
		return 1;
	}
    
	port = 9802;
	//string ipAddressReciever = "10.35.195.22"; //Thing2
    string ipAddressReciever = "10.35.195.49"; //Thing3
	sockaddr_in hitReciever;
	hitReciever.sin_family = AF_INET;
	hitReciever.sin_port = htons(port);
	inet_pton(AF_INET, ipAddressReciever.c_str(), &hitReciever.sin_addr);
	int connectRe = connect(recieverSock, (sockaddr*)&hitReciever, sizeof(hitReciever));
	if (connectRe == -1)
	{
		return 1;
	}

    //	Close the kdc socket
    close(sock);

    //--------------------------------------------------------------->

	//sets data out
	char bufArray[4096];
	string input;
	
    // Send package to Reciever------------------------------------->

    sendRes = send(recieverSock, recPackage.c_str(), recPackage.size() + 1, 0);
    if (sendRes == -1) {
        cout << "Could not send the Encrypted Package to the Reciever! Whoops!\r\n";
    }

    cout << "Sent to (IDb):" << endl;
	cout << "	Sent Session Key and IDa." << endl;
	cout << endl;

    // Send package to Reciever------------------------------------->


	// Reciever Nonce_B encrypted by session key from reciever--------->

	memset(bufArray, 0, 4096);
	int bytesRec = recv(recieverSock, bufArray, 4096, 0);
	if (bytesRec == -1)
	{
		cout << "There was an error getting response from server\r\n";
		return -1;
	}

	//		Display response
	string recStr = con.buffToString(bufArray, bytesRec);
	
	if(DEBUG){
		cout << "nonce 2:" << endl;
		printHex(recStr, recStr.length());
	}
	cout << "Recieved from (IDb): " << endl;
	vector<char> recVec = con.stringToVec(recStr, recStr.length());

	// Reciever Nonce_B encrypted by session key from reciever--------->
	

	// Decrypt nonce_B----------------------------------------------->

	Blowfish sessionBlow(sessKey);
	vector<char> decVec = sessionBlow.Decrypt(recVec);
	string decStr = con.vecToString(decVec);

	//Convert Nonce_B from String->Long
	long beforeFunction = con.stringToLong(decStr);

	//cout << "Decrypted Nonce: " << decStr << " of length " << decStr.length() << endl;

	// Decrypt nonce_B----------------------------------------------->


	// Run Nonce_B through function()------------------------------->

	Function function;
	long afterFunction = function.func(beforeFunction);
	string geek = con.longToString(afterFunction);

	cout << "	Randomized Nonce_B: " << afterFunction << endl;

	// Run Nonce_B through function()------------------------------->

	
	// Encrypt and Send to Reciever for Step 5---------------------->

	// Encrypt
	vector<char> aftFuncVec = con.stringToVec(geek, geek.length());
	vector<char> aftFuncEnc = sessionBlow.Encrypt(aftFuncVec);
	string aftEncStr = con.vecToString(aftFuncEnc);
	
	if(DEBUG){
		cout << "nonce 2 after function:" << endl;
		printHex(aftEncStr,aftEncStr.length());
	}

	// Send to Reciever
	int sendRe = send(recieverSock, aftEncStr.c_str(), aftEncStr.size(), 0);
	if (sendRe == -1){
		cout << "Could not send to server! Whoops!\r\n";
	}

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
		
		int numBuffers = (int) sizeOfFile/bufferSize;//numBuffers will always be an integer
        int remainingBytesInFile = (int) sizeOfFile%bufferSize;
        cout << "numBuffers: " << numBuffers << "\n";
        cout << "remainingBytesInFile: " << remainingBytesInFile << "\n";
		
		string numberB = to_string(numBuffers);
		string numberRem = to_string(remainingBytesInFile);
		
		char start[1]; start[0] = 'n'; 
		while(start[0] == 'n'){
				recv(recieverSock, start, 1, 0);
				
		} //wait for 'y'

			
		
		sendRe = send(recieverSock, numberB.c_str(), numberB.size(), 0);
		if (sendRe == -1){
			cout << "Could not send to server! Whoops!\r\n";
		}
		
		sendRe = send(recieverSock, numberRem.c_str(), numberRem.size(), 0);
		if (sendRe == -1){
			cout << "Could not send to server! Whoops!\r\n";
		}
		
		ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
        vector<char> buffer (bufferSize, 0);
        //ofstream output(outputname.c_str(), std::ios::out | std::ios::binary);//used for writing out to a file
		
		char finished[1];
		finished[0] = 'y';
		int numThreads = 5;
		string result;
		for(int i = 0; i < numBuffers; i ++){
			fin.read(buffer.data(), buffer.size());
			if(DEBUG){
			string toPrint;
				//for (int w = 0; w < buffer.size(); w++) { 
				//	toPrint = toPrint + buffer.data()[w]; 
				//}
				//printHex(toPrint, buffer.size());
			}
			result = con.vecToString(sessionBlow.Encrypt(buffer));
			if(DEBUG){
				printHex(result, bufferSize);
			}
			
			//==================================get a char from the server==================================
			
			while(finished[0] == 'n'){
				recv(recieverSock, finished, 1, 0);
				
			} //wait for 'y'
			//cout << "Finished: " << i;
			finished[0] = 'n'; //change back to 
				
			
			
			//==============================================================================================
			
			sendRe = send(recieverSock, result.c_str(), result.size(), 0);
			if (sendRe == -1){
				cout << "Could not send to server! Whoops!\r\n";
			}
			
			
			
		}
		if(fin.good()){
            std::vector<char> lastBuffer (remainingBytesInFile, 0);
            fin.read(lastBuffer.data(), remainingBytesInFile);
			if(DEBUG){
			string toPrint;
				//for (int w = 0; w < buffer.size(); w++) { 
				//	toPrint = toPrint + buffer.data()[w]; 
				///}
				//printHex(toPrint, buffer.size());
			}
			result = con.vecToString(sessionBlow.Encrypt(lastBuffer));
			if(DEBUG){
				printHex(result, bufferSize);
			}
			sendRe = send(recieverSock, result.c_str(), result.size(), 0);
			if (sendRe == -1){
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

void printHex(string str, int length) {
	for (int i = 0; i < str.length(); i++) {
		std::cout << std::hex << (int)str.at(i);
		cout << ":";
	}
	cout << endl;
}



