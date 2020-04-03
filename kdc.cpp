#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include "blowfish.h"

using namespace std;

void printHex(string str, int length);

int main()
{
	int DEBUG = 1;
    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1){
        cerr << "Can't create a socket! Quitting" << endl;
        return -1;
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(9808);
    inet_pton(AF_INET, "10.35.195.46", &hint.sin_addr);

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Tell Winsock the socket is for listening
    listen(listening, SOMAXCONN);

    // Wait for a connection
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

    // Close listening socket
    close(listening);

    // While loop: accept and echo message back to client
    char buf[4096];

    while (true) {
        memset(buf, 0, 4096);

        // Wait for client to send data

        // Recieve the request------------------------------------->
        
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == -1) {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        if (bytesReceived == 0) {
            cout << "Client disconnected " << endl;
            break;
        }

        cout << "Recieved from (IDa): " << endl;
        string req = string(buf, 0, bytesReceived);
        cout << "   " << req << endl;

        // Recieve the request------------------------------------->


        // Recieve the Nonce--------------------------------------->

        memset(buf, 0, 4096);
        bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }

        string nonce = string(buf, 0, bytesReceived);
        cout << "   Nonce_A Recieved: " << nonce << endl;
        cout << endl;

        // Recieve the Nonce--------------------------------------->
   

        // Request keys from the user---------------------------------->

        string sKey;
        string aKey;
        string bKey;

        cout << "Send to (IDa): " << endl;
        cout << "   Enter (Session) Key: ";
        getline(cin, sKey);

        int paddingSKey = 8 - (sKey.length() % 8);
	    for (int i = 0; i < paddingSKey; i++) {
		    sKey = sKey + "0";
	    }

        cout << "   Enter (IDa) Key: ";
        getline(cin, aKey);

        int paddingAKey = 8 - (aKey.length() % 8);
	    for (int i = 0; i < paddingAKey; i++) {
		    aKey = aKey + "0";
	    }

        cout << "   Enter (IDb) Key: ";
        getline(cin, bKey);

        int paddingBKey = 8 - (bKey.length() % 8);
	    for (int i = 0; i < paddingBKey; i++) {
		    bKey = bKey + "0";
	    }

        // Request keys from the user---------------------------------->

        // Package and encrypt----------------------------------------->

        vector<char> recieverKey(bKey.begin(), bKey.end());
        vector<char> senderKey(aKey.begin(), aKey.end());

        string idA = "10.35.195.47";
        string package1 = sKey + "/" + idA; 

        Blowfish blowfish1(recieverKey);

        vector<char> recPackage(package1.begin(), package1.end());
        vector<char> partOneEnc = blowfish1.Encrypt(recPackage);

        string package2 = sKey + "/" + req + "/" + nonce + "/";

        Blowfish blowfish2(senderKey);

        vector<char> sendPackage(package2.begin(), package2.end());
        sendPackage.insert(sendPackage.end(), partOneEnc.begin(), partOneEnc.end());

        vector<char> enc = blowfish2.Encrypt(sendPackage);
        string encPackage(enc.begin(), enc.end());
		
		if(DEBUG){
			//send 2
			printHex(encPackage,encPackage.length()); // Print encrypted
		}

        // Package and encrypt----------------------------------------->

        // Send package to IDa----------------------------------------->

        cout << endl;
        cout << "   Session Key: " << sKey << endl;
        cout << "   Nonce_A: " << nonce << endl;

        int sendRes = send(clientSocket, encPackage.c_str(), encPackage.size() + 1, 0);
        if (sendRes == -1) {
            cout << "Could not deliver the Stuff to the Sender! Whoops!\r\n";
        }

        cout << endl;

        // Send package to IDa----------------------------------------->

    }


    // Close the socket
    close(clientSocket);

    return 0;
}

void printHex(string str, int length) {
	for (int i = 0; i < str.length(); i++) {
		std::cout << std::hex << (int)str.at(i);
		cout << ":";
	}
	cout << endl;
}
