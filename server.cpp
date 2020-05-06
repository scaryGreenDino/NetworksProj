
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
int sockfd, sendRe;
struct sockaddr_in servaddr, cliaddr;
socklen_t len;

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

void initArr(char *ackArray, int *arr, int winSize)
{

    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
        arr[i] = i;
    }

    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
    }
}

void printCharArr(char *arr, int winSize)
{
    cout << "Acks = [";
    for (int i = 0; i < winSize; i++)
    {
        if (i < (winSize - 1))
        {
            cout << arr[i] << ", ";
        }
        else
        {
            cout << arr[i] << "]\n";
        }
    }
}

void printIntArr(int *arr, int winSize)
{
    cout << "Current window = [";
    for (int i = 0; i < winSize; i++)
    {
        if (i < (winSize - 1))
        {
            cout << arr[i] << ", ";
        }
        else
        {
            cout << arr[i] << "]\n";
        }
    }
}

void isReceived(char *window, int *arr, int seqNum, int winSize)
{
    for (int j = 0; j < winSize; j++)
    {
        if (arr[j] == seqNum)
        {
            window[j] = 'y';
        }
    }
}

int slide(char *window, int *arr, int base, int winSize)
{
    char tmpC[winSize];
    int tmpI[winSize];
    while (window[0] == 'y')
    {
        for (int j = 0; j < winSize; j++)
        {
            if (j < (winSize - 1))
            {
                tmpC[j] = window[j + 1];
                tmpI[j] = arr[j + 1];
            }
            else if ((winSize + base) >= (winSize * 3))
            { //Edge Case
                tmpC[j] = 'n';
                tmpI[j] = (base + winSize) - (winSize * 3);
                base++;
            }
            else
            {
                tmpC[j] = 'n';
                tmpI[j] = base + winSize;
                base++;
            }

            if (base >= (winSize * 3))
            { //Resets the first entry back to zero
                base = 0;
            }
        }

        strcpy(window, tmpC);
        memcpy(arr, tmpI, winSize * sizeof(int));
    }

    return base;
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

    //printf("Header: %s%s\n", csString, snString);

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

void selectiveRepeatSend(vector<char> buffer, ifstream &fin, int remainingBytesInFile, int numBuffers, int bufferSize)
{

    int seqNum = 0;
    int winSize = 5;            //Total number a frames inside the window
    int seqRange = winSize * 3; //Range of sequence numbers given to the frames
    int base = 0;               //Counter that keeps track of how many packets have been sent
    int send[winSize];
    char recAck[winSize];
    char *pktStore[numBuffers]; //Stores each packet for reference if any get lost

    initArr(recAck, send, winSize);
    printIntArr(send, winSize);
    //printCharArr(recAck, winSize);

    char buf[5];
    int recSN;
    string::size_type sz;
    string result;
    for (int i = 0; i < numBuffers; i++)
    {
        memset(buf, 0, sizeof(buf));
        fin.read(buffer.data(), buffer.size());
        result = vecToString(buffer);

        cout << "Packet " << seqNum << " sent.\n";
        char *pkg = packetMaker(seqNum, result.c_str(), result.size());
        pktStore[seqNum - 1] = pkg;
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        if (sendRe == -1)
        {
            cout << "Could not send to server! Whoops!\r\n";
        }

        if (seqNum >= seqRange)
        {
            seqNum = 0;
        }
        else
        {
            seqNum++;
        }

        recvfrom(sockfd, (char *)buf, sizeof(buf), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        recSN = stoi(buf, &sz);
        cout << "Ack " << recSN << " received.\n";
        isReceived(recAck, send, recSN, winSize);
        base = slide(recAck, send, base, winSize);
        printIntArr(send, winSize);
        //printCharArr(recAck, winSize);
    }

    string lastBuffString;
    if (fin.good())
    {
        std::vector<char> lastBuffer(remainingBytesInFile, 0);
        fin.read(lastBuffer.data(), remainingBytesInFile);
        lastBuffString = vecToString(lastBuffer);

        char *pkg = packetMaker(seqNum, lastBuffString.c_str(), lastBuffString.size());
        pktStore[seqNum - 1] = pkg;
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                        len);
        seqNum++;

        recvfrom(sockfd, (char *)buf, sizeof(buf), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        recSN = stoi(buf, &sz);

        cout << "Ack " << recSN << " received.\n";
        isReceived(recAck, send, recSN, winSize);
        base = slide(recAck, send, base, winSize);
        printIntArr(send, winSize);
        //printCharArr(recAck, winSize);

        if (sendRe == -1)
        {
            cout << "Could not send to server! Whoops!!\r\n";
        }
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
    struct sockaddr_in servaddr, cliaddr;

    int protocol;
    int packetSize;
    int timeoutInterval;
    int slidingWindowSize;
    long int sequenceNumberRange;
    int situationalErrors;
    string inputFileName;
    int faultyPacketNumber;
    bool defaultSettings = true;
    int regCounter = 0;
    if (defaultSettings)
    {
        protocol = 1;              // this is working
        packetSize = 20;           //this works
        timeoutInterval = 500000;  // this  still needs to be done
        slidingWindowSize = 50;    // this works
        sequenceNumberRange = 100; // this is possibly working
        situationalErrors = 1;     // this still needs to be done
        faultyPacketNumber = 3;
    }
    else
    {
        while (protocol != 1 && protocol != 2)
        {
            cout << "1. Which protocol:" << endl;
            cout << "  1: Go-Back-N" << endl;
            cout << "  2: Selective Repeat" << endl;
            cin >> protocol;
        }
        cout << "2. What would you like for size of Packet: ";
        cout << endl;
        cin >> packetSize;

        cout << "3. Timeout interval: ";
        cout << endl;
        cin >> timeoutInterval;
        timeoutInterval = timeoutInterval * 1000000;

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
    int bufferSize = packetSize;
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
    char packetSizeBuf[sizeof(packetSize)];
    sprintf(packetSizeBuf, "%d", packetSize);
    //Send Buffer Size
    sendto(sockfd, (const char *)packetSizeBuf, sizeof(packetSize),
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

    unsigned short seqNum = 7; //Sequence number for the packet

    string filename = "test.txt";

    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int sizeOfFile = rc == 0 ? stat_buf.st_size : -1;
    long int numBuffers = (int)sizeOfFile / dataSize;
    long int remainingBytesInFile = (int)sizeOfFile % bufferSize;
    string numberB = to_string(numBuffers);
    string numberRem = to_string(remainingBytesInFile);

    rc = stat(filename.c_str(), &stat_buf);
    sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

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
    if (protocol == 2)
    {
        selectiveRepeatSend(buffer, fin, remainingBytesInFile, numBuffers, bufferSize);
    }
    if (protocol == 1)
    {
        int WinSize = slidingWindowSize;
        long int SWinStart = 0;
        long int SWinEnd = 0 + WinSize;
        long int currentWindow = 0;
        long int totalPackets = numBuffers; // this line should be moved
        char start[1];
        start[0] = 'n';
        int cWinA[1];
        cWinA[0] = 8;
        long int counter = 0;

        //vector<char> buffer(bufferSize, 0);

        char finished[1];
        finished[0] = 'y';
        //int numThreads = 5;
        string result;

        if (currentWindow >= SWinStart && currentWindow <= SWinEnd)
        {
            for (long int i = 0; i < numBuffers; i++)
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
                cout << "Packet " << i << " with sequence number " << i % sequenceNumberRange << " sent" << endl;

                cout << "Current window = [";

                for (int x = 0; x <= WinSize; x++)
                {
                    if ((SWinStart + x) < totalPackets)
                    {
                        cout << " " << ((SWinStart + x) % sequenceNumberRange);
                    }
                }
                SWinStart++;

                cout << "]" << endl;

                if (regCounter % faultyPacketNumber == 0)
                {
                    //  usleep(timeoutInterval);
                    cout << "packet " << i << " with sequence number " << i % sequenceNumberRange << " timed out" << endl;
                    cout << "Packet " << i << " with sequence number " << i % sequenceNumberRange << " has been resent" << endl;
                    recvfrom(sockfd, cWinA, 1,
                             MSG_WAITALL, (struct sockaddr *)&cliaddr,
                             &len);
                    cout << "ACK recieved for packet " << (counter % sequenceNumberRange) << endl;
                    counter++;
                }
                else
                {
                    recvfrom(sockfd, cWinA, 1,
                             MSG_WAITALL, (struct sockaddr *)&cliaddr,
                             &len);
                    cout << "ACK recieved for packet " << (counter % sequenceNumberRange) << endl;
                    counter++;
                }
                regCounter++;
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

                recvfrom(sockfd, cWinA, 1,
                         MSG_WAITALL, (struct sockaddr *)&cliaddr,
                         &len);

                cout << "ACK recieved for packet " << cWinA << endl;
                counter = counter + 1;
            }
            SWinStart++;
            counter++;
        }
        fin.close();
    }
    return 0;
}
