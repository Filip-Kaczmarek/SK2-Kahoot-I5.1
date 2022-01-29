#include <QApplication>
#include "adminwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AdminWindow adminWin;
    adminWin.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()),&adminWin, SLOT(sendClose()));
    return a.exec();
}
