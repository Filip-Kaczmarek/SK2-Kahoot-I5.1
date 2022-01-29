#include "adminwindow.h"
#include "ui_adminwindow.h"
#include "clientadmin.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
QString CorrectAnswer;
AdminWindow::AdminWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminWindow)
    , m_clientAdmin(new ClientAdmin(this))
    , m_chatModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_chatModel->insertColumn(0);
    ui->chatView->setModel(m_chatModel);
    connect(m_clientAdmin, &ClientAdmin::connected, this, &AdminWindow::connectedToServer);
    connect(m_clientAdmin, &ClientAdmin::loggedIn, this, &AdminWindow::loggedIn);
    connect(m_clientAdmin, &ClientAdmin::loginError, this, &AdminWindow::loginFailed);
    connect(m_clientAdmin, &ClientAdmin::disconnected, this, &AdminWindow::disconnectedFromServer);
    connect(m_clientAdmin, &ClientAdmin::error, this, &AdminWindow::error);
    connect(m_clientAdmin, &ClientAdmin::admin, this, &AdminWindow::showAdmin);
    connect(ui->connectButton, &QPushButton::clicked, this, &AdminWindow::attemptConnection);
    connect(ui->gameButton, &QPushButton::clicked, this, &AdminWindow::requestActiveGames);
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::delay(int s)
{
    QTime dieTime= QTime::currentTime().addSecs(s);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void AdminWindow::showAdmin(const QJsonObject &admin)
{
    m_chatModel->removeRows(0,m_chatModel->rowCount());
    QJsonValue gamesn = admin["gamesn"];
    QJsonValue playersn = admin["playersn"];
    QString gamesn_str = gamesn.toString();
    int gamesn_int = gamesn_str.toInt();
    QString playersn_str = playersn.toString();
    int playersn_int = playersn_str.toInt();
    int sum = gamesn_int + playersn_int+2;
    int newRow = m_chatModel->rowCount();
    m_chatModel->insertRows(newRow, sum);
    QFont boldFont;
    boldFont.setBold(true);
    m_chatModel->setData(m_chatModel->index(newRow, 0), "Current games and creators:");
    m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_chatModel->setData(m_chatModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    for(int i=1; i<=gamesn_int;i++)
    {
       QString index = QString::number(i);
       QJsonValue creator_info = admin[index];
       m_chatModel->setData(m_chatModel->index(newRow, 0), creator_info.toString());
       m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
       ++newRow;
    }
    boldFont.setBold(true);
    m_chatModel->setData(m_chatModel->index(newRow, 0), "Current players ranking:");
    m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_chatModel->setData(m_chatModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    for(int i=1;i<=playersn_int;i++)
    {
        QString index = "Rank"+QString::number(i);
        QJsonValue creator_info = admin[index];
        m_chatModel->setData(m_chatModel->index(newRow, 0), creator_info.toString());
        m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ++newRow;
    }
}

void AdminWindow::requestActiveGames()
{
    ui->gameButton->setEnabled(false);
    QMessageBox::information(this, tr("Connection"), tr("You have requested active games list from server!"));
    m_clientAdmin->showActiveGames();
    delay(3);
    ui->gameButton->setEnabled(true);
}

void AdminWindow::attemptConnection()
{
    const QString hostAddress = QInputDialog::getText(
        this
        , tr("Choose Server")
        , tr("Server Address")
        , QLineEdit::Normal
        , QStringLiteral("127.0.0.1")
    );
    if (hostAddress.isEmpty())
        return;
    ui->connectButton->setEnabled(true);
    m_clientAdmin->connectToServer(QHostAddress(hostAddress), 1967);
}

void AdminWindow::connectedToServer()
{
    ui->connectButton->setEnabled(false);
    const QString newUsername = QInputDialog::getText(
                this
                , tr("Enter your username")
                , tr("Nick")
    );
    if (newUsername.isEmpty()){
        return m_clientAdmin->disconnectFromHost();
    }
    QJsonObject information;
    information.insert("account", newUsername);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_clientAdmin->writeText(newUsername.toUtf8().toInt(), loginMsg, LOGIN);
    ui->gameButton->setEnabled(true);
}


void AdminWindow::loggedIn()
{
    ui->chatView->setEnabled(true);
    ui->gameButton->setEnabled(true);
    m_lastUserName.clear();
    QMessageBox::information(this, tr("Connection"), tr("You have sucessfully logged in"));
}

void AdminWindow::loginFailed(const QString &reason)
{
    QMessageBox::critical(this, tr("Error"), reason);
    connectedToServer();
}

void AdminWindow::disconnectedFromServer()
{
    QMessageBox::warning(this, tr("Disconnected"), tr("The host terminated the connection"));
    ui->chatView->setEnabled(false);
    ui->gameButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    m_lastUserName.clear();
}

void AdminWindow::error(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::ProxyConnectionClosedError:
        return;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The host refused the connection"));
        break;
    case QAbstractSocket::ProxyConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The proxy refused the connection"));
        break;
    case QAbstractSocket::ProxyNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the proxy"));
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the server"));
        break;
    case QAbstractSocket::SocketAccessError:
        QMessageBox::critical(this, tr("Error"), tr("You don't have permissions to execute this operation"));
        break;
    case QAbstractSocket::SocketResourceError:
        QMessageBox::critical(this, tr("Error"), tr("Too many connections opened"));
        break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::warning(this, tr("Error"), tr("Operation timed out"));
        return;
    case QAbstractSocket::ProxyConnectionTimeoutError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy timed out"));
        break;
    case QAbstractSocket::NetworkError:
        QMessageBox::critical(this, tr("Error"), tr("Unable to reach the network"));
        break;
    case QAbstractSocket::UnknownSocketError:
        QMessageBox::critical(this, tr("Error"), tr("An unknown error occured"));
        break;
    case QAbstractSocket::UnsupportedSocketOperationError:
        QMessageBox::critical(this, tr("Error"), tr("Operation not supported"));
        break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        QMessageBox::critical(this, tr("Error"), tr("Your proxy requires authentication"));
        break;
    case QAbstractSocket::ProxyProtocolError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy comunication failed"));
        break;
    case QAbstractSocket::TemporaryError:
    case QAbstractSocket::OperationError:
        QMessageBox::warning(this, tr("Error"), tr("Operation failed, please try again"));
        return;
    default:
        Q_UNREACHABLE();
    }
    ui->connectButton->setEnabled(true);
    ui->chatView->setEnabled(false);
    m_lastUserName.clear();
}
