
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

#define PORT 9000
#define MAXLINE 1024
using namespace std;
// Driver code
const uint32_t Polynomial = 0xEDB88320;
string vecToString(vector<char> vec);

uint32_t crc32_bitwise(const void *data, size_t length, uint32_t previousCrc32);

char *packetMaker(unsigned short sn, const char *data, int dataSize);

int main(int argc, char *argv[])

{
    bool defaultSettings = true;
    // bool defaultSettings = (argc > 0 && argv[0] == "-d");/\

    int protocol;
    int packetSize;
    int timeoutInterval;
    int slidingWindowSize;
    int sequenceNumberRange;
    int situationalErrors;
    string thing1Ip = "10.35.195.47";
    string thing2Ip = "10.35.195.22";

    if (defaultSettings)
    {
        protocol = 1;
        packetSize = 1024;
        timeoutInterval = 10;
        slidingWindowSize = 50;
        sequenceNumberRange = 100;
        situationalErrors = 1;
    }
    else
    {
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
        string thing3Ip = "";
    }

    int length = thing1Ip.length() + 1;
    char thing1IpChar[length];
    strcpy(thing1IpChar, thing1Ip.c_str());

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
    // ~File transfer~
    string filename;
    int bufferSize = 2048;
    cout << "input file name for testing: ";
    // getline(cin, filename);
    filename = "test.txt";

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

    int pcksz = sizeof(long) + sizeof(long long int) + 128;

    string pckszString = to_string(pcksz);
    string test = "Hello";
    const char *pck = packetMaker(255, test.c_str(), 7);
    cout << "numBuffers: " << numBuffers << "\n";

    int sendRe = sendto(sockfd, pck, packetSize + 17, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

    return 0;
}
string vecToString(vector<char> vec)
{
    string s(vec.begin(), vec.end());
    return s;
}
//Takes in data, its size, and outputs a uint32_t bitwise
uint32_t crc32_bitwise(const void *data, size_t length, uint32_t previousCrc32 = 0)
{
    uint32_t crc = ~previousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    unsigned char *current = (unsigned char *)data;
    while (length--)
    {
        crc ^= *current++;
        for (unsigned int j = 0; j < 8; j++)
            if (crc & 1)
                crc = (crc >> 1) ^ Polynomial;
            else
                crc = crc >> 1;
    }
    return ~crc; // same as crc ^ 0xFFFFFFFF
}

char *packetMaker(unsigned short sn, const char *data, int dataSize)
{
    int c;
    uint32_t cs = crc32_bitwise(data, dataSize);
    short csSize = 4;
    short snSize = 2;
    short headerSize = csSize + snSize;
    short totalSize = headerSize + dataSize;
    char *packet = new char[totalSize];
    int totalC = 0;
    char csString[csSize];
    char snString[snSize];

    sprintf(csString, "%X", cs);
    sprintf(snString, "%X", sn);
    for (int c = 0; c < csSize; c++)
    {
        packet[c] = csString[c];
    }
    for (int c = 0; c < snSize; c++)
    {
        packet[c + csSize] = snString[c];
    }
    for (int c = headerSize; c < headerSize + dataSize; c++)
    {
        packet[c] = data[c - headerSize];
        totalC++;
    }
    return packet;
}
