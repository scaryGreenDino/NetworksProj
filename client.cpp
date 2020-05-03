
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
struct Packet
{
    short sn;
    uint32_t crc;
    char *data;
};
const uint32_t Polynomial = 0xEDB88320;
string buffToString(char *buf, int bytes);
vector<char> charToVec(char *str, int length);

char *substr(char *arr, int begin, int len)
{
    char *res = new char[len];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
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

struct Packet charsToPacket(char *chars, int dataSize)
{
    int c;
    short csSize = 4;
    short snSize = 2;
    struct Packet packet;
    char *cs = substr(chars, 0, 3);
    char *sn = substr(chars, 4, 5);
    packet.crc = stoi(cs, NULL, 16);
    packet.sn = stoi(sn, NULL, 16);
    packet.data = substr(chars, 6, dataSize - 1);
    return packet;
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
    for (int c; c < 17; c++)
    {
        cout << buf[c];
    }
    cout << endl;
    struct Packet packet = charsToPacket(buf, 17);
    cout << packet.crc << endl;
    cout << packet.sn << endl;
    cout << packet.data << endl;
    close(sockfd);
    return 0;
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
