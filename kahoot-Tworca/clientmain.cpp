#include <QApplication>
#include "creatorwindow.h"
#include <QObject>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CreatorWindow creatorWin;
    creatorWin.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()),&creatorWin, SLOT(sendClose()));
    return a.exec();
}
