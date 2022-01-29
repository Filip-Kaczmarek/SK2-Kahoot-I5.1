#pragma once
#include<iostream>
#include<string>
#include<cstring>
#include<unistd.h>
#include<vector>
#include"../ProtocolHead/protocolmsg.h"
#include<fstream>
#include<arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include"../ProtocolHead/HeadData.h"
#include "../ProtocolHead/DataEncoder.h"

#define IMAGE_PATH "./image/"
#define TCP_BUFSIZ 8192
using namespace std;

class DataProcesser {
private:
    char buffer[TCP_BUFSIZ];

    int checkSocketConnected(int sock);

public:
    DataProcesser();

    string readTextContent(int fd, unsigned int dataLength);

    void writeMsg(int fd, unsigned int account, string text, unsigned int protocolId = SEND);

    void writeTextToAllUser(const vector<int> &fds, int account, const string &text, unsigned int protocolId = SEND);

};
