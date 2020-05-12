
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
const uint32_t Polynomial = 0xEDB88320;
uint32_t crc32_bitwise(const void *data, size_t length, uint32_t previousCrc32);
int sockfd, sendRe;
struct sockaddr_in servaddr;
unsigned int len;
socklen_t serversize;

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

void resetAcks(char *ackArray, int winSize)
{

    //char ackArray[winSize];
    for (int i = 0; i < winSize; i++)
    {
        ackArray[i] = 'n';
    }
}

char *substr(char *arr, int begin, int len)
{
    char *res = new char[len];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
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
struct Packet
{
    short sn;
    uint32_t crc;
    char *data;
};

struct Packet charsToPacket(char *chars, int dataSize)
{
    int c;
    short csSize = 4;
    short snSize = 2;
    struct Packet packet;
    char *cs = substr(chars, 0, 4);
    //cout << cs << endl;
    char *sn = substr(chars, 4, 1);
    //cout << sn << endl;
    packet.crc = stoi(cs, NULL, 16);
    packet.sn = stoi(sn, NULL, 16);

    packet.data = substr(chars, 6, dataSize - 1);

    // cout << "PACKET " << endl;
    // cout << "   SEQUENCE NUM: " << packet.sn << endl;
    // cout << "   CRC:          " << packet.crc << endl;
    //cout << "   DATA:         " << packet.data << endl;

    return packet;
}
void printChunk(struct Packet pkg)
{
    cout << "=== Start Chunk =====================================================================>>>\n\n";
    cout << "   Packet Sequence Number: " << pkg.sn << endl;
    cout << "   Packet CheckSum: " << pkg.crc << endl;
    cout << "\nPacket Data:\n\n"
         << pkg.data << endl;
    cout << endl;
    cout << "=== End Chunk =====================================================================>>>\n\n";
}

void selectiveRepeatReceive(char *buf, ofstream &output, int remainingBytes, int numBuffers, int bufferSize)
{

    int winSize = 5; //Total number a frames inside the window

    int seqRange = winSize * 3; //Range of sequence numbers given to the frames
    int seqNum = 0;
    string numString;
    int snSize = numString.length();
    char snArr[snSize + 1];

    int base = 0; //Counter that keeps track of how many packets have been sent
    int rec[winSize];
    char ackSent[winSize];

    string tempPkg;
    initArr(ackSent, rec, winSize);

    char start[1];
    start[0] = 'y';

    int totalBytes;
    int lengthOfString;
    int bytesReceived;

    sendto(sockfd, start, 1,
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           serversize);

    printIntArr(rec, winSize);

    for (int i = 0; i < numBuffers; i++)
    {
        memset(buf, 0, bufferSize);
        bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize + 6, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        struct Packet pkg = charsToPacket((char *)buf, bufferSize);
        cout << "Packet " << pkg.sn << " Received.\n";

        numString = to_string(pkg.sn);
        strcpy(snArr, numString.c_str());

        sendto(sockfd, (char *)snArr, sizeof(snArr), MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);
        cout << "Ack " << pkg.sn << " sent.\n";

        isReceived(ackSent, rec, pkg.sn, winSize);
        base = slide(ackSent, rec, base, winSize);
        printIntArr(rec, winSize);

        //printChunk(pkg);

        totalBytes = bytesReceived;
        if (bytesReceived < bufferSize)
        {
            while (totalBytes != bufferSize)
            {

                //Start file transfer
                sendto(sockfd, start, 1, MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);

                bytesReceived = recvfrom(sockfd, (char *)buf, 3000, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
                struct Packet pkg = charsToPacket((char *)buf, bufferSize);
                cout << "Packet " << pkg.sn << " Received.\n";

                //printChunk(pkg);
                numString = to_string(pkg.sn);
                strcpy(snArr, numString.c_str());
                sendto(sockfd, (char *)snArr, sizeof(snArr), MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);
                cout << "Ack " << pkg.sn << " sent.\n";

                isReceived(ackSent, rec, pkg.sn, winSize);
                base = slide(ackSent, rec, base, winSize);
                printIntArr(rec, winSize);

                totalBytes += bytesReceived;
                if (bytesReceived == -1)
                {
                    cerr << "	Error in recvfrom(). Quitting!" << endl;
                    //return -1;
                }
            }

            totalBytes = 0;
        }

        vector<char> buffVec = charToVec(buf, bytesReceived);
        output.write(buffVec.data(), sizeof(char) * 2048);
    }

    int totalBytes2 = 0;
    if (remainingBytes != 0)
    {
        memset(buf, 0, bufferSize);
        bytesReceived = recvfrom(sockfd, buf, remainingBytes, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        totalBytes2 = bytesReceived;
        struct Packet pkg = charsToPacket((char *)buf, remainingBytes);
        cout << "Packet " << pkg.sn << " Received.\n";

        numString = to_string(pkg.sn);
        strcpy(snArr, numString.c_str());
        sendto(sockfd, (char *)snArr, sizeof(snArr), MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);
        cout << "Ack " << pkg.sn << " sent.\n";

        isReceived(ackSent, rec, pkg.sn, winSize);
        base = slide(ackSent, rec, base, winSize);
        printIntArr(rec, winSize);

        //printChunk(pkg);

        vector<char> buffVec = charToVec(pkg.data, bytesReceived);
        output.write(buffVec.data(), sizeof(char) * 16);
    }
}

// Driver code
int main()
{
    int situationalErrors;
    int bufferSize;
    int protocol;
    int slidingWindowSize;
    int sequenceNumberRange;
    int numBuffers;
    int remainingBytes;

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
    //============================================================================
    //++++++++SOCKET CONSTRUCTION+++++++++
    //============================================================================
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
    //============================================================================
    //++++++++RECIEVEING PROTOCOL INFORMATION+++++++++
    //============================================================================
    //recieve bufferSize
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer1[n] = '\0';
    bufferSize = stoi(buffer1, NULL, 10);
    cout << "Buffer Size:" << bufferSize << endl;
    //Recieve Protocol
    memset(buffer1, 0, 2);
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    protocol = stoi(buffer1, NULL, 10);
    cout << "Protocol:" << protocol << endl;

    memset(buffer1, 0, 2);
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    situationalErrors = stoi(buffer1, NULL, 10);
    cout << "Situational Errors:" << situationalErrors << endl;

    memset(buffer1, 0, 2);
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    slidingWindowSize = stoi(buffer1, NULL, 10);
    cout << "Sliding Window Size:" << slidingWindowSize << endl;

    memset(buffer1, 0, 2);
    n = recvfrom(sockfd, (char *)buffer1, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    sequenceNumberRange = stoi(buffer1, NULL, 10);
    cout << "Range of Sequence Numbers: " << sequenceNumberRange << endl;

    int bytesReceived;
    char buf[bufferSize];
    memset(buf, 0, bufferSize);
    string outputname = "outputtest.txt";

    //============================================================================
    //++++++++COLLECTING TRANSFER INFORMATION+++++++++
    //============================================================================
    //---Recieve necessary info from Server before transfer----------------->>>

    memset(buf, 0, bufferSize);
    bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //printf("Contents of Buf #1: %s\n", buf); // This recieve gets numBuffers
    numBuffers = atoi(buf);
    if (bytesReceived == -1)
    {
        cerr << "	Error in recvfrom(). Quitting" << endl;
        return -1;
    }
    memset(buf, 0, bufferSize);
    bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    //printf("Contents of Buf #2: %s\n", buf); //This recieve gets remainingBytes
    remainingBytes = atoi(buf);                                            //deccryption doubles size of message
    ofstream output(outputname.c_str(), std::ios::out | std::ios::binary); //used for writing out to a file
    if (protocol == 2)
    {
        selectiveRepeatReceive(buf, output, remainingBytes, numBuffers, bufferSize);
    }
    if (protocol == 1)
    {
        int cWin = 8;
        if (remainingBytes % 8 != 0)
        {
            remainingBytes = (8 - remainingBytes % 8) + remainingBytes;
        }

        int start[1];
        start[0] = 'y';
        char finished[1];
        finished[0] = 'y';
        char cWinA[1];
        cWinA[0] = cWin;

        int totalBytes;
        int lengthOfString;

        sendto(sockfd, start, 1,
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               serversize);

        //cout << "Start File Transfer." << endl;
        //struct Packet pkg;
        //cout << "Number of Buffers:" << numBuffers << endl;

        for (int i = 0; i < numBuffers; i++)
        {
            memset(buf, 0, bufferSize);
            bytesReceived = recvfrom(sockfd, (char *)buf, bufferSize + 6, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
            struct Packet pkg = charsToPacket((char *)buf, bufferSize);
            if (cWin = pkg.sn)
            {
                cout << "=== Start Chunk =====================================================================>>>\n\n";
                cout << "First loop" << endl;
                cout << "   Packet Sequence Number: " << pkg.sn << endl;
                cout << "   Packet CheckSum: " << pkg.crc << endl;
                cout << "\nPacket Data:\n\n"
                     << pkg.data << endl;
                cout << endl;
                cout << "=== End Chunk =====================================================================>>>\n\n";

                totalBytes = bytesReceived;
                if (bytesReceived < bufferSize)
                {
                    while (totalBytes != bufferSize)
                    {

                        //Start file transfer
                        sendto(sockfd, start, 1, MSG_CONFIRM, (const struct sockaddr *)&servaddr, serversize);

                        bytesReceived = recvfrom(sockfd, (char *)buf, 3000, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
                        struct Packet pkg = charsToPacket((char *)buf, bufferSize);

                        cout << "=== Start Chunk =====================================================================>>>\n\n";
                        cout << "Second loop" << endl;
                        cout << "   Packet Sequence Number: " << pkg.sn << endl;
                        cout << "   Packet CheckSum: " << pkg.crc << endl;
                        cout << "\nPacket Data:\n\n"
                             << pkg.data << endl;
                        cout << endl;
                        cout << "=== End Chunk =====================================================================>>>\n\n";

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
                cout << "Recieved chunk: " << i << "\n\n";
                //==================================send a char back to server==================================

                sendto(sockfd, finished, 1,
                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                       serversize);

                //==============================================================================================
                output.write(buffVec.data(), sizeof(char) * 2048);
                //cout << "Writing to output file." << endl;
                sendto(sockfd, cWinA, 1,
                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                       serversize);
                cWin++;
            }
        }
        int totalBytes2 = 0;
        if (remainingBytes != 0)
        {
            memset(buf, 0, bufferSize);
            bytesReceived = recvfrom(sockfd, buf, remainingBytes, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);

            totalBytes2 = bytesReceived;
            struct Packet pkg = charsToPacket((char *)buf, remainingBytes);
            cout << "=== Start Chunk =====================================================================>>>\n\n";
            cout << "Last If" << endl;
            cout << "   Packet Sequence Number: " << pkg.sn << endl;
            cout << "   Packet CheckSum: " << pkg.crc << endl;
            cout << "\nPacket Data:\n\n"
                 << pkg.data << endl;
            cout << endl;
            cout << "=== End Chunk =====================================================================>>>\n\n";

            cout << "Writing to last chunk to output file" << endl;
            vector<char> buffVec = charToVec(buf, bytesReceived);
            output.write(buffVec.data(), sizeof(char) * 16);
        }
    }
    output.close();
    close(sockfd);
    return 0;
}
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
