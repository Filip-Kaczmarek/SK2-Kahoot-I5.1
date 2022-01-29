#ifndef CLIENTPLAYER_H
#define CLIENTPLAYER_H

#include <QObject>
#include <QTcpSocket>
#include <string>
#include <QJsonArray>
#include"dataencoder.h"
#include"dataparser.h"
#define TCP_BUFSIZ 8192
using namespace std;
class QHostAddress;
class QJsonDocument;
class ClientPlayer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientPlayer)
public:
    explicit ClientPlayer(QObject *parent = nullptr);
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
    void disconnectFromHost();
private slots:

signals:
    void connected();
    void aboutToQuit();
    void loggedIn();
    void loginError(const QString &reason);
    void disconnected();
    void game(QJsonArray &questions);
    void kodgry(const QString &text);
    void error(QAbstractSocket::SocketError socketError);
    void ranking(const QJsonObject &ranking);
private:
    QTcpSocket *m_clientSocket;
    bool m_loggedIn;
    void jsonReceived(const QJsonObject &doc);

};

#endif // CLIENTPLAYER_H
