#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <bits/stdc++.h> 
#include <cstdlib>
#include <time.h>

#include "blowfish.h"
#include "function.h"
#include "convert.h"


void printHex(string str, int length);



using namespace std;

void printHex(string str, int length);

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
	// Makes a converter for all the String to vector<char> conversions
	Convert con;

	// Create a socket------------------------------------------------------->
	int listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == -1) {
		cerr << "Can't create a socket! Quitting" << endl;
		return -1;
	}
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(9802);
	inet_pton(AF_INET, "10.35.195.49", &hint.sin_addr); //Thing3
	//inet_pton(AF_INET, "10.35.195.22", &hint.sin_addr); //Thing2
	
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// gets the client
	listen(listening, SOMAXCONN);
	sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	char host[NI_MAXHOST];      // Client's remote name
	char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on
	memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0){
		cout << host << " connected on port " << service << endl;
	}else{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	cout << endl;

	// Create a socket------------------------------------------------------->

	// Close listening socket 
	//(I'm not sure why this is here but I'm leaving it alone)
	close(listening);

	// data for the sockets
	char buf[4096];
	memset(buf, 0, 4096);

	// Gets the input from the user-------(Nonce_B)-(Key_B)------------------>

	// Get IDb Key
	string bKeyStr;
	cout << "Enter (IDb) Key: ";
	getline(cin, bKeyStr);

	// Get Nonce_B from user
	string senderRandomNumber;
	cout << "Nonce_B: ";
	getline(cin, senderRandomNumber);

	// Pad the key to a multiple of 8
	int recKeyNumZeros = 8 - (bKeyStr.length() % 8);
	for (int i = 0; i < recKeyNumZeros; i++) {
		bKeyStr = bKeyStr + "0";
	}

	cout << endl;

	//Format IDb Key
	vector<char> bKey(bKeyStr.begin(), bKeyStr.end());
	
	// Gets the input from the user-------(Nonce_B)-(Key_B)------------------>


	// Recieving Package from step 3------------------------------------------------->

	int bytesReceived = recv(clientSocket, buf, 4096, 0);
	if (bytesReceived == -1){
		cerr << "Error in recv(). Quitting" << endl;
		return -1;
	}

	string package = string(buf, 0, bytesReceived);

	// Recieving Package from step 3------------------------------------------------->
	
	// decrypting step 3
	Blowfish blow(bKey);
	string recEncStr(buf, 0, bytesReceived);
	
	if(DEBUG){
		cout << "Session and nonce 1:" << endl;
		printHex(recEncStr, bytesReceived);
	}
	vector<char> recEncVec(recEncStr.begin(), recEncStr.end());
	vector<char> decVec = blow.Decrypt(recEncVec);
	string decStr(decVec.begin(), decVec.end());
	//cout << "	Decrypted String: " << decStr << endl;

    // Separate Session key and IDa
	string delimiter = "/";
    string sKey = decStr.substr(0, decStr.find(delimiter));
    string idA = decStr.erase(0, decStr.find(delimiter) + delimiter.length());

	//Format Session Key and Nonce_B
	vector<char> sessionKey(sKey.begin(), sKey.end());
	long nonce = con.stringToLong(senderRandomNumber);

	cout << "Recieved from (IDa): " << endl;
	cout << "   Session Key: " << sKey << endl;
	cout << "	Nonce_B: " << nonce << endl;
	cout << endl;

	//Create blowfish with the Session Key
	Blowfish sessionBlow(sessionKey);

	// Randomize Nonce_B using function(nonce)------------------------------------------------------->

	Function fun;
	long funNonce = fun.func(nonce);
	string nonceStr = con.longToString(nonce);

	// Randomize Nonce_B using function(nonce)------------------------------------------------------->


	// Encrypting and Sending Nonce_B for Step 4---------------------------------------->

	//Encrypt Nonce_B
	vector<char> conVec = con.stringToVec(nonceStr, nonceStr.length());
	vector<char> encConVec = sessionBlow.Encrypt(conVec);
	string encConStr(encConVec.begin(), encConVec.end());
	
	if(DEBUG){
		cout << "nonce 2:" << endl;
		printHex(encConStr, encConStr.length());
	}

	//Print response
	cout << "Send to (IDa): " << endl;	
	cout << "	Nonce_B" << endl;
	cout << endl;

	//Sending Encrypted Nonce_B
	memset(buf, 0, 4096);
	strcpy(buf, encConStr.c_str());
	send(clientSocket, buf, encConStr.length(), 0);

	// Encrypting and Sending Nonce_B for Step 4---------------------------------------->

	// Recieve from IDa for Step 5---f(Nonce_B)------------------------------------------------------>

	cout << "Received from (IDa): " << endl;
	cout << "   Recieved Nonce." << endl;

	memset(buf, 0, 4096);
	bytesReceived = recv(clientSocket, buf, 4096, 0);
	if (bytesReceived == -1)
	{
		cerr << "	Error in recv(). Quitting" << endl;
		return -1;
	}

	string afterFunc(buf, 0, bytesReceived);
	
	if(DEBUG){
		cout << "nonce 2 after function:" <<endl;
		printHex(afterFunc,afterFunc.length());
	}

	vector<char> afterFunEncVec;
	for (int i = 0; i < bytesReceived; i++) {
		//cout << +(buf[i]) << " ";
		afterFunEncVec.push_back(buf[i]);
	}

	// Recieve from IDa for Step 5---f(Nonce_B)------------------------------------------------------>


	// Decrypt and Match Nonce for Step 5--------------------------------------------------------------->

	vector<char> afterFunDecVec = sessionBlow.Decrypt(afterFunEncVec);
	string afterFunDecStr(afterFunDecVec.begin(), afterFunDecVec.end());
	long longAfterFun = con.stringToLong(afterFunDecStr);
	
	cout << "	Validating Nonce.....";

	if (longAfterFun == funNonce) {
		cout << "Nonce Matched" << endl;
	} else {
		cout << "PRETENDER (Nonce!=Match)" << endl;
	}	

	cout << endl;

	// Decrypt and Match Nonce for Step 5--------------------------------------------------------------->


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

		string outputname;
		cout << "output file name for testing: ";
		getline(cin, outputname);
		char start[1]; start[0] = 'y';
		send(clientSocket, start, 1, 0);
		// ~File transfer~
		int bufferSize = 2056;
		
		memset(buf, 0, bufferSize);
		bytesReceived = recv(clientSocket, buf, bufferSize, 0);
		if (bytesReceived == -1){
			cerr << "	Error in recv(). Quitting" << endl;
			return -1;
		}
		int numBuffers = atoi(buf);
		
		memset(buf, 0, bufferSize);
		bytesReceived = recv(clientSocket, buf, bufferSize, 0);
		
		if (bytesReceived == -1){
			cerr << "	Error in recv(). Quitting" << endl;
			return -1;
		}//"< bufferSize"
		
		int remainingBytes = atoi(buf);//deccryption doubles size of message
		int remainingBytes_origin = remainingBytes;
		if(remainingBytes%8 != 0){
			remainingBytes = (8 - remainingBytes%8) + remainingBytes;
		}
	
		ofstream output(outputname.c_str(), std::ios::out | std::ios::binary);//used for writing out to a file
		
		cout << numBuffers << " is numBuffers\n";
		cout << remainingBytes << " is remaining bytes\n";
		cout << outputname << " is output name\n";
		
		char finished[1];
		finished[0] = 'y';
		
		int totalBytes;
		int lengthOfString;
		
		
	
		
		
		for(int i = 0; i < numBuffers; i++){
			memset(buf, 0, bufferSize);
			bytesReceived = recv(clientSocket, buf, bufferSize, 0);
			totalBytes = bytesReceived;
			if(bytesReceived < bufferSize){
				while (totalBytes != bufferSize){
					//output << bytesReceived << " ";
					bytesReceived = recv(clientSocket, buf + totalBytes, bufferSize, 0);
					totalBytes +=  bytesReceived;
					if (bytesReceived == -1){
						cerr << "	Error in recv(). Quitting!" << endl;
						return -1;
					}
					
				}
				totalBytes = 0;
			}
			if(DEBUG){
			string toPrint;
				//for (int w = 0; w < bufferSize; w++) { 
				//	toPrint = toPrint + buf[w]; 
				//}
				//printHex(toPrint, bufferSize);
			}
			vector<char> decVec = sessionBlow.Decrypt(con.charToVec(buf, bufferSize));
			if(DEBUG){
				printHex(con.vecToString(decVec), bufferSize);
			}
			//output.write("recieve chunk:=================================================\n", sizeof(char)*64);
			output.write(decVec.data(), sizeof(char)*2048);
			
			cout << " recieve chunk:" << i << "\n";
			
			//==================================send a char back to server==================================
			
			send(clientSocket, finished, 1, 0);
			
			
			//==============================================================================================
			
		}
		int totalBytes2 = 0;
		if(remainingBytes != 0){
			memset(buf, 0, bufferSize);
			bytesReceived = recv(clientSocket, buf, remainingBytes, 0);
			totalBytes2 = bytesReceived;
			/*
			if(totalBytes2 < remainingBytes){
				while(totalBytes2 != remainingBytes){
					cout<< "in second while"<< totalBytes2 << "\n";
					bytesReceived = recv(clientSocket, buf + totalBytes2, remainingBytes, 0);
					totalBytes2 += bytesReceived;
					//insert here here
					if (bytesReceived == -1){
						cerr << "	Error in recv(). Quitting!!" << endl;
						return -1;
					}	
				}
				totalBytes2 = 0;
			}*/
			if(DEBUG){
			string toPrint;
				//for (int i = 0; i < bufferSize; i++) { 
					//toPrint = toPrint + buf[i]; 
				///}
				//printHex(toPrint, bufferSize);
			}
			vector<char> decVec = sessionBlow.Decrypt(con.charToVec(buf, remainingBytes));
			if(DEBUG){
				printHex(con.vecToString(con.charToVec(buf, remainingBytes)), bufferSize);
			}
			//output.write("recieve chunk:=================================================\n", sizeof(char)*64);
			output.write(decVec.data(), sizeof(char)*remainingBytes_origin);
			cout << "Recieve last chunk!!\n";
		}

		
		//read in chunk one by one
		
		//decrypt each chunk using its own thread
		
		//put each chunk into file, waiting for previous chunk to be insertd into file
		

	// File transfer should theoretically be inserted here*********************************************


	// Close the socket
	close(clientSocket);
	output.close();
	cout << "end of reciever\n";
	
	return 0;
}

void printHex(string str, int length) {
	for (int i = 0; i < str.length(); i++) {
		std::cout << std::hex << (int)str.at(i);
		cout << ":";
	}
	cout << endl;
}