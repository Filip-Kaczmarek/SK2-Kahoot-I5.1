#pragma once
#include"protocolmsg.h"
#include <unistd.h>
#include<arpa/inet.h>
#include<iostream>

using namespace std;

class HeadData {
private:
    char buffer[BASE_BUFFER_SIZE];
    const char *bp;
    unsigned int protocolId;
    unsigned int account;
    unsigned int dataType;
    unsigned int dataLength;

    bool baseParse();

    unsigned int parseInt(int len);

public:
    HeadData(int fd);

    HeadData();

    bool parse(const char *buffer);

    unsigned int getProtocolId();

    unsigned int getAccount();

    unsigned int getDataType();

    unsigned int getDataLength();

};

