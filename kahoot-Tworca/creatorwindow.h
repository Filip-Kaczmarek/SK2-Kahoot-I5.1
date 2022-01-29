#ifndef CREATORWINDOW_H
#define CREATORWINDOW_H

#include <QWidget>
#include <QAbstractSocket>
class ClientCreator;
class QStandardItemModel;
namespace Ui { class CreatorWindow; }
class CreatorWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CreatorWindow)
public:
    explicit CreatorWindow(QWidget *parent = nullptr);
    ~CreatorWindow();
private:
    Ui::CreatorWindow *ui;
    ClientCreator *m_creatorClient;
    QStandardItemModel *m_creatorModel;
    QString m_lastUserName;
private slots:
    void attemptConnection();
    void connectedToServer();
    void attemptLogin(const QString &userName);
    void loggedIn();
    void delay(int s);
    void loginFailed(const QString &reason);
    void gameCodeReceived(int code);
    void sendAnswer();
    void showCommunicate();
    void disconnectedFromServer();
    void error(QAbstractSocket::SocketError socketError);
    void requestCode();
    void saveGame();
    void startGame();
    void checkEdits();
    void sendClose();
    void waitForPlayers();
    void showRanking(const QJsonObject &ranking);
    void requestRanking();    
};

#endif // CREATORWINDOW_H
