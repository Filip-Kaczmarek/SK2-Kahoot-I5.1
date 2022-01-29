#ifndef CLIENTCREATOR_H
#define CLIENTCREATOR_H

#include <QObject>
#include <QTcpSocket>
#include <string>
#include"dataencoder.h"
#include"dataparser.h"
#define TCP_BUFSIZ 8192
using namespace std;
class QHostAddress;
class QJsonDocument;
class ClientCreator : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientCreator)
public:
    explicit ClientCreator(QObject *parent = nullptr);
    void writeText(unsigned int account , string text, unsigned int protocolId);
    void readHeadData();
    void readServerMsg();
    unsigned int protocolId;
    unsigned int account;
    unsigned int dataType;
    unsigned int dataLength;
    char buffer[TCP_BUFSIZ];
public slots:
    void connectToServer(const QHostAddress &address, quint16 port);
    void login(const QString &userName);
    void sendAnswer(const QString &username,const QString &time,const QString &question,const QString &answer1,const QString &answer2,const QString &answer3,const QString &answer4,const QString &correct,int kod);
    void sendQuestion(const QString &text,const QString &kod);
    void disconnectFromHost();
    void generate();
    void saveGame();
    void startGame();
private slots:
    void onReadyRead();
signals:
    void aboutToQuit();
    void connected();
    void loggedIn();
    void loginError(const QString &reason);
    void disconnected();
    void messageReceived(const QString &sender, const QString &text);
    void gameCodeReceived(int code);
    void showCommunicate();
    void waitForPlayers();
    void ranking(const QJsonObject &ranking);
    void error(QAbstractSocket::SocketError socketError);
private:
    QTcpSocket *m_clientSocket;
    bool m_loggedIn;
    void jsonReceived(const QJsonObject &doc);
};

#endif // CLIENTCREATOR_H
