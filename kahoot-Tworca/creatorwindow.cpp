#include "creatorwindow.h"
#include "ui_creatorwindow.h"
#include "clientcreator.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCloseEvent>

int gamecode;
QString nazwa;
CreatorWindow::CreatorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CreatorWindow)
    , m_creatorClient(new ClientCreator(this))
    , m_creatorModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_creatorModel->insertColumn(0);
    ui->chatView->setModel(m_creatorModel);
    connect(m_creatorClient, &ClientCreator::connected, this, &CreatorWindow::connectedToServer);
    connect(m_creatorClient, &ClientCreator::loggedIn, this, &CreatorWindow::loggedIn);
    connect(m_creatorClient, &ClientCreator::ranking, this, &CreatorWindow::showRanking);
    connect(m_creatorClient, &ClientCreator::loginError, this, &CreatorWindow::loginFailed);
    connect(m_creatorClient, &ClientCreator::gameCodeReceived, this, &CreatorWindow::gameCodeReceived);
    connect(m_creatorClient, &ClientCreator::disconnected, this, &CreatorWindow::disconnectedFromServer);
    connect(m_creatorClient, &ClientCreator::error, this, &CreatorWindow::error);
    connect(m_creatorClient, &ClientCreator::waitForPlayers, this, &CreatorWindow::waitForPlayers);
    connect(ui->connectButton, &QPushButton::clicked, this, &CreatorWindow::attemptConnection);
    connect(ui->sendButton, &QPushButton::clicked, this, &CreatorWindow::sendAnswer);
    connect(ui->saveButton, &QPushButton::clicked, this, &CreatorWindow::saveGame);
    connect(ui->startButton, &QPushButton::clicked, this, &CreatorWindow::startGame);
    connect(ui->rankButton, &QPushButton::clicked, this, &CreatorWindow::requestRanking);
    connect(ui->questionEdit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->timeEdit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->ans1Edit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->ans2Edit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->ans3Edit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->ans4Edit, &QLineEdit::returnPressed, this, &CreatorWindow::sendAnswer);
    connect(ui->questionEdit, &QLineEdit::textChanged, this, &CreatorWindow::checkEdits);
    connect(ui->ans1Edit, &QLineEdit::textChanged, this, &CreatorWindow::checkEdits);
    connect(ui->ans2Edit, &QLineEdit::textChanged, this, &CreatorWindow::checkEdits);
    connect(ui->ans3Edit, &QLineEdit::textChanged, this, &CreatorWindow::checkEdits);
    connect(ui->ans4Edit, &QLineEdit::textChanged, this, &CreatorWindow::checkEdits);
    connect(ui->gameButton, &QPushButton::clicked, this, &CreatorWindow::requestCode);
}

void CreatorWindow::delay(int s){
    QTime dieTime= QTime::currentTime().addSecs(s);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void CreatorWindow::requestRanking(){
    m_creatorModel->removeRows(0,m_creatorModel->rowCount());
    ui->rankButton->setEnabled(false);
    QMessageBox::information(this, tr("Connection"), tr("You have requested ranking from server!"));
    QJsonObject information;
    QString nazwa="";
    information.insert("code", gamecode);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_creatorClient->writeText(nazwa.toUtf8().toInt(), loginMsg, CREATOR_RANK);
    delay(3);
    ui->rankButton->setEnabled(true);
}

CreatorWindow::~CreatorWindow(){
    delete ui;
}

void CreatorWindow::waitForPlayers(){
    QMessageBox::information(this, tr("Warning"), tr("At least one players has to be waiting in order to start the game!"));
    ui->startButton->setEnabled(true);
    ui->rankButton->setEnabled(false);
}

void CreatorWindow::sendClose(){
    string loginMsg="puste";
    m_creatorClient->writeText(nazwa.toUtf8().toInt(), loginMsg, CLOSE);

}

void CreatorWindow::showRanking(const QJsonObject &ranking){
    QJsonValue gamesn = ranking["gamesn"];
    QJsonValue playersn = ranking["playersn"];
    QString playersn_str = playersn.toString();
    int playersn_int = playersn_str.toInt();
    int sum = playersn_int+1;
    int newRow = m_creatorModel->rowCount();
    m_creatorModel->insertRows(newRow, sum);
    QFont boldFont;
    boldFont.setBold(true);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), "Current players ranking:");
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    for(int i=1;i<=playersn_int;i++)
    {
        QString index = "Rank"+QString::number(i);
        QJsonValue creator_info = ranking[index];
        m_creatorModel->setData(m_creatorModel->index(newRow, 0), creator_info.toString());
        m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        ++newRow;
    }
}

void CreatorWindow::checkEdits(){

    if(!ui->questionEdit->text().isEmpty() && !ui->ans1Edit->text().isEmpty() &&
            !ui->ans2Edit->text().isEmpty() && !ui->ans3Edit->text().isEmpty() &&
            !ui->ans4Edit->text().isEmpty() && !ui->timeEdit->text().isEmpty())
    {
        ui->sendButton->setEnabled(true);
    }
    else
    {
        ui->sendButton->setEnabled(false);
    }
}

void CreatorWindow::requestCode(){
    QMessageBox::information(this, tr("Connection"), tr("You have sucessfully created the game!"));
    QJsonObject information;
    information.insert("type", "request");
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_creatorClient->writeText(nazwa.toUtf8().toInt(), loginMsg, REQUESTCODE);
}

void CreatorWindow::saveGame(){
    QMessageBox::information(this, tr("Game"), tr("You have sucessfully saved the game!"));
    ui->saveButton->setEnabled(false);
    ui->startButton->setEnabled(true);
    ui->gameButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->questionEdit->setEnabled(false);
    ui->ans1Edit->setEnabled(false);
    ui->ans2Edit->setEnabled(false);
    ui->ans3Edit->setEnabled(false);
    ui->ans4Edit->setEnabled(false);
    ui->correctBox->setEnabled(false);
    ui->timeEdit->setEnabled(false);

}
void CreatorWindow::startGame(){
    int newRow = m_creatorModel->rowCount();
    QFont boldFont;
    boldFont.setBold(true);
    m_creatorModel->insertRows(newRow, 1);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), "You have sucessfully started the game!");
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), boldFont, Qt::FontRole);
    ui->chatView->scrollToBottom();
    ui->timeEdit->setEnabled(false);
    ui->ans1Edit->setEnabled(false);
    ui->ans2Edit->setEnabled(false);
    ui->ans3Edit->setEnabled(false);
    ui->ans4Edit->setEnabled(false);
    ui->questionEdit->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->timeEdit->setEnabled(false);
    ui->rankButton->setEnabled(true);

    QJsonObject information;
    information.insert("code", gamecode);
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_creatorClient->writeText(nazwa.toUtf8().toInt(), loginMsg, STARTGAME);

}
void CreatorWindow::attemptConnection(){
    const QString hostAddress = QInputDialog::getText(
        this
        , tr("Chose Server")
        , tr("Server Address")
        , QLineEdit::Normal
        , QStringLiteral("127.0.0.1")
    );
    if (hostAddress.isEmpty())
        return;
    m_creatorClient->connectToServer(QHostAddress(hostAddress), 1967);
}

void CreatorWindow::connectedToServer(){
    ui->connectButton->setEnabled(false);
    const QString newUsername = QInputDialog::getText(
                this
                , tr("Podaj nazwe uzytkownika")
                , tr("Nick")
    );
    if (newUsername.isEmpty()){
        return m_creatorClient->disconnectFromHost();
    }
    nazwa=newUsername;
    QJsonObject information;
    information.insert("account", newUsername);
    information.insert("password", "wiktor123");
    QJsonDocument doc;
    doc.setObject(information);
    string loginMsg =doc.toJson(QJsonDocument::Compact).toStdString();
    m_creatorClient->writeText(newUsername.toUtf8().toInt(), loginMsg, LOGIN);
}

void CreatorWindow::attemptLogin(const QString &userName){
    m_creatorClient->login(userName);
}

void CreatorWindow::loggedIn(){
    ui->chatView->setEnabled(true);
    ui->gameButton->setEnabled(true);
    m_lastUserName.clear();
    QMessageBox::information(this, tr("Connection"), tr("You have sucessfully logged in"));
}

void CreatorWindow::loginFailed(const QString &reason){
    QMessageBox::critical(this, tr("Error"), reason);
    connectedToServer();
}

void CreatorWindow::gameCodeReceived(int code){
    gamecode = code;
    ui->questionEdit->setPlaceholderText("Type in your question");
    ui->ans1Edit->setPlaceholderText("Type in answer number 1");
    ui->ans2Edit->setPlaceholderText("Type in answer number 2");
    ui->ans3Edit->setPlaceholderText("Type in answer number 3");
    ui->ans4Edit->setPlaceholderText("Type in answer number 4");
    ui->timeEdit->setPlaceholderText("Type in time for question");
    ui->gameButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->timeEdit->setEnabled(true);
    ui->questionEdit->setEnabled(true);
    ui->ans1Edit->setEnabled(true);
    ui->ans2Edit->setEnabled(true);
    ui->ans3Edit->setEnabled(true);
    ui->ans4Edit->setEnabled(true);
    ui->correctBox->setEnabled(true);
    int newRow = m_creatorModel->rowCount();
    QFont boldFont;
    boldFont.setBold(true);
    m_creatorModel->insertRows(newRow, 2);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), "Twoj kod do gry to" + QLatin1Char(':'));
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), boldFont, Qt::FontRole);
    ++newRow;
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), gamecode);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
}

void CreatorWindow::showCommunicate(){
    const int newRow = m_creatorModel->rowCount();
    QFont boldFont;
    boldFont.setBold(true);
    m_creatorModel->insertRows(newRow, 1);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), "Question was received by the server");
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chatView->scrollToBottom();
}

void CreatorWindow::sendAnswer()
{
    m_creatorClient->sendAnswer(
                nazwa,
                ui->timeEdit->text(),
                ui->questionEdit->text(),
                ui->ans1Edit->text(),
                ui->ans2Edit->text(),
                ui->ans3Edit->text(),
                ui->ans4Edit->text(),
                ui->correctBox->text(),
                gamecode
    );
    const int newRow = m_creatorModel->rowCount();
    m_creatorModel->insertRow(newRow);
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), "You have sent set of questions and answers!");
    m_creatorModel->setData(m_creatorModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->questionEdit->clear();
    ui->ans1Edit->clear();
    ui->ans2Edit->clear();
    ui->ans3Edit->clear();
    ui->ans4Edit->clear();
    ui->chatView->scrollToBottom();
    m_lastUserName.clear();
    ui->saveButton->setEnabled(true);
}

void CreatorWindow::disconnectedFromServer(){
    QMessageBox::warning(this, tr("Disconnected"), tr("The host terminated the connection"));
    ui->sendButton->setEnabled(false);
    ui->gameButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->questionEdit->setEnabled(false);
    ui->chatView->setEnabled(false);
    ui->ans1Edit->setEnabled(false);
    ui->ans2Edit->setEnabled(false);
    ui->ans3Edit->setEnabled(false);
    ui->ans4Edit->setEnabled(false);
    ui->rankButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    m_lastUserName.clear();
}

void CreatorWindow::error(QAbstractSocket::SocketError socketError){
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
    ui->sendButton->setEnabled(false);
    ui->questionEdit->setEnabled(false);
    ui->chatView->setEnabled(false);
    m_lastUserName.clear();
}
