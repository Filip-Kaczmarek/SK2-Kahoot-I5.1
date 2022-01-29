#include <QApplication>
#include "playerwindow.h"
#include <QObject>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerWindow playerWin;
    playerWin.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()),&playerWin, SLOT(sendClose()));
    return a.exec();
}
