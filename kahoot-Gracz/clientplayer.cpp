#include "clientplayer.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <QString>

ClientPlayer::ClientPlayer(QObject *parent)
    : QObject(parent)
    , m_clientSocket(new QTcpSocket(this))
    , m_loggedIn(false)
{
    connect(m_clientSocket, &QTcpSocket::connected, this, &ClientPlayer::connected);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &ClientPlayer::disconnected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &ClientPlayer::readServerMsg);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, [this]()->void{m_loggedIn = false;});
}

void ClientPlayer:: writeText(unsigned int account ,string text, unsigned int protocolId){
    DataEncoder encoder;
    string headStr = encoder.encode(protocolId, account, TEXT, text.length());
    m_clientSocket->write(headStr.data(), headStr.length());
    //text = headStr + text;
    if(text.length() != 0){
        m_clientSocket->write(text.data(), text.length());
    }
}

void ClientPlayer::readServerMsg(){
    readHeadData();
    m_clientSocket->read(buffer, dataLength);
    QString serverMsg = QString::fromStdString(string(buffer, dataLength));
    QJsonObject serverInfoObj = QJsonDocument::fromJson(serverMsg.toUtf8()).object();
    jsonReceived(serverInfoObj);

}
void ClientPlayer:: readHeadData(){
    m_clientSocket->read(buffer, BASE_BUFFER_SIZE);
    DataParser parser(buffer);
    parser.baseParse();
    protocolId = parser.getProtocolId();
    account = parser.getAccount();
    dataType = parser.getDataType();
    dataLength = parser.getDataLength();
}

void ClientPlayer::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}

void ClientPlayer::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVall = docObj.value(QLatin1String("status"));
    int typeVal = docObj["type"].toInt();
    qDebug()<<typeVal;
    if (typeVal==LOGIN_SUCCESS) {
        qDebug()<<"Logged in";
        if (m_loggedIn)
            return;
        const QJsonValue resultVal = docObj.value(QLatin1String("username"));
        qDebug()<<resultVal;
        int loginSuccess = resultVal.toInt();
        qDebug()<<loginSuccess;
        if (loginSuccess==1) {
            emit loggedIn();
            return;
        }

    }  else if (typeVal==CODECHECKPLAYER_ACCEPT) {
        const QJsonValue  code =docObj["code"];
        const QJsonValue  text =docObj["text"];
        emit kodgry(code.toString());
    } else if (typeVal==NOTICE){
        qDebug()<<typeVal;
        qDebug()<<docObj;
        int numVal = docObj["number"].toInt();
        qDebug()<<"NUM";
        qDebug()<<numVal;
        QJsonArray questions;
        for(int i=1;i<=numVal;i++)
        {
            QString num2 = QString::number(i);
            questions.append(docObj[num2]);
        }

        qDebug()<<questions;
        emit game(questions);
    }
    else if (typeVal==CREATOR_RANK_SEND) {
           qDebug()<<docObj;
           emit ranking(docObj);
       }
}

void ClientPlayer::connectToServer(const QHostAddress &address, quint16 port){
    qDebug()<<address;
    qDebug()<<port;
    m_clientSocket->connectToHost(address, port);
}

