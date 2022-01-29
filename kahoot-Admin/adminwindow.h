#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QAbstractSocket>
class ClientAdmin;
class QStandardItemModel;
namespace Ui { class AdminWindow; }
class AdminWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AdminWindow)
public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();
private:
    Ui::AdminWindow *ui;
    ClientAdmin *m_clientAdmin;
    QStandardItemModel *m_chatModel;
    QString m_lastUserName;

private slots:
    void attemptConnection();
    void connectedToServer();
    void delay(int s);
    void loggedIn();
    void loginFailed(const QString &reason);
    void disconnectedFromServer();
    void error(QAbstractSocket::SocketError socketError);
    void requestActiveGames();
    void showAdmin(const QJsonObject &admin);
};

#endif // ADMINWINDOW_H
