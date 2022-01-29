#include "playerwindow.h"
#include "ui_playerwindow.h"
#include "clientplayer.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCloseEvent>
#include <QTimer>

QString CorrectAnswer;
QString playerName;
QString gameCode;
QString selectedAnswer;

PlayerWindow::PlayerWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayerWindow)
    , m_playerClient(new ClientPlayer(this))
    , m_playerModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_playerModel->insertColumn(0);
    ui->chatView->setModel(m_playerModel);
    connect(m_playerClient, &ClientPlayer::connected, this, &PlayerWindow::connectedToServer);
    connect(m_playerClient, &ClientPlayer::loggedIn, this, &PlayerWindow::loggedIn);
    connect(m_playerClient, &ClientPlayer::loginError, this, &PlayerWindow::loginFailed);
    connect(m_playerClient, &ClientPlayer::kodgry, this, &PlayerWindow::passGameCode);
    connect(m_playerClient, &ClientPlayer::disconnected, this, &PlayerWindow::disconnectedFromServer);
    connect(m_playerClient, &ClientPlayer::error, this, &PlayerWindow::error);
    connect(m_playerClient, &ClientPlayer::ranking, this, &PlayerWindow::showRanking);
    connect(m_playerClient, &ClientPlayer::game, this, &PlayerWindow::game);
    connect(ui->connectButton, &QPushButton::clicked, this, &PlayerWindow::attemptConnection);
    connect(ui->gameButton, &QPushButton::clicked, this, &PlayerWindow::sendGameCode);
    connect(ui->ans1, &QPushButton::clicked, this, &PlayerWindow::answer1);
    connect(ui->ans2, &QPushButton::clicked, this, &PlayerWindow::answer2);
    connect(ui->ans3, &QPushButton::clicked, this, &PlayerWindow::answer3);
    connect(ui->ans4, &QPushButton::clicked, this, &PlayerWindow::answer4);
}

PlayerWindow::~PlayerWindow(){
    delete ui;
}

void PlayerWindow::delay(int s){
    QTime dieTime= QTime::currentTime().addSecs(s);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void PlayerWindow::sendClose(){
    string loginMsg="puste";
    qDebug()<<"elo";
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, CLOSE);

}

void PlayerWindow::attemptConnection(){
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
    m_playerClient->connectToServer(QHostAddress(hostAddress), 1967);
}

void PlayerWindow::sendGameCode(){
    const QString kod = QInputDialog::getText(
        this
        , tr("Podaj kod gry")
        , tr("Kod gry")
        , QLineEdit::Normal
        , QStringLiteral("123")
    );
    if (kod.isEmpty())
        return;
    QJsonObject information;
    information.insert("code", kod);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, CODECHECKPLAYER);
}

void PlayerWindow::connectedToServer(){
    ui->connectButton->setEnabled(false);
    const QString newUsername = QInputDialog::getText(
                this
                , tr("Podaj nazwe uzytkownika")
                , tr("Nick")
    );
    if (newUsername.isEmpty()){
        return m_playerClient->disconnectFromHost();
    }
    playerName=newUsername;
    QJsonObject information;
    information.insert("account", newUsername);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(newUsername.toUtf8().toInt(), loginMsg, LOGIN);
}


void PlayerWindow::loggedIn(){
    ui->chatView->setEnabled(true);
    ui->gameButton->setEnabled(true);
    m_lastUserName.clear();
    QMessageBox::information(this, tr("Connection"), tr("You have sucessfully logged in"));
}

void PlayerWindow::loginFailed(const QString &reason){
    QMessageBox::critical(this, tr("Error"), reason);
    connectedToServer();
}


void PlayerWindow::game(QJsonArray &questions){
    ui->gameButton->setEnabled(false);
    ui->ans1->setEnabled(true);
    ui->ans2->setEnabled(true);
    ui->ans3->setEnabled(true);
    ui->ans4->setEnabled(true);
    while(!questions.empty())
    {
        QJsonObject doc = questions.at(0).toObject();
        QJsonValue question = doc["ques"];
        QJsonValue answer1 = doc["anso"];
        QJsonValue answer2 = doc["anst"];
        QJsonValue answer3 = doc["ansth"];
        QJsonValue answer4 = doc["ansf"];
        QJsonValue correct = doc["cor"];
        QJsonValue time = doc["time"];
        QString time2 = time.toString();
        CorrectAnswer = correct.toString();
        int time3 = time2.toInt();
        qDebug()<<time3;
        qDebug()<<doc;
        questions.pop_front();
        qDebug()<<questions;
        //const QString &id, const QString &question, const QString &answer1,const QString &answer2,const QString &answer3,const QString &answer4,const QString &correct
        //CorrectAnswer=correct;
        m_playerModel->removeRows(0,m_playerModel->rowCount());
        ui->gameButton->setEnabled(false);
        ui->ans1->setEnabled(true);
        ui->ans2->setEnabled(true);
        ui->ans3->setEnabled(true);
        ui->ans4->setEnabled(true);
        int newRow = m_playerModel->rowCount();
        QFont boldFont;
        boldFont.setBold(true);
        m_playerModel->insertRows(newRow, 7);
        m_playerModel->setData(m_playerModel->index(newRow, 0), "You have: " + time2 +" seconds to answer");
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        m_playerModel->setData(m_playerModel->index(newRow, 0), boldFont, Qt::FontRole);
         ++newRow;
        m_playerModel->setData(m_playerModel->index(newRow, 0), question.toString() + QLatin1Char(':'));
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        m_playerModel->setData(m_playerModel->index(newRow, 0), boldFont, Qt::FontRole);
         ++newRow;
        m_playerModel->setData(m_playerModel->index(newRow, 0), answer1.toString());
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ++newRow;
        m_playerModel->setData(m_playerModel->index(newRow, 0), answer2.toString());
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);// scroll the view to display the new message
        ++newRow;
        m_playerModel->setData(m_playerModel->index(newRow, 0), answer3.toString());
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ++newRow;
        m_playerModel->setData(m_playerModel->index(newRow, 0), answer4.toString());
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ui->chatView->scrollToBottom();
        delay(time3);
        delay(2);
        if(selectedAnswer ==CorrectAnswer)
        {
            ++newRow;
            m_playerModel->setData(m_playerModel->index(newRow, 0), "Answer Correct!");
            m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
            ui->chatView->scrollToBottom();
        }
        else
        {
            ++newRow;
            m_playerModel->setData(m_playerModel->index(newRow, 0), "Answer incorrect!");
            m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
            ui->chatView->scrollToBottom();
        }
        delay(5);
    }
    delay(5);
    m_playerModel->removeRows(0,m_playerModel->rowCount());
    QJsonObject information;
    QString nazwa="";
    information.insert("code", gameCode);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(nazwa.toUtf8().toInt(), loginMsg, CREATOR_RANK);
}
void PlayerWindow::showRanking(const QJsonObject &ranking)
{
    QJsonValue gamesn = ranking["gamesn"];
    QJsonValue playersn = ranking["playersn"];
    QString playersn_str = playersn.toString();
    int playersn_int = playersn_str.toInt();
    int sum = playersn_int+1;
    int newRow = m_playerModel->rowCount();
    m_playerModel->insertRows(newRow, sum);
    QFont boldFont;
    boldFont.setBold(true);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "Current players ranking:");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_playerModel->setData(m_playerModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    for(int i=1;i<=playersn_int;i++)
    {
        QString index = "Rank"+QString::number(i);
        QJsonValue creator_info = ranking[index];
        m_playerModel->setData(m_playerModel->index(newRow, 0), creator_info.toString());
        m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ++newRow;
    }
}
void PlayerWindow::passGameCode(const QString &text)
{
    gameCode=text;
    ui->gameButton->setEnabled(false);
    int newRow = m_playerModel->rowCount();
    QFont boldFont;
    boldFont.setBold(true);
    m_playerModel->insertRows(newRow, 2);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "Oczekiwanie na rozpoczecie gry");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_playerModel->setData(m_playerModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    m_playerModel->setData(m_playerModel->index(newRow, 0), "Twoj kod do gry to" + QLatin1Char(': ')+text);
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
}
void PlayerWindow::answer1()
{
    selectedAnswer = "1";
    QJsonObject information;
    information.insert("code",gameCode);
    information.insert("name",playerName);
    if(CorrectAnswer=="1")
    {
        information.insert("info", "correct");
    }
    else
    {
        information.insert("info", "incorrect");
    }
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    qDebug()<<playerName;
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, ANSWER);
    const int newRow = m_playerModel->rowCount();
    m_playerModel->insertRow(newRow);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "You have sent answer number 1");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
    ui->ans1->setEnabled(false);
    ui->ans2->setEnabled(false);
    ui->ans3->setEnabled(false);
    ui->ans4->setEnabled(false);
}
void PlayerWindow::answer2()
{
    selectedAnswer = "2";
    QJsonObject information;
    information.insert("code",gameCode);
    information.insert("name",playerName);
    if(CorrectAnswer=="2")
    {
        information.insert("info", "correct");
    }
    else
    {
        information.insert("info", "incorrect");
    }
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, ANSWER);
    const int newRow = m_playerModel->rowCount();
    m_playerModel->insertRow(newRow);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "You have sent answer number 2");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
    ui->ans1->setEnabled(false);
    ui->ans2->setEnabled(false);
    ui->ans3->setEnabled(false);
    ui->ans4->setEnabled(false);
}
void PlayerWindow::answer3()
{
    selectedAnswer = "3";
    QJsonObject information;
    information.insert("code",gameCode);
    information.insert("name",playerName);
    if(CorrectAnswer=="3")
    {
        information.insert("info", "correct");
    }
    else
    {
        information.insert("info", "incorrect");
    }
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, ANSWER);
    const int newRow = m_playerModel->rowCount();
    m_playerModel->insertRow(newRow);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "You have sent answer number 3");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
    ui->ans1->setEnabled(false);
    ui->ans2->setEnabled(false);
    ui->ans3->setEnabled(false);
    ui->ans4->setEnabled(false);
}
void PlayerWindow::answer4()
{
    selectedAnswer = "4";
    QJsonObject information;
    information.insert("code",gameCode);
    information.insert("name",playerName);
    if(CorrectAnswer=="4")
    {
        information.insert("info", "correct");
    }
    else
    {
        information.insert("info", "incorrect");
    }
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_playerClient->writeText(playerName.toUtf8().toInt(), loginMsg, ANSWER);
    const int newRow = m_playerModel->rowCount();
    m_playerModel->insertRow(newRow);
    m_playerModel->setData(m_playerModel->index(newRow, 0), "You have sent answer number 4");
    m_playerModel->setData(m_playerModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
    ui->ans1->setEnabled(false);
    ui->ans2->setEnabled(false);
    ui->ans3->setEnabled(false);
    ui->ans4->setEnabled(false);
}

void PlayerWindow::disconnectedFromServer()
{
    QMessageBox::warning(this, tr("Disconnected"), tr("The host terminated the connection"));
    ui->ans1->setEnabled(false);
    ui->chatView->setEnabled(false);
    ui->ans2->setEnabled(false);
    ui->ans3->setEnabled(false);
    ui->ans4->setEnabled(false);
    ui->gameButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    m_lastUserName.clear();
}


void PlayerWindow::error(QAbstractSocket::SocketError socketError)
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
