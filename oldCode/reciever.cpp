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

using namespace std;

string buffToString(char *buf, int bytes);
void printHex(string str, int length);
string vecToString(vector<char> buffer);
vector<char> charToVec(char *str, int length);

int main(int argc, char *argv[])
{
	// Create a socket------------------------------------------------------->
	int listening = socket(AF_INET, SOCK_DGRAM, 0);
	if (listening == -1)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return -1;
	}
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(9802);
	// inet_pton(AF_INET, "10.35.195.49", &hint.sin_addr); //Thing3
	inet_pton(AF_INET, "10.35.195.22", &hint.sin_addr); //Thing2

	bind(listening, (sockaddr *)&hint, sizeof(hint));
	char buf[4096];
	memset(buf, 0, 4096);
	int bytesReceived = recvfrom(clientSocket, buf, 4096, 0, (sockaddr *)&client, &clientSize);

	// gets the client
	listen(listening, SOMAXCONN);
	sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	int clientSocket = accept(listening, (sockaddr *)&client, &clientSize);
	char host[NI_MAXHOST];		 // Client's remote name
	char service[NI_MAXSERV];	 // Service (i.e. port) the client is connect on
	memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);
	if (getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else
	{
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

	// Recieving Package from step 3------------------------------------------------->

	bytesReceived = recvfrom(clientSocket, buf, 4096, 0, (sockaddr *)&client, &clientSize);
	if (bytesReceived == -1)
	{
		cerr << "Error in recv(). Quitting" << endl;
		return -1;
	}

	string package = string(buf, 0, bytesReceived);
	cout << package;
	cout << endl;

	// Recieving Package from step 3------------------------------------------------->

	// decrypting step 3
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
	char start[1];
	start[0] = 'y';
	//TODO uncomment
	//sendto(clientSocket, start, 1, 0, (sockaddr *)&hint, sizeof(hint));
	// ~File transfer~
	int bufferSize = 2056;

	memset(buf, 0, bufferSize);
	bytesReceived = recv(clientSocket, buf, bufferSize, 0);
	if (bytesReceived == -1)
	{
		cerr << "	Error in recv(). Quitting" << endl;
		return -1;
	}
	int numBuffers = atoi(buf);

	memset(buf, 0, bufferSize);
	bytesReceived = recv(clientSocket, buf, bufferSize, 0);

	if (bytesReceived == -1)
	{
		cerr << "	Error in recv(). Quitting" << endl;
		return -1;
	} //"< bufferSize"

	int remainingBytes = atoi(buf); //deccryption doubles size of message
	int remainingBytes_origin = remainingBytes;
	if (remainingBytes % 8 != 0)
	{
		remainingBytes = (8 - remainingBytes % 8) + remainingBytes;
	}

	ofstream output(outputname.c_str(), std::ios::out | std::ios::binary); //used for writing out to a file

	cout << numBuffers << " is numBuffers\n";
	cout << remainingBytes << " is remaining bytes\n";
	cout << outputname << " is output name\n";

	char finished[1];
	finished[0] = 'y';

	int totalBytes;
	int lengthOfString;

	for (int i = 0; i < numBuffers; i++)
	{
		memset(buf, 0, bufferSize);
		bytesReceived = recv(clientSocket, buf, bufferSize, 0);
		totalBytes = bytesReceived;
		if (bytesReceived < bufferSize)
		{
			while (totalBytes != bufferSize)
			{
				//output << bytesReceived << " ";
				bytesReceived = recv(clientSocket, buf + totalBytes, bufferSize, 0);
				totalBytes += bytesReceived;
				if (bytesReceived == -1)
				{
					cerr << "	Error in recv(). Quitting!" << endl;
					return -1;
				}
			}
			totalBytes = 0;
		}
		vector<char> buffVec = charToVec(buf, bytesReceived);
		cout << " recieve chunk:" << i << "\n";
		//==================================send a char back to server==================================
		//TODO UNCOMMENT
		// sendto(clientSocket, finished, 1, 0, clientSocketAdress, socketLength);

		//==============================================================================================
	}
	int totalBytes2 = 0;
	if (remainingBytes != 0)
	{
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
		vector<char> decVec = charToVec(buf, remainingBytes);
		//output.write("recieve chunk:=================================================\n", sizeof(char)*64);
		output.write(decVec.data(), sizeof(char) * remainingBytes_origin);
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
vector<char> charToVec(char *str, int length)
{
	vector<char> result;
	for (int i = 0; i < length; i++)
	{
		result.push_back(str[i]);
	}
	return result;
}
