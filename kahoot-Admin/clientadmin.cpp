#include "clientadmin.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QString>

ClientAdmin::ClientAdmin(QObject *parent)
    : QObject(parent)
    , m_clientSocket(new QTcpSocket(this))
    , m_loggedIn(false)
{
    connect(m_clientSocket, &QTcpSocket::connected, this, &ClientAdmin::connected);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &ClientAdmin::disconnected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &ClientAdmin::readServerMsg);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, [this]()->void{m_loggedIn = false;});
}

void ClientAdmin:: writeText(unsigned int account ,string text, unsigned int protocolId){
    DataEncoder encoder;
    string headStr = encoder.encode(protocolId, account, TEXT, text.length());
    m_clientSocket->write(headStr.data(), headStr.length());
    //text = headStr + text;
    if(text.length() != 0){
        m_clientSocket->write(text.data(), text.length());
    }
}

void ClientAdmin::readServerMsg(){
    readHeadData();
    m_clientSocket->read(buffer, dataLength);
    QString serverMsg = QString::fromStdString(string(buffer, dataLength));
    QJsonObject serverInfoObj = QJsonDocument::fromJson(serverMsg.toUtf8()).object();
    jsonReceived(serverInfoObj);
}

void ClientAdmin:: readHeadData(){
    m_clientSocket->read(buffer, BASE_BUFFER_SIZE);
    DataParser parser(buffer);
    parser.baseParse();
    protocolId = parser.getProtocolId();
    account = parser.getAccount();
    dataType = parser.getDataType();
    dataLength = parser.getDataLength();
}

void ClientAdmin::showActiveGames()
{
    QJsonObject information;
    QString nazwa="";
    information.insert("text", "admin");
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    writeText(nazwa.toUtf8().toInt(), loginMsg, ADMIN);
}

void ClientAdmin::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}

void ClientAdmin::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVall = docObj.value(QLatin1String("status"));
    int typeVal = docObj["type"].toInt();
    if (typeVal==LOGIN_SUCCESS) {
        if (m_loggedIn)
            return;
        const QJsonValue resultVal = docObj.value(QLatin1String("username"));
        int loginSuccess = resultVal.toInt();
        if (loginSuccess==1) {
            emit loggedIn();
            return;
        }
    }
    else if (typeVal==ADMIN_GAMES) {
        emit admin(docObj);
    }
}

void ClientAdmin::connectToServer(const QHostAddress &address, quint16 port)
{
    qDebug()<<address;
    qDebug()<<port;
    m_clientSocket->connectToHost(address, port);
}
