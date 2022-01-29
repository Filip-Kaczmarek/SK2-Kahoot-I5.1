#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QWidget>
#include <QAbstractSocket>
#include <QJsonArray>
class ClientPlayer;
class QStandardItemModel;
namespace Ui { class PlayerWindow; }
class PlayerWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlayerWindow)
public:
    explicit PlayerWindow(QWidget *parent = nullptr);
    ~PlayerWindow();
private:
    Ui::PlayerWindow *ui;
    ClientPlayer *m_playerClient;
    QStandardItemModel *m_playerModel;
    QString m_lastUserName;

private slots:
    void attemptConnection();
    void sendGameCode();
    void connectedToServer();
    void loggedIn();
    void loginFailed(const QString &reason);
    void game(QJsonArray &questions);
    void passGameCode(const QString &text);
    void answer1();
    void answer2();
    void answer3();
    void answer4();
    void disconnectedFromServer();
    void error(QAbstractSocket::SocketError socketError);
    void delay(int s);
    void showRanking(const QJsonObject &ranking);
    void sendClose();
};

#endif // PLAYERWINDOW_H
