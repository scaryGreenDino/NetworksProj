
// Server side implementation of UDP client-server model
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "checksum.h"

#define PORT 9086
#define MAXLINE 1024
using namespace std;
// Driver code
struct Packet
{
    long sn;
    long long int crc;
    char data[128];
};

string vecToString(vector<char> vec)
{
    string s(vec.begin(), vec.end());
    return s;
}

void resetAcks(char* ackArray, int winSize) {

    //char ackArray[winSize];
    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
    }

}

int main()

{
    string thing1Ip = "10.35.195.47";
    string thing2Ip = "10.35.195.22";
    int length = thing1Ip.length() + 1;
    char thing1IpChar[length];
    strcpy(thing1IpChar, thing1Ip.c_str());
    // string thing3Ip = "";

    int sockfd;
    char buffer1[MAXLINE];
    string tmp = "Hello from client";
    length = tmp.length() + 1;
    char hello[length];
    strcpy(hello, tmp.c_str());
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    cout << endl;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;

    //Use this line to bind the socket to only this ip address
    // inet_pton(AF_INET, thing1IpChar, &servaddr.sin_addr);

    servaddr.sin_port = htons(PORT);
    socklen_t serverlength = sizeof(servaddr);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             serverlength) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n;

    socklen_t len = sizeof(cliaddr); //len is value/resuslt

    cout << "Waiting for client start...";
    cout << endl;
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);
    buffer1[n] = '\0';
    printf("Client : %s\n", buffer1);
    cout << endl;
    sendto(sockfd, (const char *)hello, strlen(hello),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    printf("Hello message sent.\n");
    // ~File transfer~
    string filename = "test.txt";

    int bufferSize = 2048;
    cout << "input file name for testing: " << filename << endl;
    // getline(cin, filename);

    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

    int numBuffers = (int)sizeOfFile / bufferSize; //numBuffers will always be an integer
    int remainingBytesInFile = (int)sizeOfFile % bufferSize;
    //cout << "NumBuffers: " << numBuffers << "\n";
    //cout << "RemainingBytesInFile: " << remainingBytesInFile << "\n";

    string numberB = to_string(numBuffers);
    string numberRem = to_string(remainingBytesInFile);

    char start[1];
    start[0] = 'n';

    int pcksz = sizeof(long) + sizeof(long long int) + 128;

    string pckszString = to_string(pcksz);
    cout << "Packet Size as String: " << pckszString << endl;

    int sendRe = sendto(sockfd, pckszString.c_str(), pckszString.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                        len);

    //------Begin Selective Repeat-------------------------------------------------->>>

    //User prompt info
    int winSize = 5; //Total number a frames inside the window
    numBuffers = 20; //Number of packets that need to be sent

    int seqNum = winSize * 3; //Range of sequence numbers given to the frames
    int currPacket = 0; //Counter that keeps track of how many packets have been sent
    int send[winSize];
    char recAck[winSize];

    string pkg;
    resetAcks(recAck, winSize);

    for (int i = 1; i <= numBuffers; i++)
    {
        
        pkg = "Packet #" + to_string(i) + " info.";
        //cout << pkg << endl;

        if (i % winSize == 0)
        {
            //cout << frames[i] << "\n";
            cout << "Sending Packet #" << (currPacket + 1) << "..." << endl;

            sendRe = sendto(sockfd, pkg.c_str(), pkg.size(), MSG_CONFIRM, (const struct sockaddr*) &cliaddr, len);

            cout << "Acknowledgement of above frames sent is received by sender\n\n";
        }
        else {
            //cout << frames[i] << " ";
            cout << "Sending Packet #" << (currPacket + 1) << "..." << endl;
            sendRe = sendto(sockfd, pkg.c_str(), pkg.size(), MSG_CONFIRM, (const struct sockaddr*) & cliaddr, len);

        }

        //cout << "Current Ack: " << recAck[i % winSize] << endl;
        currPacket++;

            
    }


    //------End Selective Repeat---------------------------------------------------->>>


    //send package 0

    // while (start[0] == 'n')
    // {
    //     recvfrom(sockfd, start, 1,
    //              MSG_WAITALL, (struct sockaddr *)&cliaddr,
    //              &len);

    // } //wait for 'y'
    // cout << "numBuffers: " << numBuffers << "\n";

    // int sendRe = sendto(sockfd, numberB.c_str(), numberB.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
    //                     len);
    // if (sendRe == -1)
    // {
    //     cout << "Could not send to server! Whoops!\r\n";
    // }

    // sendRe = sendto(sockfd, numberRem.c_str(), numberRem.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
    //                 len);
    // if (sendRe == -1)
    // {
    //     cout << "Could not send to server! Whoops!\r\n";
    // }

    // ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    // vector<char> buffer(bufferSize, 0);
    // //ofstream output(outputname.c_str(), std::ios::out | std::ios::binary);//used for writing out to a file

    // char finished[1];
    // finished[0] = 'y';
    // int numThreads = 5;
    // string result;
    // for (int i = 0; i < numBuffers; i++)
    // {
    //     fin.read(buffer.data(), buffer.size());

    //     //TODO turn buffer to string
    //     result = vecToString(buffer);

    //     //==================================get a char from the server==================================

    //     while (finished[0] == 'n')
    //     {
    //         cout << "before recv" << endl;
    //         recvfrom(sockfd, finished, 1,
    //                  MSG_WAITALL, (struct sockaddr *)&cliaddr,
    //                  &len);

    //     } //wait for 'y'
    //     cout << "after recv" << endl;
    //     //cout << "Finished: " << i;
    //     finished[0] = 'n'; //change back to

    //     //==============================================================================================

    //     sendRe = sendto(sockfd, result.c_str(), result.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
    //                     len);
    //     if (sendRe == -1)
    //     {
    //         cout << "Could not send to server! Whoops!\r\n";
    //     }
    // // }
    // if (fin.good())
    // {
    //     std::vector<char> lastBuffer(remainingBytesInFile, 0);
    //     fin.read(lastBuffer.data(), remainingBytesInFile);

    //     sendRe = sendto(sockfd, result.c_str(), result.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
    //                     len);
    //     if (sendRe == -1)
    //     {
    //         cout << "Could not send to server! Whoops!!\r\n";
    //     }
    // }
    // fin.close();
    //output.close();

    return 0;
}
