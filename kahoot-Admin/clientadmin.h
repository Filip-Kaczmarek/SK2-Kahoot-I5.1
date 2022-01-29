#ifndef CLIENTADMIN_H
#define CLIENTADMIN_H

#include <QObject>
#include <QTcpSocket>
#include<QtAlgorithms>
#include <string>
#include <fstream>
#include <QMessageBox>
#include<QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include"dataencoder.h"
#include"dataparser.h"
#define TCP_BUFSIZ 8192
using namespace std;
class QHostAddress;
class QJsonDocument;
class ClientAdmin : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientAdmin)
public:
    explicit ClientAdmin(QObject *parent = nullptr);
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
    void showActiveGames();
private slots:

signals:
    void connected();
    void loggedIn();
    void loginError(const QString &reason);
    void disconnected();
    void admin(const QJsonObject &admin);
    void error(QAbstractSocket::SocketError socketError);
private:
    QTcpSocket *m_clientSocket;
    bool m_loggedIn;
    void jsonReceived(const QJsonObject &doc);

};

#endif // CLIENTADMIN_H
