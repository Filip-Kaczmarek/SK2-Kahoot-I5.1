#include "clientcreator.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <QString>
using namespace std;
ClientCreator::ClientCreator(QObject *parent)
    : QObject(parent)
    , m_clientSocket(new QTcpSocket(this))
    , m_loggedIn(false)
{
    connect(m_clientSocket, &QTcpSocket::connected, this, &ClientCreator::connected);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &ClientCreator::disconnected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &ClientCreator::readServerMsg);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, [this]()->void{m_loggedIn = false;});
}

void ClientCreator::login(const QString &userName){
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        QDataStream clientStream(m_clientSocket);
        clientStream.setVersion(QDataStream::Qt_5_7);
        QJsonObject message;
        message[QStringLiteral("type")] = QStringLiteral("login");
        message[QStringLiteral("username")] = userName;
        QJsonDocument doc;
        doc.setObject(message);
        string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
        clientStream << QJsonDocument(message).toJson(QJsonDocument::Compact);
    }
}

void ClientCreator::generate(){
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        QDataStream clientStream(m_clientSocket);
        clientStream.setVersion(QDataStream::Qt_5_7);
        QJsonObject message;
        message[QStringLiteral("type")] = QStringLiteral("kod");
        message[QStringLiteral("text")] = "generuj";
        clientStream << QJsonDocument(message).toJson(QJsonDocument::Compact);
    }
}

void ClientCreator:: writeText(unsigned int account ,string text, unsigned int protocolId){
    DataEncoder encoder;
    string headStr = encoder.encode(protocolId, account, TEXT, text.length());
    m_clientSocket->write(headStr.data(), headStr.length());
    //text = headStr + text;
    if(text.length() != 0){
        m_clientSocket->write(text.data(), text.length());
    }
    m_clientSocket->waitForBytesWritten();
}

void ClientCreator::readServerMsg(){
    readHeadData();
    m_clientSocket->read(buffer, dataLength);
    QString serverMsg = QString::fromStdString(string(buffer, dataLength));
    QJsonObject serverInfoObj = QJsonDocument::fromJson(serverMsg.toUtf8()).object();
    qDebug()<<"JSON DOTARL";
    qDebug() << serverInfoObj["type"];
    qDebug() << serverInfoObj["code"];
    jsonReceived(serverInfoObj);

}

void ClientCreator:: readHeadData(){
    m_clientSocket->read(buffer, BASE_BUFFER_SIZE);
    DataParser parser(buffer);
    parser.baseParse();
    protocolId = parser.getProtocolId();
    account = parser.getAccount();
    dataType = parser.getDataType();
    dataLength = parser.getDataLength();
    qDebug()<<protocolId;
    qDebug()<<account;
    qDebug()<<dataType;
    qDebug()<<dataLength;
}

void ClientCreator::saveGame(){
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState) { // if the client is connected
        QDataStream clientStream(m_clientSocket);
        clientStream.setVersion(QDataStream::Qt_5_7);
        QJsonObject message;
        message[QStringLiteral("type")] = QStringLiteral("save");
        message[QStringLiteral("text")] = "game";
        clientStream << QJsonDocument(message).toJson(QJsonDocument::Compact);
    }
}

void ClientCreator::startGame(){
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        QDataStream clientStream(m_clientSocket);
        clientStream.setVersion(QDataStream::Qt_5_7);
        QJsonObject message;
        message[QStringLiteral("type")] = QStringLiteral("start");
        message[QStringLiteral("text")] = "game";
        clientStream << QJsonDocument(message).toJson(QJsonDocument::Compact);
    }
}

void ClientCreator::sendAnswer(
        const QString &username,
        const QString &time,
        const QString &question,
        const QString &answer1,
        const QString &answer2,
        const QString &answer3,
        const QString &answer4,
        const QString &correct,int kod)
{
    if (question.isEmpty())
        return;
    QDataStream clientStream(m_clientSocket);
    clientStream.setVersion(QDataStream::Qt_5_7);
    QJsonObject message;
    message[QStringLiteral("creator")]=username;
    message[QStringLiteral("id")] = 1;
    message[QStringLiteral("type")] = QStringLiteral("question");
    message[QStringLiteral("ques")] = question;
    message[QStringLiteral("anso")] = answer1;
    message[QStringLiteral("anst")] = answer2;
    message[QStringLiteral("ansth")] = answer3;
    message[QStringLiteral("ansf")] = answer4;
    message[QStringLiteral("cor")] = correct;
    message[QStringLiteral("code")] = kod;
    message[QStringLiteral("time")] = time;
    qDebug()<<time;
    QJsonDocument doc;
    doc.setObject(message);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    writeText(username.toUtf8().toInt(), loginMsg, SENDQUESTION);
    qDebug()<<doc;
}

void ClientCreator::sendQuestion( const QString &text,const QString &kod){
    if (text.isEmpty())
        return;
    QDataStream clientStream(m_clientSocket);
    clientStream.setVersion(QDataStream::Qt_5_7);
    QJsonObject message;
    message[QStringLiteral("id")] = kod;
    message[QStringLiteral("type")] = QStringLiteral("question");
    message[QStringLiteral("text")] = text;
    clientStream << QJsonDocument(message).toJson();
}

void ClientCreator::disconnectFromHost(){

    m_clientSocket->disconnectFromHost();
}


void ClientCreator::jsonReceived(const QJsonObject &docObj){
    qDebug()<<"JEDZIEM";
    qDebug()<<docObj;
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
        const QJsonValue reasonVal = docObj.value(QLatin1String("reason"));
        emit loginError(reasonVal.toString());
    }  else if (typeVal==REQUESTCODE_SEND) {
        int typeVal=docObj["type"].toInt();
        int codeVal = docObj["code"].toInt();
        qDebug()<<codeVal;
        emit gameCodeReceived(codeVal);
    }  else if (typeVal==ACCEPTQUESTION) {
        int typeVal=docObj["type"].toInt();
        qDebug()<<typeVal;
        emit showCommunicate();
    }   else if (typeVal==NOTENOUGHPLAYERS) {
        int typeVal=docObj["type"].toInt();
        qDebug()<<typeVal;
        emit waitForPlayers();
    }    else if (typeVal==CREATOR_RANK_SEND) {
        qDebug()<<docObj;
        emit ranking(docObj);
    }
}

void ClientCreator::connectToServer(const QHostAddress &address, quint16 port){
    qDebug()<<address;
    qDebug()<<port;
    m_clientSocket->connectToHost(address, port);
}

void ClientCreator::onReadyRead(){
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);
    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject())
                    jsonReceived(jsonDoc.object());
            }
        } else {
            break;
        }
    }
}
