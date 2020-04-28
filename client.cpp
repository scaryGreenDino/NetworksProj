
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

#define PORT 9000
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
    string tmp = "awk";
    length = tmp.length() + 1;
    char hello[length];
    strcpy(hello, tmp.c_str());
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
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

    if (sendto(sockfd, hello, (unsigned int)strlen((char *)hello),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               serversize) < 0)
    {
        printf("Hello message failed.\n");
    }
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer1[n] = '\0';
    printf("Server : %s\n", buffer1);

    // File transfer should theoretically be inserted here*********************************************

    int bytesReceived;
    char buf[4096];
    memset(buf, 0, 4096);
    string outputname;
    cout << "output file name for testing:"
         << "\n";
    // getline(cin, outputname);
    outputname = "outputtest.txt";
    char start[1];
    start[0] = 'y';
    //TODO uncomment
    sendto(sockfd, start, 1,
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           serversize);
    // ~File transfer~
    int bufferSize = 2056;

    memset(buf, 0, bufferSize);
    bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    if (bytesReceived == -1)
    {
        cerr << "	Error in recvfrom(). Quitting" << endl;
        return -1;
    }
    // int numBuffers = atoi(buf);

    // memset(buf, 0, bufferSize);
    // bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);

    // if (bytesReceived == -1)
    // {
    //     cerr << "	Error in recvfrom(). Quitting" << endl;
    //     return -1;
    // } //"< bufferSize"

    // int remainingBytes = atoi(buf); //deccryption doubles size of message
    // int remainingBytes_origin = remainingBytes;
    // if (remainingBytes % 8 != 0)
    // {
    //     remainingBytes = (8 - remainingBytes % 8) + remainingBytes;
    // }

    // ofstream output(outputname.c_str(), std::ios::out | std::ios::binary); //used for writing out to a file

    // cout << numBuffers << " is numBuffers\n";
    // cout << remainingBytes << " is remaining bytes\n";
    // cout << outputname << " is output name\n";

    // char finished[1];
    // finished[0] = 'y';

    // int totalBytes;
    // int lengthOfString;

    // memset(buf, 0, bufferSize);
    // bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);

    // bytesReceived = recvfrom(sockfd, rcv, sizeof(Package), MSG_WAITALL, (struct sockaddr *)&servaddr, &len);

    // //TODO uncomment
    // sendto(sockfd, start, 1,
    //        MSG_CONFIRM, (const struct sockaddr *)&servaddr,
    //        serversize);

    // for (int i = 0; i < numBuffers; i++)
    // {
    //     memset(buf, 0, bufferSize);
    //     bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //     totalBytes = bytesReceived;
    //     if (bytesReceived < bufferSize)
    //     {
    //         while (totalBytes <= bufferSize)
    //         {
    //             cout << "stuck" << endl;

    //             char start[1];
    //             start[0] = 'y';
    //             //TODO uncomment
    //             sendto(sockfd, start, 1,
    //                    MSG_CONFIRM, (const struct sockaddr *)&servaddr,
    //                    serversize);
    //             bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //             // cout << " Buffer :" << buf << endl;
    //             cout << "Total byte: " << totalBytes << endl;
    //             cout << "Buffer Size: " << bufferSize << endl;

    //             totalBytes += bytesReceived;
    //             if (bytesReceived == -1)
    //             {
    //                 cerr << "	Error in recvfrom(). Quitting!" << endl;
    //                 return -1;
    //             }
    //         }
    //         totalBytes = 0;
    //     }
    //     vector<char> buffVec = charToVec(buf, bytesReceived);
    //     cout << " recieve chunk:" << i << "\n";
    //     //==================================send a char back to server==================================

    //     //==============================================================================================
    //     output.write(buffVec.data(), sizeof(char) * 2048);
    //     cout << "writing to file" << endl;
    // }
    // int totalBytes2 = 0;
    // if (remainingBytes != 0)
    // {
    //     memset(buf, 0, bufferSize);
    //     bytesReceived = recvfrom(sockfd, buf, remainingBytes, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //     totalBytes2 = bytesReceived;
    //     /*
    // 		if(totalBytes2 < remainingBytes){
    // 			while(totalBytes2 != remainingBytes){
    // 				cout<< "in second while"<< totalBytes2 << "\n";
    // 				bytesReceived = recvfrom(sockfd, buf + totalBytes2, remainingBytes, 0);
    // 				totalBytes2 += bytesReceived;
    // 				//insert here here
    // 				if (bytesReceived == -1){
    // 					cerr << "	Error in recvfrom(). Quitting!!" << endl;
    // 					return -1;
    // 				}
    // 			}
    // 			totalBytes2 = 0;
    // 		}*/
    //     //output.write("recieve chunk:=================================================\n", sizeof(char)*64);
    //     cout << "writing to file" << endl;
    //     vector<char> buffVec = charToVec(buf, bytesReceived);
    //     cout << buffToString(buf, sizeof(buf)) << endl;
    //     output.write(buffVec.data(), sizeof(char) * 16);

    //     cout << "Recieve last chunk!!\n";
    // }

    //read in chunk one by one

    //decrypt each chunk using its own thread

    //put each chunk into file, waiting for previous chunk to be insertd into file

    // File transfer should theoretically be inserted here*********************************************

    // output.close();
    close(sockfd);

    return 0;
}
