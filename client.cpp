
// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>

#define PORT 9086
#define MAXLINE 1024
using namespace std;
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

void resetAcks(char* ackArray, int winSize) {

    //char ackArray[winSize];
    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
    }

}

// Driver code
int main()
{
    string thing1Ip = "10.35.195.47";
    string thing2Ip = "10.35.195.22";
    int length = thing1Ip.length() + 1;
    char thing1IpChar[length];
    strcpy(thing1IpChar, thing1Ip.c_str());
    int sockfd;
    char buffer1[MAXLINE];
    string tmp = "Establishing Connection";
    length = tmp.length() + 1;
    char ackCon[length];
    strcpy(ackCon, tmp.c_str());
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, thing1IpChar, &servaddr.sin_addr);
    int n;
    unsigned int len;
    socklen_t serversize = sizeof(servaddr);

    if (sendto(sockfd, ackCon, (unsigned int)strlen((char *)ackCon),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               serversize) < 0)
    {
        printf("Connection Ack message failed.\n");
    }

    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer1[n] = '\0';
    printf("Server : %s\n", buffer1);

    //----------Prompt User-------------------------------------------------------->>>

        //~Prompt Stuff~

    //------End Prompt User-------------------------------------------------------->>>


    int bytesReceived;
    int bufferSize = 2056;
    char buf[4096];
    memset(buf, 0, 4096);
    string outputname = "outputtest.txt";

     memset(buf, 0, bufferSize);
     bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
     printf("Contents of Buf #1: %s\n", buf); // This recieve gets numBuffers
     int numBuffers = atoi(buf);

     if (bytesReceived == -1)
     {
         cerr << "	Error in recvfrom(). Quitting" << endl;
         return -1;
     }

     memset(buf, 0, bufferSize);
     bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
     printf("Contents of Buf #2: %s\n", buf); //This recieve gets remainingBytes
     int remainingBytes = atoi(buf); //deccryption doubles size of message

     if (remainingBytes % 8 != 0)
     {
         remainingBytes = (8 - remainingBytes % 8) + remainingBytes;
     }

     ofstream output(outputname.c_str(), std::ios::out | std::ios::binary); //used for writing out to a file

     char start[1];
     start[0] = 'y';
     char finished[1];
     finished[0] = 'y';

     int totalBytes;
     int lengthOfString;

     sendto(sockfd, start, 1,
            MSG_CONFIRM, (const struct sockaddr *)&servaddr,
            serversize);

     cout << "Start File Transfer." << endl;
     for (int i = 0; i < numBuffers; i++)
     {
         memset(buf, 0, bufferSize);
         bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
         totalBytes = bytesReceived;
         if (bytesReceived < bufferSize)
         {
             while (totalBytes != bufferSize)
             {

                 //Start file transfer
                 sendto(sockfd, start, 1, MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);
                 
                 bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
                 totalBytes += bytesReceived;
                 if (bytesReceived == -1)
                 {
                     cerr << "	Error in recvfrom(). Quitting!" << endl;
                     return -1;
                 }
             }

             totalBytes = 0;
         }

         vector<char> buffVec = charToVec(buf, bytesReceived);
         cout << "Recieve chunk: " << i << "\n";
         //==================================send a char back to server==================================

         sendto(sockfd, finished, 1,
             MSG_CONFIRM, (const struct sockaddr*)& servaddr,
             serversize);

         //==============================================================================================
         output.write(buffVec.data(), sizeof(char) * 2048);
         cout << "Writing to output file." << endl;
     }

     int totalBytes2 = 0;
     if (remainingBytes != 0)
     {
         memset(buf, 0, bufferSize);
         bytesReceived = recvfrom(sockfd, buf, remainingBytes, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
         totalBytes2 = bytesReceived;
         
         cout << "Writing to last chunk to output file" << endl;
         vector<char> buffVec = charToVec(buf, bytesReceived);
         output.write(buffVec.data(), sizeof(char) * 16);

     }

    //read in chunk one by one
    //put each chunk into file, waiting for previous chunk to be insertd into file

    //------Begin Selective Repeat-------------------------------------------------->>>

    //Recieved from server
     int winSize = 5; //Total number a frames inside the window
     numBuffers = 20; //Number of packets that need to be sent

     int seqNum = winSize * 3; //Range of sequence numbers given to the frames
     int currPacket = 0; //Counter that keeps track of how many packets have been sent
     int rec[winSize];
     char ackSent[winSize];

     string pkg;
     resetAcks(ackSent, winSize);

     for (int i = 1; i <= numBuffers; i++)
     {

         memset(buf, 0, 4096);
         if (i % winSize == 0)
         {

             n = recvfrom(sockfd, (char*)buf, bufferSize, MSG_WAITALL, (struct sockaddr*) & servaddr, &len);
             pkg = buffToString(buf, bufferSize);

             //cout << "Recieved Packet #" << (currPacket + 1) << ": " << pkg << "...\n" << endl;
             //cout << "Acknowledgement of above frames sent is received by sender\n\n";
         } else {

             //cout << "Recieving Packet #" << (currPacket + 1) << "..." << endl;
             n = recvfrom(sockfd, (char*)buf, bufferSize, MSG_WAITALL, (struct sockaddr*) & servaddr, &len);
             pkg = buffToString(buf, bufferSize);
             //cout << "Recieved Packet #" << (currPacket + 1) << ": " << pkg << "..." << endl;

         }

         //cout << "Current Ack: " << recAck[i % winSize] << endl;
         currPacket++;

     }


     //------End Selective Repeat---------------------------------------------------->>>
    
    output.close();
    close(sockfd);

    return 0;
}
