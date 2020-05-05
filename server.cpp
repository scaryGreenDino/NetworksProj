
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

const uint32_t Polynomial = 0xEDB88320;

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

string vecToString(vector<char> vec)
{
    string s(vec.begin(), vec.end());
    return s;
}

void resetAcks(char *ackArray, int winSize)
{

    //char ackArray[winSize];
    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
    }
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
    }
    return packet;
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
    struct sockaddr_in servaddr, cliaddr;

    int protocol;
    int packetSize;
    int timeoutInterval;
    int slidingWindowSize;
    int sequenceNumberRange;
    int situationalErrors;
    string inputFileName;
    bool defaultSettings = false;
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
        while (protocol != 1 && protocol != 2)
        {
            cout << "1. Which protocol:" << endl;
            cout << "1:Go-Back-N" << endl;
            cout << "2: Selective Repeat" << endl;
            cin >> protocol;
        }
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
        while (situationalErrors != 1 && situationalErrors != 2)
        {
            cout << "6. Situational Errors:" << endl;
            cout << "1: On" << endl;
            cout << "2: Off" << endl;
            cout << endl;
            cin >> situationalErrors;
        }
        cout << "7. Input File Name: ";
        cin >> inputFileName;
    }

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

    int n, sendRe;
    socklen_t len = sizeof(cliaddr); //len is value/resuslt

    cout << "Waiting for client start..." << endl;
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);

    buffer1[n] = '\0';
    printf("Client : %s\n\n", buffer1);
    //TODO TAKE USER INPUT FOR BUFFERSIZE
    int header = 6;
    string bufferSizeString = "2056";
    int bufferSize = stoi(bufferSizeString, NULL, 10);
    int dataSize = bufferSize - header;

    length = bufferSizeString.length() + 1;
    char conAck[length];
    strcpy(conAck, bufferSizeString.c_str());
    char protocolBuf[sizeof(protocol)];
    sprintf(protocolBuf, "%d", protocol);
    char situationalErrorsBuf[sizeof(situationalErrors)];
    sprintf(situationalErrorsBuf, "%d", situationalErrors);
    char slidingWindowSizeBuf[sizeof(slidingWindowSize)];
    sprintf(slidingWindowSizeBuf, "%d", slidingWindowSize);
    char sequenceNumberRangeBuf[sizeof(sequenceNumberRange)];
    sprintf(sequenceNumberRangeBuf, "%d", sequenceNumberRange);
    //Send Buffer Size
    sendto(sockfd, (const char *)conAck, strlen(conAck),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    //Send Protocol
    sendto(sockfd, (const char *)protocolBuf, sizeof(protocolBuf),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    //Send Situational Errors
    sendto(sockfd, (const char *)situationalErrorsBuf, sizeof(situationalErrorsBuf),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    //Send Sliding Window Size
    sendto(sockfd, (const char *)slidingWindowSizeBuf, sizeof(slidingWindowSizeBuf),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    //Send Range of sequence numbers
    sendto(sockfd, (const char *)sequenceNumberRangeBuf, sizeof(sequenceNumberRangeBuf),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    printf("Connection Ack message sent.\n");

    //----------Prompt User---------------------------------------------------->>>

    //~Prompt Stuff~
    //(Protocol,Size of Packet,Timeout Interval,Size of Window,Range of Sequence Numbers,Errors)

    //cout << "Input file name for transfer: ";
    //getline(cin, filename);

    //---------End Prompt User------------------------------------------------->>>

    //--------~File transfer~-------------------------------------------------->>>

    //char* packetMaker(long long int cs, unsigned short sn, const char* data, int dataSize)
    long long int checkSum = 0123456; //Check sum value for packet contents
    unsigned short seqNum = 7;        //Sequence number for the packet

    string filename = "test.txt";

    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

    int numBuffers = (int)sizeOfFile / dataSize; //numBuffers will always be an integer
    //int numBuffers = (int)sizeOfFile / bufferSize; //numBuffers will always be an integer (OG)
    int remainingBytesInFile = (int)sizeOfFile % bufferSize;
    //cout << "NumBuffers: " << numBuffers << "\n";
    //cout << "RemainingBytesInFile: " << remainingBytesInFile << "\n";

    string numberB = to_string(numBuffers);
    string numberRem = to_string(remainingBytesInFile);

    rc = stat(filename.c_str(), &stat_buf);
    sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

    //numBuffers = (int)sizeOfFile / dataSize; //numBuffers will always be an integer
    //numBuffers = (int)sizeOfFile / bufferSize; //numBuffers will always be an integer (OG)
    //remainingBytesInFile = (int)sizeOfFile % bufferSize;

    //numberB = to_string(numBuffers);
    //numberRem = to_string(remainingBytesInFile);

    //---Send necessary info to client before transfer----------------->>>

    sendRe = sendto(sockfd, numberB.c_str(), numberB.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    if (sendRe == -1)
    {
        cout << "Could not send to server! Whoops!\r\n";
    }

    sendRe = sendto(sockfd, numberRem.c_str(), numberRem.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    if (sendRe == -1)
    {
        cout << "Could not send to server! Whoops!\r\n";
    }

    //---End Send necessary info to client before transfer-------------->>>
    //END OF MAIN

    char start[1];
    start[0] = 'n';
    while (start[0] == 'n')
    {
        recvfrom(sockfd, start, 1,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);

    } //wait for 'y'
    //cout << "Start File Transfer." << "\n";

    ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    vector<char> buffer(dataSize, 0);
    //vector<char> buffer(bufferSize, 0);

    char finished[1];
    finished[0] = 'y';
    //int numThreads = 5;
    string result;
    for (int i = 0; i < numBuffers; i++)
    {
        fin.read(buffer.data(), buffer.size());
        result = vecToString(buffer);

        //========== Get finish Ack from Client =============================

        while (finished[0] == 'n')
        {
            recvfrom(sockfd, finished, 1, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        } //wait for 'y'

        finished[0] = 'n';

        //===================================================================

        //cout << "==========================================>>>\n";
        //cout << "Sending: " << result << "\n\n";
        //cout << "==========================================>>>\n\n";

        //char* packetMaker(long long int cs, unsigned short sn, const char* data, int dataSize)
        seqNum += 1;
        char *pkg = packetMaker(seqNum, result.c_str(), result.size());

        /*for (int c = 0; c < result.size() + 6; c++)
        {
            printf("%c", pkg[c]);
        }*/

        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

        //sendRe = sendto(sockfd, result.c_str(), result.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        if (sendRe == -1)
        {
            cout << "Could not send to server! Whoops!\r\n";
        }
    }

    string lastBuffString;
    if (fin.good())
    {
        std::vector<char> lastBuffer(remainingBytesInFile, 0);
        fin.read(lastBuffer.data(), remainingBytesInFile);
        lastBuffString = vecToString(lastBuffer);
        //cout << "Last Buffer: " << lastBuffString << endl;

        char *pkg = packetMaker(seqNum, lastBuffString.c_str(), lastBuffString.size());
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                        len);

        //printf("Last buffer: %s\n", pkg);

        //sendRe = sendto(sockfd, result.c_str(), result.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
        //               len);
        if (sendRe == -1)
        {
            cout << "Could not send to server! Whoops!!\r\n";
        }
    }

    fin.close();

    //--~End File transfer~----------------------------------------------------->>>

    //------Begin Selective Repeat-------------------------------------------------->>>

    //User prompt info
    /* int winSize = 5; //Total number a frames inside the window
    numBuffers = 20; //Number of packets that need to be sent

    int seqRange = winSize * 3; //Range of sequence numbers given to the frames
    int currPacket = 0;         //Counter that keeps track of how many packets have been sent
    int send[winSize];
    char recAck[winSize];

    string tempPkg;
    resetAcks(recAck, winSize);

    for (int i = 1; i <= numBuffers; i++)
    {

        tempPkg = "Packet #" + to_string(i) + " info.";
        //cout << pkg << endl;

        if (i % winSize == 0)
        {
            //cout << frames[i] << "\n";
            //cout << "Sending Packet #" << (currPacket + 1) << "..." << endl;

            sendRe = sendto(sockfd, tempPkg.c_str(), tempPkg.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

            //cout << "Acknowledgement of above frames sent is received by sender\n\n";
        }
        else
        {
            //cout << frames[i] << " ";
            //cout << "Sending Packet #" << (currPacket + 1) << "..." << endl;
            sendRe = sendto(sockfd, tempPkg.c_str(), tempPkg.size(), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        }

        //cout << "Current Ack: " << recAck[i % winSize] << endl;
        currPacket++;
    } */

    //cout << "End Selective Repeat" << endl;

    //------End Selective Repeat---------------------------------------------------->>>

    return 0;
}
