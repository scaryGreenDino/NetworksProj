
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

void initArr(char *ackArray, int* arr, int winSize)
{

    //cout << "Size of ackArray: " << sizeof(ackArray) << endl;
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

void printCharArr(char* arr, int winSize) {
    cout << "Acks = [";
    for (int i = 0; i < winSize; i++)
    {
        if (i < (winSize-1)) {
            cout << arr[i] << ", ";
        }
        else {
            cout << arr[i] << "]\n";
        }
    }
} 

void printIntArr(int* arr, int winSize) {
    cout << "Current window = [";
    for (int i = 0; i < winSize; i++)
    {
        if (i < (winSize - 1)) {
            cout << arr[i] << ", ";
        }
        else {
            cout << arr[i] << "]\n";
        }
    }
}

int slide(char *window, int* arr, int base, int winSize)
{
    char tmpC[winSize];
    int tmpI[winSize];
    while (window[0] == 'y')
    {
        for (int j = 0; j < winSize; j++) {
            if (j < (winSize - 1)) {
                tmpC[j] = window[j + 1];
                tmpI[j] = arr[j + 1];
            }
            else if ((winSize + base) >= (winSize * 3)) { //Edge Case
                tmpC[j] = 'n';
                tmpI[j] = (base + winSize) - (winSize * 3);
                base++;
            }
            else {
                tmpC[j] = 'n';
                tmpI[j] = base + winSize;
                base++;
            }

            if (base >= (winSize * 3)) { //Resets the first entry back to zero
                base = 0;
            }

            //cout << "Ack Arr[" << j << "]: " << tmpC[j] << endl;
            //cout << "Send Arr[" << j << "]: " << tmpI[j] << endl;
        }

        strcpy(window, tmpC);
        memcpy(arr, tmpI, winSize*sizeof(int));

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

void selectiveRepeatSend(vector<char> buffer, ifstream &fin, int remainingBytesInFile, int numBuffers, int bufferSize) {

    int seqNum = 0;
    int winSize = 5; //Total number a frames inside the window
    int seqRange = winSize * 3; //Range of sequence numbers given to the frames
    int base = 0;         //Counter that keeps track of how many packets have been sent
    int send[winSize];
    char recAck[winSize];
    char* pktStore[numBuffers]; //Stores each packet for reference if any get lost

    initArr(recAck, send, winSize);

    /*//printCharArr(recAck, winSize);
    recAck[0] = 'y';
    //recAck[1] = 'y';
    recAck[2] = 'y';
    recAck[3] = 'y';
    //printCharArr(recAck, winSize);

    //recAck[4] = 'y';
    printCharArr(recAck, winSize);
    printIntArr(send, winSize);

    base = slide(recAck, send, base, winSize);
    printCharArr(recAck, winSize);
    printIntArr(send, winSize);*/

    char finished[1];
    finished[0] = 'y';
    string result;
    for (int i = 0; i < numBuffers; i++)
    {
        fin.read(buffer.data(), buffer.size());
        result = vecToString(buffer);

        //========== Get finish Ack from Client =============================

        while (finished[0] == 'n')
        {
            recvfrom(sockfd, finished, 1, MSG_WAITALL, (struct sockaddr*) & cliaddr, &len);
            //cout << "Ack " << seqNum << " recieved.\n";

        } //wait for 'y'

        finished[0] = 'n';

        //===================================================================

        cout << "Packet " << seqNum << " sent.\n";
        char* pkg = packetMaker(seqNum, result.c_str(), result.size());
        pktStore[seqNum - 1] = pkg;
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr*) & cliaddr, len);
        //printf("Packet Storage Array: %s\n", pktStore[seqNum - 1]);
        if (seqNum >= seqRange) {
            seqNum = 0;
        }
        else {
            seqNum++;
        }
        

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

        char* pkg = packetMaker(seqNum, lastBuffString.c_str(), lastBuffString.size());
        pktStore[seqNum - 1] = pkg;
        //printf("Packet Storage Array: %s\n", pktStore[seqNum - 1]);
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr*) & cliaddr,
            len);
        seqNum++;

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

    //int sockfd;
    char buffer1[MAXLINE];
    //struct sockaddr_in servaddr, cliaddr;

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
    len = sizeof(cliaddr); //len is value/resuslt

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

    sendto(sockfd, (const char *)conAck, strlen(conAck),
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

    long long int checkSum = 0123456; //Check sum value for packet contents
    unsigned short seqNum = 0;        //Sequence number for the packet

    string filename = "test.txt";

    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int sizeOfFile = rc == 0 ? stat_buf.st_size : -1;

    int numBuffers = (int)sizeOfFile / dataSize; //numBuffers will always be an integer
    //int numBuffers = (int)sizeOfFile / bufferSize; //numBuffers will always be an integer (OG)
    int remainingBytesInFile = (int)sizeOfFile % bufferSize;

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

    ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    

    vector<char> buffer(dataSize, 0);
    //vector<char> buffer(bufferSize, 0);

    selectiveRepeatSend(buffer, fin, remainingBytesInFile, numBuffers, bufferSize);

    /* char finished[1];
    finished[0] = 'y';
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

        seqNum++;
        char *pkg = packetMaker(seqNum, result.c_str(), result.size());
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

        char* pkg = packetMaker(seqNum, lastBuffString.c_str(), lastBuffString.size());
        sendRe = sendto(sockfd, pkg, bufferSize, MSG_CONFIRM, (const struct sockaddr*) & cliaddr,
                           len);

        if (sendRe == -1)
        {
            cout << "Could not send to server! Whoops!!\r\n";
        }
    } */

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

