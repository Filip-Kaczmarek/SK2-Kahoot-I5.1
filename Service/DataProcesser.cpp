#include "DataProcesser.h"

DataProcesser::DataProcesser() {

}

int DataProcesser::checkSocketConnected(int sock) {
    if (sock <= 0)
        return 0;
    tcp_info info{};
    int len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *) &len);
    if (info.tcpi_state == TCP_ESTABLISHED) {
        return 1;
    } else {
        return 0;
    }
}

string DataProcesser::readTextContent(int fd, unsigned int dataLength) {
    unsigned int count = 0;
    int size = 0;
    unsigned int buffSize = TCP_BUFSIZ;
    string content;
    while (true) {
        if (checkSocketConnected(fd) == 0) {
            break;
        }
        if((size = read(fd, buffer, min(buffSize, dataLength - count))) <= 0){
            break;
        }
        if (size > 0) {
            count += size;
            content += string(buffer, size);
        }
        if (count == dataLength) {
            break;
        }
    }
    return content;
}


void DataProcesser::writeMsg(int fd, unsigned int account, string text, unsigned int protocolId) {
    DataEncoder de;
    string headStr = de.encode(protocolId, account, TEXT, text.length());
    text = headStr + text;
    cout<<endl<<text;
    send(fd, text.data(), text.length(), MSG_NOSIGNAL);
}


void DataProcesser::writeTextToAllUser(const vector<int> &fds, int account, const string &text, unsigned int protocolId) {
    for (auto &fd : fds) {
        writeMsg(fd, account, text, protocolId);
    }
}


